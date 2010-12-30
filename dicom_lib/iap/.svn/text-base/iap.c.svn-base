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
**			IAP_SendImage
** Author, Date:	Stephen M. Moore, John T. O'Neill, 16-Jun-93
** Intent:
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:04 $
** Source File:		$RCSfile: iap.c,v $
** Revision:		$Revision: 1.32 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.32 $ $RCSfile: iap.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#ifdef MACOS
#include <files.h>
#elif defined _MSC_VER
#include <io.h>
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#ifdef MACOS
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#endif

#include "../dicom/dicom.h"
#include "../uid/dicom_uids.h"
#include "../lst/lst.h"
#include "../condition/condition.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"
#include "../database/dbquery.h"
#include "../iap/iap.h"

static CONDITION parseImageInstanceUID(DCM_OBJECT ** object, int *elementCount);

/* IAP_SendImage
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**	IAP_NORMAL
**	IAP_OBJECTACCESSFAILED
**	IAP_SENDFAILED
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
IAP_SendImage(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *fileName, char *moveAETitle, unsigned short moveMessageID,
	      CONDITION(*sendCallback) (MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response,  unsigned long transmitted, unsigned long total, char *string), void *callbackCtx)
{
    static MSG_C_STORE_REQ      request;
    CONDITION					cond;
    DCM_OBJECT					* object;
    static DCM_ELEMENT 			list[] = {
											{DCM_IDSOPCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) &request.classUID[0]},
											{DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) &request.instanceUID[0]},
								};
    U32							l;
    void				        *ctx;
    int						    index;

    if (fileName == NULL) return IAP_NORMAL;
    if (fileName[0] == '\0') return IAP_NORMAL;

    cond = DCM_OpenFile(fileName, DCM_ORDERLITTLEENDIAN | DCM_NOGROUPLENGTH, &object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_SendImage");

    for (index = 0; index < (int) DIM_OF(list); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(&object, &list[index], &l, &ctx);
    	if (cond != DCM_NORMAL) {
    		(void) DCM_CloseObject(&object);
    		return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_SendImage");
    	}
    	list[index].d.string[l] = '\0';
    }

    request.type = MSG_K_C_STORE_REQ;
    request.messageID = SRV_MessageIDOut();
    request.priority = DCM_PRIORITYMEDIUM;
    request.dataSetType = DCM_CMDDATAIMAGE;
    request.dataSet = object;
    request.fileName = NULL;
    request.conditionalFields = 0;

    if (moveAETitle != NULL) {
    	strcpy(request.moveAETitle, moveAETitle);
    	request.moveMessageID = moveMessageID;
    	request.conditionalFields |= MSG_K_C_STORE_MOVEMESSAGEID | MSG_K_C_STORE_MOVEAETITLE;
    }else{
    	request.moveAETitle[0] = '\0';
    }

    cond = SRV_CStoreRequest(association, params, &request, NULL, sendCallback, callbackCtx, "");

    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(IAP_SENDFAILED, IAP_Message(IAP_SENDFAILED), "IAP_SendImage");

    return IAP_NORMAL;
}


CONDITION
IAP_SendInfoObject(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, const char *fileName, const char *fileXferSyntax,
				   const char *moveAETitle, unsigned short moveMessageID, CONDITION(*sendCallback) (), void *callbackCtx)
{
    static MSG_C_STORE_REQ      request;
    CONDITION					cond;
    DCM_OBJECT					* object;
    static DCM_ELEMENT 			list[] = {
									{DCM_IDSOPCLASSUID, DCM_UI, "", 1, sizeof(request.classUID), (void *) &request.classUID[0]},
									{DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, sizeof(request.instanceUID), (void *) &request.instanceUID[0]},
								};
    U32 						l;
    void 						*ctx;
    int 						index;

    if (fileName == NULL) return IAP_NORMAL;
    if (fileName[0] == '\0') return IAP_NORMAL;

    cond = DCM_OpenFile(fileName, DCM_PART10FILE, &object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_SendInfoObject");

    for (index = 0; index < (int) DIM_OF(list); index++) {
    	ctx = NULL;
    	cond = DCM_GetElementValue(&object, &list[index], &l, &ctx);
    	if (cond != DCM_NORMAL) {
    		(void) DCM_CloseObject(&object);
    		return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_SendInfoObject");
    	}
    	list[index].d.string[l] = '\0';
    }
    (void) DCM_RemoveGroup(&object, DCM_GROUPFILEMETA);

    request.type = MSG_K_C_STORE_REQ;
    request.messageID = SRV_MessageIDOut();
    request.priority = DCM_PRIORITYMEDIUM;
    request.dataSetType = DCM_CMDDATAIMAGE;
    request.dataSet = object;
    request.fileName = NULL;
    request.conditionalFields = 0;

    if (moveAETitle != NULL) {
    	strcpy(request.moveAETitle, moveAETitle);
    	request.moveMessageID = moveMessageID;
    	request.conditionalFields |= MSG_K_C_STORE_MOVEMESSAGEID | MSG_K_C_STORE_MOVEAETITLE;
    }else{
    	request.moveAETitle[0] = '\0';
    }
    cond = SRV_CStoreRequest(association, params, &request, NULL, sendCallback, callbackCtx, "");

    (void) DCM_CloseObject(&object);
    if (cond != SRV_NORMAL)	return COND_PushCondition(IAP_SENDFAILED, IAP_Message(IAP_SENDFAILED), "IAP_SendInfoObject");

    return IAP_NORMAL;
}


typedef struct {
    char *queryLevel;
    long flag;
}   QUERY_MAP;

static QUERY_MAP levelMap[] = {
    {DCM_QUERYLEVELPATIENT, DB_K_LEVELPAT},
    {DCM_QUERYLEVELSTUDY, DB_K_LEVELSTUDY},
    {DCM_QUERYLEVELSERIES, DB_K_LEVELSERIES},
    {DCM_QUERYLEVELIMAGE, DB_K_LEVELIMAGE}
};

typedef struct {
    char *SOPClass;
    long flag;
}   CLASS_MAP;

static CLASS_MAP classMap[] = {
    {DICOM_SOPPATIENTQUERY_FIND, DB_K_CLASSPAT},
    {DICOM_SOPSTUDYQUERY_FIND, DB_K_CLASSSTUDY},
    {DICOM_SOPPATIENTSTUDYQUERY_FIND, DB_K_CLASSPATSTUDY}
};

static Query q;
static DCM_FLAGGED_ELEMENT list[] = {
    {DCM_PATBIRTHDATE, DCM_DA, "", 1, sizeof(q.Patient.BirthDate), (void *) &q.Patient.BirthDate[0], DB_K_QBIRTHDATE, &q.Patient.Query_Flag},
    {DCM_PATNAME, DCM_PN, "", 1, sizeof(q.Patient.Name), (void *) &q.Patient.Name[0], DB_K_QNAME, &q.Patient.Query_Flag},
    {DCM_PATID, DCM_LO, "", 1, sizeof(q.Patient.PatID), (void *) &q.Patient.PatID[0], DB_K_QID, &q.Patient.Query_Flag},

    {DCM_IDSTUDYDATE, DCM_DA, "", 1, sizeof(q.Study.StudyDate), (void *) &q.Study.StudyDate[0], DB_K_QSTUDYDATE, &q.Study.Query_Flag},
    {DCM_IDSTUDYTIME, DCM_TM, "", 1, sizeof(q.Study.StudyTime), (void *) &q.Study.StudyTime[0], DB_K_QSTUDYTIME, &q.Study.Query_Flag},
#if STANDARD_VERSION >= VERSION_AUG1993
    {DCM_RELSTUDYID, DCM_SH, "", 1, sizeof(q.Study.StudyID), (void *) &q.Study.StudyID[0], DB_K_QSTUDYID, &q.Study.Query_Flag},
#else
    {DCM_RELSTUDYID, DCM_CS, "", 1,
	sizeof(q.Study.StudyID), (void *) &q.Study.StudyID[0],
    DB_K_QSTUDYID, &q.Study.Query_Flag},
#endif
#if STANDARD_VERSION >= VERSION_AUG1993
    {DCM_IDACCESSIONNUMBER, DCM_SH, "", 1, sizeof(q.Study.AccessionNumber),	(void *) &q.Study.AccessionNumber[0], DB_K_QACCESSIONNUMBER, &q.Study.Query_Flag},
#else
    {DCM_IDACCESSIONNUMBER, DCM_CS, "", 1,
	sizeof(q.Study.AccessionNumber),
	(void *) &q.Study.AccessionNumber[0],
    DB_K_QACCESSIONNUMBER, &q.Study.Query_Flag},
#endif
    {DCM_RELSTUDYINSTANCEUID, DCM_UI, "", 1, sizeof(q.Study.StudyUID), (void *) &q.Study.StudyUID[0], DB_K_QSTUDYUID, &q.Study.Query_Flag},
    {DCM_IDMODALITY, DCM_CS, "", 1,	sizeof(q.Series.Modality), (void *) &q.Series.Modality[0], DB_K_QMODALITY, &q.Series.Query_Flag},
    {DCM_RELSERIESNUMBER, DCM_IS, "", 1, sizeof(q.Series.SeriesNumber), (void *) &q.Series.SeriesNumber[0], DB_K_QSERIESNUMBER, &q.Series.Query_Flag},
    {DCM_RELSERIESINSTANCEUID, DCM_UI, "", 1, sizeof(q.Series.SeriesUID), (void *) &q.Series.SeriesUID[0], DB_K_QSERIESUID, &q.Series.Query_Flag},

    {DCM_RELIMAGENUMBER, DCM_IS, "", 1,	sizeof(q.Image.ImageNumber), (void *) &q.Image.ImageNumber[0], DB_K_QIMAGENUMBER, &q.Image.Query_Flag},
/*    {DCM_IDSOPINSTANCEUID, DCM_UI, "", 1,	sizeof(q.Image.ImageUID), (void *) &q.Image.ImageUID[0], DB_K_QIMAGEUID, &q.Image.Query_Flag}, */
    {DCM_IDSOPCLASSUID, DCM_UI, "", 1, sizeof(q.Image.ClassUID), (void *) &q.Image.ClassUID[0], DB_K_QCLASSUID, &q.Image.Query_Flag},
};

/* IAP_ObjectToQuery
**
** Purpose:
**	This function translates a DICOM object and an SOP Class into
**	the structure used by the DB query facility.  The DICOM object
**	is expected to contain a query level attribute and any number
**	of attributes that act as keys.  This function extracts only
**	the attributes that are supported as keys by the DB facility and
**	places those values in the query structure.  Bit flags are
**	also set which define the SOP class for the query and the
**	query level.
**
**	The function returns DCM_NORMAL upon success and DCM_ILLEGALOBJECT
**	upon failure.
**
** Parameter Dictionary:
**	object		Pointer to caller's object with keys stored
**			as attributes.
**	SOPClass	The SOP class of the requested query.  The
**			caller passes this string and the function sets
**			a bit in the query structure.
**	query		Pointer to caller's structure that this routine
**			will fill in.
**	elementCount	Pointer to caller's variable which holds the number
**			of keys that we extract from the object.
** Return Values:
**	IAP_NORMAL
**	IAP_OBJECTACCESSFAILED
**	IAP_ILLEGALOBJECT
**	IAP_INCOMPLETEOBJECT
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
IAP_ObjectToQuery(DCM_OBJECT ** object, char *SOPClass, Query * query, int *elementCount)
{
    CONDITION		cond, returnCondition;
    int         	index;
    void        	*ctx;
    U32				l;
    long        	flag;
    static char     queryLevelString[48] = "";	/* Initialize for AIX compiler bug */
    DCM_ELEMENT		queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), (void *) &queryLevelString[0]};

    *elementCount = 0;
    q.QueryState = 0;
    q.Patient.Query_Flag = 0;
    q.Study.Query_Flag = 0;
    q.Series.Query_Flag = 0;
    q.Image.Query_Flag = 0;

    ctx = NULL;
    returnCondition = IAP_NORMAL;

    cond = DCM_GetElementValue(object, &queryLevelElement, &l, &ctx);
    if (cond != DCM_NORMAL) {
    	if (cond == DCM_ILLEGALOBJECT){
    		return COND_PushCondition(IAP_ILLEGALOBJECT, "IAP_ObjectToQuery");
    	}else{
    		(void) COND_PopCondition(FALSE);
    		(void) COND_PushCondition(IAP_QUERYLEVELMISSING, "IAP_ObjectToQuery");
    		returnCondition = IAP_INCOMPLETEOBJECT;
    	}
    }else{
    	queryLevelString[l] = '\0';
    	if (queryLevelString[l - 1] == ' ') queryLevelString[l - 1] = '\0';

    	for (index = 0; index < DIM_OF(levelMap); index++) {
    		if (strcmp(levelMap[index].queryLevel, queryLevelString) == 0) q.QueryState |= levelMap[index].flag;
    	}
    }

    flag = 0;
    for (index = 0; index < DIM_OF(classMap); index++) {
    	if (strcmp(classMap[index].SOPClass, SOPClass) == 0) flag |= classMap[index].flag;
    }
    if (flag == 0) {
    	(void) COND_PushCondition(IAP_SOPCLASSMISSING, "");
    	returnCondition = IAP_INCOMPLETEOBJECT;
    }
    q.QueryState |= flag;

    cond = DCM_ParseObject(object, NULL, 0, list, (int) DIM_OF(list), elementCount);
    if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_ObjectToQuery");

    cond = parseImageInstanceUID(object, elementCount);
    if (cond != IAP_NORMAL)	return cond;

    *query = q;
    return returnCondition;
}

/* IAP_QueryToObject
**
** Purpose:
**	This function translates a DB query structure into a DICOM
**	object and an SOP Class UID.  The function looks at the query
**	level and attributes which are flagged in query structure
**	and creates a DICOM object to be returned to the caller.
**	The function also determines the UID of the SOP Class from the
**	query structure and returns the UID to the caller.
**
**	The function returns DCM_NORMAL upon success and other DCM conditions
**	upon failure.
**
** Parameter Dictionary:
**	query		Pointer to caller's structure.  This routine takes
**			data from that structure and creates a DICOM object.
**	object		Pointer to caller's object which will be created by
**			this routine.
**	SOPClass	Pointer to caller's area.  This function determines
**			the SOP Class from the query structure and returns
**			an ASCIZ string in the caller's area with the UID
**			of the SOP class.
**	elementCount	Pointer to caller's variable which holds the number
**			of keys that we place in the object.
** Return Values:
**	IAP_NORMAL
**	IAP_OBJECTACCESSFAILED
**	other values as returned by DCM_CreateObject
**
** Return Values:
**	IAP_NORMAL
**	IAP_CREATEOBJECTFAILED
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
IAP_QueryToObject(Query * query, DCM_OBJECT ** object, char *SOPClass, int *elementCount)
{
    CONDITION					cond, returnCondition;
    int	        				index;
    long       					flag;
    static char    				queryLevelString[48] = "";	/* Initialize for AIX compiler bug */
    DCM_ELEMENT					queryLevelElement = {DCM_IDQUERYLEVEL, DCM_CS, "", 1, sizeof(queryLevelString), (void *) &queryLevelString[0]};
    static DCM_FLAGGED_ELEMENT 	localList[] = {
										{DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, sizeof(q.Image.ImageUID), (void *) &q.Image.ImageUID[0], DB_K_QIMAGEUID, &q.Image.Query_Flag},
								};

    q = *query;
    *elementCount = 0;

    if (*object == NULL) {
    	cond = DCM_CreateObject(object, 0);
    	if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTCREATEFAILED, IAP_Message(IAP_OBJECTCREATEFAILED), "IAP_QueryToObject");
    }

    returnCondition = IAP_NORMAL;
    flag = q.QueryState & (DB_K_LEVELPAT | DB_K_LEVELSTUDY | DB_K_LEVELSERIES | DB_K_LEVELIMAGE);
    if (flag == 0){
    	returnCondition = COND_PushCondition(IAP_INCOMPLETEQUERY, "IAP_QueryToObject");
    }else{
    	for (index = 0; index < DIM_OF(levelMap); index++) {
    		if ((q.QueryState & levelMap[index].flag) != 0) {
    			(void) strcpy(queryLevelString, levelMap[index].queryLevel);
    			queryLevelElement.length = strlen(queryLevelElement.d.string);
    			cond = DCM_ModifyElements(object, &queryLevelElement, 1, NULL, 0, NULL);
    			if (cond != DCM_NORMAL) return cond;
    		}
    	}
    }


    flag = q.QueryState &
	(DB_K_CLASSPAT | DB_K_CLASSSTUDY | DB_K_CLASSPATSTUDY);
    if (flag == 0){
    	returnCondition = COND_PushCondition(IAP_INCOMPLETEQUERY, "");
    }else{
    	for (index = 0; index < DIM_OF(classMap); index++) {
    		if ((q.QueryState & classMap[index].flag) != 0) {
    			(void) strcpy(SOPClass, classMap[index].SOPClass);
    			cond = DCM_NORMAL;
    		}
    	}
    }

    cond = DCM_ModifyElements(object, NULL, 0, list, (int) DIM_OF(list), elementCount);
    if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_QueryToObject");

    cond = DCM_ModifyElements(object, NULL, 0, localList, (int) DIM_OF(localList), elementCount);
    if (cond != DCM_NORMAL)	return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "IAP_QueryToObject");

    return returnCondition;
}


static CONDITION
parseImageInstanceUID(DCM_OBJECT ** object, int *elementCount)
{
    static DCM_FLAGGED_ELEMENT 		localList[] = {
										{DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, sizeof(q.Image.ImageUID), (void *) &q.Image.ImageUID[0], DB_K_QIMAGEUID, &q.Image.Query_Flag},
									};
    CONDITION						cond;
    DCM_ELEMENT						e;
    U32								l;
    char		    				*c;
    int			    				index;

    cond = DCM_GetElementSize(object, DCM_IDSOPINSTANCEUID, &l);
    if (cond != DCM_NORMAL) {
    	(void) COND_PopCondition(FALSE);
    	return IAP_NORMAL;
    }

    cond = DCM_GetElement(object, DCM_IDSOPINSTANCEUID, &e);
    if (cond != DCM_NORMAL) {
    	(void) COND_PopCondition(FALSE);
    	return IAP_NORMAL;
    }

    if (e.multiplicity == 1) {
    	cond = DCM_ParseObject(object, NULL, 0, localList, (int) DIM_OF(localList), elementCount);
    	if (cond != DCM_NORMAL) return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "parseImageInstanceUID");
    	q.Image.Query_Flag |= DB_K_QIMAGEUID;
    	q.Image.ImageMultUIDCount = 0;
    	q.Image.ImageMultUID = NULL;
    }else{
    	c = malloc((l + 1) + (e.multiplicity * sizeof(char *)));
    	if (c == NULL) return COND_PushCondition(IAP_MALLOCFAILURE, IAP_Message(IAP_MALLOCFAILURE), (l + 1) + (e.multiplicity * sizeof(char *)), "parseImageInstanceUID");

    	q.Image.ImageMultUIDCount = e.multiplicity;
    	q.Image.ImageMultUID = (char **) c;
    	c += e.multiplicity * sizeof(char *);

    	e.d.string = c;

    	cond = DCM_ParseObject(object, &e, 1, NULL, 0, elementCount);
    	if (cond != DCM_NORMAL) return COND_PushCondition(IAP_OBJECTACCESSFAILED, IAP_Message(IAP_OBJECTACCESSFAILED), "parseImageInstanceUID");

    	for (index = 0; (unsigned long) index < e.multiplicity; index++) {
    		q.Image.ImageMultUID[index] = c;
    		while ((*c != '\\') && (*c != '\0')){
    			c++;
    		}
    		*c++ = '\0';
    	}
    	q.Image.Query_Flag |= DB_K_QIMAGEMULTUID;
    }
    return IAP_NORMAL;
}
