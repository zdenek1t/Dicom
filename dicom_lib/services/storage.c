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
** Module Name(s):	SRV_CStoreRequest
**			SRV_CStoreResponse
**	private modules
**			localCallback
**			genericImageSize
** Author, Date:	Stephen M. Moore, 22-Apr-93
** Intent:		This module contains routines which implement the
**			storage service class (but user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:11 $
** Source File:		$RCSfile: storage.c,v $
** Revision:		$Revision: 1.39 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.39 $ $RCSfile: storage.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
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
#include "../utility/utility.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"
#include "private.h"


static unsigned long genericImageSize(char *SOPClass);
typedef struct {
    CONDITION			(*callback) ();
    MSG_C_STORE_REQ 	*storeRequest;
    MSG_C_STORE_RESP 	*storeResponse;
    void 				*userCtx;
}   STORAGE_CTX;
static CONDITION
localCallback(unsigned long bytesTransmitted, unsigned long totalBytes, STORAGE_CTX * storageCtx);
typedef struct {
  char 				*xferSyntax;
  unsigned long 	options;
} XFER_SYNTAX_MAP;

static
XFER_SYNTAX_MAP xferSyntaxMap[] = {
	{DICOM_TRANSFERLITTLEENDIAN, DCM_ORDERLITTLEENDIAN },
	{DICOM_TRANSFERLITTLEENDIANEXPLICIT, DCM_EXPLICITLITTLEENDIAN },
	{DICOM_TRANSFERBIGENDIANEXPLICIT, DCM_EXPLICITBIGENDIAN}
};

/*
static unsigned long
exportOptionsFromXferSyntax(const char* xferSyntax)
{
  int index = 0;

  for (index = 0; index < DIM_OF(xferSyntaxMap); index++) {
    if (strcmp(xferSyntax, xferSyntaxMap[index].xferSyntax) == 0)
      return xferSyntaxMap[index].options;
  }
  return 0;
}
*/


static CONDITION
callbackFunction(void* buf, U32 bytesExported, int lastFlag, void* ctx)
{
  int fd = *(int*)ctx;

  if (write(fd, buf, bytesExported) != (int)bytesExported) return 0;

  return DCM_NORMAL;
}

static int
writePart10MetaHeader(int fd, DCM_FILE_META* fileMeta)
{
  DCM_OBJECT* 		obj = 0;
  char 				buffer[2048];

  CONDITION cond = DCM_CreateObject(&obj, 0);
  if (cond != DCM_NORMAL) return -1;

  cond = DCM_SetFileMeta(&obj, fileMeta);
  if (cond != DCM_NORMAL) return -1;

  cond = DCM_ExportStream(&obj, DCM_EXPLICITLITTLEENDIAN | DCM_PART10FILE, buffer, sizeof(buffer), callbackFunction, &fd);
  if (cond != DCM_NORMAL) return -1;

  return 0;
}

static int
setFileMeta(DCM_FILE_META* fileMeta, MSG_C_STORE_REQ* msg, const char* xferSyntax)
{
  memset(fileMeta->preamble, 0, sizeof(fileMeta->preamble));
  fileMeta->fileMetaInformationVersion[0] = 0x01;
  fileMeta->fileMetaInformationVersion[1] = 0x00;
  strcpy(fileMeta->mediaStorageSOPClassUID, msg->classUID);
  strcpy(fileMeta->mediaStorageSOPInstanceUID, msg->instanceUID);
  strcpy(fileMeta->transferSyntaxUID,  xferSyntax);
  strcpy(fileMeta->implementationClassUID, MIR_IMPLEMENTATIONCLASSUID);
  strcpy(fileMeta->implementationVersionName, MIR_IMPLEMENTATIONVERSIONNAME);
  strcpy(fileMeta->sourceApplicationEntityTitle, "XX");
  strcpy(fileMeta->privateInformationCreatorUID, "");

  return 0;
}

/* SRV_CStoreRequest
**
** Purpose:
**	SRV_CStoreRequest assists an application that wants to be an SCU of
**	one of the storage SOP classes.  This function constructs a
**	C-STORE-REQ Message and sends it to the peer application which
**	is acting as the SCP for the storage SOP class.  After the request
**	message is sent, SRV_CStoreRequest sends the data set which
**	contains the object of the store request.
**
**	The user specifies the data set for the operation by placing
**	a legal DICOM Information Object in the MSG_C_STORE_REQ structure
**	or by including a file name in the structure that points to a
**	DICOM Information Object.
**
**	The function calculates the numbers of bytes that are present
**	in the data set and calls the user callback function during the
**	send process.  The callback function is called after each 10%
**	of image transmission.
**
**	The arguments to the callback function are:
**		MSG_C_STORE_REQ	*storeRequest
**		MSG_C_STORE_RESP *storeResponse
**		char		*abstractSyntax
**		unsigned long	bytesTransmitted
**		unsigned long	totalBytes
**		void		*callbackCtx
**	where
**		storeRequest	Pointer to MSG structure with C-STORE request.
**		storeRespponse	Pointer to MSG structure with C-STORE response.
**		abstractSyntax	A character string which identifies the
**				abstract syntax of the SOP Class of the
**				object to be stored.
**		bytesTransmitted Number of bytes transmitted so far.
**		totalBytes	Total number of bytes in data set.
**		callbackCtx	User's callbackCtx argument which is used to
**				maintain context information in the callback
**				function.
**
**	On each invocation of the callback function, the user should examine
**	the storeResponse pointer.  This pointer will be NULL during the
**	store process.  After SRV_CStoreRequest completes the process of
**	sending the image, it waits for the C-STORE RESPONSE from the peer.
**	When this reponse is receive, the callback function is called a
**	final time with the response message.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association.  This
**			list includes the list of presentation contexts.
**	storeRequest	Pointer to structure in caller's memory which contains
**			the STORE request.
**	storeResponse	Address of pointer to response message. This function
**			will create a response message and return the
**			address of the structure to the caller.
**	callback	Address of user routine which is called during the
**			send process and when the response message is received.
**	callbackCtx	User context information which is supplied during call
**			to callback function.
**      dirName         Name of directory where files may be created.
**
** Return Values:
**
**	SRV_CALLBACKABORTEDSERVICE
**	SRV_ILLEGALPARAMETER
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_NOSERVICEINASSOCIATION
**	SRV_OBJECTBUILDFAILED
**	SRV_REQUESTFAILED
**	SRV_UNEXPECTEDCOMMAND
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_CStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResponse,
				  SRV_C_STORE_REQ_CALLBACK * callback, void *callbackCtx, char *dirName)
{
    DCM_OBJECT					* commandObject,	/* Handle for a command object */
								*dataSetObject;		/* Handle for a Data Set object */
    CONDITION					cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void				       *message;
    MSG_TYPE					messageType;
    unsigned long		        objectSize;
    unsigned short		        command;
    STORAGE_CTX					localCtx;
    MSG_C_STORE_RESP			* response;

    localCtx.callback = callback;
    localCtx.storeRequest = storeRequest;
    localCtx.storeResponse = NULL;
    localCtx.userCtx = callbackCtx;

    if (callback == NULL)
    	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CStoreRequest");
    if (storeRequest->type != MSG_K_C_STORE_REQ)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "STORE Request", "SRV_CStoreRequest");
    if (storeRequest->dataSetType == DCM_CMDDATANULL)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "STORE Request", "SRV_CStoreRequest");

    presentationCtx = SRVPRV_PresentationContext(params, storeRequest->classUID);
    if (presentationCtx == NULL)
    	return COND_PushCondition(SRV_NOSERVICEINASSOCIATION, SRV_Message(SRV_NOSERVICEINASSOCIATION), storeRequest->classUID, "SRV_CStoreRequest");

    cond = MSG_BuildCommand(storeRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "STORE Request", "SRV_CStoreRequest");

    dataSetObject = storeRequest->dataSet;
    if (dataSetObject == NULL) {
    	cond = DCM_OpenFile(storeRequest->fileName, DCM_ORDERLITTLEENDIAN, &dataSetObject);
    	if (cond != DCM_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");
    }

    cond = DCM_GetObjectSize(&dataSetObject, &objectSize);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");


    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");

    cond = callback(storeRequest, NULL, 0, objectSize, callbackCtx);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");

    cond = SRV_SendDataSet(association, presentationCtx, &dataSetObject, localCallback, &localCtx, objectSize);
    if (storeRequest->dataSet == NULL) (void) DCM_CloseObject(&dataSetObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");


    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CStoreRequest");

    if (messageType != MSG_K_C_STORE_RESP) {
    	(void) MSG_Free(&message);
    	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_CStoreRequest");
    }
    response = (MSG_C_STORE_RESP *) message;
    if (storeResponse != NULL) *storeResponse = *response;

    cond = callback(storeRequest, response, objectSize, objectSize, callbackCtx);
    (void) MSG_Free(&message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CStoreRequest");

    return SRV_NORMAL;
}

/* localCallback
**
** Purpose:
**	Local call back routine
**
** Parameter Dictionary:
**	bytesTransmitted	Number of bytes transmitted
**	totalBytes		Total number of bytes to be transmitted
**	storageCtx		Context specific information, here it is
**				a storage context.
**
** Return Values:
**	SRV_NORMAL on success
**	other conditions returned by the callback routine of the storage
**	context.
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
localCallback(unsigned long bytesTransmitted, unsigned long totalBytes, STORAGE_CTX * storageCtx)
{
    CONDITION		    cond = SRV_NORMAL;

    if (storageCtx->callback != NULL)
	cond = storageCtx->callback( storageCtx->storeRequest, storageCtx->storeResponse, bytesTransmitted, totalBytes, storageCtx->userCtx);

    return cond;
}


/* SRV_CStoreResponse
**
** Purpose:
**	SRV_CStoreResponse assists an application that wants to be an SCp of
**	one of the storage SOP classes.  When an application receives a
**	C-STORE REQ message, it calls this function with the request message
**	and other parameters.  This function opens the file name specified
**	by the caller and receives the data set from the network.
**
**	SRV_CStoreRequest estimates the size of the incoming data set
**	from the SOP Class in the Request message.  Based on this estimate,
**	SRV_CStoreRequest invokes the user callback function approximately
**	ten times.  (Since the size is only an estimate, the callback can
**	be invoked more or less than ten times).
**
**	Once the entire data set is received, the callback function is
**	invoked one final time.  At this last callbackback, the Store
**	Response structure will contain a DICOM Information Object which
**	was created by opening the data set that was just received.
**	The callback function should examine the Information Object.
**	In this last callback, the callback function should set status
**	values in the Response message.  After the final callback, this
**	function creates a C-STORE Response message and sends it to
**	the requesting application.
**
**	The arguments to the callback function are:
**		MSG_C_STORE_REQ	*storeRequest
**		MSG_C_STORE_RESP *storeResponse
**		unsigned long	bytesReceived
**		unsigned long	totalBytes
**		DCM_OBJECT	**object
**		void		*callbackCtx
**	where
**		storeRequest	Pointer to MSG structure with C-STORE request.
**		storeRespponse	Pointer to MSG structure with C-STORE response.
**		bytesReceived	Number of bytes received so far.
**		totalBytes	Estimated number of bytes in data set.
**		object		DICOM Information Object that was
**				received.
**		callbackCtx	User's callbackCtx argument which is used to
**				maintain context information in the callback
**				function.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	association	The key for the Association on which the STORE request
**			was received and will be used to transmit the STORE
**			response.
**	ctx		Pointer to presentation context for this STORE request.
**	storeRequest	Pointer to the structure which contains the STORE
**			request received by the application.
**	storeResponse	Pointer to structure in caller's space used to hold the
**			STORE response message.
**	callback	Address of callback routine which is invoked during
**			the storage process.
**	callbackCtx	Pointer to any context information required by the
**			caller's callback function.
**      dirName         Name of directory where files may be created.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_OBJECTBUILDFAILED
**	SRV_RESPONSEFAILED
**	SRV_UNEXPECTEDPDVTYPE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_CStoreResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_STORE_REQ ** storeRequest, MSG_C_STORE_RESP * storeReply,
				   char *fileName, SRV_C_STORE_RESP_CALLBACK * callback, void *callbackCtx, char *dirName)
{
    int		        	fd, done;
    unsigned long		total, estimatedSize, nextCallback;
    unsigned short      sendStatus = 0x0000;
    DUL_PDV				pdv;
    CONDITION			cond;
    DCM_OBJECT			* object;
    DCM_FILE_META 		fileMeta;
    int 				storeAsPart10 = 0;
    char* 				paramValue;

    storeReply->messageIDRespondedTo = (*storeRequest)->messageID;
    storeReply->type = MSG_K_C_STORE_RESP;
    (void) strcpy(storeReply->classUID, (*storeRequest)->classUID);
    (void) strcpy(storeReply->instanceUID, (*storeRequest)->instanceUID);
    storeReply->conditionalFields = MSG_K_C_STORERESP_CLASSUID | MSG_K_C_STORERESP_INSTANCEUID;
    storeReply->dataSetType = DCM_CMDDATANULL;

    if (strlen(fileName) == 0 || fileName == NULL) {
    	fd = -1;
    }else{
#ifdef _MSC_VER
    	fd = _open(fileName, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, _S_IREAD | _S_IWRITE);
#else
    	fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
#endif
    	if (fd < 0) (void) COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), fileName, "SRV_CStoreResponse");
    }
    if (strcmp(ctx->acceptedTransferSyntax, DICOM_TRANSFERLITTLEENDIAN) != 0){
    	storeAsPart10 = 1;
    }else{
    	paramValue = UTL_GetConfigParameter("STORAGE/PART10FLAG");
    	if (paramValue != NULL) {
    		if (strcmp(paramValue, "1") == 0) storeAsPart10 = 1;
    	}
    }
    if ((storeAsPart10 == 1) && (fd > 0)){
    	int localStatus = 0;
    	memset(&fileMeta, 0, sizeof(fileMeta));
    	setFileMeta(&fileMeta, *storeRequest, ctx->acceptedTransferSyntax);
    	localStatus = writePart10MetaHeader(fd, &fileMeta);
    	if (localStatus != 0) fd = -1;
    }
    if (fd < 0) {
    	sendStatus = MSG_K_C_STORE_OUTOFRESOURCES;
    	storeReply->conditionalFields |= MSG_K_C_STORERESP_ERRORCOMMENT;
    	strcpy(storeReply->errorComment, "Storage Service unable to create local file");
    	fprintf(stderr,"Storage Service unable to create local file\n");
    }
    done = 0;
    total = 0;
    estimatedSize = genericImageSize((*storeRequest)->classUID);
    nextCallback = estimatedSize / 10;

    while (!done) {
    	cond = SRVPRV_ReadNextPDV(association, DUL_BLOCK, 0, &pdv);
    	if (cond != SRV_NORMAL) {
    		(void) MSG_Free((void **) storeRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CStoreResponse");
    	}

    	if (pdv.pdvType != DUL_DATASETPDV) {
    		(void) MSG_Free((void **) storeRequest);
    		return COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE), (int) pdv.pdvType, "SRV_CStoreResponse");
    	}

    	if (sendStatus == 0) if (write(fd, pdv.data, pdv.fragmentLength) != (int) pdv.fragmentLength) sendStatus = MSG_K_C_STORE_OUTOFRESOURCES;

    	total += pdv.fragmentLength;
    	if (pdv.lastPDV) done++;

    	if (total > nextCallback){
    		cond = callback(*storeRequest, NULL, total, estimatedSize, NULL, callbackCtx, ctx);
    		if (cond != SRV_NORMAL){
    			(void) MSG_Free((void **) storeRequest);
    			return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CStoreResponse");
    		}
    		nextCallback += estimatedSize / 10;
    	}
    }
    if (fd >= 0) (void) close(fd);

    storeReply->status = sendStatus;

    if (sendStatus == 0x0000) {
    	unsigned long options = DCM_ORDERLITTLEENDIAN;
    	if (storeAsPart10 == 1) options = DCM_PART10FILE;

    	cond = DCM_OpenFile(fileName, options, &object);
    	if (cond != DCM_NORMAL) {
    		(void) MSG_Free((void **) storeRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CStoreResponse");
    	}

    	cond = callback(*storeRequest, storeReply, total, estimatedSize, &object, callbackCtx, ctx);
    	if (cond != SRV_NORMAL) {
    		(void) DCM_CloseObject(&object);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CStoreResponse");
    	}
    	(void) DCM_CloseObject(&object);
    }
    (void) MSG_Free((void **) storeRequest);

    cond = MSG_BuildCommand(storeReply, &object);
    if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "STORE Request", "SRV_CStoreResponse");

    cond = SRV_SendCommand(association, ctx, &object);
    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CStoreResponse");

    return SRV_NORMAL;
}


/* genericImageSize
**
** Purpose:
**	return the size of the image if found in the table else return
**	a default value.
**
** Parameter Dictionary:
**	SOPClass	SOP Class ID of the image whose size is to be returned
**
** Return Values:
**	size of image
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

typedef struct {
    char 			*classUID;
    unsigned long 	imageSize;
}   CLASS_SIZE_MAP;

static unsigned long
genericImageSize(char *SOPClass)
{
    static CLASS_SIZE_MAP size_map[] = {
    		{DICOM_SOPCLASSCOMPUTEDRADIOGRAPHY, 2 * 1024 * 1024},
    		{DICOM_SOPCLASSCT, 2 * 512 * 512},
    		{DICOM_SOPCLASSMR, 2 * 256 * 256},
    		{DICOM_SOPCLASSNM, 2 * 64 * 64},
    		{DICOM_SOPCLASSUS1993, 1 * 512 * 512},
    		{DICOM_SOPCLASSUS, 1 * 512 * 512},
    		{DICOM_SOPCLASSUSMULTIFRAMEIMAGE1993, 30 * 256 * 256},
    		{DICOM_SOPCLASSUSMULTIFRAMEIMAGE, 30 * 256 * 256},
    		{DICOM_SOPCLASSXRAYANGIO, 2 * 1024 * 1024},
    		{DICOM_SOPCLASSXRAYFLUORO, 2 * 1024 * 1024},
    };
    int        index;

    for (index = 0; index < (int) DIM_OF(size_map); index++){
    	if (strcmp(SOPClass, size_map[index].classUID) == 0) return size_map[index].imageSize;
    }
    return 512 * 512;
}
