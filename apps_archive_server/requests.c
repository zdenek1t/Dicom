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
**				DICOM 94
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
** Author, Date:	Stephen Moore, 11-Feb-94
** Intent:
** Last Update:		$Author: smm $, $Date: 2002/12/13 15:20:17 $
** Source File:		$RCSfile: requests.c,v $
** Revision:		$Revision: 1.16 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.16 $ $RCSfile: requests.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/info_entity/dicom_ie.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"
#ifdef CTN_MULTIBYTE
#include "tblmb.h"
#include "idbmb.h"
#else
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/idb/idb.h"
#endif
#include "../dicom_lib/manage/manage.h"
//#include "fis.h"
//#include "copy.h"

#include "image_archive.h"
//#include "find.h"
//#include "move.h"
//#include "cget.h"
//#include "../dicom_lib/services/naction.h"
#include "../dicom_lib/gq/gq.h"
#include "../dicom_lib/apps_include/iqueues.h"

extern char 		*controlDatabase;
extern CTNBOOLEAN 	doVerification;
extern int 			queueNumber;
static char 		lastImageInAssociation[512] = "";
static char 		callingAPTitle[20] = "";
static int 			receivedImageCount = 0;

typedef struct {
    char 			*fileName;
    char 			*transferSyntax;
    char 			*owner;
    char 			*groupName;
    char 			*priv;
    DMAN_HANDLE 	**handle;
    int  			imageCount;
}   STORAGE_PARAMS;

static IDB_HANDLE 	*IDBHandle;

#ifdef FIS
static FIS_HANDLE 	*fisHandle;
#endif

static CONDITION
serviceThisCommand(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_TYPE messageType, void **message, DUL_ASSOCIATESERVICEPARAMETERS * params, DMAN_HANDLE ** handle);
static CONDITION
echoRequest(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_ECHO_REQ ** message);
static CONDITION
storeRequest(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_ASSOCIATIONKEY ** association, DMAN_HANDLE ** handle, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_STORE_REQ ** message);
static CONDITION
echoCallback(MSG_C_ECHO_REQ * echoRequest, MSG_C_ECHO_RESP * echoResponse, void *ctx, DUL_PRESENTATIONCONTEXT * pc);
static CONDITION
storageCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response,	unsigned long received, unsigned long estimate,DCM_OBJECT ** object, void *paramsPtr, DUL_PRESENTATIONCONTEXT * pc); /*DCM_OBJECT ** object, STORAGE_PARAMS * params, */
static CONDITION
storeImage(DMAN_HANDLE ** handle, char *origFileName, char *transferSyntax, char *SOPClass, char *owner, char *groupName, char *priv, DCM_OBJECT ** object);

extern CTNBOOLEAN 	silent;

/* serviceRequests
**
** Purpose:
**	This function reads requests from the network and services those
**	requests.
**
** Parameter Dictionary:
**	network		The key which is used to access the network.
**	association	They key which is used to access the association
**			on which requests are received.
**	service		The parameter list which describes the association.
**			This list includes the list of presentation contexts
**			for the association.
**	reducedCapability If true, we are operating with reduced capability.
**			This means we will not use FIS for anything.
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
serviceRequests(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * service,	CTNBOOLEAN reducedCapability)
{
    CONDITION					cond;
    DUL_PRESENTATIONCONTEXT		* ctx;
    DUL_PRESENTATIONCONTEXTID	ctxID;
    void      					*message;
    MSG_TYPE					messageType;
    CTNBOOLEAN 					networkLink = TRUE, commandServiced;
    DMAN_HANDLE 				* handle;
    DMAN_STORAGEACCESS			storage;
#ifdef FIS
    DMAN_FISACCESS 				fisAccess;
#endif
    receivedImageCount = 0;

    queueInitialize();
    strcpy(callingAPTitle, service->callingAPTitle);
    queueNewAssociation(service->callingAPTitle);
    queueNetworkNewAssociation(service->callingAPTitle);

    cond = DMAN_Open(controlDatabase, service->callingAPTitle, service->calledAPTitle, &handle);
    if (cond != DMAN_NORMAL) fprintf(stderr, "Unable to open control database\n");

    if (cond == DMAN_NORMAL) cond = DMAN_LookupStorage(&handle, service->calledAPTitle, &storage);
    if (cond != DMAN_NORMAL){
    	fprintf(stderr, "Unable to lookup storage\n");
    	COND_DumpConditions();
    }
    if (cond == DMAN_NORMAL) {
    	cond = IDB_Open(controlDatabase, &IDBHandle);
#ifdef CTN_MULTIBYTE
    	IDB_SetCharSet(&IDBHandle, storage.Comment);
#endif
    }

    if (cond != IDB_NORMAL) fprintf(stderr, "Unable to open IDB tables\n");

#ifdef FIS
    if (!reducedCapability && !CTN_ERROR(cond)){
    	cond = DMAN_LookupFISAccess(&handle, service->calledAPTitle, &fisAccess);
    	if (!CTN_ERROR(cond)) cond = FIS_Open(fisAccess.DbKey, &fisHandle);
    }
#endif

    while ((networkLink == TRUE) && !CTN_ERROR(cond)) {
    	cond = SRV_ReceiveCommand(association, service, DUL_BLOCK, 0, &ctxID, NULL, &messageType, &message);
    	if (cond == SRV_PEERREQUESTEDRELEASE){
    		networkLink = FALSE;
    		(void)COND_PopCondition(TRUE);
    		(void)DUL_AcknowledgeRelease(association);
    		(void)DUL_DropAssociation(association);
	    }else if (cond == SRV_PEERABORTEDASSOCIATION){
	    	networkLink = FALSE;
	    	(void) DUL_DropAssociation(association);
	    }else if (cond != SRV_NORMAL) {
	    	(void) DUL_DropAssociation(association);
	    	COND_DumpConditions();
	    	cond = 2;
	    }else{
	    	ctx = LST_Head(&service->acceptedPresentationContext);
	    	if (ctx != NULL) (void) LST_Position(&service->acceptedPresentationContext, ctx);
	    	commandServiced = FALSE;

	    	while (ctx != NULL && !commandServiced){
	    		if (ctx->presentationContextID == ctxID) {
	    			if (commandServiced){
	    				if (!silent) printf(stderr, "Context ID Repeat in serviceRequests (%p)\n", ctxID);
	    			}else{
	    				cond = serviceThisCommand(network, association, ctx, messageType, &message, service, &handle);
	    				if (cond == SRV_OPERATIONCANCELLED) {
	    					if (!silent) printf("Operation canceled\n");
	    					(void) COND_PopCondition(TRUE);
	    				}else if (cond != SRV_NORMAL){
	    					if (!silent) printf("Operation not serviced\n");
	    					COND_DumpConditions();
	    				}
	    				commandServiced = TRUE;
	    			}
	    		}
	    		ctx = LST_Next(&service->acceptedPresentationContext);
	    	}
	    	if (!commandServiced) {
	    		fprintf(stderr, "In serviceRequests, context ID %d not found\n", ctxID);
	    		(void) DUL_DropAssociation(association);
	    		networkLink = FALSE;
	    	}
	    }
    }

    if (!silent) COND_DumpConditions();
    queueClosedAssociation(service->callingAPTitle, receivedImageCount);

    if (lastImageInAssociation[0] != '\0') queueDisplayImage(service->callingAPTitle, lastImageInAssociation);

    (void) DMAN_Close(&handle);
    (void) IDB_Close(&IDBHandle);
#ifdef FIS
    if (!reducedCapability) (void) FIS_Close(&fisHandle);
#endif
    (void) DUL_ClearServiceParameters(service);
    free(service);
    return cond;
}

/* serviceThisCommand
**
** Purpose:
**	This function serves as a dispatch routine for the commands
**	that can be received from the network.  It uses a case statement
**	to identify the command and call the function which will
**	respond to the request.
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	messageType	The type of message that we are to recognize.
**	message		Pointer to a structure which contains the message.
**			We will use "messageType" to get the proper type.
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
serviceThisCommand(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_TYPE messageType,
				   void **message, DUL_ASSOCIATESERVICEPARAMETERS * params, DMAN_HANDLE ** handle)
{
    CONDITION    	cond;
    MSG_GENERAL  	* general;

    general = *(MSG_GENERAL **) message;

    if (!silent) MSG_DumpMessage((void *) general, stdout);

    switch (messageType) {
		case MSG_K_C_ECHO_REQ:
	                          cond = echoRequest(association, ctx, (MSG_C_ECHO_REQ **) message);
	                          break;
		case MSG_K_C_STORE_REQ:
	                          cond = storeRequest(params, association, handle, ctx, (MSG_C_STORE_REQ **) message);
	                          break;
		case MSG_K_C_FIND_REQ:
	                          cond = findRequest(association, params, ctx, (MSG_C_FIND_REQ **) message, &IDBHandle);
	                          break;
		case MSG_K_C_MOVE_REQ:
	                          cond = moveRequest(network, association, ctx, (MSG_C_MOVE_REQ **) message, params, &IDBHandle, handle);
	                          break;
		case MSG_K_C_GET_REQ:
	                          cond = cgetRequest(network, association, ctx, (MSG_C_GET_REQ **) message, params, &IDBHandle,	handle);
	                          break;
		case MSG_K_C_CANCEL_REQ:
	                          cond = SRV_NORMAL;	/* This must have happened after we were done */
	                          break;
		case MSG_K_N_ACTION_REQ:
#ifdef FIS
							  cond = nactionRequest(params, association, handle, ctx, (MSG_N_ACTION_REQ **) message, &fisHandle, &IDBHandle);
#else
	                          fprintf(stderr, "Unimplemented message type (N_ACTION_REQ): %d\n", messageType);
	                          cond = SRV_UNSUPPORTEDSERVICE;
	                          //cond = 1;
#endif
							  break;

		default:
	                          fprintf(stderr, "Unimplemented message type: %d\n", messageType);
	                          cond = SRV_UNSUPPORTEDSERVICE;
	                          //cond = 1;
	                          break;
    }
    return cond;
}


/* echoRequest
**
** Purpose:
**	This function responds to an echo request from the network.
**	It creates an echo response message with a status of success
**	and sends the message to the peer application.
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	message		Pointer to the MSG_C_ECHO_REQ message that was
**			received by the server.
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

static CONDITION
echoRequest(DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_ECHO_REQ ** echoReq)
{
    MSG_C_ECHO_RESP echoResponse = {MSG_K_C_ECHO_RESP, 0, 0, DCM_CMDDATANULL, 0, ""};
    return SRV_CEchoResponse(association, ctx, echoReq, &echoResponse, echoCallback, NULL, "");
}

/* echoCallback
**
** Purpose:
**	Call back routine provided by the service provider. It is invoked
**	by the SRV Echo Response function.
**
** Parameter Dictionary:
**	request		Pointer to C-Echo Request Message
**	response	Pointer to C-Echo Response Message
**	ctx		Context information that we ignore
**	pc		Presentation context pointer that we ignore
**
** Return Values:
**	SRV_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
echoCallback(MSG_C_ECHO_REQ * request, MSG_C_ECHO_RESP * response, void *ctx, DUL_PRESENTATIONCONTEXT * pc)
{
    if (!silent) printf("Echo Request Received/Acknowledged\n");

    response->dataSetType = DCM_CMDDATANULL;
    response->messageIDRespondedTo = request->messageID;
    response->status = MSG_K_SUCCESS;
    strcpy(response->classUID, request->classUID);

    return SRV_NORMAL;
}


/* storeRequest
**
** Purpose:
**	This function responds to a request to store an image.
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	message		Pointer to the MSG_C_STORE_REQ message that was
**			received by the server.
**
** Return Values:
**
**	SRV_FILECREATEFAILED
**	SRV_NORMAL
**	SRV_OBJECTBUILDFAILED
**	SRV_RESPONSEFAILED
**	SRV_UNEXPECTEDPDVTYPE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/


static CONDITION
storeRequest(DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_ASSOCIATIONKEY ** association, DMAN_HANDLE ** handle, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_STORE_REQ ** request)
{
    char              	fileName[1024];
    MSG_C_STORE_RESP	response;
    CONDITION 	      	cond;
    STORAGE_PARAMS	  	storageParams;
    static int        	imageCount = 0;

    imageCount++;

    cond = DMAN_TempImageFile(handle, (*request)->classUID, fileName, sizeof(fileName)); /* Get Temp file name */
    if (cond != DMAN_NORMAL){
    	COND_DumpConditions();
    	fileName[0] = '\0';
    }else{
#ifdef DEBUG
    	fprintf(stderr, "Temp file name: %s\n", fileName);
#endif
    }

    storageParams.fileName = fileName;
    storageParams.transferSyntax = ctx->acceptedTransferSyntax;
    storageParams.handle = handle;
    storageParams.owner = params->callingAPTitle;
    storageParams.groupName = "";
    storageParams.priv = "";
    storageParams.imageCount = imageCount;

    queueNewImage(params->callingAPTitle, imageCount);

    cond = SRV_CStoreResponse(association, ctx, request, &response, fileName, storageCallback, &storageParams, ""); /* Save Temp file */

#ifdef _MSC_VER
    _unlink(fileName);
#endif
    return cond;
}


/* storageCallback
**
** Purpose:
**	Call back routine provided by the service provider and invoked by
**	the C-STORE response routine
**
** Parameter Dictionary:
**	request		Pointer to C-STORE request message
**	response	Pointer to C-STORE response message
**	received	Number of bytes received so far
**	estimate	Estimated number of bytes in the data set
**	object		Handle to DICOM object
**	params		Context information which is a set of pointers we
**			use for storing the image and data in the database.
**	pc		Presentation context which describes this SOP class.
**
** Return Values:
**	SRV_NORMAL
**
** Notes:
**	We pass the hand
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
storageCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response,	unsigned long received, unsigned long estimate,
				DCM_OBJECT ** object, void *paramsPtr, DUL_PRESENTATIONCONTEXT * pc)
{
    CONDITION    	cond = 0;
    IE_OBJECT    	* ieObject;
/*  The definition and assignment help satisfy prototypes for this callback function (defined by dicom_services.h) */
    STORAGE_PARAMS 	*params;
    int 			percentage;

    params = (STORAGE_PARAMS *) paramsPtr;

    percentage = (100 * received) / estimate;
    if (percentage > 100) percentage = 100;

    networkqueuePartialImage(callingAPTitle, percentage);

    if (!silent) printf("%8ld bytes received of %8ld estimated \n", received, estimate);
    if (!silent && (object != NULL)) (void) DCM_DumpElements(object, 0);

    if (object != NULL) {
    	if (doVerification) {
    		cond = IE_ExamineObject(object, &ieObject);
    		(void) IE_Free((void **) &ieObject);
    		if (cond != IE_NORMAL) {	/* The image does not satisfy Part 3 requirements */
    			response->status = MSG_K_C_STORE_DATASETNOTMATCHSOPCLASSERROR;
    			return SRV_NORMAL;
    		}
    	}
    	cond = storeImage(params->handle, params->fileName, params->transferSyntax, request->classUID, params->owner, params->groupName, params->priv, object);
    }
    if (response != NULL) {
    	if (cond == APP_NORMAL){
    		response->status = MSG_K_SUCCESS;
    	}else{
    		response->status = MSG_K_C_STORE_OUTOFRESOURCES;
    	}
    }
    return SRV_NORMAL;
}

static void
logImageError(const char *msg, const char *fileName)
{
    FILE 		*log;
    time_t 		t;
    char 		*timeString;

    t = time(0);
    timeString = ctime(&t);

    log = fopen("archive_logfile", "a");
    if (log == NULL){
    	fprintf(stderr, "%s %s %s\n", timeString, msg, fileName);
    }else{
    	fprintf(log, "%s %s %s\n", timeString, msg, fileName);
    	fclose(log);
    }
}

#ifdef CTN_IDBV2
typedef struct {
    void 		*reserved[2];
    IDB_Query 	query;
}   STUDY_NODE;

CONDITION LST_Sort(LST_HEAD ** list, size_t nodeSize, int (*compare) ());

static int
compareStudyInsertTimes(STUDY_NODE * p1, STUDY_NODE * p2)
{
    long   		d1, d2;
    double 		t1, t2;

    d1 = UTL_ConvertDatetoLong(p1->query.study.InsertDate);
    d2 = UTL_ConvertDatetoLong(p2->query.study.InsertDate);

    if (d1 > d2){
    	return 1;
    }else if (d1 < d2){
    	return -1;
    }
    t1 = UTL_ConvertTimetoFloat(p1->query.study.InsertTime);
    t2 = UTL_ConvertTimetoFloat(p2->query.study.InsertTime);

    if (t1 > t2){
    	return 1;
    }else if (t1 < t2){
    	return -1;
    }
    return 0;
}

static CONDITION
selectCallback(IDB_Query * query, long count, LST_HEAD * lst)
{
    STUDY_NODE 		*node;

    node = malloc(sizeof(*node));
    node->query = *query;
    (void) LST_Enqueue(&lst, node);
    return IDB_NORMAL;
}

static void
purgeIDB(IDB_HANDLE ** handle, long size, long limit)
{
    CONDITION   	cond;
    IDB_Query   	pssi;
    LST_HEAD    	*lst;
    STUDY_NODE  	*node;
    long        	count = 0;
    IDB_Limits  	limitStructure;

    memset(&pssi, 0, sizeof(pssi));
    lst = LST_Create();

    cond = IDB_Select(handle, STUDY_ROOT, IDB_STUDY_LEVEL, IDB_STUDY_LEVEL, &pssi, &count, selectCallback, lst);
    if (cond != IDB_NORMAL){
    	COND_DumpConditions();
    	return;
    }

    LST_Sort(&lst, sizeof(*node), compareStudyInsertTimes);

    node = LST_Dequeue(&lst);

    while (size > limit && (node != NULL)){
#ifdef DEBUG
    	printf("Deleting study: %s\n", node->query.study.StuInsUID);
#endif
    	cond = IDB_Delete(handle, IDB_STUDY_LEVEL, node->query.study.StuInsUID, TRUE);
    	free(node);
    	memset(&limitStructure, 0, sizeof(limitStructure));
    	cond = IDB_SelectLimits(handle, &limitStructure);
    	size = limitStructure.DBSize;
    	limit = limitStructure.DBLimit;

    	if (size > limit) node = LST_Dequeue(&lst);
    }
    while ((node = LST_Dequeue(&lst)) != NULL){
    	free(node);
    }
    LST_Destroy(&lst);
}

static void
checkStorageLimits(IDB_HANDLE ** handle)
{
    IDB_Limits  	limits;
    CONDITION   	cond;
    long        	l1, l2, l3;

    memset(&limits, 0, sizeof(limits));

    cond = IDB_SelectLimits(handle, &limits);
    if (cond != IDB_NORMAL){
    	COND_DumpConditions();
    	return;
    }
    if (limits.DBSize > limits.DBLimit) {
#ifdef DEBUG
    	printf("Need to purge DB, Size: %10d, limit: %10d\n", limits.DBSize, limits.DBLimit);
#endif
    	purgeIDB(handle, limits.DBSize, limits.DBLimit);
    }
}
#endif

typedef struct {
  char* 	c1;
  char* 	c2;
} STRING_MATRIX;

#ifdef CTN_MULTIBYTE
static CONDITION
checkCharacterSets(DCM_OBJECT** object)
{
  char        imageCharSet[1024];
  CONDITION   cond;
  U32         length = 0;
  void*       ctx = 0;
  char        databaseCharSet[1024];
  int         nullSpecificCharSet = 0;
  int         index;
  CONDITION   returnValue = 0;

  STRING_MATRIX firstElement[] = {
    { "EUCJP", "" },
    { "EUCJP", "ISO_IR 100" },
    { "EUCJP", "ISO_IR 100 " },
    { "EUCJP", "ISO 2022 IR 6" },
    { "EUCJP", "ISO 2022 IR 6 " },
    { "EUCJP", "ISO 2022 IR 100" },
    { "EUCJP", "ISO 2022 IR 100 " },
    { "EUCJPROMAJI", "ISO_IR 13"},
    { "EUCJPROMAJI", "ISO_IR 13 "},
    { "EUCJPROMAJI", "ISO 2022 IR 13"},
    { "EUCJPROMAJI", "ISO 2022 IR 13 "},
    { "EUCKR", "" }
  };
  STRING_MATRIX secondaryElements[] = {
    { "EUCJP", "ISO 2022 IR 87" },
    { "EUCJP", "ISO 2022 IR 87 " },
    { "EUCJP", "ISO 2022 IR 159" },
    { "EUCJP", "ISO 2022 IR 159 " },
    { "EUCJPROMAJI", "ISO 2022 IR 87" },
    { "EUCJPROMAJI", "ISO 2022 IR 87 " },
    { "EUCJPROMAJI", "ISO 2022 IR 159" },
    { "EUCJPROMAJI", "ISO 2022 IR 159 " },
    { "EUCKR", "ISO 2022 IR 149" },
    { "EUCKR", "ISO 2022 IR 149 " }
  };
  char* tok1;
  char* tok2;

  DCM_ELEMENT e = { DCM_IDSPECIFICCHARACTER, DCM_CS, "", 1,	sizeof(imageCharSet), NULL};

  /* Get the Default Character Set.  If the attribute is not there, treat as if the attribute is there but empty. */

  e.d.string = imageCharSet;

  cond = DCM_GetElementValue(object, &e, &length, &ctx);
  if (cond == DCM_ELEMENTNOTFOUND){
	  (void)COND_PopCondition(TRUE);
      strcpy(imageCharSet, "");
  }else if (cond != DCM_NORMAL){
      COND_DumpConditions();
      return 0;
  }else{
      imageCharSet[length] = '\0';
      if (imageCharSet[length-1] == ' ') imageCharSet[length-1] = '\0';
  }

  IDB_GetCharSet(&IDBHandle, databaseCharSet);

  if (imageCharSet[0] == '\\'){
      tok1 = "";
      tok2 = strtok(imageCharSet, "\\");
  }else if (imageCharSet[0] == '\0'){
      tok1 = "";
      tok2 = NULL;
  }else{
      tok1 = strtok(imageCharSet, "\\");
      tok2 = strtok(NULL, "\\");
  }

  returnValue = 0;
  for (index = 0;	index < (int)DIM_OF(firstElement) && returnValue == 0; index++){
	  if ((strcmp(firstElement[index].c1, databaseCharSet) == 0) && (strcmp(firstElement[index].c2, tok1) == 0) ) {
		  returnValue = APP_NORMAL;
	  }
  }
  if (returnValue != APP_NORMAL) return 0;

  while(tok2 != NULL) {
	  returnValue = 0;
	  for (index = 0;	index < (int)DIM_OF(secondaryElements) && returnValue == 0;	index++) {
		  if ((strcmp(secondaryElements[index].c1, databaseCharSet) == 0) && (strcmp(secondaryElements[index].c2, tok2) == 0) ) {
			  returnValue = APP_NORMAL;
		  }
	  }

	  if (returnValue != APP_NORMAL) return 0;

	  tok2 = strtok(NULL, "\\");
  }

  return APP_NORMAL;
}
#endif

static CONDITION
storeImage(DMAN_HANDLE ** handle, char *origFileName, char *transferSyntax, char *SOPClass, char *owner, char *groupName, char *priv, DCM_OBJECT ** object)
{
    IDB_Insertion   Insertion;
    CONDITION       cond;
    char            fileName[256];
    char            *accession;
    unsigned long   objectSize = 0;
    int             i = 0;
    static char     displayedImage[512] = "";

#ifdef CTN_MULTIBYTE
    cond = checkCharacterSets(object);
    if (cond != APP_NORMAL) {
    	fprintf(stderr, "Error on image character sets\n");
    	return 0;
    }
#endif

    memset(&Insertion, 0, sizeof(Insertion));

#ifdef CTN_MULTIBYTE
    cond = parseImageForInsertMB(object, &Insertion);
#else
    cond = parseImageForInsert(object, &Insertion);
#endif

    if (cond != APP_NORMAL) {
    	fprintf(stderr, "Could not parse image for insertion into database\n");
    	COND_DumpConditions();
    	return 0;
    }
    strcpy(Insertion.patient.Owner, owner);
    strcpy(Insertion.patient.GroupName, groupName);
    strcpy(Insertion.patient.Priv, priv);

    if (strlen(Insertion.study.AccNum) != 0){
    	accession = Insertion.study.AccNum;
    }else{
    	accession = Insertion.study.StuInsUID;
    }

    cond = DMAN_PermImageFile(handle, SOPClass, accession, Insertion.series.SerInsUID, fileName, sizeof(fileName));
    if (cond != DMAN_NORMAL){
    	COND_DumpConditions();
    	return 0;
    }else{
#ifdef DEBUG
    	fprintf(stderr, "Perm. file name: %s\n", fileName);
#endif
    }

#ifdef _MSC_VER
    if (!CopyFile(origFileName, fileName, FALSE)){

    	DWORD  		er;
    	LPVOID 		lpMsgBuf;

    	fprintf(stderr, "Unable to move file from %s to %s\n", origFileName, fileName);

    	er = GetLastError();

    	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, er, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) & lpMsgBuf, 0, NULL);

    	printf("%s\n", lpMsgBuf);

    	return 0;

    }
#else
    i = rename(origFileName, fileName);
    if (i != 0){
    	fprintf(stderr, "Unable to rename file from %s to %s\n", origFileName, fileName);
      	i = errno;
      	return 0;
    }
#endif
    strcpy(Insertion.image.Path, fileName);
    strcpy(Insertion.image.Transfer, transferSyntax);

#if 0
    if (strlen(Insertion.patient.PatNam) == 0){
    	logImageError("Zero length patient name", fileName);
    	(void) COND_PopCondition(TRUE);
    	return APP_NORMAL;
    }
    if (strlen(Insertion.patient.PatID) == 0){
    	logImageError("Zero length patient ID", fileName);
    	(void) COND_PopCondition(TRUE);
    	return APP_NORMAL;
    }
#endif

    if (displayedImage[0] == '\0'){
        strcpy(displayedImage, fileName);
        queueDisplayImage(callingAPTitle , displayedImage);
    }else{
        strcpy(lastImageInAssociation, fileName);
    }
    receivedImageCount++;

    (void) DCM_GetObjectSize(object, &objectSize);
    Insertion.image.Size = objectSize;

    cond = IDB_InsertImage(&IDBHandle, &Insertion);
    if (cond != IDB_NORMAL){
#ifdef _MSC_VER
    	_unlink(fileName);
#else
    	fprintf(stderr, "Move wrong file to %s\n", origFileName);
        i = rename(fileName, origFileName);
        if (i != 0){
        	fprintf(stderr, "Unable to rename wrong file from %s to %s\n", fileName, origFileName);
          	i = errno;
          	return 0;
        }
// ZT - Aby se zachoval puvodni soubor
//    	unlink(fileName);
#endif
    	COND_DumpConditions();
    	return 0;
    }

    (void) COND_PopCondition(TRUE);
#ifdef CTN_IDBV2
    checkStorageLimits(&IDBHandle);
#endif


    return APP_NORMAL;
}
