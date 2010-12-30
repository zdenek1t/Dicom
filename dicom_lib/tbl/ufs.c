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
** Module Name(s):	TBL_Open
**			TBL_Close
**			TBL_Select
**			TBL_Update
**			TBL_Insert
**			TBL_Delete
** Author, Date:
** Intent:		Provide a general set of functions to be performed
**			on tables in a relational database.  This implementation
**			is data stored in ASCII files and only supports the
**			select operation (implying the user enters the data
**			by hand).  I really only expect people to do this
**			for certain control functions where there are not
**			many tables and the entries are few.  If you want a
**			real database, you need to look at one of the other
**			implementations.
** Last Update:		$Author: smm $, $Date: 1999/04/29 03:48:32 $
** Source File:		$RCSfile: ufs.c,v $
** Revision:		$Revision: 1.14 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.14 $ $RCSfile: ufs.c,v $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "tblprivate.h"
#include "tbl.h"
#include "tbl_ufs.h"

static void 			translate(const char *from, char *to);
static CONDITION		localOpen(char *fileName, TBL_PRV_HANDLE * handle);
static TBL_PRV_HANDLE 	*handleCreate();
static CONDITION		localSelect(TBL_PRV_HANDLE * handle, const TBL_CRITERIA * criteria, TBL_FIELD * fields, long *count, CONDITION(*callback) (), void *ctx);
static int 				stringCompare(const TBL_CRITERIA * criteria, char *value);
static CONDITION 		fieldFill(TBL_FIELD * fields, char *fieldName, char *value);
static CONDITION 		valueFill(TBL_FIELD * field, char *value);
static CONDITION 		stringFill(TBL_FIELD * field, char *value);
static CONDITION 		signed4Fill(TBL_FIELD * field, char *value);
static int 				criteriaMatch(const TBL_CRITERIA * criteria, char *fieldName, char *value);
static int 				valueCompare(const TBL_CRITERIA * criteria, char *value);

/* TBL_Open
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
TBL_Open(const char *databaseName, const char *tableName, TBL_HANDLE ** handle)
{
    TBL_PRV_HANDLE			* privateHandle;
    static TBL_PRV_HANDLE   *   masterFileHandle = NULL;
    static CTNBOOLEAN       firstTrip = TRUE;
    char			        localTableName[128], masterFile[128];
    CONDITION				cond;
    TBL_CRITERIA			criteria[2];
    TBL_FIELD				fields[2];


    privateHandle = handleCreate();
    if (privateHandle == NULL) return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), tableName);
    if (masterFileHandle == NULL) masterFileHandle = handleCreate();
    if (masterFileHandle == NULL) return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), tableName);

    translate(tableName, localTableName);
    translate("TABLE_FILE", masterFile);

    if (firstTrip) {
    	cond = localOpen(masterFile, masterFileHandle);
    	if (cond != TBL_NORMAL) return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), tableName);
    	firstTrip = FALSE;
    }
    criteria[0].FieldName = "TABLE";
    criteria[0].Operator = TBL_EQUAL;
    TBL_LOAD_STRING(&criteria[0].Value, localTableName);
    criteria[1].FieldName = NULL;

    fields[0].FieldName = "FILENAME";
    TBL_EXISTING_STRING(&fields[0].Value, privateHandle->fileName);
    fields[1].FieldName = NULL;

    cond = localSelect(masterFileHandle, criteria, fields, NULL, NULL, NULL);
    if (cond == TBL_NORMAL) {
    	cond = localOpen(privateHandle->fileName, privateHandle);
    	if (cond == TBL_NORMAL) *handle = (TBL_HANDLE *) privateHandle;
    }
    return cond;
}

CONDITION
TBL_Close(TBL_HANDLE ** handle)
{
    free(*handle);
    *handle = NULL;
    return TBL_NORMAL;
}

CONDITION
TBL_Select(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_FIELD * fieldList, long *count, CONDITION(*callback) (), void *ctx)
{
    TBL_PRV_HANDLE		** privateHandle;
    CONDITION			cond;

    privateHandle = (TBL_PRV_HANDLE **) handle;

    cond = localSelect(*privateHandle, criteriaList, fieldList, count, callback, ctx);
    return cond;
}

CONDITION
TBL_Update(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList){
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Update");
}
CONDITION
TBL_Insert(TBL_HANDLE ** handle, TBL_FIELD * fieldList){
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Insert");
}
CONDITION
TBL_Delete(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList){
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Delete");
}

CONDITION
TBL_Layout(char *databaseName, char *tableName, CONDITION(*callback) (), void *ctx){
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Layout");
}

CONDITION
TBL_Debug(CTNBOOLEAN flag){
    return TBL_NORMAL;
}

void
TBL_BeginInsertTransaction(void){
}

void
TBL_CommitInsertTransaction(void){
}

CONDITION
TBL_NextUnique(TBL_HANDLE ** handle, char *name, int *unique){
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "");
}

void
TBL_RollbackInsertTransaction(void){
}

int
TBL_HasViews(void){
    return TBL_UNIMPLEMENTED;
}


static void
translate(const char *from, char *to)
{
    char	*env;

    strcpy(to, from);
    while ((env = getenv(to)) != NULL)
	strcpy(to, env);

}

static CONDITION
localOpen(char *fileName, TBL_PRV_HANDLE * handle){
    char			line[1024], scratch[1024];
    TBL_PRV_FIELD	* field;

    (void) strcpy(handle->fileName, fileName);
    handle->file = fopen(fileName, "r");

    if (handle->file == NULL) return COND_PushCondition(TBL_ERROR(TBL_FILEOPENFAILED), "localOpen", fileName);

    while (fgets(line, sizeof(line), handle->file) != NULL) {
    	if (sscanf(line, "%s", scratch) == 1) {
    		if (strcmp(scratch, "DATA") == 0) break;
    		if (strcmp(scratch, "FIELDNAME") == 0) {
    			if ((field = malloc(sizeof(*field))) == NULL) return COND_PushCondition(TBL_ERROR(TBL_MALLOCFAILURE), "localOpen");
    			if (sscanf(line, "%s %s", scratch, field->fieldName) != 2) return COND_PushCondition(TBL_ERROR(TBL_ILLEGALFORMAT), "no field name", "localOpen");
    			if (LST_Enqueue(&handle->fields, field) != LST_NORMAL) return COND_PushCondition(TBL_ERROR(TBL_LISTFAILURE), "localOpen");
    		}
    	}
    }
    handle->dataOffset = ftell(handle->file);
    return TBL_NORMAL;
}


static TBL_PRV_HANDLE *
handleCreate()
{
	TBL_PRV_HANDLE	   * privateHandle;

    privateHandle = malloc(sizeof(*privateHandle));
    if (privateHandle == NULL) {
    	(void) COND_PushCondition(TBL_ERROR(TBL_MALLOCFAILURE), "TBL_Open", sizeof(*privateHandle));
    	return NULL;
    }
    privateHandle->fields = LST_Create();
    if (privateHandle->fields == NULL) {
    	(void) COND_PushCondition(TBL_ERROR(TBL_LISTCREATEFAILURE), "handleCreate");
    	free(privateHandle);
    	privateHandle = NULL;
    }
    return privateHandle;
}

static CONDITION
localSelect(TBL_PRV_HANDLE * handle, const TBL_CRITERIA * criteria, TBL_FIELD * fields, long *count, CONDITION(*callback) (), void *ctx)
{
    TBL_PRV_FIELD    * field;
    char			 line[512], scratch[256], *p;
    int				 charsRead, match;
    CONDITION		 cond;
    long		     localCount = 0;


    field = LST_Head(&handle->fields);
    if (field != NULL)(void) LST_Position(&handle->fields, field);

    (void) fseek(handle->file, handle->dataOffset, 0);

    while (fgets(line, sizeof(line), handle->file) != NULL) {
    	field = LST_Head(&handle->fields);
    	if (field != NULL) (void) LST_Position(&handle->fields, field);
    	p = line;
    	match = 1;
    	while ((field != NULL) && match) {
    		if (sscanf(p, "%s %n", scratch, &charsRead) != 1) exit(1);
    		match &= criteriaMatch(criteria, field->fieldName, scratch);
    		p += charsRead;
    		field = LST_Next(&handle->fields);
    	}
    	if (match) {
    		field = LST_Head(&handle->fields);
    		if (field != NULL)(void) LST_Position(&handle->fields, field);
    		p = line;
    		while (field != NULL) {
    			if (sscanf(p, "%s %n", scratch, &charsRead) != 1) exit(1);
    			cond = fieldFill(fields, field->fieldName, scratch);
    			if (cond != TBL_NORMAL) return 0;
    			p += charsRead;
    			field = LST_Next(&handle->fields);
    		}
    		++localCount;
    		if (callback != NULL) cond = callback(fields, localCount, ctx);
    	}
    }
    if (count != NULL) *count = localCount;

    return TBL_NORMAL;
}

static int
criteriaMatch(const TBL_CRITERIA * criteria, char *fieldName, char *value)
{
    while (criteria->FieldName != NULL) {
    	if (strcmp(criteria->FieldName, fieldName) == 0){
    		break;
    	}else{
    		criteria++;
    	}
	}

    if (criteria->FieldName == NULL) return 1;

    return valueCompare(criteria, value);
}

static int
valueCompare(const TBL_CRITERIA * criteria, char *value)
{
    int 	cond;

    switch (criteria->Value.Type) {
		case TBL_STRING:
							cond = stringCompare(criteria, value);
							break;
		default:
							cond = 0;
							break;
    }
    return cond;
}

static int
stringCompare(const TBL_CRITERIA * criteria, char *value)
{
    int cond;
    switch (criteria->Operator) {
		case TBL_NULL:
		case TBL_NOT_NULL:
							cond = 0;
							break;
		case TBL_EQUAL:
							cond = (strcmp(criteria->Value.Value.String, value) == 0);
							break;
		case TBL_NOT_EQUAL:
							cond = (strcmp(criteria->Value.Value.String, value) != 0);
							break;
		case TBL_GREATER:
							cond = (strcmp(criteria->Value.Value.String, value) > 0);
							break;
		case TBL_GREATER_EQUAL:
							cond = (strcmp(criteria->Value.Value.String, value) >= 0);
							break;
		case TBL_LESS:
							cond = (strcmp(criteria->Value.Value.String, value) < 0);
							break;
		case TBL_LESS_EQUAL:
							cond = (strcmp(criteria->Value.Value.String, value) <= 0);
							break;
		default:
							cond = 0;
							break;
    }
    return cond;
}

static CONDITION
fieldFill(TBL_FIELD * fields, char *fieldName, char *value)
{
    while (fields->FieldName != NULL) {
    	if (strcmp(fields->FieldName, fieldName) == 0){
    		break;
    	}else{
    		fields++;
    	}
    }

    if (fields->FieldName == NULL) return TBL_NORMAL;

    return valueFill(fields, value);
}

static CONDITION
valueFill(TBL_FIELD * field, char *value)
{
    CONDITION 	cond;

    switch (field->Value.Type) {
		case TBL_STRING:
							cond = stringFill(field, value);
							break;
		case TBL_SIGNED4:
							cond = signed4Fill(field, value);
							break;
		default:
							cond = 0;
							break;
    }
    return cond;
}

static CONDITION
stringFill(TBL_FIELD * field, char *value)
{
    (void) strcpy(field->Value.Value.String, value);
    field->Value.IsNull = FALSE;
    field->Value.Size = strlen(field->Value.Value.String);
    return TBL_NORMAL;
}

static CONDITION
signed4Fill(TBL_FIELD * field, char *value)
{
    if (sscanf(value, "%d", field->Value.Value.Signed4) != 1) return 0;
    field->Value.IsNull = FALSE;
    field->Value.Size = 4;
    return TBL_NORMAL;
}
