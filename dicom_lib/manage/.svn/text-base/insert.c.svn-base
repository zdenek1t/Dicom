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
** Module Name(s):	DMAN_Insert
** Author, Date:	Steve Moore, Summer 1994
** Intent:		Provide a set of insert routines to operate on
**			specific tables in our database.
** Last Update:		$Author: smm $, $Date: 1998/05/22 18:19:16 $
** Source File:		$RCSfile: insert.c,v $
** Revision:		$Revision: 1.16 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.16 $ $RCSfile: insert.c,v $";

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
insertAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord);
static CONDITION
insertStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * workRecord);
static CONDITION
insertSecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * workRecord);
static CONDITION
insertStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * workRecord);
static CONDITION
insertFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * workRecord);
static CONDITION
insertPrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * workRecord);
static CONDITION
insertVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * workRecord);
static CONDITION
insertGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord);

/* DMAN_Insert
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
DMAN_Insert(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord)
{
    CONDITION			cond;
    PRIVATE_HANDLE		* prv;

    prv = *(PRIVATE_HANDLE **) handle;

    switch (workRecord->Type) {
		case DMAN_K_APPLICATIONENTITY:
								cond = insertAE(&prv->applicationEntityHandle, (DMAN_APPLICATIONENTITY *) workRecord);
								break;
		case DMAN_K_STORAGECONTROL:
								cond = insertStorageControl(&prv->storageControlHandle, (DMAN_STORAGECONTROL *) workRecord);
								break;
		case DMAN_K_SECURITYMATRIX:
								cond = insertSecurityMatrix(&prv->securityMatrixHandle, (DMAN_SECURITYMATRIX *) workRecord);
								break;
		case DMAN_K_GROUPNAMES:
								cond = insertGroupNames(&prv->groupNamesHandle, (DMAN_GROUPNAMES *) workRecord);
								break;
		case DMAN_K_STORAGEACCESS:
								cond = insertStorageAccess(&prv->storageAccessHandle, (DMAN_STORAGEACCESS *) workRecord);
								break;
		case DMAN_K_FISACCESS:
								cond = insertFISAccess(&prv->FISAccessHandle, (DMAN_FISACCESS *) workRecord);
								break;
		case DMAN_K_PRINTSERVERCFG:
								cond = insertPrintServerCFG(&prv->printServerCFGHandle, (DMAN_PRINTSERVERCFG *) workRecord);
								break;
		case DMAN_K_VIDEOIMAGEDEST:
								cond = insertVideoImageDest(&prv->VideoImageDestHandle, (DMAN_VIDEOIMAGEDEST *) workRecord);
								break;
		default:
								break;
    }
    return cond;
}

static CONDITION
insertAE(TBL_HANDLE ** handle, DMAN_APPLICATIONENTITY * workRecord)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[10];
    int			        i;

    i = 0;
    if (workRecord->Flag & DMAN_K_APPLICATION_TITLE) {
    	fields[i].FieldName = "Title";
    	TBL_EXISTING_STRING(&fields[i].Value, workRecord->Title);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_NODE) {
    	fields[i].FieldName = "Node";
    	TBL_EXISTING_STRING(&fields[i].Value, workRecord->Node);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_COMMENT) {
    	fields[i].FieldName = "Comment";
    	TBL_EXISTING_STRING(&fields[i].Value, workRecord->Comment);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_PORT) {
    	TBL_FIELD_LOAD_NUM(fields[i], "Port", workRecord->Port, TBL_SIGNED4);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_APPLICATION_ORGANIZATION) {
    	fields[i].FieldName = "Organization";
    	TBL_EXISTING_STRING(&fields[i].Value, workRecord->Organization);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
insertStorageControl(TBL_HANDLE ** handle, DMAN_STORAGECONTROL * workRecord)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[7];
    int			        i;

    i = 0;
    if (workRecord->Flag & DMAN_K_STORAGECONTROL_REQUESTING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RequestingTitle", workRecord->RequestingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_STORAGECONTROL_RESPONDING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RespondingTitle", workRecord->RespondingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_STORAGECONTROL_MEDIUM) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Medium", workRecord->Medium, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_STORAGECONTROL_ROOT) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Root", workRecord->Root, TBL_STRING);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
insertSecurityMatrix(TBL_HANDLE ** handle, DMAN_SECURITYMATRIX * workRecord)
{
    CONDITION			cond;
    static TBL_FIELD    fields[3];
    int			        i;

    i = 0;
    if (workRecord->Flag & DMAN_K_SECURITY_REQUESTING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RequestingTitle", workRecord->RequestingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_SECURITY_RESPONDING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RespondingTitle", workRecord->RespondingTitle, TBL_STRING);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}


static CONDITION
insertStorageAccess(TBL_HANDLE ** handle, DMAN_STORAGEACCESS * s)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[10];
    int			        i;

    i = 0;
	if (s->Flag & DMAN_K_STORAGEACCESS_TITLE) {
		TBL_FIELD_LOAD_BYTE(fields[i], "Title", s->Title, TBL_STRING);
		i++;
    }
    if (s->Flag & DMAN_K_STORAGEACCESS_DBKEY) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "DbKey", s->DbKey, TBL_STRING);
    	i++;
    }
    if (s->Flag & DMAN_K_STORAGEACCESS_OWNER) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Owner", s->Owner, TBL_STRING);
    	i++;
    }
    if (s->Flag & DMAN_K_STORAGEACCESS_GROUPNAME) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "GroupName", s->GroupName, TBL_STRING);
    	i++;
    }
    if (s->Flag & DMAN_K_STORAGEACCESS_COMMENT) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Comment", s->Comment, TBL_STRING);
    	i++;
    }
    if (s->Flag & DMAN_K_STORAGEACCESS_ACCESS) {
    	TBL_FIELD_LOAD_NUM(fields[i], "Access", s->Access, TBL_SIGNED4);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
insertGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * g)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[3];
    int				    i;

    i = 0;
    if (g->Flag & DMAN_K_GROUP_TITLE) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Title", g->Title, TBL_STRING);
    	i++;
    }
    if (g->Flag & DMAN_K_GROUP_GROUP) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "GroupName", g->GroupName, TBL_STRING);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

#ifdef SMM
static CONDITION
insertGroupNames(TBL_HANDLE ** handle, DMAN_GROUPNAMES * workRecord, DMAN_CRITERIA criteria,
				 DMAN_GROUPNAMES * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx)
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

static CONDITION
insertFISAccess(TBL_HANDLE ** handle, DMAN_FISACCESS * f)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[10];
    int			        i;

    i = 0;
    if (f->Flag & DMAN_K_FISACCESS_TITLE) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Title", f->Title, TBL_STRING);
    	i++;
    }
    if (f->Flag & DMAN_K_FISACCESS_DBKEY) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "DbKey", f->DbKey, TBL_STRING);
    	i++;
    }
    if (f->Flag & DMAN_K_FISACCESS_OWNER) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Owner", f->Owner, TBL_STRING);
    	i++;
    }
    if (f->Flag & DMAN_K_FISACCESS_GROUPNAME) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "GroupName", f->GroupName, TBL_STRING);
    	i++;
    }
    if (f->Flag & DMAN_K_FISACCESS_COMMENT) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "Comment", f->Comment, TBL_STRING);
    	i++;
    }
    if (f->Flag & DMAN_K_FISACCESS_ACCESS) {
    	TBL_FIELD_LOAD_NUM(fields[i], "Access", f->Access, TBL_SIGNED4);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
insertPrintServerCFG(TBL_HANDLE ** handle, DMAN_PRINTSERVERCFG * workRecord)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[4];
    int			        i;

    i = 0;
    if (workRecord->Flag & DMAN_K_PRINTSERVER_REQUESTING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RequestingTitle", workRecord->RequestingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_PRINTSERVER_RESPONDING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RespondingTitle", workRecord->RespondingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_PRINTSERVER_GQID) {
    	TBL_FIELD_LOAD_NUM(fields[i], "GQId", workRecord->GQId, TBL_SIGNED4);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}

static CONDITION
insertVideoImageDest(TBL_HANDLE ** handle, DMAN_VIDEOIMAGEDEST * workRecord)
{
    CONDITION		    cond;
    static TBL_FIELD    fields[4];
    int			        i;

    i = 0;
    if (workRecord->Flag & DMAN_K_VIDEOIMAGE_REQUESTING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RequestingTitle", workRecord->RequestingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_VIDEOIMAGE_RESPONDING) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "RespondingTitle", workRecord->RespondingTitle, TBL_STRING);
    	i++;
    }
    if (workRecord->Flag & DMAN_K_VIDEOIMAGE_IMAGETYPE) {
    	TBL_FIELD_LOAD_BYTE(fields[i], "ImageType", workRecord->ImageType, TBL_STRING);
    	i++;
    }
    fields[i].FieldName = NULL;

    cond = TBL_Insert(handle, fields);
    if (cond == TBL_NORMAL){
    	return DMAN_NORMAL;
    }else{
    	return 0;
    }
}
