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
** Module Name(s):	DMAN_Set
** Author, Date:	Steve Moore, Summer 1994
** Intent:		Provide a set of routines to operate on management
**			tables in our database.  The functions in this
**			file are used to set (modify) existing rows in a table.
** Last Update:		$Author: smm $, $Date: 1998/05/22 18:19:38 $
** Source File:		$RCSfile: set.c,v $
** Revision:		$Revision: 1.8 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.8 $ $RCSfile: set.c,v $";

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
#include "manage.h"
#include "dmanprivate.h"

#define DMAN_ERROR(a) (a), DMAN_Message((a))
typedef struct {
    LST_HEAD **lst;
    DMAN_GENERICRECORD *record;
}   CONTEXT;

static void clearList(LST_HEAD * lst);
#ifdef SMM
static CONDITION
setAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord, DMAN_CRITERIA criteria, DMAN_APPLICATIONENTITY * criteriaRecord);
static CONDITION
callbackAE(TBL_FIELD * field, int count, CONTEXT * ctx);
static CONDITION
queryGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord, DMAN_CRITERIA criteria,	DMAN_GROUPNAMES * criteriaRecord, LST_HEAD * head,
				CONDITION(*callback) (), long *count, void *ctx);
#endif

/* DMAN_Set
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

#ifdef SMM
CONDITION
DMAN_Set(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord, DMAN_CRITERIA criteria, DMAN_GENERICRECORD * criteriaRecord)
{
    CONDITION			cond;
    PRIVATE_HANDLE		* prv;

    prv = *(PRIVATE_HANDLE **) handle;

    switch (criteriaRecord->Type) {
		case DMAN_K_APPLICATIONENTITY:
						cond = setAE(&prv->applicationEntityHandle,(DMAN_APPLICATIONENTITY *) workRecord, criteria, (DMAN_APPLICATIONENTITY *) criteriaRecord);
						break;
		case DMAN_K_GROUPNAMES:
						break;
		case DMAN_K_STORAGEACCESS:
						break;
		default:
						break;
    }
    return cond;
}
#endif

static void
clearList(LST_HEAD * lst)
{
    LST_NODE	    * node;

    while ((node = LST_Dequeue(&lst)) != NULL)
    	free(node);
}

#ifdef SMM
static CONDITION
setAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord, DMAN_CRITERIA criteria, DMAN_APPLICATIONENTITY * criteriaRecord)
{
    CONDITION		   	cond;
    TBL_CRITERIA		criteriaList[2];
    static TBL_UPDATE	updateList[5];
    int			        i;

    switch (criteria) {
		case DMAN_K_ALLRECORDS:
							criteriaList[0].FieldName = NULL;
							break;
		case DMAN_K_BYTITLE:
							criteriaList[0].FieldName = "Title";
							criteriaList[0].Operator = TBL_EQUAL;
							TBL_LOAD_STRING(&criteriaList[0].Value, criteriaRecord->Title);
							criteriaList[1].FieldName = NULL;
							break;
		case DMAN_K_BYNODE:
							criteriaList[0].FieldName = "Node";
							criteriaList[0].Operator = TBL_EQUAL;
							TBL_LOAD_STRING(&criteriaList[0].Value, criteriaRecord->Node);
							criteriaList[1].FieldName = NULL;
							break;
		default:
							break;
    }
    i = 0;
    if (workRecord->Flag & DMAN_K_APPLICATION_TITLE) {
    	updateList[i].FieldName = "Title";
    	updateList[i].Function = TBL_SET;
    	TBL_LOAD_STRING(&updateList[i].Value, workRecord->Title);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_NODE) {
    	updateList[i].FieldName = "Node";
    	updateList[i].Function = TBL_SET;
    	TBL_LOAD_STRING(&updateList[i].Value, workRecord->Node);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_COMMENT) {
    	updateList[i].FieldName = "Comment";
    	updateList[i].Function = TBL_SET;
    	TBL_LOAD_STRING(&updateList[i].Value, workRecord->Comment);
    	i++;
    }
    if (i == 0)	return DMAN_NORMAL;

    updateList[i].FieldName = NULL;

    cond = TBL_Update(handle, criteriaList, updateList);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}
#endif

static CONDITION
callbackAE(TBL_FIELD * field, int count, CONTEXT * ctx)
{
    DMAN_APPLICATIONENTITY    * ae, *localAE;
    CONDITION				  cond;

    localAE = malloc(sizeof(*localAE));
    if (localAE == NULL) return 0;

    ae = (DMAN_APPLICATIONENTITY *) ctx->record;
    *localAE = *ae;

    cond = LST_Enqueue(ctx->lst, localAE);
    if (cond != LST_NORMAL)	return 0;

    return TBL_NORMAL;
}

#ifdef SMM
static CONDITION
queryGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord, DMAN_CRITERIA criteria, DMAN_GROUPNAMES * criteriaRecord, LST_HEAD * head,
				CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[3];
    static TBL_FIELD    fields[4];

    switch (criteria) {
		case DMAN_K_ALLRECORDS:
							criteriaList[0].FieldName = NULL;
							break;
		case DMAN_K_BYTITLE:
							criteriaList[0].FieldName = "Title";
							criteriaList[0].Operator = TBL_EQUAL;
							TBL_LOAD_STRING(&criteriaList[0].Value, criteriaRecord->Title);
							criteriaList[1].FieldName = NULL;
							break;
		case DMAN_K_BYTITLE_GROUPNAME:
							criteriaList[0].FieldName = "Title";
							criteriaList[0].Operator = TBL_EQUAL;
							TBL_LOAD_STRING(&criteriaList[0].Value, criteriaRecord->Title);
							criteriaList[1].FieldName = "GroupName";
							criteriaList[1].Operator = TBL_EQUAL;
							TBL_LOAD_STRING(&criteriaList[1].Value, criteriaRecord->GroupName);
							criteriaList[2].FieldName = NULL;
							break;
		default:
							break;
    }
    fields[0].FieldName = "Title";
    TBL_EXISTING_STRING(&fields[0].Value, workRecord->Title);
    fields[1].FieldName = "GroupName";
    TBL_EXISTING_STRING(&fields[1].Value, workRecord->GroupName);
    fields[2].FieldName = NULL;

    cond = TBL_Select(handle, criteriaList, fields, count, callback, ctx);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}
#endif
