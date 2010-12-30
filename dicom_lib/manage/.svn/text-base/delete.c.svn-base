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
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	DMAN_Delete
** Author, Date:	Steve Moore, Summer 1994
** Intent:		This file contains the entry point for deleting
**			data from the DMAN tables.  This entry point is
**			used to call the specific function to delete one or
**			more rows from a DMAN table (based on the criteria
**			supplied by the caller).
** Last Update:		$Author: smm $, $Date: 1998/05/22 18:18:18 $
** Source File:		$RCSfile: delete.c,v $
** Revision:		$Revision: 1.14 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.14 $ $RCSfile: delete.c,v $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "../manage/manage.h"
#include "dmanprivate.h"

#define DMAN_ERROR(a) (a), DMAN_Message((a))

static CONDITION
deleteAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * criteriaRecord);
static CONDITION
deleteGroup(TBL_HANDLE ** handle, DMAN_GROUPNAMES * criteriaRecord);
static CONDITION
deleteStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * criteriaRecord);
static CONDITION
deleteSecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * criteriaRecord);
static CONDITION
deleteStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * criteriaRecord);
static CONDITION
deleteFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * criteriaRecord);
static CONDITION
deletePrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * criteriaRecord);
static CONDITION
deleteVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * criteriaRecord);

/* DMAN_Delete
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

CONDITION
DMAN_Delete(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * criteriaRecord)
{
    CONDITION			cond;
    PRIVATE_HANDLE		* prv;

    prv = *(PRIVATE_HANDLE **) handle;
    switch (criteriaRecord->Type) {
		case DMAN_K_APPLICATIONENTITY:
							cond = deleteAE(&prv->applicationEntityHandle,(DMAN_APPLICATIONENTITY *) criteriaRecord);
							break;
		case DMAN_K_GROUPNAMES:
							cond = deleteGroup(&prv->groupNamesHandle, (DMAN_GROUPNAMES *) criteriaRecord);
							break;
		case DMAN_K_STORAGECONTROL:
							cond = deleteStorageControl(&prv->storageControlHandle, (DMAN_STORAGECONTROL *) criteriaRecord);
							break;
		case DMAN_K_SECURITYMATRIX:
							cond = deleteSecurityMatrix(&prv->securityMatrixHandle, (DMAN_SECURITYMATRIX *) criteriaRecord);
							break;
		case DMAN_K_STORAGEACCESS:
							cond = deleteStorageAccess(&prv->storageAccessHandle, (DMAN_STORAGEACCESS *) criteriaRecord);
							break;
		case DMAN_K_FISACCESS:
							cond = deleteFISAccess(&prv->FISAccessHandle, (DMAN_FISACCESS *) criteriaRecord);
							break;
		case DMAN_K_PRINTSERVERCFG:
							cond = deletePrintServerCFG(&prv->printServerCFGHandle, (DMAN_PRINTSERVERCFG *) criteriaRecord);
							break;
		case DMAN_K_VIDEOIMAGEDEST:
							cond = deleteVideoImageDest(&prv->VideoImageDestHandle, (DMAN_VIDEOIMAGEDEST *) criteriaRecord);
							break;
		default:
							break;
    }
    return cond;

}

static CONDITION
deleteAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * criteriaRecord)
{
    CONDITION	    		cond;
    static TBL_CRITERIA     criteria[5];
    int					    i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_APPLICATION_TITLE) {
    	criteria[i].FieldName = "Title";
    	criteria[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteria[i].Value, criteriaRecord->Title);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_APPLICATION_NODE) {
    	criteria[i].FieldName = "Node";
    	criteria[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteria[i].Value, criteriaRecord->Node);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
	if (cond == TBL_NORMAL){
		return DMAN_NORMAL;
	}else{
    	return 0;
	}
}

static CONDITION
deleteGroup(TBL_HANDLE ** handle, DMAN_GROUPNAMES * criteriaRecord)
{
    CONDITION	   			cond;
    static TBL_CRITERIA     criteria[5];
    int				        i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_GROUP_TITLE) {
    	criteria[i].FieldName = "Title";
    	criteria[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteria[i].Value, criteriaRecord->Title);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_GROUP_GROUP) {
    	criteria[i].FieldName = "GroupName";
    	criteria[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteria[i].Value, criteriaRecord->GroupName);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}


static CONDITION
deleteStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[5];
    int				        i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_STORAGECONTROL_REQUESTING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RequestingTitle", criteriaRecord->RequestingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_STORAGECONTROL_RESPONDING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RespondingTitle", criteriaRecord->RespondingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
deleteSecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[3];
    int				        i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_SECURITY_REQUESTING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RequestingTitle", criteriaRecord->RequestingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_SECURITY_RESPONDING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RespondingTitle", criteriaRecord->RespondingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
deleteStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[2];
    int					    i = 0;

    if (criteriaRecord->Flag & DMAN_K_STORAGEACCESS_TITLE) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "Title", criteriaRecord->Title, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
deleteFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[2];
    int				        i = 0;

    if (criteriaRecord->Flag & DMAN_K_FISACCESS_TITLE) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "Title", criteriaRecord->Title, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
deletePrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[4];		/* 3 fields and one sentinel (NULL) */
    int				        i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_REQUESTING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RequestingTitle", criteriaRecord->RequestingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_RESPONDING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RespondingTitle", criteriaRecord->RespondingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_GQID) {
    	TBL_CRITERIA_LOAD_NUM(criteria[i], "GQId", criteriaRecord->GQId, TBL_SIGNED4, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
deleteVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * criteriaRecord)
{
    CONDITION			    cond;
    static TBL_CRITERIA     criteria[4];		/* 3 fields and one sentinel (NULL) */
    int				        i;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_REQUESTING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RequestingTitle", criteriaRecord->RequestingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_RESPONDING) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "RespondingTitle", criteriaRecord->RespondingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_IMAGETYPE) {
    	TBL_CRITERIA_LOAD_BYTE(criteria[i], "ImageType", criteriaRecord->ImageType, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteria[i].FieldName = NULL;

    cond = TBL_Delete(handle, criteria);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}
