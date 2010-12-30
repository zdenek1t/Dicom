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
** Module Name(s):	SRV_CMoveRequest
**			SRV_CMoveResponse
**	private modules
**			clearUIDList
**			createUIDList
** Author, Date:	Stephen M. Moore, 2-Jun-93
** Intent:		This module contains routines which implement the
**			MOVE service class (user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:07 $
** Source File:		$RCSfile: move.c,v $
** Revision:		$Revision: 1.33 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.33 $ $RCSfile: move.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/file.h>
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
#if STANDARD_VERSION < VERSION_JUL1993
static void
    clearUIDList(LST_HEAD ** l);
static CONDITION
    createUIDList(LST_HEAD ** l);
#endif

/* SRV_CMoveRequest
**
** Purpose:
**	SRV_CMoveRequest assists an application that wants to be an SCU of
**	the Query/Retrieve SOP class (MOVE).  This function constructs a
**	C-MOVE-REQ Message and sends it to the peer application which is
**	acting as the SCP for the Query/Retrive SOP class.  This function
**	waits for the responses from the peer application and invokes the
**	caller's callback function one time for each response.  These responses
**	are a number of "pending" responses followed by one "final"
**	response.
**
**	The arguments to the callback function are:
**		MSG_C_MOVE_REQ	*moveRequest
**		MSG_C_MOVE_RESP	*moveResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		moveRequest	Pointer to MSG structure with C-MOVE request.
**		moveRespponse	Pointer to MSG structure with C-MOVE response.
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
**	the contents of the status field.  This will tell indicate if
**	the response message is a "pending" response or a "final" response.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association.  This
**			list includes the list of presentation contexts.
**	moveRequest	Pointer to structure in caller's memory which contains
**			the MOVE request.
**	moveResponse	Address of pointer to response message. This function
**			will create a response message and return the
**			address of the structure to the caller.
**	callback	Address of user routine which is called one time for
**			each response received from the network.
**	callbackCtx	User context information which is supplied during call
**			to callback function.
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
SRV_CMoveRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,  MSG_C_MOVE_REQ * moveRequest, MSG_C_MOVE_RESP * moveResponse,
				 SRV_C_MOVE_REQ_CALLBACK * callback, void *callbackCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;			/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void       					*message;
    MSG_TYPE					messageType;
    int        					done = 0;
    U32							l;
    unsigned long        		responseCount = 0;
    MSG_C_MOVE_RESP				* localResponse;
    void       					*ctx;
    char        				queryLevelString[48] = "";	/* Initialization for AIX compiler */
    DCM_ELEMENT					queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};
    unsigned short		        command;
    MSG_STATUS_DESCRIPTION		statusDescription;
    CTNBOOLEAN					cancelled = FALSE;
    MSG_C_CANCEL_REQ			cancelRequest = {MSG_K_C_CANCEL_REQ, 0, 0, DCM_CMDDATANULL};

    queryLevelElement.d.string = queryLevelString;

    if (callback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CMoveRequest");
    if (moveRequest->type != MSG_K_C_MOVE_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "MOVE Request", "SRV_CMoveRequest");
    if (moveRequest->dataSetType == DCM_CMDDATANULL) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "MOVE Request", "SRV_CMoveRequest");

    ctx = NULL;
    cond = DCM_GetElementValue(&moveRequest->identifier, &queryLevelElement, &l, &ctx);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_OBJECTACCESSFAILED, SRV_Message(SRV_OBJECTACCESSFAILED), "Query Identifier", "SRV_CMoveRequest");

    queryLevelString[l] = '\0';
    if (queryLevelString[l - 1] == ' ') queryLevelString[l - 1] = '\0';

    presentationCtx = SRVPRV_PresentationContext(params, moveRequest->classUID);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_NOSERVICEINASSOCIATION, SRV_Message(SRV_NOSERVICEINASSOCIATION), moveRequest->classUID, "SRV_CMoveRequest");

    cond = MSG_BuildCommand(moveRequest, &commandObject);
    if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "MOVE Request", "SRV_CMoveRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");

    cond = SRV_SendDataSet(association, presentationCtx, &moveRequest->identifier, NULL, NULL, 0);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");

    while (!done){
    	cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");

    	if (messageType != MSG_K_C_MOVE_RESP) {
    		(void) MSG_Free(&message);
    		return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_CMoveRequest");
    	}
    	localResponse = message;
    	cond = MSG_StatusLookup(localResponse->status, MSG_K_C_MOVE_RESP, &statusDescription);
    	if (cond != MSG_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");

    	if (statusDescription.statusClass != MSG_K_CLASS_PENDING) done = TRUE;

#if STANDARD_VERSION >= VERSION_JUL1993
    	if (localResponse->dataSetType != DCM_CMDDATANULL){
    		cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->dataSet);
    		if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");
    	}
#endif
    	responseCount++;
    	if (!cancelled) {
    		cond = callback(moveRequest, localResponse, responseCount, presentationCtx->abstractSyntax, queryLevelString, callbackCtx);
    		if (cond == SRV_OPERATIONCANCELLED) {
    			cancelled = TRUE;
    			cancelRequest.messageIDRespondedTo = moveRequest->messageID;

    			cond = MSG_BuildCommand(&cancelRequest, &commandObject);
    			if (cond != MSG_NORMAL)
    				return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "CANCEL Request", "SRV_CMoveRequest");

    			cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    			(void) DCM_CloseObject(&commandObject);
    			if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CMoveRequest");
    		}else if (cond != SRV_NORMAL){
    			done = TRUE;
    		}
    	}
    	if (moveResponse != NULL) {
    		*moveResponse = *localResponse;
#if STANDARD_VERSION < VERSION_JUL1993
    		moveResponse->successUIDList = NULL;
    		moveResponse->failedUIDList = NULL;
    		moveResponse->warningUIDList = NULL;
    		moveResponse->conditionalFields &= !(MSG_K_C_MOVE_SUCCESSUID |  MSG_K_C_MOVE_FAILEDUID | MSG_K_C_MOVE_WARNINGUID);
#else
    		moveResponse->dataSet = NULL;
#endif
    	}
    	(void) MSG_Free(&message);
    }
    if (cancelled){
    	return SRV_OPERATIONCANCELLED;
    }else{
    	return SRV_NORMAL;
    }
}

/* SRV_CMoveResponse
**
** Purpose:
**	SRV_CFindResponse is used by an application which is acting as an
**	SCP of the Query/Retrieve SOP class (MOVE).  When an application
**	receives a C-MOVE Request message, it calls this function with the
**	C-MOVE request and other parameters.  SRV_CMoveRequest checks the
**	caller's parameters and polls the network, waiting for an identifier
**	which contains the query used for the MOVE.
**
**	Once SRV_CMoveResponse has read the identifier from the network,
**	the user's callback routine is invoked with the following parameters:
**		MSG_C_MOVE_REQ	*moveRequest
**		MSG_C_MOVE_RESP	*moveResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		moveRequest	Pointer to MSG structure with C-MOVE request.
**		moveRespponse	Pointer to MSG structure with C-MOVE response.
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
**	If responseCount is 1, the callback function initiates a new
**	database search.  This query should identify the entire set of
**	images that are to be sent to the peer.  From the callback
**	routine, the user should create an Association and send images
**	to the destination specified by the MOVE command.
**
**	The callback function can return at any stage of the move process.
**	The caller should modify elements in the MSG_C_MOVE_RESP structure
**	which give the current status of the move operation.  If the
**	move is not complete (pending), SRV_CMoveResponse sends a pending
**	message to the peer application and invokes the callback function
**	again.  In this way, the call can direct the SRV routine to send
**	as many or as few pending responses as desired.  It is possible but
**	not required to send a pending response after reach image is
**	transmitted.
**
**	The user indicates the move is complete by placing the
**	appropriate status value in the status field of the response message.
**	The callback function should always return SRV_NORMAL.  Any
**	other value will cause SRV_CFindRequest to abort the Association.
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	association	The key for the Association on which the MOVE request
**			was received and will be used to transmit the MOVE
**			response.
**	presentationCtx	Pointer to presentation context for this MOVE request.
**	moveRequest	Pointer to the structure which contains the MOVE
**			request received by the application.
**	moveResponse	Pointer to structure in caller's space used to hold the
**			MOVE response message.
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
SRV_CMoveResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_MOVE_REQ ** moveRequest, MSG_C_MOVE_RESP * moveResponse,
				  SRV_C_MOVE_RESP_CALLBACK * moveCallback, void *moveCtx, char *dirName)
{
    int       		flag, responseCount = 0;
    U32				l;
    char        	queryLevelString[48] = "";	/* Initialization for AIX compiler */
    CONDITION		cond, rtnCond = SRV_NORMAL;
    DCM_OBJECT		* responseObject;
    void       		*ctx;
    DCM_ELEMENT		queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};

    static char 	*allowedQueryLevels[] = {
							DCM_QUERYLEVELPATIENT,
							DCM_QUERYLEVELSTUDY,
							DCM_QUERYLEVELSERIES,
							DCM_QUERYLEVELIMAGE};

    MSG_STATUS_DESCRIPTION	statusDescription;
    char pendingMsg[] = "\
In SRV_CMoveResponse, the response message returned by your callback has \n\
a status of pending and data set that is not null.\n";
    unsigned short      cancelCommand;
    MSG_TYPE			cancelMsgType;
    MSG_C_CANCEL_REQ	* cancelMessage;
    char		        classUID[DICOM_UI_LENGTH + 1];

    queryLevelElement.d.string = queryLevelString;

    if (moveCallback == NULL) {
    	(void) MSG_Free((void **) moveRequest);
    	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CMoveResponse");
    }
    if (moveResponse->type != MSG_K_C_MOVE_RESP) {
    	(void) MSG_Free((void **) moveRequest);
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "MOVE Request", "SRV_CMoveResponse");
    }
    (void) strcpy(classUID, (*moveRequest)->classUID);

#if STANDARD_VERSION < VERSION_JUL1993
    clearUIDList(&moveResponse->successUIDList);
    clearUIDList(&moveResponse->failedUIDList);
    clearUIDList(&moveResponse->warningUIDList);
    if (createUIDList(&moveResponse->successUIDList) != SRV_NORMAL)
    	return COND_PushCondition(SRV_LISTCREATEFAILURE, SRV_Message(SRV_LISTCREATEFAILURE), "SRV_CMoveResponse");
    if (createUIDList(&moveResponse->failedUIDList) != SRV_NORMAL)
    	return COND_PushCondition(SRV_LISTCREATEFAILURE, SRV_Message(SRV_LISTCREATEFAILURE), "SRV_CMoveResponse");
    if (createUIDList(&moveResponse->warningUIDList) != SRV_NORMAL)
    	return COND_PushCondition(SRV_LISTCREATEFAILURE, SRV_Message(SRV_LISTCREATEFAILURE), "SRV_CMoveResponse");
#else
    cond = DCM_CreateObject(&moveResponse->dataSet, 0);
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
#endif

    cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &(*moveRequest)->identifier);

    if (PRVSRV_debug && (cond == SRV_NORMAL)) (void) DCM_DumpElements(&(*moveRequest)->identifier, 0);

    if (cond != SRV_NORMAL) {
    	(void) MSG_Free((void **) moveRequest);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
    }
    ctx = NULL;
    cond = DCM_GetElementValue(&(*moveRequest)->identifier, &queryLevelElement, &l, &ctx);
    if (cond != DCM_NORMAL) {
    	(void) MSG_Free((void **) moveRequest);
    	(void) COND_PushCondition(SRV_QUERYLEVELATTRIBUTEMISSING, SRV_Message(SRV_QUERYLEVELATTRIBUTEMISSING), "SRV_CMoveResponse");
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
    }
    queryLevelString[l] = '\0';
    if (queryLevelString[l - 1] == ' ') queryLevelString[l - 1] = '\0';

    for (flag = 0, l = 0; l < DIM_OF(allowedQueryLevels) && !flag; l++){
    	if (strcmp(queryLevelString, allowedQueryLevels[l]) == 0) flag = 1;
    }
    if (!flag) {
    	(void) MSG_Free((void **) moveRequest);
    	(void) COND_PushCondition(SRV_ILLEGALQUERYLEVELATTRIBUTE, SRV_Message(SRV_ILLEGALQUERYLEVELATTRIBUTE), queryLevelString, "SRV_CMoveResponse");
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
    }
    moveResponse->messageIDRespondedTo = (*moveRequest)->messageID;
    moveResponse->conditionalFields = 0;
    moveResponse->remainingSubOperations = 0;
    moveResponse->completedSubOperations = 0;
    moveResponse->failedSubOperations = 0;
    moveResponse->warningSubOperations = 0;

    moveResponse->conditionalFields = MSG_K_C_MOVERESP_CLASSUID;

    flag = 0;
    while (!flag) {
    	cond = SRV_TestForCancel(association, DUL_NOBLOCK, 0, presentationCtx->presentationContextID, &cancelCommand, &cancelMsgType, (void **) &cancelMessage);
    	if (cond == SRV_NORMAL){
    		rtnCond = SRV_OPERATIONCANCELLED;
    		moveResponse->status = MSG_K_CANCEL;
    		(void) MSG_Free((void **) &cancelMessage);
    		(void) moveCallback(*moveRequest, moveResponse, 0, presentationCtx->abstractSyntax, queryLevelString, moveCtx);
    		moveResponse->status = MSG_K_CANCEL;
    		moveResponse->messageIDRespondedTo = (*moveRequest)->messageID;
    		moveResponse->dataSetType = DCM_CMDDATANULL;
    	}else{
    		responseCount++;
    		moveResponse->dataSetType = DCM_CMDDATANULL;
    		moveResponse->status = 0xffff;
    		cond = moveCallback(*moveRequest, moveResponse, responseCount, presentationCtx->abstractSyntax, queryLevelString, moveCtx);
    	}
    	if (cond == SRV_NORMAL) {
    		cond = MSG_StatusLookup(moveResponse->status, MSG_K_C_MOVE_RESP, &statusDescription);
    		if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
    		if (statusDescription.statusClass != MSG_K_CLASS_PENDING) flag = 1;

    		/* Consistency check */
    		if ((statusDescription.statusClass == MSG_K_CLASS_PENDING) && (moveResponse->dataSetType != DCM_CMDDATANULL)) {
    			if (PRVSRV_debug) fprintf(DEBUG_DEVICE, pendingMsg);
    			rtnCond = COND_PushCondition(SRV_SUSPICIOUSRESPONSE, SRV_Message(SRV_SUSPICIOUSRESPONSE), "C-MOVE",	"pending", "not null", "SRV_CFindResponse");
    		}
    	}else{
    		flag = 1;
    	}
    	strcpy(moveResponse->classUID, classUID);
    	moveResponse->conditionalFields |= MSG_K_C_MOVERESP_CLASSUID;

    	cond = MSG_BuildCommand(moveResponse, &responseObject);
    	if (cond != MSG_NORMAL) {
#if STANDARD_VERSION < VERSION_JUL1993
    		clearUIDList(&moveResponse->successUIDList);
    		clearUIDList(&moveResponse->failedUIDList);
    		clearUIDList(&moveResponse->warningUIDList);
#endif
    		(void) MSG_Free((void **) moveRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
    	}

    	cond = SRV_SendCommand(association, presentationCtx, &responseObject);
		if (cond != SRV_NORMAL) {
#if STANDARD_VERSION < VERSION_JUL1993
			clearUIDList(&moveResponse->successUIDList);
			clearUIDList(&moveResponse->failedUIDList);
			clearUIDList(&moveResponse->warningUIDList);
#endif
			(void) MSG_Free((void **) moveRequest);
			return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
		}
		if (moveResponse->dataSetType != DCM_CMDDATANULL) {
			cond = SRV_SendDataSet(association, presentationCtx,  &moveResponse->dataSet, NULL, NULL, 0);
			if (cond != SRV_NORMAL) {
#if STANDARD_VERSION < VERSION_JUL1993
				clearUIDList(&moveResponse->successUIDList);
				clearUIDList(&moveResponse->failedUIDList);
				clearUIDList(&moveResponse->warningUIDList);
#endif
				(void) MSG_Free((void **) moveRequest);
				return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CMoveResponse");
			}
		}
    }
    (void) MSG_Free((void **) moveRequest);
    return rtnCond;
}

#if STANDARD_VERSION < VERSION_JUL1993
/* clearUIDList
**
** Purpose:
**	Free the list of UIDs
**
** Parameter Dictionary:
**	l	Handle to the list of UIDs
**
** Return Values:
**	None
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
clearUIDList(LST_HEAD ** l)
{
    MSG_UID_ITEM    * UIDItem;

    if (*l == NULL)	return;

    while ((UIDItem = LST_Pop(l)) != NULL){
    	CTN_FREE(UIDItem);
    }
}

/* createUIDList
**
** Purpose:
**	Create a list of UIDs.
**
** Parameter Dictionary:
**	l	Handle to the list of UIDs to be created
**
** Return Values:
**	SRV_LISTCREATEFAILURE
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
createUIDList(LST_HEAD ** l)
{
    if (*l != NULL)	return SRV_NORMAL;
    if ((*l = LST_Create()) == NULL) return SRV_LISTCREATEFAILURE;

    return SRV_NORMAL;
}
#endif
