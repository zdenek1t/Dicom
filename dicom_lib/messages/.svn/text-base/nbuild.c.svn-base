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
**			MSG_BuildNEventReportRequest
**			MSG_BuildNEventReportResponse
** Author, Date:	Stephen M. Moore, 8-Jun-93
** Intent:		This module contains functions for the COMMAND objects
**			for the messages in DIMSE-N Services.  The functions
**			extract data values from fixed structures and
**			create DICOM objects.
** Last Update:		$Author: smm $, $Date: 1993/06/17 03:18:21 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/messages/nbuild.c,v $
** Revision:		$Revision: 1.8 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.8 $ $RCSfile: nbuild.c,v $";

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

/* MSG_BuildNEventReportRequest
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
MSG_BuildNEventReportRequest(MSG_N_EVENT_REPORT_REQ * eventReportRequest, DCM_OBJECT ** object)
{
    CONDITION						cond;
    int		        				index;
    static MSG_N_EVENT_REPORT_REQ   request;
    static unsigned short			command = DCM_N_EVENT_REPORT_REQUEST, type = DCM_CMDDATAIDENTIFIER;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.affectedInstanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDEVENTTYPEID, DCM_UI, "", 1, sizeof(request.eventTypeID), (void *) &request.eventTypeID},
    };

    request = *eventReportRequest;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNEventReportResponse
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
MSG_BuildNEventReportResponse(MSG_N_EVENT_REPORT_RESP * eventReportResp, DCM_OBJECT ** object)
{
    CONDITION						   cond;
    int		        				   index;
    static MSG_N_EVENT_REPORT_RESP     response;
    static unsigned short		       command = DCM_N_EVENT_REPORT_REQUEST, type = DCM_CMDDATANULL;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.affectedInstanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDEVENTTYPEID, DCM_UI, "", 1, sizeof(response.eventTypeID), (void *) &response.eventTypeID},
    };

    response.status = DCM_STATUS_SUCCESS;

    response = *eventReportResp;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}

/* MSG_BuildNGetRequest
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
MSG_BuildNGetRequest(MSG_N_GET_REQ * getRequest, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_GET_REQ    request;
    static unsigned short   command = DCM_N_GET_REQUEST, type = DCM_CMDDATANULL;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.requestedInstanceUID}
    };
    DCM_ELEMENT e = {DCM_GROUPCOMMAND, DCM_CMDATTRIBUTEIDLIST, DCM_AT, "", 1, 0, NULL};

    request = *getRequest;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    e.length = sizeof(unsigned long) * getRequest->attributeCount;
    e.d.ul = getRequest->attributeList;

    cond = DCM_AddElement(object, &e);
    if (cond != DCM_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNGetResponse
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
MSG_BuildNGetResponse( MSG_N_GET_RESP * getResp, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int		        		index;
    static MSG_N_GET_RESP   response;
    static unsigned short   command = DCM_N_GET_RESPONSE, type = DCM_CMDDATAOTHER;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.affectedInstanceUID},
    };

    response.status = DCM_STATUS_SUCCESS;

    response = *getResp;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}

/* MSG_BuildNSetRequest
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
MSG_BuildNSetRequest(MSG_N_SET_REQ * getRequest, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_SET_REQ    request;
    static unsigned short   command = DCM_N_SET_REQUEST, type = DCM_CMDDATAOTHER;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.instanceUID}
    };

    request = *getRequest;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNSetResponse
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
MSG_BuildNSetResponse(MSG_N_SET_RESP * setResp, DCM_OBJECT ** object)
{
    CONDITION				cond;
    int				        index;
    static MSG_N_SET_RESP   response;
    static unsigned short   command = DCM_N_SET_RESPONSE, type = DCM_CMDDATAOTHER;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.instanceUID}
    };

    response = *setResp;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}

/* MSG_BuildNActionRequest
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
MSG_BuildNActionRequest(MSG_N_ACTION_REQ * actionRequest, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int				        	index;
    static MSG_N_ACTION_REQ     request;
    static unsigned short       command = DCM_N_SET_REQUEST, type = DCM_CMDDATANULL;
    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.instanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(request.actionTypeID), (void *) &request.actionTypeID}
    };

    request = *actionRequest;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNActionResponse
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
MSG_BuildNActionResponse(MSG_N_ACTION_RESP * actionResp, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_ACTION_RESP    response;
    static unsigned short       command = DCM_N_ACTION_RESPONSE, type = DCM_CMDDATANULL;
    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.instanceUID},
    		{DCM_GROUPCOMMAND, DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(response.actionTypeID), (void *) &response.actionTypeID}
    };

    response = *actionResp;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}

/* MSG_BuildNCreateRequest
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
MSG_BuildNCreateRequest(MSG_N_CREATE_REQ * createRequest, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_CREATE_REQ     request;
    static unsigned short       command = DCM_N_CREATE_REQUEST, type = DCM_CMDDATANULL;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1, sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.instanceUID}
    };

    request = *createRequest;
    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNCreateResponse
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
MSG_BuildNCreateResponse(MSG_N_CREATE_RESP * createResp, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_CREATE_RESP    response;
    static unsigned short       command = DCM_N_CREATE_RESPONSE, type = DCM_CMDDATAOTHER;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.instanceUID}
    };

    response = *createResp;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}

/* MSG_BuildNDeleteRequest
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
MSG_BuildNDeleteRequest(MSG_N_DELETE_REQ * deleteRequest, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_DELETE_REQ     request;
    static unsigned short       command = DCM_N_DELETE_REQUEST, type = DCM_CMDDATANULL;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) request.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGID, DCM_US, "", 1,	sizeof(request.messageID), (void *) &request.messageID},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(type), (void *) &type},
    		{DCM_GROUPCOMMAND, DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) request.instanceUID}
    };

    request = *deleteRequest;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}


/* MSG_BuildNDeleteResponse
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
MSG_BuildNDeleteResponse(MSG_N_DELETE_RESP * deleteResponse, DCM_OBJECT ** object)
{
    CONDITION					cond;
    int					        index;
    static MSG_N_DELETE_RESP    response;
    static unsigned short       command;

    static DCM_ELEMENT elementList[] = {
    		{DCM_GROUPCOMMAND, DCM_CMDCLASSUID, DCM_UI, "", 1, 0, (void *) response.classUID},
    		{DCM_GROUPCOMMAND, DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    		{DCM_GROUPCOMMAND, DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(response.messageIDRespondedTo), (void *) &response.messageIDRespondedTo},
    		{DCM_GROUPCOMMAND, DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(response.dataSetType), (void *) &response.dataSetType},
    		{DCM_GROUPCOMMAND, DCM_CMDSTATUS, DCM_US, "", 1, sizeof(response.status), (void *) &response.status},
    		{DCM_GROUPCOMMAND, DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, 0, (void *) response.instanceUID}
    };

    response = *deleteResponse;

    cond = MSGPRV_BuildObject(object, elementList, (int) DIM_OF(elementList), NULL, 0);
    if (cond != MSG_NORMAL)	return cond;

    return MSG_NORMAL;
}
