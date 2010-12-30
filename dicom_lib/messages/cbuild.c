/*
+-+-+-+-+-+-+-+-+-
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	MSG_BuildCEchoRequest
**			MSG_BuildCEchoResponse
**			MSG_BuildCFindRequest
**			MSG_BuildCFindResponse
**			MSG_BuildCMoveRequest
**			MSG_BuildCMoveResponse
**			MSG_BuildCStoreRequest
**			MSG_BuildCStoreResponse
** Author, Date:	Stephen M. Moore, 27-Apr-93
** Intent:		This module contains routines for building DICOM
**			objects which corresond to the COMMAND portion of
**			a message in the DIMSE-C services.  These routines
**			extract attributes from fixed structures and create
**			DICOM objects.
** Last Update:		$Author: smm $, $Date: 1993/06/18 17:05:30 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/messages/cbuild.c,v $
** Revision:		$Revision: 1.8 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.8 $ $RCSfile: cbuild.c,v $";

#include "../dicom/ctn_os.h"
/*
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
*/
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "msgprivate.h"


/* MSG_BuildCEchoRequest
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCEchoRequest(MSG_C_ECHO_REQ * echoRequest, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int		        			index;
    static MSG_C_ECHO_REQ	  	request;
    static unsigned short       command = DCM_ECHO_REQUEST, type = DCM_CMDDATANULL;
    static DCM_ELEMENT 			elementList[] = {
									{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
									{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
									{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
								};
    static DCM_ELEMENT 			uidElement = {DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.UID};

    cond = DCM_CreateObject(object,0);
    if (cond != DCM_NORMAL)	return COND_PushCondition(1, "");	/* repair */

    request = *echoRequest;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(1, "");	/* repair */
    }

    uidElement.length = strlen(request.classUID);
    cond = DCM_AddElement(object, &uidElement);
    if (cond != DCM_NORMAL) return COND_PushCondition(1, "");	/* repair */

    return MSG_NORMAL;
}


/* MSG_BuildCEchoResponse
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCEchoResponse(MSG_C_ECHO_RESP * echoResp, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_C_ECHO_RESP  response;
    static unsigned short	command = DCM_ECHO_RESPONSE, type = DCM_CMDDATANULL;
    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.UID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo}
    };

    response.status = DCM_STATUS_SUCCESS;

    cond = DCM_CreateObject(object,0);
    if (cond != DCM_NORMAL)	return COND_PushCondition(1, "");	/* repair */

    response = *echoResp;
    elementList[0].length = strlen(response.classUID);

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(1, "");	/* repair */
    }

    return MSG_NORMAL;
}


/* MSG_BuildCStoreRequest
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCStoreRequest(MSG_C_STORE_REQ * store, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_C_STORE_REQ  request;
    static unsigned short   command = DCM_STORE_REQUEST;
    static DCM_ELEMENT      e, elementList[] = {
									{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
									{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
									{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
									{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(request.priority), (void *) &request.priority},
									{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
									{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.instanceUID}
								};
    long 					flagBit;

    static MSGPRV_CONDITIONAL conditional[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDMOVEAETITLE, DCM_AE, "", 1, 0, (void *) request.moveAETitle, &request.conditionalFields, MSG_K_C_STORE_MOVEAETITLE},
    		{DCM_GROUPCOMMAND, DCM_CMDMOVEMESSAGEID, DCM_US, "", 1, sizeof(request.moveMessageID), (void *) &request.moveMessageID,	&request.conditionalFields, MSG_K_C_STORE_MOVEAETITLE},
    };

    if (strlen(store->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCStoreRequest");
    if (strlen(store->instanceUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHINSTANCEUID, MSG_Message(MSG_ZEROLENGTHINSTANCEUID), "MSG_BuildCStoreRequest");

    request = *store;

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(1, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	e = elementList[index];
    	if (e.length == 0) e.length = strlen(e.d.string);

    	cond = DCM_AddElement(object, &e);
    	if (cond != DCM_NORMAL) return COND_PushCondition(1, "");	/* repair */
    }
    for (index = 0; index < (int) DIM_OF(conditional); index++) {
    	if (*conditional[index].flag & conditional[index].flagBit) {
    		e = conditional[index].e;
    		if (e.length == 0) e.length = strlen(e.d.string);

    		cond = DCM_AddElement(object, &e);
    		if (cond != DCM_NORMAL) return COND_PushCondition(1, "");	/* repair */
    	}
    }

    return MSG_NORMAL;
}

/* MSG_BuildCStoreResponse
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCStoreResponse(MSG_C_STORE_RESP * store, DCM_OBJECT ** object)
{
    CONDITION				 cond;
    int						 index;
    static MSG_C_STORE_RESP	 response;
    static unsigned short    command = DCM_STORE_RESPONSE, type = DCM_CMDDATANULL;
    static DCM_ELEMENT 		elementList[] = {
								{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
								{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
								{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
								{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
								{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
								{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.instanceUID}
							};

    if (strlen(store->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCStoreResponse");
    if (strlen(store->instanceUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHINSTANCEUID, MSG_Message(MSG_ZEROLENGTHINSTANCEUID), "MSG_BuildCStoreReponse");

    response = *store;
    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	if (elementList[index].d.ot == response.classUID){
    		elementList[index].length = strlen(response.classUID);
    	}else if (elementList[index].d.ot == response.instanceUID){
    		elementList[index].length = strlen(response.instanceUID);
    	}
    }

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    }

    return MSG_NORMAL;
}

/* MSG_BuildCFindRequest
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCFindRequest(MSG_C_FIND_REQ * find, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_C_FIND_REQ   request;
    static unsigned short   command = DCM_FIND_REQUEST, type = DCM_CMDDATAIDENTIFIER;
    static DCM_ELEMENT 		elementList[] = {
								{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
								{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
								{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
								{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
								{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(request.priority), (void *) &request.priority}
							};

    if (strlen(find->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCFindRequest");

    request = *find;
    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	if (elementList[index].d.ot == request.classUID) elementList[index].length = strlen(request.classUID);
    }

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    }

    return MSG_NORMAL;
}

/* MSG_BuildCFindResponse
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCFindResponse(MSG_C_FIND_RESP * find, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_C_FIND_RESP	response;
    static unsigned short   command = DCM_FIND_RESPONSE, type = DCM_CMDDATAIDENTIFIER;
    static DCM_ELEMENT 		elementList[] = {
								{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
								{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
								{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
								{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
								{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status}
							};

    if (strlen(find->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCFindRequest");

    response = *find;
    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	if (elementList[index].d.ot == response.classUID)  elementList[index].length = strlen(response.classUID);
    }

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    }

    return MSG_NORMAL;
}

/* MSG_BuildCMoveRequest
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCMoveRequest(MSG_C_MOVE_REQ * move, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int		        		index;
    static MSG_C_MOVE_REQ   request;
    static unsigned short	command = DCM_MOVE_REQUEST, type = DCM_CMDDATAIDENTIFIER;
    static DCM_ELEMENT 		elementList[] = {
								{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
								{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
								{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
								{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
								{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, 0, (void *) &request.priority},
								{DCM_GROUPCOMMAND, DCM_CMDMOVEDESTINATION, DCM_AE, "", 1, 0, (void *) request.moveDestination},
							};

    if (strlen(move->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCMoveRequest");

    request = *move;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	if (elementList[index].d.ot == request.classUID){
    		elementList[index].length = strlen(request.classUID);
    	}else if (elementList[index].d.ot == request.moveDestination){
    		elementList[index].length = strlen(request.moveDestination);
    	}
    }

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    }

    return MSG_NORMAL;
}

/* MSG_BuildCMoveResponse
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCMoveResponse(MSG_C_MOVE_RESP * move, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_C_MOVE_RESP  response;
    static unsigned short   command = DCM_MOVE_RESPONSE, type = DCM_CMDDATANULL;
    DCM_ELEMENT				e;			/* Used for conditional elements */

    static DCM_ELEMENT elementList[] = {
			{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
			{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
			{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
			{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
			{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status}
    };
    static MSGPRV_CONDITIONAL conditionalList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDSUCCESSINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.successUIDs,	&response.conditionalFields, MSG_K_C_MOVE_SUCCESSUID},
    		{DCM_GROUPCOMMAND, DCM_CMDFAILEDINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.failedUIDs, &response.conditionalFields, MSG_K_C_MOVE_FAILEDUID},
    		{DCM_GROUPCOMMAND, DCM_CMDWARNINGINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.warningUIDs,	&response.conditionalFields, MSG_K_C_MOVE_WARNINGUID},
    		{DCM_GROUPCOMMAND, DCM_CMDREMAININGSUBOPERATIONS, DCM_US, "", 1, sizeof(response.remainingSubOperations), (void *) &response.remainingSubOperations, &response.conditionalFields, MSG_K_C_MOVE_REMAINING},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMPLETEDSUBOPERATIONS, DCM_US, "", 1, sizeof(response.completedSubOperations), (void *) &response.completedSubOperations, &response.conditionalFields, MSG_K_C_MOVE_COMPLETED},
    		{DCM_GROUPCOMMAND, DCM_CMDFAILEDSUBOPERATIONS, DCM_US, "", 1, sizeof(response.failedSubOperations), (void *) &response.failedSubOperations,	&response.conditionalFields, MSG_K_C_MOVE_FAILED},
    		{DCM_GROUPCOMMAND, DCM_CMDWARNINGSUBOPERATIONS, DCM_US, "", 1, sizeof(response.warningSubOperations), (void *) &response.warningSubOperations, &response.conditionalFields, MSG_K_C_MOVE_WARNING},
    };

    if (strlen(move->classUID) == 0)
    	return COND_PushCondition(MSG_ZEROLENGTHCLASSUID, MSG_Message(MSG_ZEROLENGTHCLASSUID), "MSG_BuildCMoveRequest");

    response = *move;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	if (elementList[index].d.ot == response.classUID) elementList[index].length = strlen(response.classUID);
    }

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	cond = DCM_AddElement(object, &elementList[index]);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    }

    for (index = 0; index < (int) DIM_OF(conditionalList); index++) {
    	if (*conditionalList[index].flag & conditionalList[index].flagBit) {
    		e = conditionalList[index].e;
    		if (e.length == 0) {
    			e.d.string = *e.d.stringArray;
    			e.length = strlen(e.d.string);
    		}
    		cond = DCM_AddElement(object, &e);
    		if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */
    	}
    }
    return MSG_NORMAL;
}
