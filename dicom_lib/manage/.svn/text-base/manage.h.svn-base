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
** Module Name(s):	include file
** Author, Date:	Steve Moore
** Intent:		Define typedefs and function prototypes for
**			DMAN facility (for handling general query operations).
** Last Update:		$Author: smm $, $Date: 1998/10/22 17:41:27 $
** Source File:		$RCSfile: manage.h,v $
** Revision:		$Revision: 1.21 $
** Status:		$State: Exp $
*/


#ifndef _DMAN_IS_IN
#define _DMAN_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif


typedef enum {
    DMAN_K_APPLICATIONENTITY,
    DMAN_K_GROUPNAMES,
    DMAN_K_STORAGEACCESS,
    DMAN_K_STORAGECONTROL,
    DMAN_K_SECURITYMATRIX,
    DMAN_K_FISACCESS,
    DMAN_K_PRINTSERVERCFG,
    DMAN_K_VIDEOIMAGEDEST
}   DMAN_DATATYPE;

#define	DMAN_K_APPLICATION_TITLE		(1 << 0)
#define	DMAN_K_APPLICATION_NODE			(1 << 1)
#define	DMAN_K_APPLICATION_COMMENT		(1 << 2)
#define	DMAN_K_APPLICATION_PORT			(1 << 3)
#define	DMAN_K_APPLICATION_ORGANIZATION	(1 << 4)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			Title[17];
    char 			Node[65];
    char 			Comment[81];
    int 			Port;
    char			Organization[33];
}   DMAN_APPLICATIONENTITY;

#define	DMAN_K_GROUP_TITLE	(1 << 0)
#define	DMAN_K_GROUP_GROUP	(1 << 1)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			Title[17];
    char 			GroupName[17];
}   DMAN_GROUPNAMES;

#define	DMAN_K_STORAGEACCESS_ACCESS		(1 << 0)
#define	DMAN_K_STORAGEACCESS_TITLE		(1 << 1)
#define	DMAN_K_STORAGEACCESS_DBKEY		(1 << 2)
#define	DMAN_K_STORAGEACCESS_OWNER		(1 << 3)
#define	DMAN_K_STORAGEACCESS_GROUPNAME	(1 << 4)
#define	DMAN_K_STORAGEACCESS_COMMENT	(1 << 5)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    S32 			Access;
    char 			Title[17];
    char 			DbKey[65];
    char 			Owner[17];
    char 			GroupName[17];
    char 			Comment[81];
}   DMAN_STORAGEACCESS;

#define	DMAN_K_STORAGECONTROL_REQUESTING	(1 << 0)
#define	DMAN_K_STORAGECONTROL_RESPONDING	(1 << 1)
#define	DMAN_K_STORAGECONTROL_MEDIUM		(1 << 2)
#define	DMAN_K_STORAGECONTROL_ROOT			(1 << 3)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			RequestingTitle[17];
    char 			RespondingTitle[17];
    char 			Medium[33];
    char 			Root[256];
}   DMAN_STORAGECONTROL;

#define	DMAN_K_SECURITY_REQUESTING	(1 << 0)
#define	DMAN_K_SECURITY_RESPONDING	(1 << 1)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			RequestingTitle[17];
    char 			RespondingTitle[17];
}   DMAN_SECURITYMATRIX;

#define	DMAN_K_FISACCESS_ACCESS		(1 << 0)
#define	DMAN_K_FISACCESS_TITLE		(1 << 1)
#define	DMAN_K_FISACCESS_DBKEY		(1 << 2)
#define	DMAN_K_FISACCESS_OWNER		(1 << 3)
#define	DMAN_K_FISACCESS_GROUPNAME	(1 << 4)
#define	DMAN_K_FISACCESS_COMMENT	(1 << 5)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    S32 			Access;
    char 			Title[17];
    char 			DbKey[65];
    char 			Owner[17];
    char 			GroupName[17];
    char 			Comment[81];
}   DMAN_FISACCESS;

#define	DMAN_K_PRINTSERVER_REQUESTING	(1 << 0)
#define	DMAN_K_PRINTSERVER_RESPONDING	(1 << 1)
#define DMAN_K_PRINTSERVER_GQID			(1 << 2)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			RequestingTitle[17];
    char 			RespondingTitle[17];
    int 			GQId;
}   DMAN_PRINTSERVERCFG;

#define	DMAN_K_VIDEOIMAGE_REQUESTING	(1 << 0)
#define	DMAN_K_VIDEOIMAGE_RESPONDING	(1 << 1)
#define DMAN_K_VIDEOIMAGE_IMAGETYPE		(1 << 2)

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
    int 			Flag;
    char 			RequestingTitle[17];
    char 			RespondingTitle[17];
    char 			ImageType[7];
}   DMAN_VIDEOIMAGEDEST;

typedef struct {
    void 			*reserved[2];
    DMAN_DATATYPE 	Type;
}   DMAN_GENERICRECORD;

typedef void DMAN_HANDLE;

/* Define the function prototypes for this facility. */

CONDITION
DMAN_Open(char *databaseName, char *requestingTitle, char *respondingTitle, DMAN_HANDLE ** handle);
CONDITION
DMAN_Close(DMAN_HANDLE ** handle);
CONDITION
DMAN_Select(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord, DMAN_GENERICRECORD * criteriaRecord, LST_HEAD * head, CONDITION(*callback) (), long *count, void *ctx);
CONDITION
DMAN_Insert(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord);
CONDITION
DMAN_Delete(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord);
/*CONDITION
DMAN_Set(DMAN_HANDLE ** handle, DMAN_GENERICRECORD * workRecord, DMAN_CRITERIA criteria, DMAN_GENERICRECORD * criteriaRecord);
*/

/* Define a higher level of functions that provide specific services for the application. */

CONDITION
DMAN_LookupApplication(DMAN_HANDLE ** handle, char *title, DMAN_APPLICATIONENTITY * ae);
CONDITION
DMAN_VerifyApplication(DMAN_HANDLE ** handle, char *title, char *node);
CONDITION
DMAN_ApplicationAccess(DMAN_HANDLE ** handle, const char *requestingTitle, const char *respondingTitle, CTNBOOLEAN * accessFlag);
CONDITION
DMAN_LookupStorage(DMAN_HANDLE ** handle, char *applicationTitle, DMAN_STORAGEACCESS * storage);
CONDITION
DMAN_StorageControl(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, DMAN_STORAGECONTROL * control);
CONDITION
DMAN_StorageAccess(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, CTNBOOLEAN * readAccess, CTNBOOLEAN * writeAccess);
CONDITION
DMAN_TempImageFile(DMAN_HANDLE ** handle, char *SOPClass, char *rtnFileName, size_t fileNameLength);
CONDITION
DMAN_PermImageFile(DMAN_HANDLE ** handle, char *SOPClass, const char *study, const char* series, char *rtnFileName, size_t fileNameLength);
CONDITION
DMAN_VerifyPrintServerCFG(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, int gqID, CTNBOOLEAN * accessFlag);
CONDITION
DMAN_LookupFISAccess(DMAN_HANDLE ** handle, char *applicationTitle, DMAN_FISACCESS * fis);
CONDITION
DMAN_ClearList(LST_HEAD * lst);
CONDITION
DMAN_SelectImageDestinations(DMAN_HANDLE **handle, const char* srcApplication, LST_HEAD *lst);

char
*DMAN_Message(CONDITION condition);


/* Define condition values */

#define	DMAN_NORMAL	FORM_COND(FAC_DMAN, SEV_SUCC, 1)
#define	DMAN_UNIMPLEMENTED	FORM_COND(FAC_DMAN, SEV_ERROR, 2)
#define	DMAN_MALLOCFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 3)
#define	DMAN_TABLEOPENFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 4)
#define	DMAN_APPLICATIONVERIFICATIONFAILED FORM_COND(FAC_DMAN, SEV_ERROR, 5)
#define	DMAN_APPLICATIONNODEMISMATCH FORM_COND(FAC_DMAN, SEV_ERROR, 6)
#define	DMAN_TITLENOTFOUND	FORM_COND(FAC_DMAN, SEV_ERROR, 7)
#define	DMAN_ILLEGALCONNECTION	FORM_COND(FAC_DMAN, SEV_ERROR, 8)
#define	DMAN_STORAGEACCESSDENIED	FORM_COND(FAC_DMAN, SEV_ERROR, 9)
#define	DMAN_FILEGENERATIONFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 10)
#define	DMAN_FILENAMETOOLONG	FORM_COND(FAC_DMAN, SEV_ERROR, 11)
#define	DMAN_PATHNOTDIR		FORM_COND(FAC_DMAN, SEV_ERROR, 12)
#define	DMAN_FILECREATEFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 13)
#define	DMAN_APPLICATIONLOOKUPFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 14)
#define	DMAN_STORAGELOOKUPFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 15)
#define DMAN_ILLEGALPRINTSERVERCONFIGURATION	FORM_COND(FAC_DMAN, SEV_ERROR, 16)
#define	DMAN_FISACCESSLOOKUPFAILED	FORM_COND(FAC_DMAN, SEV_ERROR, 17)
#define	DMAN_ILLEGALHANDLE	FORM_COND(FAC_DMAN, SEV_ERROR, 18)

#ifdef  __cplusplus
}
#endif

#endif
