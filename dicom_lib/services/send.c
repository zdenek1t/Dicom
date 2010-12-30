/*
          Copyright (C) 1993, 1994, RSNA and Washington University

          The software and supporting documentation for the Radiological
          Society of North America (RSNA) 1993, 1994 Digital Imaging and
          Communications in Medicine (DICOM) Demonstration were developed
          at the
                  Electronic Radiology Laboratory
                  Mallinckrodt Institute of Radiology
                  Washington University School of Medicine
                  510 S. Kingshighway Blvd.
                  St. Louis, MO 63110
          as part of the 1993, 1994 DICOM Central Test Node project for, and
          under contract with, the Radiological Society of North America.

          THIS SOFTWARE IS MADE AVAILABLE, AS IS, AND NEITHER RSNA NOR
          WASHINGTON UNIVERSITY MAKE ANY WARRANTY ABOUT THE SOFTWARE, ITS
          PERFORMANCE, ITS MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR
          USE, FREEDOM FROM ANY COMPUTER DISEASES OR ITS CONFORMITY TO ANY
          SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND PERFORMANCE OF
          THE SOFTWARE IS WITH THE USER.

          Copyright of the software and supporting documentation is
          jointly owned by RSNA and Washington University, and free access
          is hereby granted as a license to use this software, copy this
          software and prepare derivative works based upon this software.
          However, any distribution of this software source code or
          supporting documentation or derivative works (source code and
          supporting documentation) must include the three paragraphs of
          the copyright notice.
*/
/* Copyright marker.  Copyright will be inserted above.  Do not remove */

/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
**			SRV_ReceiveCommand
**			SRV_ReceiveDataSet
**			SRV_SendCommand
**			SRV_SendDataSet
**			SRV_TestForCancel
**	private modules
**			SRVPRV_ReadNextPDV
**			writeCallback
**			dequeCommand
**			enqueCommand
** Author, Date:	Stephen M. Moore, 15-Apr-93
** Intent:		This module contains general routines which are used
**			in our implementation of service classes.  These
**			routines allow users to request and accept service
**			classes, build and manipulate the public DUL
**			structures, send and receive messages, and request
**			unique Message IDs.
** Last Update:		$Author: smm $, $Date: 2002/02/26 21:09:26 $
** Source File:		$RCSfile: send.c,v $
** Revision:		$Revision: 1.21 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.21 $ $RCSfile: send.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#ifdef _MSC_VER
#else
#include <sys/file.h>
#endif
#ifdef SOLARIS
#include <sys/fcntl.h>
#endif
#endif

#include "../dicom/dicom.h"
#include "../uid/dicom_uids.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"
#include "private.h"

#define FRAGMENTMAX 65536
typedef struct {
    DUL_ASSOCIATIONKEY 	*association;
    DUL_PDVLIST 		*pdvList;
        CONDITION(*callback) ();
    void 				*ctx;
    unsigned long 		bytesTransmitted;
    unsigned long 		totalBytes;
#ifdef CTN_NO_RUNT_PDVS
    unsigned long 		cumulativeBytesTransmitted;
#endif
}   CALLBACK_STRUCTURE;

typedef unsigned long SRVPERMITTED;

typedef struct {
    char 				classUID[DICOM_UI_LENGTH + 1];
    SRVPERMITTED 		*permittedSrvList;	/* list of permitted services */
    unsigned short 		permittedSrvListSize;
}   SOPCLASSPERMITTEDSRV;	/* defines the various services permitted for a given SOP class */

typedef struct {
    void 						*reserved[2];
    DUL_ASSOCIATIONKEY 			**association;
    DUL_PRESENTATIONCONTEXTID 	ctxID;
    unsigned short 				command;
    MSG_TYPE 					messageType;
    void 						*message;
}   COMMAND_ENTRY;

typedef struct {
  char 				*xferSyntax;
  unsigned long 	options;
} XFER_SYNTAX_MAP;

XFER_SYNTAX_MAP xferSyntaxMap[] = {
	{DICOM_TRANSFERLITTLEENDIAN, DCM_ORDERLITTLEENDIAN },
	{DICOM_TRANSFERLITTLEENDIANEXPLICIT, DCM_EXPLICITLITTLEENDIAN },
	{DICOM_TRANSFERBIGENDIANEXPLICIT, DCM_EXPLICITBIGENDIAN}
};

static int 		simulateError = 0;

void
SRV_SimulateError(int err)
{
  simulateError = err;
}

unsigned long
exportOptionsFromXferSyntax(const char* xferSyntax)
{
  int index = 0;

  for (index = 0; index < DIM_OF(xferSyntaxMap); index++) {
    if (strcmp(xferSyntax, xferSyntaxMap[index].xferSyntax) == 0) return xferSyntaxMap[index].options;
  }
  return DCM_ENCAPSULATEDPIXELS;	/* Assume encapsulated */
}

/*static LST_HEAD *commandList = NULL; */

extern CTNBOOLEAN 	PRVSRV_debug;

static CONDITION
writeCallback(void *buffer, U32 length, int last, void *callbackStructure); /* int last, CALLBACK_STRUCTURE * callbackStructure);*/

/* SRV_SendCommand
**
** Purpose:
**	Send a DICOM command to a peer using an established Association.
**
** Parameter Dictionary:
**	association	Key which describes the Association used for
**			transmitting the command.
**	context		Presentation context to be used when sending a command.
**	object		Address of DICOM object which contains the attributes
**			of the command to be transmitted.
**
** Return Values:
**	SRV_NORMAL on success.
**	SRV_NOTRANSFERSYNTAX
**	SRV_SENDFAILED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_SendCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object)
{
    /* repair */
    unsigned char       	buf[4096];
    unsigned long        	options = DCM_ORDERLITTLEENDIAN;
    CONDITION				cond;
    DUL_PDVLIST				pdvListSend;
    DUL_PDV					pdv;
    CALLBACK_STRUCTURE		callbackStructure;

    memset(&callbackStructure, 0, sizeof(callbackStructure));
    callbackStructure.association = *association;
    callbackStructure.pdvList = &pdvListSend;
    callbackStructure.callback = NULL;

#ifdef CTN_NO_RUNT_PDVS
    callbackStructure.cumulativeBytesTransmitted = 0;
    cond = DCM_ComputeExportLength(object, DCM_ORDERLITTLEENDIAN, &callbackStructure.totalBytes);
#endif

#if 0
    /* Removed this check 12/28/2000, smm.  We no longer require IVRLE */
    if (strcmp(context->acceptedTransferSyntax, DICOM_TRANSFERLITTLEENDIAN)	== 0)
    	options = DCM_ORDERLITTLEENDIAN;
    else
    	return COND_PushCondition(SRV_NOTRANSFERSYNTAX, SRV_Message(SRV_NOTRANSFERSYNTAX), context->abstractSyntax, context->acceptedTransferSyntax, "SRV_SendCommand");
#endif

    pdv.fragmentLength = 0;
    pdv.presentationContextID = context->presentationContextID;
    pdv.pdvType = DUL_COMMANDPDV;
    pdv.lastPDV = 0;
    pdv.data = buf;

    pdvListSend.count = 1;
    pdvListSend.pdv = &pdv;
    cond = DCM_ExportStream(object, options, buf, sizeof(buf), writeCallback, &callbackStructure);
    if (CTN_ERROR(cond)) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "COMMAND", "SRV_SendCommand");
    return SRV_NORMAL;
}

/* SRV_SendDataSet
**
** Purpose:
**	Send a DICOM dataset to a peer using an established Association.
**
** Parameter Dictionary:
**	association	Key which describes the Association used for
**			transmitting the command.
**	context		Presentation context to be used when sending the
**			command.
**	object		Address of DICOM object which contains the dataset to
**			be sent.
**	callback	User callback function which is called periodically
**			while the dataset is sent accross the network. This
**			allows the application to monitor the rate of
**			transmission and cancel the transmission.
**	callbackCtx	User context information which is passed to the
**			callback function as a parameter.
**	length		Length in bytes of the amount of data to transmit
**			over the network before calling the user's callback
**			function.
**
** Return Values:
**
**	SRV_NORMAL
**	SRV_NOTRANSFERSYNTAX
**	SRV_OBJECTACCESSFAILED
**	SRV_SENDFAILED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_SendDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object,	CONDITION(*callback) (), void *callbackCtx, unsigned long length)
{
#if 0
    unsigned char       *buf;
#else
    unsigned char 		buf[16384];
#endif
    unsigned long       options;
    CONDITION			cond;
    DUL_PDVLIST			pdvListSend;
    DUL_PDV				pdv;
    CALLBACK_STRUCTURE	callbackStructure;
    int 				maxPDUsize;
    int 				fudgeFactor = 20;

    callbackStructure.association = *association;
    callbackStructure.pdvList = &pdvListSend;
    callbackStructure.callback = callback;
    callbackStructure.ctx = callbackCtx;
    callbackStructure.bytesTransmitted = 0;

#if 0
    cond = DUL_AssociationParameter(association, DUL_K_MAX_PDV_XMIT, DUL_K_INTEGER, &maxPDUsize, 4);
    if (cond != DUL_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "as unable to get maxPDUsize", "SRV_SendDataSet");
    if (maxPDUsize < fudgeFactor) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "due to invalid maxPDUsize", "SRV_SendDataSet");

    buf = (unsigned char *) CTN_MALLOC(maxPDUsize - fudgeFactor);

    if (buf == NULL) return COND_PushCondition(SRV_MALLOCFAILURE, SRV_Message(SRV_MALLOCFAILURE), maxPDUsize - fudgeFactor, "SRV_SendDataSet");
#else
    maxPDUsize = sizeof(buf);
#endif

#ifdef CTN_NO_RUNT_PDVS
    callbackStructure.cumulativeBytesTransmitted = 0;
    cond = DCM_ComputeExportLength(object, DCM_ORDERLITTLEENDIAN, &callbackStructure.totalBytes);
#else
    cond = DCM_GetObjectSize(object, &callbackStructure.totalBytes);

#endif
    if (cond != DCM_NORMAL) {
#if 0
    	CTN_FREE(buf);
#endif
    	return COND_PushCondition(SRV_OBJECTACCESSFAILED, SRV_Message(SRV_OBJECTACCESSFAILED), "data set", "SRV_SendDataSet");
    }
    options = exportOptionsFromXferSyntax(context->acceptedTransferSyntax);
    if (options == 0) return COND_PushCondition(SRV_NOTRANSFERSYNTAX, SRV_Message(SRV_NOTRANSFERSYNTAX), context->abstractSyntax, context->acceptedTransferSyntax, "SRV_SendDataSet");

    /* Removed 12/28/2000, smm.  We now support other xfer syntaxes
    if (strcmp(context->acceptedTransferSyntax, DICOM_TRANSFERLITTLEENDIAN)	== 0){
		options = DCM_ORDERLITTLEENDIAN;
	}else{
#if 0
		CTN_FREE(buf);
#endif
		return COND_PushCondition(SRV_NOTRANSFERSYNTAX, SRV_Message(SRV_NOTRANSFERSYNTAX), context->abstractSyntax,	context->acceptedTransferSyntax, "SRV_SendDataSet");
    }
    */
    pdv.presentationContextID = context->presentationContextID;
    pdv.pdvType = DUL_DATASETPDV;
    pdv.data = buf;

    pdvListSend.count = 1;
    pdvListSend.pdv = &pdv;
    {
	 cond = DCM_ExportStream(object, options, buf, maxPDUsize - fudgeFactor, writeCallback, &callbackStructure);
	 if (CTN_ERROR(cond)){
#if 0
	 	CTN_FREE(buf);
#endif
	     return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "DATA SET", "SRV_SendDataSet");
	 }
    }
    if (cond != DCM_NORMAL) {
#if 0
    	CTN_FREE(buf);
#endif
    	printf("Unexepected condition after export command: %lx\n", cond);
    	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "DATA SET", "SRV_SendDataSet");
    }
#if 0
    CTN_FREE(buf);
#endif

    return SRV_NORMAL;
}

/*
**  =======================================================
**  Following are  private functions which are not available outside
**  this module
*/

/* writeCallback
**
** Purpose:
**	Call back routine for write.
**
** Parameter Dictionary:
**	buffer		Buffer to be written
**	length		Length of buffer
**	last		Indicates if it is the last PDU
**	callbackStructure
**			Handle to the call back structure
**
** Return Values:
**	DCM_NORMAL
**	SRV_SENDFAILED
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
writeCallback(void *buffer, U32 length, int last, void * callbackStructurePtr)
{
    CONDITION			    cond;
    DUL_PDV					* pdv;
    CALLBACK_STRUCTURE 		*callbackStructure;

    callbackStructure = (CALLBACK_STRUCTURE *) callbackStructurePtr;

    pdv = callbackStructure->pdvList->pdv;
    pdv->fragmentLength = length;
#ifdef CTN_SIMULATE_WRITE_ERROR
    if (simulateError == 1) pdv->fragmentLength--;
#endif
#ifdef CTN_NO_RUNT_PDVS
    callbackStructure->cumulativeBytesTransmitted += length;
    if (length == 0) return DCM_NORMAL;
    pdv->lastPDV = (callbackStructure->totalBytes == callbackStructure->cumulativeBytesTransmitted);
#else
    pdv->lastPDV = last;
#endif

    cond = DUL_WritePDVs(&callbackStructure->association, callbackStructure->pdvList);
    if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "unknown", "writeCallback");

    callbackStructure->bytesTransmitted += length;
    if (callbackStructure->callback != NULL) {
    	cond = callbackStructure->callback(callbackStructure->bytesTransmitted, callbackStructure->totalBytes, callbackStructure->ctx);
    	if (!CTN_SUCCESS(cond)) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "unknown", "writeCallback");
    }
    return DCM_NORMAL;
}
