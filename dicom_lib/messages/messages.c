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
** Module Name(s):	MSG_Free
**			MSG_BuildCommand
**			MSG_ParseCommand
**			MSG_StatusLookup
**			buildObject
**			parseCommand
** Author, Date:	Stephen M. Moore, 27-Apr-93
** Intent:		This module contains routines for the MSG facility.
**			These include the routines for translating between
**			DICOM objects and the fixed MSG structures.
** Last Update:		$Author: drm $, $Date: 2002/07/17 19:40:05 $
** Source File:		$RCSfile: messages.c,v $
** Revision:		$Revision: 1.54 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.54 $ $RCSfile: messages.c,v $";

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
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "msgprivate.h"

#ifdef CTN_USE_THREADS
#include "../thread/ctnthread.h"
#endif


/* MSG_Free
**
** Purpose:
**	This function frees a MSG structure that was allocated by this
**	facility.  This requires freeing any lists or DICOM Objects
**	that were created with the structure and freeing the memory for
**	the structure itself.
**
**	The caller passes the address of a pointer to a structure.
**	This function
**		- determines the type of structure
**		- frees any subelement of the structure as necessary
**		- frees the structure, and
**		- writes a NULL into the caller's pointer
**	The last step eliminates the caller's reference to the structure.
**
** Parameter Dictionary:
**	msg	Address of a pointer to a MSG structure
**
** Return Values:
**	MSG_NORMAL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_Free(void **msg)
{
    CONDITION		cond = MSG_NORMAL;
    MSG_GENERAL		* m;

    m = (MSG_GENERAL *) * msg;
    switch (m->type) {
		case MSG_K_C_ECHO_REQ:
		case MSG_K_C_ECHO_RESP:
		case MSG_K_C_GET_REQ:
		case MSG_K_C_GET_RESP:
		case MSG_K_C_MOVE_REQ:
		case MSG_K_C_MOVE_RESP:
		case MSG_K_C_PRINT_REQ:
		case MSG_K_C_PRINT_RESP:
		case MSG_K_C_STORE_REQ:
		case MSG_K_C_STORE_RESP:
		case MSG_K_C_CANCEL_REQ:
		case MSG_K_N_EVENT_REPORT_REQ:
		case MSG_K_N_EVENT_REPORT_RESP:
		case MSG_K_N_GET_REQ:
		case MSG_K_N_GET_RESP:
		case MSG_K_N_SET_REQ:
		case MSG_K_N_SET_RESP:
		case MSG_K_N_ACTION_REQ:
		case MSG_K_N_ACTION_RESP:
		case MSG_K_N_CREATE_REQ:
		case MSG_K_N_CREATE_RESP:
		case MSG_K_N_DELETE_REQ:
		case MSG_K_N_DELETE_RESP:
		case MSG_K_REFERENCED_ITEM:
										CTN_FREE(m);
										*msg = NULL;
										break;
		case MSG_K_C_FIND_REQ:
										/* if (((MSG_C_FIND_REQ *)msg)->identifier != NULL)
										(void) DCM_CloseObject(&((MSG_C_FIND_REQ *)m)->identifier); */
										CTN_FREE(m);
										*msg = NULL;
										break;
		case MSG_K_C_FIND_RESP:
										/* if (((MSG_C_FIND_RESP *)msg)->identifier != NULL)
										(void) DCM_CloseObject(&((MSG_C_FIND_RESP *)m)->identifier); */
										CTN_FREE(m);
										*msg = NULL;
										break;
		default:
										cond = COND_PushCondition(MSG_ILLEGALMESSAGETYPE, MSG_Message(MSG_ILLEGALMESSAGETYPE), (int) m->type, "MSG_Free");
										break;
    }
    return cond;
}

/* The following section defines a set of static structures that
** provide memory for the Build and Parse routines which are implemented
** below.  The tables define the set of required and conditional elements
** for the messages which are supported by this facility.
*/

static unsigned short    command;
static MSG_C_ECHO_REQ    CEchoRequest;
static DCM_ELEMENT CEchoRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CEchoRequest.dataSetType), (void *) &CEchoRequest.dataSetType},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(CEchoRequest.messageID), (void *) &CEchoRequest.messageID},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CEchoRequest.classUID), (void *) &CEchoRequest.classUID[0]},
};

static MSG_C_ECHO_RESP    CEchoResponse;
static DCM_ELEMENT CEchoResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CEchoResponse.dataSetType), (void *) &CEchoResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(CEchoResponse.status), (void *) &CEchoResponse.status},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CEchoResponse.messageIDRespondedTo), (void *) &CEchoResponse.messageIDRespondedTo},
};
static DCM_FLAGGED_ELEMENT CEchoResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CEchoResponse.classUID), (void *) &CEchoResponse.classUID[0], MSG_K_C_ECHORESP_CLASSUID, &CEchoResponse.conditionalFields},
};

static MSG_C_STORE_REQ    CStoreRequest;
static DCM_ELEMENT CStoreRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CStoreRequest.classUID), (void *) &CStoreRequest.classUID[0]},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(CStoreRequest.messageID), (void *) &CStoreRequest.messageID},
    {DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(CStoreRequest.priority), (void *) &CStoreRequest.priority},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CStoreRequest.dataSetType), (void *) &CStoreRequest.dataSetType},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(CStoreRequest.instanceUID), (void *) &CStoreRequest.instanceUID[0]}
};

static DCM_FLAGGED_ELEMENT CStoreRequestC[] = {
    {DCM_CMDMOVEAETITLE, DCM_AE, "", 1, sizeof(CStoreRequest.moveAETitle), (void *) &CStoreRequest.moveAETitle[0], MSG_K_C_STORE_MOVEAETITLE, &CStoreRequest.conditionalFields},
    {DCM_CMDMOVEMESSAGEID, DCM_US, "", 1, sizeof(CStoreRequest.moveMessageID), (void *) &CStoreRequest.moveMessageID, MSG_K_C_STORE_MOVEMESSAGEID, &CStoreRequest.conditionalFields},
};

static MSG_C_STORE_RESP    CStoreResponse;
static DCM_ELEMENT CStoreResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CStoreResponse.messageIDRespondedTo), (void *) &CStoreResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CStoreResponse.dataSetType), (void *) &CStoreResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(CStoreResponse.status), (void *) &CStoreResponse.status},
};

static DCM_FLAGGED_ELEMENT CStoreResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CStoreResponse.classUID), (void *) &CStoreResponse.classUID[0], MSG_K_C_STORERESP_CLASSUID, &CStoreResponse.conditionalFields},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(CStoreResponse.instanceUID), (void *) &CStoreResponse.instanceUID[0], MSG_K_C_STORERESP_INSTANCEUID, &CStoreResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(CStoreResponse.errorComment), (void *) &CStoreResponse.errorComment[0], MSG_K_C_STORERESP_ERRORCOMMENT, &CStoreResponse.conditionalFields},
};

static MSG_C_FIND_REQ    CFindRequest;
static DCM_ELEMENT CFindRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CFindRequest.classUID), (void *) &CFindRequest.classUID[0]},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(CFindRequest.messageID), (void *) &CFindRequest.messageID},
    {DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(CFindRequest.priority), (void *) &CFindRequest.priority},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CFindRequest.dataSetType), (void *) &CFindRequest.dataSetType},
};

static MSG_C_FIND_RESP    CFindResponse;
static DCM_ELEMENT CFindResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CFindResponse.classUID), (void *) &CFindResponse.classUID[0]},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CFindResponse.messageIDRespondedTo), (void *) &CFindResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CFindResponse.dataSetType), (void *) &CFindResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(CFindResponse.status), (void *) &CFindResponse.status},
};

static DCM_FLAGGED_ELEMENT CFindResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CFindResponse.classUID), (void *) &CFindResponse.classUID[0], MSG_K_C_FINDRESP_CLASSUID, &CFindResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(CFindResponse.errorComment), (void *) &CFindResponse.errorComment[0], MSG_K_C_FINDRESP_ERRORCOMMENT, &CFindResponse.conditionalFields }
};

static MSG_C_MOVE_REQ    CMoveRequest;
static DCM_ELEMENT CMoveRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CMoveRequest.classUID), (void *) &CMoveRequest.classUID[0]},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(CMoveRequest.messageID), (void *) &CMoveRequest.messageID},
    {DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(CMoveRequest.priority), (void *) &CMoveRequest.priority},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CMoveRequest.dataSetType), (void *) &CMoveRequest.dataSetType},
    {DCM_CMDMOVEDESTINATION, DCM_AE, "", 1, sizeof(CMoveRequest.moveDestination), (void *) &CMoveRequest.moveDestination[0]}
};

static MSG_C_MOVE_RESP    CMoveResponse;
static DCM_ELEMENT CMoveResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CMoveResponse.messageIDRespondedTo), (void *) &CMoveResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CMoveResponse.dataSetType), (void *) &CMoveResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(CMoveResponse.status), (void *) &CMoveResponse.status},
};
static DCM_FLAGGED_ELEMENT CMoveResponseC[] = {
    {DCM_CMDREMAININGSUBOPERATIONS, DCM_US, "", 1, sizeof(CMoveResponse.remainingSubOperations), (void *) &CMoveResponse.remainingSubOperations, MSG_K_C_MOVE_REMAINING, &CMoveResponse.conditionalFields},
    {DCM_CMDCOMPLETEDSUBOPERATIONS, DCM_US, "", 1, sizeof(CMoveResponse.completedSubOperations), (void *) &CMoveResponse.completedSubOperations, MSG_K_C_MOVE_COMPLETED, &CMoveResponse.conditionalFields},
    {DCM_CMDFAILEDSUBOPERATIONS, DCM_US, "", 1,	sizeof(CMoveResponse.failedSubOperations), (void *) &CMoveResponse.failedSubOperations, MSG_K_C_MOVE_FAILED, &CMoveResponse.conditionalFields},
    {DCM_CMDWARNINGSUBOPERATIONS, DCM_US, "", 1, sizeof(CMoveResponse.warningSubOperations), (void *) &CMoveResponse.warningSubOperations, MSG_K_C_MOVE_WARNING, &CMoveResponse.conditionalFields},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CMoveResponse.classUID), (void *) &CMoveResponse.classUID[0], MSG_K_C_MOVERESP_CLASSUID, &CMoveResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(CMoveResponse.errorComment), (void *) &CMoveResponse.errorComment[0], MSG_K_C_MOVERESP_ERRORCOMMENT, &CMoveResponse.conditionalFields }
};

static MSG_C_GET_REQ    CGetRequest;
static DCM_ELEMENT CGetRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CGetRequest.classUID), (void *) &CGetRequest.classUID[0]},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(CGetRequest.messageID), (void *) &CGetRequest.messageID},
    {DCM_CMDPRIORITY, DCM_US, "", 1, sizeof(CGetRequest.priority), (void *) &CGetRequest.priority},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CGetRequest.dataSetType), (void *) &CGetRequest.dataSetType}
};

static MSG_C_GET_RESP	 CGetResponse;
static DCM_ELEMENT CGetResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CGetResponse.messageIDRespondedTo), (void *) &CGetResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CGetResponse.dataSetType), (void *) &CGetResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(CGetResponse.status), (void *) &CGetResponse.status}
};

static DCM_FLAGGED_ELEMENT CGetResponseC[] = {
    {DCM_CMDREMAININGSUBOPERATIONS, DCM_US, "", 1, sizeof(CGetResponse.remainingSubOperations),	(void *) &CGetResponse.remainingSubOperations, MSG_K_C_GET_REMAINING, &CGetResponse.conditionalFields},
    {DCM_CMDCOMPLETEDSUBOPERATIONS, DCM_US, "", 1, sizeof(CGetResponse.completedSubOperations),	(void *) &CGetResponse.completedSubOperations, MSG_K_C_GET_COMPLETED, &CGetResponse.conditionalFields},
    {DCM_CMDFAILEDSUBOPERATIONS, DCM_US, "", 1,	sizeof(CGetResponse.failedSubOperations), (void *) &CGetResponse.failedSubOperations, MSG_K_C_GET_FAILED, &CGetResponse.conditionalFields},
    {DCM_CMDWARNINGSUBOPERATIONS, DCM_US, "", 1, sizeof(CGetResponse.warningSubOperations),	(void *) &CGetResponse.warningSubOperations, MSG_K_C_GET_WARNING, &CGetResponse.conditionalFields},
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(CGetResponse.classUID), (void *) &CGetResponse.classUID[0], MSG_K_C_GETRESP_CLASSUID, &CGetResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(CGetResponse.errorComment),	(void *) &CGetResponse.errorComment[0],	MSG_K_C_GETRESP_ERRORCOMMENT, &CGetResponse.conditionalFields}
};

static MSG_C_CANCEL_REQ   CCancelRequest;
static DCM_ELEMENT CCancelRequestR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(CCancelRequest.messageIDRespondedTo), (void *) &CCancelRequest.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(CCancelRequest.dataSetType), (void *) &CCancelRequest.dataSetType},
};

static MSG_N_EVENT_REPORT_REQ    NEventReportRequest;
static DCM_ELEMENT NEventReportRequestR[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NEventReportRequest.classUID), (void *) &NEventReportRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NEventReportRequest.messageID), (void *) &NEventReportRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NEventReportRequest.dataSetType), (void *) &NEventReportRequest.dataSetType},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NEventReportRequest.affectedInstanceUID), (void *) &NEventReportRequest.affectedInstanceUID[0]},
    {DCM_CMDEVENTTYPEID, DCM_US, "", 1,	sizeof(NEventReportRequest.eventTypeID), (void *) &NEventReportRequest.eventTypeID},
};

static MSG_N_EVENT_REPORT_RESP    NEventReportResponse;
static DCM_ELEMENT NEventReportResponseR[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NEventReportResponse.classUID), (void *) &NEventReportResponse.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NEventReportResponse.messageIDRespondedTo),(void *) &NEventReportResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1,	sizeof(NEventReportResponse.dataSetType), (void *) &NEventReportResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NEventReportResponse.status), (void *) &NEventReportResponse.status}
#if 0
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NEventReportResponse.affectedInstanceUID), (void *) &NEventReportResponse.affectedInstanceUID[0]}
#endif
};

static DCM_FLAGGED_ELEMENT NEventReportResponseC[] = {
    {DCM_CMDEVENTTYPEID, DCM_US, "", 1,	sizeof(NEventReportResponse.eventTypeID), (void *) &NEventReportResponse.eventTypeID,	MSG_K_N_EVENTREPORTRESP_EVENTTYPEID, &NEventReportResponse.conditionalFields},
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NEventReportResponse.requestedClassUID), (void *) &NEventReportResponse.requestedClassUID[0], MSG_K_N_EVENTREPORTRESP_REQUESTEDCLASSUID, &NEventReportResponse.conditionalFields},
#if 0
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NEventReportResponse.affectedInstanceUID),(void *) &NEventReportResponse.affectedInstanceUID[0], MSG_K_N_EVENTREPORTRESP_AFFECTEDINSTANCEUID,	&NEventReportResponse.conditionalFields},
#endif
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NEventReportResponse.requestedInstanceUID), (void *) &NEventReportResponse.requestedInstanceUID[0],	MSG_K_N_EVENTREPORTRESP_REQUESTEDINSTANCEUID, &NEventReportResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NEventReportResponse.errorComment), (void *) &NEventReportResponse.errorComment[0],	MSG_K_N_EVENTREPORTRESP_ERRORCOMMENT, &NEventReportResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NEventReportResponse.errorID), (void *) &NEventReportResponse.errorID, MSG_K_N_EVENTREPORTRESP_ERRORID, &NEventReportResponse.conditionalFields}
};

static MSG_N_GET_REQ    NGetRequest;
static DCM_ELEMENT NGetRequestR[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NGetRequest.classUID), (void *) &NGetRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NGetRequest.messageID), (void *) &NGetRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NGetRequest.dataSetType), (void *) &NGetRequest.dataSetType},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NGetRequest.requestedInstanceUID), (void *) &NGetRequest.requestedInstanceUID[0]}
};

static MSG_N_GET_RESP    NGetResponse;
static DCM_ELEMENT NGetResponseR[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NGetResponse.classUID), (void *) &NGetResponse.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NGetResponse.messageIDRespondedTo), (void *) &NGetResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NGetResponse.dataSetType), (void *) &NGetResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NGetResponse.status), (void *) &NGetResponse.status},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NGetResponse.affectedInstanceUID), (void *) &NGetResponse.affectedInstanceUID[0]},
};

static DCM_FLAGGED_ELEMENT NGetResponseC[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NGetResponse.requestedClassUID),(void *) &NGetResponse.requestedClassUID[0], MSG_K_N_GETRESP_REQUESTEDCLASSUID,	&NGetResponse.conditionalFields},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NGetResponse.requestedInstanceUID),	(void *) &NGetResponse.requestedInstanceUID[0],	MSG_K_N_GETRESP_REQUESTEDINSTANCEUID, &NGetResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NGetResponse.errorComment),	(void *) &NGetResponse.errorComment[0],	MSG_K_N_GETRESP_ERRORCOMMENT, &NGetResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NGetResponse.errorID), (void *) &NGetResponse.errorID,	MSG_K_N_GETRESP_ERRORID, &NGetResponse.conditionalFields}
};

static MSG_N_SET_REQ    NSetRequest;
static DCM_ELEMENT NSetRequestR[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NSetRequest.classUID), (void *) &NSetRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NSetRequest.messageID), (void *) &NSetRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NSetRequest.dataSetType), (void *) &NSetRequest.dataSetType},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NSetRequest.instanceUID), (void *) &NSetRequest.instanceUID[0]}
};
static MSG_N_SET_RESP    NSetResponse;
static DCM_ELEMENT NSetResponseR[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NSetResponse.classUID), (void *) &NSetResponse.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NSetResponse.messageIDRespondedTo), (void *) &NSetResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NSetResponse.dataSetType), (void *) &NSetResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NSetResponse.status), (void *) &NSetResponse.status},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NSetResponse.instanceUID), (void *) &NSetResponse.instanceUID[0]}
};

static DCM_FLAGGED_ELEMENT NSetResponseC[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NSetResponse.requestedClassUID), (void *) &NSetResponse.requestedClassUID[0], MSG_K_N_SETRESP_REQUESTEDCLASSUID, &NSetResponse.conditionalFields},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NSetResponse.requestedInstanceUID), (void *) &NSetResponse.requestedInstanceUID[0],	MSG_K_N_SETRESP_REQUESTEDINSTANCEUID, &NSetResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NSetResponse.errorComment),	(void *) &NSetResponse.errorComment[0],	MSG_K_N_SETRESP_ERRORCOMMENT, &NSetResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NSetResponse.errorID), (void *) &NSetResponse.errorID,	MSG_K_N_SETRESP_ERRORID, &NSetResponse.conditionalFields}
};

static MSG_N_ACTION_REQ    NActionRequest;
static DCM_ELEMENT NActionRequestR[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NActionRequest.classUID), (void *) &NActionRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NActionRequest.messageID), (void *) &NActionRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NActionRequest.dataSetType), (void *) &NActionRequest.dataSetType},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NActionRequest.instanceUID), (void *) &NActionRequest.instanceUID[0]},
    {DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(NActionRequest.actionTypeID), (void *) &NActionRequest.actionTypeID}
};

static MSG_N_ACTION_RESP    NActionResponse;
static DCM_ELEMENT NActionResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NActionResponse.messageIDRespondedTo), (void *) &NActionResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NActionResponse.dataSetType), (void *) &NActionResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NActionResponse.status), (void *) &NActionResponse.status} /*,
    {DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(NActionResponse.actionTypeID), (void *) &NActionResponse.actionTypeID} */
};

static DCM_FLAGGED_ELEMENT NActionResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NActionResponse.classUID), (void *) &NActionResponse.classUID[0], MSG_K_N_ACTIONRESP_AFFECTEDCLASSUID, &NActionResponse.conditionalFields},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NActionResponse.instanceUID), (void *) &NActionResponse.instanceUID[0], MSG_K_N_ACTIONRESP_AFFECTEDINSTANCEUID, &NActionResponse.conditionalFields},

    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NActionResponse.errorComment), (void *) &NActionResponse.errorComment[0], MSG_K_N_ACTIONRESP_ERRORCOMMENT,	&NActionResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NActionResponse.errorID), (void *) &NActionResponse.errorID, MSG_K_N_ACTIONRESP_ERRORID, &NActionResponse.conditionalFields},
    {DCM_CMDACTIONTYPEID, DCM_US, "", 1, sizeof(NActionResponse.actionTypeID), (void *) &NActionResponse.actionTypeID,	MSG_K_N_ACTIONRESP_ACTIONTYPEID, &NActionResponse.conditionalFields}
};

static MSG_N_CREATE_REQ    NCreateRequest;
static DCM_ELEMENT NCreateRequestR[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NCreateRequest.classUID), (void *) &NCreateRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NCreateRequest.messageID), (void *) &NCreateRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NCreateRequest.dataSetType), (void *) &NCreateRequest.dataSetType}
};
static DCM_FLAGGED_ELEMENT NCreateRequestC[] = {
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NCreateRequest.instanceUID), (void *) &NCreateRequest.instanceUID[0], MSG_K_N_CREATEREQ_INSTANCEUID, &NCreateRequest.conditionalFields}
};

static MSG_N_CREATE_RESP    NCreateResponse;
static DCM_ELEMENT NCreateResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NCreateResponse.messageIDRespondedTo), (void *) &NCreateResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NCreateResponse.dataSetType), (void *) &NCreateResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NCreateResponse.status), (void *) &NCreateResponse.status},
};

static DCM_FLAGGED_ELEMENT NCreateResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NCreateResponse.classUID), (void *) &NCreateResponse.classUID[0], MSG_K_N_CREATERESP_AFFECTEDCLASSUID, &NCreateResponse.conditionalFields},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NCreateResponse.instanceUID), (void *) &NCreateResponse.instanceUID[0], MSG_K_N_CREATERESP_AFFECTEDINSTANCEUID, &NCreateResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NCreateResponse.errorComment), (void *) NCreateResponse.errorComment, MSG_K_N_CREATERESP_ERRORCOMMENT, &NCreateResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NCreateResponse.errorID), (void *) &NCreateResponse.errorID,	MSG_K_N_CREATERESP_ERRORID,	&NCreateResponse.conditionalFields}
};

static MSG_N_DELETE_REQ    NDeleteRequest;
static DCM_ELEMENT NDeleteRequestR[] = {
    {DCM_CMDREQUESTEDCLASSUID, DCM_UI, "", 1, sizeof(NDeleteRequest.classUID), (void *) &NDeleteRequest.classUID[0]},
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGID, DCM_US, "", 1, sizeof(NDeleteRequest.messageID), (void *) &NDeleteRequest.messageID},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NDeleteRequest.dataSetType), (void *) &NDeleteRequest.dataSetType},
    {DCM_CMDREQUESTEDINSTANCEUID, DCM_UI, "", 1, sizeof(NDeleteRequest.instanceUID), (void *) &NDeleteRequest.instanceUID[0]}
};

static MSG_N_DELETE_RESP    NDeleteResponse;
static DCM_ELEMENT NDeleteResponseR[] = {
    {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(command), (void *) &command},
    {DCM_CMDMSGIDRESPOND, DCM_US, "", 1, sizeof(NDeleteResponse.messageIDRespondedTo), (void *) &NDeleteResponse.messageIDRespondedTo},
    {DCM_CMDDATASETTYPE, DCM_US, "", 1, sizeof(NDeleteResponse.dataSetType), (void *) &NDeleteResponse.dataSetType},
    {DCM_CMDSTATUS, DCM_US, "", 1, sizeof(NDeleteResponse.status), (void *) &NDeleteResponse.status}
};

static DCM_FLAGGED_ELEMENT NDeleteResponseC[] = {
    {DCM_CMDAFFECTEDCLASSUID, DCM_UI, "", 1, sizeof(NDeleteResponse.classUID), (void *) &NDeleteResponse.classUID[0], MSG_K_N_DELETERESP_AFFECTEDCLASSUID, &NDeleteResponse.conditionalFields},
    {DCM_CMDAFFECTEDINSTANCEUID, DCM_UI, "", 1,	sizeof(NDeleteResponse.instanceUID), (void *) &NDeleteResponse.instanceUID[0], MSG_K_N_DELETERESP_AFFECTEDINSTANCEUID,	&NDeleteResponse.conditionalFields},
    {DCM_CMDERRORCOMMENT, DCM_LO, "", 1, sizeof(NDeleteResponse.errorComment), (void *) &NDeleteResponse.errorComment[0], MSG_K_N_DELETERESP_ERRORCOMMENT, &NDeleteResponse.conditionalFields},
    {DCM_CMDERRORID, DCM_US, "", 1,	sizeof(NDeleteResponse.errorID), (void *) &NDeleteResponse.errorID, MSG_K_N_DELETERESP_ERRORID, &NDeleteResponse.conditionalFields}
};

static MSG_REFERENCED_ITEM    referencedItem;
static DCM_ELEMENT referencedItemR[] = {
    {DCM_IDREFERENCEDSOPCLASSUID, DCM_UI, "", 1, sizeof(referencedItem.classUID), (void *) &referencedItem.classUID[0]},
    {DCM_IDREFERENCEDSOPINSTUID, DCM_UI, "", 1, sizeof(referencedItem.instanceUID), (void *) &referencedItem.instanceUID[0]}
};

/* Now that we have defined memory and tables for each of the messages,
** the structure below allows us to do table lookups to find the proper
** tables for a given COMMAND or MSG type.
*/

typedef struct {
    MSG_TYPE 				type;				/* One of our enumerated types */
    unsigned short 			command;			/* As defined in Part 7 of the Std */
    DCM_ELEMENT 			*required;			/* List of required elements */
    int 					requiredCount;		/* Number of required elements */
    DCM_FLAGGED_ELEMENT 	*conditional;		/* List of conditional elements */
    int 					conditionalCount;	/* Number of conditional elements */
    void 					*structure;			/* Pointer to the structure for cmd */
    size_t 					structureSize;		/* Size of the command structure */
}   MESSAGE_TABLE;

static MESSAGE_TABLE messageTable[] = {
    {MSG_K_C_ECHO_REQ, DCM_ECHO_REQUEST, CEchoRequestR, (int) DIM_OF(CEchoRequestR), NULL, 0, &CEchoRequest, sizeof(CEchoRequest)}, /* C-ECHO Request */
    {MSG_K_C_ECHO_RESP, DCM_ECHO_RESPONSE, CEchoResponseR, (int) DIM_OF(CEchoResponseR), CEchoResponseC, (int) DIM_OF(CEchoResponseC), &CEchoResponse, sizeof(CEchoResponse)}, /* C-ECHO Response */
    {MSG_K_C_STORE_REQ, DCM_STORE_REQUEST, CStoreRequestR, (int) DIM_OF(CStoreRequestR), CStoreRequestC, (int) DIM_OF(CStoreRequestC), &CStoreRequest, sizeof(CStoreRequest)}, /* C-STORE Request */
    {MSG_K_C_STORE_RESP, DCM_STORE_RESPONSE, CStoreResponseR, (int) DIM_OF(CStoreResponseR),	CStoreResponseC, (int) DIM_OF(CStoreResponseC), &CStoreResponse, sizeof(CStoreResponse)}, /* C-STORE Response */
    {MSG_K_C_FIND_REQ, DCM_FIND_REQUEST, CFindRequestR, (int) DIM_OF(CFindRequestR), NULL, 0, &CFindRequest, sizeof(CFindRequest)}, /* C-FIND Request */
    {MSG_K_C_FIND_RESP, DCM_FIND_RESPONSE, CFindResponseR, (int) DIM_OF(CFindResponseR), CFindResponseC, (int) DIM_OF(CFindResponseC), &CFindResponse, sizeof(CFindResponse)}, /* C-FIND Response */
    {MSG_K_C_MOVE_REQ, DCM_MOVE_REQUEST, CMoveRequestR, (int) DIM_OF(CMoveRequestR), NULL, 0, &CMoveRequest, sizeof(CMoveRequest)}, /* C-MOVE Request */
    {MSG_K_C_MOVE_RESP, DCM_MOVE_RESPONSE, CMoveResponseR, (int) DIM_OF(CMoveResponseR), CMoveResponseC, (int) DIM_OF(CMoveResponseC), &CMoveResponse, sizeof(CMoveResponse)}, /* C-MOVE Response */
    {MSG_K_C_GET_REQ, DCM_GET_REQUEST, CGetRequestR, (int) DIM_OF(CGetRequestR), NULL, 0, &CGetRequest, sizeof(CGetRequest)}, /* C-GET Request */
    {MSG_K_C_GET_RESP, DCM_GET_RESPONSE, CGetResponseR, (int) DIM_OF(CGetResponseR), CGetResponseC, (int) DIM_OF(CGetResponseC), &CGetResponse, sizeof(CGetResponse)}, /* C-GET Response */
    {MSG_K_C_CANCEL_REQ, DCM_CANCEL_REQUEST, CCancelRequestR, (int) DIM_OF(CCancelRequestR), NULL, 0, &CCancelRequest, sizeof(CCancelRequest)}, /* C-CANCEL Request */

    {MSG_K_N_EVENT_REPORT_REQ, DCM_N_EVENT_REPORT_REQUEST, NEventReportRequestR, (int) DIM_OF(NEventReportRequestR), NULL, 0, &NEventReportRequest, sizeof(NEventReportRequest)}, /* N-EVENT Report */
    {MSG_K_N_EVENT_REPORT_RESP, DCM_N_EVENT_REPORT_RESPONSE, NEventReportResponseR, (int) DIM_OF(NEventReportResponseR), NEventReportResponseC, (int) DIM_OF(NEventReportResponseC), &NEventReportResponse, sizeof(NEventReportResponse)}, /* N-EVENT Rep */
    {MSG_K_N_GET_REQ, DCM_N_GET_REQUEST, NGetRequestR, (int) DIM_OF(NGetRequestR), NULL, 0, &NGetRequest, sizeof(NGetRequest)}, /* N-GET Request */
    {MSG_K_N_GET_RESP, DCM_N_GET_RESPONSE, NGetResponseR, (int) DIM_OF(NGetResponseR),	NGetResponseC, (int) DIM_OF(NGetResponseC), &NGetResponse, sizeof(NGetResponse)}, /* N-GET Response */
    {MSG_K_N_SET_REQ, DCM_N_SET_REQUEST, NSetRequestR, (int) DIM_OF(NSetRequestR), NULL, 0, &NSetRequest, sizeof(NSetRequest)}, /* N-SET Request */
    {MSG_K_N_SET_RESP, DCM_N_SET_RESPONSE, NSetResponseR, (int) DIM_OF(NSetResponseR),	NSetResponseC, (int) DIM_OF(NSetResponseC), &NSetResponse, sizeof(NSetResponse)}, /* N-SET Response */
    {MSG_K_N_ACTION_REQ, DCM_N_ACTION_REQUEST, NActionRequestR, (int) DIM_OF(NActionRequestR), NULL, 0, &NActionRequest, sizeof(NActionRequest)}, /* N-ACTION Request */
    {MSG_K_N_ACTION_RESP, DCM_N_ACTION_RESPONSE, NActionResponseR, (int) DIM_OF(NActionResponseR), NActionResponseC, (int) DIM_OF(NActionResponseC), &NActionResponse, sizeof(NActionResponse)}, /* N-ACTION Response */
    {MSG_K_N_CREATE_REQ, DCM_N_CREATE_REQUEST, NCreateRequestR, (int) DIM_OF(NCreateRequestR),	NCreateRequestC, (int) DIM_OF(NCreateRequestC), &NCreateRequest, sizeof(NCreateRequest)}, /* N-CREATE Request */
    {MSG_K_N_CREATE_RESP, DCM_N_CREATE_RESPONSE, NCreateResponseR, (int) DIM_OF(NCreateResponseR), NCreateResponseC, (int) DIM_OF(NCreateResponseC), &NCreateResponse, sizeof(NCreateResponse)}, /* N-CREATE Response */
    {MSG_K_N_DELETE_REQ, DCM_N_DELETE_REQUEST, NDeleteRequestR, (int) DIM_OF(NDeleteRequestR), NULL, 0, &NDeleteRequest, sizeof(NDeleteRequest)}, /* N-DELETE Request */
    {MSG_K_N_DELETE_RESP, DCM_N_DELETE_RESPONSE, NDeleteResponseR, (int) DIM_OF(NDeleteResponseR), NDeleteResponseC, (int) DIM_OF(NDeleteResponseC), &NDeleteResponse, sizeof(NDeleteResponse)}, /* N-DELETE Response */
    {MSG_K_REFERENCED_ITEM, 0, referencedItemR, (int) DIM_OF(referencedItemR), NULL, 0, &referencedItem, sizeof(referencedItem)} /* Referenced item */
};

static CONDITION
buildObject(MSG_GENERAL * msg, MESSAGE_TABLE * table, DCM_OBJECT ** object);
static CONDITION
parseCommand(DCM_OBJECT ** object, MESSAGE_TABLE * table, unsigned short cmd, void **message);


/* MSG_BuildCommand
**
** Purpose:
**	MSG_BuildCommand builds the COMMAND group of a DICOM message.  The
**	caller passes the address of one of the defined MSG structures and
**	the address of a DICOM Object.  The caller is expected to have
**	filled in the MSG structure with all of the required attributes.
**	If the structure includes any conditional attributes, these are
**	noted with the appropriate flag.
**
**	This routine checks the message type against its list of known
**	messages.  When the proper message type is found via a table lookup,
**	the DICOM Object is created and populated with the appropriate
**	required and conditional attributes.
**
** Parameter Dictionary:
**	message		Pointer to one of the defined MSG structures supported
**			by this facility.  Caller has filled in data fields
**			before calling this function and requests that the
**			function translate the structure into a DICOM Object.
**	object		Address of caller's pointer to a DICOM Object.  The
**			function will create a new object and add the
**			appropriate elements.
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildCommand(void *message, DCM_OBJECT ** object)
{
    MSG_GENERAL		* msg;
    int		        index;

    msg = (MSG_GENERAL *) message;

    for (index = 0; index < (int) DIM_OF(messageTable); index++) {
    	if (msg->type == messageTable[index].type) return buildObject(msg, &messageTable[index], object);
    }
    return COND_PushCondition(MSG_ILLEGALMESSAGETYPE, MSG_Message(MSG_ILLEGALMESSAGETYPE), (int) msg->type, "MSG_BuildCommand");
}

/* MSG_ParseCommand
**
** Purpose:
**	MSG_ParseCommand parses a DICOM Object and places the attribute values
**	in fixed structures.  The caller passes a DICOM Object which is
**	assumed to contain a DICOM Command (data in the command group).  This
**	function extracts the DICOM Command value from the DICOM Object and
**	performs a table lookup to determine if the command can be parsed by
**	this function.  If the command is supported by this function, a
**	private function is called which parses the DICOM Object and places
**	the data values in the MSG structure.
**
**	The caller's second argument is the address of a pointer to
**	a MSG structure.  If the caller's pointer is NULL, this function
**	allocates a structure of the proper size and writes the address
**	of the structure in the caller's memory.  If the pointer is not
**	NULL, we assume the caller has allocated memory for the structure.
**
**	The size and type of structure depends on the command value which
**	is extracted from the COMMAND group.  If the caller asks this routine
**	to allocate memory for a MSG structure, the structure should be
**	freed with the MSG_Free routine.
**
** Parameter Dictionary:
**	object		Address of a DICOM Object pointer which holds the
**			DICOM Command to be parsed.
**	msg		Address of a pointer to a MSG structure.  The pointer
**			should point to pre-existing memory or NULL.
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_ParseCommand(DCM_OBJECT ** object, void **message)
{
    CONDITION 		cond;
    unsigned short 	cmd;
    DCM_ELEMENT 	e = {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(cmd), 0};
    int			    index;
    U32				l;
    void	        *ctx;

    e.d.us = &cmd;
    ctx = NULL;
    cond = DCM_GetElementValue(object, &e, &l, &ctx);
    if (cond != DCM_NORMAL)	return COND_PushCondition(MSG_NOCOMMANDELEMENT, MSG_Message(MSG_NOCOMMANDELEMENT), "MSG_ParseCommand");

    for (index = 0; index < (int) DIM_OF(messageTable); index++) {
    	if (cmd == messageTable[index].command) return parseCommand(object, &messageTable[index], cmd, message);
    }
    return COND_PushCondition(MSG_UNSUPPORTEDCOMMAND, MSG_Message(MSG_UNSUPPORTEDCOMMAND), (int) cmd, "MSG_ParseCommand");
}

static MSG_STATUS_DESCRIPTION statusTbl[] = {
    {MSG_K_C_STORE_OUTOFRESOURCES, 0xff00, MSG_K_C_STORE_RESP, MSG_K_CLASS_REFUSED, "Out of Resources"},
    {MSG_K_C_STORE_SOPCLASSNOTSUPPORTED, 0xff00, MSG_K_C_STORE_RESP, MSG_K_CLASS_REFUSED, "SOP Class Not Supported"},
    {MSG_K_C_STORE_DATASETNOTMATCHSOPCLASSERROR, 0xff00, MSG_K_C_STORE_RESP, MSG_K_CLASS_FAILURE, "Data Set does not match SOP Class (Failure)"},
    {MSG_K_C_STORE_CANNOTUNDERSTAND, 0xf000, MSG_K_C_STORE_RESP, MSG_K_CLASS_FAILURE, "Cannot Understand"},
    {MSG_K_C_STORE_DATAELEMENTCOERCION, 0xffff, MSG_K_C_STORE_RESP, MSG_K_CLASS_WARNING, "Coercion of Data Elements"},
    {MSG_K_C_STORE_DATASETNOTMATCHSOPCLASSWARN, 0xffff, MSG_K_C_STORE_RESP, MSG_K_CLASS_WARNING, "Data Set does not match SOP Class (Warning)"},
    {MSG_K_C_STORE_ELEMENTSDISCARDED, 0xffff, MSG_K_C_STORE_RESP, MSG_K_CLASS_WARNING, "Elements Discarded"},

    {MSG_K_C_FIND_OUTOFRESOURCES, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_REFUSED, "Out of Resources"},
    {MSG_K_C_FIND_SOPCLASSNOTSUPPORTED, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_REFUSED, "SOP Class Not Supported"},
    {MSG_K_C_FIND_IDENTIFIERNOTMATCHSOPCLASS, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_FAILURE, "Identifier Does Not Match SOP Class"},
    {MSG_K_C_FIND_UNABLETOPROCESS, 0xf000, MSG_K_C_FIND_RESP, MSG_K_CLASS_FAILURE, "Unable to process"},
    {MSG_K_C_FIND_MATCHCANCELLED, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_CANCEL, "Matching terminated due to Cancel Request"},
    {MSG_K_C_FIND_MATCHCONTINUING, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_PENDING, "Match continuing; all keys supported"},
    {MSG_K_C_FIND_MATCHCONTINUINGWARN, 0xffff, MSG_K_C_FIND_RESP, MSG_K_CLASS_PENDING, "Match continuing; some optional keys unsupported"},

    {MSG_K_C_MOVE_UNABLETOCACULATEMATCHCOUNT, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_REFUSED, "Out of resources; cannot calculate match count"},
    {MSG_K_C_MOVE_UNABLETOPERFORMSUBOPERATIONS, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_REFUSED, "Out of resources; cannot perform sub-operations"},
    {MSG_K_C_MOVE_SOPCLASSNOTSUPPORTED, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_REFUSED, "SOP Class not supported"},
    {MSG_K_C_MOVE_MOVEDESTINATIONUNKNOWN, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_REFUSED, "Move Destination unknown"},
    {MSG_K_C_MOVE_IDENTIFIERNOTMATCHSOPCLASS, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_FAILURE, "Identifier does not match SOP Class"},
    {MSG_K_C_MOVE_UNABLETOPROCESS, 0xf000, MSG_K_C_MOVE_RESP, MSG_K_CLASS_FAILURE, "Unable to process"},
    {MSG_K_C_MOVE_SUBOPERATIONSCANCELLED, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_CANCEL, "Sub-operations terminated by cancel request"},
    {MSG_K_C_MOVE_COMPLETEWITHFAILURES, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_WARNING, "Sub-operations complete; one or more failures"},
    {MSG_K_C_MOVE_SUBOPERATIONSCONTINUING, 0xffff, MSG_K_C_MOVE_RESP, MSG_K_CLASS_PENDING, "Sub-operations continuing"},

    {MSG_K_C_GET_UNABLETOCACULATEMATCHCOUNT, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_REFUSED, "Out of resources; cannot calculate match count"},
    {MSG_K_C_GET_UNABLETOPERFORMSUBOPERATIONS, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_REFUSED, "Out of resources; cannot perform sub-operations"},
    {MSG_K_C_GET_SOPCLASSNOTSUPPORTED, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_REFUSED, "SOP Class not supported"},
    {MSG_K_C_GET_IDENTIFIERNOTMATCHSOPCLASS, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_FAILURE, "Identifier does not match SOP Class"},
    {MSG_K_C_GET_UNABLETOPROCESS, 0xf000, MSG_K_C_GET_RESP, MSG_K_CLASS_FAILURE, "Unable to process"},
    {MSG_K_C_GET_SUBOPERATIONSCANCELLED, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_CANCEL, "Sub-operations terminated by cancel request"},
    {MSG_K_C_GET_COMPLETEWITHFAILURES, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_WARNING, "Sub-operations complete; one or more failures"},
    {MSG_K_C_GET_SUBOPERATIONSCONTINUING, 0xffff, MSG_K_C_GET_RESP, MSG_K_CLASS_PENDING, "Sub-operations continuing"},

    {MSG_K_N_ACTION_UNABLETOUPDATE, 0xffff, MSG_K_N_ACTION_RESP, MSG_K_CLASS_REFUSED, "Refused: GP-SPS may no longer be updated"},
    {MSG_K_N_ACTION_WRONGTRANSACTIOUID, 0xffff, MSG_K_N_ACTION_RESP, MSG_K_CLASS_REFUSED, "Refused: wrong transaction UID used"},
    {MSG_K_N_ACTION_ALREADYINPROGRESS, 0xffff, MSG_K_N_ACTION_RESP, MSG_K_CLASS_REFUSED, "Refused: GP-SPS already in progress"},

/*  This section of messages goes last.  It is overridden by status codes for the various message types. */
    {MSG_K_SUCCESS, 0xffff, MSG_K_NONE, MSG_K_CLASS_SUCCESS, "Successful operation"},
    {MSG_K_CANCEL, 0xffff, MSG_K_NONE, MSG_K_CLASS_CANCEL, "Operation canceled"},
    {MSG_K_ATTRIBUTELISTERRORR, 0xffff, MSG_K_NONE, MSG_K_CLASS_WARNING, "Attribute List Error"},
    {MSG_K_CLASSINSTANCECONFLICT, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Class-Instance Conflict"},
    {MSG_K_DUPLICATESOPINSTANCE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Duplicate SOP Instance"},
    {MSG_K_DUPLICATEINVOCATION, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Duplicate Invocation"},
    {MSG_K_INVALIDARGUMENTVALUE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Invalid Argument Value"},
    {MSG_K_INVALIDATTRIBUTEVALUE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Invalid Attribute Value"},
    {MSG_K_INVALIDOBJECTINSTANCE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Invalid Object Instance"},
    {MSG_K_MISSINGATTRIBUTE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Missing Attribute"},
    {MSG_K_MISSINGATTRIBUTEVALUE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Missing Attribute Value"},
    {MSG_K_MISTYPEDARGUMENT, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Mistyped Argument"},
    {MSG_K_NOSUCHARGUMENT, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "No Such Argument"},
    {MSG_K_NOSUCHATTRIBUTE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "No Such Attribute"},
    {MSG_K_NOSUCHEVENTTYPE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "No Such Event Type"},
    {MSG_K_NOSUCHOBJECTINSTANCE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "No Such Object Instance"},
    {MSG_K_NOSUCHSOPCLASS, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "No Such SOP Class"},
    {MSG_K_PROCESSINGFAILURE, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Processing Failure"},
    {MSG_K_RESOURCELIMITATION, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Resource Limitation"},
    {MSG_K_UNRECOGNIZEDOPERATION, 0xffff, MSG_K_NONE, MSG_K_CLASS_FAILURE, "Unrecognized Operation"},
};

CONDITION
MSG_StatusLookup(unsigned short code, MSG_TYPE messageType, MSG_STATUS_DESCRIPTION * statusDescription)
{
    int	        index;
    CTNBOOLEAN	codeMatch, msgMatch;

    for (index = 0; index < (int) DIM_OF(statusTbl); index++) {
    	codeMatch = (statusTbl[index].code == (code & statusTbl[index].mask));
    	msgMatch = ((statusTbl[index].messageType == messageType) || statusTbl[index].messageType == MSG_K_NONE);

    	if (codeMatch && msgMatch) {
    		*statusDescription = statusTbl[index];
    		statusDescription->messageType = messageType;
    		return MSG_NORMAL;
    	}
    }
    return COND_PushCondition(MSG_STATUSCODENOTFOUND, MSG_Message(MSG_STATUSCODENOTFOUND), (unsigned long) code, "MSG_StatusLookup");
}

/* Private functions defined below.
** -----------------------------------
*/

/* buildObject
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
buildObject(MSG_GENERAL * msg, MESSAGE_TABLE * table, DCM_OBJECT ** object)
{
    CONDITION 	cond;
    CONDITION 	returnCondition = MSG_NORMAL;

    cond = DCM_CreateObject(object, 0);
    if (cond != DCM_NORMAL)	return COND_PushCondition(MSG_OBJECTCREATEFAILED, MSG_Message(MSG_OBJECTCREATEFAILED), "buildObject");

#ifdef CTN_USE_THREADS
    cond = THR_ObtainMutex(FAC_MSG);
    if (cond != THR_NORMAL) {
    	returnCondition = COND_PushCondition(MSG_MUTEXFAILED, MSG_Message(MSG_MUTEXFAILED), "buildObject");
    	goto releaseMutex;
    }
#endif

    (void) memcpy(table->structure, msg, table->structureSize);
    command = table->command;

    cond = DCM_ModifyElements(object, table->required, table->requiredCount, table->conditional, table->conditionalCount, NULL);
    if (cond != DCM_NORMAL) {
    	returnCondition = COND_PushCondition(MSG_MODIFICATIONFAILURE, MSG_Message(MSG_MODIFICATIONFAILURE), "buildObject");
    	goto releaseMutex;
    }
    if ((msg->type == MSG_K_N_GET_REQ) || (msg->type == MSG_K_N_GET_RESP) || (msg->type == MSG_K_N_SET_RESP) || (msg->type == MSG_K_N_CREATE_RESP)) {

    	DCM_ELEMENT ele = {DCM_CMDATTRIBUTEIDLIST, DCM_AT, "", 1, 0, NULL};
    	int 	count = 0;

    	switch (msg->type) {
			case MSG_K_N_GET_REQ:
										count = ((MSG_N_GET_REQ *) msg)->attributeCount;
										ele.d.at = ((MSG_N_GET_REQ *) msg)->attributeList;
										break;
			case MSG_K_N_GET_RESP:
										count = ((MSG_N_GET_RESP *) msg)->attributeCount;
										ele.d.at = ((MSG_N_GET_RESP *) msg)->attributeIdentifierList;
										break;
			case MSG_K_N_SET_RESP:
										count = ((MSG_N_SET_RESP *) msg)->attributeCount;
										ele.d.at = ((MSG_N_SET_RESP *) msg)->attributeIdentifierList;
										break;
			case MSG_K_N_CREATE_RESP:
										count = ((MSG_N_CREATE_RESP *) msg)->attributeCount;
										ele.d.at = ((MSG_N_CREATE_RESP *) msg)->attributeIdentifierList;
										break;
    	}

    	if (count != 0) {
    		ele.length = count * sizeof(DCM_TAG);

    		cond = DCM_ModifyElements(object, &ele, 1, NULL, 0, NULL);
    		if (cond != DCM_NORMAL) {
    			returnCondition = COND_PushCondition(MSG_MODIFICATIONFAILURE, MSG_Message(MSG_MODIFICATIONFAILURE), "buildObject");
    			goto releaseMutex;
    		}
    	}
    }else if (msg->type == MSG_K_C_MOVE_RESP){
#if STANDARD_VERSION < VERSION_JUL1993
    	/* No code here anymore */
#endif
    }
releaseMutex:
#ifdef CTN_USE_THREADS
    cond = THR_ReleaseMutex(FAC_MSG);
    if (cond != THR_NORMAL)	returnCondition = COND_PushCondition(MSG_MUTEXFAILED, MSG_Message(MSG_MUTEXFAILED), "buildObject");

#endif
    return returnCondition;
}

/* parseCommand
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
parseCommand(DCM_OBJECT ** object, MESSAGE_TABLE * table, unsigned short cmd, void **message)
{
    CONDITION 		cond;
    CONDITION 		returnCondition = MSG_NORMAL;
    MSG_GENERAL		* msg;
#if STANDARD_VERSION < VERSION_JUL1993
    int		        index;
#endif
    U32				l;
    void	        *ctx;

#ifdef CTN_USE_THREADS
    cond = THR_ObtainMutex(FAC_MSG);
    if (cond != THR_NORMAL) {
    	returnCondition = COND_PushCondition(MSG_MUTEXFAILED, MSG_Message(MSG_MUTEXFAILED), "parseCommand");
    	goto releaseMutex;
    }
#endif

    if (*message == NULL) {
    	*message = CTN_MALLOC(table->structureSize);
    	if (*message == NULL) {
    		returnCondition = COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), table->structureSize, "parseCommand");
    		goto releaseMutex;
    	}
    }
    (void) memset(table->structure, 0, table->structureSize);

    cond = DCM_ParseObject(object, table->required, table->requiredCount, table->conditional, table->conditionalCount, NULL);
    if (cond != DCM_NORMAL) {
    	returnCondition = COND_PushCondition(MSG_PARSEFAILED, MSG_Message(MSG_PARSEFAILED), (int) table->type, "parseCommand");
    	goto releaseMutex;
    }
    msg = (MSG_GENERAL *) * message;
    (void) memcpy(msg, table->structure, table->structureSize);

    if ((table->type == MSG_K_N_GET_REQ) ||	(table->type == MSG_K_N_GET_RESP) || (table->type == MSG_K_N_SET_RESP) || (table->type == MSG_K_N_CREATE_RESP)) {

    	DCM_ELEMENT 		e = {DCM_CMDATTRIBUTEIDLIST, DCM_AT, "", 1, 0, NULL};
    	MSG_N_GET_REQ 		*ngetRequest;
    	MSG_N_GET_RESP 		*ngetResponse;
    	MSG_N_SET_RESP 		*nsetResponse;
    	MSG_N_CREATE_RESP 	*ncreateResponse;

    	cond = DCM_GetElementSize(object, DCM_CMDATTRIBUTEIDLIST, &l);

    	switch (table->type) {
			case MSG_K_N_GET_REQ:
										ngetRequest = (MSG_N_GET_REQ *) msg;
										if (cond != DCM_NORMAL) {
											(void) COND_PopCondition(FALSE);
											ngetRequest->attributeList = NULL;
											ngetRequest->attributeCount = 0;
										}else{
											ngetRequest->attributeList = CTN_MALLOC(l);
											if (ngetRequest->attributeList == NULL)
												return COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), l, "parseCommand");
											ngetRequest->attributeCount = l / sizeof(DCM_TAG);
											e.length = l;
											e.d.at = ngetRequest->attributeList;
											ctx = NULL;

											cond = DCM_GetElementValue(object, &e, &l, &ctx);
											if (cond != DCM_NORMAL) {
												returnCondition = COND_PushCondition(MSG_OBJECTACCESSERROR, MSG_Message(MSG_OBJECTACCESSERROR), (int) table->type, "parseCommand");
												goto releaseMutex;
											}
										}
										break;
			case MSG_K_N_GET_RESP:
										ngetResponse = (MSG_N_GET_RESP *) msg;
										if (cond != DCM_NORMAL) {
											(void) COND_PopCondition(FALSE);
											ngetResponse->attributeIdentifierList = NULL;
											ngetResponse->attributeCount = 0;
										}else{
											ngetResponse->attributeIdentifierList = CTN_MALLOC(l);
											if (ngetResponse->attributeIdentifierList == NULL) {
												returnCondition = COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), l, "parseCommand");
												goto releaseMutex;
											}
											ngetResponse->attributeCount = l / sizeof(DCM_TAG);
											ngetResponse->conditionalFields |=  MSG_K_N_GETRESP_ATTRIBUTEIDENTIFIERLIST;
											e.length = l;
											e.d.at = ngetResponse->attributeIdentifierList;
											ctx = NULL;

											cond = DCM_GetElementValue(object, &e, &l, &ctx);
											if (cond != DCM_NORMAL) {
												returnCondition = COND_PushCondition(MSG_OBJECTACCESSERROR, MSG_Message(MSG_OBJECTACCESSERROR), (int) table->type, "parseCommand");
												goto releaseMutex;
											}
										}
										break;
			case MSG_K_N_SET_RESP:
										nsetResponse = (MSG_N_SET_RESP *) msg;
										if (cond != DCM_NORMAL) {
											(void) COND_PopCondition(FALSE);
											nsetResponse->attributeIdentifierList = NULL;
											nsetResponse->attributeCount = 0;
										}else{
											nsetResponse->attributeIdentifierList = CTN_MALLOC(l);
											if (nsetResponse->attributeIdentifierList == NULL) {
												returnCondition = COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), l, "parseCommand");
												goto releaseMutex;
											}
											nsetResponse->attributeCount = l / sizeof(DCM_TAG);
											nsetResponse->conditionalFields |= MSG_K_N_SETRESP_ATTRIBUTEIDENTIFIERLIST;
											e.length = l;
											e.d.at = nsetResponse->attributeIdentifierList;
											ctx = NULL;

											cond = DCM_GetElementValue(object, &e, &l, &ctx);
											if (cond != DCM_NORMAL) {
												returnCondition = COND_PushCondition(MSG_OBJECTACCESSERROR, MSG_Message(MSG_OBJECTACCESSERROR), (int) table->type, "parseCommand");
												goto releaseMutex;
											}
										}
										break;
			case MSG_K_N_CREATE_RESP:
										ncreateResponse = (MSG_N_CREATE_RESP *) msg;
										if (cond != DCM_NORMAL) {
											(void) COND_PopCondition(FALSE);
											ncreateResponse->attributeIdentifierList = NULL;
											ncreateResponse->attributeCount = 0;
										}else{
											ncreateResponse->attributeIdentifierList = CTN_MALLOC(l);
											if (ncreateResponse->attributeIdentifierList == NULL)
												return COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), l, "parseCommand");
											ncreateResponse->attributeCount = l / sizeof(DCM_TAG);
											ncreateResponse->conditionalFields |= MSG_K_N_CREATERESP_ATTRIBUTEIDENTIFIERLIST;
											e.length = l;
											e.d.at = ncreateResponse->attributeIdentifierList;
											ctx = NULL;

											cond = DCM_GetElementValue(object, &e, &l, &ctx);
											if (cond != DCM_NORMAL) {
												returnCondition = COND_PushCondition(MSG_OBJECTACCESSERROR, MSG_Message(MSG_OBJECTACCESSERROR), (int) table->type, "parseCommand");
												goto releaseMutex;
											}
										}
										break;
    	}
    }else if (table->type == MSG_K_C_MOVE_RESP){

    	MSG_C_MOVE_RESP *moveResponse;
#if STANDARD_VERSION < VERSION_JUL1993
#else
    	moveResponse = (MSG_C_MOVE_RESP *) msg;
    	moveResponse->dataSet = NULL;
#endif
    }

    msg->type = table->type;

releaseMutex:
#ifdef CTN_USE_THREADS
    cond = THR_ReleaseMutex(FAC_MSG);
    if (cond != THR_NORMAL) {
    	returnCondition = COND_PushCondition(MSG_MUTEXFAILED, MSG_Message(MSG_MUTEXFAILED), "parseCommand");
    	goto releaseMutex;
    }
#endif
    return returnCondition;
}
