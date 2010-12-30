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
** Module Name(s):	SRV_NActionRequest
**			SRV_NActionResponse
** Author, Date:	Aniruddha S. Gokhale, July 1, 1993
** Intent:		This module contains routines which implement the
**			normalized action service class (user and provider)
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:08 $
** Source File:		$RCSfile: naction.c,v $
** Revision:		$Revision: 1.24 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.24 $ $RCSfile: naction.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _MSC_VER
#include <sys/types.h>
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

/* SRV_NActionRequest
**
** Purpose:
**	SRV_NActionRequest assists an application that wants to be an SCU of
**	a number of SOP classes. This function constructs an N-ACTION-REQ
**	message and sends it to the peer application which is acting as the
**	SCP for a SOP class. This function waits for the response from the
**	peer application and invokes the caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_N_ACTION_REQ	*actionRequest
**		MSG_N_ACTION_RESP	*actionResponse
**		void			*actionCtx
**
**	The first two arguments are MSG structures that contain the N-ACTION
**	Request and N-ACTION Response messages respectively. The final
**	argument is the caller's context variable that is passed to
**	SRV_NActionRequest.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association. This
**			list includes the list of presentation contexts.
**	SOPClass	UID of the SOP class used when the association was
**			negotiated. Since this can be a meta class, it may
**			not be the same as the class UID in the N-ACTION
**			request message.
**	actionRequest	Pointer to the structure with the N-ACTION request
**			parameters.
**	actionResponse	Pointer to caller's pointer to an N-ACTION response.
**			This function will allocate an MSG_N_ACTION_RESP
**			message and return the pointer to the caller.
**	actionCallback	Address of user callback function to be called
**			with the N-ACTION response from SCP.
**	actionCtx	Pointer to user context information which will be
**			passed to the callback function. Caller uses this
**			variable to contain any context required for the
**			callback function.
**      dirName         Name of directory where files may be created.
**
**
** Return Values:
**
**	SRV_CALLBACKABORTEDSERVICE
**	SRV_ILLEGALPARAMETER
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_OBJECTBUILDFAILED
**	SRV_REQUESTFAILED
**	SRV_UNEXPECTEDCOMMAND
**	SRV_UNSUPPORTEDSERVICE
**
** Algorithm:
**	Check if the callback function has been provided.
**	Determine the presentation context
**	Encode the N-ACTION request message as a command object and
**		send it to an SCP
**	Send data set, if one exists, to the SCP.
**	Wait for a response message to arrive from the SCP
**	Receive a data set, if one exists, from the SCP
**	Return address of Response message structure to caller.
**
** Notes:
**	The caller is responsible for explicitly setting the following
**	fields in the N-ACTION request message:
**
**	type
**	messageID
**	classUID
**	dataSetType
**	instanceUID
**	actionTypeID
**
**	The caller is also responsible for releasing the Response message
**	structure after the SRV_NActionRequest function returns,
**	using MSG_Free.
*/
CONDITION
SRV_NActionRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_ACTION_REQ * actionRequest,
				   MSG_N_ACTION_RESP * actionResponse, SRV_N_ACTION_REQ_CALLBACK * actionCallback, void *actionCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void       					*message;
    MSG_TYPE					messageType;
    MSG_N_ACTION_RESP			* localResponse;
    unsigned short        		command;

    if (actionCallback == NULL)	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NActionRequest");

    presentationCtx = SRVPRV_PresentationContext(params, SOPClass);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), actionRequest->classUID, "SRV_NActionRequest");

    if (actionRequest->type != MSG_K_N_ACTION_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-ACTION Request", "SRV_NActionRequest");

    cond = MSG_BuildCommand(actionRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-ACTION Request", "SRV_NActionRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NActionRequest");

    if (actionRequest->dataSetType != DCM_CMDDATANULL){
    	/* application specific action information exists */
    	cond = SRV_SendDataSet(association, presentationCtx, &actionRequest->actionInformation, NULL, NULL, 0);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NActionRequest");
    }

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NActionRequest");

    if (messageType != MSG_K_N_ACTION_RESP) {
    	(void) MSG_Free(&message);
    	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_NActionRequest");
    }
    localResponse = (MSG_N_ACTION_RESP *) message;

    if (localResponse->dataSetType == DCM_CMDDATANULL) {
    	localResponse->actionReply = NULL;
    }else{
    	/* application specific action reply needs to be received */
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->actionReply);
    	if (cond != SRV_NORMAL) {
    		(void) MSG_Free(&message);
    		return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NActionRequest");
    	}
    }

    cond = actionCallback(actionRequest, localResponse, actionCtx);
    (void) MSG_Free(&message);
    if (localResponse->actionReply != NULL) {
    	(void) DCM_CloseObject(&localResponse->actionReply);
    	localResponse->actionReply = NULL;
    }
    if (actionResponse != NULL)	*actionResponse = *localResponse;

    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NActionRequest");

    return SRV_NORMAL;
}

/* SRV_NActionResponse
**
** Purpose:
**	SRV_NActionResponse assists an application that wants to be an SCP
**	of a number of SOP classes. When an application receives an N-ACTION
**	request message, it calls this function with the N-ACTION request
**	and other parameters. SRV_NActionResponse checks the caller's
**	parameters and calls the user's callback function. In the callback
**	function, the caller fills in the parameters of the N-ACTION response
**	message and then returns to the SRV function.
**
**	After the callback function returns, SRV_NActionResponse constructs a
**	N-ACTION Response message and sends it to the peer application which
**	sent the request message.
**
**	The arguments to the callback function are:
**		MSG_N_ACTION_REQ	*actionRequest
**		MSG_N_ACTION_RESP	*actionResponse
**		void			*actionCtx
**		DUL_PRESENTATIONCONTEXT	*pc
**
**	The first two arguments are MSG structures that contain the N-ACTION
**	Request and N-ACTION Response messages respectively. The third
**	argument is the caller's context variable that is passed to
**	SRV_NActionResponse.  The presentation context describes the SOP
**	class that was negotiated for this message.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used for transmitting the N-ACTION
**			response.
**	presentationCtx	Pointer to presentation context to be used when sending
**			the N-ACTION response.
**	actionRequest	Pointer to the structure with the N-ACTION request
**			parameters which was received by the application.
**	actionResponse	Pointer to structure in the caller's area which will be
**			filled with the parameters of the N-ACTION response
**			command. After the parameters are filled in, the
**			N-ACTION response is sent to the peer application
**			which sent the request.
**	actionCallback	Address of user callback function to be called
**			with the N-ACTION response from SCP.
**	actionCtx	Pointer to user context information which will be
**			passed to the callback function. Caller uses this
**			variable to contain any context required for the
**			callback function.
**      dirName         Name of directory where files may be created.
**
**
** Return Values:
**
**	SRV_CALLBACKABORTEDSERVICE
**	SRV_ILLEGALPARAMETER
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_OBJECTBUILDFAILED
**	SRV_RESPONSEFAILED
**
** Algorithm:
**	Receive a data set, if any, from the peer application
**	Invoke the callback function, if it exists.
**	Build a command and send it to the requesting peer application.
**	Send a data set, if any, to the peer application.
**
** Notes:
**	The callback function is responsible for explicitly setting the
**	following fields in the N-ACTION response message:
**
**	type
**	messageIDRespondedTo
**	classUID
**	dataSetType
**	instanceUID
**	status
**	actionReply (if any)
**
*/
CONDITION
SRV_NActionResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_ACTION_REQ ** actionRequest,  MSG_N_ACTION_RESP * actionResponse,
					SRV_N_ACTION_RESP_CALLBACK * actionCallback, void *actionCtx, char *dirName)
{
    CONDITION	rtnCond = SRV_NORMAL, cond;
    DCM_OBJECT	* responseObject = NULL;

    actionResponse->status = MSG_K_SUCCESS;	/* initialize */
    actionResponse->dataSetType = DCM_CMDDATANULL;
    actionResponse->actionReply = NULL;

    if (actionCallback == NULL) {
    	rtnCond = COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NActionResponse");
    	actionResponse->status = MSG_K_RESOURCELIMITATION;
    	goto SEND_RESPONSE;
    }
    /* check if there is any data coming over the network */
    if ((*actionRequest)->dataSetType != DCM_CMDDATANULL) {
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &(*actionRequest)->actionInformation);
    	if (cond != SRV_NORMAL) {
    		rtnCond = COND_PushCondition(SRV_RECEIVEDATASETFAILED, SRV_Message(SRV_RECEIVEDATASETFAILED), "SRV_NActionResponse");
    		actionResponse->status = MSG_K_PROCESSINGFAILURE;
    		goto SEND_RESPONSE;
    	}
    }
    /* SRV routine allocates space for the action reply */
    cond = DCM_CreateObject(&actionResponse->actionReply, 0);
    if (cond != DCM_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "SRV_NActionResponse");
    	actionResponse->status = MSG_K_PROCESSINGFAILURE;
    	goto SEND_RESPONSE;
    }
    cond = actionCallback(*actionRequest, actionResponse, actionCtx, presentationCtx);
    if (cond != SRV_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NActionRequest");
    	goto SEND_RESPONSE;
    }
    if (actionResponse->type != MSG_K_N_ACTION_RESP) {
    	rtnCond = COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-ACTION Response", "SRV_NActionResponse");
    	goto SEND_RESPONSE;
    }
SEND_RESPONSE:
    /* At this point we free the request pointer */
    (void) MSG_Free((void **) actionRequest);

    /*
     * If there are any errors in the following steps, it is necessary for
     * the SRV routine to free the data set object it had created earlier.
     * But we must verify that such a data set creation operation itself was
     * successful before we try to free the data set
     */

    /* send the action reply (if any) only on success status */
    if (actionResponse->status != MSG_K_SUCCESS) {
    	actionResponse->dataSetType = DCM_CMDDATANULL;
    	if (actionResponse->actionReply) (void) DCM_CloseObject(&actionResponse->actionReply);
    	return rtnCond;
    }
    /* build a command object */
    cond = MSG_BuildCommand(actionResponse, &responseObject);
    if (cond != MSG_NORMAL) {
    	if (actionResponse->actionReply) (void) DCM_CloseObject(&actionResponse->actionReply);
    	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-ACTION Response", "SRV_NActionResponse");
    }

    cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    (void) DCM_CloseObject(&responseObject);
    if (cond != SRV_NORMAL) {
    	if (actionResponse->actionReply) (void) DCM_CloseObject(&actionResponse->actionReply);
    	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "Command", "SRV_NActionResponse");
    }
    /*
     * At this point, the response message status is "SUCCESS". This also
     * implies that the "data set" object exists but may be empty (not NULL)
     * Hence if there is any error, we need to close it first before we
     * return.
     */
    if (actionResponse->dataSetType != DCM_CMDDATANULL) {
    	cond = SRV_SendDataSet(association, presentationCtx, &actionResponse->actionReply, NULL, NULL, 0);
    	if (cond != SRV_NORMAL) {
    		(void) DCM_CloseObject(&actionResponse->actionReply);
    		return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "Data Set", "SRV_NActionResponse");
    	}
    }
    (void) DCM_CloseObject(&actionResponse->actionReply);	/* was allocated memory */
    return SRV_NORMAL;		/* everything is successful at this point */
}
