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
** Module Name(s):	SRV_CGetRequest
**			SRV_CGetResponse
** Author, Date:	Aniruddha S. Gokhale, 05/23/94
** Intent:		This module contains routines which implement the
**			GET service class (user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:07 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/services/get.c,v $
** Revision:		$Revision: 1.34 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.34 $ $RCSfile: get.c,v $";

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
#include <sys/stat.h>
#else
#include <sys/file.h>
#endif

#ifdef _MSC_VER
#include <fcntl.h>
#endif
#ifdef SOLARIS
#include <fcntl.h>
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

extern CTNBOOLEAN PRVSRV_debug;

static CONDITION
processGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, char *dirName, MSG_C_GET_REQ * getRequest,
				   MSG_C_GET_RESP * localResponse, char *queryLevelString, CONDITION(*callback) (), void *callbackCtx, CTNBOOLEAN * done,
				   CTNBOOLEAN * cancelled, MSG_C_GET_RESP * getResponse, unsigned long *responseCount);
static CONDITION
processStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_STORE_REQ * storeRequest, CONDITION(*callback) (), void *callbackCtx);
static CONDITION
sendStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_STORE_REQ * storeRequest);
static unsigned long
genericImageSize(char *SOPClass);

/* SRV_CGetRequest
**
** Purpose:
**	SRV_CGetRequest assists an application that wants to be an SCU of
**	the Query/Retrieve SOP class (GET).  This function constructs a
**	C-GET-REQ Message and sends it to the peer application which is
**	acting as the SCP for the Query/Retrive SOP class.  This function
**	waits for two kinds of messages viz. (1) the CGET responses from the
**	peer application and invokes the caller's getcallback function one
**	time for each response.  These responses are a number of "pending"
**	responses followed by one "final" response. (2) the CStore requests
**	for which this application is acting as an SCP. The store requests are
**	for those images for which the CGET query succeeded. The facility
**	will actually write to a file provided by the callback. The store
**	callback routine will be called for the first time at which the
**	application should provide a file name (absolute/relative path) into
**	which the image is to be stored. Thereafter, the same store callback
**	will be invoked for every partially received image. The callback will
**	be invoked one last time when the entire image has been received and
**	will be passed a handle to a DICOM object representing the image
**	received. The callback can then decide to operate on the object and
**	return various status codes for the store response.
**
**	The arguments to the  get callback function are:
**		MSG_C_GET_REQ	*getRequest
**		MSG_C_GET_RESP	*getResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		getRequest	Pointer to MSG structure with C-GET request.
**		getResponse	Pointer to MSG structure with C-GET response.
**		responseCount	Number of times callback function has been
**				called for this query (starts at 1).
**		abstractSyntax	A character string which identifies the
**				abstract syntax of the SOP Class of the query.
**		queryLevelString A character string which identifies one of
**				the four levels in the hierarchical query
**				model.
**		callbackCtx	User's callbackCtx argument which is used to
**				maintain context information in the callback
**				function.
**
**	On each invocation of the callback function, the user should examine
**	the contents of the status field.
**	The final response indicates completion of the CGet request and the
**	status will indicate whether the request succeeded.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**
**	The arguments to the store callback are:
**		MSG_C_STORE_REQ		* storeRequest
**		MSG_C_STORE_RESP 	* storeResponse
**		CTNBOOLEAN			first
**		char			* fileName
**		unsigned long		total
**		unsigned long		estimatedSize
**		DCM_OBJECT		** object
**	where
**		storeRequest	Pointer to the CStore Request message
**		storeResponse	Pointer to the CStore Response message
**		first		For the first invocation of the callback,
**				this will be TRUE and hence the application
**				will provide a file name. Thereafter it will
**				be FALSE
**		fileName	name of the file in which the image is to be
**				stored.
**		total		Total number of bytes received so far.
**		estimatedSize	Estimated size of the image
**		object		Handle to the object represting the image.
**				This parameter will be null except in the
**				last invocation of the callback.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association.  This
**			list includes the list of presentation contexts.
**	getRequest	Pointer to structure in caller's memory which contains
**			the GET request.
**	getResponse	Address of pointer to response message. This function
**			will create a response message and return the
**			address of the structure to the caller.
**	getCallback	Address of user routine which is called one time for
**			each CGet response received from the network.
**	getCallbackCtx	User context information which is supplied during call
**			to getCallback function.
**	storageCallback	Address of user routine to be invoked for every
**			store request received from the network
**	storageCallbackCtx
**			Any context information to be supplied to the storage
**			callback routine.
**      dirName         Name of directory where files may be created.
**
** Return Values:
**
**	SRV_ILLEGALPARAMETER
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_NOSERVICEINASSOCIATION
**	SRV_OBJECTACCESSFAILED
**	SRV_OBJECTBUILDFAILED
**	SRV_OPERATIONCANCELLED
**	SRV_REQUESTFAILED
**	SRV_UNEXPECTEDCOMMAND
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_CGetRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,	MSG_C_GET_REQ * getRequest, MSG_C_GET_RESP * getResponse,
				SRV_C_GET_REQ_CALLBACK * getCallback, void *getCallbackCtx, CONDITION(*storageCallback) (), void *storageCallbackCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;			/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx,	/* Presentation context for GET service */	*storePresentationCtx;	/* for the arriving store request */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void      					*message;
    MSG_TYPE					messageType;
    CTNBOOLEAN					done = FALSE;
    U32							l;
    unsigned long        		responseCount = 0;
    MSG_C_STORE_REQ				* storeRequest;
    void       					*ctx;
    char        				queryLevelString[48] = "";	/* Initialization for AIX compiler */
    DCM_ELEMENT					queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};
    unsigned short		        command;
    CTNBOOLEAN					cancelled = FALSE;
    MSG_C_CANCEL_REQ			cancelRequest = {MSG_K_C_CANCEL_REQ, 0, 0, DCM_CMDDATANULL};

    queryLevelElement.d.string = queryLevelString;

    if ((getCallback == NULL) || (storageCallback == NULL))	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CGetRequest");
    if (getRequest->type != MSG_K_C_GET_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "GET Request", "SRV_CGetRequest");
    if (getRequest->dataSetType == DCM_CMDDATANULL)	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "GET Request", "SRV_CGetRequest");

    ctx = NULL;
    cond = DCM_GetElementValue(&getRequest->identifier, &queryLevelElement, &l, &ctx);
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_OBJECTACCESSFAILED, SRV_Message(SRV_OBJECTACCESSFAILED), "Query Identifier", "SRV_CGetRequest");

    queryLevelString[l] = '\0';
    if (queryLevelString[l - 1] == ' ')	queryLevelString[l - 1] = '\0';

    presentationCtx = SRVPRV_PresentationContext(params, getRequest->classUID);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_NOSERVICEINASSOCIATION, SRV_Message(SRV_NOSERVICEINASSOCIATION), getRequest->classUID, "SRV_CGetRequest");

    cond = MSG_BuildCommand(getRequest, &commandObject);
    if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "GET Request", "SRV_CGetRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");

    cond = SRV_SendDataSet(association, presentationCtx, &getRequest->identifier, NULL, NULL, 0);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");

    while (!done) {		/* do until there are no more pending requests */
    	cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");

    	switch (messageType) {
			case MSG_K_C_GET_RESP:
											cond = processGetResponse(association, presentationCtx, dirName, getRequest, (MSG_C_GET_RESP *) message,
																	  queryLevelString, getCallback, getCallbackCtx, &done, &cancelled, getResponse, &responseCount);
											(void) MSG_Free(&message);
											if ((CTN_FATAL(cond)) || (CTN_ERROR(cond)))	return cond;
											break;
			case MSG_K_C_STORE_REQ:
											storeRequest = (MSG_C_STORE_REQ *) message;
											storePresentationCtx = SRVPRV_PresentationContext(params, storeRequest->classUID);
											if (storePresentationCtx == NULL)
												return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_CGetRequest");

											cond = processStoreRequest(association, storePresentationCtx, storeRequest, storageCallback, storageCallbackCtx);
											if ((CTN_FATAL(cond)) || (CTN_ERROR(cond)))	return cond;
											break;
			default:
											(void) MSG_Free(&message);
											return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_CGetRequest");
    	}
    }
    if (cancelled){
    	return SRV_OPERATIONCANCELLED;
    }else{
    	return SRV_NORMAL;
    }
}

/* SRV_CGetResponse
**
** Purpose:
**	SRV_CGetResponse is used by an application which is acting as an
**	SCP of the Query/Retrieve SOP class (GET).  When an application
**	receives a C-GET Request message, it calls this function with the
**	C-GET request and other parameters.  SRV_CGetRequest checks the
**	caller's parameters and polls the network, waiting for an identifier
**	which contains the query used for the GET.
**
**	Once SRV_CGetResponse has read the identifier from the network,
**	the user's callback routine is invoked with the following parameters:
**		MSG_C_GET_REQ	*getRequest
**		MSG_C_GET_RESP	*getResponse
**		MSG_C_STORE_REQ	*storeRequest
**		MSG_C_STORE_RESP*storeResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		getRequest	Pointer to MSG structure with C-GET request.
**		getRespponse	Pointer to MSG structure with C-GET response.
**		storeRequest	Pointer to MSG structure with C-STORE request
**		storeResponse	Pointer to MSG structure with C_STORE response
**		responseCount	Number of times callback function has been
**				called for this query (starts at 1).
**		abstractSyntax	A character string which identifies the
**				abstract syntax of the SOP Class of the query.
**		queryLevelString A character string which identifies one of
**				the four levels in the hierarchical query
**				model.
**		callbackCtx	User's callbackCtx argument which is used to
**				maintain context information in the callback
**				function.
**
**	If responseCount is 0, the callback function initiates a new
**	database search.  This query should identify the entire set of
**	images that are to be sent to the peer.  From the callback
**	routine, the user should supply this information in the store
**	request parameter.
**
**	The facility will then attempt to send this "store" request back to
**	the CGET requestor. Thereafter, as every store response is received,
**	the user callback is called. The user callback should continue to supply
**	the next image to be stored in the store request. Also, depending
**	on the status of the received store response, it should update
**	information about the number of successful, warning and failed
**	operations and may decide to send "pending" CGET responses.
**	If there are no more images left to be stored, the callback must
**	generate a "final" CGET response message and update all necessary
**	fields and send it back to the SCU.
**
**	The user indicates the get is complete by placing the
**	appropriate status value in the status field of the response message.
**	The callback function should always return SRV_NORMAL.  Any
**	other value will cause SRV_CGetRequest to abort the Association.
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	association	The key for the Association on which the GET request
**			was received and will be used to transmit the GET
**			response.
**	params		Handle to service parameters
**	presentationCtx	Pointer to presentation context for this GET request.
**	getRequest	Pointer to the structure which contains the GET
**			request received by the application.
**	getResponse	Pointer to structure in caller's space used to hold the
**			GET response message.
**	callback	Address of callback routine which is used to invoke
**			database query and provide subsequent database
**			retrievals.
**	callbackCtx	Pointer to any context information required by the
**			caller's callback function.
**      dirName         Name of directory where files may be created.
**
** Return Values:
**
**	SRV_ILLEGALPARAMETER
**	SRV_LISTCREATEFAILURE
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_OPERATIONCANCELLED
**	SRV_RESPONSEFAILED
**	SRV_SUSPICIOUSRESPONSE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_CGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXT * getPresentationCtx,
				 MSG_C_GET_REQ ** getRequest, MSG_C_GET_RESP * getResponse, SRV_C_GET_RESP_CALLBACK * getCallback, void *getCtx, char *dirName)
{
    int      		flag, responseCount = 0;
    U32				l;
    char        	queryLevelString[48] = "";	/* Initialization for AIX compiler */
    CONDITION		cond, rtnCond = SRV_NORMAL;
    DCM_OBJECT		* responseObject;	/* get response object */
    void       		*ctx;
    DCM_ELEMENT		queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};
    static char 	*allowedQueryLevels[] = {
							DCM_QUERYLEVELPATIENT,
							DCM_QUERYLEVELSTUDY,
							DCM_QUERYLEVELSERIES,
							DCM_QUERYLEVELIMAGE};
    MSG_STATUS_DESCRIPTION	statusDescription;
    char pendingMsg[] = "\
In SRV_CGetResponse, the response message returned by your callback has \n\
a status of pending and data set that is not null.\n";
    MSG_TYPE		messageType;
    void	       *message;
    DUL_PRESENTATIONCONTEXTID	ctxID;
    MSG_C_STORE_REQ				storeRequest;
    char				        classUID[DICOM_UI_LENGTH + 1];

    queryLevelElement.d.string = queryLevelString;

    if (getCallback == NULL) {
    	(void) MSG_Free((void **) getRequest);
    	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CGetResponse");
    }
    if (getResponse->type != MSG_K_C_GET_RESP) {
    	(void) MSG_Free((void **) getRequest);
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "GET Request", "SRV_CGetResponse");
    }
    (void) strcpy(classUID, (*getRequest)->classUID);

    cond = DCM_CreateObject(&getResponse->identifier, 0);
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");

    cond = SRV_ReceiveDataSet(association, getPresentationCtx, DUL_BLOCK, 0, dirName, &(*getRequest)->identifier);
    if (PRVSRV_debug && (cond == SRV_NORMAL)) (void) DCM_DumpElements(&(*getRequest)->identifier, 0);

    if (cond != SRV_NORMAL) {
    	(void) MSG_Free((void **) getRequest);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    }
    ctx = NULL;

    cond = DCM_GetElementValue(&(*getRequest)->identifier, &queryLevelElement, &l, &ctx);
    if (cond != DCM_NORMAL) {
    	(void) MSG_Free((void **) getRequest);
    	(void) COND_PushCondition(SRV_QUERYLEVELATTRIBUTEMISSING, SRV_Message(SRV_QUERYLEVELATTRIBUTEMISSING), "SRV_CGetResponse");
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    }
    queryLevelString[l] = '\0';
    if (queryLevelString[l - 1] == ' ')	queryLevelString[l - 1] = '\0';

    for (flag = 0, l = 0; l < DIM_OF(allowedQueryLevels) && !flag; l++){
    	if (strcmp(queryLevelString, allowedQueryLevels[l]) == 0) flag = 1;
    }
    if (!flag) {
    	(void) MSG_Free((void **) getRequest);
    	(void) COND_PushCondition(SRV_ILLEGALQUERYLEVELATTRIBUTE, SRV_Message(SRV_ILLEGALQUERYLEVELATTRIBUTE), queryLevelString, "SRV_CGetResponse");
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    }
    getResponse->messageIDRespondedTo = (*getRequest)->messageID;
    getResponse->conditionalFields = 0;
    getResponse->remainingSubOperations = 0;
    getResponse->completedSubOperations = 0;
    getResponse->failedSubOperations = 0;
    getResponse->warningSubOperations = 0;

    /*
     * Fill up the Store request and let the callback routine do the matching
     * and fill the rest of the fields
     */
    storeRequest.type = MSG_K_C_STORE_REQ;
    storeRequest.conditionalFields = 0;
    storeRequest.messageID = SRV_MessageIDOut();
    storeRequest.priority = 0;
    storeRequest.moveMessageID = (*getRequest)->messageID;

    /*
     * We invoke the callback for the first time without any StoreResponse so
     * that the callback can deal with the query and fill the Store Request
     * message and the GET response message. We also pass responseCount as 0
     * when we invoke the callback for the first time
     */
    cond = getCallback(*getRequest, getResponse, &storeRequest, NULL, 0, getPresentationCtx->abstractSyntax, queryLevelString, getCtx);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CGetRespose");

    if (storeRequest.dataSetType != DCM_CMDDATANULL) {
    	/* Send a Store Request */
    	cond = sendStoreRequest(association, params, &storeRequest);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED),"sendStoreRequest", "SRV_CGetResponse");
	}else{
		/*
		 * No images are to be transferred as the query may have been
		 * unsuccessful in the call back routine. We expect the callback
		 * routine to have supplied the appropriate values for the various
		 * fields of the getResponse message.
		 *
		 * Send the CGetResponse Message and return from the routine
		 */
		cond = MSG_BuildCommand(getResponse, &responseObject);
		if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "getResponse", "SRV_CGetResponse");

		cond = SRV_SendCommand(association, getPresentationCtx, &responseObject);
		if (cond != SRV_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED),"SRV_SendCommand", "SRV_CGetResponse");

		if (getResponse->dataSetType != DCM_CMDDATANULL) {
			cond = SRV_SendDataSet(association, getPresentationCtx, &getResponse->identifier, NULL, NULL, 0);
			if (cond != SRV_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "SRV_SendDataSet", "SRV_CGetResponse");
		}
		return SRV_NORMAL;
    }
    flag = 0;
    while (!flag) {
    	/*
    	 * Now wait for incoming Store Responses or a Cancel Request. If it
    	 * is a Store Response, invoke the callback routine, with the Store
    	 * Response. The callback will update the Get Response message
    	 * accordingly and also fill the Store Request message if any more
    	 * requests are to be made
    	 */
    	cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, NULL, &messageType, (void **) &message);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_CGetResponse");

    	switch (messageType) {
			case MSG_K_C_CANCEL_REQ:
											rtnCond = SRV_OPERATIONCANCELLED;
											getResponse->status = MSG_K_CANCEL;
											(void) MSG_Free(&message);

											cond = getCallback(*getRequest, getResponse, NULL, NULL, responseCount, getPresentationCtx->abstractSyntax, queryLevelString, getCtx);
											if (cond != SRV_NORMAL) return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CGetRespose");

											getResponse->messageIDRespondedTo = (*getRequest)->messageID;
											break;
			case MSG_K_C_STORE_RESP:
											responseCount++;
											getResponse->dataSetType = DCM_CMDDATANULL;
											getResponse->status = 0xffff;

											/*
											 * Fill up the Store request and let the callback routine do the
											 * matching and fill the rest of the fields
											 */
											storeRequest.type = MSG_K_C_STORE_REQ;
											storeRequest.conditionalFields = 0;
											storeRequest.messageID = SRV_MessageIDOut();
											storeRequest.priority = 0;
											storeRequest.moveMessageID = (*getRequest)->messageID;
											cond = getCallback(*getRequest, getResponse, &storeRequest, (MSG_C_STORE_RESP *) message, responseCount,
															   getPresentationCtx->abstractSyntax, queryLevelString, getCtx);
											if (cond != SRV_NORMAL)
												return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CGetRespose");

											if (storeRequest.dataSetType != DCM_CMDDATANULL) {
												/* make the next store request */
												cond = sendStoreRequest(association, params, &storeRequest);
												if (cond != SRV_NORMAL)
													return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "sendStoreRequest", "SRV_CGetResponse");
											}
											break;
			default:
											break;
    	}

    	if (cond == SRV_NORMAL) {
    		cond = MSG_StatusLookup(getResponse->status, MSG_K_C_GET_RESP, &statusDescription);
    		if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");

    		if (statusDescription.statusClass != MSG_K_CLASS_PENDING) flag = 1;

    		/* Consistency check */
    		if ((statusDescription.statusClass == MSG_K_CLASS_PENDING) && (getResponse->dataSetType != DCM_CMDDATANULL)) {

    			if (PRVSRV_debug) fprintf(DEBUG_DEVICE, pendingMsg);
    			rtnCond = COND_PushCondition(SRV_SUSPICIOUSRESPONSE, SRV_Message(SRV_SUSPICIOUSRESPONSE), "C-GET", "pending", "not null", "SRV_CGetResponse");
    		}
    	}else{
    		flag = 1;
    	}

    	strcpy(getResponse->classUID, classUID);
    	getResponse->conditionalFields |= MSG_K_C_GETRESP_CLASSUID;

    	cond = MSG_BuildCommand(getResponse, &responseObject);
    	if (cond != MSG_NORMAL) {
    		(void) MSG_Free((void **) getRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    	}

    	cond = SRV_SendCommand(association, getPresentationCtx, &responseObject);
    	if (cond != SRV_NORMAL) {
    		(void) MSG_Free((void **) getRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    	}

    	if (getResponse->dataSetType != DCM_CMDDATANULL) {
    		cond = SRV_SendDataSet(association, getPresentationCtx, &getResponse->identifier, NULL, NULL, 0);
    		if (cond != SRV_NORMAL) {
    			(void) MSG_Free((void **) getRequest);
    			return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetResponse");
    		}
    	}
    }
    (void) MSG_Free((void **) getRequest);
    return rtnCond;
}

/* private routines */

/* processGetResponse
**
** Purpose:
**	This function processes the incoming response to the Get request.
**
** Parameter Dictionary:
**	association	Handle to the association
**	presentationCtx	Handle to the presentation context negotiated between
**			the two communicating peers
**	dirName		path name of file where we wish to receive the data
**			set.
**	getRequest	Pointer to the C-GET request structure
**	localResponse	Pointer to the C-GET response structure
**	queryLevelString
**			A string specifying the query at a specified level
**	callback	Pointer to user callback routine that is to be invoked
**			after a response is received
**	callbackCtx	Any user context information that needs to be passed
**			to the callback routine
**	done		A boolean flag indicating whether there are any more
**			pending store requests
**	cancelled	A boolean flag indicating whether a Cancel request had
**			been made
**	getResponse	Pointer to the GetResponse message received
**
** Return Values:
**	SRV_NORMAL unless there is any error
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
processGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, char *dirName, MSG_C_GET_REQ * getRequest, MSG_C_GET_RESP * localResponse,
				   char *queryLevelString, CONDITION(*callback) (), void *callbackCtx, CTNBOOLEAN * done, CTNBOOLEAN * cancelled, MSG_C_GET_RESP * getResponse, unsigned long *responseCount)
{
    CONDITION    				cond;
    MSG_STATUS_DESCRIPTION		statusDescription;
    DCM_OBJECT					* commandObject;
    MSG_C_CANCEL_REQ			cancelRequest = {MSG_K_C_CANCEL_REQ, 0, 0, DCM_CMDDATANULL};

    cond = MSG_StatusLookup(localResponse->status, MSG_K_C_GET_RESP, &statusDescription);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");

    /* find the general category of the status received */
    if (statusDescription.statusClass != MSG_K_CLASS_PENDING) *done = TRUE;		/* No more pending requests */

    if (localResponse->dataSetType != DCM_CMDDATANULL) { /* Some data set (identifier) has been sent back */
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->identifier);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");
    }
    *responseCount++;

    if (!*cancelled) {		/* no cancel request made */
    	cond = callback(getRequest, localResponse, *responseCount, presentationCtx->abstractSyntax, queryLevelString, callbackCtx);
    	if (cond == SRV_OPERATIONCANCELLED) {
    		*cancelled = TRUE;
    		/* build a cancel request */
    		cancelRequest.messageIDRespondedTo = getRequest->messageID;

    		cond = MSG_BuildCommand(&cancelRequest, &commandObject);
    		if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "CANCEL Request", "SRV_CGetRequest");

    		cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    		(void) DCM_CloseObject(&commandObject);
    		if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CGetRequest");
    	}else if (cond != SRV_NORMAL){
    		*done = TRUE;	/* callback did not return SRV_NORMAL. Hence we opt to quit the loop */
    	}
    }
    if (getResponse != NULL) {
    	*getResponse = *localResponse;
    	getResponse->identifier = NULL;
    }
    return SRV_NORMAL;
}

/* processStoreRequest
**
** Purpose:
**	Process the Store sub operation request.
**
** Parameter Dictionary:
**	association	Handle to the association
**	presentationCtx	The presentation context negotiated between the peers
**	storeRequest	Pointer to the received C-STORE request
**	callback	Pointer to user callback function to be invoked during
**			store operation
**	callbackCtx	Any context information that needs to be passed to the
**			callback function
**
** Return Values:
**	SRV_NORMAL unless there is any error
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
processStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_STORE_REQ * storeRequest, CONDITION(*callback) (), void *callbackCtx)
{
    CONDITION   		cond;
    int        			fd;			/* file descriptor */
    CTNBOOLEAN			first = TRUE,		/* to tell the callback function if it was invoked the first time or not */	done = FALSE;
    unsigned long       total, estimatedSize, nextCallback;
    unsigned short      sendStatus = 0x0000;
    DUL_PDV				pdv;
    DCM_OBJECT			* object;
    char        		fileName[1024];		/* name of image file */
    MSG_C_STORE_RESP	storeResponse;

    storeResponse.messageIDRespondedTo = storeRequest->messageID;
    storeResponse.type = MSG_K_C_STORE_RESP;
    (void) strcpy(storeResponse.classUID, storeRequest->classUID);
    (void) strcpy(storeResponse.instanceUID, storeRequest->instanceUID);
    storeResponse.conditionalFields = MSG_K_C_STORERESP_CLASSUID | MSG_K_C_STORERESP_INSTANCEUID;
    storeResponse.dataSetType = DCM_CMDDATANULL;
    storeResponse.status = MSG_K_SUCCESS;	/* we initialize it to success but the callback can change it */

    /* invoke the call back for the first time and get the file name */
    total = 0;
    estimatedSize = genericImageSize(storeRequest->classUID);
    cond = callback(storeRequest, &storeResponse, first, fileName, total, estimatedSize, NULL, callbackCtx);
    first = FALSE;
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "processStoreRequest");

#ifdef _MSC_VER
    fd = open(fileName, O_BINARY | O_CREAT | O_WRONLY | O_TRUNC, S_IREAD | S_IWRITE);
#else
    fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
#endif
    if (fd < 0) return COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), fileName, "SRV_CGetRequest");

    done = FALSE;
    nextCallback = estimatedSize / 10;
    while (!done) {
    	cond = SRVPRV_ReadNextPDV(association, DUL_BLOCK, 0, &pdv);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetRequest");

    	if (pdv.pdvType != DUL_DATASETPDV) return COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE), (int) pdv.pdvType, "SRV_CGetRequest");

    	if (sendStatus == 0) {
    		if (write(fd, pdv.data, pdv.fragmentLength) !=	(int) pdv.fragmentLength) sendStatus = 0xaf01;
    	}
    	total += pdv.fragmentLength;

    	if (pdv.lastPDV) done = TRUE;

    	if (total > nextCallback) {
    		cond = callback(storeRequest, &storeResponse, first, NULL, total, estimatedSize, NULL, callbackCtx);
    		if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetRequest");
	        nextCallback += estimatedSize / 10;
    	}
    }
    (void) close(fd);

    cond = DCM_OpenFile(fileName, DCM_ORDERLITTLEENDIAN, &object);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetRequest");

    /* invoke the callback the last time with a valid object */
    cond = callback(storeRequest, &storeResponse, first, NULL, total, estimatedSize, &object, callbackCtx);
    if (cond != SRV_NORMAL) {
    	(void) DCM_CloseObject(&object);
    	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CGetRequest");
    }

    (void) DCM_CloseObject(&object);

    /* Now build a C-Store response message and send it to the peer */
    cond = MSG_BuildCommand(&storeResponse, &object);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "storeResponse", "processStoreRequest");

    cond = SRV_SendCommand(association, presentationCtx, &object);
    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CGetRequest");
    return SRV_NORMAL;
}

typedef struct {
    char 			*classUID;
    unsigned long 	imageSize;
}   CLASS_SIZE_MAP;

static unsigned long
genericImageSize(char *SOPClass)
{
    static CLASS_SIZE_MAP size_map[] = {
    		{DICOM_SOPCLASSCOMPUTEDRADIOGRAPHY, 2 * 1024 * 1024},
    		{DICOM_SOPCLASSDIGXRAYPRESENTATION, 2 * 2048 * 2048},
    		{DICOM_SOPCLASSCT, 2 * 512 * 512},
    		{DICOM_SOPCLASSMR, 2 * 256 * 256},
    		{DICOM_SOPCLASSNM, 2 * 64 * 64},
    		{DICOM_SOPCLASSPET, 2 * 128 * 128},
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

static CONDITION
sendStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_STORE_REQ * storeRequest)
{
    DUL_PRESENTATIONCONTEXT    	* storePresentationCtx;
    DCM_OBJECT					* storeRequestObject;
    CONDITION					cond;

    /* switch the presentation context to the store presentation context */
    storePresentationCtx = SRVPRV_PresentationContext(params, storeRequest->classUID);
    if (!storePresentationCtx) return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "sendStoreRequest");

    /* build a StoreRequest command object */
    cond = MSG_BuildCommand(storeRequest, &storeRequestObject);
    if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "MSG_BuildCommand", "sendStoreRequest");

    /* Now send the store request on the same association */
    cond = SRV_SendCommand(association, storePresentationCtx, &storeRequestObject);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "SRV_SendCommand", "sendStoreRequest");

    /* Now send the data set */
    cond = SRV_SendDataSet(association, storePresentationCtx, &storeRequest->dataSet, NULL, NULL, 0);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "SRV_SendDataSet", "sendStoreRequest");

    return SRV_NORMAL;
}
