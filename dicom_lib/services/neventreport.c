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
** Module Name(s):	SRV_NEventReportRequest
**			SRV_NEventReportResponse
**
** Author, Date:	Stephen M. Moore, 16-Jun-93
** Intent:		This module contains routines which implement the
**			N Event Reportservice class (user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:09 $
** Source File:		$RCSfile: neventreport.c,v $
** Revision:		$Revision: 1.26 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.26 $ $RCSfile: neventreport.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#ifndef WIN32
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

/* SRV_NEventReportRequest
**
** Purpose:
**	SRV_NEventReportRequest assists an application that wants to be an SCP
**	of a number of SOP classes. This function constructs an N-EVENT_REPORT
**	request message and sends it to the peer application which is acting as
**	SCU for a SOP class. This function waits for the response from the
**	peer application and invokes the caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_N_EVENT_REPORT_REQ	*eventRequest
**		MSG_N_EVENT_REPORT_RESP	*eventResponse
**		void			*eventCtx
**
**	The first two arguments are MSG structures that contain the
**	N-EVENT_REPORT Request and N-EVENT_REPORT Response messages
**	respectively. The final argument is the caller's context variable
**	that is passed to SRV_NEventReportRequest.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association. This
**			list includes the list of presentation contexts.
**	eventRequest	Pointer to the structure with the N-EVENT_REPORT request
**			parameters.
**	eventResponse	Pointer to caller's pointer to an N-EVENT_REPORT
**			response. This function will allocate an
**			MSG_N_EVENT_REPORT_RESP	message and return the
**			pointer to the caller.
**	eventCallback	Address of user callback function to be called
**			with the N-EVENT_REPORT response from SCU.
**	eventCtx	Pointer to user context information which will be
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
**	Encode the N-EVENT_REPORT request message as a command object and
**		send it to an SCU
**	Send data set, if one exists, to the SCU.
**	Wait for a response message to arrive from the SCU
**	Receive a data set, if one exists, from the SCU
**	Return address of Response message structure to caller.
**
** Notes:
**	The caller is responsible for explicitly setting the following
**	fields in the N-EVENT_REPORT request message:
**
**	type
**	messageID
**	classUID
**	dataSetType
**	affectedInstanceUID
**	eventTypeID
**
**	The caller is also responsible for releasing the Response message
**	structure after the SRV_NEventReportRequest function returns,
**	using MSG_Free.
*/
CONDITION
SRV_NEventReportRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,	MSG_N_EVENT_REPORT_REQ * eventRequest,
						MSG_N_EVENT_REPORT_RESP * eventResponse, SRV_N_EVENTREPORT_REQ_CALLBACK * eventCallback,	void *eventCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void       					*message;
    MSG_TYPE					messageType;
    MSG_N_EVENT_REPORT_RESP		* localResponse;
    unsigned short        		command;

    if (eventCallback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NEventReportRequest");

    presentationCtx = SRVPRV_PresentationContext(params, eventRequest->classUID);
    if (presentationCtx == NULL)
    	return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), eventRequest->classUID, "SRV_NEventReportRequest");
    if (eventRequest->type != MSG_K_N_EVENT_REPORT_REQ)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-EVENT_REPORT Request", "SRV_NEventReportRequest");

    cond = MSG_BuildCommand(eventRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-EVENT_REPORT Request", "SRV_NEventReportRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NEventReportRequest");

    if (eventRequest->dataSetType != DCM_CMDDATANULL){	/* application specific event information exists */
    	cond = SRV_SendDataSet(association, presentationCtx, &eventRequest->dataSet, NULL, NULL, 0);
    	if (cond != SRV_NORMAL) return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NEventReportRequest");
    }

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NEventReportRequest");

    if (messageType != MSG_K_N_EVENT_REPORT_RESP) {
    	(void) MSG_Free(&message);
    	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_NEventReportRequest");
    }
    localResponse = (MSG_N_EVENT_REPORT_RESP *) message;
    if (localResponse->dataSetType == DCM_CMDDATANULL) {
    	localResponse->dataSet = NULL;
    }else{
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &localResponse->dataSet);
    	if (cond != SRV_NORMAL) {
    		(void) MSG_Free(&message);
    		return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NEventReportRequest");
    	}
    }
    cond = eventCallback(eventRequest, eventResponse, eventCtx);
    (void) MSG_Free(&message);
    if (localResponse->dataSet != NULL) {
    	(void) DCM_CloseObject(&localResponse->dataSet);
    	localResponse->dataSet = NULL;
    }
    if (eventResponse != NULL) *eventResponse = *localResponse;

    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NEventReportRequest");

    return SRV_NORMAL;
}

/*
** SRV_NEventReportResponse
**
** Purpose:
**	SRV_NEventReportResponse assists an application that wants to be an SCU
**	of a number of SOP classes. When an application receives an
**	N-EVENT_REPORT request message, it calls this function with the
**	N-EVENT_REPORT request and other parameters. SRV_NEventReportResponse
**	checks the caller's parameters and calls the user's callback function.
**	In the callback	function, the caller fills in the parameters of the
**	N-EVENT_REPORT response	message and then returns to the SRV function.
**
**	After the callback function returns, SRV_NEventReportResponse
**	constructs a N-EVENT_REPORT Response message and sends it to the peer
**	application which sent the request message.
**
**	The arguments to the callback function are:
**		MSG_N_EVENT_REPORT_REQ	*eventRequest
**		MSG_N_EVENT_REPORT_RESP	*eventResponse
**		void			*eventCtx
**		DUL_PRESENTATIONCONTEXT	*pc
**
**	The first two arguments are MSG structures that contain the
**	N-EVENT_REPORT Request and N-EVENT_REPORT Response messages
**	respectively. The third argument is the caller's context variable that
**	is passed to SRV_NEventReportResponse.  The presentation context is
**	the one negotiated for this SOP class.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used for transmitting the
**			N-EVENT_REPORT response.
**	presentationCtx	Pointer to presentation context to be used when sending
**			the N-EVENT_REPORT response.
**	eventRequest	Pointer to the structure with the N-EVENT_REPORT request
**			parameters which was received by the application.
**	eventResponse	Pointer to structure in the caller's area which will be
**			filled with the parameters of the N-EVENT_REPORT
**			response command. After the parameters are filled in,
**			the N-EVENT_REPORT response is sent to the peer
**			application which sent the request.
**	eventCallback	Address of user callback function to be called
**			with the N-EVENT_REPORT response from SCU.
**	eventCtx	Pointer to user context information which will be
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
**	following fields in the N-EVENT_REPORT request message:
**
**	type
**	messageIDRespondedTo
**	classUID
**	dataSetType
**	instanceUID
**	status
**
*/
CONDITION
SRV_NEventReportResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_EVENT_REPORT_REQ ** eventRequest,
						 MSG_N_EVENT_REPORT_RESP * eventResponse, SRV_N_EVENTREPORT_RESP_CALLBACK * eventCallback, void *eventCtx, char *dirName)
{
    CONDITION	cond;
    DCM_OBJECT	* responseObject;

    eventResponse->status = MSG_K_SUCCESS;
    if (eventCallback == NULL) {
    	(void) MSG_Free((void **) eventRequest);
    	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NEventReportResponse");
    }
    if ((*eventRequest)->dataSetType != DCM_CMDDATANULL){
    	cond = SRV_ReceiveDataSet(association, presentationCtx, DUL_BLOCK, 0, dirName, &(*eventRequest)->dataSet);
    	if (cond != SRV_NORMAL){
    		(void) MSG_Free((void **) eventRequest);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NEventReportResponse");
    	}
    }
    eventResponse->dataSetType = DCM_CMDDATANULL;

    cond = DCM_CreateObject(&eventResponse->dataSet, 0);
    if (cond != DCM_NORMAL) {
    	(void) MSG_Free((void **) eventRequest);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NEventReportResponse");
    }

    cond = eventCallback(*eventRequest, eventResponse, eventCtx, presentationCtx);
    (void) MSG_Free((void **) eventRequest);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NEventReportRequest");

    if (eventResponse->type != MSG_K_N_EVENT_REPORT_RESP)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-EVENT_REPORT Response", "SRV_NEventReportResponse");

    cond = MSG_BuildCommand(eventResponse, &responseObject);
    if (cond != MSG_NORMAL) {
    	(void) DCM_CloseObject(&eventResponse->dataSet);
    	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-EVENT_REPORT Response", "SRV_NEventReportResponse");
    }

    cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    (void) DCM_CloseObject(&responseObject);
    if (cond != SRV_NORMAL) {
    	(void) DCM_CloseObject(&eventResponse->dataSet);
    	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NEventReportResponse");
    }
    if (eventResponse->dataSetType != DCM_CMDDATANULL){ /* application specific event reply exists */
    	cond = SRV_SendDataSet(association, presentationCtx, &eventResponse->dataSet, NULL, NULL, 0);
    	if (cond != SRV_NORMAL){
    		(void) DCM_CloseObject(&eventResponse->dataSet);
    		return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NEventReportResponse");
    	}
    }
    (void) DCM_CloseObject(&eventResponse->dataSet);
    return SRV_NORMAL;
}
