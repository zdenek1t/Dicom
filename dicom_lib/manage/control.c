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
** Module Name(s):	DMAN_Open
**			DMAN_Close
**			DMAN_LookupApplication
**			DMAN_VerifyApplication
**			DMAN_ApplicationAccess
**			DMAN_LookupStorage
**			DMAN_StorageAccess
**			DMAN_StorageControl
**			DMAN_TempImageFile
**			DMAN_PermImageFile
**			DMAN_LookupFISAccess
** Author, Date:	Stephen M. Moore, 18-Apr-1994
** Intent:		This file contains a number of control functions
**			that are part of the DMAN facility.  These include
**			the standard open and close functions as well
**			as a number of lookup functions.
** Last Update:		$Author: smm $, $Date: 1999/02/06 16:03:53 $
** Source File:		$RCSfile: control.c,v $
** Revision:		$Revision: 1.29 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.29 $ $RCSfile: control.c,v $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <direct.h>
#include <process.h>
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../uid/dicom_uids.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "../manage/manage.h"
#include "dmanprivate.h"

#define DMAN_ERROR(a) (a), DMAN_Message((a))

#define ACCESS_READ  		1
#define ACCESS_WRITE 		2
#define ACCESS_READWRITE 	3

static CTNBOOLEAN
nodeCompare(const char *node1, const char *node2);
static CONDITION
verifyCreatePath(char *path);

#ifdef _MSC_VER
static void
modifyWindowsFileName(char *name)
{
    while (*name != '\0') {
    	if (*name == '.'){
    		*name = '_';
    	}else if (*name == '\\'){
    		*name = '/';
    	}
    	name++;
    }
}
#endif

CONDITION
DMAN_Open(char *databaseName, char *requestingTitle, char *respondingTitle, DMAN_HANDLE ** handle)
{
    CONDITION			cond;
    PRIVATE_HANDLE		* prv;

    *handle = NULL;
    prv = malloc(sizeof(*prv));
    if (prv == NULL) return COND_PushCondition(DMAN_ERROR(DMAN_MALLOCFAILED), sizeof(*prv), "DMAN_Open");

    (void) strcpy(prv->requestingTitle, requestingTitle);
    (void) strcpy(prv->respondingTitle, respondingTitle);
    prv->storage = NULL;
    prv->storageAccess = FALSE;
    prv->readAccess = FALSE;
    prv->writeAccess = FALSE;

    cond = TBL_Open(databaseName, "ApplicationEntity", &prv->applicationEntityHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "ApplicationEntity", "DMAN_Open");

    cond = TBL_Open(databaseName, "GroupNames", &prv->groupNamesHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "GroupNames", "DMAN_Open");

    cond = TBL_Open(databaseName, "StorageAccess", &prv->storageAccessHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "StorageAccess", "DMAN_Open");

    cond = TBL_Open(databaseName, "StorageControl", &prv->storageControlHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "StorageControl", "DMAN_Open");

    cond = TBL_Open(databaseName, "SecurityMatrix", &prv->securityMatrixHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "SecurityMatrix", "DMAN_Open");
#ifdef FIS
    cond = TBL_Open(databaseName, "FISAccess", &prv->FISAccessHandle);
    if (cond != TBL_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "FISAccess", "DMAN_Open");
#endif
/*ZT
    cond = TBL_Open(databaseName, "PrintServerCFG", &prv->printServerCFGHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "PrintServerCFG", "DMAN_Open");

    cond = TBL_Open(databaseName, "VideoImageDest", &prv->VideoImageDestHandle);
    if (cond != TBL_NORMAL)	return COND_PushCondition(DMAN_ERROR(DMAN_TABLEOPENFAILED), "VideoImageDest", "DMAN_Open");
*/
    *handle = (DMAN_HANDLE *) prv;
    return DMAN_NORMAL;
}

CONDITION
DMAN_Close(DMAN_HANDLE ** handle)
{
    PRIVATE_HANDLE		** prv;

    prv = (PRIVATE_HANDLE **) handle;
    if (*prv == NULL) return COND_PushCondition(DMAN_ERROR(DMAN_ILLEGALHANDLE), "DMAN_Close");
    (void) TBL_Close(&(*prv)->applicationEntityHandle);
    (void) TBL_Close(&(*prv)->groupNamesHandle);
    (void) TBL_Close(&(*prv)->storageAccessHandle);
    (void) TBL_Close(&(*prv)->storageControlHandle);
    (void) TBL_Close(&(*prv)->securityMatrixHandle);
#ifdef FIS
    (void) TBL_Close(&(*prv)->FISAccessHandle);
#endif
    /*ZT
    (void) TBL_Close(&(*prv)->printServerCFGHandle);
    (void) TBL_Close(&(*prv)->VideoImageDestHandle);
*/
    if ((*prv)->storage != NULL) free((*prv)->storage);

    free(*prv);
    *handle = NULL;
    return DMAN_NORMAL;
}

CONDITION
DMAN_LookupApplication(DMAN_HANDLE ** handle, char *title, DMAN_APPLICATIONENTITY * ae)
{
    CONDITION					cond;
    DMAN_APPLICATIONENTITY		aeCriteria;
    PRIVATE_HANDLE				** prv;
    long				        count;

    prv = (PRIVATE_HANDLE **) handle;

    aeCriteria.Type = DMAN_K_APPLICATIONENTITY;
    strcpy(aeCriteria.Title, title);
    aeCriteria.Flag = DMAN_K_APPLICATION_TITLE;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) ae, (DMAN_GENERICRECORD *) & aeCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONLOOKUPFAILED), title, "DMAN_LookupApplication");
    if (count != 1){
    	(void) COND_PushCondition(DMAN_ERROR(DMAN_TITLENOTFOUND), title, "DMAN_LookupApplication");
    	return COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONLOOKUPFAILED), title, "DMAN_LookupApplication");
    }
    return DMAN_NORMAL;
}

CONDITION
DMAN_VerifyApplication(DMAN_HANDLE ** handle, char *title, char *node)
{
    CONDITION					cond;
    DMAN_APPLICATIONENTITY		aeWork,	aeCriteria;
    PRIVATE_HANDLE				** prv;
    long				        count;

    prv = (PRIVATE_HANDLE **) handle;

    aeWork.Type = aeCriteria.Type = DMAN_K_APPLICATIONENTITY;
    strcpy(aeCriteria.Title, title);
    aeCriteria.Flag = DMAN_K_APPLICATION_TITLE;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & aeWork, (DMAN_GENERICRECORD *) & aeCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL)
    	return COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONVERIFICATIONFAILED), title, node, "DMAN_VerifyApplication");

    if (count != 1){
    	(void) COND_PushCondition(DMAN_ERROR(DMAN_TITLENOTFOUND), title, "DMAN_VerifyApplication");
    	return COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONVERIFICATIONFAILED), title, node, "DMAN_VerifyApplication");
    }

    if (!nodeCompare(node, aeWork.Node)){
    	if(strcmp(aeWork.Node, "*") == 0){
    		strcpy(aeWork.Node, node);
    	}else{
    		(void) COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONNODEMISMATCH), title, node, aeWork.Node, "DMAN_VerifyApplication");
    		return COND_PushCondition(DMAN_ERROR(DMAN_APPLICATIONVERIFICATIONFAILED), title, node, "DMAN_VerifyApplication");
    	}
    }
    return DMAN_NORMAL;
}

CONDITION
DMAN_ApplicationAccess(DMAN_HANDLE ** handle, const char *requestingTitle, const char *respondingTitle, CTNBOOLEAN * access)
{
    CONDITION				cond;
    DMAN_SECURITYMATRIX		matrixWork, matrixCriteria;
    PRIVATE_HANDLE			** prv;
    long			        count;

    prv = (PRIVATE_HANDLE **) handle;
    *access = FALSE;

    matrixWork.Type = matrixCriteria.Type = DMAN_K_SECURITYMATRIX;
    strcpy(matrixCriteria.RequestingTitle, requestingTitle);
    strcpy(matrixCriteria.RespondingTitle, respondingTitle);
    matrixCriteria.Flag = DMAN_K_SECURITY_REQUESTING |	DMAN_K_SECURITY_RESPONDING;

    cond = DMAN_Select(handle,(DMAN_GENERICRECORD *) & matrixWork, (DMAN_GENERICRECORD *) & matrixCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_ILLEGALCONNECTION), requestingTitle, respondingTitle, "DMAN_ApplicationAccess");
    if (count == 1)	*access = TRUE;

    return DMAN_NORMAL;
}

CONDITION
DMAN_LookupStorage(DMAN_HANDLE ** handle, char *applicationTitle, DMAN_STORAGEACCESS * s)
{
    CONDITION 				cond;
    DMAN_STORAGEACCESS		storageCriteria;
    PRIVATE_HANDLE 			**prv;
    long			        count;

    prv = (PRIVATE_HANDLE **) handle;

    s->Type = storageCriteria.Type = DMAN_K_STORAGEACCESS;
    strcpy(storageCriteria.Title, applicationTitle);
    storageCriteria.Flag = DMAN_K_STORAGEACCESS_TITLE;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) s, (DMAN_GENERICRECORD *) & storageCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL || count != 1) return COND_PushCondition(DMAN_ERROR(DMAN_STORAGELOOKUPFAILED), applicationTitle, "DMAN_LookupStorage");

    return DMAN_NORMAL;
}

CONDITION
DMAN_StorageAccess(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, CTNBOOLEAN * readAccess, CTNBOOLEAN * writeAccess)
{
    CONDITION				cond;
    DMAN_STORAGEACCESS		databaseWork, databaseCriteria;
    DMAN_GROUPNAMES			groupWork, groupCriteria;
    PRIVATE_HANDLE			** prv;
    long			        count;
    int					    accessFlag = 0, accessRight = 0;

    prv = (PRIVATE_HANDLE **) handle;

    if ((*prv)->storageAccess && strcmp((*prv)->requestingTitle, requestingTitle) == 0 && strcmp((*prv)->respondingTitle, respondingTitle) == 0) {
    	*readAccess  = (*prv)->readAccess;
    	*writeAccess = (*prv)->writeAccess;
    	return DMAN_NORMAL;
    }
    *readAccess = *writeAccess = FALSE;

    databaseWork.Type = databaseCriteria.Type = DMAN_K_STORAGEACCESS;
    strcpy(databaseCriteria.Title, respondingTitle);

    cond = DMAN_Select(handle,(DMAN_GENERICRECORD *) & databaseWork,(DMAN_GENERICRECORD *) & databaseCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL)
    	return COND_PushCondition(DMAN_ERROR(DMAN_STORAGEACCESSDENIED), requestingTitle, respondingTitle, "DMAN_StorageAccess");
    if (count == 0) {
    	(*prv)->storageAccess = TRUE;
    	return DMAN_NORMAL;
    }

    accessRight = databaseWork.Access;

    /* If the calling application is the owner of the database, turn on access rights for the owner. */
    if (accessRight > 99){ // Owner access
    	if (strcmp(requestingTitle, databaseWork.Owner) == 0) accessFlag |= accessRight / 100;
    	accessRight = databaseWork.Access % 100;
    }

    /* If we still don't have all of the access rights we might want to
    ** have, see if the requesting application is in the group and turn
    ** on access for that group.
    */
    if (accessRight > 9){ // Group access
    	groupWork.Type = groupCriteria.Type = DMAN_K_GROUPNAMES;
    	strcpy(groupCriteria.Title, requestingTitle);
    	strcpy(groupCriteria.GroupName, databaseWork.GroupName);

    	cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & groupWork, (DMAN_GENERICRECORD *) & groupCriteria, NULL, NULL, &count, NULL);
    	if (cond != DMAN_NORMAL)
    		return COND_PushCondition(DMAN_ERROR(DMAN_STORAGEACCESSDENIED), requestingTitle, respondingTitle, "DMAN_StorageAccess");
   		if (count == 1) accessFlag |= accessRight / 10;
			accessRight = databaseWork.Access % 10;
    }

    /* If all access is not turned on, try the access for anyone  */
    accessFlag |= accessRight;

    (*prv)->storageAccess = TRUE;

    if (accessFlag == ACCESS_READ){
    	*readAccess  = TRUE;
    	*writeAccess = FALSE;
	}else if (accessFlag == ACCESS_WRITE){
    	*readAccess  = FALSE;
    	*writeAccess = TRUE;
    }else if (accessFlag == ACCESS_READWRITE){
    	*readAccess  = TRUE;
    	*writeAccess = TRUE;
    }else{
    	*readAccess  = FALSE;
    	*writeAccess = FALSE;
        (*prv)->storageAccess = FALSE;
    }

    return DMAN_NORMAL;
}
CONDITION
DMAN_StorageControl(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, DMAN_STORAGECONTROL * storage)
{
    CONDITION				cond;
    DMAN_STORAGECONTROL		criteria;
    PRIVATE_HANDLE			** prv;
    long			        count;

    prv = (PRIVATE_HANDLE **) handle;

    storage->Type = criteria.Type = DMAN_K_STORAGECONTROL;
    strcpy(criteria.RequestingTitle, requestingTitle);
    strcpy(criteria.RespondingTitle, respondingTitle);
    criteria.Flag = DMAN_K_STORAGECONTROL_REQUESTING | DMAN_K_STORAGECONTROL_RESPONDING;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) storage, (DMAN_GENERICRECORD *) & criteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL) return 0;
    if (count != 1){
    	fprintf(stderr,"Not found %s/%s in StorageControl\n",requestingTitle,respondingTitle);
    	return 0;
    };

    return DMAN_NORMAL;
}

typedef struct {
    char 	*label;
    char 	*SOP;
}   MAP;

static MAP map[] = {
    {"CR", 	DICOM_SOPCLASSCOMPUTEDRADIOGRAPHY},
    {"CT", 	DICOM_SOPCLASSCT},
    {"USM", DICOM_SOPCLASSUSMULTIFRAMEIMAGE},
    {"MR", 	DICOM_SOPCLASSMR},
    {"NM", 	DICOM_SOPCLASSNM},
    {"US", 	DICOM_SOPCLASSUS},
    {"SC", 	DICOM_SOPCLASSSECONDARYCAPTURE},
    {"XRA", DICOM_SOPCLASSXRAYANGIO},
    {"XRF", DICOM_SOPCLASSXRAYFLUORO}
};

CONDITION
DMAN_TempImageFile(DMAN_HANDLE ** handle, char *SOPClass, char *rtnFileName, size_t fileNameLength)
{
    PRIVATE_HANDLE		** prv;
    CONDITION			cond;
    char		        scratch[1024], scratch1[1024], *lbl;
    int			        i;
    static int	        count = 0;

    prv = (PRIVATE_HANDLE **) handle;
    if ((*prv)->storage == NULL) {
    	(*prv)->storage = malloc(sizeof(*(*prv)->storage));
    	if ((*prv)->storage == NULL) return COND_PushCondition(DMAN_ERROR(DMAN_MALLOCFAILED), sizeof(*(*prv)->storage), "DMAN_TempImageFile");
    	(*prv)->storage->RequestingTitle[0] = '\0';
    	(*prv)->storage->RespondingTitle[0] = '\0';
    }

    if ((strcmp((*prv)->storage->RequestingTitle, (*prv)->requestingTitle) != 0) ||	(strcmp((*prv)->storage->RespondingTitle, (*prv)->respondingTitle) != 0)) {
    	cond = DMAN_StorageControl(handle, (*prv)->requestingTitle, (*prv)->respondingTitle, (*prv)->storage);
    	if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_TempImageFile");
    }
    strcpy(scratch, (*prv)->storage->Root);

    for (i = 0; i < DIM_OF(map); i++){
    	if (strcmp(map[i].SOP, SOPClass) == 0) break;
    }
    if (i != DIM_OF(map)){
    	lbl = map[i].label;
    }else{
    	lbl = "OT";
    }
    sprintf(scratch1, "/%s", lbl);
    strcat(scratch, scratch1);
#ifdef _MSC_VER
    modifyWindowsFileName(scratch);
#endif
    cond = verifyCreatePath(scratch);
    if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_TempImageFile");

#ifdef _MSC_VER
    i = _getpid();
#else
    i = getpid();
#endif

    sprintf(scratch1, "/%s.%-d.%-d", lbl, i, ++count);
    strcat(scratch, scratch1);

    if (strlen(scratch) >= fileNameLength) {
    	(void) COND_PushCondition(DMAN_ERROR(DMAN_FILENAMETOOLONG), strlen(scratch), fileNameLength, "DMAN_TempImageFile");
    	return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_TempImageFile");
    }
    strcpy(rtnFileName, scratch);
#ifdef _MSC_VER
    modifyWindowsFileName(rtnFileName);
    strcat(rtnFileName, ".dcm");
#endif

    return DMAN_NORMAL;
}

CONDITION
DMAN_PermImageFile(DMAN_HANDLE ** handle, char *SOPClass, const char *study, const char *series, char *rtnFileName, size_t fileNameLength)
{
    PRIVATE_HANDLE		** prv;
    CONDITION			cond;
    char		        scratch[1024], scratch1[1024], *lbl;
    int			        i;
    static int	        count = 0;

    prv = (PRIVATE_HANDLE **) handle;

    if ((*prv)->storage == NULL) {
    	(*prv)->storage = malloc(sizeof(*(*prv)->storage));
    	if ((*prv)->storage == NULL) return COND_PushCondition(DMAN_ERROR(DMAN_MALLOCFAILED), sizeof(*(*prv)->storage), "DMAN_PermImageFile");

    	(*prv)->storage->RequestingTitle[0] = '\0';
    	(*prv)->storage->RespondingTitle[0] = '\0';
    }

    if ((strcmp((*prv)->storage->RequestingTitle, (*prv)->requestingTitle) != 0) || (strcmp((*prv)->storage->RespondingTitle, (*prv)->respondingTitle) != 0)) {
    	cond = DMAN_StorageControl(handle, (*prv)->requestingTitle, (*prv)->respondingTitle, (*prv)->storage);
    	if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_PermImageFile");
    }
    strcpy(scratch, (*prv)->storage->Root);

    for (i = 0; i < DIM_OF(map); i++) {
    	if (strcmp(map[i].SOP, SOPClass) == 0) break;
    }
    if (i != DIM_OF(map)){
    	lbl = map[i].label;
    }else{
    	lbl = "OT";
    }
    sprintf(scratch1, "/%s/%s", study, series);
    strcat(scratch, scratch1);

#ifdef _MSC_VER
    modifyWindowsFileName(scratch);
#endif

    cond = verifyCreatePath(scratch);
    if (cond != DMAN_NORMAL) return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_PermImageFile");

    i = getpid();
    sprintf(scratch1, "/%s.%-d.%-d", lbl, i, ++count);
    strcat(scratch, scratch1);

    if (strlen(scratch) >= fileNameLength) {
    	(void) COND_PushCondition(DMAN_ERROR(DMAN_FILENAMETOOLONG), strlen(scratch), fileNameLength, "DMAN_PermImageFile");
    	return COND_PushCondition(DMAN_ERROR(DMAN_FILEGENERATIONFAILED), (*prv)->requestingTitle, (*prv)->respondingTitle, "DMAN_PermImageFile");
    }

    strcpy(rtnFileName, scratch);
#ifdef _MSC_VER
    modifyWindowsFileName(rtnFileName);
    strcat(rtnFileName, ".dcm");
#endif
    return DMAN_NORMAL;
}

CONDITION
DMAN_LookupFISAccess(DMAN_HANDLE ** handle, char *applicationTitle, DMAN_FISACCESS * f)
{
    CONDITION 			cond;
    DMAN_FISACCESS		criteria;
    PRIVATE_HANDLE 		**prv;
    long		        count;

    prv = (PRIVATE_HANDLE **) handle;

    f->Type = criteria.Type = DMAN_K_FISACCESS;
    strcpy(criteria.Title, applicationTitle);
    criteria.Flag = DMAN_K_FISACCESS_TITLE;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) f, (DMAN_GENERICRECORD *) & criteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL || count != 1) return COND_PushCondition(DMAN_ERROR(DMAN_FISACCESSLOOKUPFAILED), applicationTitle, "DMAN_LookupFISAccess");

    return DMAN_NORMAL;
}


/*----------------------------------------------------------------------
** Private functions defined below this point.
*/
#ifndef _MSC_VER
struct hostent *gethostbyname();
#endif

static struct hostent *
localHostEnt(const char *node)
{
    CTNBOOLEAN 		isIPAddress = TRUE;
    int 			index = 0;
    struct hostent 	*h;

    for (index = 0; (node[index] != '\0') && isIPAddress; index++) {
    	if (!isdigit(node[index]) && !(node[index] == '.')) {
    		isIPAddress = FALSE;
    		break;
    	}
    }

    if (isIPAddress){
    	unsigned long 	ipAddress;

    	ipAddress = inet_addr(node);
    	h = gethostbyaddr((char *) &ipAddress, 4, PF_INET);
    }else{
    	h = gethostbyname(node);
    }
    return h;
}

static CTNBOOLEAN
nodeCompare(const char *node1, const char *node2)
{
    struct hostent 	*h;

#ifdef _MSC_VER
    char 	fullName1[512 + 1];
#else
    char 	fullName1[MAXHOSTNAMELEN + 1];
#endif

    if (strcmp(node1, node2) == 0) return TRUE;

    h = localHostEnt(node1);
    if (h == NULL) return FALSE;

    strcpy(fullName1, h->h_name);

    h = localHostEnt(node2);
    if (h == NULL) return FALSE;

    return (strcmp(fullName1, h->h_name) == 0);
}

/*ZT
static void
translateAccess(const char *accessString, int *access)
{
    *access = 0;

    if (accessString[0] == 'r' || accessString[0] == 'R')
		*access |= ACCESS_READ;

    if (accessString[1] == 'w' || accessString[1] == 'W')
		*access |= ACCESS_WRITE;
}*/

static CONDITION
verifyCreatePath(char *path)
{
    int 			i;
#ifdef _MSC_VER
    struct _stat 	buf;
#else
    struct stat 	buf;
#endif
    char		    *p, temp[1024];
    int 			flag = 0;

#ifdef _MSC_VER
    statCount++;
    i = _stat(path, &buf);
#else
    i = stat(path, &buf);
#endif


    if (i == 0) {
#ifdef _MSC_VER
    	flag = ((buf.st_mode & _S_IFDIR) != 0);
#else
    	flag = (S_ISDIR(buf.st_mode));
#endif
    	if (flag){
    		return DMAN_NORMAL;
    	}else{
    		return COND_PushCondition(DMAN_ERROR(DMAN_PATHNOTDIR), path);
    	}
    }
    p = temp;

    while (*path != '\0') {
    	*p++ = *path++;

    	while (*path != '/' && *path != '\\' && *path != '\0') {
#ifdef _MSC_VER
    		if (*path == ':') {
    			*p++ = *path++;
    			if (*path == '\0') break;	/* We should not get C:\0, but test it */
    		}
#endif
    		*p++ = *path++;
    	}

    	*p = '\0';
#ifdef _MSC_VER
    	statCount++;
    	i = _stat(temp, &buf);
#else
    	i = stat(temp, &buf);
#endif

    	if (i == 0) {
#ifdef _MSC_VER
    		flag = ((buf.st_mode & _S_IFDIR) != 0);
#else
    		flag = (S_ISDIR(buf.st_mode));
#endif
    		if (!flag) return COND_PushCondition(DMAN_ERROR(DMAN_PATHNOTDIR), temp);
    	}else{
#ifdef _MSC_VER
    		int e1;
    		e1 = errno;
    		memset(&buf, 0, sizeof(buf));
    		fprintf(stderr, "Stat Count = %d\n", statCount);
    		statCount++;
    		i = _stat(temp, &buf);
    		e1 = errno;
    		i = _mkdir(temp);
#else
    		i = mkdir(temp, 0777);
#endif
    		if (i != 0) {
    			int e1;
    			e1 = errno;
    			perror(temp);
    			return COND_PushCondition(DMAN_ERROR(DMAN_FILECREATEFAILED), temp, "DIRECTORY", "verifyCreatePath (DMAN)");
    		}
    	}
    }
    return DMAN_NORMAL;
}

CONDITION
DMAN_VerifyPrintServerCFG(DMAN_HANDLE ** handle, char *requestingTitle, char *respondingTitle, int gqID, CTNBOOLEAN * access)
{
    CONDITION				cond;
    DMAN_PRINTSERVERCFG		cfgWork, cfgCriteria;
    PRIVATE_HANDLE			** prv;
    long			        count;

    prv = (PRIVATE_HANDLE **) handle;
    *access = FALSE;

    cfgWork.Type = cfgCriteria.Type = DMAN_K_PRINTSERVERCFG;
    strcpy(cfgCriteria.RequestingTitle, requestingTitle);
    strcpy(cfgCriteria.RespondingTitle, respondingTitle);
    cfgCriteria.GQId = gqID;
    cfgCriteria.Flag = DMAN_K_PRINTSERVER_REQUESTING | DMAN_K_PRINTSERVER_RESPONDING | DMAN_K_PRINTSERVER_GQID;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & cfgWork, (DMAN_GENERICRECORD *) & cfgCriteria, NULL, NULL, &count, NULL);
    if (cond != DMAN_NORMAL) {
    	char	    strOfGQId[10];

    	sprintf(strOfGQId, "%d", gqID);
    	return COND_PushCondition(DMAN_ERROR(DMAN_ILLEGALPRINTSERVERCONFIGURATION), requestingTitle, respondingTitle, strOfGQId, "DMAN_VerifyPrintServerCFG");
    }
    if (count == 1)	*access = TRUE;

    return DMAN_NORMAL;
}

CONDITION
DMAN_SelectImageDestinations(DMAN_HANDLE ** handle, const char *srcApplication, LST_HEAD * lst)
{
    CONDITION 				cond;
    DMAN_VIDEOIMAGEDEST 	imageDestination;
    DMAN_VIDEOIMAGEDEST 	workRecord;
    PRIVATE_HANDLE 			**prv;
    long 					count;

    prv = (PRIVATE_HANDLE **) handle;

    memset(&imageDestination, 0, sizeof(imageDestination));
    imageDestination.Type = DMAN_K_VIDEOIMAGEDEST;
    workRecord = imageDestination;

    strcpy(imageDestination.RequestingTitle, srcApplication);
    imageDestination.Flag = DMAN_K_VIDEOIMAGE_REQUESTING;

    cond = DMAN_Select(handle, (DMAN_GENERICRECORD *) & workRecord, (DMAN_GENERICRECORD *) & imageDestination, lst, NULL, &count, NULL);
    if (cond != DMAN_NORMAL) return cond;		/* repair */

    return DMAN_NORMAL;
}


CONDITION
DMAN_ClearList(LST_HEAD * lst)
{
    LST_NODE	* node;

    if (lst != NULL) {
    	while ((node = LST_Dequeue(&lst)) != NULL)
    		free(node);
    }
    return DMAN_NORMAL;
}
