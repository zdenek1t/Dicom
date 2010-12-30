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
** Module Name(s):	DMAN_Select
** Author, Date:	Steve Moore, Summer 1994
** Intent:		Provide a set of query routines to operate on
**			specific tables in our database.
** Last Update:		$Author: smm $, $Date: 1998/10/22 17:42:04 $
** Source File:		$RCSfile: select.c,v $
** Revision:		$Revision: 1.17 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.17 $ $RCSfile: select.c,v $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "manage.h"
#include "dmanprivate.h"

#define DMAN_ERROR(a) (a), DMAN_Message((a))
typedef struct {
    LST_HEAD 				**lst;
    DMAN_GENERICRECORD 		*record;
}   CONTEXT;

static void clearList(LST_HEAD * lst);
static CONDITION
queryAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord, DMAN_APPLICATIONENTITY * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackAE(TBL_FIELD * field, int count, void *ctx);
static CONDITION
queryGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord,	DMAN_GROUPNAMES * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
queryStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * workRecord, DMAN_STORAGEACCESS * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackSA(TBL_FIELD * field, int count, void *ctx);

static CONDITION
queryStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * workRecord, DMAN_STORAGECONTROL * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackStorageControl(TBL_FIELD * field, int count, void *ctx);
static CONDITION
querySecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * workRecord, DMAN_SECURITYMATRIX * criteriaRecord, LST_HEAD * l,	CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackSecurityMatrix(TBL_FIELD * field, int count, void *ctx);
static CONDITION
queryFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * workRecord, DMAN_FISACCESS * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackFA(TBL_FIELD * field, int count, void *ctx);
static CONDITION
queryPrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * workRecord, DMAN_PRINTSERVERCFG * criteriaRecord, LST_HEAD * l,	CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackPrintServerCFG(TBL_FIELD * field, int count, void *ctx);

static CONDITION
queryVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * workRecord, DMAN_VIDEOIMAGEDEST * criteriaRecord, LST_HEAD * l, CONDITION(*callback) (), long *count, void *ctx);
static CONDITION
callbackVideoImageDest(TBL_FIELD * field, int count, void *ctx);

/* DMAN_Select
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
DMAN_Select(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord, DMAN_GENERICRECORD * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION			cond;
    PRIVATE_HANDLE		* prv;

    clearList(head);
    prv = *(PRIVATE_HANDLE **) handle;

    switch (criteriaRecord->Type) {
		case DMAN_K_APPLICATIONENTITY:
									cond = queryAE(&prv->applicationEntityHandle, (DMAN_APPLICATIONENTITY *) workRecord, (DMAN_APPLICATIONENTITY *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_GROUPNAMES:
									cond = queryGroupNames(&prv->groupNamesHandle, (DMAN_GROUPNAMES *) workRecord, (DMAN_GROUPNAMES *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_STORAGEACCESS:
									cond = queryStorageAccess(&prv->storageAccessHandle, (DMAN_STORAGEACCESS *) workRecord, (DMAN_STORAGEACCESS *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_STORAGECONTROL:
									cond = queryStorageControl(&prv->storageControlHandle, (DMAN_STORAGECONTROL *) workRecord, (DMAN_STORAGECONTROL *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_SECURITYMATRIX:
									cond = querySecurityMatrix(&prv->securityMatrixHandle, (DMAN_SECURITYMATRIX *) workRecord, (DMAN_SECURITYMATRIX *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_FISACCESS:
									cond = queryFISAccess(&prv->FISAccessHandle, (DMAN_FISACCESS *) workRecord, (DMAN_FISACCESS *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_PRINTSERVERCFG:
									cond = queryPrintServerCFG(&prv->printServerCFGHandle, (DMAN_PRINTSERVERCFG *) workRecord, (DMAN_PRINTSERVERCFG *) criteriaRecord, head, callback, count, ctx);
									break;
		case DMAN_K_VIDEOIMAGEDEST:
									cond = queryVideoImageDest(&prv->VideoImageDestHandle, (DMAN_VIDEOIMAGEDEST *) workRecord, (DMAN_VIDEOIMAGEDEST *) criteriaRecord, head, callback, count, ctx);
									break;
		default:
									break;
    }
    return cond;
}

static void
clearList(LST_HEAD * lst)
{
    LST_NODE    * node;

    if (lst == NULL) return;

    while ((node = LST_Dequeue(&lst)) != NULL){
    	free(node);
    }
}

static CONDITION
queryAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord, DMAN_APPLICATIONENTITY * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION		    cond;
    TBL_CRITERIA		criteriaList[3];
    static TBL_FIELD    fields[10];
    int				    i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_APPLICATION_TITLE) {
    	criteriaList[i].FieldName = "Title";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->Title);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_APPLICATION_NODE) {
    	criteriaList[i].FieldName = "Node";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->Node);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    fields[i].FieldName = "Title";
    TBL_EXISTING_STRING(&fields[i].Value, workRecord->Title);
    i++;
    fields[i].FieldName = "Node";
    TBL_EXISTING_STRING(&fields[i].Value, workRecord->Node);
    i++;
    fields[i].FieldName = "Comment";
    TBL_EXISTING_STRING(&fields[i].Value, workRecord->Comment);
    i++;
    TBL_FIELD_LOAD_NUM(fields[i], "Port", workRecord->Port, TBL_SIGNED4);
    i++;
    fields[i].FieldName = "Organization";
    TBL_EXISTING_STRING(&fields[i].Value, workRecord->Organization);
    i++;

    fields[i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackAE, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackAE(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_APPLICATIONENTITY	    * ae, *localAE;
    CONDITION			 	    cond;
    int					        i;
    static FLAG_MAP
        map[] = {
        		{"Title", DMAN_K_APPLICATION_TITLE},
        		{"Node", DMAN_K_APPLICATION_NODE},
        		{"Port", DMAN_K_APPLICATION_PORT},
        		{"Comment", DMAN_K_APPLICATION_COMMENT},
        		{"Organization", DMAN_K_APPLICATION_ORGANIZATION},
    };
    CONTEXT *ctx;

    ctx = (CONTEXT *) ctxArg;

    ae = (DMAN_APPLICATIONENTITY *) ctx->record;
    ae->Flag = 0;
    while (field->FieldName != NULL) {
    	for (i = 0; i < (int) DIM_OF(map); i++) {
    		if (strcmp(field->FieldName, map[i].FieldName) == 0) {
    			if (!field->Value.IsNull) ae->Flag |= map[i].Flag;
    			break;
    		}
    	}
    	field++;
    }

    if (*ctx->lst != NULL) {
    	localAE = malloc(sizeof(*localAE));
    	if (localAE == NULL) return 0;
    	*localAE = *ae;


    	cond = LST_Enqueue(ctx->lst, localAE);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}


static CONDITION
callbackGroup(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_GROUPNAMES    * g, *localGroup;
    CONDITION		   cond;
    int			       i;
    static FLAG_MAP
        map[] = {
        		{"Title", DMAN_K_GROUP_TITLE},
        		{"GroupName", DMAN_K_GROUP_GROUP},
    };
    CONTEXT *ctx;

    ctx = (CONTEXT *) ctxArg;
    g = (DMAN_GROUPNAMES *) ctx->record;
    g->Flag = 0;
    while (field->FieldName != NULL) {
    	for (i = 0; i < (int) DIM_OF(map); i++) {
    		if (strcmp(field->FieldName, map[i].FieldName) == 0) {
    			if (!field->Value.IsNull) g->Flag |= map[i].Flag;
    			break;
    		}
    	}
    	field++;
    }

    if (*ctx->lst != NULL) {
    	localGroup = malloc(sizeof(*localGroup));
    	if (localGroup == NULL) return 0;
    	*localGroup = *g;

    	cond = LST_Enqueue(ctx->lst, localGroup);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}
static CONDITION
queryGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord,	DMAN_GROUPNAMES * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[3];
    static TBL_FIELD   	fields[4];
    int			        i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_GROUP_TITLE) {
    	criteriaList[i].FieldName = "Title";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->Title);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_GROUP_GROUP) {
    	criteriaList[i].FieldName = "GroupName";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->GroupName);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    fields[0].FieldName = "Title";
    TBL_EXISTING_STRING(&fields[0].Value, workRecord->Title);
    fields[1].FieldName = "GroupName";
    TBL_EXISTING_STRING(&fields[1].Value, workRecord->GroupName);
    fields[2].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackGroup, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
queryStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * workRecord, DMAN_STORAGEACCESS * criteriaRecord, LST_HEAD * head,	CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[2];
    static TBL_FIELD    fields[10];
    int			        i;
    CONTEXT				context;

    i = 0;
    if ((criteriaRecord->Flag & DMAN_K_STORAGEACCESS_TITLE)) {
    	criteriaList[i].FieldName = "Title";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->Title);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "Title", workRecord->Title, sizeof(workRecord->Title));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "DbKey", workRecord->DbKey, sizeof(workRecord->DbKey));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Owner", workRecord->Owner, sizeof(workRecord->Owner));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "GroupName", workRecord->GroupName, sizeof(workRecord->GroupName));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Comment", workRecord->Comment, sizeof(workRecord->Comment));
    i++;
    TBL_FIELD_DECLARE_NUM(fields[i], "Access", TBL_SIGNED4, workRecord->Access, sizeof(workRecord->Access));


    fields[++i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackSA, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackSA(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_STORAGEACCESS		* sa, *localSA;
    CONDITION				cond;
    int				        i;
    static FLAG_MAP
        map[] = {
        		{"Title", DMAN_K_STORAGEACCESS_TITLE},
        		{"DbKey", DMAN_K_STORAGEACCESS_DBKEY},
        		{"Owner", DMAN_K_STORAGEACCESS_OWNER},
        		{"GroupName", DMAN_K_STORAGEACCESS_GROUPNAME},
        		{"Comment", DMAN_K_STORAGEACCESS_COMMENT}
    };
    CONTEXT *ctx;

    ctx = (CONTEXT *) ctxArg;
    sa = (DMAN_STORAGEACCESS *) ctx->record;
    sa->Flag = 0;

    while (field->FieldName != NULL) {
    	for (i = 0; i < (int) DIM_OF(map); i++) {
    		if (strcmp(field->FieldName, map[i].FieldName) == 0) {
    			if (!field->Value.IsNull) sa->Flag |= map[i].Flag;
    			break;
    		}
    	}
    	field++;
    }

    if (*ctx->lst != NULL) {
    	localSA = malloc(sizeof(*localSA));
    	if (localSA == NULL) return 0;

    	*localSA = *sa;

    	cond = LST_Enqueue(ctx->lst, localSA);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}

static CONDITION
queryStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * workRecord, DMAN_STORAGECONTROL * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[3];
    static TBL_FIELD	fields[7];
    int				    i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_STORAGECONTROL_REQUESTING) {
    	TBL_CRITERIA_LOAD_BYTE(criteriaList[i], "RequestingTitle", criteriaRecord->RequestingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_STORAGECONTROL_RESPONDING) {
    	TBL_CRITERIA_LOAD_BYTE(criteriaList[i], "RespondingTitle", criteriaRecord->RespondingTitle, TBL_STRING, TBL_EQUAL);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "RequestingTitle", workRecord->RequestingTitle, sizeof(workRecord->RequestingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "RespondingTitle", workRecord->RespondingTitle, sizeof(workRecord->RespondingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Medium", workRecord->Medium, sizeof(workRecord->Medium));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Root", workRecord->Root, sizeof(workRecord->Root));

    fields[++i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;
    cond = TBL_Select(handle, criteriaList, fields, count, callbackStorageControl, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackStorageControl(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_STORAGECONTROL    	* s, *localStorage;
    CONDITION				cond;
    int				        i;
    static FLAG_MAP
        map[] = {
        		{"RequestingTitle", DMAN_K_STORAGECONTROL_REQUESTING},
        		{"RespondingTitle", DMAN_K_STORAGECONTROL_RESPONDING},
        		{"Medium", DMAN_K_STORAGECONTROL_MEDIUM},
        		{"Root", DMAN_K_STORAGECONTROL_ROOT}
		};
    CONTEXT 				*ctx;

    ctx = (CONTEXT *) ctxArg;

    s = (DMAN_STORAGECONTROL *) ctx->record;
    s->Flag = 0;
    while (field->FieldName != NULL) {
    	for (i = 0; i < (int) DIM_OF(map); i++) {
    		if (strcmp(field->FieldName, map[i].FieldName) == 0) {
    			if (!field->Value.IsNull) s->Flag |= map[i].Flag;
    			break;
    		}
    	}
    	field++;
    }

    if (*ctx->lst != NULL) {
    	localStorage = malloc(sizeof(*localStorage));
    	if (localStorage == NULL) return 0;
    	*localStorage = *s;


    	cond = LST_Enqueue(ctx->lst, localStorage);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}

static CONDITION
querySecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * workRecord, DMAN_SECURITYMATRIX * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[3];
    static TBL_FIELD	fields[3];
    int			        i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_SECURITY_REQUESTING) {
    	criteriaList[i].FieldName = "RequestingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RequestingTitle);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_SECURITY_RESPONDING) {
    	criteriaList[i].FieldName = "RespondingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RespondingTitle);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "RequestingTitle", workRecord->RequestingTitle, sizeof(workRecord->RequestingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "RespondingTitle", workRecord->RespondingTitle, sizeof(workRecord->RespondingTitle));

    i++;
    fields[i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;
    cond = TBL_Select(handle, criteriaList, fields, count, callbackSecurityMatrix, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackSecurityMatrix(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_SECURITYMATRIX 	* matrix, *localMatrix;
    CONDITION				cond;
    CONTEXT 				*ctx;

    ctx = (CONTEXT *) ctxArg;

    matrix = (DMAN_SECURITYMATRIX *) ctx->record;
    matrix->Flag = 0;

    while (field->FieldName != NULL) {
    	if (strcmp(field->FieldName, "RequestingTitle") == 0) {
    		if (!field->Value.IsNull) matrix->Flag |= DMAN_K_SECURITY_REQUESTING;
    	}else if (strcmp(field->FieldName, "RespondingTitle") == 0){
    		if (!field->Value.IsNull) matrix->Flag |= DMAN_K_SECURITY_RESPONDING;
    	}
    	field++;
    }


    if (*ctx->lst != NULL) {
    	localMatrix = malloc(sizeof(*localMatrix));
    	if (localMatrix == NULL) return 0;
    	*localMatrix = *matrix;

    	cond = LST_Enqueue(ctx->lst, localMatrix);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}

static CONDITION
queryFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * workRecord, DMAN_FISACCESS * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[2];
    static TBL_FIELD    fields[10];
    int			        i;
    CONTEXT				context;

    i = 0;
    if ((criteriaRecord->Flag & DMAN_K_FISACCESS_TITLE)) {
    	criteriaList[i].FieldName = "Title";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->Title);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "Title", workRecord->Title, sizeof(workRecord->Title));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "DbKey", workRecord->DbKey, sizeof(workRecord->DbKey));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Owner", workRecord->Owner, sizeof(workRecord->Owner));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "GroupName", workRecord->GroupName, sizeof(workRecord->GroupName));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "Comment", workRecord->Comment, sizeof(workRecord->Comment));
    i++;
    TBL_FIELD_DECLARE_NUM(fields[i], "Access", TBL_SIGNED4, workRecord->Access, sizeof(workRecord->Access));


    fields[++i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackFA, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackFA(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_FISACCESS  	* sa, *localSA;
    CONDITION			cond;
    int				    i;
    static FLAG_MAP
        map[] = {
        		{"Title", DMAN_K_FISACCESS_TITLE},
        		{"DbKey", DMAN_K_FISACCESS_DBKEY},
        		{"Owner", DMAN_K_FISACCESS_OWNER},
        		{"GroupName", DMAN_K_FISACCESS_GROUPNAME},
        		{"Comment", DMAN_K_FISACCESS_COMMENT}
    };
    CONTEXT *ctx;

    ctx = (CONTEXT *) ctxArg;

    sa = (DMAN_FISACCESS *) ctx->record;
    sa->Flag = 0;

    while (field->FieldName != NULL) {
    	for (i = 0; i < (int) DIM_OF(map); i++) {
    		if (strcmp(field->FieldName, map[i].FieldName) == 0) {
    			if (!field->Value.IsNull) sa->Flag |= map[i].Flag;
    			break;
    		}
    	}
    	field++;
    }

    if (*ctx->lst != NULL) {
    	localSA = malloc(sizeof(*localSA));
    	if (localSA == NULL) return 0;

    	*localSA = *sa;
    	cond = LST_Enqueue(ctx->lst, localSA);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}
static CONDITION
queryPrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * workRecord, DMAN_PRINTSERVERCFG * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[4];
    static TBL_FIELD	fields[4];
    int			        i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_REQUESTING) {
    	criteriaList[i].FieldName = "RequestingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RequestingTitle);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_RESPONDING) {
    	criteriaList[i].FieldName = "RespondingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RespondingTitle);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_PRINTSERVER_GQID) {
    	TBL_CRITERIA_LOAD_NUM(criteriaList[i], "GQId", criteriaRecord->GQId, TBL_SIGNED4, TBL_EQUAL);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "RequestingTitle", workRecord->RequestingTitle, sizeof(workRecord->RequestingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "RespondingTitle", workRecord->RespondingTitle, sizeof(workRecord->RespondingTitle));
    i++;
    TBL_FIELD_DECLARE_NUM(fields[i], "GQId", TBL_SIGNED4, workRecord->GQId, sizeof(workRecord->GQId));
    i++;
    fields[i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackPrintServerCFG, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
callbackPrintServerCFG(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_PRINTSERVERCFG    * cfg, *localCfg;
    CONDITION				cond;
    CONTEXT 				*ctx;

    ctx = (CONTEXT *) ctxArg;
    cfg = (DMAN_PRINTSERVERCFG *) ctx->record;
    cfg->Flag = 0;

    while (field->FieldName != NULL) {
    	if (strcmp(field->FieldName, "RequestingTitle") == 0) {
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_PRINTSERVER_REQUESTING;
    	}else if (strcmp(field->FieldName, "RespondingTitle") == 0){
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_PRINTSERVER_RESPONDING;
    	}else if (strcmp(field->FieldName, "GQId") == 0){
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_PRINTSERVER_GQID;
    	}
    	field++;
    }


    if (*ctx->lst != NULL) {
    	localCfg = malloc(sizeof(*localCfg));
    	if (localCfg == NULL) return 0;
    	*localCfg = *cfg;

    	cond = LST_Enqueue(ctx->lst, localCfg);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}

static CONDITION
queryVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * workRecord, DMAN_VIDEOIMAGEDEST * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
{
    CONDITION	    	cond;
    TBL_CRITERIA		criteriaList[4];
    static TBL_FIELD	fields[4];
    int				    i;
    CONTEXT				context;

    i = 0;
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_REQUESTING) {
    	criteriaList[i].FieldName = "RequestingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RequestingTitle);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_RESPONDING) {
    	criteriaList[i].FieldName = "RespondingTitle";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->RespondingTitle);
    	i++;
    }
    if (criteriaRecord->Flag & DMAN_K_VIDEOIMAGE_IMAGETYPE) {
    	criteriaList[i].FieldName = "ImageType";
    	criteriaList[i].Operator = TBL_EQUAL;
    	TBL_LOAD_STRING(&criteriaList[i].Value, criteriaRecord->ImageType);
    	i++;
    }
    criteriaList[i].FieldName = NULL;

    i = 0;
    TBL_FIELD_DECLARE_STRING(fields[i], "RequestingTitle", workRecord->RequestingTitle, sizeof(workRecord->RequestingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "RespondingTitle", workRecord->RespondingTitle, sizeof(workRecord->RespondingTitle));
    i++;
    TBL_FIELD_DECLARE_STRING(fields[i], "ImageType", workRecord->ImageType, sizeof(workRecord->ImageType));
    i++;
    fields[i].FieldName = NULL;

    context.lst = &head;
    context.record = (DMAN_GENERICRECORD *) workRecord;

    cond = TBL_Select(handle, criteriaList, fields, count, callbackVideoImageDest, &context);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}
static CONDITION
callbackVideoImageDest(TBL_FIELD * field, int count, void *ctxArg)
{
    DMAN_VIDEOIMAGEDEST     * cfg, *localCfg;
    CONDITION				cond;
    CONTEXT 				*ctx;

    ctx = (CONTEXT *) ctxArg;

    cfg = (DMAN_VIDEOIMAGEDEST *) ctx->record;
    cfg->Flag = 0;

    while (field->FieldName != NULL) {
    	if (strcmp(field->FieldName, "RequestingTitle") == 0){
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_VIDEOIMAGE_REQUESTING;
    	}else if (strcmp(field->FieldName, "RespondingTitle") == 0){
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_VIDEOIMAGE_RESPONDING;
    	}else if (strcmp(field->FieldName, "ImageType") == 0){
    		if (!field->Value.IsNull) cfg->Flag |= DMAN_K_VIDEOIMAGE_IMAGETYPE;
    	}
    	field++;
    }


    if (*ctx->lst != NULL) {
    	localCfg = malloc(sizeof(*localCfg));
    	if (localCfg == NULL) return 0;
    	*localCfg = *cfg;

    	cond = LST_Enqueue(ctx->lst, localCfg);
    	if (cond != LST_NORMAL) return 0;
    }
    return TBL_NORMAL;
}
