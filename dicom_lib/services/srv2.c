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
**			dequeCommand
**			enqueCommand
**			createFile
** Author, Date:	Stephen M. Moore, 15-Apr-93
** Intent:		This module contains general routines which are used
**			in our implementation of service classes.  These
**			routines allow users to request and accept service
**			classes, build and manipulate the public DUL
**			structures, send and receive messages, and request
**			unique Message IDs.
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:11 $
** Source File:		$RCSfile: srv2.c,v $
** Revision:		$Revision: 1.19 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.19 $ $RCSfile: srv2.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#ifdef MALLOC_DEBUG
#include "malloc.h"
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
#include "../thread/ctnthread.h"
#include "../services/dicom_services.h"
#include "private.h"

#define FRAGMENTMAX 65536
typedef unsigned long SRVPERMITTED;

/*extern DUL_PDVLIST pdvList; */

typedef struct {
    void 						*reserved[2];
    DUL_ASSOCIATIONKEY 			**association;
    DUL_PRESENTATIONCONTEXTID 	ctxID;
    unsigned short 				command;
    MSG_TYPE 					messageType;
    void 						*message;
}   COMMAND_ENTRY;

static LST_HEAD *commandList = NULL;	/* Need to do something for threads */

extern CTNBOOLEAN PRVSRV_debug;

static CONDITION
createFile(char *dirName, char *qualifiedFile, int *fd);
CONDITION
PRVSRV_verifyCommandValidity(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXTID ctxid, unsigned short command);
static CONDITION
dequeCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXTID ctxID, MSG_TYPE messageType, COMMAND_ENTRY * commandEntry);
static CONDITION
enqueCommand(COMMAND_ENTRY * commandEntry);

/* SRV_ReceiveCommand
**
** Purpose:
**	Check the status of the network and receive the next command on the
**	network.
**
** Parameter Dictionary:
**	association	The key which describes which association to use to
**			check the network for a command.
**	params		The set of service parameters which describe the
**			services which are valid for this association.
**	block		A flag indicating if the caller wishes to block
**			waiting for a command (DUL_BLOCK) or return
**			immediately if there is no command present
**			(DUL_NOBLOCK).
**	timeout		If the application chooses to block and wait for a
**			command, the amount of time to wait before returning
**			to the caller (in seconds).
**	ctxId		Pointer to a caller variable where this function will
**			store the presentation context ID for the command
**			received from the network.
**	command		Pointer to caller variable where this function will
**			store the command value from the COMMAND group which
**			was read from the network.
**	messageType	Pointer to caller variable where this function will
**			store one of the enumerated message types from the
**			MSG facility. There should be a one to one
**			correspondence between the COMMAND received from the
**			network and this messageType.
**	messageArg	Address of pointer in caller's space. This function
**			allocates a MSG structure and writes the address of the
**			allocated structure in the caller's messageArg pointer.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PARSEFAILED
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**	SRV_UNSUPPORTEDCOMMAND
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SRV_ReceiveCommand(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_BLOCKOPTIONS block, int timeout,
				   DUL_PRESENTATIONCONTEXTID * ctxID, unsigned short *command, MSG_TYPE * messageType, void **messageArg)
{
    CONDITION			cond;
    unsigned char	    stream[8192], *s;
    unsigned long	    bytesRead;
    DUL_DATAPDV			type;
    CTNBOOLEAN			last;
    DUL_PDV				pdv;
    DCM_OBJECT			* object;
    MSG_GENERAL			** msg;
    unsigned short      dcmCommand;

    msg = (MSG_GENERAL **) messageArg;
    *ctxID = 0;

    if (PRVSRV_debug) fprintf(DEBUG_DEVICE, "------ Beginning to read a COMMAND PDV ------\n");

    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_COMMANDPDV; type == DUL_COMMANDPDV && !last;) {
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)) {
    		(void) COND_PushCondition(SRV_OBJECTTOOLARGE, SRV_Message(SRV_OBJECTTOOLARGE), "SRV_ReceiveCommand");
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (*ctxID == 0) *ctxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != *ctxID) {
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (pdv.pdvType != DUL_COMMANDPDV) {
    		(void) COND_PushCondition(SRV_PDVRECEIVEDOUTOFSEQUENCE, SRV_Message(SRV_PDVRECEIVEDOUTOFSEQUENCE), "Command", "Data", pdv.presentationContextID, "SRV_ReceiveCommand");
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}

    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    	type = pdv.pdvType;
    }
    if (type != DUL_COMMANDPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    }
    /* repair */
    cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN, &object);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    if (PRVSRV_debug) (void) DCM_DumpElements(&object, 0);

    /* repair */
    {
	 void 			*ctx;
	 U32 			elementLength;
	 DCM_ELEMENT 	element = {DCM_CMDCOMMANDFIELD,	DCM_US, "", 1, sizeof(dcmCommand), NULL};

	 element.d.us = &dcmCommand;

	 ctx = NULL;
	 cond = DCM_GetElementValue(&object, &element, &elementLength, &ctx);
	 if (cond != DCM_NORMAL) {
		 (void) DCM_CloseObject(&object);
		 return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
	 }
	 if (command != NULL) *command = dcmCommand;
	 if (PRVSRV_debug) fprintf(DEBUG_DEVICE, "Command received: %04x\n", (int) dcmCommand);

	 /* we verify if the command is applicable for the SOP class that is negotiated for the association */
	 cond = PRVSRV_verifyCommandValidity(params, *ctxID, dcmCommand);
	 if (cond != SRV_NORMAL) {
		 (void) DCM_CloseObject(&object);
		 return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");
	 }

	 switch (dcmCommand) {
		 case DCM_ECHO_REQUEST:
		 case DCM_ECHO_RESPONSE:
		 case DCM_STORE_REQUEST:
		 case DCM_STORE_RESPONSE:
		 case DCM_FIND_REQUEST:
		 case DCM_FIND_RESPONSE:
		 case DCM_MOVE_REQUEST:
		 case DCM_MOVE_RESPONSE:
		 case DCM_GET_REQUEST:
		 case DCM_GET_RESPONSE:
		 case DCM_CANCEL_REQUEST:
		 case DCM_N_EVENT_REPORT_REQUEST:
		 case DCM_N_EVENT_REPORT_RESPONSE:
		 case DCM_N_GET_REQUEST:
		 case DCM_N_GET_RESPONSE:
		 case DCM_N_SET_REQUEST:
		 case DCM_N_SET_RESPONSE:
		 case DCM_N_ACTION_REQUEST:
		 case DCM_N_ACTION_RESPONSE:
		 case DCM_N_CREATE_REQUEST:
		 case DCM_N_CREATE_RESPONSE:
		 case DCM_N_DELETE_REQUEST:
		 case DCM_N_DELETE_RESPONSE:
											 *msg = NULL;
											 cond = MSG_ParseCommand(&object, (void **) msg);
											 (void) DCM_CloseObject(&object);
											 if (cond != MSG_NORMAL) return COND_PushCondition(SRV_PARSEFAILED, SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");
											 *messageType = (*msg)->type;
											 break;

		 default:
											 return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");
	 }
    }

    return SRV_NORMAL;
}

/* SRV_ReceiveDataSet
**
** Purpose:
**	Poll an Association and read what is expected to be a data set.
**
** Parameter Dictionary:
**	association	The key used to receive the database from the network.
**	presentationCtx	Presentation context on which we expect to receive
**			the dataset.
**	block		Flag to be passed to network routines for blocking or
**			non-blocking I/O.
**	timeout		Timeout passed to network routines.
**	dirName		Directory to be used to create file for (possibly large)**			data sets.
**	dataSet		Address of DICOM object variable which will be created
**			when data set is received.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_MALLOCFAILURE
**	SRV_NORMAL
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**
** Algorithm:
**	Notes : dirName is an optional parameter. Some datasets are too large
**		to store directly in memory. When the function determines that
**		a large dataset is being read, it will write it to a file. This
**		file will be stored in the directory indicated by the parameter
**		dirName. If dirName is empty or NULL, the current working
**		directory is assumed.
*/

CONDITION
SRV_ReceiveDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, DUL_BLOCKOPTIONS block, int timeout, char *dirName, DCM_OBJECT ** dataSet)
{
    CONDITION					cond;
    unsigned char	    		stream[16384], *s;
    unsigned long       		bytesRead;
    DUL_DATAPDV					type;
    CTNBOOLEAN					last;
    DUL_PDV						pdv;
    DCM_OBJECT					* object;
    DUL_PRESENTATIONCONTEXTID	ctxID = 0;
    int					        fd = 0;	/* File descriptor if we need to create file */
    char				        qualifiedFile[1024];
    char					    *currentDir = ".";

    /* First we deal with the various values possible for the argument * dirName */
    if ((dirName == NULL) || (strlen(dirName) == 0)) dirName = currentDir;	/* NULL or empty argument passed */

    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_DATASETPDV; type == DUL_DATASETPDV && !last;) {
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)) {
    		if (fd == 0) {
    			cond = createFile(dirName, qualifiedFile, &fd);
    			if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    		}
    		if (write(fd, stream, bytesRead) != (int)bytesRead) {
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    		}
    		s = stream;
    		bytesRead = 0;
    	}
    	if (ctxID == 0) ctxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != ctxID) {
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    	}

    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    }
    if (type != DUL_DATASETPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    }
    if (fd != 0) {
    	if (write(fd, stream, bytesRead) != (int)bytesRead) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
	  	(void) close(fd);
    	cond = DCM_OpenFile(qualifiedFile, DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION | DCM_DELETEONCLOSE, &object);
    }else{
    	/* repair */
    	cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION, &object);
    	if (cond != DCM_NORMAL) {
    		int 	fd1;
    		char 	scratch[1024];

    		strcpy(scratch, dirName);
    		strcat(scratch, "/ctn_bad_dataset.dcm");
#ifdef _WIN32
    		fd1 = _open(scratch, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, _S_IREAD | _S_IWRITE);
#else
    		fd1 = open(scratch, O_WRONLY | O_CREAT | O_TRUNC, 0666);
#endif
    		write(fd1, stream, bytesRead);
    		close(fd1);
    	}
    }
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    if (PRVSRV_debug) (void) DCM_DumpElements(&object, 0);

    *dataSet = object;

    return SRV_NORMAL;
}

/* SRV_TestForCancel
**
** Purpose:
**	Check the status of the network and read a cancel request
**	(if present).
**
** Parameter Dictionary:
**	association	The key which describes which association to use to
**			check the network for a command.
**	params		The set of service parameters which describe the
**			services which are valid for this association.
**	block		A flag indicating if the caller wishes to block
**			waiting for a command (DUL_BLOCK) or return
**			immediately if there is no command present
**			(DUL_NOBLOCK).
**	timeout		If the application chooses to block and wait for a
**			command, the amount of time to wait before returning
**			to the caller (in seconds).
**	ctxId		The presentation context ID indicating under which
**			context we might expect a CANCEL Request.
**	command		Pointer to caller variable where this function will
**			store the command value from the COMMAND group which
**			was read from the network.
**	messageType	Pointer to caller variable where this function will
**			store one of the enumerated message types from the
**			MSG facility. There should be a one to one
**			correspondence between the COMMAND received from the
**			network and this messageType.
**	messageArg	Address of pointer in caller's space. This function
**			allocates a MSG structure and writes the address of the
**			allocated structure in the caller's messageArg pointer.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PARSEFAILED
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**	SRV_UNSUPPORTEDCOMMAND
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_TestForCancel(DUL_ASSOCIATIONKEY ** association, DUL_BLOCKOPTIONS block, int timeout, DUL_PRESENTATIONCONTEXTID ctxID, unsigned short *command, MSG_TYPE * messageType, void **messageArg)
{
    CONDITION					cond;
    unsigned char       		stream[8192], *s;
    unsigned long       		bytesRead;
    DUL_DATAPDV					type;
    CTNBOOLEAN					last;
    DUL_PDV						pdv;
    DCM_OBJECT					* object;
    MSG_GENERAL					** msg;
    unsigned short	    		dcmCommand;
    COMMAND_ENTRY				commandEntry;
    DUL_PRESENTATIONCONTEXTID	localCtxID;

    cond = dequeCommand(association, ctxID, MSG_K_C_CANCEL_REQ, &commandEntry);
    if (cond == SRV_NORMAL) {
    	*command = commandEntry.command;
    	*messageType = commandEntry.messageType;
    	*messageArg = commandEntry.message;
    	return SRV_NORMAL;
    }
    msg = (MSG_GENERAL **) messageArg;
    localCtxID = 0;

    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_COMMANDPDV; type == DUL_COMMANDPDV && !last;) {
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)) {
    		(void) COND_PushCondition(SRV_OBJECTTOOLARGE, SRV_Message(SRV_OBJECTTOOLARGE));
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (localCtxID == 0) localCtxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != localCtxID) {
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    	type = pdv.pdvType;
    }
    if (type != DUL_COMMANDPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    }
    /* repair */
    cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN, &object);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    if (PRVSRV_debug)(void) DCM_DumpElements(&object, 0);

    /* repair */
    {
 	 void	 		*ctx;
	 U32 	 		elementLength;
	 DCM_ELEMENT 	element = {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(dcmCommand), NULL};

	 element.d.us = &dcmCommand;

	 ctx = NULL;

	 cond = DCM_GetElementValue(&object, &element, &elementLength, &ctx);
	 if (cond != DCM_NORMAL) {
		 (void) DCM_CloseObject(&object);
		 return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
	 }

	 switch (dcmCommand) {
		 case DCM_CANCEL_REQUEST:
											 *msg = NULL;
											 cond = MSG_ParseCommand(&object, (void **) msg);
											 (void) DCM_CloseObject(&object);
											 if (cond != MSG_NORMAL) return COND_PushCondition(SRV_PARSEFAILED,	SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");
											 *messageType = (*msg)->type;
											 if (command != NULL) *command = dcmCommand;
											 break;
		 case DCM_ECHO_REQUEST:
		 case DCM_ECHO_RESPONSE:
		 case DCM_STORE_REQUEST:
		 case DCM_STORE_RESPONSE:
		 case DCM_FIND_REQUEST:
		 case DCM_FIND_RESPONSE:
		 case DCM_MOVE_REQUEST:
		 case DCM_MOVE_RESPONSE:
		 case DCM_GET_REQUEST:
		 case DCM_GET_RESPONSE:
		 case DCM_N_EVENT_REPORT_REQUEST:
		 case DCM_N_EVENT_REPORT_RESPONSE:
		 case DCM_N_GET_REQUEST:
		 case DCM_N_GET_RESPONSE:
		 case DCM_N_SET_REQUEST:
		 case DCM_N_SET_RESPONSE:
		 case DCM_N_ACTION_REQUEST:
		 case DCM_N_ACTION_RESPONSE:
		 case DCM_N_CREATE_REQUEST:
		 case DCM_N_CREATE_RESPONSE:
		 case DCM_N_DELETE_REQUEST:
		 case DCM_N_DELETE_RESPONSE:
											 commandEntry.message = NULL;
											 cond = MSG_ParseCommand(&object, (void **) &commandEntry.message);
											 (void) DCM_CloseObject(&object);
											 if (cond != MSG_NORMAL) return COND_PushCondition(SRV_PARSEFAILED, SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");
											 commandEntry.association = association;
											 commandEntry.ctxID = localCtxID;
											 commandEntry.command = dcmCommand;
											 commandEntry.messageType = ((MSG_GENERAL *) commandEntry.message)->type;
											 cond = enqueCommand(&commandEntry);
											 if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
											 break;

		 default:
											 return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");
	 }
    }

    return SRV_NORMAL;
}

/*
**  =======================================================
**  Following are  private functions which are not available outside
**  this module
*/

/* SRVPRV_ReadNextPDV
**
** Purpose:
**	Read the next PDV
**
** Parameter Dictionary:
**	association	Handle to the Association
**	block		Blocking options
**	timeout		Time interval within which to read
**	pdv		Handle to the pdv
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READPDVFAILED
**	SRV_READTIMEOUT
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SRVPRV_ReadNextPDV(DUL_ASSOCIATIONKEY ** association, DUL_BLOCKOPTIONS block, int timeout, DUL_PDV * pdv)
{
    CONDITION			cond;
    /* DUL_PDVLIST 		pdvList; */
    char 				msg[128];

    cond = DUL_NextPDV(association, pdv);
    if (cond != DUL_NORMAL) {
    	(void) COND_PopCondition(FALSE);
		cond = DUL_ReadPDVs(association, NULL, block, timeout);
		if (cond != DUL_PDATAPDUARRIVED){
			if (cond == DUL_NULLKEY || cond == DUL_ILLEGALKEY){
				return COND_PushCondition(SRV_ILLEGALASSOCIATION, SRV_Message(SRV_ILLEGALASSOCIATION));
			}else if (cond == DUL_PEERREQUESTEDRELEASE){
				return COND_PushCondition(SRV_PEERREQUESTEDRELEASE, SRV_Message(SRV_PEERREQUESTEDRELEASE), "SRVPRV_ReadNextPDV");
			}else if (cond == DUL_PEERABORTEDASSOCIATION){
				return COND_PushCondition(SRV_PEERABORTEDASSOCIATION, SRV_Message(SRV_PEERABORTEDASSOCIATION), "SRVPRV_ReadNextPDV");
			}else if (cond == DUL_READTIMEOUT){
				return SRV_READTIMEOUT;
			}else{
				sprintf(msg, "DUL Error: %08lx\n", cond);
				COND_PushCondition(cond, msg);
				return COND_PushCondition(SRV_READPDVFAILED, SRV_Message(SRV_READPDVFAILED), "SRVPRV_ReadNextPDV");
			}
		}
		cond = DUL_NextPDV(association, pdv);
		if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_READPDVFAILED, SRV_Message(SRV_READPDVFAILED)); /* repair */
    }
    return SRV_NORMAL;
}

/* createFile
**
** Purpose:
**	Creates a file in the directory when large datasets are received.
**
** Parameter Dictionary:
**	dirName		Directory in which file is to be created.
**	qualifiedFile	Name assigned to the file which is created.
**	fd		File descriptor.
**
** Return Values:
**
**	SRV_FILECREATEFAILED
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
createFile(char *dirName, char *qualifiedFile, int *fd)
{
    static int        fileIndex = 1;		/* For creating unique file names */
    int		          pid;

    pid = getpid();
    sprintf(qualifiedFile, "%s/%-d.%-d", dirName, pid, fileIndex);
#ifdef CTN_USE_THREADS
    if (THR_ObtainMutex(FAC_SRV) != THR_NORMAL)	return COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), qualifiedFile, "createFile");
#endif
    fileIndex++;
#ifdef CTN_USE_THREADS
    if (THR_ReleaseMutex(FAC_SRV) != THR_NORMAL) return COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), qualifiedFile, "createFile");
#endif

    *fd = open(qualifiedFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (*fd < 0){
    	(void) COND_PushCondition(SRV_SYSTEMERROR, SRV_Message(SRV_SYSTEMERROR), strerror(errno), "createFile");
    	return COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), qualifiedFile, "createFile");
    }else{
    	return SRV_NORMAL;
    }
}

/* dequeCommand
**
** Purpose:
**	Dequeue the command
**
** Parameter Dictionary:
**	association		Handle to the Association
**	ctxID			Presentation context ID
**	messageType		Type of the message
**	commandEntry		Handle to command entry in the table
**
** Return Values:
**
**	SRV_EMPTYCOMMANDQUEUE
**	SRV_NOAVAILABLECOMMAND
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
dequeCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXTID ctxID, MSG_TYPE messageType, COMMAND_ENTRY * commandEntry)
{
    COMMAND_ENTRY   	* c;
    CTNBOOLEAN			match;

    if (commandList == NULL) return SRV_EMPTYCOMMANDQUEUE;

    c = LST_Head(&commandList);
    if (c == NULL) return SRV_EMPTYCOMMANDQUEUE;

    (void) LST_Position(&commandList, c);
    match = FALSE;

    while ((c != NULL) && !match) {
    	match = TRUE;
    	if (c->association != association) match = FALSE;
    	if ((ctxID != 0) && (c->ctxID != ctxID)) match = FALSE;
    	if ((messageType != MSG_K_NONE) && (c->messageType != messageType)) match = FALSE;
    	if (!match) c = LST_Next(&commandList);
    }

    if (match){
    	*commandEntry = *c;
    	(void) LST_Remove(&commandList, LST_K_AFTER);
    	CTN_FREE(c);
    	return SRV_NORMAL;
    }else{
    	return SRV_NOAVAILABLECOMMAND;
    }
}

/* enqueCommand
**
** Purpose:
**	Enqueue the command
**
** Parameter Dictionary:
**	commandEntry		Handle to the command entry
**
** Return Values:
**
**	SRV_LISTFAILURE
**	SRV_MALLOCFAILURE
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
enqueCommand(COMMAND_ENTRY * commandEntry)
{
    COMMAND_ENTRY    * c, *tail;

    if (commandList == NULL) commandList = LST_Create();
    if (commandList == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "enqueCommand");

    c = CTN_MALLOC(sizeof(*c));
    if (c == NULL) return COND_PushCondition(SRV_MALLOCFAILURE, SRV_Message(SRV_MALLOCFAILURE), sizeof(*c), "enqueCommand");

    *c = *commandEntry;

    tail = LST_Tail(&commandList);
    (void) LST_Position(&commandList, tail);
    if (LST_Insert(&commandList, c, LST_K_AFTER) != LST_NORMAL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "enqueCommand");

    return SRV_NORMAL;
}
