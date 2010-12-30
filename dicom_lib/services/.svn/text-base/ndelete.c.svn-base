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
** Module Name(s):	SRV_NDeleteRequest
**			SRV_NDeleteResponse
** Author, Date:	Aniruddha S. Gokhale, July 1, 1993
** Intent:a		This module contains routines which implement
**			the DIMSE N-DELETE service class.
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:08 $
** Source File:		$RCSfile: ndelete.c,v $
** Revision:		$Revision: 1.20 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.20 $ $RCSfile: ndelete.c,v $";

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

/* SRV_NDeleteRequest
**
** Purpose:
**	SRV_NDeleteRequest assists an application that wants to be an SCU of
**	a number of SOP classes. This function constructs an N-DELETE-REQ
**	message and sends it to the peer application which is acting as the
**	SCP for a SOP class. This function waits for the response from the
**	peer application and invokes the caller's callback function.
**
**	The arguments to the callback function are:
**		MSG_N_DELETE_REQ	*deleteRequest
**		MSG_N_DELETE_RESP	*deleteResponse
**		void			*deleteCtx
**
**	The first two arguments are MSG structures that contain the N-DELETE
**	Request and N-DELETE Response messages respectively. The final
**	argument is the caller's context variable that is passed to
**	SRV_NDeleteRequest.
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
**			not be the same as the class UID in the N-DELETE
**			request message.
**	deleteRequest	Pointer to the structure with the N-DELETE request
**			parameters.
**	deleteResponse	Pointer to caller's pointer to an N-DELETE response.
**			This function will allocate an MSG_N_DELETE_RESP
**			message and return the pointer to the caller.
**	deleteCallback	Address of user callback function to be called
**			with the N-DELETE response from SCP.
**	deleteCtx	Pointer to user context information which will be
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
**	Encode the N-DELETE request message as a command object and
**		send it to an SCP
**	Send data set, if one exists, to the SCP.
**	Wait for a response message to arrive from the SCP
**	Receive a data set, if one exists, from the SCP
**	Return address of Response message structure to caller.
**
** Notes:
**	The caller is responsible for explicitly setting the following
**	fields in the N-DELETE request message:
**
**	type
**	messageID
**	classUID
**	dataSetType
**	instanceUID
**
**	The caller is also responsible for releasing the Response message
**	structure after the SRV_NDeleteRequest function returns,
**	using MSG_Free.
*/
CONDITION
SRV_NDeleteRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_DELETE_REQ * deleteRequest,
				   MSG_N_DELETE_RESP * deleteResponse, SRV_N_DELETE_REQ_CALLBACK * deleteCallback, void *deleteCtx, char *dirName)
{
    DCM_OBJECT					* commandObject;	/* Handle for a command object */
    CONDITION					cond;				/* Return value from function calls */
    DUL_PRESENTATIONCONTEXT		* presentationCtx;	/* Presentation context for this service */
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void       					*message;
    MSG_TYPE					messageType;
    MSG_N_DELETE_RESP			* localResponse;
    unsigned short        		command;

    /*
     * Since there is no data set field in the Request message, the data set
     * type field must be set to 0101H
     */
    deleteRequest->dataSetType = DCM_CMDDATANULL;

    if (deleteCallback == NULL)	return COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NDeleteRequest");

    presentationCtx = SRVPRV_PresentationContext(params, SOPClass);
    if (presentationCtx == NULL) return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), deleteRequest->classUID, "SRV_NDeleteRequest");
    if (deleteRequest->type != MSG_K_N_DELETE_REQ)	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-DELETE Request", "SRV_NDeleteRequest");

    cond = MSG_BuildCommand(deleteRequest, &commandObject);
    if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-DELETE Request", "SRV_NDeleteRequest");

    cond = SRV_SendCommand(association, presentationCtx, &commandObject);
    (void) DCM_CloseObject(&commandObject);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NDeleteRequest");

    cond = SRV_ReceiveCommand(association, params, DUL_BLOCK, 0, &ctxID, &command, &messageType, &message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_REQUESTFAILED, SRV_Message(SRV_REQUESTFAILED), "SRV_NDeleteRequest");

    if (messageType != MSG_K_N_DELETE_RESP)	return COND_PushCondition(SRV_UNEXPECTEDCOMMAND, SRV_Message(SRV_UNEXPECTEDCOMMAND), (int) command, "SRV_NDeleteRequest");

    localResponse = (MSG_N_DELETE_RESP *) message;
    if (localResponse->dataSetType != DCM_CMDDATANULL)	/* No attribute list exists for a delete response */
    	return COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "N-DELETE Response", "SRV_NDeleteRequest");

    if (deleteResponse != NULL)	*deleteResponse = *localResponse;

    cond = deleteCallback(deleteRequest, localResponse, deleteCtx);
    (void) MSG_Free(&message);
    if (cond != SRV_NORMAL)	return COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NDeleteRequest");

    return SRV_NORMAL;
}

/* SRV_NDeleteResponse
**
** Purpose:
**	SRV_NDeleteResponse assists an application that wants to be an SCP
**	of a number of SOP classes. When an application receives an N-DELETE
**	request message, it calls this function with the N-DELETE request
**	and other parameters. SRV_NDeleteResponse checks the caller's
**	parameters and calls the user's callback function. In the callback
**	function, the caller fills in the parameters of the N-DELETE response
**	message and then returns to the SRV function.
**
**	After the callback function returns, SRV_NDeleteResponse constructs a
**	N-DELETE Response message and sends it to the peer application which
**	sent the request message.
**
**	The arguments to the callback function are:
**		MSG_N_DELETE_REQ	*deleteRequest
**		MSG_N_DELETE_RESP	*deleteResponse
**		void			*deleteCtx
**		DUL_PRESENTATIONCONTEXT	*pc
**
**	The first two arguments are MSG structures that contain the N-DELETE
**	Request and N-DELETE Response messages respectively. The third
**	argument is the caller's context variable that is passed to
**	SRV_NDeleteResponse.  The presentation context describes the SOP
**	class that was negotiated for this message.
**
**	The callback function should return SRV_NORMAL. Any other return
**	value will cause the SRV facility to abort the Association.
**
** Parameter Dictionary:
**	association	The key which is used for transmitting the N-DELETE
**			response.
**	presentationCtx	Pointer to presentation context to be used when sending
**			the N-DELETE response.
**	deleteRequest	Pointer to the structure with the N-DELETE request
**			parameters which was received by the application.
**	deleteResponse	Pointer to structure in the caller's area which will be
**			filled with the parameters of the N-DELETE response
**			command. After the parameters are filled in, the
**			N-DELETE response is sent to the peer application
**			which sent the request.
**	deleteCallback	Address of user callback function to be called
**			with the N-DELETE response from SCP.
**	deleteCtx	Pointer to user context information which will be
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
**	following fields in the N-DELETE response message:
**
**	type
**	messageIDRespondedTo
**	classUID
**	dataSetType
**	instanceUID
**
*/
CONDITION
SRV_NDeleteResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_DELETE_REQ ** deleteRequest, MSG_N_DELETE_RESP * deleteResponse,
				    SRV_N_DELETE_RESP_CALLBACK * deleteCallback, void *deleteCtx, char *dirName)
{
    CONDITION	rtnCond = SRV_NORMAL, cond;
    DCM_OBJECT	* responseObject = NULL;

    /*
     * Since no data set exists in the response message, the data set type
     * field must be set to 0101H
     */
    deleteResponse->status = MSG_K_SUCCESS;
    deleteResponse->dataSetType = DCM_CMDDATANULL;

    if (deleteCallback == NULL){
    	rtnCond = COND_PushCondition(SRV_NOCALLBACK, SRV_Message(SRV_NOCALLBACK), "SRV_NDeleteResponse");
    	deleteResponse->status = MSG_K_RESOURCELIMITATION;
    	goto SEND_RESPONSE;
    }
    if ((*deleteRequest)->dataSetType != DCM_CMDDATANULL){	/* there cannot be any attribute list in the N-DELETE request mesg */
    	rtnCond = COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "dataSetType", "N-DELETE Request", "SRV_NDeleteResponse");
    	deleteResponse->status = MSG_K_PROCESSINGFAILURE;
    	goto SEND_RESPONSE;
    }
    cond = deleteCallback(*deleteRequest, deleteResponse, deleteCtx, presentationCtx);
    if (cond != SRV_NORMAL){
    	rtnCond = COND_PushCondition(SRV_CALLBACKABORTEDSERVICE, SRV_Message(SRV_CALLBACKABORTEDSERVICE), "SRV_NDeleteRequest");
    	goto SEND_RESPONSE;
    }
    if (deleteResponse->type != MSG_K_N_DELETE_RESP) {
    	rtnCond = COND_PushCondition(SRV_ILLEGALPARAMETER, SRV_Message(SRV_ILLEGALPARAMETER), "type", "N-DELETE Response", "SRV_NDeleteResponse");
    	goto SEND_RESPONSE;
    }
SEND_RESPONSE:
    /* At this point we free the request pointer */
    (void) MSG_Free((void **) deleteRequest);

    /* build a command object to be sent back */
    cond = MSG_BuildCommand(deleteResponse, &responseObject);
    if (cond != MSG_NORMAL) return COND_PushCondition(SRV_OBJECTBUILDFAILED, SRV_Message(SRV_OBJECTBUILDFAILED), "N-DELETE Response", "SRV_NDeleteResponse");

    cond = SRV_SendCommand(association, presentationCtx, &responseObject);
    (void) DCM_CloseObject(&responseObject);
    if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RESPONSEFAILED, SRV_Message(SRV_RESPONSEFAILED), "SRV_NDeleteResponse");

    return rtnCond;		/* return whatever condition was held in rtnCond */
}
