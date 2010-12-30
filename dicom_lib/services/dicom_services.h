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
** Module Name(s):
** Author, Date:	Stephen M. Moore, 15-Apr-93
** Intent:		This is the include file for the DICOM SERVICES
**			facility.  This facility provides functions for
**			implementing the services which are defined in
**			Part 4 of the DICOM standard.  This file defines
**			constants required for that facility and function
**			prototypes.
** Last Update:		$Author: smm $, $Date: 2002/02/26 22:41:25 $
** Source File:		$RCSfile: dicom_services.h,v $
** Revision:		$Revision: 1.39 $
** Status:		$State: Exp $
*/

#ifndef DICOM_SERVICES_IS_IN
#define DICOM_SERVICES_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

/* Define the function prototypes for this set of routines.
** The first set defines initialization routines for using these
** services as a user or provider.
*/
CONDITION
SRV_AcceptServiceClass(DUL_PRESENTATIONCONTEXT * requestedCtx, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params);
CONDITION
SRV_AcceptSOPClass(DUL_PRESENTATIONCONTEXT * requestedCtx, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params, char** xferSyntaxes, int xferSyntaxCount, int isStorageClass);
CONDITION
SRV_RejectServiceClass(DUL_PRESENTATIONCONTEXT * requestedCtx, unsigned short result, DUL_ASSOCIATESERVICEPARAMETERS * params);
CONDITION
SRV_RequestServiceClass(const char *SOPClass, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params);
CONDITION
SRV_RegisterSOPClass(const char *SOPClass, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params);
CONDITION
SRV_RegisterSOPClassXfer(const char *SOPClass, const char* xferSyntax, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params);
CONDITION
SRV_ProposeSOPClassWithXfer(const char*SOPClass, DUL_SC_ROLE role, char** xferSyntaxes, int xferSyntaxCount, int isStorageClass, DUL_ASSOCIATESERVICEPARAMETERS* params);
CONDITION
SRV_ReceiveCommand(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_BLOCKOPTIONS block, int timeout, DUL_PRESENTATIONCONTEXTID * ctxId, unsigned short *command, MSG_TYPE * messageType, void **messageArg);
CONDITION
SRV_TestForCancel(DUL_ASSOCIATIONKEY ** association, DUL_BLOCKOPTIONS block, int timeout, DUL_PRESENTATIONCONTEXTID ctxID, unsigned short *command, MSG_TYPE * messageType, void **messageArg);
CONDITION
SRV_ReceiveDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, DUL_BLOCKOPTIONS block, int timeout, char *dirName, DCM_OBJECT ** dataSet);
CONDITION
SRV_SendCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object);
CONDITION
SRV_SendDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object, CONDITION(*callback) (), void *callbackCtx, unsigned long length);
char *SRV_Message(CONDITION cond);
void SRV_Debug(CTNBOOLEAN flag);

/*
** Prototypes for two functions which keep track of message IDs
** in a global context.
*/

unsigned short SRV_MessageIDOut(void);
void SRV_MessageIDIn(unsigned short messageID);

/* Now, define the prototypes for the various classes of services.
*/

typedef CONDITION(SRV_C_ECHO_REQ_CALLBACK) (MSG_C_ECHO_REQ * echoRequest, MSG_C_ECHO_RESP * echoResonse, void *ctx);
CONDITION
SRV_CEchoRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_ECHO_REQ * echoRequest, MSG_C_ECHO_RESP * echoResponse, SRV_C_ECHO_REQ_CALLBACK * cEchoCallback, void *ctx, char *dirName);

typedef CONDITION(SRV_C_ECHO_RESP_CALLBACK) (MSG_C_ECHO_REQ * echoRequest, MSG_C_ECHO_RESP * echoResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_CEchoResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_C_ECHO_REQ ** echoRequest, MSG_C_ECHO_RESP * echoReply, SRV_C_ECHO_RESP_CALLBACK * cEchoCallback, void *ctx, char *dirName);

typedef CONDITION(SRV_C_STORE_REQ_CALLBACK) (MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResonse, unsigned long bytesTransmitted,  unsigned long totalBytes, void *ctx);
CONDITION
SRV_CStoreRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResponse, SRV_C_STORE_REQ_CALLBACK * callback, void *callbackCtx, char *dirName);

typedef CONDITION(SRV_C_STORE_RESP_CALLBACK) (MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResonse, unsigned long bytesTransmitted, unsigned long totalBytes, DCM_OBJECT ** object, void *callerCtx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_CStoreResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_STORE_REQ ** storeRequest, MSG_C_STORE_RESP * storeReply, char *filename, SRV_C_STORE_RESP_CALLBACK * callback, void *callbackCtx, char *dirName);

typedef CONDITION(SRV_C_MOVE_REQ_CALLBACK) (MSG_C_MOVE_REQ * moveRequest, MSG_C_MOVE_RESP * moveResonse, int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CMoveRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_MOVE_REQ * moveRequest, MSG_C_MOVE_RESP * moveResponse, SRV_C_MOVE_REQ_CALLBACK * statusCallback, void *statusCtx, char *dirName);

typedef CONDITION(SRV_C_MOVE_RESP_CALLBACK) (MSG_C_MOVE_REQ * moveRequest, MSG_C_MOVE_RESP * moveResonse, int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CMoveResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_MOVE_REQ ** moveRequest, MSG_C_MOVE_RESP * moveResponse, SRV_C_MOVE_REQ_CALLBACK * nextFileCallback, void *nextImageCtx, char *dirName);

typedef CONDITION(SRV_C_GET_REQ_CALLBACK) (MSG_C_GET_REQ * getRequest,  MSG_C_GET_RESP * getResonse, int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CGetRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,	MSG_C_GET_REQ * getRequest, MSG_C_GET_RESP * getResponse,
				SRV_C_GET_REQ_CALLBACK * getCallback, void *getCtx,	CONDITION(*storageCallback) (), void *storageCtx, char *dirName);

typedef CONDITION(SRV_C_GET_RESP_CALLBACK) (MSG_C_GET_REQ * getRequest, MSG_C_GET_RESP * getResonse, MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResponse,
				  int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_GET_REQ ** getRequest, MSG_C_GET_RESP * getResponse,
				 SRV_C_GET_RESP_CALLBACK * nextFileCallback, void *nextImageCtx, char *dirName);

typedef CONDITION(SRV_C_FIND_REQ_CALLBACK) (MSG_C_FIND_REQ * findRequest, MSG_C_FIND_RESP * findResonse, int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CFindRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_C_FIND_REQ * findRequest, MSG_C_FIND_RESP * findResponse, SRV_C_FIND_REQ_CALLBACK * nextRecordCallback, void *nextRecordCtx, char *dirName);

typedef CONDITION(SRV_C_FIND_RESP_CALLBACK) (MSG_C_FIND_REQ * findRequest, MSG_C_FIND_RESP * findResonse, int responseCount, char *abstractSyntax, char *queryLevel, void *ctx);
CONDITION
SRV_CFindResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_FIND_REQ ** findRequest, MSG_C_FIND_RESP * findReply, SRV_C_FIND_RESP_CALLBACK * findCallback, void *findCtx, char *dirName);

typedef CONDITION(SRV_N_GET_REQ_CALLBACK) (MSG_N_GET_REQ * getRequest, MSG_N_GET_RESP * getResonse, void *ctx);
CONDITION
SRV_NGetRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass,	MSG_N_GET_REQ * getRequest, MSG_N_GET_RESP * getResponse, SRV_N_GET_REQ_CALLBACK * getCallback, void *getCtx, char *dirName);

typedef CONDITION(SRV_N_GET_RESP_CALLBACK) (MSG_N_GET_REQ * getRequest, MSG_N_GET_RESP * getResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NGetResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_N_GET_REQ ** ngetRequest, MSG_N_GET_RESP * ngetReply, SRV_N_GET_RESP_CALLBACK * ngetCallback, void *ngetCtx, char *dirName);

typedef CONDITION(SRV_N_CREATE_REQ_CALLBACK) (MSG_N_CREATE_REQ * createRequest, MSG_N_CREATE_RESP * createResonse, void *ctx);
CONDITION
SRV_NCreateRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_CREATE_REQ * createRequest, MSG_N_CREATE_RESP * createResponse, SRV_N_CREATE_REQ_CALLBACK * createCallback, void *createCtx, char *dirName);

typedef CONDITION(SRV_N_CREATE_RESP_CALLBACK) (MSG_N_CREATE_REQ * createRequest, MSG_N_CREATE_RESP * createResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NCreateResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_CREATE_REQ ** createRequest,
					MSG_N_CREATE_RESP * createReply, SRV_N_CREATE_RESP_CALLBACK * createCallback, void *createCtx, char *dirName);

typedef CONDITION(SRV_N_EVENTREPORT_REQ_CALLBACK) (MSG_N_EVENT_REPORT_REQ * eventReportRequest, MSG_N_EVENT_REPORT_RESP * eventReportResonse, void *ctx);
CONDITION
SRV_NEventReportRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, MSG_N_EVENT_REPORT_REQ * eventRequest, MSG_N_EVENT_REPORT_RESP * eventResponse,
						SRV_N_EVENTREPORT_REQ_CALLBACK * eventCallback,	void *eventCtx, char *dirName);

typedef CONDITION(SRV_N_EVENTREPORT_RESP_CALLBACK) (MSG_N_EVENT_REPORT_REQ * eventReportRequest, MSG_N_EVENT_REPORT_RESP * eventReportResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NEventReportResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_EVENT_REPORT_REQ ** eventRequest,
						 MSG_N_EVENT_REPORT_RESP * eventReply, SRV_N_EVENTREPORT_RESP_CALLBACK * eventCallback, void *eventCtx, char *dirName);

typedef CONDITION(SRV_N_SET_REQ_CALLBACK) (MSG_N_SET_REQ * setRequest, MSG_N_SET_RESP * setResonse, void *ctx);
CONDITION
SRV_NSetRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params,	char *SOPClass,	MSG_N_SET_REQ * setRequest, MSG_N_SET_RESP * setResponse,	SRV_N_SET_REQ_CALLBACK * setCallback, void *setCtx,	char *dirName);

typedef CONDITION(SRV_N_SET_RESP_CALLBACK) (MSG_N_SET_REQ * setRequest, MSG_N_SET_RESP * setResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NSetResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_SET_REQ ** setRequest, MSG_N_SET_RESP * setResponse,
				 SRV_N_SET_RESP_CALLBACK * setCallback, void *setCtx, char *dirName);

typedef CONDITION(SRV_N_DELETE_REQ_CALLBACK) (MSG_N_DELETE_REQ * deleteRequest, MSG_N_DELETE_RESP * deleteResonse, void *ctx);
CONDITION
SRV_NDeleteRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_DELETE_REQ * deleteRequest,
				   MSG_N_DELETE_RESP * deleteResponse, SRV_N_DELETE_REQ_CALLBACK * deleteCallback, void *deleteCtx, char *dirName);

typedef CONDITION(SRV_N_DELETE_RESP_CALLBACK)(MSG_N_DELETE_REQ * deleteRequest, MSG_N_DELETE_RESP * deleteResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NDeleteResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_DELETE_REQ ** deleteRequest, MSG_N_DELETE_RESP * deleteResponse,
				    SRV_N_DELETE_RESP_CALLBACK * deleteCallback, void *deleteCtx, char *dirName);

typedef CONDITION(SRV_N_ACTION_REQ_CALLBACK)(MSG_N_ACTION_REQ * actionRequest, MSG_N_ACTION_RESP * actionResonse, void *ctx);
CONDITION
SRV_NActionRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, MSG_N_ACTION_REQ * actionRequest,
				   MSG_N_ACTION_RESP * actionResponse, SRV_N_ACTION_REQ_CALLBACK * actionCallback, void *actionCtx, char *dirName);

typedef
CONDITION(SRV_N_ACTION_RESP_CALLBACK)(MSG_N_ACTION_REQ * actionRequest, MSG_N_ACTION_RESP * actionResonse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
CONDITION
SRV_NActionResponse(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, MSG_N_ACTION_REQ ** actionRequest, MSG_N_ACTION_RESP * actionResponse,
					SRV_N_ACTION_RESP_CALLBACK * actionCallback, void *actionCtx, char *dirName);

void
SRV_SimulateError(int err);

#define	SRV_NORMAL							FORM_COND(FAC_SRV, SEV_SUCC, 1)
#define	SRV_UNSUPPORTEDSERVICE				FORM_COND(FAC_SRV, SEV_ERROR, 2)
#define	SRV_UNSUPPORTEDTRANSFERSYNTAX		FORM_COND(FAC_SRV, SEV_ERROR, 3)
#define	SRV_PEERREQUESTEDRELEASE			FORM_COND(FAC_SRV, SEV_ERROR, 4)
#define	SRV_PEERABORTEDASSOCIATION			FORM_COND(FAC_SRV, SEV_ERROR, 5)
#define SRV_READPDVFAILED					FORM_COND(FAC_SRV, SEV_ERROR, 6)
#define SRV_RECEIVEFAILED					FORM_COND(FAC_SRV, SEV_ERROR, 7)
#define SRV_OBJECTTOOLARGE					FORM_COND(FAC_SRV, SEV_ERROR, 8)
#define SRV_UNEXPECTEDPRESENTATIONCONTEXTID FORM_COND(FAC_SRV, SEV_ERROR, 9)
#define SRV_UNEXPECTEDPDVTYPE				FORM_COND(FAC_SRV, SEV_ERROR, 10)
#define SRV_ILLEGALASSOCIATION				FORM_COND(FAC_SRV, SEV_ERROR, 11)
#define SRV_SENDFAILED						FORM_COND(FAC_SRV, SEV_ERROR, 12)
#define	SRV_NOSERVICEINASSOCIATION			FORM_COND(FAC_SRV, SEV_ERROR, 13)
#define	SRV_FILECREATEFAILED				FORM_COND(FAC_SRV, SEV_ERROR, 14)
#define	SRV_LISTFAILURE						FORM_COND(FAC_SRV, SEV_ERROR, 15)
#define	SRV_MALLOCFAILURE					FORM_COND(FAC_SRV, SEV_ERROR, 16)
#define	SRV_PRESENTATIONCONTEXTERROR		FORM_COND(FAC_SRV, SEV_ERROR, 17)
#define	SRV_PARSEFAILED						FORM_COND(FAC_SRV, SEV_ERROR, 18)
#define	SRV_UNSUPPORTEDCOMMAND				FORM_COND(FAC_SRV, SEV_ERROR, 19)
#define	SRV_NOTRANSFERSYNTAX				FORM_COND(FAC_SRV, SEV_ERROR, 20)
#define	SRV_PRESENTATIONCTXREJECTED			FORM_COND(FAC_SRV, SEV_WARN,  21)
#define	SRV_NOCALLBACK						FORM_COND(FAC_SRV, SEV_ERROR, 22)
#define	SRV_ILLEGALPARAMETER				FORM_COND(FAC_SRV, SEV_ERROR, 23)
#define	SRV_OBJECTBUILDFAILED				FORM_COND(FAC_SRV, SEV_ERROR, 24)
#define	SRV_REQUESTFAILED					FORM_COND(FAC_SRV, SEV_ERROR, 25)
#define	SRV_UNEXPECTEDCOMMAND				FORM_COND(FAC_SRV, SEV_ERROR, 26)
#define	SRV_CALLBACKABORTEDSERVICE			FORM_COND(FAC_SRV, SEV_ERROR, 27)
#define	SRV_RESPONSEFAILED					FORM_COND(FAC_SRV, SEV_ERROR, 28)
#define	SRV_OBJECTACCESSFAILED				FORM_COND(FAC_SRV, SEV_ERROR, 29)
#define	SRV_QUERYLEVELATTRIBUTEMISSING		FORM_COND(FAC_SRV, SEV_ERROR, 30)
#define	SRV_ILLEGALQUERYLEVELATTRIBUTE		FORM_COND(FAC_SRV, SEV_ERROR, 31)
#define	SRV_SYSTEMERROR						FORM_COND(FAC_SRV, SEV_ERROR, 32)
#define	SRV_LISTCREATEFAILURE				FORM_COND(FAC_SRV, SEV_ERROR, 33)
#define	SRV_SUSPICIOUSRESPONSE				FORM_COND(FAC_SRV, SEV_WARN,  34)
#define	SRV_OPERATIONCANCELLED				FORM_COND(FAC_SRV, SEV_INFORM,35)
#define	SRV_EMPTYCOMMANDQUEUE				FORM_COND(FAC_SRV, SEV_INFORM,36)
#define	SRV_NOAVAILABLECOMMAND				FORM_COND(FAC_SRV, SEV_INFORM,37)
#define	SRV_READTIMEOUT						FORM_COND(FAC_SRV, SEV_INFORM,38)
#define	SRV_PDVRECEIVEDOUTOFSEQUENCE		FORM_COND(FAC_SRV, SEV_ERROR, 39)
#define SRV_RECEIVEDATASETFAILED			FORM_COND(FAC_SRV, SEV_ERROR, 40)

#ifdef  __cplusplus
}
#endif

#endif
