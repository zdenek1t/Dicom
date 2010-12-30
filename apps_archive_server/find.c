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
          WASHINGTON UNIVERSITY MAKE ANY WARRANTY ACTNBOUT THE SOFTWARE, ITS
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
** Last Update:		$Author: smm $, $Date: 2002/11/27 15:20:54 $
** Source File:		$RCSfile: find.c,v $
** Revision:		$Revision: 1.7 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.7 $ $RCSfile: find.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
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
#include "../dicom_lib/manage/manage.h"
#ifdef CTN_MULTIBYTE
#include "tblmb.h"
#include "idbmb.h"
#else
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/idb/idb.h"
#endif

#include "image_archive.h"
#include "find.h"

static CTNBOOLEAN waitFlag = FALSE;		/* A debugging flag, we usually set it with dbx */
extern CTNBOOLEAN silent;

typedef struct {
    DUL_ASSOCIATESERVICEPARAMETERS 	*params;
    IDB_HANDLE 						**handle;
}   FIND_PARAMS;

typedef struct {
    char 		*levelChar;
    int 		levelInt;
}   QUERY_MAP;

typedef struct {
    void 		*reserved[2];
    IDB_Query 	query;
}   QUERY_LIST_ITEM;

static CONDITION
findCallback(MSG_C_FIND_REQ * request, MSG_C_FIND_RESP * response, int responseCount, char *SOPClass, char *queryLevel, void *findParamsPtr); /*FIND_PARAMS * findParams); */

static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst);

/* findRequest
**
** Purpose:
**	This function responds to a query request
**
** Parameter Dictionary:
**	association	They key which is used to access the association
**			on which requests are received.
**	ctx		Pointer to the presentation context for this command
**	message		Pointer to the MSG_C_FIND_REQ message that was
**			received by the server.
**
** Return Values:
**
**	SRV_ILLEGALPARAMETER
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
findRequest(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DUL_PRESENTATIONCONTEXT * ctx, MSG_C_FIND_REQ ** request, IDB_HANDLE ** handle)
{
    MSG_C_FIND_RESP   response = {MSG_K_C_FIND_RESP, 0, 0, 0, 0, NULL, ""};
    FIND_PARAMS       findParams;

    findParams.params = params;
    findParams.handle = handle;

    return SRV_CFindResponse(association, ctx, request, &response, findCallback, &findParams, "");
}

/* findCallback
**
** Purpose:
**	Callback routine called by SRV C-FIND Response handling routine
**
** Parameter Dictionary:
**	request		Pointer to C-FIND request message
**	response	Pointer to C-FIND response message
**	responseCount	Total number of responses
**	SOPClass	Abstract Syntax (to be found)
**	queryLevel	Database access level
**	findParams	Pointer to a structure with context information
**			that we use during query operation
**
** Return Values:
**	SRV_NORMAL
**
** Notes:
**	We pass findParams as a void* to satisfy the prototype found in
**	dicom_services.h.  We will cast it below to be a FIND_PARAMS*.
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
findCallback(MSG_C_FIND_REQ * request, MSG_C_FIND_RESP * response, int responseCount, char *SOPClass, char *queryLevel, void *findParamsPtr) /*FIND_PARAMS * findParams) */
{
    CONDITION         					cond;
    static LST_HEAD   					* lst = NULL;
    DUL_ASSOCIATESERVICEPARAMETERS    	* params;
    IDB_HANDLE        					** handle;
    static QUERY_MAP  map[] = {{"PATIENT", IDB_PATIENT_LEVEL},
                               {"STUDY", IDB_STUDY_LEVEL},
                               {"SERIES", IDB_SERIES_LEVEL},
                               {"IMAGE", IDB_IMAGE_LEVEL},
                              };
    int               i, searchQueryEnd = 0;
    CTNBOOLEAN        done;
    long              selectCount;
    DCM_ELEMENT       titleElement = {DCM_IDRETRIEVEAETITLE, DCM_AE, "", 1, 0, NULL};
    IDB_Query         queryStructure;
    QUERY_LIST_ITEM   * queryItem;
    /*  This definition and assignment help satisfy prototypes for this callback
    **  function as defined in dicom_services.h
    */
    FIND_PARAMS       *findParams;
    
    findParams = (FIND_PARAMS *) findParamsPtr;

    if (lst == NULL) {
    	lst = LST_Create();
    	if (lst == NULL) return 0;
    }
    params = findParams->params;
    handle = findParams->handle;

    if (!silent) {
    	printf("==================== Find callback ====================\n");
    	printf("SOP Class:      %s\n", SOPClass);
    	printf("Query Level:    %s\n", queryLevel);
    	printf("Response Count: %d\n", responseCount);
    	printf("-------------------------------------------------------\n");    }
    
    if (response->status == MSG_K_CANCEL) {
    	if (!silent) printf("Query cancelled\n");
    	return SRV_NORMAL;
    }
    if (responseCount == 1){
    	if (!silent){
    		printf(">> DUMP Query:\n");
    		(void) DCM_DumpElements(&request->identifier, 0);
    	}
#ifdef CTN_MULTIBYTE
    	cond = parseQueryMB(&request->identifier, &queryStructure);
#else
    	cond = parseQuery(&request->identifier, &queryStructure);
#endif
    	if (cond != APP_NORMAL) {
    		response->status = MSG_K_C_FIND_UNABLETOPROCESS;
    		return 0;
    	}

    	for (i = 0, done = FALSE; !done && i < (int) DIM_OF(map); i++) {
    		if (strcmp(map[i].levelChar, queryLevel) == 0){
    			searchQueryEnd = map[i].levelInt;
    			done = TRUE;
	        }
    	}
#ifdef CTN_MULTIBYTE
    	cond = verifyQueryMB(&queryStructure, SOPClass, searchQueryEnd);
#else
    	cond = verifyQuery(&queryStructure, SOPClass, searchQueryEnd);
#endif
    	if (cond != APP_NORMAL) {
    		response->status = MSG_K_C_FIND_UNABLETOPROCESS;
    		strcpy(response->errorComment, "Query was formatted improperly.  Check required keys");
    		response->conditionalFields |= MSG_K_C_FINDRESP_ERRORCOMMENT;
    		return 0;
    	}
    	if (strcmp(SOPClass, DICOM_SOPPATIENTQUERY_FIND) == 0){
    		cond = IDB_Select(handle, PATIENT_ROOT, IDB_PATIENT_LEVEL, searchQueryEnd, &queryStructure, &selectCount, selectCallback, lst);
    	}else if (strcmp(SOPClass, DICOM_SOPPATIENTSTUDYQUERY_FIND) == 0){
    		cond = IDB_Select(handle, PATIENTSTUDY_ONLY, IDB_PATIENT_LEVEL, searchQueryEnd, &queryStructure, &selectCount, selectCallback, lst);
    	}else if (strcmp(SOPClass, DICOM_SOPSTUDYQUERY_FIND) == 0){
    		cond = IDB_Select(handle, STUDY_ROOT, IDB_PATIENT_LEVEL, searchQueryEnd, &queryStructure, &selectCount, selectCallback, lst);
    	}else{
    		cond = 0;
    	}
	    if ((cond != IDB_NORMAL) && (cond != IDB_NOMATCHES)) {
	    	COND_DumpConditions();
	    	return 0;
	    }

	    if (!silent) {
	    	int responseCount = 0;
	    	responseCount = LST_Count(&lst);
	    	printf("Total number of matching responses: %d\n", responseCount);
	    }
    }
    response->status = MSG_K_C_FIND_MATCHCONTINUING;
    response->dataSetType = DCM_CMDDATAIDENTIFIER;
    queryItem = LST_Dequeue(&lst);

    if (queryItem == NULL) {
    	response->status = MSG_K_SUCCESS;
    	response->dataSetType = DCM_CMDDATANULL;
    }else{
        #ifdef CTN_MULTIBYTE
    		cond = buildQueryMB(&response->identifier, &queryItem->query);
    	#else
    		cond = buildQuery(&response->identifier, &queryItem->query);
    	#endif
    	if (!CTN_SUCCESS(cond)){
    		COND_DumpConditions();
    		(void) DCM_DumpElements(&response->identifier, 0);
    	}else{
    		if (!silent) (void) DCM_DumpElements(&response->identifier, 0);
    	}

    	(void) free(queryItem);
    	titleElement.d.string = params->calledAPTitle;
    	titleElement.length = strlen(titleElement.d.string);
	
        cond = DCM_ModifyElements(&response->identifier, &titleElement, 1, NULL, 0, NULL);
	    if (!CTN_SUCCESS(cond)) {
	    	COND_DumpConditions();
	    	(void) DCM_DumpElements(&response->identifier, 0);
	    }
    }
    if (waitFlag) {
		#ifdef _MSC_VER
    	    	Sleep(1000);
		#else
    	    	sleep(1);
		#endif
    }
    return SRV_NORMAL;
}

static CONDITION
selectCallback(IDB_Query * queryResponse, long count, LST_HEAD * lst)
{
    QUERY_LIST_ITEM    	* item;

    item = malloc(sizeof(*item));
    if (item == NULL) return 0;

    item->query = *queryResponse;
    (void) LST_Enqueue(&lst, item);
    return IDB_NORMAL;
}
