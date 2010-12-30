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
** @$=@$=@$=
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
**			MSG_DumpMessage
**	Private Functions
**			DumpCEchoRequest
**			DumpCEchoResponse
**			DumpCFindRequest
**			DumpCFindResponse
**			DumpCMoveRequest
**			DumpCMoveResponse
**			DumpCGetRequest
**			DumpCGetResponse
**			DumpCStoreRequest
**			DumpCStoreResponse
**			DumpNEventReportRequest
**			DumpNEventReportResponse
**			DumpNGetRequest
**			DumpNGetResponse
**			DumpNSetRequest
**			DumpNSetResponse
**			DumpNActionRequest
**			DumpNActionResponse
**			DumpNCreateRequest
**			DumpNCreateResponse
**			DumpNDeleteRequest
**			DumpNDeleteResponse
**			dumpStatus
**			dumpErrorStatusInformation
**			dumpActionReply
**			dumpActionTypeID
**			dumpEventReply
**			dumpEventID
**			dumpRequestedClassUID
**			dumpRequestedInstanceUID
**			dumpErrorComment
**			dumpErrorID
**			dumpAttributeIdentifierList
**
** Author, Date:	Stephen M. Moore, 14-Jun-93
** Intent:		This module contains routines for dumping MSG messages
**			to a file.
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:05 $
** Source File:		$RCSfile: dump.c,v $
** Revision:		$Revision: 1.30 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.30 $ $RCSfile: dump.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#include <stdarg.h>
#include <sys/types.h>
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "msgprivate.h"

static void
DumpCEchoRequest(MSG_C_ECHO_REQ * echo, FILE * f);
static void
DumpCEchoResponse(MSG_C_ECHO_RESP * echo, FILE * f);
static void
DumpCFindRequest(MSG_C_FIND_REQ * find, FILE * f);
static void
DumpCFindResponse(MSG_C_FIND_RESP * find, FILE * f);
static void
DumpCMoveRequest(MSG_C_MOVE_REQ * move, FILE * f);
static void
DumpCMoveResponse(MSG_C_MOVE_RESP * move, FILE * f);
static void
DumpCGetRequest(MSG_C_GET_REQ * get, FILE * f);
static void
DumpCGetResponse(MSG_C_GET_RESP * get, FILE * f);
static void
DumpCStoreRequest(MSG_C_STORE_REQ * store, FILE * f);
static void
DumpCStoreResponse(MSG_C_STORE_RESP * store, FILE * f);
static void
DumpNEventReportRequest(MSG_N_EVENT_REPORT_REQ * eventReport, FILE * f);
static void
DumpNEventReportResponse(MSG_N_EVENT_REPORT_RESP * eventReport, FILE * f);
static void
DumpNGetRequest(MSG_N_GET_REQ * get, FILE * f);
static void
DumpNGetResponse(MSG_N_GET_RESP * get, FILE * f);
static void
DumpNSetRequest(MSG_N_SET_REQ * set, FILE * f);
static void
DumpNSetResponse(MSG_N_SET_RESP * set, FILE * f);
static void
DumpNActionRequest(MSG_N_ACTION_REQ * action, FILE * f);
static void
DumpNActionResponse(MSG_N_ACTION_RESP * action, FILE * f);
static void
DumpNCreateRequest(MSG_N_CREATE_REQ * create, FILE * f);
static void
DumpNCreateResponse(MSG_N_CREATE_RESP * create, FILE * f);
static void
DumpNDeleteRequest(MSG_N_DELETE_REQ * delete, FILE * f);
static void
DumpNDeleteResponse(MSG_N_DELETE_RESP * delete, FILE * f);

#if STANDARD_VERSION < VERSION_JUL1993
static void
dumpUIDList(LST_HEAD * l, FILE * f);
#endif

static void
dumpStatus(unsigned short status, MSG_TYPE messageType, FILE * f);
static void
dumpErrorStatusInformation(void *message, FILE * f);
static void
dumpActionTypeID(void *message, FILE * f);
static void
dumpActionReply(void *message, FILE * f);
static void
dumpEventID(void *message, FILE * f);
static void
dumpEventReply(void *message, FILE * f);
static void
dumpRequestedClassUID(void *message, FILE * f);
static void
dumpRequestedInstanceUID(void *message, FILE * f);
static void
dumpErrorComment(void *message, FILE * f);
static void
dumpErrorID(void *message, FILE * f);
static void
dumpAttributeIdentifierList(void *message, FILE * f);

typedef struct {
    unsigned long 	flag;
    void (*dump) 	(void *, FILE *);
}   TABLE;

/* MSG_DumpMessage
**
** Purpose:
**	This function dumps an MSG message structure in ASCII to a file
**	which has been opened by the caller.  The caller passes a pointer
**	to one of the MSG structures which are defined by this facility.
**	This function looks at the "type" field in the message and calls
**	an appropriate private function which will interpret the data in
**	that structure and dump it to a file.  If the function does
**	not recognize the "type" of the message, it returns to the caller
**	with no error indication.
**
**	The caller's second argument is a FILE pointer to an open file.
**	Many times, this is stdout or stderr.
**
** Parameter Dictionary:
**	message		Pointer to the message to be dumped
**	f		Pointer to file into which the message is to be dumped
**
** Return Values:
**	None
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

void
MSG_DumpMessage(void *message, FILE * f)
{
    MSG_GENERAL    * msg;

    msg = (MSG_GENERAL *) message;

    fprintf(f, "******************* DumpMessage *******************\n");

    switch (msg->type) {
		case MSG_K_C_ECHO_REQ:
								DumpCEchoRequest((MSG_C_ECHO_REQ *) msg, f);
								break;
		case MSG_K_C_ECHO_RESP:
								DumpCEchoResponse((MSG_C_ECHO_RESP *) msg, f);
								break;
		case MSG_K_C_FIND_REQ:
								DumpCFindRequest((MSG_C_FIND_REQ *) msg, f);
								break;
		case MSG_K_C_FIND_RESP:
								DumpCFindResponse((MSG_C_FIND_RESP *) msg, f);
								break;
		case MSG_K_C_MOVE_REQ:
								DumpCMoveRequest((MSG_C_MOVE_REQ *) msg, f);
								break;
		case MSG_K_C_MOVE_RESP:
								DumpCMoveResponse((MSG_C_MOVE_RESP *) msg, f);
								break;
		case MSG_K_C_GET_REQ:
								DumpCGetRequest((MSG_C_GET_REQ *) msg, f);
								break;
		case MSG_K_C_GET_RESP:
								DumpCGetResponse((MSG_C_GET_RESP *) msg, f);
								break;
		case MSG_K_C_STORE_REQ:
								DumpCStoreRequest((MSG_C_STORE_REQ *) msg, f);
								break;
		case MSG_K_C_STORE_RESP:
								DumpCStoreResponse((MSG_C_STORE_RESP *) msg, f);
								break;
		case MSG_K_N_EVENT_REPORT_REQ:
								DumpNEventReportRequest((MSG_N_EVENT_REPORT_REQ *) msg, f);
								break;
		case MSG_K_N_EVENT_REPORT_RESP:
								DumpNEventReportResponse((MSG_N_EVENT_REPORT_RESP *) msg, f);
								break;
		case MSG_K_N_GET_REQ:
								DumpNGetRequest((MSG_N_GET_REQ *) msg, f);
								break;
		case MSG_K_N_GET_RESP:
								DumpNGetResponse((MSG_N_GET_RESP *) msg, f);
								break;
		case MSG_K_N_SET_REQ:
								DumpNSetRequest((MSG_N_SET_REQ *) msg, f);
								break;
		case MSG_K_N_SET_RESP:
								DumpNSetResponse((MSG_N_SET_RESP *) msg, f);
								break;
		case MSG_K_N_ACTION_REQ:
								DumpNActionRequest((MSG_N_ACTION_REQ *) msg, f);
								break;
		case MSG_K_N_ACTION_RESP:
								DumpNActionResponse((MSG_N_ACTION_RESP *) msg, f);
								break;
		case MSG_K_N_CREATE_REQ:
								DumpNCreateRequest((MSG_N_CREATE_REQ *) msg, f);
								break;
		case MSG_K_N_CREATE_RESP:
								DumpNCreateResponse((MSG_N_CREATE_RESP *) msg, f);
								break;
		case MSG_K_N_DELETE_REQ:
								DumpNDeleteRequest((MSG_N_DELETE_REQ *) msg, f);
								break;
		case MSG_K_N_DELETE_RESP:
								DumpNDeleteResponse((MSG_N_DELETE_RESP *) msg, f);
								break;
		default:
								break;
    };
    fprintf(f, "==================== DumpMessage ====================\n");
}

/* MSG_DumpCEchoRequest
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

static void
DumpCEchoRequest(MSG_C_ECHO_REQ * echo, FILE * f)
{
    fprintf(f, "Echo Request\n");
    fprintf(f, "Message ID:     %d\n", echo->messageID);
    fprintf(f, "Data Set Type:  %04x\n", echo->dataSetType);
    fprintf(f, "Class UID:      %s\n", echo->classUID);
}


/* MSG_DumpCEchoResponse
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

static void
DumpCEchoResponse(MSG_C_ECHO_RESP * echo, FILE * f)
{
    fprintf(f, "Echo Response\n");
    fprintf(f, "Message ID Responded To: %d\n", echo->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", echo->dataSetType);
    fprintf(f, "Status:                  %04x  ", echo->status);
    dumpStatus(echo->status, echo->type, f);
    fprintf(f, "Class UID:               %s\n", echo->classUID);
}


/* DumpCStoreRequest
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

static void
DumpCStoreRequest(MSG_C_STORE_REQ * store, FILE * f)
{
    fprintf(f, "Store Request\n");
    fprintf(f, "Message ID:     %d\n", store->messageID);
    fprintf(f, "Priority:       %04x\n", store->priority);
    fprintf(f, "Data Set Type:  %04x\n", store->dataSetType);
    fprintf(f, "Class UID:      %s\n", store->classUID);
    fprintf(f, "Instance UID:   %s\n", store->instanceUID);

    if (store->conditionalFields & MSG_K_C_STORE_MOVEMESSAGEID)
    	fprintf(f, "Move Message ID:            %hd\n", store->moveMessageID);
    if (store->conditionalFields & MSG_K_C_STORE_MOVEAETITLE)
    	fprintf(f, "Move AE Title:              %s\n", store->moveAETitle);
}

/* DumpCStoreResponse
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

static void
DumpCStoreResponse(MSG_C_STORE_RESP * store, FILE * f)
{
    fprintf(f, "Store Response\n");
    fprintf(f, "Message ID Resp:%d\n", store->messageIDRespondedTo);
    fprintf(f, "Data Set Type:  %04x\n", store->dataSetType);
    fprintf(f, "Status:         %04x  ", store->status);
    dumpStatus(store->status, store->type, f);

    if (store->conditionalFields & MSG_K_C_STORERESP_CLASSUID)
    	fprintf(f, "Class UID:      %s\n", store->classUID);
    if (store->conditionalFields & MSG_K_C_STORERESP_INSTANCEUID)
    	fprintf(f, "Instance UID:   %s\n", store->instanceUID);
    if (store->conditionalFields & MSG_K_C_STORERESP_ERRORCOMMENT)
    	fprintf(f, "ErrorComment:   %s\n", store->errorComment);
}

/* DumpCFindRequest
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

static void
DumpCFindRequest(MSG_C_FIND_REQ * find, FILE * f)
{
    fprintf(f, "CFind Request\n");
    fprintf(f, "Message ID:              %d\n", find->messageID);
    fprintf(f, "Data Set Type:           %04x\n", find->dataSetType);
    fprintf(f, "Priority:                %04x\n", find->priority);
    fprintf(f, "Class UID:               %s\n", find->classUID);
    (void) DCM_DumpElements(&find->identifier, 0);
}

/* MSG_DumpCFindResponse
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

static void
DumpCFindResponse(MSG_C_FIND_RESP * find, FILE * f)
{
    fprintf(f, "CFind Response\n");
    fprintf(f, "Message ID Responded To: %d\n", find->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", find->dataSetType);
    fprintf(f, "Status:                  %04x  ", find->status);
    dumpStatus(find->status, find->type, f);
    fprintf(f, "Class UID:               %s\n", find->classUID);
    if (find->conditionalFields & MSG_K_C_FINDRESP_ERRORCOMMENT)
    	fprintf(f, "Error Comment:           %s\n", find->errorComment);
    (void) DCM_DumpElements(&find->identifier, 0);
}

/* DumpCMoveRequest
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

static void
DumpCMoveRequest(MSG_C_MOVE_REQ * move, FILE * f)
{
    fprintf(f, "Move Request\n");
    fprintf(f, "Message ID:              %d\n", move->messageID);
    fprintf(f, "Move Data Set Type:      %04x\n", move->dataSetType);
    fprintf(f, "Move Priority:           %04x\n", move->priority);
    fprintf(f, "Move Destination Title:  %s\n", move->moveDestination);
    fprintf(f, "Move classUID:           %s\n", move->classUID);
    (void) DCM_DumpElements(&move->identifier, 0);
}

/* DumpCMoveResponse
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

static void
DumpCMoveResponse(MSG_C_MOVE_RESP * resp, FILE * f)
{
    fprintf(f, "Move Response\n");
    fprintf(f, "Message ID Responded To: %d\n", resp->messageIDRespondedTo);
    fprintf(f, "Move Status:             %04x  ", resp->status);
    dumpStatus(resp->status, resp->type, f);
    fprintf(f, "Move classUID:           %s\n", resp->classUID);
    if (resp->conditionalFields & MSG_K_C_MOVERESP_ERRORCOMMENT)
    	fprintf(f, "ErrorComment:   %s\n", resp->errorComment);

#if STANDARD_VERSION < VERSION_JUL1993
    if (resp->conditionalFields & MSG_K_C_MOVE_SUCCESSUID) {
    	fprintf(f, "Success UIDs:\n");
    	dumpUIDList(resp->successUIDList, f);
    }
    if (resp->conditionalFields & MSG_K_C_MOVE_FAILEDUID) {
    	fprintf(f, "Failed UIDs:\n");
    	dumpUIDList(resp->failedUIDList, f);
    }
    if (resp->conditionalFields & MSG_K_C_MOVE_WARNINGUID) {
    	fprintf(f, "Warning UIDs:\n");
    	dumpUIDList(resp->warningUIDList, f);
    }
#else
    if (resp->dataSet != NULL)
    	(void) DCM_DumpElements(&resp->dataSet, 0);
#endif
    if (resp->conditionalFields & MSG_K_C_MOVE_REMAINING)
    	fprintf(f, "Remaining Suboperations: %hd\n", resp->remainingSubOperations);
    if (resp->conditionalFields & MSG_K_C_MOVE_COMPLETED)
    	fprintf(f, "Completed Suboperations: %hd\n", resp->completedSubOperations);
    if (resp->conditionalFields & MSG_K_C_MOVE_FAILED)
    	fprintf(f, "Failed Suboperations:    %hd\n", resp->failedSubOperations);
    if (resp->conditionalFields & MSG_K_C_MOVE_WARNING)
    	fprintf(f, "Warning Suboperations:   %hd\n", resp->warningSubOperations);
}

#if STANDARD_VERSION < VERSION_JUL1993
static void
dumpUIDList(LST_HEAD * l, FILE * f)
{
    MSG_UID_ITEM    * item;

    item = LST_Head(&l);
    if (item != NULL) (void) LST_Position(&l, item);

    while (item != NULL) {
    	fprintf(f, "                  %s\n", item->UID);
    	item = LST_Next(&l);
    }
}
#endif

/* DumpCGetRequest
**
** Purpose:
**	Dump fields of the CGetRequest structure
**
** Parameter Dictionary:
**	get	Pointer to CGET request structure
**	f	File pointer
**
** Return Values:
**	None
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
DumpCGetRequest(MSG_C_GET_REQ * get, FILE * f)
{
    fprintf(f, "Get Request\n");
    fprintf(f, "Message ID:              %d\n", get->messageID);
    fprintf(f, "Get Data Set Type:      %04x\n", get->dataSetType);
    fprintf(f, "Get Priority:           %04x\n", get->priority);
    fprintf(f, "Get classUID:           %s\n", get->classUID);
    (void) DCM_DumpElements(&get->identifier, 0);
}


/* DumpCGetResponse
**
** Purpose:
**	Dump the elements of the CGetResponse message
**
** Parameter Dictionary:
**	resp		Pointer to the CGET response message
**	f		File pointer
**
** Return Values:
**	None
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
DumpCGetResponse(MSG_C_GET_RESP * resp, FILE * f)
{
    fprintf(f, "Get Response\n");
    fprintf(f, "Message ID Responded To: %d\n", resp->messageIDRespondedTo);
    fprintf(f, "Get Status:             %04x  ", resp->status);
    dumpStatus(resp->status, resp->type, f);
    fprintf(f, "Get classUID:           %s\n", resp->classUID);
    if (resp->conditionalFields & MSG_K_C_GETRESP_ERRORCOMMENT)
    	fprintf(f, "ErrorComment:   %s\n", resp->errorComment);

    if (resp->dataSetType != DCM_CMDDATANULL)
    	(void) DCM_DumpElements(&resp->identifier, 0);
    if (resp->conditionalFields & MSG_K_C_GET_REMAINING)
    	fprintf(f, "Remaining Suboperations: %hd\n", resp->remainingSubOperations);
    if (resp->conditionalFields & MSG_K_C_GET_COMPLETED)
    	fprintf(f, "Completed Suboperations: %hd\n", resp->completedSubOperations);
    if (resp->conditionalFields & MSG_K_C_GET_FAILED)
    	fprintf(f, "Failed Suboperations:    %hd\n", resp->failedSubOperations);
    if (resp->conditionalFields & MSG_K_C_GET_WARNING)
    	fprintf(f, "Warning Suboperations:   %hd\n", resp->warningSubOperations);
}
static void
DumpNEventReportRequest(MSG_N_EVENT_REPORT_REQ * eventReport, FILE * f)
{
    fprintf(f, "N EventReport Request\n");
    fprintf(f, "Message ID:          %d\n", eventReport->messageID);
    fprintf(f, "Data Set Type:       %04x\n", eventReport->dataSetType);
    fprintf(f, "Event Type ID:       %04x\n", eventReport->eventTypeID);
    fprintf(f, "Class UID:           %s\n", eventReport->classUID);
    fprintf(f, "Instance UID:        %s\n", eventReport->affectedInstanceUID);

    if (eventReport->dataSetType == DCM_CMDDATAOTHER) {
    	fprintf(f, "Event Information:\n");
    	(void) DCM_DumpElements(&eventReport->dataSet, 0);
    }
}

static void
DumpNEventReportResponse(MSG_N_EVENT_REPORT_RESP * eventReport, FILE * f)
{
    fprintf(f, "N EventReport Response\n");
    fprintf(f, "Message ID Responded To: %d\n", eventReport->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", eventReport->dataSetType);
    fprintf(f, "Status:                  %04x\n", eventReport->status);
    fprintf(f, "Event Type ID:           %04x\n", eventReport->eventTypeID);
    fprintf(f, "Class UID:               %s\n", eventReport->classUID);
    fprintf(f, "Instance UID:            %s\n", eventReport->affectedInstanceUID);
    fprintf(f, "ConditionalFields:	 %04lx\n", eventReport->conditionalFields);

    if (eventReport->dataSetType != DCM_CMDDATANULL) {
    	fprintf(f, "Event Reply:\n");
    	(void) DCM_DumpElements(&eventReport->dataSet, 0);
    }
    dumpStatus(eventReport->status, eventReport->type, f);
    /* On Error status, dump the status information */
    if (eventReport->conditionalFields != 0) {
    	dumpErrorStatusInformation(eventReport, f);
    }
}

static void
DumpNGetRequest(MSG_N_GET_REQ * get, FILE * f)
{
    int       index;

    fprintf(f, "N Get Request\n");
    fprintf(f, "Message ID: %d\n", get->messageID);
    fprintf(f, "Class UID:           %s\n", get->classUID);
    fprintf(f, "Instance UID:        %s\n", get->requestedInstanceUID);

    for (index = 0; index < get->attributeCount; index++){
    	fprintf(f, "Attribute:   %04x %04x\n", DCM_TAG_GROUP(get->attributeList[index]), DCM_TAG_ELEMENT(get->attributeList[index]));
    }
}

static void
DumpNGetResponse(MSG_N_GET_RESP * get, FILE * f)
{
    fprintf(f, "N Get Response\n");
    fprintf(f, "Message ID Responded To: %d\n", get->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", get->dataSetType);
    fprintf(f, "Status:                  %04x  ", get->status);
    fprintf(f, "Class UID:               %s\n", get->classUID);
    fprintf(f, "Instance UID:            %s\n", get->affectedInstanceUID);
    fprintf(f, "ConditionalFields:	 %04lx\n", get->conditionalFields);

    if (get->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&get->dataSet, 0);
    }
    dumpStatus(get->status, get->type, f);
    if (get->conditionalFields != 0) {
    	dumpErrorStatusInformation(get, f);
    }
}

static void
DumpNSetRequest(MSG_N_SET_REQ * set, FILE * f)
{
    fprintf(f, "N Set Request\n");
    fprintf(f, "Message ID: %d\n", set->messageID);
    fprintf(f, "Class UID:           %s\n", set->classUID);
    fprintf(f, "Instance UID:        %s\n", set->instanceUID);
    fprintf(f, "Data Set Type:           %04x\n", set->dataSetType);
    if (set->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&set->dataSet, 0);
    }
}
static void
DumpNSetResponse(MSG_N_SET_RESP * set, FILE * f)
{
    fprintf(f, "N Set Response\n");
    fprintf(f, "Message ID Responded To: %d\n", set->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", set->dataSetType);
    fprintf(f, "Class UID:               %s\n", set->classUID);
    fprintf(f, "Instance UID:            %s\n", set->instanceUID);
    fprintf(f, "ConditionalFields:	 %04lx\n", set->conditionalFields);
    if (set->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&set->dataSet, 0);
    }
    dumpStatus(set->status, set->type, f);
	if (set->conditionalFields != 0) {
		dumpErrorStatusInformation(set, f);
	}
}
static void
DumpNActionRequest(MSG_N_ACTION_REQ * action, FILE * f)
{
    fprintf(f, "N Action Request\n");
    fprintf(f, "Message ID: %d\n", action->messageID);
    fprintf(f, "Class UID:           %s\n", action->classUID);
    fprintf(f, "Instance UID:        %s\n", action->instanceUID);
    fprintf(f, "Action Type ID:           %04x\n", action->actionTypeID);
    fprintf(f, "Data Set Type:           %04x\n", action->dataSetType);

    if (action->dataSetType != DCM_CMDDATANULL) {
    	fprintf(f, "Action Information\n");
    	(void) DCM_DumpElements(&action->actionInformation, 0);
    }
}
static void
DumpNActionResponse(MSG_N_ACTION_RESP * action, FILE * f)
{
    fprintf(f, "N Action Response\n");
    fprintf(f, "Message ID Responded To: %d\n", action->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", action->dataSetType);
    fprintf(f, "Class UID:               %s\n", action->classUID);
    fprintf(f, "Instance UID:            %s\n", action->instanceUID);
    fprintf(f, "ConditionalFields:	 %04lx\n", action->conditionalFields);

    if (action->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&action->actionReply, 0);
    }
    dumpStatus(action->status, action->type, f);
    if (action->conditionalFields != 0) {
    	dumpErrorStatusInformation(action, f);
    }
}
static void
DumpNCreateRequest(MSG_N_CREATE_REQ * create, FILE * f)
{
    fprintf(f, "N Create Request\n");
    fprintf(f, "Message ID: %d\n", create->messageID);
    fprintf(f, "Class UID:           %s\n", create->classUID);
    fprintf(f, "Instance UID:        %s\n", create->instanceUID);
    fprintf(f, "Data Set Type:           %04x\n", create->dataSetType);

    if (create->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&create->dataSet, 0);
    }
}
static void
DumpNCreateResponse(MSG_N_CREATE_RESP * create, FILE * f)
{
    fprintf(f, "N Create Response\n");
    fprintf(f, "Message ID Responded To: %d\n", create->messageIDRespondedTo);
    fprintf(f, "Class UID:               %s\n", create->classUID);
    fprintf(f, "Instance UID:            %s\n", create->instanceUID);
    fprintf(f, "Data Set Type:           %04x\n", create->dataSetType);
    fprintf(f, "ConditionalFields:	 %04lx\n", create->conditionalFields);

    if (create->dataSetType != DCM_CMDDATANULL) {
    	(void) DCM_DumpElements(&create->dataSet, 0);
    }
    dumpStatus(create->status, create->type, f);
    if (create->conditionalFields != 0) {
    	dumpErrorStatusInformation(create, f);
    }
}
static void
DumpNDeleteRequest(MSG_N_DELETE_REQ * delete, FILE * f)
{
    fprintf(f, "N Delete Request\n");
    fprintf(f, "Message ID: %d\n", delete->messageID);
    fprintf(f, "Class UID:           %s\n", delete->classUID);
    fprintf(f, "Instance UID:        %s\n", delete->instanceUID);
    fprintf(f, "Data Set Type:           %04x\n", delete->dataSetType);
}
static void
DumpNDeleteResponse(MSG_N_DELETE_RESP * delete, FILE * f)
{
    fprintf(f, "N Delete Response\n");
    fprintf(f, "Message ID Responded To: %d\n", delete->messageIDRespondedTo);
    fprintf(f, "Data Set Type:           %04x\n", delete->dataSetType);
    fprintf(f, "Class UID:               %s\n", delete->classUID);
    fprintf(f, "Instance UID:            %s\n", delete->instanceUID);
    fprintf(f, "ConditionalFields:	 %04lx\n", delete->conditionalFields);
    dumpStatus(delete->status, delete->type, f);

    if (delete->conditionalFields != 0) {
    	dumpErrorStatusInformation(delete, f);
    }
}

static void
dumpStatus(unsigned short status, MSG_TYPE messageType, FILE * f)
{
    CONDITION				cond;
    MSG_STATUS_DESCRIPTION	description;

    fprintf(f, "Status Information:- \n");

    cond = MSG_StatusLookup(status, messageType, &description);
    if (cond == MSG_NORMAL){
    	fprintf(f, "\t%s\n", description.description);
    }else{
    	fprintf(f, "\tDescription of status code not found\n");
    	(void) COND_PopCondition(FALSE);
    }
}

static TABLE neventReportTable[] = {
    {MSG_K_N_EVENTREPORTRESP_EVENTTYPEID, dumpEventID},
    {MSG_K_N_EVENTREPORTRESP_EVENTINFORMATION, dumpEventReply},
    {MSG_K_N_EVENTREPORTRESP_REQUESTEDCLASSUID, dumpRequestedClassUID},
    {MSG_K_N_EVENTREPORTRESP_REQUESTEDINSTANCEUID, dumpRequestedInstanceUID},
    {MSG_K_N_EVENTREPORTRESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_EVENTREPORTRESP_ERRORID, dumpErrorID}
};

static TABLE ngetTable[] = {
    {MSG_K_N_GETRESP_ATTRIBUTEIDENTIFIERLIST, dumpAttributeIdentifierList},
    {MSG_K_N_GETRESP_REQUESTEDCLASSUID, dumpRequestedClassUID},
    {MSG_K_N_GETRESP_REQUESTEDINSTANCEUID, dumpRequestedInstanceUID},
    {MSG_K_N_GETRESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_GETRESP_ERRORID, dumpErrorID}
};

static TABLE nsetTable[] = {
    {MSG_K_N_SETRESP_ATTRIBUTEIDENTIFIERLIST, dumpAttributeIdentifierList},
    {MSG_K_N_SETRESP_REQUESTEDCLASSUID, dumpRequestedClassUID},
    {MSG_K_N_SETRESP_REQUESTEDINSTANCEUID, dumpRequestedInstanceUID},
    {MSG_K_N_SETRESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_SETRESP_ERRORID, dumpErrorID}
};

static TABLE nactionTable[] = {
    {MSG_K_N_ACTIONRESP_ACTIONINFORMATION, dumpActionReply},
    {MSG_K_N_ACTIONRESP_ACTIONTYPEID, dumpActionTypeID},
    /* {MSG_K_N_ACTIONRESP_REQUESTEDCLASSUID, dumpRequestedClassUID}, repair */
    /*
     * {MSG_K_N_ACTIONRESP_REQUESTEDINSTANCEUID, dumpRequestedInstanceUID},
     * repair
     */
    {MSG_K_N_ACTIONRESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_ACTIONRESP_ERRORID, dumpErrorID}
};

static TABLE ncreateTable[] = {
    {MSG_K_N_CREATERESP_ATTRIBUTEIDENTIFIERLIST, dumpAttributeIdentifierList},
    /* {MSG_K_N_CREATERESP_AFFECTEDCLASSUID, dumpRequestedClassUID}, repair */
    /*
     * {MSG_K_N_CREATERESP_AFFECTEDINSTANCEUID, dumpRequestedInstanceUID},
     * repair
     */
    {MSG_K_N_CREATERESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_CREATERESP_ERRORID, dumpErrorID}
};

static TABLE ndeleteTable[] = {
    /* {MSG_K_N_DELETERESP_REQUESTEDCLASSUID, dumpRequestedClassUID}, repair */
    /*
     * {MSG_K_N_DELETERESP_REQUESTEDINSTANCEUID, dumpRequestedInstanceUID},
     * repair
     */
    {MSG_K_N_DELETERESP_ERRORCOMMENT, dumpErrorComment},
    {MSG_K_N_DELETERESP_ERRORID, dumpErrorID}
};

static void
dumpErrorStatusInformation(void *message, FILE * f)
{
    /*
     * This routine is invoked if the status value is not MSG_K_SUCCESS. This
     * dumps values of the conditional fields that are received which provide
     * additional information on the status code
     */
    int		        i;
    MSG_GENERAL		* msg;

    fprintf(f, "Conditional fields :-\n");
    msg = (MSG_GENERAL *) message;

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
										for (i = 0; i < (int) DIM_OF(neventReportTable); i++) {
											if (((MSG_N_EVENT_REPORT_RESP *) msg)->conditionalFields & neventReportTable[i].flag)
												neventReportTable[i].dump(message, f);
										}
										break;
		case MSG_K_N_GET_RESP:
										for (i = 0; i < (int) DIM_OF(ngetTable); i++) {
											if (((MSG_N_GET_RESP *) msg)->conditionalFields & ngetTable[i].flag)
												ngetTable[i].dump(message, f);
										}
										break;
		case MSG_K_N_SET_RESP:
										for (i = 0; i < (int) DIM_OF(nsetTable); i++) {
											if (((MSG_N_SET_RESP *) msg)->conditionalFields & nsetTable[i].flag)
												nsetTable[i].dump(message, f);
										}
										break;
		case MSG_K_N_ACTION_RESP:
										for (i = 0; i < (int) DIM_OF(nactionTable); i++) {
											if (((MSG_N_ACTION_RESP *) msg)->conditionalFields & nactionTable[i].flag)
												nactionTable[i].dump(message, f);
										}
										break;
		case MSG_K_N_CREATE_RESP:
										for (i = 0; i < (int) DIM_OF(ncreateTable); i++) {
											if (((MSG_N_CREATE_RESP *) msg)->conditionalFields & ncreateTable[i].flag)
												ncreateTable[i].dump(message, f);
										}
										break;
		case MSG_K_N_DELETE_RESP:
										for (i = 0; i < (int) DIM_OF(ndeleteTable); i++) {
											if (((MSG_N_DELETE_RESP *) msg)->conditionalFields & ndeleteTable[i].flag)
												ndeleteTable[i].dump(message, f);
										}
										break;
		default:
										fprintf(f, "INVALID Message sent for dumping error status information\n");
    }
}
static void
dumpActionTypeID(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    switch (msg->type) {
		case MSG_K_N_ACTION_RESP:
									fprintf(f, "\tAction Type ID:	%04x\n", ((MSG_N_ACTION_RESP *) message)->actionTypeID);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping Action Type ID\n");
    }
}
static void
dumpActionReply(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    switch (msg->type) {
		case MSG_K_N_ACTION_RESP:
									fprintf(f, "\tAction Reply:\n");
									(void) DCM_DumpElements(&((MSG_N_ACTION_RESP *) message)->actionReply, 0);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping Action Reply\n");
    }
}
static void
dumpEventID(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "\tEvent Type ID:	%04x\n", ((MSG_N_EVENT_REPORT_RESP *) message)->eventTypeID);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping event ID\n");
    }
}
static void
dumpEventReply(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "\tEvent Reply:\n");
									(void) DCM_DumpElements(&((MSG_N_EVENT_REPORT_RESP *) message)->dataSet, 0);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping Event reply\n");
    }
}
static void
dumpRequestedClassUID(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    fprintf(f, "\tRequested Class UID:	");

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "%s\n", ((MSG_N_EVENT_REPORT_RESP *) message)->requestedClassUID);
									break;
		case MSG_K_N_GET_RESP:
									fprintf(f, "%s\n", ((MSG_N_GET_RESP *) message)->requestedClassUID);
									break;
		case MSG_K_N_SET_RESP:
									fprintf(f, "%s\n", ((MSG_N_SET_RESP *) message)->requestedClassUID);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping requested class UID\n");
    }
}
static void
dumpRequestedInstanceUID(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    fprintf(f, "\tRequested Instance UID:	");

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "%s\n", ((MSG_N_EVENT_REPORT_RESP *) message)->requestedInstanceUID);
									break;
		case MSG_K_N_GET_RESP:
									fprintf(f, "%s\n", ((MSG_N_GET_RESP *) message)->requestedInstanceUID);
									break;
		case MSG_K_N_SET_RESP:
									fprintf(f, "%s\n", ((MSG_N_SET_RESP *) message)->requestedInstanceUID);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping requested instance UID\n");
    }
}
static void
dumpErrorComment(void *message, FILE * f)
{
    MSG_GENERAL    * msg = (MSG_GENERAL *) message;

    fprintf(f, "\tError Comment:		");

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "%s\n", ((MSG_N_EVENT_REPORT_RESP *) message)->errorComment);
									break;
		case MSG_K_N_GET_RESP:
									fprintf(f, "%s\n", ((MSG_N_GET_RESP *) message)->errorComment);
									break;
		case MSG_K_N_SET_RESP:
									fprintf(f, "%s\n", ((MSG_N_SET_RESP *) message)->errorComment);
									break;
		case MSG_K_N_ACTION_RESP:
									fprintf(f, "%s\n", ((MSG_N_ACTION_RESP *) message)->errorComment);
									break;
		case MSG_K_N_CREATE_RESP:
									fprintf(f, "%s\n", ((MSG_N_CREATE_RESP *) message)->errorComment);
									break;
		case MSG_K_N_DELETE_RESP:
									fprintf(f, "%s\n", ((MSG_N_DELETE_RESP *) message)->errorComment);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping error comment UID\n");
    }
}
static void
dumpErrorID(void *message, FILE * f)
{
    MSG_GENERAL	    * msg = (MSG_GENERAL *) message;

    fprintf(f, "\tError ID:	");

    switch (msg->type) {
		case MSG_K_N_EVENT_REPORT_RESP:
									fprintf(f, "%04x\n", ((MSG_N_EVENT_REPORT_RESP *) message)->errorID);
									break;
		case MSG_K_N_GET_RESP:
									fprintf(f, "%04x\n", ((MSG_N_GET_RESP *) message)->errorID);
									break;
		case MSG_K_N_SET_RESP:
									fprintf(f, "%04x\n", ((MSG_N_SET_RESP *) message)->errorID);
									break;
		case MSG_K_N_ACTION_RESP:
									fprintf(f, "%04x\n", ((MSG_N_ACTION_RESP *) message)->errorID);
									break;
		case MSG_K_N_CREATE_RESP:
									fprintf(f, "%04x\n", ((MSG_N_CREATE_RESP *) message)->errorID);
									break;
		case MSG_K_N_DELETE_RESP:
									fprintf(f, "%04x\n", ((MSG_N_DELETE_RESP *) message)->errorID);
									break;
		default:
									/* This part of code never to be reached */
									fprintf(f, "INVALID Message sent for dumping error ID UID\n");
    }
}
static void
dumpAttributeIdentifierList(void *message, FILE * f)
{
    int		        i;
    DCM_ELEMENT		e;
    MSG_GENERAL		* msg = (MSG_GENERAL *) message;

    fprintf(f, "Attribute List:\n");

    switch (msg->type) {
		case MSG_K_N_GET_RESP:
								for (i = 0; i < ((MSG_N_GET_RESP *) message)->attributeCount; i++) {
									fprintf(f, "\tAttribute:   %04x %04x\n", DCM_TAG_GROUP(((MSG_N_GET_RESP *) message)->attributeIdentifierList[i]),
											DCM_TAG_ELEMENT(((MSG_N_GET_RESP *) message)->attributeIdentifierList[i]));
									memset(&e, 0, sizeof(DCM_ELEMENT));
									e.tag = ((MSG_N_GET_RESP *) message)->attributeIdentifierList[i];
									if (DCM_LookupElement(&e) != DCM_NORMAL) {
										COND_DumpConditions();
										fprintf(f, "\t\tDescription: Unknown Attribute\n");
									}else{
										fprintf(f, "\t\tDescription: %s\n", e.description);
									}
								}
								break;
		case MSG_K_N_SET_RESP:
								for (i = 0; i < ((MSG_N_SET_RESP *) message)->attributeCount; i++) {
									fprintf(f, "\tAttribute:   %04x %04x\n", DCM_TAG_GROUP(((MSG_N_SET_RESP *) message)->attributeIdentifierList[i]),
											DCM_TAG_ELEMENT(((MSG_N_SET_RESP *) message)->attributeIdentifierList[i]));
									memset(&e, 0, sizeof(DCM_ELEMENT));
									e.tag = ((MSG_N_SET_RESP *) message)->attributeIdentifierList[i];
									if (DCM_LookupElement(&e) != DCM_NORMAL) {
										COND_DumpConditions();
										fprintf(f, "\t\tDescription: Unknown Attribute\n");
									}else{
										fprintf(f, "\t\tDescription: %s\n", e.description);
									}
								}
								break;
		case MSG_K_N_CREATE_RESP:
								for (i = 0; i < ((MSG_N_CREATE_RESP *) message)->attributeCount; i++) {
									fprintf(f, "\tAttribute:   %04x %04x\n", DCM_TAG_GROUP(((MSG_N_CREATE_RESP *) message)->attributeIdentifierList[i]),
											DCM_TAG_ELEMENT(((MSG_N_CREATE_RESP *) message)->attributeIdentifierList[i]));
									memset(&e, 0, sizeof(DCM_ELEMENT));
									e.tag = ((MSG_N_CREATE_RESP *) message)->attributeIdentifierList[i];
									if (DCM_LookupElement(&e) != DCM_NORMAL) {
										COND_DumpConditions();
										fprintf(f, "\t\tDescription: Unknown Attribute\n");
									}else{
										fprintf(f, "\t\tDescription: %s\n", e.description);
									}
								}
								break;
		default:
								/* This part of code never to be reached */
								fprintf(f, "INVALID Message sent for dumping attribute list UID\n");
    }
}
