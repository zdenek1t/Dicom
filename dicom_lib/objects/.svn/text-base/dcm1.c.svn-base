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
**	private functions
** Author, Date:	Stephen M. Moore, 7-Jun-95
** Intent:
** Last Update:		$Author: smm $, $Date: 1998/07/31 19:39:09 $
** Source File:		$RCSfile: dcm1.c,v $
** Revision:		$Revision: 1.5 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.5 $ $RCSfile: dcm1.c,v $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#ifdef GCCSUNOS
#include <unistd.h>
#endif

#ifdef MACOS
#include <files.h>
#elif defined _MSC_VER
#else
#include <sys/file.h>
#endif

#ifdef SOLARIS
#include <sys/fcntl.h>
#endif
#ifdef MACOS
#include <fcntl.h>
#endif
#include <sys/types.h>
#ifdef MACOS
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../uid/dicom_uids.h"
#include "../objects/dicom_objects.h"
#include "dcmprivate.h"


/* checkObject
**
** Purpose:
**	Examine a PRIVATE OBJECT to see if it looks like is has the proper
**	fields defined.  This function is used to try to make certain that
**	users call the DCM routines with the proper objects.  If the object
**	is legal, the function returns DCM_NORMAL.  If the object is not
**	legal, the function will return an error.
**
** Parameter Dictionary:
**	object		PRIVATE_OBJECT to be examined by this function
**	caller		Name of the function (ASCIZ) that called this
**			function.  In case of failure, this becomes part of
**			the error message that is pushed on the stack.
**
** Return Values:
**	DCM_NORMAL
**	DCM_NULLOBJECT
**	DCM_ILLEGALOBJECT
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
checkObject(PRIVATE_OBJECT ** object, char *caller)
{
    if (object == NULL)
    	return COND_PushCondition(DCM_NULLOBJECT, DCM_Message(DCM_NULLOBJECT), caller);
    if (*object == NULL)
    	return COND_PushCondition(DCM_NULLOBJECT, DCM_Message(DCM_NULLOBJECT), caller);
    if (strcmp((*object)->keyType, KEY_DCM_OBJECT) != 0)
    	return COND_PushCondition(DCM_ILLEGALOBJECT, DCM_Message(DCM_ILLEGALOBJECT), caller);
    return DCM_NORMAL;
}

/*  Define the public interfaces
*/

static DCM_FILE_META meta;
DCM_ELEMENT metaRequired[] = {
    {DCM_METAINFORMATIONVERSION, DCM_OB, "", 1,	sizeof(meta.fileMetaInformationVersion), (void *) meta.fileMetaInformationVersion},
    {DCM_METAMEDIASTORAGESOPCLASS, DCM_UI, "", 1, sizeof(meta.mediaStorageSOPClassUID), (void *) meta.mediaStorageSOPClassUID},
    {DCM_METAMEDIASTORAGESOPINSTANCE, DCM_UI, "", 1, sizeof(meta.mediaStorageSOPInstanceUID), (void *) meta.mediaStorageSOPInstanceUID},
    {DCM_METATRANSFERSYNTAX, DCM_UI, "", 1,	sizeof(meta.transferSyntaxUID), (void *) meta.transferSyntaxUID},
    {DCM_METAIMPLEMENTATIONCLASS, DCM_UI, "", 1, sizeof(meta.implementationClassUID), (void *) meta.implementationClassUID}
};

DCM_FLAGGED_ELEMENT metaOptional[] = {
    {DCM_METAIMPLEMENTATIONVERSION, DCM_SH, "", 1, sizeof(meta.implementationVersionName), (void *) meta.implementationVersionName,	DCM_FILEMETA_IMPLEMENTATIONVERSIONNAME, &meta.flag},
    {DCM_METASOURCEAETITLE, DCM_AE, "", 1, sizeof(meta.sourceApplicationEntityTitle), meta.sourceApplicationEntityTitle, DCM_FILEMETA_SOURCEAPPLICATIONENTITYTITLE, &meta.flag},
    {DCM_METAPRIVATEINFORMATIONCREATOR, DCM_UI, "", 1, sizeof(meta.privateInformationCreatorUID), (void *) meta.privateInformationCreatorUID, DCM_FILEMETA_PRIVATEINFORMATIONCREATORUID, &meta.flag}
};

CONDITION
DCM_GetFileMeta(DCM_OBJECT ** callerObject, DCM_FILE_META ** fileMeta)
{
    CONDITION 		cond;
    PRIVATE_OBJECT 	**object;

    object = (PRIVATE_OBJECT **) callerObject;
    cond = checkObject(object, "DCM_GetFileMeta");
    if (cond != DCM_NORMAL)	return cond;

    memset(&meta, 0, sizeof(meta));

    cond = DCM_ParseObject(callerObject, metaRequired, (int) DIM_OF(metaRequired), metaOptional, (int) DIM_OF(metaOptional), NULL);
    if (cond != DCM_NORMAL)	return cond;		/* repair */

    *fileMeta = CTN_MALLOC(sizeof(DCM_FILE_META));
    if (*fileMeta == NULL) return 0;		/* repair */

    **fileMeta = meta;
    return DCM_NORMAL;
}

CONDITION
DCM_SetFileMeta(DCM_OBJECT ** callerObject, DCM_FILE_META * fileMeta)
{
    CONDITION 		cond;

    meta = *fileMeta;
    cond = DCM_ModifyElements(callerObject, metaRequired, (int) DIM_OF(metaRequired), metaOptional, (int) DIM_OF(metaOptional), NULL);
    if (cond != DCM_NORMAL)	return cond;		/* repair */

    return DCM_NORMAL;
}

CONDITION
DCM_FreeFileMeta(DCM_FILE_META ** fileMeta)
{
    if (fileMeta == NULL) return 0;		/* repair */
    if (*fileMeta == NULL) return 0;		/* repair */

    if ((*fileMeta)->privateInformation != NULL) CTN_FREE((*fileMeta)->privateInformation);

    CTN_FREE(*fileMeta);
    *fileMeta = NULL;

    return DCM_NORMAL;
}

CONDITION
DCM_DefaultFileMeta(DCM_OBJECT ** object, DCM_FILE_META ** fileMeta)
{
    DCM_ELEMENT e[] = {
    		{DCM_IDSOPCLASSUID, DCM_UI, "", 1, DICOM_UI_LENGTH + 1, NULL},
    		{DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, DICOM_UI_LENGTH + 1, NULL},
    };
    CONDITION cond;

    *fileMeta = calloc(1, sizeof(DCM_FILE_META));
    if (*fileMeta == NULL) return 0;		/* repair */

    memset((*fileMeta)->preamble, 0, sizeof((*fileMeta)->preamble));
    (*fileMeta)->fileMetaInformationVersion[0] = 0x00;
    (*fileMeta)->fileMetaInformationVersion[1] = 0x01;
    e[0].d.string = (*fileMeta)->mediaStorageSOPClassUID;
    e[1].d.string = (*fileMeta)->mediaStorageSOPInstanceUID;

    cond = DCM_ParseObject(object, e, (int) DIM_OF(e), NULL, 0, NULL);
    if (cond != DCM_NORMAL)	return 0;		/* repair */

    strcpy((*fileMeta)->transferSyntaxUID, DICOM_TRANSFERLITTLEENDIAN);
    strcpy((*fileMeta)->implementationClassUID, MIR_IMPLEMENTATIONCLASSUID);
    strcpy((*fileMeta)->implementationVersionName, MIR_IMPLEMENTATIONVERSIONNAME);

    (*fileMeta)->flag |= DCM_FILEMETA_IMPLEMENTATIONVERSIONNAME;

    return DCM_NORMAL;
}
