/*
          Copyright (C) 1993, RSNA and Washington University

          The software and supporting documentation for the Radiological
          Society of North America (RSNA) 1993 Digital Imaging and
          Communications in Medicine (DICOM) Demonstration were developed
          at the
                  Electronic Radiology Laboratory
                  Mallinckrodt Institute of Radiology
                  Washington University School of Medicine
                  510 S. Kingshighway Blvd.
                  St. Louis, MO 63110
          as part of the 1993 DICOM Central Test Node project for, and
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
/*
** @$=@$=@$=
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	SRV_AcceptServiceClass
**			SRV_MessageIDOut
**			SRV_MessageIDIn
**			SRV_ReceiveCommand
**			SRV_ReceiveDataSet
**			SRV_RejectServiceClass
**			SRV_RequestServiceClass
**			SRV_SendCommand
**			SRV_SendDataSet
**			SRV_TestForCancel
**	private modules
**			SRVPRV_ReadNextPDV
**			writeCallback
**			verifyCommandValidity
**			dequeCommand
**			enqueCommand
**			findPresentationContext
**			createFile
** Author, Date:	Stephen M. Moore, 15-Apr-93
** Intent:		This module contains general routines which are used
**			in our implementation of service classes.  These
**			routines allow users to request and accept service
**			classes, build and manipulate the public DUL
**			structures, send and receive messages, and request
**			unique Message IDs.
** Last Update:		$Author: smm $, $Date: 2000/01/20 17:57:45 $
** Source File:		$RCSfile: services.c,v $
** Revision:		$Revision: 1.55 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.55 $ $RCSfile: services.c,v $";

#include "../dicom/ctn_os.h"
/*
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef SOLARIS
#include <sys/fcntl.h>
#endif
ZT*/

#include "../dicom/dicom.h"
#include "../uid/dicom_uids.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"
#include "private.h"

#define FRAGMENTMAX 65536
typedef struct {
    DUL_ASSOCIATIONKEY 	*association;
    DUL_PDVLIST 		*pdvList;
    CONDITION			(*callback) ();
    void				*ctx;
    unsigned long 		bytesTransmitted;
    unsigned long 		totalBytes;
}   CALLBACK_STRUCTURE;

typedef unsigned long SRVPERMITTED;

typedef struct {
    char 			classUID[DICOM_UI_LENGTH + 1];
    SRVPERMITTED 	*permittedSrvList;	/* list of permitted services */
    unsigned short 	permittedSrvListSize;
}   SOPCLASSPERMITTEDSRV;	/* defines the various services permitted for a given SOP class */

static char fragmentBuffer[FRAGMENTMAX + 1000];
static DUL_PDVLIST pdvList = {0, fragmentBuffer, sizeof(fragmentBuffer), {0x00, 0x00, 0x00}, NULL};

static char *syntaxList[] = {
    DICOM_SOPCLASSVERIFICATION,
    DICOM_SOPCLASSCOMPUTEDRADIOGRAPHY,
    DICOM_SOPCLASSDIGXRAYPRESENTATION,
    DICOM_SOPCLASSCT,
    DICOM_SOPCLASSUSMULTIFRAMEIMAGE,
    DICOM_SOPCLASSMR,
    DICOM_SOPCLASSNM,
    DICOM_SOPCLASSUS,
    DICOM_SOPCLASSSECONDARYCAPTURE,
    DICOM_SOPPATIENTQUERY_FIND,
    DICOM_SOPPATIENTQUERY_MOVE,
    DICOM_SOPPATIENTQUERY_GET,
    DICOM_SOPSTUDYQUERY_FIND,
    DICOM_SOPSTUDYQUERY_MOVE,
    DICOM_SOPSTUDYQUERY_GET,
    DICOM_SOPPATIENTSTUDYQUERY_FIND,
    DICOM_SOPPATIENTSTUDYQUERY_MOVE,
    DICOM_SOPPATIENTSTUDYQUERY_GET,
    DICOM_SOPCLASSGREYSCALEPRINTMGMTMETA,
    DICOM_SOPCLASSCOLORPRINTMGMTMETA,
    DICOM_SOPCLASSDETACHEDPATIENTMGMT,
    DICOM_SOPCLASSDETACHEDVISITMGMT,
    DICOM_SOPCLASSDETACHEDPATIENTMGMTMETA,
    DICOM_SOPCLASSDETACHEDSTUDYMGMT,
    DICOM_SOPCLASSDETACHEDRESULTSMGMT,
    DICOM_SOPCLASSDETACHEDINTERPRETMGMT,
    DICOM_SOPCLASSDETACHEDRESULTSMGMTMETA
};

typedef struct {
    void 						*reserved[2];
    DUL_ASSOCIATIONKEY 			**association;
    DUL_PRESENTATIONCONTEXTID 	ctxID;
    unsigned short 				command;
    MSG_TYPE 					messageType;
    void 						*message;
}   COMMAND_ENTRY;

static LST_HEAD *commandList = NULL;

CTNBOOLEAN PRVSRV_debug = 0;

static CONDITION
writeCallback(void *buffer, unsigned long length, int last, CALLBACK_STRUCTURE * callbackStructure);
static CONDITION
createFile(char *dirName, char *qualifiedFile, int *fd);
static CONDITION
verifyCommandValidity(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXTID ctxid, unsigned short command);
static CONDITION
dequeCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXTID ctxID, MSG_TYPE messageType, COMMAND_ENTRY * commandEntry);
static CONDITION
enqueCommand(COMMAND_ENTRY * commandEntry);
static DUL_PRESENTATIONCONTEXT *
findPresentationCtx(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXTID ctxid);

/* SRV_RequestServiceClass
**
** Purpose:
**	This function is called by an application which is proposing an
**	Association and wishes to request a service class.  The application
**	can request the Service Class as an SCU or as an SCP.  This function
**	determines if the SERVICES library supports the service class
**	(table lookup).  If so, it builds a Presentation Context for the
**	service and adds it to the list of Presentation Contexts for the
**	Association which is to be requested.
**
** Parameter Dictionary:
**	SOPClass	UID of the abstract syntrax which defines the
**			service class
**	role		Role the application wishes to propose for this
**			class (SCU, SCP, both, default)
**	params		Parameter list for the Association to be requested.
**			This includes the list of Presentation Contexts
**			for the Association.
**
** Return Values:
**
**	SRV_LISTFAILURE
**	SRV_MALLOCFAILURE
**	SRV_NORMAL
**	SRV_PRESENTATIONCONTEXTERROR
**	SRV_UNSUPPORTEDSERVICE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_RequestServiceClass(const char *SOPClass, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params)
{
    static DUL_PRESENTATIONCONTEXTID        contextID = 1;
    int								        index;
    CTNBOOLEAN								found = FALSE;
    CONDITION								cond;
    DUL_PRESENTATIONCONTEXT					* ctx;

    if (params->requestedPresentationContext == NULL) {
    	params->requestedPresentationContext = LST_Create();
    	if (params->requestedPresentationContext == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_RequestServiceClass");
	}

    ctx = LST_Head(&params->requestedPresentationContext);
    if (ctx != NULL) (void) LST_Position(&params->requestedPresentationContext, ctx);

    while (ctx != NULL) {
    	if (strcmp(SOPClass, ctx->abstractSyntax) == 0) return SRV_NORMAL;
    	ctx = LST_Next(&params->requestedPresentationContext);
    }

    for (index = 0; index < (int) DIM_OF(syntaxList) != 0 && !found; index++) {
    	if (strcmp(SOPClass, syntaxList[index]) == 0) found = TRUE;
	}

    if (found) {
    	ctx = malloc(sizeof(*ctx));
    	if (ctx == NULL) return COND_PushCondition(SRV_MALLOCFAILURE, SRV_Message(SRV_MALLOCFAILURE), sizeof(*ctx), "SRV_RequestServiceClass");

    	cond = DUL_MakePresentationCtx(&ctx, role, DUL_SC_ROLE_DEFAULT, contextID, 0, SOPClass, "", DICOM_TRANSFERLITTLEENDIAN, NULL);
    	if (cond != DUL_NORMAL){
    		return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_RequestServiceClass");
    	}else{
    		cond = LST_Enqueue(&params->requestedPresentationContext, ctx);
    		if (cond != LST_NORMAL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_RequestServiceClass");
    		contextID += 2;
    	}
    }else{
    	return COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), SOPClass, "SRV_RequestServiceClass");
    }
    pdvList.count = 0;

    return SRV_NORMAL;
}

/* SRV_AcceptServiceClass
**
** Purpose:
**	Determine if the SRV facility can accept a proposed service class and
**	and build the appropriate response for the Association Accept message.
**
** Parameter Dictionary:
**	requestedCtx	The presentation context for the sevice which has been
**			requested by the Requesting Application. This context
**			includes the UID of the service class as well as the
**			proposed transfer syntax UIDs.
**	role		Role proposed by the application for this service
**			class.
**	params		The list of service parameters for the Association
**			which is being negotiated.
**
** Return Values:
**
**	SRV_LISTFAILURE
**	SRV_NORMAL
**	SRV_PRESENTATIONCONTEXTERROR
**	SRV_PRESENTATIONCTXREJECTED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_AcceptServiceClass(DUL_PRESENTATIONCONTEXT * requestedCtx, DUL_SC_ROLE role, DUL_ASSOCIATESERVICEPARAMETERS * params)
{
    int					        index;
    CTNBOOLEAN						abstractFound = FALSE, transferFound = FALSE;
    CONDITION					cond, rtnCond = SRV_NORMAL;
    DUL_PRESENTATIONCONTEXT		* ctx;
    DUL_TRANSFERSYNTAX			* transfer;

    if (params->acceptedPresentationContext == NULL) {
    	params->acceptedPresentationContext = LST_Create();
    	if (params->acceptedPresentationContext == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_AcceptServiceClass");
	}

    for (index = 0; index < (int) DIM_OF(syntaxList) && !abstractFound; index++){
    	if (strcmp(requestedCtx->abstractSyntax, syntaxList[index]) == 0) abstractFound = TRUE;
	}

    if (abstractFound) {
    	if ((transfer = LST_Head(&requestedCtx->proposedTransferSyntax)) == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_AcceptServiceClass");
    	(void) LST_Position(&requestedCtx->proposedTransferSyntax, transfer);

    	while (!transferFound && (transfer != NULL)) {
    		if (strcmp(transfer->transferSyntax, DICOM_TRANSFERLITTLEENDIAN) == 0){
    			transferFound = TRUE;
    		}else{
    			transfer = LST_Next(&requestedCtx->proposedTransferSyntax);
    		}
    	}

    	if (transferFound) {
    		cond = DUL_MakePresentationCtx(&ctx, requestedCtx->proposedSCRole, role, requestedCtx->presentationContextID, DUL_PRESENTATION_ACCEPT,
										   requestedCtx->abstractSyntax, DICOM_TRANSFERLITTLEENDIAN, DICOM_TRANSFERLITTLEENDIAN, NULL);
    		if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_AcceptServiceClass");
    	}else{
    		cond = DUL_MakePresentationCtx(&ctx, requestedCtx->proposedSCRole, DUL_SC_ROLE_DEFAULT,	requestedCtx->presentationContextID, DUL_PRESENTATION_REJECT_TRANSFER_SYNTAX,
										   requestedCtx->abstractSyntax, DICOM_TRANSFERLITTLEENDIAN, DICOM_TRANSFERLITTLEENDIAN, NULL);
    		if (cond != DUL_NORMAL) return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_AcceptServiceClass");

    		(void) COND_PushCondition(SRV_UNSUPPORTEDTRANSFERSYNTAX, SRV_Message(SRV_UNSUPPORTEDTRANSFERSYNTAX), requestedCtx->abstractSyntax, "SRV_AcceptServiceClass");

    		rtnCond = COND_PushCondition(SRV_PRESENTATIONCTXREJECTED, SRV_Message(SRV_PRESENTATIONCTXREJECTED), requestedCtx->abstractSyntax, "SRV_AcceptServiceClass");
    	}
    }else{
    	cond = DUL_MakePresentationCtx(&ctx, requestedCtx->proposedSCRole, DUL_SC_ROLE_DEFAULT, requestedCtx->presentationContextID, DUL_PRESENTATION_REJECT_ABSTRACT_SYNTAX,
									   requestedCtx->abstractSyntax, DICOM_TRANSFERLITTLEENDIAN, DICOM_TRANSFERLITTLEENDIAN, NULL);
    	if (cond != DUL_NORMAL) return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_AcceptServiceClass");
    	(void) COND_PushCondition(SRV_UNSUPPORTEDSERVICE, SRV_Message(SRV_UNSUPPORTEDSERVICE), requestedCtx->abstractSyntax, "SRV_AcceptServiceClass");
    	rtnCond = COND_PushCondition(SRV_PRESENTATIONCTXREJECTED, SRV_Message(SRV_PRESENTATIONCTXREJECTED), requestedCtx->abstractSyntax, "SRV_AcceptServiceClass");
    }

    cond = LST_Enqueue(&params->acceptedPresentationContext, ctx);
    if (cond != LST_NORMAL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_AcceptServiceClass");

    pdvList.count = 0;

    return rtnCond;
}

/* SRV_RejectServiceClass
**
** Purpose:
**	Reject an SOP class proposed by a calling application.
**
** Parameter Dictionary:
**	requestedCtx	Pointerto requested Presentation Context which user
**			is rejecting.
**	result		One of the defined DUL results which provide reasons
**			for rejecting a Presentation Context.
**	params		The structure which contains parameters which defines
**			the association.
**
** Return Values:
**
**	SRV_LISTFAILURE
**	SRV_NORMAL
**	SRV_PRESENTATIONCONTEXTERROR
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_RejectServiceClass(DUL_PRESENTATIONCONTEXT * requestedCtx, unsigned short result, DUL_ASSOCIATESERVICEPARAMETERS * params)
{
    CONDITION					cond;
    DUL_PRESENTATIONCONTEXT		* ctx;

    if (params->acceptedPresentationContext == NULL) {
    	params->acceptedPresentationContext = LST_Create();
    	if (params->acceptedPresentationContext == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_RejectServiceClass");
    }
    cond = DUL_MakePresentationCtx(&ctx, requestedCtx->proposedSCRole, DUL_SC_ROLE_DEFAULT, requestedCtx->presentationContextID, result, requestedCtx->abstractSyntax,
								   DICOM_TRANSFERLITTLEENDIAN, DICOM_TRANSFERLITTLEENDIAN, NULL);
    if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_PRESENTATIONCONTEXTERROR, SRV_Message(SRV_PRESENTATIONCONTEXTERROR), "SRV_RejectServiceClass");

    cond = LST_Enqueue(&params->acceptedPresentationContext, ctx);
    if (cond != LST_NORMAL)	return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "SRV_RejectServiceClass");

    return SRV_NORMAL;
}


/* Initialization of the SRVPERMITTED list
*/

/* verification SOP classes */
static SRVPERMITTED verifySOPClass[] = {
    DCM_ECHO_REQUEST,
    DCM_ECHO_RESPONSE
};
/* Storage SOP classes */
static SRVPERMITTED computedRadiographySOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED digXRayPresentationSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED ctImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED usMultiframeImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED mrImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED nmImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED usImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED secondaryCaptureImageStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED standAloneOverlayStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED standAloneCurveStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED standAloneModalityLUTStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
static SRVPERMITTED standAloneVOILUTStorageSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
/* Query/Retrieve SOP classes */
static SRVPERMITTED patientQueryRetrieveFindSOPClass[] = {
    DCM_FIND_REQUEST,
    DCM_FIND_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED patientQueryRetrieveMoveSOPClass[] = {
    DCM_MOVE_REQUEST,
    DCM_MOVE_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED patientQueryRetrieveGetSOPClass[] = {
    DCM_GET_REQUEST,
    DCM_GET_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED studyQueryRetrieveFindSOPClass[] = {
    DCM_FIND_REQUEST,
    DCM_FIND_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED studyQueryRetrieveMoveSOPClass[] = {
    DCM_MOVE_REQUEST,
    DCM_MOVE_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED studyQueryRetrieveGetSOPClass[] = {
    DCM_GET_REQUEST,
    DCM_GET_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED patientStudyQueryRetrieveFindSOPClass[] = {
    DCM_FIND_REQUEST,
    DCM_FIND_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED patientStudyQueryRetrieveMoveSOPClass[] = {
    DCM_MOVE_REQUEST,
    DCM_MOVE_RESPONSE,
    DCM_CANCEL_REQUEST
};
static SRVPERMITTED patientStudyQueryRetrieveGetSOPClass[] = {
    DCM_GET_REQUEST,
    DCM_GET_RESPONSE,
    DCM_CANCEL_REQUEST
};
/* Notification Service classes */
static SRVPERMITTED basicStudyContentNotificationSOPClass[] = {
    DCM_STORE_REQUEST,
    DCM_STORE_RESPONSE
};
/* Patient Management Service SOP classes */
static SRVPERMITTED detachedPatientManagementSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE
};
static SRVPERMITTED detachedVisitManagementSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED detachedPatientManagementMetaSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
/* Study Management Service SOP classes */
static SRVPERMITTED detachedStudyManagementSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED studyComponentManagementSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED studyManagementMetaSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
/* Results Management Service SOP classes */
static SRVPERMITTED detachedResultsManagementSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE
};
static SRVPERMITTED detachedInterpretationManagementSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED detachedResultsManagementMetaSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
/* Print Management Service SOP Classes */
static SRVPERMITTED basicGreyscalePrintManagementMetaSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED basicColorPrintManagementMetaSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED referencedGreyscalePrintManagementMetaSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED referencedColorPrintManagementMetaSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED basicFilmSessionSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED basicFilmBoxSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_ACTION_REQUEST,
    DCM_N_ACTION_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED basicGreyscaleImageBoxSOPClass[] = {
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED basicColorImageBoxSOPClass[] = {
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED referencedImageBoxSOPClass[] = {
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED basicAnnotationBoxSOPClass[] = {
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE
};
static SRVPERMITTED printJobSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE
};
static SRVPERMITTED printerSOPClass[] = {
    DCM_N_EVENT_REPORT_REQUEST,
    DCM_N_EVENT_REPORT_RESPONSE,
    DCM_N_GET_REQUEST,
    DCM_N_GET_RESPONSE
};
static SRVPERMITTED basicVOILUTSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};
static SRVPERMITTED imageOverlayBoxSOPClass[] = {
    DCM_N_CREATE_REQUEST,
    DCM_N_CREATE_RESPONSE,
    DCM_N_SET_REQUEST,
    DCM_N_SET_RESPONSE,
    DCM_N_DELETE_REQUEST,
    DCM_N_DELETE_RESPONSE
};


/* Initialization of the permitted services table for each defined
   SOP class
*/
static SOPCLASSPERMITTEDSRV classPermittedSrvTable[] = {
    /* verification service */
    {DICOM_SOPCLASSVERIFICATION, verifySOPClass, DIM_OF(verifySOPClass)},
    /* storage service */
    {DICOM_SOPCLASSCOMPUTEDRADIOGRAPHY, computedRadiographySOPClass, DIM_OF(computedRadiographySOPClass)},
    {DICOM_SOPCLASSDIGXRAYPRESENTATION, digXRayPresentationSOPClass, DIM_OF(digXRayPresentationSOPClass)},
    {DICOM_SOPCLASSCT, ctImageStorageSOPClass, DIM_OF(ctImageStorageSOPClass)},
    {DICOM_SOPCLASSUSMULTIFRAMEIMAGE, usMultiframeImageStorageSOPClass, DIM_OF(usMultiframeImageStorageSOPClass)},
    {DICOM_SOPCLASSMR, mrImageStorageSOPClass, DIM_OF(mrImageStorageSOPClass)},
    {DICOM_SOPCLASSNM, nmImageStorageSOPClass, DIM_OF(nmImageStorageSOPClass)},
    {DICOM_SOPCLASSUS, usImageStorageSOPClass, DIM_OF(usImageStorageSOPClass)},
    {DICOM_SOPCLASSSECONDARYCAPTURE, secondaryCaptureImageStorageSOPClass, DIM_OF(secondaryCaptureImageStorageSOPClass)},
    {DICOM_SOPCLASSSTANDALONEOVERLAY, standAloneOverlayStorageSOPClass, DIM_OF(standAloneOverlayStorageSOPClass)},
    {DICOM_SOPCLASSSTANDALONECURVE, standAloneCurveStorageSOPClass, DIM_OF(standAloneCurveStorageSOPClass)},
    {DICOM_SOPCLASSSTANDALONEMODALITYLUT, standAloneModalityLUTStorageSOPClass, DIM_OF(standAloneModalityLUTStorageSOPClass)},
    {DICOM_SOPCLASSSTANDALONEVOILUT, standAloneVOILUTStorageSOPClass, DIM_OF(standAloneVOILUTStorageSOPClass)},
    /* Query/Retrieve Service */
    {DICOM_SOPPATIENTQUERY_FIND, patientQueryRetrieveFindSOPClass, DIM_OF(patientQueryRetrieveFindSOPClass)},
    {DICOM_SOPPATIENTQUERY_MOVE, patientQueryRetrieveMoveSOPClass, DIM_OF(patientQueryRetrieveMoveSOPClass)},
    {DICOM_SOPPATIENTQUERY_GET, patientQueryRetrieveGetSOPClass, DIM_OF(patientQueryRetrieveGetSOPClass)},
    {DICOM_SOPSTUDYQUERY_FIND, studyQueryRetrieveFindSOPClass, DIM_OF(studyQueryRetrieveFindSOPClass)},
    {DICOM_SOPSTUDYQUERY_MOVE, studyQueryRetrieveMoveSOPClass, DIM_OF(studyQueryRetrieveMoveSOPClass)},
    {DICOM_SOPSTUDYQUERY_GET, studyQueryRetrieveGetSOPClass, DIM_OF(studyQueryRetrieveGetSOPClass)},
    {DICOM_SOPPATIENTSTUDYQUERY_FIND, patientStudyQueryRetrieveFindSOPClass, DIM_OF(patientStudyQueryRetrieveFindSOPClass)},
    {DICOM_SOPPATIENTSTUDYQUERY_MOVE, patientStudyQueryRetrieveMoveSOPClass, DIM_OF(patientStudyQueryRetrieveMoveSOPClass)},
    {DICOM_SOPPATIENTSTUDYQUERY_GET, patientStudyQueryRetrieveGetSOPClass, DIM_OF(patientStudyQueryRetrieveGetSOPClass)},
    /* Notification Service */
    {DICOM_SOPCLASSBASICSTUDYCONTENTNOTIFICATION, basicStudyContentNotificationSOPClass, DIM_OF(basicStudyContentNotificationSOPClass)},    /* Patient Management Service */
    {DICOM_SOPCLASSDETACHEDPATIENTMGMT, detachedPatientManagementSOPClass, DIM_OF(detachedPatientManagementSOPClass)},
    {DICOM_SOPCLASSDETACHEDVISITMGMT, detachedVisitManagementSOPClass, DIM_OF(detachedVisitManagementSOPClass)},
    {DICOM_SOPCLASSDETACHEDPATIENTMGMTMETA, detachedPatientManagementMetaSOPClass, DIM_OF(detachedPatientManagementMetaSOPClass)},
    /* Study Management Service */
    {DICOM_SOPCLASSDETACHEDSTUDYMGMT, detachedStudyManagementSOPClass, DIM_OF(detachedStudyManagementSOPClass)},
    {DICOM_SOPCLASSSTUDYCOMPONENTMGMT, studyComponentManagementSOPClass, DIM_OF(studyComponentManagementSOPClass)},
    {DICOM_SOPCLASSDETACHEDSTUDYMGMTMETA, studyManagementMetaSOPClass, DIM_OF(studyManagementMetaSOPClass)},
    /* Results Management Services */
    {DICOM_SOPCLASSDETACHEDRESULTSMGMT, detachedResultsManagementSOPClass, DIM_OF(detachedResultsManagementSOPClass)},
    {DICOM_SOPCLASSDETACHEDINTERPRETMGMT, detachedInterpretationManagementSOPClass, DIM_OF(detachedInterpretationManagementSOPClass)},
    {DICOM_SOPCLASSDETACHEDRESULTSMGMTMETA,	detachedResultsManagementMetaSOPClass, DIM_OF(detachedResultsManagementMetaSOPClass)},
    /* Print Management services */
    {DICOM_SOPCLASSGREYSCALEPRINTMGMTMETA, basicGreyscalePrintManagementMetaSOPClass, DIM_OF(basicGreyscalePrintManagementMetaSOPClass)},
    {DICOM_SOPCLASSCOLORPRINTMGMTMETA, basicColorPrintManagementMetaSOPClass, DIM_OF(basicColorPrintManagementMetaSOPClass)},
    {DICOM_SOPCLASSREFGREYSCALEPRINTMGMTMETA, referencedGreyscalePrintManagementMetaSOPClass, DIM_OF(referencedGreyscalePrintManagementMetaSOPClass)},
    {DICOM_SOPCLASSREFCOLORPRINTMGMTMETA, referencedColorPrintManagementMetaSOPClass, DIM_OF(referencedColorPrintManagementMetaSOPClass)},
    {DICOM_SOPCLASSBASICFILMSESSION, basicFilmSessionSOPClass, DIM_OF(basicFilmSessionSOPClass)},
    {DICOM_SOPCLASSBASICFILMBOX, basicFilmBoxSOPClass, DIM_OF(basicFilmBoxSOPClass)},
    {DICOM_SOPCLASSBASICGREYSCALEIMAGEBOX, basicGreyscaleImageBoxSOPClass, DIM_OF(basicGreyscaleImageBoxSOPClass)},
    {DICOM_SOPCLASSBASICCOLORIMAGEBOX, basicColorImageBoxSOPClass, DIM_OF(basicColorImageBoxSOPClass)},
    {DICOM_SOPCLASSREFERENCEDIMAGEBOX, referencedImageBoxSOPClass, DIM_OF(referencedImageBoxSOPClass)},
    {DICOM_SOPCLASSBASICANNOTATIONBOX, basicAnnotationBoxSOPClass, DIM_OF(basicAnnotationBoxSOPClass)},
    {DICOM_SOPCLASSPRINTJOB, printJobSOPClass, DIM_OF(printJobSOPClass)},
    {DICOM_SOPCLASSPRINTER, printerSOPClass, DIM_OF(printerSOPClass)},
    {DICOM_SOPCLASSVOILUT, basicVOILUTSOPClass, DIM_OF(basicVOILUTSOPClass)},
    {DICOM_SOPCLASSIMAGEOVERLAYBOX, imageOverlayBoxSOPClass, DIM_OF(imageOverlayBoxSOPClass)}
};

/* SRV_ReceiveCommand
**
** Purpose:
**	Check the status of the network and receive the next command on the
**	network.
**
** Parameter Dictionary:
**	association	The key which describes which association to use to
**			check the network for a command.
**	params		The set of service parameters which describe the
**			services which are valid for this association.
**	block		A flag indicating if the caller wishes to block
**			waiting for a command (DUL_BLOCK) or return
**			immediately if there is no command present
**			(DUL_NOBLOCK).
**	timeout		If the application chooses to block and wait for a
**			command, the amount of time to wait before returning
**			to the caller (in seconds).
**	ctxId		Pointer to a caller variable where this function will
**			store the presentation context ID for the command
**			received from the network.
**	command		Pointer to caller variable where this function will
**			store the command value from the COMMAND group which
**			was read from the network.
**	messageType	Pointer to caller variable where this function will
**			store one of the enumerated message types from the
**			MSG facility. There should be a one to one
**			correspondence between the COMMAND received from the
**			network and this messageType.
**	messageArg	Address of pointer in caller's space. This function
**			allocates a MSG structure and writes the address of the
**			allocated structure in the caller's messageArg pointer.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PARSEFAILED
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**	SRV_UNSUPPORTEDCOMMAND
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SRV_ReceiveCommand(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_BLOCKOPTIONS block, int timeout, DUL_PRESENTATIONCONTEXTID * ctxID,
				   unsigned short *command, MSG_TYPE * messageType, void **messageArg)
{
    CONDITION				  cond;
    unsigned char        	  stream[8192], *s;
    unsigned long        	  bytesRead;
    DUL_DATAPDV				  type;
    CTNBOOLEAN					  last;
    DUL_PDV					  pdv;
    DCM_OBJECT				  * object;
    MSG_GENERAL			  	  ** msg;
    static unsigned short     dcmCommand;

    msg = (MSG_GENERAL **) messageArg;
    *ctxID = 0;

    if (PRVSRV_debug) fprintf(stderr, "Beginning to read a COMMAND PDV\n");

    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_COMMANDPDV; type == DUL_COMMANDPDV && !last;){
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)){
    		(void) COND_PushCondition(SRV_OBJECTTOOLARGE, SRV_Message(SRV_OBJECTTOOLARGE));
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (*ctxID == 0) *ctxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != *ctxID) {
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (pdv.pdvType != DUL_COMMANDPDV) {
    		(void) COND_PushCondition(SRV_PDVRECEIVEDOUTOFSEQUENCE, SRV_Message(SRV_PDVRECEIVEDOUTOFSEQUENCE), "Command", "Data", pdv.presentationContextID, "SRV_ReceiveCommand");
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    	type = pdv.pdvType;
    }
    if (type != DUL_COMMANDPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    }
    /* repair */
    cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN, &object);
    if (cond != DCM_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    if (PRVSRV_debug) (void) DCM_DumpElements(&object,0);

    /* repair */
    {
	 void 					*ctx;
	 U32 					elementLength;
	 static DCM_ELEMENT 	element = {DCM_CMDCOMMANDFIELD,	DCM_US, "", 1, sizeof(dcmCommand), (void *) &dcmCommand};

	 ctx = NULL;
	 cond = DCM_GetElementValue(&object, &element, &elementLength, &ctx);
	 if (cond != DCM_NORMAL){
		 (void) DCM_CloseObject(&object);
		 return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
	 }
	 if (command != NULL) *command = dcmCommand;
	 if (PRVSRV_debug) fprintf(stderr, "Command received: %04x\n", (int) dcmCommand);

	 /* we verify if the command is applicable for the SOP class that is negotiated for the association */
	 cond = verifyCommandValidity(params, *ctxID, dcmCommand);
	 if (cond != SRV_NORMAL) return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");

	 switch (dcmCommand) {
		 case DCM_ECHO_REQUEST:
		 case DCM_ECHO_RESPONSE:
		 case DCM_STORE_REQUEST:
		 case DCM_STORE_RESPONSE:
		 case DCM_FIND_REQUEST:
		 case DCM_FIND_RESPONSE:
		 case DCM_MOVE_REQUEST:
		 case DCM_MOVE_RESPONSE:
		 case DCM_GET_REQUEST:
		 case DCM_GET_RESPONSE:
		 case DCM_CANCEL_REQUEST:
		 case DCM_N_EVENT_REPORT_REQUEST:
		 case DCM_N_EVENT_REPORT_RESPONSE:
		 case DCM_N_GET_REQUEST:
		 case DCM_N_GET_RESPONSE:
		 case DCM_N_SET_REQUEST:
		 case DCM_N_SET_RESPONSE:
		 case DCM_N_ACTION_REQUEST:
		 case DCM_N_ACTION_RESPONSE:
		 case DCM_N_CREATE_REQUEST:
		 case DCM_N_CREATE_RESPONSE:
		 case DCM_N_DELETE_REQUEST:
		 case DCM_N_DELETE_RESPONSE:
											 *msg = NULL;
											 cond = MSG_ParseCommand(&object, (void **) msg);
											 (void) DCM_CloseObject(&object);
											 if (cond != MSG_NORMAL) return COND_PushCondition(SRV_PARSEFAILED, SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");
											 *messageType = (*msg)->type;
											 break;

		 default:
											 return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");
	 }
    }

    return SRV_NORMAL;
}


/* SRV_SendCommand
**
** Purpose:
**	Send a DICOM command to a peer using an established Association.
**
** Parameter Dictionary:
**	association	Key which describes the Association used for
**			transmitting the command.
**	context		Presentation context to be used when sending a command.
**	object		Address of DICOM object which contains the attributes
**			of the command to be transmitted.
**
** Return Values:
**	SRV_NORMAL on success.
**	SRV_NOTRANSFERSYNTAX
**	SRV_SENDFAILED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_SendCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object)
{
    /* repair */
    unsigned char       buf[2048];
    unsigned long       options;
    CONDITION			cond;
    DUL_PDVLIST			pdvListSend;
    DUL_PDV				pdv;
    CALLBACK_STRUCTURE	callbackStructure;

    callbackStructure.association = *association;
    callbackStructure.pdvList = &pdvListSend;
    callbackStructure.callback = NULL;

    if (strcmp(context->acceptedTransferSyntax, DICOM_TRANSFERLITTLEENDIAN)	== 0){
    	options = DCM_ORDERLITTLEENDIAN;
    }else{
    	return COND_PushCondition(SRV_NOTRANSFERSYNTAX, SRV_Message(SRV_NOTRANSFERSYNTAX), context->abstractSyntax,	context->acceptedTransferSyntax, "SRV_SendCommand");
    }
    pdv.fragmentLength = 0;
    pdv.presentationContextID = context->presentationContextID;
    pdv.pdvType = DUL_COMMANDPDV;
    pdv.lastPDV = 0;
    pdv.data = buf;

    pdvListSend.count = 1;
    pdvListSend.pdv = &pdv;
    cond = DCM_ExportStream(object, options, buf, sizeof(buf), writeCallback, &callbackStructure);
    if (ERROR(cond)) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "COMMAND", "SRV_SendCommand");
    return SRV_NORMAL;
}

/* SRV_SendDataSet
**
** Purpose:
**	Send a DICOM dataset to a peer using an established Association.
**
** Parameter Dictionary:
**	association	Key which describes the Association used for
**			transmitting the command.
**	context		Presentation context to be used when sending the
**			command.
**	object		Address of DICOM object which contains the dataset to
**			be sent.
**	callback	User callback function which is called periodically
**			while the dataset is sent accross the network. This
**			allows the application to monitor the rate of
**			transmission and cancel the transmission.
**	callbackCtx	User context information which is passed to the
**			callback function as a parameter.
**	length		Length in bytes of the amount of data to transmit
**			over the network before calling the user's callback
**			function.
**
** Return Values:
**
**	SRV_NORMAL
**	SRV_NOTRANSFERSYNTAX
**	SRV_OBJECTACCESSFAILED
**	SRV_SENDFAILED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_SendDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * context, DCM_OBJECT ** object,	CONDITION(*callback) (), void *callbackCtx, unsigned long length)
{
    /* repair */
    unsigned char        	buf[16384];
    unsigned long        	options;
    CONDITION				cond;
    DUL_PDVLIST				pdvListSend;
    DUL_PDV					pdv;
    CALLBACK_STRUCTURE		callbackStructure;

    callbackStructure.association = *association;
    callbackStructure.pdvList = &pdvListSend;
    callbackStructure.callback = callback;
    callbackStructure.ctx = callbackCtx;
    callbackStructure.bytesTransmitted = 0;

    cond = DCM_GetObjectSize(object, &callbackStructure.totalBytes);
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_OBJECTACCESSFAILED, SRV_Message(SRV_OBJECTACCESSFAILED), "data set", "SRV_SendDataSet");

    if (strcmp(context->acceptedTransferSyntax, DICOM_TRANSFERLITTLEENDIAN)	== 0){
    	options = DCM_ORDERLITTLEENDIAN;
    }else{
    	return COND_PushCondition(SRV_NOTRANSFERSYNTAX, SRV_Message(SRV_NOTRANSFERSYNTAX), context->abstractSyntax, context->acceptedTransferSyntax, "SRV_SendDataSet");
    }
    /*cond = DCM_EXPORTINCOMPLETE; ZT*/
    pdv.presentationContextID = context->presentationContextID;
    pdv.pdvType = DUL_DATASETPDV;
    pdv.data = buf;

    pdvListSend.count = 1;
    pdvListSend.pdv = &pdv;
    {
	 cond = DCM_ExportStream(object, options, buf, sizeof(buf), writeCallback, &callbackStructure);
	 if (ERROR(cond)) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "DATA SET", "SRV_SendDataSet");
    }

    if (cond != DCM_NORMAL) {
    /*if (cond != DCM_EXPORTCOMMANDEND && cond != DCM_NORMAL) { ZT */
    	printf("Unexepected condition after export command: %x\n", cond);
    	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "DATA SET", "SRV_SendDataSet");
    }
    return SRV_NORMAL;
}

/* SRV_ReceiveDataSet
**
** Purpose:
**	Poll an Association and read what is expected to be a data set.
**
** Parameter Dictionary:
**	association	The key used to receive the database from the network.
**	presentationCtx	Presentation context on which we expect to receive
**			the dataset.
**	block		Flag to be passed to network routines for blocking or
**			non-blocking I/O.
**	timeout		Timeout passed to network routines.
**	dirName		Directory to be used to create file for (possibly large)**			data sets.
**	dataSet		Address of DICOM object variable which will be created
**			when data set is received.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_MALLOCFAILURE
**	SRV_NORMAL
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**
** Algorithm:
**	Notes : dirName is an optional parameter. Some datasets are too large
**		to store directly in memory. When the function determines that
**		a large dataset is being read, it will write it to a file. This
**		file will be stored in the directory indicated by the parameter
**		dirName. If dirName is empty or NULL, the current working
**		directory is assumed.
*/

CONDITION
SRV_ReceiveDataSet(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * presentationCtx, DUL_BLOCKOPTIONS block, int timeout, char *dirName, DCM_OBJECT ** dataSet)
{
    CONDITION					cond;
    unsigned char        		stream[16384], *s;
    unsigned long        		bytesRead;
    DUL_DATAPDV					type;
    CTNBOOLEAN					last;
    DUL_PDV						pdv;
    DCM_OBJECT					* object;
    DUL_PRESENTATIONCONTEXTID	ctxID = 0;
    int	        				fd = 0; /* File descriptor if we need to create file */
    char        				qualifiedFile[1024];

    /* First we deal with the various values possible for the argument dirName  */
    if ((dirName == NULL) || (strlen(dirName) == 0)) {
    	/* NULL or empty argument passed */
    	dirName = malloc(sizeof(char) + 1);
    	if (dirName == NULL) return COND_PushCondition(SRV_MALLOCFAILURE, SRV_Message(SRV_MALLOCFAILURE), sizeof(*dirName), "SRV_ReceiveDataSet");
    	(void) strcpy(dirName, ".");	/* make the directory name as current working directory */
    }
    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_DATASETPDV; type == DUL_DATASETPDV && !last;) {
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)){
    		if (fd == 0){
    			cond = createFile(dirName, qualifiedFile, &fd);
    			if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    		}
    		if (write(fd, stream, bytesRead) != bytesRead) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");

    		s = stream;
    		bytesRead = 0;
    	}
    	if (ctxID == 0) ctxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != ctxID){
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    	}
    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    }
    if (type != DUL_DATASETPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    }
    if (fd != 0) {
    	if (write(fd, stream, bytesRead) != bytesRead) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");

    	(void) close(fd);
    	cond = DCM_OpenFile(qualifiedFile, DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION | DCM_DELETEONCLOSE, &object);
    }else{
    	/* repair */
    	cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION, &object);
    }
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveDataSet");
    if (PRVSRV_debug) (void) DCM_DumpElements(&object,0);

    *dataSet = object;

    return SRV_NORMAL;
}

/* SRV_MessageIDOut
**
** Purpose:
**	Get a unique message ID which can be used in a DICOM command.
**
** Parameter Dictionary:
**	NONE
**
** Return Values:
**	Unique message ID.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

unsigned short
SRV_MessageIDOut()
{
    static unsigned short        messageID = 1;

    return messageID++;
}

/* SRV_MessageIDIn
**
** Purpose:
**	Function to reclaim ID messages after they have been used.
**
** Parameter Dictionary:
**	messageID	ID to be reclaimed.
**
** Return Values:
**	NONE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

void
SRV_MessageIDIn(unsigned short messageID)
{
}

/* SRV_TestForCancel
**
** Purpose:
**	Check the status of the network and read a cancel request
**	(if present).
**
** Parameter Dictionary:
**	association	The key which describes which association to use to
**			check the network for a command.
**	params		The set of service parameters which describe the
**			services which are valid for this association.
**	block		A flag indicating if the caller wishes to block
**			waiting for a command (DUL_BLOCK) or return
**			immediately if there is no command present
**			(DUL_NOBLOCK).
**	timeout		If the application chooses to block and wait for a
**			command, the amount of time to wait before returning
**			to the caller (in seconds).
**	ctxId		The presentation context ID indicating under which
**			context we might expect a CANCEL Request.
**	command		Pointer to caller variable where this function will
**			store the command value from the COMMAND group which
**			was read from the network.
**	messageType	Pointer to caller variable where this function will
**			store one of the enumerated message types from the
**			MSG facility. There should be a one to one
**			correspondence between the COMMAND received from the
**			network and this messageType.
**	messageArg	Address of pointer in caller's space. This function
**			allocates a MSG structure and writes the address of the
**			allocated structure in the caller's messageArg pointer.
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PARSEFAILED
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READTIMEOUT
**	SRV_RECEIVEFAILED
**	SRV_UNSUPPORTEDCOMMAND
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
SRV_TestForCancel(DUL_ASSOCIATIONKEY ** association, DUL_BLOCKOPTIONS block, int timeout, DUL_PRESENTATIONCONTEXTID ctxID, unsigned short *command,
				  MSG_TYPE * messageType, void **messageArg)
{
    CONDITION    					cond;
    unsigned char        			stream[8192], *s;
    unsigned long        			bytesRead;
    DUL_DATAPDV						type;
    CTNBOOLEAN							last;
    DUL_PDV							pdv;
    DCM_OBJECT						* object;
    MSG_GENERAL						** msg;
    static unsigned short        	dcmCommand;
    COMMAND_ENTRY					commandEntry;
    DUL_PRESENTATIONCONTEXTID		localCtxID;

    cond = dequeCommand(association, ctxID, MSG_K_C_CANCEL_REQ, &commandEntry);
    if (cond == SRV_NORMAL) {
    	*command = commandEntry.command;
    	*messageType = commandEntry.messageType;
    	*messageArg = commandEntry.message;
    	return SRV_NORMAL;
    }
    msg = (MSG_GENERAL **) messageArg;
    localCtxID = 0;

    for (s = stream, last = FALSE, bytesRead = 0, type = DUL_COMMANDPDV; type == DUL_COMMANDPDV && !last;) {
    	cond = SRVPRV_ReadNextPDV(association, block, timeout, &pdv);
    	if (cond != SRV_NORMAL) {
    		if (cond == SRV_READPDVFAILED){
    			return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    		}else{
    			return cond;
    		}
    	}
    	if (bytesRead + pdv.fragmentLength > sizeof(stream)){
    		(void) COND_PushCondition(SRV_OBJECTTOOLARGE, SRV_Message(SRV_OBJECTTOOLARGE));
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	if (localCtxID == 0) localCtxID = pdv.presentationContextID;

    	if (pdv.presentationContextID != localCtxID) {
    		(void) COND_PushCondition(SRV_UNEXPECTEDPRESENTATIONCONTEXTID, SRV_Message(SRV_UNEXPECTEDPRESENTATIONCONTEXTID), (long) pdv.presentationContextID);
    		return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    	}
    	(void) memcpy(s, pdv.data, pdv.fragmentLength);
    	bytesRead += pdv.fragmentLength;
    	s += pdv.fragmentLength;
    	last = pdv.lastPDV;
    	type = pdv.pdvType;
    }
    if (type != DUL_COMMANDPDV) {
    	(void) COND_PushCondition(SRV_UNEXPECTEDPDVTYPE, SRV_Message(SRV_UNEXPECTEDPDVTYPE));
    	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    }
    /* repair */
    cond = DCM_ImportStream(stream, bytesRead, DCM_ORDERLITTLEENDIAN, &object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
    if (PRVSRV_debug) (void) DCM_DumpElements(&object,0);

    /* repair */
    {
	 void 					*ctx;
	 U32 					elementLength;
	 static DCM_ELEMENT 	element = {DCM_CMDCOMMANDFIELD, DCM_US, "", 1, sizeof(dcmCommand), (void *) &dcmCommand};

	 ctx = NULL;
	 cond = DCM_GetElementValue(&object, &element, &elementLength, &ctx);
	 if (cond != DCM_NORMAL) {
		 (void) DCM_CloseObject(&object);
		 return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
	 }
	 switch (dcmCommand) {
		 case DCM_CANCEL_REQUEST:
										 *msg = NULL;
										 cond = MSG_ParseCommand(&object, (void **) msg);
										 (void) DCM_CloseObject(&object);
										 if (cond != MSG_NORMAL) return COND_PushCondition(SRV_PARSEFAILED, SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");
										 *messageType = (*msg)->type;
										 if (command != NULL) *command = dcmCommand;
										 break;
		 case DCM_ECHO_REQUEST:
		 case DCM_ECHO_RESPONSE:
		 case DCM_STORE_REQUEST:
		 case DCM_STORE_RESPONSE:
		 case DCM_FIND_REQUEST:
		 case DCM_FIND_RESPONSE:
		 case DCM_MOVE_REQUEST:
		 case DCM_MOVE_RESPONSE:
		 case DCM_GET_REQUEST:
		 case DCM_GET_RESPONSE:
		 case DCM_N_EVENT_REPORT_REQUEST:
		 case DCM_N_EVENT_REPORT_RESPONSE:
		 case DCM_N_GET_REQUEST:
		 case DCM_N_GET_RESPONSE:
		 case DCM_N_SET_REQUEST:
		 case DCM_N_SET_RESPONSE:
		 case DCM_N_ACTION_REQUEST:
		 case DCM_N_ACTION_RESPONSE:
		 case DCM_N_CREATE_REQUEST:
		 case DCM_N_CREATE_RESPONSE:
		 case DCM_N_DELETE_REQUEST:
		 case DCM_N_DELETE_RESPONSE:
										commandEntry.message = NULL;

										cond = MSG_ParseCommand(&object, (void **) &commandEntry.message);
										(void) DCM_CloseObject(&object);
										if (cond != MSG_NORMAL)	return COND_PushCondition(SRV_PARSEFAILED, SRV_Message(SRV_PARSEFAILED), "SRV_ReceiveCommand");

										commandEntry.association = association;
										commandEntry.ctxID = localCtxID;
										commandEntry.command = dcmCommand;
										commandEntry.messageType = ((MSG_GENERAL *) commandEntry.message)->type;

										cond = enqueCommand(&commandEntry);
										if (cond != SRV_NORMAL) return COND_PushCondition(SRV_RECEIVEFAILED, SRV_Message(SRV_RECEIVEFAILED), "SRV_ReceiveCommand");
										break;

		 default:
									    return COND_PushCondition(SRV_UNSUPPORTEDCOMMAND, SRV_Message(SRV_UNSUPPORTEDCOMMAND), (int) dcmCommand, "SRV_ReceiveCommand");
	 }
    }

    return SRV_NORMAL;
}

/*
**  =======================================================
**  Following are  private functions which are not available outside
**  this module
*/

/* SRVPRV_ReadNextPDV
**
** Purpose:
**	Read the next PDV
**
** Parameter Dictionary:
**	association	Handle to the Association
**	block		Blocking options
**	timeout		Time interval within which to read
**	pdv		Handle to the pdv
**
** Return Values:
**
**	SRV_ILLEGALASSOCIATION
**	SRV_NORMAL
**	SRV_PEERABORTEDASSOCIATION
**	SRV_PEERREQUESTEDRELEASE
**	SRV_READPDVFAILED
**	SRV_READTIMEOUT
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SRVPRV_ReadNextPDV(DUL_ASSOCIATIONKEY ** association, DUL_BLOCKOPTIONS block, int timeout, DUL_PDV * pdv)
{
    CONDITION		cond;

    cond = DUL_NextPDV(association, pdv);
    if (cond != DUL_NORMAL) {
    	(void) COND_PopCondition(FALSE);
    	cond = DUL_ReadPDVs(association, &pdvList, block, timeout);
    	if (cond != DUL_PDATAPDUARRIVED) {
    		if (cond == DUL_NULLKEY || cond == DUL_ILLEGALKEY){
    			return COND_PushCondition(SRV_ILLEGALASSOCIATION, SRV_Message(SRV_ILLEGALASSOCIATION));
    		}else if (cond == DUL_PEERREQUESTEDRELEASE){
    			return COND_PushCondition(SRV_PEERREQUESTEDRELEASE, SRV_Message(SRV_PEERREQUESTEDRELEASE), "SRVPRV_ReadNextPDV");
    		}else if (cond == DUL_PEERABORTEDASSOCIATION){
    			return COND_PushCondition(SRV_PEERABORTEDASSOCIATION, SRV_Message(SRV_PEERABORTEDASSOCIATION), "SRVPRV_ReadNextPDV");
    		}else if (cond == DUL_READTIMEOUT){
    			return SRV_READTIMEOUT;
    		}else{
    			return COND_PushCondition(SRV_READPDVFAILED, SRV_Message(SRV_READPDVFAILED), "SRVPRV_ReadNextPDV");
    		}
    	}
    	cond = DUL_NextPDV(association, pdv);
    	if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_READPDVFAILED, SRV_Message(SRV_READPDVFAILED));
    }
    return SRV_NORMAL;
}

/* SRV_Debug
**
** Purpose:
**	Set debug flag in this module and in the other modules.
**
** Parameter Dictionary:
**	flag	Boolean flag indicating if debug mode is to be ON or OFF.
**
** Return Values:
**	NONE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

void
SRV_Debug(CTNBOOLEAN flag)
{
    PRVSRV_debug = flag;
}

/* writeCallback
**
** Purpose:
**	Call back routine for write.
**
** Parameter Dictionary:
**	buffer		Buffer to be written
**	length		Length of buffer
**	last		Indicates if it is the last PDU
**	callbackStructure
**			Handle to the call back structure
**
** Return Values:
**	DCM_NORMAL
**	SRV_SENDFAILED
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
writeCallback(void *buffer, unsigned long length, int last, CALLBACK_STRUCTURE * callbackStructure)
{
    CONDITION    cond;
    DUL_PDV		* pdv;

    pdv = callbackStructure->pdvList->pdv;
    pdv->fragmentLength = length;
    pdv->lastPDV = last;

    cond = DUL_WritePDVs(&callbackStructure->association, callbackStructure->pdvList);
    if (cond != DUL_NORMAL)	return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "unknown", "writeCallback");

    callbackStructure->bytesTransmitted += length;
    if (callbackStructure->callback != NULL){
    	cond = callbackStructure->callback(callbackStructure->bytesTransmitted, callbackStructure->totalBytes, callbackStructure->ctx);
    	if (!SUCCESS(cond)) return COND_PushCondition(SRV_SENDFAILED, SRV_Message(SRV_SENDFAILED), "unknown", "writeCallback");
    }
    return DCM_NORMAL;
}


/* createFile
**
** Purpose:
**	Creates a file in the directory when large datasets are received.
**
** Parameter Dictionary:
**	dirName		Directory in which file is to be created.
**	qualifiedFile	Name assigned to the file which is created.
**	fd		File descriptor.
**
** Return Values:
**
**	SRV_FILECREATEFAILED
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
createFile(char *dirName, char *qualifiedFile, int *fd)
{
    static int        fileIndex = 1;		/* For creating unique file names */
    int		          pid;

    pid = getpid();
    sprintf(qualifiedFile, "%s/%-d.%-d", dirName, pid, fileIndex++);
    *fd = open(qualifiedFile, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (*fd < 0) {
    	(void) COND_PushCondition(SRV_SYSTEMERROR, SRV_Message(SRV_SYSTEMERROR), strerror(errno), "createFile");
    	return COND_PushCondition(SRV_FILECREATEFAILED, SRV_Message(SRV_FILECREATEFAILED), qualifiedFile, "createFile");
    }else{
    	return SRV_NORMAL;
    }
}


/* verifyCommandValidity
**
** Purpose:
**	Verify if the command is valid for the proposed abstract syntax
**
** Parameter Dictionary:
**	params		The association service parameters which contain the
**			list of all the abstract syntaxes accepted on
**			the association.
**	ctxid		The context id which determines the current abstract
**			syntax for which the command is received
**	command		The DICOM normalized/composite service being requested
**			for the abstract syntax.
**
** Return Values:
**	SRV_NORMAL
**	SRV_UNSUPPORTEDCOMMAND
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
verifyCommandValidity(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXTID ctxid, unsigned short command)
{

    /* verify if the command is valid for the given abstract syntax i.e. SOP class */
    int      					  index1, index2;
    DUL_PRESENTATIONCONTEXT	      * ctx;

    /* using the ctxid, we first extract the abstract syntax for which the command has been requested. */
    ctx = findPresentationCtx(params, ctxid);
    if (ctx == NULL) return SRV_UNSUPPORTEDCOMMAND;

    /*
     * From the current presentation context we have obtained, extract the
     * abstract syntax and find the list of valid commands for that abstract
     * syntax and then verify if the requested command is valid or not
     */
    for (index1 = 0; index1 < DIM_OF(classPermittedSrvTable); index1++) {
    	if (strcmp(classPermittedSrvTable[index1].classUID, ctx->abstractSyntax) == 0){	/* they matched */
    		for (index2 = 0; index2 < (int) classPermittedSrvTable[index1].permittedSrvListSize; index2++){
    			if (classPermittedSrvTable[index1].permittedSrvList[index2] == command) return SRV_NORMAL;
    		}
    		return SRV_UNSUPPORTEDCOMMAND;
    	}
    }
    return SRV_UNSUPPORTEDCOMMAND;
}


/* dequeCommand
**
** Purpose:
**	Dequeue the command
**
** Parameter Dictionary:
**	association		Handle to the Association
**	ctxID			Presentation context ID
**	messageType		Type of the message
**	commandEntry		Handle to command entry in the table
**
** Return Values:
**
**	SRV_EMPTYCOMMANDQUEUE
**	SRV_NOAVAILABLECOMMAND
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
dequeCommand(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXTID ctxID, MSG_TYPE messageType, COMMAND_ENTRY * commandEntry)
{
    COMMAND_ENTRY    * c;
    CTNBOOLEAN		 match;

    if (commandList == NULL) return SRV_EMPTYCOMMANDQUEUE;

    c = LST_Head(&commandList);
    if (c == NULL) return SRV_EMPTYCOMMANDQUEUE;

    (void) LST_Position(&commandList, c);
    match = FALSE;

    while ((c != NULL) && !match){
    	match = TRUE;
    	if (c->association != association) match = FALSE;
    	if ((ctxID != 0) && (c->ctxID != ctxID)) match = FALSE;
    	if ((messageType != MSG_K_NONE) && (c->messageType != messageType)) match = FALSE;
    	if (!match) c = LST_Next(&commandList);
    }
    if (match){
    	*commandEntry = *c;
    	(void) LST_Remove(&commandList, LST_K_AFTER);
    	free(c);
    	return SRV_NORMAL;
    }else{
    	return SRV_NOAVAILABLECOMMAND;
    }
}

/* enqueCommand
**
** Purpose:
**	Enqueue the command
**
** Parameter Dictionary:
**	commandEntry		Handle to the command entry
**
** Return Values:
**
**	SRV_LISTFAILURE
**	SRV_MALLOCFAILURE
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
enqueCommand(COMMAND_ENTRY * commandEntry)
{
    COMMAND_ENTRY    * c, *tail;

    if (commandList == NULL) commandList = LST_Create();
    if (commandList == NULL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "enqueCommand");

    c = malloc(sizeof(*c));
    if (c == NULL) return COND_PushCondition(SRV_MALLOCFAILURE,	SRV_Message(SRV_MALLOCFAILURE), sizeof(*c), "enqueCommand");

    *c = *commandEntry;

    tail = LST_Tail(&commandList);
    (void) LST_Position(&commandList, tail);
    if (LST_Insert(&commandList, c, LST_K_AFTER) != LST_NORMAL) return COND_PushCondition(SRV_LISTFAILURE, SRV_Message(SRV_LISTFAILURE), "enqueCommand");

    return SRV_NORMAL;
}

/* findPresentationCtx
**
** Purpose:
**	Find the presentation context in the service parameters using the
**	context ID
**
** Parameter Dictionary:
**	params		Service parameters
**	ctxid		Context ID using which the presentation context
**			is to be returned
**
** Return Values:
**	Handle to the presentation context, if found, else NULL.
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static DUL_PRESENTATIONCONTEXT
*
findPresentationCtx(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXTID ctxid)
{
    DUL_PRESENTATIONCONTEXT    * ctx;

    if (params->acceptedPresentationContext == NULL) return NULL;
    ctx = LST_Head(&params->acceptedPresentationContext);
    if (ctx == NULL) return NULL;
    (void) LST_Position(&params->acceptedPresentationContext, ctx);

    while (ctx != NULL) {
    	if (ctx->presentationContextID == ctxid) break;	/* context id found */
    	ctx = LST_Next(&params->acceptedPresentationContext);
    }

    return ctx;
}
