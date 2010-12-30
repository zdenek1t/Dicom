/*
+-+-+-+-+-+-+-+-+-
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	MSG_ParseCEchoRequest
**			MSG_ParseCEchoResponse
**			MSG_ParseCFindRequest
**			MSG_ParseCFindResponse
**			MSG_ParseCMoveRequest
**			MSG_ParseCMoveResponse
**			MSG_ParseCStoreRequest
**			MSG_ParseCStoreResponse
** Author, Date:	Stephen M. Moore, 27-Apr-93
** Intent:		This module contains functions which parse the
**			COMMAND part of a message for the DIMSE-C services.
**			The routines parse a DICOM object and place the
**			extracted data values into fixed structures.
** Last Update:		$Author: smm $, $Date: 1993/06/17 03:01:28 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/messages/cparse.c,v $
** Revision:		$Revision: 1.7 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.7 $ $RCSfile: cparse.c,v $";

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

/* MSG_ParseCEchoRequest
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
MSG_ParseCEchoRequest(DCM_OBJECT ** object, MSG_C_ECHO_REQ * echoRequest)
{
    static MSG_C_ECHO_REQ        request;
    static unsigned short        command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.UID), (void *) request.UID}
    };
    void	        *ctx;
    int		        index;
    unsigned long   rtnLength;
    CONDITION		cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    request.type = MSG_K_C_ECHO_REQ;
    *echoRequest = request;
    return MSG_NORMAL;
}

/* MSG_ParseCEchoResponse
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
MSG_ParseCEchoResponse(DCM_OBJECT ** object, MSG_C_ECHO_RESP * echoResponse)
{
    static MSG_C_ECHO_RESP	   response;
    static unsigned short      command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.UID), (void *) response.UID}
    };
    void	        *ctx;
    int		        index;
    unsigned long   rtnLength;
    CONDITION		cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }

    if (response.status != DCM_STATUS_SUCCESS) return COND_PushCondition(1, "");	/* repair */

    response.type = MSG_K_C_ECHO_RESP;
    *echoResponse = response;
    return MSG_NORMAL;
}


/* MSG_ParseCStoreRequest
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
MSG_ParseCStoreRequest(DCM_OBJECT ** object, MSG_C_STORE_REQ * storeRequest)
{
    static MSG_C_STORE_REQ	   request;
    static unsigned short      command;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(request.priority), (void *) &request.priority},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) request.instanceUID}
    };

    static MSGPRV_CONDITIONAL conditional[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDMOVEAETITLE, DCM_AE, "", 1, sizeof(request.moveAETitle), (void *) request.moveAETitle, &request.conditionalFields, MSG_K_C_STORE_MOVEAETITLE},
    		{DCM_GROUPCOMMAND, DCM_CMDMOVEMESSAGEID, DCM_US, "", 1, sizeof(request.moveMessageID), (void *) &request.moveMessageID,	&request.conditionalFields, MSG_K_C_STORE_MOVEMESSAGEID},
    };
    void	        *ctx;
    int		        index;
    unsigned long   rtnLength;
    CONDITION		cond;

    request.conditionalFields = 0;
    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }

    for (index = 0; index < (int) DIM_OF(conditional); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &conditional[index].e, &rtnLength, &ctx);
    	if (cond == DCM_NORMAL) {
    		if (DCM_IsString(conditional[index].e.representation)) conditional[index].e.d.string[rtnLength] = '\0';
    		*conditional[index].flag |= conditional[index].flagBit;
    	}else{
    		(void) COND_PopCondition(FALSE);
    	}
    }
    request.type = MSG_K_C_STORE_REQ;
    *storeRequest = request;
    return MSG_NORMAL;
}

/* MSG_ParseCStoreResponse
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
MSG_ParseCStoreResponse(DCM_OBJECT ** object, MSG_C_STORE_RESP * storeResponse)
{
    static MSG_C_STORE_RESP	        response;
    static unsigned short	        command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.instanceUID), (void *) response.instanceUID}
    };

    void		       *ctx;
    int        		   index;
    unsigned long      rtnLength;
    CONDITION		   cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    response.type = MSG_K_C_STORE_RESP;
    *storeResponse = response;
    return MSG_NORMAL;
}

/* MSG_ParseCFindRequest
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
MSG_ParseCFindRequest(DCM_OBJECT ** object, MSG_C_FIND_REQ * findRequest)
{
    static MSG_C_FIND_REQ        request;
    static unsigned short        command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(request.priority), (void *) &request.priority},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    };

    void       		 *ctx;
    int   		     index;
    unsigned long    rtnLength;
    CONDITION		 cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    request.type = MSG_K_C_FIND_REQ;
    *findRequest = request;
    return MSG_NORMAL;
}

/* MSG_ParseCFindResponse
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
MSG_ParseCFindResponse(DCM_OBJECT ** object, MSG_C_FIND_RESP * findResponse)
{
    static MSG_C_FIND_RESP       response;
    static unsigned short        command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    };

    void		    *ctx;
    int		        index;
    unsigned long   rtnLength;
    CONDITION 		cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    response.type = MSG_K_C_FIND_RESP;
    *findResponse = response;
    return MSG_NORMAL;
}

/* MSG_ParseCMoveRequest
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
MSG_ParseCMoveRequest(DCM_OBJECT ** object, MSG_C_MOVE_REQ * moveRequest)
{
    static MSG_C_MOVE_REQ       request;
    static unsigned short       command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(request.priority), (void *) &request.priority},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDMOVEDESTINATION, DCM_AE, "", 1, sizeof(request.moveDestination), (void *) request.moveDestination}
    };
    void	       *ctx;
    int     	   index;
    unsigned long  rtnLength;
    CONDITION	   cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    request.type = MSG_K_C_MOVE_REQ;
    *moveRequest = request;
    return MSG_NORMAL;
}

/* MSG_ParseCMoveResponse
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
MSG_ParseCMoveResponse(DCM_OBJECT ** object, MSG_C_MOVE_RESP * moveResponse)
{
    static MSG_C_MOVE_RESP	    response;
    static unsigned short       command, type;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    };

    DCM_ELEMENT		e;			/* Used for conditional elements */

    static MSGPRV_CONDITIONAL conditionalList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDSUCCESSINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.successUIDs, &response.conditionalFields, MSG_K_C_MOVE_SUCCESSUID},
    		{DCM_GROUPCOMMAND, DCM_CMDFAILEDINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.failedUIDs, &response.conditionalFields, MSG_K_C_MOVE_FAILEDUID},
    		{DCM_GROUPCOMMAND, DCM_CMDWARNINGINSTANCEUIDLIST, DCM_UI, "", 1, 0, (void *) &response.warningUIDs, &response.conditionalFields, MSG_K_C_MOVE_WARNINGUID},
    		{DCM_GROUPCOMMAND, DCM_CMDREMAININGSUBOPERATIONS, DCM_US, "", 1, sizeof(response.remainingSubOperations), (void *) &response.remainingSubOperations, &response.conditionalFields, MSG_K_C_MOVE_REMAINING},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMPLETEDSUBOPERATIONS, DCM_US, "", 1, sizeof(response.completedSubOperations), (void *) &response.completedSubOperations, &response.conditionalFields, MSG_K_C_MOVE_COMPLETED},
    		{DCM_GROUPCOMMAND, DCM_CMDFAILEDSUBOPERATIONS, DCM_US, "", 1, sizeof(response.failedSubOperations), (void *) &response.failedSubOperations, &response.conditionalFields, MSG_K_C_MOVE_FAILED},
    		{DCM_GROUPCOMMAND, DCM_CMDWARNINGSUBOPERATIONS, DCM_US, "", 1, sizeof(response.warningSubOperations), (void *) &response.warningSubOperations, &response.conditionalFields, MSG_K_C_MOVE_WARNING},
    };

    void		       *ctx;
    int			       index;
    unsigned long	   rtnLength;
    CONDITION		   cond;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &rtnLength, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED));
    }
    response.type = MSG_K_C_MOVE_RESP;

    for (index = 0; index < (int) DIM_OF(conditionalList); index++) {
    	cond = DCM_NORMAL;
    	e = conditionalList[index].e;
    	if (e.length == 0) {
    		cond = DCM_GetElementSize(object, DCM_MAKETAG(e.group, e.element), &rtnLength);
    		if (cond == DCM_NORMAL) {
    			e.length = rtnLength;
    			e.d.string = malloc(rtnLength + 1);
    			if (e.d.string == NULL) return 0;	/* repair */
    			*conditionalList[index].e.d.stringArray = e.d.string;
    		}else{
    			(void) COND_PopCondition(FALSE);
    		}
    	}
    	if (cond == DCM_NORMAL) {
    		ctx = NULL;
    		cond = DCM_GetElementValue(object, &e, &rtnLength, &ctx);
    		if (cond == DCM_NORMAL) {
    			*conditionalList[index].flag |= conditionalList[index].flagBit;
    			e.d.string[rtnLength] = '\0';
    		}
    	}
    }

    *moveResponse = response;
    return MSG_NORMAL;
}
