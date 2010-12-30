/*
+-+-+-+-+-+-+-+-+-
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
** Author, Date:	Stephen M. Moore, 12-Jun-93
** Intent:		This module contains parse functions for the COMMAND
**			objects for the messages in DIMSE-N Services.  The
**			functions extract data values from DICOM objects and
**			place them in fixed structures.
** Intent:
** Last Update:		$Author: smm $, $Date: 2000/01/20 17:24:39 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/messages/nparse.c,v $
** Revision:		$Revision: 1.9 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.9 $ $RCSfile: nparse.c,v $";

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"

/* MSG_ParseNEventReportRequest
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
MSG_ParseNEventReportRequest(DCM_OBJECT ** object, MSG_N_EVENT_REPORT_REQ * eventReportRequest)
{
    CONDITION						cond;
    int						        index;
    static MSG_N_EVENT_REPORT_REQ   request;
    static unsigned short	        command;
    unsigned long			        l;
    void						    *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.affectedInstanceUID), (void *) request.affectedInstanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDEVENTTYPEID, DCM_UI, "", 1, sizeof(request.eventTypeID), (void *) &request.eventTypeID},
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;

    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *eventReportRequest = request;

    return MSG_NORMAL;
}


/* MSG_ParseNEventReportResponse
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
MSG_ParseNEventReportResponse(DCM_OBJECT ** object, MSG_N_EVENT_REPORT_RESP * eventReportResp)
{
    CONDITION							cond;
    int							        index;
    static MSG_N_EVENT_REPORT_RESP      response;
    static unsigned short		        command;
    unsigned long				        l;
    void							    *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
#if 0
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.affectedInstanceUID), (void *) response.affectedInstanceUID},
#endif
    		{DCM_GROUPCOMMAND, DCM_CMDEVENTTYPEID, DCM_UI, "", 1, sizeof(response.eventTypeID), (void *) &response.eventTypeID},
    };

    response.status = DCM_STATUS_SUCCESS;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;

    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *eventReportResp = response;
    return MSG_NORMAL;
}

/* MSG_ParseNGetRequest
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
MSG_ParseNGetRequest(DCM_OBJECT ** object, MSG_N_GET_REQ * getRequest)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_GET_REQ	request;
    static unsigned short   command;
    unsigned long	        l;
    void				    *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.requestedInstanceUID), (void *) request.requestedInstanceUID}
    };

    getRequest->attributeCount = 0;
    cond = MSGPRV_ParseObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    request.dataSetType = MSG_K_N_GET_REQ;
    *getRequest = request;
    return MSG_NORMAL;
}


/* MSG_ParseNGetResponse
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
MSG_ParseNGetResponse(DCM_OBJECT ** object, MSG_N_GET_RESP * getResp)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_GET_RESP   response;
    static unsigned short   command;
    unsigned long		    l;
    void				    *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.affectedInstanceUID), (void *) response.affectedInstanceUID},
    };

    response.status = DCM_STATUS_SUCCESS;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *getResp = response;
    return MSG_NORMAL;
}

/* MSG_ParseNSetRequest
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
MSG_ParseNSetRequest(DCM_OBJECT ** object, MSG_N_SET_REQ * getRequest)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_SET_REQ	request;
    static unsigned short   command;
    unsigned long	        l;
    void			        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) request.instanceUID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *getRequest = request;
    return MSG_NORMAL;
}


/* MSG_ParseNSetResponse
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
MSG_ParseNSetResponse(DCM_OBJECT ** object, MSG_N_SET_RESP * setResp)
{
    CONDITION 				cond;
    int				        index;
    static MSG_N_SET_RESP   response;
    static unsigned short   command;
    unsigned long	        l;
    void			        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.instanceUID), (void *) response.instanceUID}
    };

    response.status = DCM_STATUS_SUCCESS;

    response = *setResp;
    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *setResp = response;
    return MSG_NORMAL;
}

/* MSG_ParseNActionRequest
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
MSG_ParseNActionRequest(DCM_OBJECT ** object, MSG_N_ACTION_REQ * actionRequest)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_ACTION_REQ	    request;
    static unsigned short       command;
    unsigned long		        l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) request.instanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(request.actionTypeID), (void *) &request.actionTypeID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *actionRequest = request;
    return MSG_NORMAL;
}


/* MSG_ParseNActionResponse
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
MSG_ParseNActionResponse(DCM_OBJECT ** object, MSG_N_ACTION_RESP * actionResp)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_ACTION_RESP    response;
    static unsigned short       command;
    unsigned long		        l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.instanceUID), (void *) response.instanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(response.actionTypeID), (void *) &response.actionTypeID}
    };

    response.status = DCM_STATUS_SUCCESS;

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *actionResp = response;
    return MSG_NORMAL;
}

/* MSG_ParseNCreateRequest
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
MSG_ParseNCreateRequest(DCM_OBJECT ** object, MSG_N_CREATE_REQ * createRequest)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_CREATE_REQ     request;
    static unsigned short       command;
    unsigned long		        l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) request.instanceUID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *createRequest = request;
    return MSG_NORMAL;
}


/* MSG_ParseNCreateResponse
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
MSG_ParseNCreateResponse(DCM_OBJECT ** object, MSG_N_CREATE_RESP * createResp)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_CREATE_RESP    response;
    static unsigned short       command;
    unsigned long		        l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.instanceUID), (void *) response.instanceUID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *createResp = response;
    return MSG_NORMAL;
}

/* MSG_ParseNDeleteRequest
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
MSG_ParseNDeleteRequest(DCM_OBJECT ** object, MSG_N_DELETE_REQ * deleteRequest)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_DELETE_REQ     request;
    static unsigned short       command;
    unsigned long			    l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(request.dataSetType), (void *) &request.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) request.instanceUID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *deleteRequest = request;
    return MSG_NORMAL;
}


/* MSG_ParseNDeleteResponse
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
MSG_ParseNDeleteResponse(DCM_OBJECT ** object, MSG_N_DELETE_RESP * createResp)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_DELETE_RESP    response;
    static unsigned short       command;
    unsigned long		        l;
    void				        *ctx;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, sizeof(response.classUID), (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(response.instanceUID), (void *) response.instanceUID}
    };

    for (index = 0; index < (int) DIM_OF(elementList); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(object, &elementList[index], &l, &ctx);
    	if (cond != DCM_NORMAL) return COND_PushCondition(0, "");	/* repair */

    	if (elementList[index].representation == DCM_UI) elementList[index].d.string[l] = '\0';
    }
    *createResp = response;
    return MSG_NORMAL;
}
