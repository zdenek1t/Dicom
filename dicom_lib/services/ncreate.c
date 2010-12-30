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
** Module Name(s):	SRV_NCreateRequest
**			SRV_NCreateResponse
** Author, Date:	Aniruddha S. Gokhale, 21-June-93
** Intent:		This module contains routines which implement the
**			normalized create service class (user and provider)
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:08 $
** Source File:		$RCSfile: ncreate.c,v $
** Revision:		$Revision: 1.30 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.30 $ $RCSfile: ncreate.c,v $";

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


/* SRV_NCreateRequest
**
** Purpose:
**	SRV_NCreateRequest assists an application that wants to be an SCU of
**	a number of SOP classes. This function constructs an N-CREATE-REQ
**	message and sends it to the peer application which is acting as the
**	SCP for a SOP class. This function waits for the response from the
**	peer application and invokes the caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_N_CREATE_REQ	*createRequest
**		MSG_N_CREATE_RESP	*createResponse
**		void			*createCtx
**
**	The first two arguments are MSG structures that contain the N-CREATE
**	Request and N-CREATE Response messages respectively. The final
**	argument is the caller's context variable that is passed to
**	SRV_NCreateRequest.
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
**			not be the same as the class UID in the N-CREATE
**			request message.
**	createRequest	Pointer to the structure with the N-CREATE request
**			parameters.
**	createResponse	Pointer to caller's pointer to an N-CREATE response.
**			This function will allocate an MSG_N_CREATE_RESP
**			message and return the pointer to the caller.
**	createCallback	Address of user callback function to be called
**			with the N-CREATE response from SCP.
**	createCtx	Pointer to user context information which will be
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
**	Encode the N-CREATE request message as a command object and
**		send it to an SCP
**	Send data set, if one exists, to the SCP.
**	Wait for a response message to arrive from the SCP
**	Receive a data set, if one exists, from the SCP
**	Return address of Response message structure to caller.
**
** Notes:
**	The caller is responsible for explicitly setting the following
**	fields in the N-CREATE request message:
**
**	type
**	messageID
**	classUID
**	dataSetType
**	instanceUID (if any)
**	dataSet
**
**	The caller is also responsible for releasing the Response message
**	structure after the SRV_NCreateRequest function returns,
**	using MSG_Free.
*/
CONDITION
SRV_NCreateRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_CREATE_REQ * createRequest,
				   MSG_N_CREATE_RESP * createResponse, SRV_N_CREATE_REQ_CALLBACK * createCallback, void *createCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void       					*message;
    MSG_TYPE					messageType;
    MSG_N_CREATE_RESP			* localResponse;
    unsigned short        		command;

    if (createCallback == NULL)	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NCreateRequest");

    presentationCtx = SRVPRV_PresentationContext(params, SOPClass);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), createRequest->classUID, "SRV_NCreateRequest");
    if (createRequest->type != MSG_K_N_CREATE_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-CREATE Request", "SRV_NCreateRequest");

    cond = MSG_BuildCommand(createRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-CREATE Request", "SRV_NCreateRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NCreateRequest");

    if (createRequest->dataSetType == DCM_CMDDATANULL) /* sending attribute list is mandatory */
    	return COND_PushCondition(SRV_ILLEGALPARAMETER,	SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "N-CREATE Request", "SRV_NCreateRequest");

    cond = SRV_SendDataSet(association, presentationCtx, &createRequest->dataSet, NULL, NULL, 0);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NCreateRequest");

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NCreateRequest");

    if (messageType != MSG_K_N_CREATE_RESP) {
    	(void) MSG_Free(&message);
    	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_NCreateRequest");
    }
    localResponse = (MSG_N_CREATE_RESP *) message;

    if (localResponse->dataSetType == DCM_CMDDATANULL) {
    	localResponse->dataSet = NULL;
    }else{
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->dataSet);
    	if (cond != SRV_NORMAL){
    		(void) MSG_Free(&message);
    		return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NCreateRequest");
    	}
    }

    cond = createCallback(createRequest, localResponse, createCtx);

    if (localResponse->dataSet != NULL) {
    	(void) DCM_CloseObject(&localResponse->dataSet);
    	localResponse->dataSet = NULL;
    }
    if (createResponse != NULL)	*createResponse = *localResponse;
    (void) MSG_Free(&message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NCreateRequest");

    return SRV_NORMAL;
}

/* SRV_NCreateResponse
**
** Purpose:
**	SRV_NCreateResponse assists an application that wants to be an SCP
**	of a number of SOP classes. When an application receives an N-CREATE
**	request message, it calls this function with the N-CREATE request
**	and other parameters. SRV_NCreateResponse checks the caller's
**	parameters and calls the user's callback function. In the callback
**	function, the caller fills in the parameters of the N-CREATE response
**	message and then returns to the SRV function.
**
**	After the callback function returns, SRV_NCreateResponse constructs a
**	N-CREATE Response message and sends it to the peer application which
**	sent the request message.
**
**	The arguments to the callback function are:
**		MSG_N_CREATE_REQ	*createRequest
**		MSG_N_CREATE_RESP	*createResponse
**		void			*createCtx
**		DUL_PRESENTATIONCONTEXT	*pc
**
**	The first two arguments are MSG structures that contain the N-CREATE
**	Request and N-CREATE Response messages respectively. The third
**	argument is the caller's context variable that is passed to
**	SRV_NCreateResponse.  The presentation context can be used by
**	the callback function to get information about the negotiated
**	SOP Class.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used for transmitting the N-CREATE
**			response.
**	presentationCtx	Pointer to presentation context to be used when sending
**			the N-CREATE response.
**	createRequest	Pointer to the structure with the N-CREATE request
**			parameters which was received by the application.
**	createResponse	Pointer to structure in the caller's area which will be
**			filled with the parameters of the N-CREATE response
**			command. After the parameters are filled in, the
**			N-CREATE response is sent to the peer application
**			which sent the request.
**	createCallback	Address of user callback function to be called
**			with the N-CREATE response from SCP.
**	createCtx	Pointer to user context information which will be
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
**	following fields in the N-CREATE response message:
**
**	type
**	messageIDRespondedTo
**	classUID
**	dataSetType
**	instanceUID
**	dataSet (if any)
**	status
**
*/
CONDITION
SRV_NCreateResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_CREATE_REQ ** createRequest, MSG_N_CREATE_RESP * createResponse,
					SRV_N_CREATE_RESP_CALLBACK * createCallback, void *createCtx, char *dirName)
{
    CONDITION	rtnCond = SRV_NORMAL, cond;
    DCM_OBJECT	* responseObject = NULL;

    createResponse->status = MSG_K_SUCCESS;	/* initialize */
    createResponse->dataSetType = DCM_CMDDATANULL;
    createResponse->dataSet = NULL;

    if (createCallback == NULL) {
    	rtnCond = COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NCreateResponse");
    	createResponse->status = MSG_K_RESOURCELIMITATION;
    	goto SEND_RESPONSE;
    }
    if ((*createRequest)->dataSetType == DCM_CMDDATANULL) {
    	(*createRequest)->dataSet = NULL;
    }else{
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &(*createRequest)->dataSet);
    	if (cond != SRV_NORMAL) {
    		rtnCond = COND_PushCondition(SRV_RECEIVEDATASETFAILED, SRV_Message(SRV_RECEIVEDATASETFAILED), "SRV_NCreateResponse");
    		createResponse->status = MSG_K_PROCESSINGFAILURE;
    		goto SEND_RESPONSE;
    	}
    }

    /* The SRV routine allocates space for a data set */
    cond = DCM_CreateObject(&createResponse->dataSet, 0);
    if (cond != DCM_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "SRV_NCreateResponse");
    	createResponse->status = MSG_K_PROCESSINGFAILURE;
    	goto SEND_RESPONSE;
    }

    cond = createCallback(*createRequest, createResponse, createCtx, presentationCtx);
    if (cond != SRV_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NCreateRequest");
    	goto SEND_RESPONSE;
    }

    if (createResponse->type != MSG_K_N_CREATE_RESP) {
    	rtnCond = COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-CREATE Response", "SRV_NCreateResponse");
    	goto SEND_RESPONSE;
    }
SEND_RESPONSE:
	/*  Assume the callback function has set the status value.  If return value is not normal, we will set a status value of processing failure */
    /* At this point we free the request pointer */
    (void) MSG_Free((void **) createRequest);

    /*
     * If there are any errors in the following steps, it is necessary for
     * the SRV routine to free the data set object it had created earlier.
     * But we must verify that such a data set creation operation itself was
     * successful before we try to free the data set
     */

    /* we send attributes (if any) back only on success status */
    if (createResponse->status != MSG_K_SUCCESS) {
    	createResponse->dataSetType = DCM_CMDDATANULL;
    	if (createResponse->dataSet) (void) DCM_CloseObject(&createResponse->dataSet);
    	return rtnCond;
    }
    /* build a command object to send back */
    cond = MSG_BuildCommand(createResponse, &responseObject);
    if (cond != MSG_NORMAL) {
    	if (createResponse->dataSet) (void) DCM_CloseObject(&createResponse->dataSet);
    	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-CREATE Response", "SRV_NCreateResponse");
    }
    cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    (void) DCM_CloseObject(&responseObject);
    if (cond != SRV_NORMAL) {
    	if (createResponse->dataSet) (void) DCM_CloseObject(&createResponse->dataSet);
    	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "Command", "SRV_NCreateResponse");
    }
    /*
     * At this point, the response message status is "SUCCESS". This also
     * implies that the "data set" object exists but may be empty (not NULL)
     * Hence if there is any error, we need to close it first before we
     * return.
     */

    if (createResponse->dataSetType != DCM_CMDDATANULL) {
    	/* sending attribute list in response message is optional */
    	cond = SRV_SendDataSet(association, presentationCtx, &createResponse->dataSet, NULL, NULL, 0);
    	(void) DCM_CloseObject(&createResponse->dataSet);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NCreateResponse");
    }
    (void) DCM_CloseObject(&createResponse->dataSet);
    return SRV_NORMAL;		/* everything successful at this point */
}
