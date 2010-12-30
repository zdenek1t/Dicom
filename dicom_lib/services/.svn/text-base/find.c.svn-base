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
** Module Name(s):	SRV_FindRequest
**			SRV_FindProvide
** Author, Date:	Stephen M. Moore, 2-Jun-93
** Intent:		This module contains routines which implement the
**			query service class (user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:07 $
** Source File:		$RCSfile: find.c,v $
** Revision:		$Revision: 1.34 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.34 $ $RCSfile: find.c,v $";

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

static CTNBOOLEAN
classRequiresLevel(const char *classUID)
{
    if (strcmp(classUID, DICOM_SOPMODALITYWORKLIST_FIND) == 0){
    	return FALSE;
    }else{
    	return TRUE;
    }
}

/* SRV_CFindRequest
**
** Purpose:
**	SRV_CFindRequest assists an application that wants to be an SCU of
**	the Query SOP class.  This function constructs a C-FIND-REQ Message
** 	and sends it to the peer application which is acting as the SCP for
**	the Query SOP class.  This function waits for the responses from
**	the peer application and invokes the caller's callback function
**	one time for each response.
**
**	The arguments to the callback function are:
**		MSG_C_FIND_REQ	*findRequest
**		MSG_C_FIND_RESP	*findResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		findRequest	Pointer to MSG structure with C-FIND request.
**		findRespponse	Pointer to MSG structure with C-FIND response.
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
**	the contents of the status and dataSetType fields.  This will tell
**	the user if the identifier in the C-FIND response contains an
**	actual response or is NULL.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		Parameters which define the service classes that are
**			available on this Association.
**	findRequest	Pointer to structure in caller's memory which contains
**			the query request.
**	findResponse	Address of pointer to response message. This function
**			will create a response message and return the
**			address of the structure to the caller.
**	callback	Address of user routine which is called one time for
**			each response received from the network.
**	ctx		User context information which is supplied during call
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
SRV_CFindRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_FIND_REQ * findRequest, MSG_C_FIND_RESP * findResponse,
				 SRV_C_FIND_REQ_CALLBACK * callback, void *callbackCtx, char *dirName)
{
    DCM_OBJECT						* commandObject;	/* Handle for a command object */
    CONDITION						cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT			* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID		ctxID;
    void       						*message;
    MSG_TYPE						messageType;
    int        						done = 0;
    unsigned long        			responseCount = 0;
    MSG_C_FIND_RESP					* localResponse;
    char        					queryLevelString[48] = "";	/* Initialization for AIX compiler */
    DCM_ELEMENT						queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};
    unsigned short        			command;
    MSG_STATUS_DESCRIPTION			statusDescription;
    CTNBOOLEAN						cancelled = FALSE;
    MSG_C_CANCEL_REQ				cancelRequest = {MSG_K_C_CANCEL_REQ, 0, 0, DCM_CMDDATANULL};
    CTNBOOLEAN 						levelFlag;

    queryLevelElement.d.string = queryLevelString;

    if (callback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CFindRequest");
    if (findRequest->type != MSG_K_C_FIND_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "FIND Request", "SRV_CFindRequest");
    if (findRequest->dataSetType == DCM_CMDDATANULL) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "FIND Request", "SRV_CFindRequest");

    presentationCtx = SRVPRV_PresentationContext(params, findRequest->classUID);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_NOSERVICEINASSOCIATION, SRV_Message(SRV_NOSERVICEINASSOCIATION), findRequest->classUID, "SRV_CFindRequest");

    /*
     * See if the caller included a string for query level.  This is *
     * required for query/retrieve classes, but not for Modality Work List
     */

    cond = DCM_ParseObject(&findRequest->identifier, &queryLevelElement, 1, NULL, 0, NULL);
    if (cond != DCM_NORMAL) {
    	levelFlag = classRequiresLevel(findRequest->classUID);
    	if (levelFlag) {	/* e.g., Patient Root model requires level */
    		return COND_PushCondition(SRV_OBJECTACCESSFAILED, SRV_Message(SRV_OBJECTACCESSFAILED), "Query Identifier", "SRV_CFindRequest");
    	}else{
    		(void) COND_PopCondition(TRUE);
    	}
    }

    cond = MSG_BuildCommand(findRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "FIND Request", "SRV_CFindRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");

    cond = SRV_SendDataSet(association, presentationCtx, &findRequest->identifier, NULL, NULL, 0);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");

    while (!done) {
    	cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");

    	if (messageType != MSG_K_C_FIND_RESP) {
    		(void) MSG_Free(&message);
    		return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_CFindRequest");
    	}
    	localResponse = message;

    	cond = MSG_StatusLookup(localResponse->status, MSG_K_C_FIND_RESP, &statusDescription);
    	if (cond != MSG_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");

    	if (statusDescription.statusClass != MSG_K_CLASS_PENDING) done = TRUE;

    	if (localResponse->dataSetType != DCM_CMDDATANULL) {
    		cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->identifier);
    		if (cond != SRV_NORMAL) {
    			(void) MSG_Free(&message);
    			return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");
    		}
    		responseCount++;
    	}else{
    		localResponse->identifier = NULL;
    	}
    	if (!cancelled) {
    		cond = callback(findRequest, localResponse, responseCount, presentationCtx->abstractSyntax, queryLevelString, callbackCtx);
    		if (cond == SRV_OPERATIONCANCELLED) {
    			cancelled = TRUE;
    			cancelRequest.messageIDRespondedTo = findRequest->messageID;
    			cond = MSG_BuildCommand(&cancelRequest, &commandObject);
    			if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "CANCEL Request", "SRV_CFindRequest");

    			cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    			(void) DCM_CloseObject(&commandObject);
    			if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CFindRequest");
    		}else if (cond != SRV_NORMAL){
    			done = TRUE;
    		}
    	}
    	if (localResponse->identifier != NULL) (void) DCM_CloseObject(&localResponse->identifier);

    	if (findResponse != NULL) {
    		*findResponse = *localResponse;
    		findResponse->identifier = NULL;
    	}

    	(void) MSG_Free(&message);
    }
    if (cancelled){
    	return SRV_OPERATIONCANCELLED;
    }else{
    	return SRV_NORMAL;
    }
}

/* SRV_CFindResponse
**
** Purpose:
**	SRV_CFindResponse is used by an application which is acting as an
**	SCP of the C-FIND service.  When an application receives a
**	C-FIND Request message, it calls this function with the C-FIND
**	request and other parameters.  SRV_CFindRequest checks the caller's
**	parameters and polls the network, waiting for an identifier which
**	contains the query.
**
**	Once SRV_CFindResponse has read the identifier from the network,
**	it creates an empty DCM_OBJECT in the identifier of the response
**	message.  The user's callback routine is invoked with the
**	following parameters:
**		MSG_C_FIND_REQ	*findRequest
**		MSG_C_FIND_RESP	*findResponse
**		int		responseCount
**		char		*abstractSyntax
**		char		*queryLevelString
**		void		*callbackCtx
**	where
**		findRequest	Pointer to MSG structure with C-FIND request.
**		findRespponse	Pointer to MSG structure with C-FIND response.
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
**	database search.  When the first response is received, the caller
**	modifies the elements in the identifier in the response message
**	and returns.  SRV_CFindResponse takes the identifier, formats
**	a C-FIND response message, and transmits the message to the requesting
**	peer application.  After the response is sent to the SCU application,
**	SRV_CFindResponse invokes the callback function again.
**
**	If responseCount is any value other than 1, the callback function
**	continues the database search.  For each match, the caller modifies
**	the elements in the identifier in the response message and returns.
**	The SRV function sends the proper message to the peer application
**	for each response.
**
**	The user indicates the search is complete by placing the
**	appropriate status value in the status field of the response message.
**	The callback function should always return SRV_NORMAL.  Any
**	other value will cause SRV_CFindRequest to abort the Association.
**
** Parameter Dictionary:
**	association	The key for the Association on which the FIND request
**			was received and will be used to transmit the FIND
**			response.
**	ctx		Pointer to presentation context for this FIND request.
**	findRequest	Pointer to the structure which contains the FIND
**			request received by the application.
**	findResponse	Pointer to structure in caller's space used to hold the
**			FIND response message.
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
SRV_CFindResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_FIND_REQ ** findRequest, MSG_C_FIND_RESP * findResponse,
				  SRV_C_FIND_RESP_CALLBACK * findCallback, void *findCtx, char *dirName)
{
    int        		flag = 0, responseCount = 0;
    U32				l;
    char        	queryLevelString[48] = "";	/* Initialization for AIX compiler */
    CONDITION 		cond, rtnCond = SRV_NORMAL;
    DCM_OBJECT		* responseObject;
    DCM_ELEMENT		queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), NULL};
    static char 	*allowedQueryLevels[] = {
						DCM_QUERYLEVELPATIENT,
						DCM_QUERYLEVELSTUDY,
						DCM_QUERYLEVELSERIES,
						DCM_QUERYLEVELIMAGE};
    MSG_STATUS_DESCRIPTION		statusDescription;

    char pendingNullMsg[] = "\
In SRV_CFindResponse, the response message returned by your callback has \n\
a status of pending and a null data set.\n";
    char notPendingnotNullMsg[] = "\
In SRV_CFindResponse, the response message returned by your callback has \n\
a status of other than pending and a data set that is not null.\n";
    unsigned short        cancelCommand;
    MSG_TYPE			  cancelMsgType;
    MSG_C_CANCEL_REQ	  * cancelMessage;
    char 				  classUID[DICOM_UI_LENGTH + 1];
    CTNBOOLEAN 			  levelFlag;

    queryLevelElement.d.string = queryLevelString;

    if (findCallback == NULL) {
    	(void) MSG_Free((void **) findRequest);
    	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CFindResponse");
    }
    if (findResponse->type != MSG_K_C_FIND_RESP) {
    	(void) MSG_Free((void **) findRequest);
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "FIND Request", "SRV_CFindResponse");
    }
    (void) strcpy(classUID, (*findRequest)->classUID);

    cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &(*findRequest)->identifier);
    if (PRVSRV_debug && (cond == SRV_NORMAL)) (void) DCM_DumpElements(&(*findRequest)->identifier, 0);
    if (cond != SRV_NORMAL) {
    	(void) MSG_Free((void **) findRequest);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    }

    cond = DCM_ParseObject(&(*findRequest)->identifier, &queryLevelElement, 1, NULL, 0, NULL);

    /*
     * Determine if the C-FIND request has a query level.  If not, * check to
     * see if the SOP class requires it.  Abort the request * right now if we
     * need it (e.g., Patient Root queries require it).
     */
    levelFlag = classRequiresLevel((*findRequest)->classUID);
    if (cond != DCM_NORMAL) {
    	levelFlag = classRequiresLevel((*findRequest)->classUID);
    	if (levelFlag) {
    		(void) MSG_Free((void **) findRequest);
    		(void) COND_PushCondition(SRV_QUERYLEVELATTRIBUTEMISSING, SRV_Message(SRV_QUERYLEVELATTRIBUTEMISSING), "SRV_CFindResponse");
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    	}
    }
    if (levelFlag) {
    	for (flag = 0, l = 0; l < DIM_OF(allowedQueryLevels) && !flag; l++){
    		if (strcmp(queryLevelString, allowedQueryLevels[l]) == 0) flag = 1;
    	}
    }
    if (levelFlag && !flag) {
    	(void) MSG_Free((void **) findRequest);
    	(void) COND_PushCondition(SRV_ILLEGALQUERYLEVELATTRIBUTE, SRV_Message(SRV_ILLEGALQUERYLEVELATTRIBUTE), queryLevelString, "SRV_CFindResponse");
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    }
    findResponse->messageIDRespondedTo = (*findRequest)->messageID;
    findResponse->conditionalFields = MSG_K_C_FINDRESP_CLASSUID;

    cond = DCM_CreateObject(&findResponse->identifier, 0);
    if (cond != DCM_NORMAL) {
    	(void) MSG_Free((void **) findRequest);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    }
    if (levelFlag) {
    	queryLevelElement.length = strlen(queryLevelString);
    	cond = DCM_AddElement(&findResponse->identifier, &queryLevelElement);
    	if (cond != DCM_NORMAL) {
    		(void) MSG_Free((void **) findRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    	}
    }
    flag = 0;
    while (!flag) {
    	cond = SRV_TestForCancel(association, DUL_NOBLOCK, 0, presentationCtx->presentationContextID, &cancelCommand, &cancelMsgType, (void **) &cancelMessage);
    	if (cond == SRV_NORMAL) {
    		rtnCond = SRV_OPERATIONCANCELLED;
    		findResponse->status = MSG_K_CANCEL;
    		(void) MSG_Free((void **) &cancelMessage);
    		(void) findCallback(*findRequest, findResponse, 0, presentationCtx->abstractSyntax, queryLevelString, findCtx);
    		findResponse->status = MSG_K_CANCEL;
    		findResponse->messageIDRespondedTo = (*findRequest)->messageID;
    		findResponse->dataSetType = DCM_CMDDATANULL;
    	}else{
    		responseCount++;
    		findResponse->dataSetType = DCM_CMDDATANULL;
    		findResponse->status = 0xffff;
    		cond = findCallback(*findRequest, findResponse, responseCount, presentationCtx->abstractSyntax, queryLevelString, findCtx);
    	}

    	if (cond == SRV_NORMAL) {
    		cond = MSG_StatusLookup(findResponse->status, MSG_K_C_FIND_RESP, &statusDescription);
    		if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    		if (statusDescription.statusClass != MSG_K_CLASS_PENDING) flag = 1;

    		/* Consistency check */
    		if ((statusDescription.statusClass == MSG_K_CLASS_PENDING) && (findResponse->dataSetType == DCM_CMDDATANULL)) {
    			if (PRVSRV_debug) fprintf(DEBUG_DEVICE, pendingNullMsg);
    			rtnCond = COND_PushCondition(SRV_SUSPICIOUSRESPONSE, SRV_Message(SRV_SUSPICIOUSRESPONSE), "C-FIND", "pending", "null", "SRV_CFindResponse");
    		}
    		if ((statusDescription.statusClass != MSG_K_CLASS_PENDING) && (findResponse->dataSetType != DCM_CMDDATANULL)) {
    			if (PRVSRV_debug) fprintf(DEBUG_DEVICE, notPendingnotNullMsg);
    			rtnCond = COND_PushCondition(SRV_SUSPICIOUSRESPONSE, SRV_Message(SRV_SUSPICIOUSRESPONSE), "C-FIND", "not pending", "not null", "SRV_CFindResponse");
    		}
    	}else{
    		findResponse->dataSetType = DCM_CMDDATANULL;
    	}

    	findResponse->conditionalFields |= MSG_K_C_FINDRESP_CLASSUID;
    	(void) strcpy(findResponse->classUID, classUID);

    	cond = MSG_BuildCommand(findResponse, &responseObject);
    	if (cond != MSG_NORMAL) {
    		(void) MSG_Free((void **) findRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    	}

    	cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    	(void) DCM_CloseObject(&responseObject);
    	if (cond != SRV_NORMAL) {
    		(void) MSG_Free((void **) findRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    	}

    	if (findResponse->dataSetType != DCM_CMDDATANULL) {
    		cond = SRV_SendDataSet(association, presentationCtx, &findResponse->identifier, NULL, NULL, 0);
    		if (cond != SRV_NORMAL) {
    			(void) MSG_Free((void **) findRequest);
    			return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CFindResponse");
    		}
    	}
    }
    (void) MSG_Free((void **) findRequest);
    (void) DCM_CloseObject(&findResponse->identifier);
    return rtnCond;
}
