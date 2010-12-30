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
** Module Name(s):	SRV_VerificationRequest
**			SRV_VerificationResponse
** Author, Date:	Stephen M. Moore, 19-Apr-93
** Intent:		This module contains routines which implement the
**			verification service class (both user and provider).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:11 $
** Source File:		$RCSfile: verify.c,v $
** Revision:		$Revision: 1.23 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.23 $ $RCSfile: verify.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
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


/* SRV_CEchoRequest
**
** Purpose:
**	SRV_CEchoRequest assists an application that wants to be an SCU of
**	the Verification SOP class.  This function constructs a C-ECHO-REQ
** 	Message and sends it to the peer application which is
**	acting as the SCP for the Verification class.  This function
**	waits for the response from the peer application and invokes the
**	caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_C_ECHO_REQUEST	*echoRequest
**		MSG_C_ECHO_RESP		*echoResponse
**		void			*ctx
**
**	The first two arguments are MSG structures that contain the
**	C-ECHO Request and C-ECHO Response messages.  The final argument
**	is the caller's ctx variable that is passed to SRV_CEchoRequest.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used to access an existing
**			association.
**	params		The list of parameters for the association.  This
**			list includes the list of presentation contexts;
**	echoRequest	Pointer to the structure with the echo request
**			parameters.
**	echoResponse	Pointer to caller's pointer to an echo response.
**			This function will allocate an MSG_C_ECHO_RESP
**			message and return the pointer to the caller.
**	callback	Address of user callback function to be called
**			with ECHO Response from SCP.
**	ctx		Pointer to user context information which will be
**			passed to the callback function.  Caller uses this
**			variable to contain any context required for callback
**			function.
**	dirName		Name of directory where files may be created.
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
** Notes:
**	SRV_EchoRequest allocates an MSG structure with the ECHO Response
**	and returns the address of that structure to the caller.  After the
**	verification is complete, the caller should release the structure
**	with MSG_Free.
*/

CONDITION
SRV_CEchoRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_ECHO_REQ * echoRequest, MSG_C_ECHO_RESP * echoResponse,
				 SRV_C_ECHO_REQ_CALLBACK *callback, void *ctx, char *dirName)
{
    DCM_OBJECT					* object = NULL;	/* Handle for an encoded object */
    CONDITION					cond;			/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void				        *message;
    MSG_TYPE					messageType;
    unsigned short		        command;
    MSG_C_ECHO_RESP				* response;

    if (callback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CEchoRequest");

    presentationCtx = SRVPRV_PresentationContext(params, DICOM_SOPCLASSVERIFICATION);
    if (presentationCtx == NULL)
    	return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), "Verification", "SRV_CEchoRequest");

    if (echoRequest->type != MSG_K_C_ECHO_REQ)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "ECHO Request", "SRV_CEchoRequest");
    if (echoRequest->dataSetType != DCM_CMDDATANULL)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "ECHO Request", "SRV_CEchoRequest");

    cond = MSG_BuildCommand(echoRequest, &object);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "ECHO Request", "SRV_CEchoRequest");

    cond = SRV_SendCommand(association, presentationCtx, &object);
    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CEchoRequest");

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_CEchoRequest");

    if (messageType != MSG_K_C_ECHO_RESP) {
    	(void) MSG_Free(&message);
    	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_CEchoRequest");
    }
    response = (MSG_C_ECHO_RESP *) message;
    if (echoResponse != NULL) *echoResponse = *response;

    cond = callback(echoRequest, response, ctx);
    (void) MSG_Free(&message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CEchoRequest");

    return SRV_NORMAL;
}


/* SRV_CEchoResponse
**
** Purpose:
**	SRV_CEchoResponse assists an application that wants to be an SCP of
**	the Verification SOP class.  When an application receives an
**	ECHO Request message, it calls this function with the ECHO
**	request and other parameters.  SRV_CEchoResponse checks the caller's
**	parameters and invokes the user's callback function.  In the
**	callback function, the caller fills in the parameters of the
**	ECHO Response message and then returns to the SRV function.
**
**	After the callback function returns, SRV_CEchoReponse constructs a
**	C-ECHO-RESPONSE message and sends it to the peer application which
**	sent the request message.
**
**	The arguments to the callback function are:
**		MSG_C_ECHO_REQUEST	*echoRequest
**		MSG_C_ECHO_RESP		*echoResponse
**		void			*ctx
**		DUL_PRESENTATIONCONTEXT	*pc
**
**	The first two arguments are MSG structures that contain the
**	C-ECHO Request and C-ECHO Response messages.  The third argument
**	is the caller's ctx variable that is passed to SRV_CEchoResponse.
**	The presentation context pointer gives the callback function a
**	reference to the presentation context for this class.
**
**	The callback function should return SRV_NORMAL.  Any other return
**	value will cause the SRV facility to abort the Association.
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	association	Key which describes the Association used for
**			transmitting the ECHO response.
**	presentationCtx	Pointer to presentation context to be used when
**			sending the ECHO response.
**	echoRequest	Pointer to structure containing the parameters to
**			the ECHO request which was received by the application.
**	echoReply	Pointer to structure in the caller's area which will
**			be filled with the parameters of the ECHO response
**			command.  After the parameters are filled in, the
**			ECHO response is sent to the peer application which
**			sent the request.
**	callback	Address of user callback function to be called
**			with ECHO Response from SCP.
**	ctx		Pointer to user context information which will be
**			passed to the callback function.  Caller uses this
**			variable to contain any context required for callback
**      dirName         Name of directory where files may be created.
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
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_CEchoResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_ECHO_REQ ** echoRequest,
				  MSG_C_ECHO_RESP * echoResponse, SRV_C_ECHO_RESP_CALLBACK* callback, void *ctx, char *dirName)
{
    DCM_OBJECT		* object = NULL;	/* Handle for an encoded object */
    CONDITION		cond;			/* Return value from function calls */

    if (callback == NULL) return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_CEchoResponse");
    if (echoResponse->type != MSG_K_C_ECHO_RESP)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "ECHO Response", "SRV_CEchoResponse");

    cond = callback(*echoRequest, echoResponse, ctx, presentationCtx);
    (void) strcpy(echoResponse->classUID, (*echoRequest)->classUID);
    (void) MSG_Free((void **) echoRequest);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_CEchoRequest");

    if (echoResponse->dataSetType != DCM_CMDDATANULL)
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "ECHO Response", "SRV_CEchoResponse");

    echoResponse->conditionalFields = MSG_K_C_ECHORESP_CLASSUID;

    cond = MSG_BuildCommand(echoResponse, &object);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "ECHO Response", "SRV_CEchoResponse");

    cond = SRV_SendCommand(association, presentationCtx, &object);
    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_CEchoResponse");

    return SRV_NORMAL;
}
