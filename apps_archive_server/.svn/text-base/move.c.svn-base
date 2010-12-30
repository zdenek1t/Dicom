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
** Last Update:		$Author: smm $, $Date: 2002/12/13 15:30:24 $
** Source File:		$RCSfile: move.c,v $
** Revision:		$Revision: 1.12 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.12 $ $RCSfile: move.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/info_entity/dicom_ie.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"
#include "../dicom_lib/utility/utility.h"
#ifdef CTN_MULTIBYTE
#include "tblmb.h"
#include "idbmb.h"
#else
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/idb/idb.h"
#endif
#include "../dicom_lib/manage/manage.h"
#include "../dicom_lib/iap/iap.h"

#include "image_archive.h"
#include "move.h"

extern CTNBOOLEAN silent;

typedef struct {
    DUL_NETWORKKEY                  **network;
    DUL_ASSOCIATESERVICEPARAMETERS  *params;
    IDB_HANDLE                      **handle;
    DMAN_HANDLE                     **manageHandle;
}   MOVE_PARAMS;


typedef struct {
    char      	*levelChar;
    int       	levelInt;
}   QUERY_MAP;

typedef struct {
    void        *reserved[2];
    IDB_Query   query;
}   QUERY_LIST_ITEM;


typedef struct {
  char    		caller[40];
  char    		destination[40];
} SEND_CTX;


static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst);
static CONDITION
sendCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response, unsigned long transmitted, unsigned long total, SEND_CTX* ctx);
static CONDITION
moveCallback(MSG_C_MOVE_REQ * request, MSG_C_MOVE_RESP * response, int responseCount, char *SOPClass, char *queryLevel, void *moveParams); /* MOVE_PARAMS * moveParams); */
static void
findMoveClasses(LST_HEAD ** SOPClassList);

CONDITION
establishSendAssociation(DUL_NETWORKKEY ** networkKey, LST_HEAD * queryList, char *moveDestination, DMAN_HANDLE ** handle, DUL_ASSOCIATIONKEY ** sendAssociation, DUL_ASSOCIATESERVICEPARAMETERS * params);

/* moveRequest
**
** Purpose:
**	This function responds to a request to move an image.
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	message		Pointer to the MSG_C_MOVE_REQ message that was
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
moveRequest(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_MOVE_REQ ** request,
	        DUL_ASSOCIATESERVICEPARAMETERS * params, IDB_HANDLE ** handle, DMAN_HANDLE ** manageHandle)
{
    MSG_C_MOVE_RESP   	response = { MSG_K_C_MOVE_RESP, 0, 0, DCM_CMDDATANULL, 0, 0,
	                                   0, 0, 0,
	                                   NULL,		/* DCM_OBJECT for failed UID List (data set) */
	                                   ""			/* SOP Class UID */
                                   };
    MOVE_PARAMS       	p;
    CONDITION         	cond;
    void*               timeStamp;
    double              deltaTime = 0;

    p.network = network;
    p.params = params;
    p.handle = handle;
    p.manageHandle = manageHandle;
    timeStamp = UTL_GetTimeStamp();

    cond = SRV_CMoveResponse(association, ctx, request, &response, moveCallback, &p, "");

    deltaTime = UTL_DeltaTime(timeStamp);
    UTL_ReleaseTimeStamp(timeStamp);
    
    if (!silent) printf("C-Move delta time: %8.4f\n", deltaTime);
    return cond;
}

typedef struct {
    void 		*reserved[2];
    char 		classUID[DICOM_UI_LENGTH + 1];
}   UID_STRUCT;

/* moveCallback
**
** Purpose:
**	Callback routine called by the C-MOVE Response handling routine.
**
** Parameter Dictionary:
**	request		Pointer to the C-MOVE request message
**	response	Pointer to the C-MOVE response message
**	responseCount	Total number of responses
**	SOPClass	Abstract Syntax for which MOVE has been requested
**	queryLevel	Database access query level
**	moveParams	Parameters describing the move operation
**
** Return Values:
**	APP_FAILURE
**	SRV_NORMAL
**
** Notes:
**	We pass moveParams as a void* and cast it later to MOVE_PARAMS*
**	to satisfy prototypes for move callbacks as defined in dicom_services.h
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
moveCallback(MSG_C_MOVE_REQ * request, MSG_C_MOVE_RESP * response, int responseCount, char *SOPClass, char *queryLevel, void *moveParamsPtr) /* MOVE_PARAMS * moveParams) */

{
    CONDITION          		cond;
    static LST_HEAD    		* imageList = NULL;
    IDB_HANDLE         		** handle;
    int                		i, searchQueryEnd = 0;
    CTNBOOLEAN	       		done;
    long               		selectCount;
    IDB_Query 	       		queryStructure;
    QUERY_LIST_ITEM 	 	* queryItem;
    MSG_UID_ITEM    	 	* UIDItem;
    static LST_HEAD    		* queryList = NULL, *failedList = NULL;
    static int         		transmitCount = 0;
    void*              		timeStamp;
    double             		deltaTime = 0;
    IDB_InstanceListElement	     * instance;
    static DUL_NETWORKKEY        * network;
    static DUL_ASSOCIATIONKEY    * sendAssociation = NULL;
    DUL_ASSOCIATESERVICEPARAMETERS	         * params;
    static DUL_ASSOCIATESERVICEPARAMETERS    sendParams;    
    static QUERY_MAP   map[] = {{"PATIENT", IDB_PATIENT_LEVEL},
                                {"STUDY", IDB_STUDY_LEVEL},
                                {"SERIES", IDB_SERIES_LEVEL},
                                {"IMAGE", IDB_IMAGE_LEVEL},
                               };
    DCM_ELEMENT     	e = { DCM_IDFAILEDINSTANCEUIDLIST, DCM_UI, "", 1, 0, NULL };

    /*  The definition and cast operation help satisfy prototypes for move
    **  callbacks as defined in dicom_services.h
    */
    MOVE_PARAMS     *moveParams;
    moveParams = (MOVE_PARAMS *) moveParamsPtr;

    network = *moveParams->network;
    params = moveParams->params;
    handle = moveParams->handle;

    response->conditionalFields = 0;
    response->dataSetType = DCM_CMDDATANULL;
    
    if (response->status == MSG_K_CANCEL) {
    	if (!silent) printf("Move cancelled\n");
    	if (queryList != NULL) {
    		while ((queryItem = LST_Dequeue(&queryList)) != NULL){
    			free(queryItem);
    		}
    	}
    }else{
    	response->status = MSG_K_C_MOVE_SUBOPERATIONSCONTINUING;
    }
    if (imageList == NULL) {
    	imageList = LST_Create();
    	if (imageList == NULL) return 0;
    }
    if (!silent) {
    	printf("Move callback\n");
    	printf("SOP Class:      %s\n", SOPClass);
    	printf("Query Level:    %s\n", queryLevel);
    	printf("Response Count: %d\n", responseCount);
    }
    if (response->status == MSG_K_CANCEL) {
    	if (!silent) printf("Query cancelled\n");
    	return SRV_NORMAL;
    }
    if (responseCount == 1) {
    	if (!silent) (void) DCM_DumpElements(&request->identifier, 0);
#ifdef CTN_MULTIBYTE
    	cond = parseQueryMB(&request->identifier, &queryStructure);
#else
    	cond = parseQuery(&request->identifier, &queryStructure);
#endif
    	if (cond != APP_NORMAL) return 0;

    	for (i = 0, done = FALSE; !done && i < (int) DIM_OF(map); i++) {
    		if (strcmp(map[i].levelChar, queryLevel) == 0){
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

    	timeStamp = UTL_GetTimeStamp();

    	if (strcmp(SOPClass, DICOM_SOPPATIENTQUERY_MOVE) == 0){
    		cond = IDB_Select(handle, PATIENT_ROOT, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else if (strcmp(SOPClass, DICOM_SOPPATIENTSTUDYQUERY_MOVE) == 0){
    		cond = IDB_Select(handle, PATIENTSTUDY_ONLY, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else if (strcmp(SOPClass, DICOM_SOPSTUDYQUERY_MOVE) == 0){
    		cond = IDB_Select(handle, STUDY_ROOT, IDB_PATIENT_LEVEL, IDB_IMAGE_LEVEL, &queryStructure, &selectCount, selectCallback, imageList);
    	}else{
    		cond = 0;
    	}
	
    	deltaTime = UTL_DeltaTime(timeStamp);
    	UTL_ReleaseTimeStamp(timeStamp);
	
    	if (!silent) printf(" C-Move delta time for DB query: %8.4f\n", deltaTime);

       	if ((cond != IDB_NORMAL) && (cond != IDB_NOMATCHES)) {
       		COND_DumpConditions();
       		return 0;
       	}
	
        response->remainingSubOperations = LST_Count(&imageList);
	      
        DUL_DefaultServiceParameters(&sendParams);
	      
        strcpy(sendParams.callingAPTitle, params->calledAPTitle);
        strcpy(sendParams.calledAPTitle, request->moveDestination);
        timeStamp = UTL_GetTimeStamp();
	
        cond = establishSendAssociation(&network, imageList, request->moveDestination, moveParams->manageHandle, &sendAssociation, &sendParams);

      	deltaTime = UTL_DeltaTime(timeStamp);
      	UTL_ReleaseTimeStamp(timeStamp);

      	if (!silent) printf(" C-Move, time to create new association: %8.4f\n", deltaTime);

      	if (cond != APP_NORMAL) {
      		sendAssociation = NULL;
      	}else{
      		transmitCount = 0;
      		timeStamp = UTL_GetTimeStamp();
	    
            queueTransmitAssociation(params->callingAPTitle, request->moveDestination);
	  
            deltaTime = UTL_DeltaTime(timeStamp);
            UTL_ReleaseTimeStamp(timeStamp);
	  
            if (!silent) printf(" C-Move, time to queue Transmit Association operation: %8.4f\n", deltaTime);
      	}
	
        failedList = LST_Create();
        if (failedList == NULL) return 0;
    }
    
    if (imageList != NULL){
    	queryItem = LST_Dequeue(&imageList);
    }else{
    	queryItem = NULL;
    }
    
    if (queryItem != NULL) {
    	if (queryItem->query.image.InstanceList == NULL) {
    		cond = APP_FAILURE;
    		instance = NULL;
    	}else{
    		instance = LST_Head(&queryItem->query.image.InstanceList);
    	}

    	if (instance != NULL && sendAssociation != NULL) {
    		SEND_CTX   sendCtx;
/* #ifdef VERBOSE */
    		if (!silent) printf("Sending image (%s) to %s\n", instance->Path, sendParams.calledPresentationAddress);
/* #endif */

    		transmitCount++;
    		timeStamp = UTL_GetTimeStamp();
	    
            queueTransmitImage(params->callingAPTitle, request->moveDestination, 0);
            queueStartTransmit(params->callingAPTitle, request->moveDestination, transmitCount);
	    
            deltaTime = UTL_DeltaTime(timeStamp);
            UTL_ReleaseTimeStamp(timeStamp);
	    
            if (!silent) printf(" C-Move, time to queue Transmit Image operation: %8.4f\n", deltaTime);

            strcpy(sendCtx.caller, params->callingAPTitle);
            strcpy(sendCtx.destination, request->moveDestination);

            timeStamp = UTL_GetTimeStamp();

            if (strcmp(instance->Transfer, DICOM_TRANSFERLITTLEENDIAN) == 0) {
            	cond = IAP_SendImage(&sendAssociation, &sendParams, instance->Path, params->callingAPTitle, request->messageID, sendCallback, &sendCtx);
            }else{
            	cond = IAP_SendInfoObject(&sendAssociation, &sendParams, instance->Path, instance->Transfer, params->callingAPTitle, request->messageID, sendCallback, &sendCtx);
            }

            deltaTime = UTL_DeltaTime(timeStamp);
            UTL_ReleaseTimeStamp(timeStamp);
	    
            if (!silent) printf(" C-Move, time to send one instance: %8.4f\n", deltaTime);
	
    	}else{
    		cond = APP_FAILURE;
        }
	
        if (CTN_SUCCESS(cond)){
        	response->completedSubOperations++;
        }else if (CTN_WARNING(cond)){
/* #ifdef VERBOSE */
        	if (!silent) printf("Received a warning after send: %lx\n", cond);
/* #endif */
        	if (!silent) COND_DumpConditions();
        	response->warningSubOperations++;
        }else{
/* #ifdef VERBOSE */
        	if (!silent) printf("Received an error after send: %lx\n", cond);
/* #endif */
        	if (!silent) COND_DumpConditions();
        	response->failedSubOperations++;

        	UIDItem = malloc(sizeof(*UIDItem));
        	if (UIDItem == NULL) return 0;

        	(void) strcpy(UIDItem->UID, queryItem->query.image.SOPInsUID);
        	cond = LST_Enqueue(&failedList, UIDItem);
        }
        response->remainingSubOperations--;
        response->conditionalFields |= MSG_K_C_MOVE_REMAINING | MSG_K_C_MOVE_COMPLETED |  MSG_K_C_MOVE_FAILED | MSG_K_C_MOVE_WARNING;
    }else{
    	if (response->status == MSG_K_CANCEL);

    	else if (response->failedSubOperations == 0)
    		response->status = MSG_K_SUCCESS;
    	else
    		response->status = MSG_K_C_MOVE_COMPLETEWITHFAILURES;

    	if (response->status != MSG_K_CANCEL) {
    		response->dataSetType = DCM_CMDDATANULL;
    		if (LST_Count(&failedList) != 0) {
    			response->dataSetType = DCM_CMDDATAOTHER;
    			cond = DCM_AddElementList(&response->dataSet, &e, failedList, STRUCT_OFFSET(MSG_UID_ITEM, UID));
    		}
    		response->conditionalFields |= MSG_K_C_MOVE_REMAINING | MSG_K_C_MOVE_COMPLETED | MSG_K_C_MOVE_FAILED | MSG_K_C_MOVE_WARNING;
    	}else{
    		while ((UIDItem = LST_Dequeue(&failedList)) != NULL){
    			free(UIDItem);
    		}
    	}

    	if (sendAssociation != NULL) {
    		(void) DUL_ReleaseAssociation(&sendAssociation);
    		queueClosedTransmitAssociation(params->callingAPTitle, request->moveDestination, transmitCount);
    	}
    }
    return SRV_NORMAL;
}

static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst)
{
    QUERY_LIST_ITEM         	* item;
    IDB_InstanceListElement 	* e1, *e2;

    item = malloc(sizeof(*item));

    if (item == NULL) return 0;

    item->query = *queryResponse;
    
    if (queryResponse->image.InstanceList != NULL) {
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

/* sendCallback
**
** Purpose:
**	Callback routine for the C-SEND Response primitive
**
** Parameter Dictionary:
**	request		Pointer to request message
**	response	Pointer to response message
**	transmitted	Number of bytes sent
**	total		Total number of bytes to be sent
**	string		Context Information
**
** Return Values:
**	SRV_NORMAL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
sendCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response, unsigned long transmitted, unsigned long total, SEND_CTX* ctx)
{
  int     percentage = 0;

  if (transmitted == 0){
	    if (!silent) printf("Initial call to sendCallback\n");
  }
  if (!silent) printf("%8ld bytes transmitted of %8ld (%s)\n", transmitted, total,ctx->destination);

  percentage = (100 * transmitted) / total;
  if (percentage > 100) percentage = 100;

  queueTransmitImage(ctx->caller, ctx->destination, percentage);

  if (response != NULL) {
	    if (!silent) MSG_DumpMessage(response, stdout);
  }
  return SRV_NORMAL;
}

/* establishSendAssociation
**
** Purpose:
**	Request for the specific service class and then establish an
**	Association
**
** Parameter Dictionary:
**	networkKey		Key describing the network connection
**	queryList		Handle to list of queries
**	moveDestination		Name of destination where images are to be moved
**	sendAssociation		Key describing the Association
**	params			Service parameters describing the Association
**
** Return Values:
**	DUL_NORMAL	on success
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
establishSendAssociation(DUL_NETWORKKEY ** networkKey, LST_HEAD * queryList, char *moveDestination,
			             DMAN_HANDLE ** handle, DUL_ASSOCIATIONKEY ** sendAssociation, DUL_ASSOCIATESERVICEPARAMETERS * params)
{
  QUERY_LIST_ITEM          	* q;
  CONDITION                	cond;
  DMAN_APPLICATIONENTITY   	ae, criteria;
  long                     	count;
  IDB_InstanceListElement* 	instance;

  if (queryList == NULL) return 0;

  memset(&criteria, 0, sizeof(criteria));
  criteria.Type = ae.Type = DMAN_K_APPLICATIONENTITY;
  criteria.Flag = DMAN_K_APPLICATION_TITLE;
  strcpy(criteria.Title, moveDestination);

  cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & ae, (DMAN_GENERICRECORD *) & criteria, NULL, NULL, &count, NULL);
  if (cond != DMAN_NORMAL) return 0;

  if (count != 1) return 0;

  logMessage ("Move: %s %s %s %d\n", params->callingAPTitle, params->calledAPTitle, ae.Node, ae.Port);

  sprintf(params->calledPresentationAddress, "%s:%-d", ae.Node, ae.Port);

  q = LST_Head(&queryList);
  if (q == NULL) return 0;
  (void) LST_Position(&queryList, q);

  while (q != NULL) {
	  if (q->query.image.InstanceList == NULL) return 0;
	  instance = LST_Head(&q->query.image.InstanceList);

	  cond = SRV_RegisterSOPClassXfer(q->query.image.SOPClaUID,	instance->Transfer, DUL_SC_ROLE_DEFAULT, params);
	  if (CTN_INFORM(cond)){
		  (void) COND_PopCondition(FALSE);
	  }else if (cond != SRV_NORMAL){
		  return 0;		/* repair */
	  }

	  q = LST_Next(&queryList);
  }
  
  cond = DUL_RequestAssociation(networkKey, params, sendAssociation);
  if (cond != DUL_NORMAL) {
      printf("Could not establish Association with %s %s %s\n",
	           params->callingAPTitle,
	           params->calledAPTitle,
	           params->calledPresentationAddress);
      COND_DumpConditions();
      return 0;		/* repair */
  }
  return APP_NORMAL;
}

#if 0
CONDITION
establishSendAssociation(DUL_NETWORKKEY ** networkKey, LST_HEAD * queryList, char *moveDestination, DMAN_HANDLE ** handle,
			             DUL_ASSOCIATIONKEY ** sendAssociation, DUL_ASSOCIATESERVICEPARAMETERS * params)
{
    QUERY_LIST_ITEM   		    * q;
    CONDITION               	cond;
    DMAN_APPLICATIONENTITY  	ae, criteria;
    long                    	count;

    if (queryList == NULL) return 0;
    
    memset(&criteria, 0, sizeof(criteria));
    criteria.Type = ae.Type = DMAN_K_APPLICATIONENTITY;
    criteria.Flag = DMAN_K_APPLICATION_TITLE;
    strcpy(criteria.Title, moveDestination);
    
    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & ae, (DMAN_GENERICRECORD *) & criteria, NULL, NULL, &count, NULL);
    
    if (cond != DMAN_NORMAL) return 0;
    if (count != 1) return 0;

    printf("Move: %s %s %s %d\n", params->callingAPTitle, params->calledAPTitle, ae.Node, ae.Port);

    sprintf(params->calledPresentationAddress, "%s:%-d", ae.Node, ae.Port);

    q = LST_Head(&queryList);
    
    if (q == NULL) return 0;
    
    (void) LST_Position(&queryList, q);
    while (q != NULL) {
    	cond = SRV_RequestServiceClass(q->query.image.SOPClaUID, DUL_SC_ROLE_DEFAULT, params);
    	if (CTN_INFORM(cond)){
    		(void) COND_PopCondition(FALSE);
    	}else if (cond != SRV_NORMAL){
    		return 0;		/* repair */
    	}
    	q = LST_Next(&queryList);
    }

    cond = DUL_RequestAssociation(networkKey, params, sendAssociation);    
    if (cond != DUL_NORMAL) {
    	printf("Could not establish Association with %s %s %s\n",
	            params->callingAPTitle,
	            params->calledAPTitle,
	            params->calledPresentationAddress);
	    COND_DumpConditions();
	    return 0;		/* repair */
    }
    return APP_NORMAL;
}
#endif
