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
** Author, Date:	Stephen Moore, 20-May-94
** Intent:
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:37:38 $
** Source File:		$RCSfile: cget.c,v $
** Revision:		$Revision: 1.6 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.6 $ $RCSfile: cget.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#ifdef MACH
#include <unistd.h>
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

#include "image_archive.h"
#include "cget.h"

extern CTNBOOLEAN silent;

typedef struct {
    DUL_NETWORKKEY                 	**network;
    DUL_ASSOCIATESERVICEPARAMETERS 	*params;
    IDB_HANDLE                     	**handle;
    DMAN_HANDLE                    	**manageHandle;
}   CGET_PARAMS;


typedef struct {
    char 		*levelChar;
    int 		levelInt;
}   QUERY_MAP;

typedef struct {
    void 		*reserved[2];
    IDB_Query 	query;
}   QUERY_LIST_ITEM;

static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst);
static CONDITION
sendCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response, unsigned long transmitted, unsigned long total, char *string);
static CONDITION
cgetCallback(MSG_C_GET_REQ * request, MSG_C_GET_RESP * response,  MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResponse,
	           int responseCount, char *SOPClass, char *queryLevel, void *cgetParams); /* CGET_PARAMS * cgetParams); */
static void
findMoveClasses(LST_HEAD ** SOPClassList);
static int
supportedClass(char *abstractSyntax, char **classArray);

static CTNBOOLEAN waitFlag = FALSE;

/* cgetRequest
**
** Purpose:
**	This function responds to a request to move an image.
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	message		Pointer to the MSG_C_GET_REQ message that was
**			received by the server.
**
** Return Values:
**
**	SRV_ILLEGALPARAMETER
**	SRV_LISTFAILURE
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_OPERATIONCANCELLED
**	SRV_RESPONSEFAILED
**	SRV_SUSPICIOUSRESPONSE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
cgetRequest(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_GET_REQ ** request,
	        DUL_ASSOCIATESERVICEPARAMETERS * params, IDB_HANDLE ** handle, DMAN_HANDLE ** manageHandle)
{
    MSG_C_GET_RESP    response;
    CGET_PARAMS       p;
    CONDITION         cond;

    memset(&response, 0, sizeof(response));
    response.type = MSG_K_C_GET_RESP;

    p.network = network;
    p.params = params;
    p.handle = handle;
    p.manageHandle = manageHandle;

    cond = SRV_CGetResponse(association, params, ctx, request, &response, cgetCallback, &p, "");
    return cond;
}

typedef struct {
    void 		*reserved[2];
    char 		classUID[DICOM_UI_LENGTH + 1];
}   UID_STRUCT;

/* cgetCallback
**
** Purpose:
**	Callback routine called by the C-GET Response handling routine.
**
** Parameter Dictionary:
**	request		Pointer to the C-GET request message
**	response	Pointer to the C-GET response message
**	responseCount	Total number of responses
**	SOPClass	Abstract Syntax for which GET has been requested
**	queryLevel	Database access query level
**	cgetParams	Parameters describing the cgete operation
**
** Return Values:
**	APP_FAILURE
**	SRV_NORMAL
**
** Notes:
**	We pass the cgetParams as void* and later cast to CGET_PARAMS*
**	to satisfy the prototype for a get callback as defined in
**	dicom_services.h
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
cgetCallback(MSG_C_GET_REQ * request, MSG_C_GET_RESP * response, MSG_C_STORE_REQ * storeRequest, MSG_C_STORE_RESP * storeResponse,
	         int responseCount, char *SOPClass, char *queryLevel, void *cgetParamsPtr) /* CGET_PARAMS * cgetParams) */

{
    CONDITION         					cond;
    static LST_HEAD   					*imageList = NULL;
    IDB_HANDLE        					**handle;
    static QUERY_MAP  					map[] = {{"PATIENT", IDB_PATIENT_LEVEL},
												 {"STUDY", IDB_STUDY_LEVEL},
												 {"SERIES", IDB_SERIES_LEVEL},
												 {"IMAGE", IDB_IMAGE_LEVEL},
												};
    int               					i, searchQueryEnd = 0;
    CTNBOOLEAN        					done;
    long              					selectCount;
    IDB_Query         					queryStructure;
    QUERY_LIST_ITEM   					* queryItem;
    static LST_HEAD   					* queryList = NULL, *failedList = NULL;
    static DUL_NETWORKKEY             	* network;
    DUL_ASSOCIATESERVICEPARAMETERS    	* params;
    DCM_ELEMENT       				  	e = {	DCM_IDFAILEDINSTANCEUIDLIST, DCM_UI, "", 1, 0, NULL };
    IDB_InstanceListElement           	* instance;
    MSG_STATUS_DESCRIPTION            	statusDescription;	/* to check the status returned by a StoreResponse */

    /*  The following definition and cast operation allow us to satisfy prototypes for get callbacks as defined in dicom_services.h. */
    CGET_PARAMS       					*cgetParams;

    cgetParams = (CGET_PARAMS *) cgetParamsPtr;

    network = *cgetParams->network;
    params = cgetParams->params;
    handle = cgetParams->handle;

    strcpy(response->classUID, request->classUID);
    response->conditionalFields = DCM_CMDDATANULL;
    response->dataSetType = DCM_CMDDATANULL;
    response->identifier = NULL;

    if (response->status == MSG_K_CANCEL) {
    	if (!silent) printf("CGet cancelled\n");
    	if (queryList != NULL) {
    		while ((queryItem = LST_Dequeue(&queryList)) != NULL){
		            free(queryItem);
    		}
    	}
    	response->conditionalFields |= MSG_K_C_GET_COMPLETED | MSG_K_C_GET_FAILED | MSG_K_C_GET_WARNING;
    	/* check if there is any failed UID list existing */
    	if (failedList != NULL) {
    		response->dataSetType = DCM_CMDDATAIDENTIFIER;
    		response->identifier = failedList;
    	}
    	response->conditionalFields |= MSG_K_C_GET_REMAINING;
    	return SRV_NORMAL;
    }
    /*  Check if there was a store response from the previous store request.
    **  If so, set various fields of the CGetResponse message.
    **  The first time this callback is invoked, store response will be
    **  NULL and response count will equal 0.
    */
    if (storeResponse != NULL) {
    	if (!silent) {
    		printf("Store Response received\n");
    		MSG_DumpMessage(storeResponse, stdout);
    	}

    	cond = MSG_StatusLookup(storeResponse->status, MSG_K_C_STORE_RESP, &statusDescription);
    	if (cond != MSG_NORMAL) {
    		fprintf(stderr, "Invalid status code in store response: %d\n", storeResponse->status);
    		return 0;		/* repair */
    	}

    	switch (statusDescription.statusClass){
			case MSG_K_CLASS_SUCCESS:
										response->completedSubOperations++;
										break;
			case MSG_K_CLASS_WARNING:
										response->warningSubOperations++;
										break;
			case MSG_K_CLASS_REFUSED:
			case MSG_K_CLASS_FAILURE:
										response->failedSubOperations++;
										break;
			default:
										fprintf(stderr, "Invalid status code in store response: %d\n", storeResponse->status);
										break;
    	}
        /*
        ** A status of pending does not contain a failed UID list. Also, a
        ** pending status must contain the number of remaining, completed,
        ** failed and warning sub operations
        */
	      response->dataSetType = DCM_CMDDATANULL;
	      response->identifier = NULL;
	      response->conditionalFields |= MSG_K_C_GET_REMAINING | MSG_K_C_GET_COMPLETED | MSG_K_C_GET_FAILED | MSG_K_C_GET_WARNING;
	      /* we send a PENDING response */
	      response->status = MSG_K_C_GET_SUBOPERATIONSCONTINUING;
    }
    if (imageList == NULL) {
    	imageList = LST_Create();
    	if (imageList == NULL) return 0;
    }
    if (!silent) {
    	printf("CGet callback\n");
    	printf("SOP Class:      %s\n", SOPClass);
    	printf("Query Level:    %s\n", queryLevel);
    	printf("Response Count: %d\n", responseCount);
    }
    if (responseCount == 0) {
    	if (!silent) (void) DCM_DumpElements(&request->identifier, 0);
#ifdef CTN_MULTIBYTE
    	cond = parseQueryMB(&request->identifier, &queryStructure);
#else
    	cond = parseQuery(&request->identifier, &queryStructure);
#endif
    	if (cond != APP_NORMAL) return 0;

    	for (i = 0, done = FALSE; !done && i < (int) DIM_OF(map); i++) {
    		if (strcmp(map[i].levelChar, queryLevel) == 0) {
    			searchQueryEnd = map[i].levelInt;
    			done = TRUE;
	        }
    	}

    	switch (searchQueryEnd) {
			case IDB_PATIENT_LEVEL:
	                               queryStructure.study.StuInsUID[0] = '\0';
	                               queryStructure.StudyQFlag = QF_STU_StuInsUID;
	                               queryStructure.StudyNullFlag = QF_STU_StuInsUID;

	                               queryStructure.series.SerInsUID[0] = '\0';
	                               queryStructure.SeriesQFlag = QF_SER_SerInsUID;
	                               queryStructure.SeriesNullFlag = QF_SER_SerInsUID;

	                               queryStructure.image.SOPInsUID[0] = '\0';
	                               queryStructure.ImageQFlag = QF_IMA_SOPInsUID;
	                               queryStructure.ImageNullFlag = QF_IMA_SOPInsUID;
	                               break;
			case IDB_STUDY_LEVEL:
	                               queryStructure.series.SerInsUID[0] = '\0';
	                               queryStructure.SeriesQFlag = QF_SER_SerInsUID;
	                               queryStructure.SeriesNullFlag = QF_SER_SerInsUID;

	                               queryStructure.image.SOPInsUID[0] = '\0';
	                               queryStructure.ImageQFlag = QF_IMA_SOPInsUID;
	                               queryStructure.ImageNullFlag = QF_IMA_SOPInsUID;
	                               break;
			case IDB_SERIES_LEVEL:
	                               queryStructure.image.SOPInsUID[0] = '\0';
	                               queryStructure.ImageQFlag = QF_IMA_SOPInsUID;
	                               queryStructure.ImageNullFlag = QF_IMA_SOPInsUID;
	                               break;
			case IDB_IMAGE_LEVEL:
	                               break;
    	}

    	if (strcmp(SOPClass, DICOM_SOPPATIENTQUERY_GET) == 0){
    		cond = IDB_Select(handle, PATIENT_ROOT, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else if (strcmp(SOPClass, DICOM_SOPPATIENTSTUDYQUERY_GET) == 0){
    		cond = IDB_Select(handle, PATIENTSTUDY_ONLY, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else if (strcmp(SOPClass, DICOM_SOPSTUDYQUERY_GET) == 0){
    		cond = IDB_Select(handle, STUDY_ROOT, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else{
    		cond = 0;
    	}

    	if ((cond != IDB_NORMAL) && (cond != IDB_NOMATCHES)) {
    		COND_DumpConditions();
    		return 0;
    	}

    	response->remainingSubOperations = LST_Count(&imageList);
    	if (!silent) printf("Total store requests: %d\n", response->remainingSubOperations);

    	failedList = LST_Create();
    	if (failedList == NULL) return 0;
    }
    if (imageList != NULL){
    	queryItem = LST_Dequeue(&imageList);
    }else{
    	queryItem = NULL;
    }
    if (queryItem != NULL) {
    	instance = LST_Head(&queryItem->query.image.InstanceList);
    	if (instance != NULL) {
    		storeRequest->dataSetType = DCM_CMDDATAOTHER;
    		strcpy(storeRequest->classUID, queryItem->query.image.SOPClaUID);
    		strcpy(storeRequest->instanceUID, queryItem->query.image.SOPInsUID);

    		if (!silent) printf("Image file name: %s\n", instance->Path);

    		if (strcmp(instance->Transfer, DICOM_TRANSFERLITTLEENDIAN) == 0){
    			cond = DCM_OpenFile(instance->Path,	DCM_ORDERLITTLEENDIAN | DCM_NOGROUPLENGTH, &storeRequest->dataSet);
    		}else{
    			cond = DCM_OpenFile(instance->Path,	DCM_PART10FILE | DCM_NOGROUPLENGTH, &storeRequest->dataSet);
    			(void) DCM_RemoveGroup(&storeRequest->dataSet, DCM_GROUPFILEMETA);
    		}

    		if (cond != DCM_NORMAL)	return 0;	/* repair */
    	}else{
    		return 0;		/* repair */
    	};
    	free(queryItem);	/* repair - memory leak */
    	response->remainingSubOperations--;

    	if (!silent) MSG_DumpMessage(response, stdout);
    }else{			/* No more requests remain */
    	storeRequest->dataSetType = DCM_CMDDATANULL;

    	if ((response->failedSubOperations == 0) && (response->warningSubOperations == 0)){
    		/* complete success */
    		response->status = MSG_K_SUCCESS;
    		response->dataSetType = DCM_CMDDATANULL;
    		response->identifier = NULL;
    		response->conditionalFields |= MSG_K_C_GET_COMPLETED | MSG_K_C_GET_FAILED | MSG_K_C_GET_WARNING;
    	}else if ((response->warningSubOperations == 0) && (response->completedSubOperations == 0)){
    		/* All sub operations failed */
    		response->status = MSG_K_C_GET_UNABLETOPROCESS;
    		response->dataSetType = DCM_CMDDATAIDENTIFIER;
    		response->identifier = failedList;
    		response->conditionalFields |= MSG_K_C_GET_COMPLETED | MSG_K_C_GET_FAILED | MSG_K_C_GET_WARNING;
    	}else{
    		/* Some sub operations may have failed or had warnings */
    		response->status = MSG_K_C_GET_COMPLETEWITHFAILURES;
    		response->dataSetType = DCM_CMDDATAIDENTIFIER;
    		response->identifier = failedList;
    		response->conditionalFields |= MSG_K_C_GET_COMPLETED | MSG_K_C_GET_FAILED | MSG_K_C_GET_WARNING;
    	}
    }

    return SRV_NORMAL;
}

static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst)
{
    QUERY_LIST_ITEM           	* item;
    IDB_InstanceListElement 	* e1, *e2;

    item = malloc(sizeof(*item));

    if (item == NULL) return 0;

    item->query = *queryResponse;

    if (queryResponse->image.InstanceList != NULL){
    	item->query.image.InstanceList = LST_Create();
    	if (item->query.image.InstanceList == NULL) return 0;
    	e1 = LST_Head(&queryResponse->image.InstanceList);
    	(void) LST_Position(&queryResponse->image.InstanceList, e1);

    	while (e1 != NULL) {
    		e2 = malloc(sizeof(*e2));
			*e2 = *e1;
	        (void) LST_Enqueue(&item->query.image.InstanceList, e2);
	        e1 = LST_Next(&queryResponse->image.InstanceList);
    	}
    }
    (void) LST_Enqueue(&lst, item);
    return IDB_NORMAL;
}
