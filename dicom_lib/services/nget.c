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
** Module Name(s):	SRV_NGetRequest
**			SRV_NGetResponse
**
** Author, Date:	Stephen M. Moore, 2-Jun-93
** Intent:		This module contains routines which implement the
**			query service class (user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:09 $
** Source File:		$RCSfile: nget.c,v $
** Revision:		$Revision: 1.34 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.34 $ $RCSfile: nget.c,v $";

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

/* SRV_NGetRequest
**
** Purpose:
**	SRV_NGetRequest assists an application that wants to be an SCU of
**	a number of SOP classes. This function constructs an N-GET-REQ
**	message and sends it to the peer application which is acting as the
**	SCP for a SOP class. This function waits for the response from the
**	peer application and invokes the caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_N_GET_REQ	*getRequest
**		MSG_N_GET_RESP	*getResponse
**		void		*getCtx
**
**	The first two arguments are MSG structures that contain the N-GET
**	Request and N-GET Response messages respectively. The final
**	argument is the caller's context variable that is passed to
**	SRV_NGetRequest.
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
**			not be the same as the class UID in the N-GET
**			request message.
**	getRequest	Pointer to the structure with the N-GET request
**			parameters.
**	getResponse	Pointer to caller's pointer to an N-GET response.
**			This function will allocate an MSG_N_GET_RESP
**			message and return the pointer to the caller.
**	getCallback	Address of user callback function to be called
**			with the N-GET response from SCP.
**	getCtx		Pointer to user context information which will be
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
**	Encode the N-GET request message as a command object and
**		send it to an SCP
**	Send data set, if one exists, to the SCP.
**	Wait for a response message to arrive from the SCP
**	Receive a data set, if one exists, from the SCP
**	Return address of Response message structure to caller.
**
** Notes:
**	The caller is responsible for explicitly setting the following
**	fields in the N-GET request message:
**
**	type
**	messageID
**	classUID
**	dataSetType
**	requestedInstanceUID
**	attributeList
**	attributeCount
**
**	The caller is also responsible for releasing the Response message
**	structure after the SRV_NGetRequest function returns,
**	using MSG_Free.
*/
CONDITION
SRV_NGetRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,	char *SOPClass,	MSG_N_GET_REQ * getRequest, MSG_N_GET_RESP * getResponse,
				SRV_N_GET_REQ_CALLBACK * getCallback, void *getCtx, char *dirName)
{
    DCM_OBJECT						* commandObject;	/* Handle for a command object */
    CONDITION						cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT			* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID		ctxID;
    void       						*message;
    MSG_TYPE						messageType;
    MSG_N_GET_RESP					* localResponse;
    unsigned short        			command;

    /*
     * The data set type field of the request message must be 0101H to
     * indicate absence of any data set.
     */
    getRequest->dataSetType = DCM_CMDDATANULL;

    if (getCallback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NGetRequest");

    presentationCtx = SRVPRV_PresentationContext(params, SOPClass);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), getRequest->classUID, "SRV_NGetRequest");
    if (getRequest->type != MSG_K_N_GET_REQ) return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-GET Request", "SRV_NGetRequest");

    cond = MSG_BuildCommand(getRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-GET Request", "SRV_NGetRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NGetRequest");

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NGetRequest");

    if (messageType != MSG_K_N_GET_RESP) return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_NGetRequest");

    localResponse = (MSG_N_GET_RESP *) message;

    if (localResponse->dataSetType == DCM_CMDDATANULL)	/* attribute list must be present in the response */
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "N-GET Response", "SRV_NGetRequest");

    cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->dataSet);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NGetRequest");

    cond = SRV_NORMAL;
    cond = getCallback(getRequest, localResponse, getCtx);
    if (cond != SRV_NORMAL)	(void) COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NGetRequest");

    (void) DCM_CloseObject(&localResponse->dataSet);
    localResponse->dataSet = NULL;
    (void) MSG_Free(&message);

    if (getResponse != NULL) *getResponse = *localResponse;
    return cond;
}

/* SRV_NGetResponse
**
** Purpose:
**	SRV_NGetResponse assists an application that wants to be an SCP
**	of a number of SOP classes. When an application receives an N-GET
**	request message, it calls this function with the N-GET request
**	and other parameters. SRV_NGetResponse checks the caller's
**	parameters and calls the user's callback function. In the callback
**	function, the caller fills in the parameters of the N-GET response
**	message and then returns to the SRV function.
**
**	After the callback function returns, SRV_NGetResponse constructs a
**	N-GET Response message and sends it to the peer application which
**	sent the request message.
**
**	The arguments to the callback function are:
**		MSG_N_GET_REQ	*getRequest
**		MSG_N_GET_RESP	*getResponse
**		void		*getCtx
**
**	The first two arguments are MSG structures that contain the N-GET
**	Request and N-GET Response messages respectively. The final
**	argument is the caller's context variable that is passed to
**	SRV_NGetResponse.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used for transmitting the N-GET
**			response.
**	presentationCtx	Pointer to presentation context to be used when sending
**			the N-GET response.
**	getRequest	Pointer to the structure with the N-GET request
**			parameters which was received by the application.
**	getResponse	Pointer to structure in the caller's area which will be
**			filled with the parameters of the N-GET response
**			command. After the parameters are filled in, the
**			N-GET response is sent to the peer application
**			which sent the request.
**	getCallback	Address of user callback function to be called
**			with the N-GET response from SCP.
**	getCtx		Pointer to user context information which will be
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
**	following fields in the N-GET response message:
**
**	type
**	messageIDRespondedTo
**	classUID
**	dataSetType
**	dataSet
**	affectedInstanceUID
**	status
**
*/
CONDITION
SRV_NGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_GET_REQ ** getRequest, MSG_N_GET_RESP * getResponse,
				 SRV_N_GET_RESP_CALLBACK * getCallback, void *getCtx, char *dirName)
{
    CONDITION		rtnCond = SRV_NORMAL, cond;
    DCM_OBJECT		* responseObject = NULL;

    getResponse->status = MSG_K_SUCCESS;	/* initialize */
    getResponse->dataSetType = DCM_CMDDATANULL;
    getResponse->dataSet = NULL;

    if (getCallback == NULL) {
    	rtnCond = COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NGetResponse");
    	getResponse->status = MSG_K_RESOURCELIMITATION;
    	goto SEND_RESPONSE;
    }
    /* The SRV routine allocates space for the data set */
    cond = DCM_CreateObject(&getResponse->dataSet, 0);
    if (cond != DCM_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "NGet Response Data Set", "SRV_NGetResponse");
    	getResponse->status = MSG_K_PROCESSINGFAILURE;
    	goto SEND_RESPONSE;
    }
    cond = getCallback(*getRequest, getResponse, getCtx, presentationCtx);
    if (cond != SRV_NORMAL) {
    	rtnCond = COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NGetResponse");
    	goto SEND_RESPONSE;
    }
    if (getResponse->type != MSG_K_N_GET_RESP) {
    	rtnCond = COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-GET Response", "SRV_NGetResponse");
    	goto SEND_RESPONSE;
    }
SEND_RESPONSE:
    /* At this point we free the request pointer */
    (void) MSG_Free((void **) getRequest);

    /*
     * If there are any errors in the following steps, it is necessary for
     * the SRV routine to free the data set object it had created earlier.
     * But we must verify that such a data set creation operation itself was
     * successful before we try to free the data set
     */

    /* The attribute list (data set) is sent back only on success status */
    if (getResponse->status != MSG_K_SUCCESS) {
    	getResponse->dataSetType = DCM_CMDDATANULL;
    	if (getResponse->dataSet) (void) DCM_CloseObject(&getResponse->dataSet);
    	return rtnCond;
    }
    /* now build a command object from the response message */
    cond = MSG_BuildCommand(getResponse, &responseObject);
    if (cond != MSG_NORMAL) {
    	if (getResponse->dataSet) (void) DCM_CloseObject(&getResponse->dataSet);
    	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-GET Response", "SRV_NGetResponse");
    }
    cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    (void) DCM_CloseObject(&responseObject);
    if (cond != SRV_NORMAL){
    	if (getResponse->dataSet) (void) DCM_CloseObject(&getResponse->dataSet);
    	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "Command", "SRV_NGetResponse");
    }
    /*
     * At this point, the response message status is "SUCCESS". This also
     * implies that the "data set" object exists but may be empty (not NULL)
     * Hence if there is any error, we need to close it first before we
     * return.
     */

    if (getResponse->dataSetType == DCM_CMDDATANULL) {
    	/* response message must have an attribute list */
    	(void) DCM_CloseObject(&getResponse->dataSet);
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "N-GET Response", "SRV_NGetResponse");
    }
    cond = SRV_SendDataSet(association, presentationCtx, &getResponse->dataSet, NULL, NULL, 0);
    (void) DCM_CloseObject(&getResponse->dataSet);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "DataSet", "SRV_NGetResponse");


    return SRV_NORMAL;		/* at this point everything is successful */
}
