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
** @$=@$=@$=
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s): 	DB_CreateDB, DB_DeleteDB, DB_Init, DB_Open, DB_Close,
**			DB_AddPatient, DB_AddStudy, DB_AddSeries, DB_AddImage,
**			DB_DelPatient, DB_DelStudy, DB_DelSeries, DB_DelImage,
**			DB_GetPatient, DB_GetNextPatient, DB_GetStudy,
**			DB_GetNextStudy, DB_GetSeries, DB_GetNextSeries,
**			DB_GetImage, DB_GetNextImage, DB_Addid, DB_Removeid,
**			DB_Findid, DB_CompareImageUID, DB_CompareSeriesUID,
**			DB_CompareStudyUID, DB_ComparePatID, DB_DumpDB,
**			DB_PrintPatient, DB_PrintStudy, DB_PrintSeries,
**			DB_PrintImage, DB_WriteStudyNode, DB_ReadStudyNode,
**			DB_ReadSeriesNode, DB_WriteSeriesNode, DB_DelBelowStudy,
**			DB_DelBelowSeries, DB_UpdatePatientContext,
**			DB_UpdateStudyContext, DB_UpdateSeriesContext,
**			DB_UpdateImageContext, DB_GetPatientContext,
**			DB_GetStudyContext, DB_GetSeriesContext, DB_GetImageContext
**			DB_GetNumberofStudies, DB_ComparePat, DB_CompareStudy,
**			DB_CompareSeries, DB_CompareImage, DB_Query, DB_NextQuery,
**			DB_ResetQueryContext,DB_SetChangeFlag, DB_CompareChangeFlag
**
** Author, Date:	David E. Beecher, 3-June-93
**
** Intent:		These routines provide access to the databases for the DICOM
**			project.  They rely heavily on the hunk file facility for
**			low-level file handling functions.  All routines in this
**			file are not meant for public use, but
**			rather just the routines which have prototypes in the file
**			dbquery.h.  dbprivate.h contains the prototypes for all other
**			functions to be used only by the database implementor.
**
** Last Update:		$Author: smm $, $Date: 1997/08/10 19:21:59 $
** Source File:		$RCSfile: database.c,v $
** Revision:		$Revision: 1.42 $
** Status:		$State: Exp $
*/
#include <stdio.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#include <string.h>
/*lint -e652 */
#include <ctype.h>
/*lint +e652 */
#include <sys/types.h>
#include <sys/time.h>

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../hunks/hunk_man.h"
#include "../utility/utility.h"
#include "../database/dbquery.h"
#include "dbprivate.h"

static char rcsid[] = "$Revision: 1.42 $ $RCSfile: database.c,v $";

static Root
    GS_root;

static DBidstruct
   *GS_context = (DBidstruct *) NULL;

static short
    GS_dbid = 0;

/* DB_CreateDB
**
** Purpose:
**	This routine creates a new database for use by the system.
**
** Parameter Dictionary:
**	char *dbkey:
**		a null terminated string indicating the file name to use for the db.
**
**
** Return Values:
**
**	DB_NORMAL: 	The creation command succeeded
**	DB_EXISTS: 	This database exists and must be removed first
**	DB_?:		Any valid return from the routine DB_Init
**
** Algorithm:
**	A database file can only be created if it represents a filename which is
**	located in a directory to which the caller has write access.  The file name
**	specified by dbkey must not already exist, or the routine returns DB_EXISTS
**	and the existing database is retained.  The user may use DB_DeleteDB to first
**	remove this database and it may then be created with DB_CreateDB.  More
**	information concerning database creation will be explained in the DB_Init call.
*/
CONDITION
DB_CreateDB(char *dbkey)
{
    short
        dbid;

    if (DB_Open(dbkey, &dbid) == DB_NORMAL) {
	(void) DB_Close(dbid);
	(void) COND_PushCondition(DB_EXISTS, "DB_CreateDB: Database %s is already open", dbkey);
	return (DB_EXISTS);
    }
    return (DB_Init(dbkey));
}

/* DB_DeleteDB
**
** Purpose:
**	This routine deletes an existing database.
**
** Parameter Dictionary:
**	char *dbkey:
**		a null terminated string indicating the file name to use for the db.
**
**
** Return Values:
**
**	DB_NORMAL: 	The deletion command succeeded
**
** Algorithm:
**	The deletion algorithm always assumes it succeeds.  It just deletes the file
**	name passed in dbkey.  This routine may be smartened up if the need arises.
*/
CONDITION
DB_DeleteDB(char *dbkey)
{

    char
       *temp;

    if ((temp = (char *) malloc(2048)) == NULL) {
	(void) COND_PushCondition(DB_NOMEMORY, "DB_DeleteDB: No memory available");
	return (DB_NOMEMORY);
    }
    strcpy(temp, "rm -f ");
    strcat(temp, dbkey);
    (void) system(temp);
    free(temp);

    return (DB_NORMAL);

}

/* DB_Init
**
** Purpose:
**	This routine initializes a new database..
**
** Parameter Dictionary:
**	char *dbkey:
**		a null terminated string indicating the file name to use for the db.
**
**
** Return Values:
**
**	DB_NORMAL: 	The initialization command succeeded
**	DB_CREATERROR:	The database file could not be created
**	DB_OPENERROR:	The newly created database could not be opened
**	DB_ALLOCATERROR:The root node record could not be allocated
**	DB_WRITERROR:	The root node record could not be written
**
** Algorithm:
**	The initialization relies on the hunk_man facility to perform most of the
**	work.  Default parameters are used to set up the hunk file to be used
**	by this database, namely, DB_HUNKINITIALALLOCATION for the number of hunks
**	to initially allocate to this file, DB_HUNKRECORDSIZE, for the hunk record
**	size, and DB_NUMRECSPERHUNK, for the number of records per hunk.  These
**	parameters can be tuned for optimal performance.  The root node is then
**	initialized by nulling out all patient records and written to the hunk.
*/
CONDITION
DB_Init(char *dbkey)
{
    int i;

    if (HF_Create(dbkey, DB_HUNKINITIALALLOCATION,
		  DB_HUNKRECORDSIZE, DB_NUMRECSPERHUNK) != HF_NORMAL) {
	(void) COND_PushCondition(DB_CREATERROR, "DB_Init: HF_Create error");
	return (DB_CREATERROR);
    }
    if (HF_Open(dbkey) != HF_NORMAL) {
	(void) COND_PushCondition(DB_OPENERROR, "DB_Init: HF_Open error");
	return (DB_OPENERROR);
    }
    strcpy(GS_root.dbkey, dbkey);
    GS_root.num_patients = 0;
    for (i = 0; i < DB_MAXPATIENTS; i++) {
	GS_root.patient_loc[i].hunk_number = HUNK_PTR_NULL;
	GS_root.patient_loc[i].node_number = HUNK_PTR_NULL;
    }

    if (HF_AllocateStaticRecord(0, sizeof(GS_root)) != HF_NORMAL) {
	(void) COND_PushCondition(DB_ALLOCATERROR, "DB_Init: HF_AllocateStaticRecord error");
	return (DB_ALLOCATERROR);
    }
    if (HF_WriteStaticRecord(0, sizeof(GS_root), (void *) &GS_root)
	!= HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_Init: HF_WriteStaticRecord error");
	return (DB_WRITERROR);
    }
    (void) HF_Close(dbkey);

    return (DB_NORMAL);
}
/* DB_Close
**
** Purpose:
**	This routine closes a previously opened database.
**
** Parameter Dictionary:
**	short dbid:
**		the database identifier received upon opening the database.
**
**
** Return Values:
**
**	DB_NORMAL: 	The close command succeeded
**	DB_CLOSERROR:	The database file could not be closed
**
** Algorithm:
**	The close simply performs any writes needed at the hunk_file level and
**	removes this database id from the list of active dbids.
*/
CONDITION
DB_Close(short dbid)
{

    CONDITION
	retval;
    char
        ts[DB_MAXKEYLENGTH];
    if ((retval = DB_Removeid(dbid, ts)) != DB_NORMAL) {
	(void) COND_PushCondition(retval, "DB_Close: dbid handling error");
	return (retval);
    }
    if (HF_Close(ts) != HF_NORMAL) {
	(void) COND_PushCondition(DB_CLOSERROR, "DB_Close: HF_Close error");
	return (DB_CLOSERROR);
    }
    return (DB_NORMAL);
}
/* DB_Addid
**
** Purpose:
**	This routine adds a new database access id to an internal list which
**	also stores contextual information for sequential database searches.
**
** Parameter Dictionary:
**	short dbid:
**		the database identifier to be generated by this routine.
**
**
** Return Values:
**
**	DB_NORMAL: 	The command succeeded and generated a new dbid.
**	DB_NOMEMORY:	No memory is available for list allocation
**
** Algorithm:
**	The routine simply increments a static global variable, GS_dbid, to
**	create the next dbid.  This dbid will be used in all subsequent access
**	calls to this database.  Then, another DBidstruct is allocated and added
**	to the list of existing structs which may be opened by other users.
**	Currently, each process may only have a single database open at one time.
**	This can easily be modified if the need arises.
*/
CONDITION
DB_Addid(short *dbid)
{

    DBidstruct
       *temp;

    *dbid = ++GS_dbid;

    if (GS_context == (DBidstruct *) NULL) {
	if ((GS_context = (DBidstruct *) malloc(sizeof(DBidstruct))) ==
	    (DBidstruct *) NULL) {
	    return (DB_NOMEMORY);
	}
	GS_context->next = (DBidstruct *) NULL;
    } else {
	if ((temp = (DBidstruct *) malloc(sizeof(DBidstruct))) ==
	    (DBidstruct *) NULL) {
	    return (DB_NOMEMORY);
	}
	temp->next = GS_context;
	GS_context = temp;
    }
    return (DB_NORMAL);
}
/* DB_Removeid
**
** Purpose:
**	This routine removes the DBidstruct from the existing list if a match
**	is found with the dbid passed.
**
** Parameter Dictionary:
**	short dbid:
**		the database identifier to be removed by this routine.
**
**
** Return Values:
**
**	DB_NORMAL: 	The command succeeded and generated a new dbid.
**	DB_IDREMERROR:	The DBidstruct list is null, there are no entries.
**			Or, the dbid could not be found in the list.
**
** Algorithm:
**	The routine searches through the DBidstruct list pointed to by the
**	global variable GS_context, and deletes the entry which contains the
**	the dbid passed as input to the routine.
*/
CONDITION
DB_Removeid(short dbid, char *dbkey)
{

    DBidstruct
       *pt,
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return (DB_IDREMERROR);

    pt = temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    strcpy(dbkey, temp->dbkey);
	    if (temp == GS_context) {
		GS_context = temp->next;
		free(temp);
	    } else {
		pt->next = temp->next;
		free(temp);
	    }
	    return (DB_NORMAL);
	}
	pt = temp;
	temp = temp->next;
    }

    return (DB_IDREMERROR);
}
/* DB_Open
**
** Purpose:
**	This routine attempts to open for access the database pointed to by
**	the input string dbkey.
**
** Parameter Dictionary:
**	char *dbkey:
**		the name of the database to open
**	short dbid:
**		will contain the newly created dbid upon successful return
**
**
** Return Values:
**
**	DB_NORMAL: 	The open succeeded and a new database has been opened.
**	DB_OPENERROR:	The corresponding hunk_file could not be opened.
**	DB_?:		Any error return from DB_Addid.
**	DB_READERROR:	The root record of the database could not be read.
**
** Algorithm:
**	DB_Open first attempts to open the corresponding hunk file for access. It
**	then attempts to generate a new dbid and context structure.  Failure for
**	either of these two functions results in an open failure.
**	If successful, the context structure is effectively set to null, and the
**	root record of the database is read.
*/
CONDITION
DB_Open(char *dbkey, short *dbid)
{

    CONDITION retval;
    HunkBufAdd temp;

    if (HF_Open(dbkey) != HF_NORMAL) {
	(void) COND_PushCondition(DB_OPENERROR, "DB_Open: HF_Open error");
	return (DB_OPENERROR);
    }
    if ((retval = DB_Addid(dbid)) != DB_NORMAL) {
	(void) COND_PushCondition(retval, "DB_Open: id handling error");
	(void) HF_Close(dbkey);
	return (retval);
    }
    strcpy(GS_context->dbkey, dbkey);
    GS_context->dbid = *dbid;
    temp.hunk_number = HUNK_PTR_NULL;
    temp.node_number = HUNK_PTR_NULL;
    DB_UpdatePatientContext(*dbid, &temp, -1, DB_PATIENTCONTEXT);
    DB_UpdatePatientContext(*dbid, &temp, -1, DB_STUDYCONTEXT);
    DB_UpdatePatientContext(*dbid, &temp, -1, DB_SERIESCONTEXT);
    DB_UpdatePatientContext(*dbid, &temp, -1, DB_IMAGECONTEXT);
    DB_UpdateStudyContext(*dbid, &temp, -1, DB_PATIENTCONTEXT);
    DB_UpdateStudyContext(*dbid, &temp, -1, DB_STUDYCONTEXT);
    DB_UpdateStudyContext(*dbid, &temp, -1, DB_SERIESCONTEXT);
    DB_UpdateStudyContext(*dbid, &temp, -1, DB_IMAGECONTEXT);
    DB_UpdateSeriesContext(*dbid, &temp, -1, DB_PATIENTCONTEXT);
    DB_UpdateSeriesContext(*dbid, &temp, -1, DB_STUDYCONTEXT);
    DB_UpdateSeriesContext(*dbid, &temp, -1, DB_SERIESCONTEXT);
    DB_UpdateSeriesContext(*dbid, &temp, -1, DB_IMAGECONTEXT);
    DB_UpdateImageContext(*dbid, &temp, -1, DB_PATIENTCONTEXT);
    DB_UpdateImageContext(*dbid, &temp, -1, DB_STUDYCONTEXT);
    DB_UpdateImageContext(*dbid, &temp, -1, DB_SERIESCONTEXT);
    DB_UpdateImageContext(*dbid, &temp, -1, DB_IMAGECONTEXT);
    DB_UpdateQueryContext(*dbid, &temp, -1, DB_PATIENTCONTEXT);
    DB_UpdateQueryContext(*dbid, &temp, -1, DB_STUDYCONTEXT);
    DB_UpdateQueryContext(*dbid, &temp, -1, DB_SERIESCONTEXT);
    DB_UpdateQueryContext(*dbid, &temp, -1, DB_IMAGECONTEXT);

    if (HF_ReadStaticRecord(0, sizeof(GS_root), (void *) &GS_root)
	!= HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_Open: HF_ReadStaticRecord error");
	return (DB_READERROR);
    }
    return (DB_NORMAL);
}
/* DB_AddPatient
**
** Purpose:
**	This routine adds a new patient to the database.
**
** Parameter Dictionary:
**	short dbid:
**		Contains the database identifier.
**	PatientLevel *pat:
**		Contains the patient information to add to the database.
**
** Return Values:
**
**	DB_NORMAL: 	The patient add succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_TOOMANYPATS:	Only DB_MAXPATIENTS are allowed, this add would result
**		in one more (DB_MAXPATIENTS is currently 20).
**	DB_DUPATIENT:	The patien with this PatID is already in the database.
**	DB_ALLOCATERROR:The new record in the hunk file could not be allocated.
**	DB_WRITERROR:	The new record could not be written to the hunk file.
**
** Algorithm:
**	The dbid must be found in the struct list.  Then, the check is made for the
**	maximum number of patients and the pre-existing patient.  If all this checks
**	out, the new records are allocated, the root node is updated, and all new
**	information is written out to the database file.
*/
CONDITION
DB_AddPatient(short dbid, PatientLevel *pat)
{

    int i;
    HunkBufAdd hadd;
    PatientNode pnode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_AddPatient: DB not opened");
	return (DB_NOTOPENED);
    }
    if (GS_root.num_patients >= DB_MAXPATIENTS) {
	(void) COND_PushCondition(DB_TOOMANYPATS, "DB_AddPatient: Too many patients");
	return (DB_TOOMANYPATS);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_AddPatient: Lock Error");
	return (DB_LOCKERROR);
    }
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(pat->PatID, &GS_root.patient_loc[i]) == DB_MATCH) {
	    (void) COND_PushCondition(DB_DUPATIENT, "DB_AddPatient: Duplicate patient");
	    return (DB_UnLock(DB_DUPATIENT));
	}
    }
    if (HF_AllocateRecord(sizeof(PatientNode), &hadd) != HF_NORMAL) {
	(void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddPatient: HF_AllocateRecord error");
	return (DB_UnLock(DB_ALLOCATERROR));
    }
    GS_root.patient_loc[GS_root.num_patients].hunk_number = hadd.hunk_number;
    GS_root.patient_loc[GS_root.num_patients].node_number = hadd.node_number;
    pnode.time_stamp = time((time_t *) NULL);
    pnode.pat = *pat;
    pnode.num_studies = 0;
    for (i = 0; i < DB_MAXSTUDIES; i++) {
	pnode.study_loc[i].hunk_number = HUNK_PTR_NULL;
	pnode.study_loc[i].node_number = HUNK_PTR_NULL;
    }
    if (HF_WriteRecord(&hadd, sizeof(PatientNode), (void *) &pnode)
	!= HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddPatient: HF_WriteRecord error");
	return (DB_UnLock(DB_WRITERROR));
    }
    GS_root.num_patients++;

    if (HF_WriteStaticRecord(0, sizeof(GS_root), (void *) &GS_root)
	!= HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddPatient: HF_WriteStaticRecord error");
	return (DB_UnLock(DB_WRITERROR));
    }
    return (DB_UnLock(DB_NORMAL));
}
/* DB_FindID
**
** Purpose:
**	This routine attempts to locate the dbid in the maintained list.
**
** Parameter Dictionary:
**	short dbid:
**		Contains the database identifier.
**
** Return Values:
**
**	DB_NORMAL: 	The dbid was found.
**	DB_IDNOTHERE:	The dbid was not found.
**
** Algorithm:
**	The dbid is search for and if found, DB_NORMAL is returned, otherwise,
**	DB_IDNOTHERE is returned.
*/
CONDITION
DB_Findid(short dbid)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return (DB_IDNOTHERE);

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    return (DB_NORMAL);
	}
	temp = temp->next;
    }
    return (DB_IDNOTHERE);
}
/* DB_CompareImageUID
**
** Purpose:
**	This Checks an image record to see if the image uid matches the one
**	passed to it.
**
** Parameter Dictionary:
**	char *imageuid:
**		The image uid to check
**	HunkBufAdd *image_loc:
**		The hunk buffer address of the ImageNode struct to check.
**
** Return Values:
**
**	DB_MATCH: 	The uid's match.
**	DB_NOMATCH:	The uid's don't match.
**
** Algorithm:
**	The image UID is compared to the image UID int the ImageNode struct
**	in the hunk file.  If a match is found, the routine returns DB_MATCH,
**	otherwise, DB_NOMATCH is returned.
*/
CONDITION
DB_CompareImageUID(char *imageuid, HunkBufAdd *image_loc)
{

    ImageNode inode;

    if (HF_ReadRecord(image_loc, sizeof(inode), (void *) &inode) != HF_NORMAL)
	return (DB_NOMATCH);

    if (strcmp(inode.image.ImageUID, imageuid) == 0)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
/* DB_CompareSeriesImageUID
**
** Purpose:
**	This Checks a series record to see if the series uid matches the one
**	passed to it.
**
** Parameter Dictionary:
**	char *seriesuid:
**		The series uid to check.
**	HunkBufAdd *series_loc:
**		The hunk buffer address of the SeriesNode struct to check.
**
** Return Values:
**
**	DB_MATCH: 	The uid's match.
**	DB_NOMATCH:	The uid's don't match.
**
** Algorithm:
**	The series UID is compared to the series UID int the SeriesNode struct
**	in the hunk file.  If a match is found, the routine returns DB_MATCH,
**	otherwise, DB_NOMATCH is returned.
*/
CONDITION
DB_CompareSeriesUID(char *seriesuid, HunkBufAdd *series_loc)
{

    SeriesNode snode;

    if (HF_ReadRecord(series_loc, sizeof(snode), (void *) &snode) != HF_NORMAL)
	return (DB_NOMATCH);

    if (strcmp(snode.series.SeriesUID, seriesuid) == 0)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
/* DB_CompareStudyUID
**
** Purpose:
**	This Checks a study record to see if the study uid matches the one
**	passed to it.
**
** Parameter Dictionary:
**	char *studyuid:
**		The series uid to check.
**	HunkBufAdd *study_loc:
**		The hunk buffer address of the SeriesNode struct to check.
**
** Return Values:
**
**	DB_MATCH: 	The uid's match.
**	DB_NOMATCH:	The uid's don't match.
**
** Algorithm:
**	The study UID is compared to the series UID int the StudyNode struct
**	in the hunk file.  If a match is found, the routine returns DB_MATCH,
**	otherwise, DB_NOMATCH is returned.
*/
CONDITION
DB_CompareStudyUID(char *studyuid, HunkBufAdd *study_loc)
{

    StudyNode snode;

    if (HF_ReadRecord(study_loc, sizeof(snode), (void *) &snode) != HF_NORMAL)
	return (DB_NOMATCH);

    if (strcmp(snode.study.StudyUID, studyuid) == 0)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
/* DB_ComparePatUID
**
** Purpose:
**	This checks a PatientNode to see if the patient id matches the one
**	passed to it.
**
** Parameter Dictionary:
**	char *patid:
**		The patient id to check.
**	HunkBufAdd *study_loc:
**		The hunk buffer address of the PatientNode struct to check.
**
** Return Values:
**
**	DB_MATCH: 	The uid's match.
**	DB_NOMATCH:	The uid's don't match.
**
** Algorithm:
**	The patient UID is compared to the patient UID int the PatinetNode struct
**	in the hunk file.  If a match is found, the routine returns DB_MATCH,
**	otherwise, DB_NOMATCH is returned.
*/
CONDITION
DB_ComparePatID(char *patid, HunkBufAdd *pat_loc)
{

    PatientNode pnode;

    if (HF_ReadRecord(pat_loc, sizeof(pnode), (void *) &pnode) != HF_NORMAL)
	return (DB_NOMATCH);

    if (strcmp(pnode.pat.PatID, patid) == 0)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
/* DB_AddStudy
**
** Purpose:
**	DB_AddStudy adds a study node to an existing patient
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**	char *patid:
**		The patient id to add the study to.
**	StudyLevel *study:
**		The study level struct to add to the specified patient.
**
** Return Values:
**
**	DB_NORMAL: 	The add study succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The patient specified counld not be found.
**	DB_READERROR:	The hunk record could not be read.
**	DB_TOOMANYSTUDS:Adding this study would exceed the maximum allowed which
**		is set at DB_MAXSTUDIES.  This value is currently 20.
**	DB_DUPSTUDY:	The study being added already exists for this patient.
**	DB_ALLOCATERROR:The new record could not be allocated.
**	DB_WRITERROR:	The new record could not be written.
**
** Algorithm:
**	We do the normal checks for database open.  Then we try and find the patient
**	specified by patid in the database.  It is an error not to be able to locate
**	this patient.  Once the patient is found, we check to see if the study about
**	to be added already exists for this patient, if so, this is an error and we
**	return.  Otherwise, we attempt to allocate the new hunk records, fill them
**	with the new data, and write them back to the file.
**
*/
CONDITION
DB_AddStudy(short dbid, char *patid, StudyLevel *study)
{

    int i,
        found;
    HunkBufAdd
        patadd,
        studadd;
    PatientNode pnode;
    StudyNode snode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_AddStudy: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_AddStudy: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_AddStudy: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_AddStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if (pnode.num_studies >= DB_MAXSTUDIES) {
	(void) COND_PushCondition(DB_TOOMANYSTUDS, "DB_AddStudy: Too many studies");
	return (DB_UnLock(DB_TOOMANYSTUDS));
    }
    /*
     * Check for a duplicate Study...
     */
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(study->StudyUID, &pnode.study_loc[i]) == DB_MATCH) {
	    (void) COND_PushCondition(DB_DUPSTUDY, "DB_AddStudy: Duplicate study");
	    return (DB_UnLock(DB_DUPSTUDY));
	}
    }

    if (HF_AllocateRecord(sizeof(StudyNode), &studadd) != HF_NORMAL) {
	(void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddStudy: HF_AllocateRecord failed");
	return (DB_UnLock(DB_ALLOCATERROR));
    }
    snode.time_stamp = time((time_t *) NULL);
    snode.study = *study;
    snode.num_allocated = DB_INITSIZE;
    snode.num_series = 0;
    for (i = 0; i < DB_INITSIZE; i++) {
	snode.series_loc[i].hunk_number = HUNK_PTR_NULL;
	snode.series_loc[i].node_number = HUNK_PTR_NULL;
    }
    pnode.study_loc[pnode.num_studies].hunk_number = studadd.hunk_number;
    pnode.study_loc[pnode.num_studies].node_number = studadd.node_number;
    pnode.num_studies++;

    if (HF_WriteRecord(&patadd, sizeof(PatientNode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddStudy: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    if (HF_WriteRecord(&studadd, sizeof(StudyNode), (void *) &snode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddStudy: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    return (DB_UnLock(DB_NORMAL));
}
/* DB_dumpDB
**
** Purpose:
**	DB_dumpDB is a utility to dump the database to standard out.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**
** Return Values:
**
**	none
**
** Algorithm:
**	Just a utility do dump the database mostly for diagnostic purposes...not really
**	meant for end users.
*/
void
DB_DumpDB(short dbid)
{
    int i,
        j,
        k,
        l;

    PatientNode pnode;
    StudyNode
       *stn;
    SeriesNode *sen;
    ImageNode inode;

    HunkBufAdd paadd,
        stadd,
        seadd,
        imadd;

    if (DB_Findid(dbid) != DB_NORMAL)
	return;

    if (HF_SharedLock() != HF_NORMAL)
	return;

    printf("DB Key: %s  Number of Patients: %d Number of Additions/Deletions: %ld\n\n", GS_root.dbkey, GS_root.num_patients, HF_ReadUpdateFlag());

    for (i = 0; i < DB_MAXPATIENTS; i++) {
	paadd = GS_root.patient_loc[i];
	printf("Patient Record %3d %3d\n", paadd.hunk_number, paadd.node_number);
	if (paadd.hunk_number != HUNK_PTR_NULL &&
	    paadd.node_number != HUNK_PTR_NULL) {
	    if (HF_ReadRecord(&paadd, sizeof(pnode), (void *) &pnode) == HF_NORMAL) {
		DB_PrintPatient(pnode);
	    } else
		printf("Bad read on Patient Record %3d %3d\n",
		       paadd.hunk_number, paadd.node_number);
	    for (j = 0; j < DB_MAXSTUDIES; j++) {
		stadd = pnode.study_loc[j];
		printf("Study Record %3d %3d\n", stadd.hunk_number, stadd.node_number);
		if (stadd.hunk_number != HUNK_PTR_NULL && stadd.node_number != HUNK_PTR_NULL) {
		    if (DB_ReadStudyNode(&stadd, (void *) &stn) == DB_NORMAL) {
		    	DB_PrintStudy(stn);
		    	for (k = 0; k < stn->num_allocated; k++) {
		    		seadd = stn->series_loc[k];
		    		printf("Series Record %3d %3d\n", seadd.hunk_number, seadd.node_number);
		    		if (seadd.hunk_number != HUNK_PTR_NULL && seadd.node_number != HUNK_PTR_NULL) {
		    			if (DB_ReadSeriesNode(&seadd, (void *) &sen) == DB_NORMAL) {
		    				DB_PrintSeries(sen);
		    				for (l = 0; l < sen->num_allocated; l++) {
		    					imadd = sen->image_loc[l];
		    					printf("Image Record %3d %3d\n", imadd.hunk_number, imadd.node_number);
		    					if (imadd.hunk_number != HUNK_PTR_NULL && imadd.node_number != HUNK_PTR_NULL) {
		    						if (HF_ReadRecord(&imadd, sizeof(inode), &inode) == HF_NORMAL) DB_PrintImage(inode);

		    					}
		    				}
		    				free(sen);
		    			}else{
		    				printf("Bad read on Series Record %3d %3d\n", seadd.hunk_number, seadd.node_number);
		    			}
		    		}
		    	}
		    	free(stn);
		    }else{
		    	printf("Bad read on Study Record %3d %3d\n", stadd.hunk_number, stadd.node_number);
		    }
		}
	    }
	}
    }
    (void) HF_UnLock();
}
/* DB_PrintSeries
**
** Purpose:
**	DB_PrintSeries is a utility to dump a series record to standard out.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**
** Return Values:
**
**	none
**
** Algorithm:
**	Just a utility do dump a series record for diagnostic purposes...not really
**	meant for end users.
*/
void
DB_PrintSeries(SeriesNode *snode)
{
    printf("\t\tImages(act/alloc) %d %d\n",
	   snode->num_images, snode->num_allocated);
    printf("\t\t     Modality: %s\n", snode->series.Modality);
    printf("\t\tSeries Number: %s\n", snode->series.SeriesNumber);
    printf("\t\t   Series UID: %s\n", snode->series.SeriesUID);
    printf("\t\t  Series Time: %ld\n", snode->time_stamp);
}
/* DB_PrintStudy
**
** Purpose:
**	DB_PrintStudy is a utility to dump a study record to standard out.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**
** Return Values:
**
**	none
**
** Algorithm:
**	Just a utility do dump a study record for diagnostic purposes...not really
**	meant for end users.
*/
void
DB_PrintStudy(StudyNode *snode)
{

    printf("\t\tSeries(act/alloc) %d %d\n",
	   snode->num_series, snode->num_allocated);
    printf("\t\t                    Study Date: %s\n", snode->study.StudyDate);
    printf("\t\t                    Study Time: %s\n", snode->study.StudyTime);
    printf("\t\t                      Study ID: %s\n", snode->study.StudyID);
    printf("\t\t                     Accession: %s\n", snode->study.AccessionNumber);
    printf("\t\t                     Study UID: %s\n", snode->study.StudyUID);
    printf("\t\t             ReferringPhysName: %s\n", snode->study.ReferringPhysName);
    printf("\t\t          InterpretingPhysName: %s\n", snode->study.InterpretingPhysName);
    printf("\t\t          ProcedureDescription: %s\n", snode->study.ProcedureDescription);
    printf("\t\t AdmittingDiagnosedDescription: %s\n", snode->study.AdmittingDiagnosedDescription);
    printf("\t\tStudy Time: %ld\n", snode->time_stamp);
}
/* DB_PrintPatient
**
** Purpose:
**	DB_PrintPatient is a utility to dump a patient record to standard out.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**
** Return Values:
**
**	none
**
** Algorithm:
**	Just a utility do dump a patient record for diagnostic purposes...not really
**	meant for end users.
*/
void
DB_PrintPatient(PatientNode pnode)
{

    printf("\tBirthdate: %s\n", pnode.pat.BirthDate);
    printf("\t     Name: %s\n", pnode.pat.Name);
    printf("\t    PatID: %s\n", pnode.pat.PatID);
    printf("\t Pat Time: %ld\n\n", pnode.time_stamp);
}

/* DB_PrintImage
**
** Purpose:
**	DB_PrintImage is a utility to dump a image record to standard out.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**
** Return Values:
**
**	none
**
** Algorithm:
**	Just a utility do dump a image record for diagnostic purposes...not really
**	meant for end users.
*/
void
DB_PrintImage(ImageNode inode)
{

    printf("\tImageNumber: %s\n", inode.image.ImageNumber);
    printf("\t  ImageUID: %s\n", inode.image.ImageUID);
    printf("\t Class UID: %s\n", inode.image.ClassUID);
    printf("\t  FileName: %s\n", inode.image.FileName);
    printf("\tImage Time: %ld\n\n", inode.time_stamp);
}

/* DB_AddSeries
**
** Purpose:
**	DB_AddSeries adds a series node to an existing patient and study.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**	char *patid:
**		The patient id to add the series to.
**	char *studyuid:
**		The study uid to add the series to.
**	SeriesLevel *series:
**		The series level struct to add to the specified patient and study.
**
** Return Values:
**
**	DB_NORMAL: 	The add series succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The patient specified counld not be found.
**	DB_READERROR:	The hunk record could not be read.
**	DB_BADSTUDY:	The study specified counld not be found.
**	DB_DUPSERIES:	The series being added already exists for
**			this patient and study.
**	DB_ALLOCATERROR:The new record could not be allocated.
**	DB_WRITERROR:	The new record could not be written.
**	DB_NOMEMORY:	Couldn't allocate any local memory for temps.
**
** Algorithm:
**	We do the normal checks for database open.  Then we try and find the patient
**	and study specified by patid and studyuid in the database.  It is an error
**	not to be able to locate the patient and study.  Once both are found, we
**	check to see if the series about to be added already exists for this patient
**	and study, if so, this is an error and we return.  Otherwise, we attempt to
**	allocate the new hunk records, fill them with the new data, and write them
**	back to the file. There is one twist here.  Since we don't have an upper
**	limit on the number of series/study, we may have to reallocate the study
**	record if the default number of series is exceeded.  This will have to be
**	rewritten to the database and the pointers in the patient record will have
**	to be appropriately updated and rewritten.  This is why this routine is a
**	little more complex that you might normally think.
**
*/
CONDITION
DB_AddSeries(short dbid, char *patid, char *studyuid, SeriesLevel *series)
{

    int i,
        size_to_alloc,
        study_inx = 0,
        found;
    HunkBufAdd
        patadd,
        stuadd,
        newstuadd,
        seradd;
    PatientNode pnode;
    StudyNode *snode,
       *new_snode;
    SeriesNode sernode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_AddSeries: DB not opened");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_AddSeries: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_AddSeries: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_AddSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &pnode.study_loc[i]) == DB_MATCH) {
	    stuadd = pnode.study_loc[i];
	    study_inx = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_AddSeries: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&stuadd, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_AddSeries: DB_ReadStudyNode failed");
	return (DB_UnLock(DB_READERROR));
    }
    /*
     * Check for a duplicate Series
     */
    for (i = 0; i < snode->num_series; i++) {
	if (DB_CompareSeriesUID(series->SeriesUID,
				&(snode->series_loc[i])) == DB_MATCH) {
	    free(snode);
	    (void) COND_PushCondition(DB_DUPSERIES, "DB_AddSeries: Duplicate Series");
	    return (DB_UnLock(DB_DUPSERIES));
	}
    }

    size_to_alloc = sizeof(StudyNode) + (snode->num_allocated - DB_INITSIZE) *
	sizeof(HunkBufAdd);
    if (snode->num_allocated == snode->num_series) {	/* Need to expand the
							 * list */
	size_to_alloc = sizeof(StudyNode);
	size_to_alloc += (snode->num_allocated + DB_EXPANDSIZE - DB_INITSIZE) *
	    sizeof(HunkBufAdd);
	new_snode = (StudyNode *) malloc(size_to_alloc);
	if (new_snode == (StudyNode *) NULL) {
	    free(snode);
	    (void) COND_PushCondition(DB_NOMEMORY, "DB_AddSeries: No memory");
	    return (DB_UnLock(DB_NOMEMORY));
	}
	memcpy((void *) new_snode, (void *) snode,
	       sizeof(StudyNode) + ((snode->num_allocated - DB_INITSIZE) *
				    sizeof(HunkBufAdd)));
	for (i = snode->num_allocated; i < snode->num_allocated + DB_EXPANDSIZE; i++) {
	    new_snode->series_loc[i].hunk_number = HUNK_PTR_NULL;
	    new_snode->series_loc[i].node_number = HUNK_PTR_NULL;
	}
	new_snode->num_allocated += DB_EXPANDSIZE;
	free(snode);
	if (HF_AllocateRecord(size_to_alloc, &newstuadd) != HF_NORMAL) {
	    free(new_snode);
	    (void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddSeries: HF_AllocateRecord failed");
	    return (DB_UnLock(DB_ALLOCATERROR));
	}
	pnode.study_loc[study_inx] = newstuadd;
	if (HF_WriteRecord(&patadd, sizeof(PatientNode), (void *) &pnode)
	    != HF_NORMAL) {
	    free(new_snode);
	    (void) COND_PushCondition(DB_WRITERROR, "DB_AddSeries: HF_WriteRecord failed");
	    return (DB_UnLock(DB_WRITERROR));
	}
	snode = new_snode;
	(void) HF_DeallocateRecord(&stuadd);
	stuadd = newstuadd;
	(void) HF_IncrementUpdateFlag();	/* We need to let folks know
						 * the db has changed */
    }
    if (HF_AllocateRecord(sizeof(SeriesNode), &seradd) != HF_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddSeries: HF_AllocateRecord failed");
	return (DB_UnLock(DB_ALLOCATERROR));
    }
    snode->series_loc[snode->num_series] = seradd;
    snode->num_series++;

    if (HF_WriteRecord(&stuadd, size_to_alloc, (void *) snode)
	!= HF_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddSeries: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    sernode.time_stamp = time((time_t *) NULL);
    sernode.series = *series;
    sernode.num_allocated = DB_INITSIZE;
    sernode.num_images = 0;
    for (i = 0; i < DB_INITSIZE; i++) {
	sernode.image_loc[i].hunk_number = HUNK_PTR_NULL;
	sernode.image_loc[i].node_number = HUNK_PTR_NULL;
    }
    if (HF_WriteRecord(&seradd, sizeof(SeriesNode), (void *) &sernode)
	!= HF_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddSeries: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    free(snode);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_AddImage
**
** Purpose:
**	DB_AddImage adds an image node to an existing patient, study, and series.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifer.
**	char *patid:
**		The patient id to add the image to.
**	char *studyuid:
**		The study uid to add the image to.
**	char *seriesuid:
**		The series uid to add the image to.
**	ImageLevel *image:
**		The image level struct to add to the specified patient, study, and series.
**
** Return Values:
**
**	DB_NORMAL: 	The add image succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The patient specified counld not be found.
**	DB_READERROR:	The hunk record could not be read.
**	DB_BADSTUDY:	The study specified could not be found.
**	DB_BADSERIES:	The series specified could not be found.
**	DB_ALLOCATERROR:The new record could not be allocated.
**	DB_WRITERROR:	The new record could not be written.
**	DB_NOMEMORY:	Couldn't allocate any local memory for temps.
**
** Algorithm:
**	We do the normal checks for database open.  Then we try and find the patient,
**	study, and series specified by patid, studyuid, and seriesuid in the database.
**	It is an error not to be able to locate the patient, study, and series.
**	Once all are found, we check to see if the image about to be added already
**	exists for this patient, study, and series (actually, this is not done yet
**	but will be added...) if so, this is an error and we return.  Otherwise, we
**	attempt to allocate the new hunk records, fill them with the new data, and
**	write them back to the file. There is one twist here.  Since we don't have
**	an upper limit on the number of series/study or on the number of images/series,
**	we may have to reallocate the series record if the default number of images
**	is exceeded.  This will have to be rewritten to the database and the pointers
**	in the study record will have to be appropriately updated and rewritten.
**	This is why this routine is a little more complex that you might normally think.
**
*/
CONDITION
DB_AddImage(short dbid, char *patid, char *studyuid, char *seriesuid, ImageLevel *image)
{

    int i,
        size_to_alloc,
        series_inx = 0,
        found;

    HunkBufAdd
        patadd,
        stuadd,
        seradd,
        newseradd,
        imgadd;

    PatientNode pnode;
    StudyNode *snode;
    SeriesNode *sernode,
       *new_sernode;
    ImageNode inode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_AddImage: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_AddImage: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    /*
     * Check for the Patient...
     */
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_AddImage: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_AddImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    /*
     * Check for the Study...
     */
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &pnode.study_loc[i]) == DB_MATCH) {
	    stuadd = pnode.study_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_AddImage: Can't find studyuid %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&stuadd, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_AddImage: DB_ReadStudyNode failed");
	return (DB_UnLock(DB_READERROR));
    }
    /*
     * Check for the Series...
     */
    found = 0;
    for (i = 0; i < snode->num_series; i++) {
	if (DB_CompareSeriesUID(seriesuid, &(snode->series_loc[i])) == DB_MATCH) {
	    seradd = snode->series_loc[i];
	    series_inx = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	free(snode);
	(void) COND_PushCondition(DB_BADSERIES, "DB_AddImage: Can't find series %s", seriesuid);
	return (DB_UnLock(DB_BADSERIES));
    }
    if (DB_ReadSeriesNode(&seradd, &sernode) != DB_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_AddImage: DB_ReadSeriesNode failed");
	return (DB_UnLock(DB_READERROR));
    }
    /*
     * Check for a duplicate Image
     */
    for (i = 0; i < sernode->num_images; i++) {
	if (DB_CompareImageUID(image->ImageUID,
			       &(sernode->image_loc[i])) == DB_MATCH) {
	    free(snode);
	    free(sernode);
	    (void) COND_PushCondition(DB_DUPIMAGE, "DB_AddImage: Duplicate image");
	    return (DB_UnLock(DB_DUPIMAGE));
	}
    }


    size_to_alloc = sizeof(SeriesNode) + (sernode->num_allocated - DB_INITSIZE) *
	sizeof(HunkBufAdd);
    if (sernode->num_allocated == sernode->num_images) {	/* Need to expand the
								 * list */
	int study_node_size;
	size_to_alloc = sizeof(SeriesNode);
	size_to_alloc += (sernode->num_allocated + DB_EXPANDSIZE - DB_INITSIZE) *
	    sizeof(HunkBufAdd);
	new_sernode = (SeriesNode *) malloc(size_to_alloc);
	if (new_sernode == (SeriesNode *) NULL) {
	    free(snode);
	    free(sernode);
	    (void) COND_PushCondition(DB_NOMEMORY, "DB_AddImage: No memory");
	    return (DB_UnLock(DB_NOMEMORY));
	}
	memcpy((void *) new_sernode, (void *) sernode,
	       sizeof(SeriesNode) + ((sernode->num_allocated - DB_INITSIZE) *
				     sizeof(HunkBufAdd)));
	for (i = sernode->num_allocated; i < sernode->num_allocated + DB_EXPANDSIZE; i++) {
	    new_sernode->image_loc[i].hunk_number = HUNK_PTR_NULL;
	    new_sernode->image_loc[i].node_number = HUNK_PTR_NULL;
	}
	new_sernode->num_allocated += DB_EXPANDSIZE;
	free(sernode);

	if (HF_AllocateRecord(size_to_alloc, &newseradd) != HF_NORMAL) {
	    free(snode);
	    free(new_sernode);
	    (void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddImage: HF_AllocateRecord failed");
	    return (DB_UnLock(DB_ALLOCATERROR));
	}
	snode->series_loc[series_inx] = newseradd;

	study_node_size = sizeof(StudyNode) + (snode->num_allocated - DB_INITSIZE) *
	    sizeof(HunkBufAdd);
	if (HF_WriteRecord(&stuadd, study_node_size, (void *) snode)
	    != HF_NORMAL) {
	    free(snode);
	    free(new_sernode);
	    (void) COND_PushCondition(DB_WRITERROR, "DB_AddImage: HF_WriteRecord failed");
	    return (DB_UnLock(DB_WRITERROR));
	}
	sernode = new_sernode;
	(void) HF_DeallocateRecord(&seradd);
	seradd = newseradd;
	(void) HF_IncrementUpdateFlag();	/* We need to let folks know
						 * the db has changed */
    }
    free(snode);
    if (HF_AllocateRecord(sizeof(ImageNode), &imgadd) != HF_NORMAL) {
	free(sernode);
	(void) COND_PushCondition(DB_ALLOCATERROR, "DB_AddImage: HF_AllocateRecord failed");
	return (DB_UnLock(DB_ALLOCATERROR));
    }
    sernode->image_loc[sernode->num_images] = imgadd;
    sernode->num_images++;

    if (HF_WriteRecord(&seradd, size_to_alloc, (void *) sernode)
	!= HF_NORMAL) {
	free(sernode);
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddImage: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    free(sernode);

    inode.time_stamp = time((time_t *) NULL);
    inode.image = *image;
    if (HF_WriteRecord(&imgadd, sizeof(ImageNode), (void *) &inode)
	!= HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_AddImage: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    return (DB_UnLock(DB_NORMAL));
}
/* DB_ReadSeriesNode
**
** Purpose:
**	DB_ReadSeriesNode reads a series node from the database.
**
** Parameter Dictionary:
**	HunkBufAdd *seradd:
**		The hunk address of the series node.
**	SeriesNode **snode:
**		A pointer to the SeriesNode struct to read from the database.
**
** Return Values:
**
**	DB_NORMAL: 	The series node was read successfully.
**	DB_READERROR:	The record could not be read.
**	DB_NOMEMORY:	Couldn't allocate any memory for the new node.
**
** Algorithm:
**	Normally, this would be a straight HF_ReadRecord call, however, Series
**	Nodes (and Study Nodes), are not fixed in length.  Their length depends
**	on how many children they have.  So, we must read the record with the
**	default length, and then check to see how large it really is, and allocate
**	enough memory to hold this new structure, then re-read it with the proper
**	length.
**
*/
CONDITION
DB_ReadSeriesNode(HunkBufAdd *seradd, SeriesNode **snode)
{

    SeriesNode temp;
    int new_size;

    if (HF_ReadRecord(seradd, sizeof(temp), (void *) &temp) != HF_NORMAL)
	return (DB_READERROR);

    if (temp.num_allocated > DB_INITSIZE) {
	new_size = sizeof(SeriesNode);
	new_size += (temp.num_allocated - DB_INITSIZE) * sizeof(HunkBufAdd);
	*snode = (SeriesNode *) malloc(new_size);
	if (*snode == (SeriesNode *) NULL)
	    return (DB_NOMEMORY);
	if (HF_ReadRecord(seradd, new_size, (void *) *snode) != HF_NORMAL)
	    return (DB_READERROR);
    } else {
	*snode = (SeriesNode *) malloc(sizeof(SeriesNode));
	if (*snode == (SeriesNode *) NULL)
	    return (DB_NOMEMORY);
	**snode = temp;
    }

    return (DB_NORMAL);
}
/* DB_WriteSeriesNode
**
** Purpose:
**	DB_WriteSeriesNode writes a series node to the database.
**
** Parameter Dictionary:
**	HunkBufAdd *seradd:
**		The hunk address of the series node.
**	SeriesNode *sernode:
**		A pointer to the SeriesNode struct to write to the database.
**
** Return Values:
**
**	DB_NORMAL: 	The series node was written successfully.
**	DB_WRITEERROR:	The record could not be written.
**
** Algorithm:
**	Normally, this would be a straight HF_WriteRecord call, however, Series
**	Nodes (and Study Nodes), are not fixed in length.  Their length depends
**	on how many children they have. Therefore, we must compute the length
**	from information in the record so that we write out the actual length
**	of the structure...this is just a convenience routine.
**
*/
CONDITION
DB_WriteSeriesNode(HunkBufAdd *seradd, SeriesNode *sernode)
{

    int size_to_write;

    size_to_write = sizeof(SeriesNode);
    size_to_write += (sernode->num_allocated - DB_INITSIZE) * sizeof(HunkBufAdd);
    if (HF_WriteRecord(seradd, size_to_write, (void *) sernode) != HF_NORMAL)
	return (DB_WRITERROR);

    return (DB_NORMAL);
}
/* DB_WriteStudyNode
**
** Purpose:
**	DB_WriteStudyNode writes a study node to the database.
**
** Parameter Dictionary:
**	HunkBufAdd *stuadd:
**		The hunk address of the study node.
**	SeriesNode *snode:
**		A pointer to the StudyNode struct to write to the database.
**
** Return Values:
**
**	DB_NORMAL: 	The study node was written successfully.
**	DB_WRITEERROR:	The record could not be written.
**
** Algorithm:
**	Normally, this would be a straight HF_WriteRecord call, however, Study
**	Nodes (and Series Nodes), are not fixed in length.  Their length depends
**	on how many children they have. Therefore, we must compute the length
**	from information in the record so that we write out the actual length
**	of the structure...this is just a convenience routine.
**
*/
CONDITION
DB_WriteStudyNode(HunkBufAdd *stuadd, StudyNode *snode)
{

    int size_to_write;

    size_to_write = sizeof(StudyNode);
    size_to_write += (snode->num_allocated - DB_INITSIZE) * sizeof(HunkBufAdd);
    if (HF_WriteRecord(stuadd, size_to_write, (void *) snode) != HF_NORMAL)
	return (DB_WRITERROR);

    return (DB_NORMAL);
}
/* DB_ReadStudyNode
**
** Purpose:
**	DB_ReadStudyNode reads a study node from the database.
**
** Parameter Dictionary:
**	HunkBufAdd *stuadd:
**		The hunk address of the study node.
**	SeriesNode **snode:
**		A pointer to the StudyNode struct to read from the database.
**
** Return Values:
**
**	DB_NORMAL: 	The study node was read successfully.
**	DB_READERROR:	The record could not be read.
**	DB_NOMEMORY:	Couldn't allocate any memory for the new node.
**
** Algorithm:
**	Normally, this would be a straight HF_ReadRecord call, however, Study
**	Nodes (and Series Nodes), are not fixed in length.  Their length depends
**	on how many children they have.  So, we must read the record with the
**	default length, and then check to see how large it really is, and allocate
**	enough memory to hold this new structure, then re-read it with the proper
**	length.
**
*/
CONDITION
DB_ReadStudyNode(HunkBufAdd *stuadd, StudyNode **snode)
{

    StudyNode temp;
    int new_size;

    if (HF_ReadRecord(stuadd, sizeof(temp), (void *) &temp) != HF_NORMAL)
	return (DB_READERROR);

    if (temp.num_allocated > DB_INITSIZE) {
	new_size = sizeof(StudyNode);
	new_size += (temp.num_allocated - DB_INITSIZE) * sizeof(HunkBufAdd);
	*snode = (StudyNode *) malloc(new_size);
	if (*snode == (StudyNode *) NULL)
	    return (DB_NOMEMORY);
	if (HF_ReadRecord(stuadd, new_size, (void *) *snode) != HF_NORMAL)
	    return (DB_READERROR);
    } else {
	*snode = (StudyNode *) malloc(sizeof(StudyNode));
	if (*snode == (StudyNode *) NULL)
	    return (DB_NOMEMORY);
	**snode = temp;
    }

    return (DB_NORMAL);
}
/* DB_DelPatient
**
** Purpose:
**	Deletes the patient and all descendants from the database.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifier.
**	char *patid:
**		Identifies the patient to be deleted.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The specified patient could not be found.
**	DB_READERROR:	The database record could not be read from the hunk file.
**	DB_?:		Any valid return from DB_DelBelowStudy
**	DB_WRITERROR:	The modified record could not be rewritten.
**
** Algorithm:
**	This routine makes the normal checks for database not open, then tries
**	to locate the patient record specified by patid. Inability to find this
**	record is an error.  Once found, DelPatient removes everything below
**	each study underneath this patient, and eventually removes the patient
**	record itsself.
*/
CONDITION
DB_DelPatient(short dbid, char *patid)
{

    int
        i,
        found,
        pat_inx = 0;
    PatientNode
        pnode;
    StudyNode
       *snode;
    CONDITION
	retval;
    HunkBufAdd
        patadd;


    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_DelPatient: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_DelPatient: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    /*
     * Check for the Patient...
     */
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    pat_inx = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_DelPatient: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    /*
     * Read the Patient Record and remove all the studies for that patient.
     */
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelPatient: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_ReadStudyNode(&(pnode.study_loc[i]), &snode) != DB_NORMAL) {
	    (void) COND_PushCondition(DB_READERROR, "DB_DelPatient: DB_ReadStudyNode failed");
	    return (DB_UnLock(DB_READERROR));
	}
	if ((retval = DB_DelBelowStudy(snode)) != DB_NORMAL) {
	    (void) COND_PushCondition(retval, "DB_DelPatient: DB_DelBelowStudy failed");
	    return (DB_UnLock(retval));
	}
	(void) HF_DeallocateRecord(&(pnode.study_loc[i]));
	free(snode);
    }
    (void) HF_DeallocateRecord(&patadd);
    if (pat_inx == (GS_root.num_patients - 1)) {
	GS_root.patient_loc[pat_inx].hunk_number = HUNK_PTR_NULL;
	GS_root.patient_loc[pat_inx].node_number = HUNK_PTR_NULL;
    } else {
	for (i = pat_inx; i < (GS_root.num_patients - 1); i++) {
	    GS_root.patient_loc[i] = GS_root.patient_loc[i + 1];
	}
	GS_root.patient_loc[GS_root.num_patients - 1].hunk_number = HUNK_PTR_NULL;
	GS_root.patient_loc[GS_root.num_patients - 1].node_number = HUNK_PTR_NULL;
    }

    GS_root.num_patients--;
    if (HF_WriteStaticRecord(0, sizeof(GS_root), (void *) &GS_root) != HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelPatient: HF_WriteStaticRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    (void) HF_IncrementUpdateFlag();
    return (DB_UnLock(DB_NORMAL));
}
/* DB_DelStudy
**
** Purpose:
**	Deletes the study and all its descendants from the database.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifier.
**	char *patid:
**		Identifies the patient to be deleted.
**	char *studyuid:
**		Identifies the study to be deleted.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The specified patient could not be found.
**	DB_READERROR:	The database record could not be read from the hunk file.
**	DB_BADSTUDY:	The specified study could not be found.
**	DB_?:		Any valid return from DB_DelBelowStudy
**	DB_WRITERROR:	The modified record could not be rewritten.
**	DB_?:		Any valid return from DB_DelPatient
**
** Algorithm:
**	This routine makes the normal checks for database not open, then tries
**	to locate the patient record specified by patid and the study record
**	indicated by studyuid. Inability to either of these records is an error.
**	Once found, DelBelow Study removes everything below the study underneath
**	this patient, and then removes the study.
*/
CONDITION
DB_DelStudy(short dbid, char *patid, char *studyuid)
{

    int
        i,
        found,
        study_inx = 0;
    PatientNode
        pnode;
    StudyNode
       *snode;
    CONDITION
	retval;
    HunkBufAdd
        patadd,
        studyadd;


    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_DelStudy: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_DelStudy: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    /*
     * Check for the Patient...
     */
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_DelStudy: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    /*
     * Read the Patient Record and remove all the studies for that patient.
     */
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &(pnode.study_loc[i])) == DB_MATCH) {
	    studyadd = pnode.study_loc[i];
	    study_inx = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_DelStudy: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&studyadd, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if ((retval = DB_DelBelowStudy(snode)) != DB_NORMAL) {
	(void) COND_PushCondition(retval, "DB_DelStudy: DB_DelBelowStudy failed");
	return (DB_UnLock(retval));
    }
    (void) HF_DeallocateRecord(&studyadd);
    free(snode);

    if (study_inx == (pnode.num_studies - 1)) {
	pnode.study_loc[study_inx].hunk_number = HUNK_PTR_NULL;
	pnode.study_loc[study_inx].node_number = HUNK_PTR_NULL;
    } else {
	for (i = study_inx; i < (pnode.num_studies - 1); i++) {
	    pnode.study_loc[i] = pnode.study_loc[i + 1];
	}
	pnode.study_loc[pnode.num_studies - 1].hunk_number = HUNK_PTR_NULL;
	pnode.study_loc[pnode.num_studies - 1].node_number = HUNK_PTR_NULL;
    }

    pnode.num_studies--;

    if (HF_WriteRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelStudy: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    (void) HF_IncrementUpdateFlag();
    return (DB_UnLock(DB_NORMAL));
}
/* DB_DelSeries
**
** Purpose:
**	Deletes the series and all its descendants(images) from the database.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifier.
**	char *patid:
**		Identifies the patient to be deleted.
**	char *studyuid:
**		Identifies the study to be deleted.
**	char *seriesuid:
**		Identifies the series to be deleted.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The specified patient could not be found.
**	DB_READERROR:	The database record could not be read from the hunk file.
**	DB_BADSTUDY:	The specified study could not be found.
**	DB_BADSERIES:	The specified series could not be found.
**	DB_?:		Any valid return from DB_DelBelowSereis
**	DB_WRITERROR:	The modified record could not be rewritten.
**	DB_?:		Any valid return from DB_DelStudy
**
** Algorithm:
**	This routine makes the normal checks for database not open, then tries
**	to locate the patient record specified by patid, the study record
**	indicated by studyuid, and the series record indicated by seriesuid.
**	Inability to retrieve these records is an error.  Once found, DelBelowSeries
**	removes everything below each series underneath this patient and study,
**	and then call DelStudy if this was the last series for this particular study.
**	Recall that DelStudy may delete the patient if this is the last study for that
**	particular patient.  The thing to remember here is that deletion of a single
**	series could well result in the removal of that patient.
*/
CONDITION
DB_DelSeries(short dbid, char *patid, char *studyuid, char *seriesuid)
{

    int
        i,
        found,
        series_inx = 0;
    PatientNode
        pnode;
    StudyNode
       *snode;
    SeriesNode
       *sernode;
    CONDITION
	retval;
    HunkBufAdd
        patadd,
        studyadd,
        seriesadd;


    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelSeries: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_DelSeries: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    /*
     * Check for the Patient...
     */
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_DelSeries: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    /*
     * Read the Patient Record and remove all the studies for that patient.
     */
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &(pnode.study_loc[i])) == DB_MATCH) {
	    studyadd = pnode.study_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_DelSeries: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&studyadd, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < snode->num_series; i++) {
	if (DB_CompareSeriesUID(seriesuid, &(snode->series_loc[i])) == DB_MATCH) {
	    seriesadd = snode->series_loc[i];
	    series_inx = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	free(snode);
	(void) COND_PushCondition(DB_BADSERIES, "DB_DelSeries: Can't find series %s", seriesuid);
	return (DB_UnLock(DB_BADSERIES));
    }
    if (DB_ReadSeriesNode(&seriesadd, &sernode) != DB_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_DelSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if ((retval = DB_DelBelowSeries(sernode)) != DB_NORMAL) {
	free(snode);
	free(sernode);
	(void) COND_PushCondition(retval, "DB_DelSeries: DB_DelBelowSeries failed");
	return (DB_UnLock(retval));
    }
    (void) HF_DeallocateRecord(&seriesadd);
    free(sernode);

    if (series_inx == (snode->num_series - 1)) {
	snode->series_loc[series_inx].hunk_number = HUNK_PTR_NULL;
	snode->series_loc[series_inx].node_number = HUNK_PTR_NULL;
    } else {
	for (i = series_inx; i < (snode->num_series - 1); i++) {
	    snode->series_loc[i] = snode->series_loc[i + 1];
	}
	snode->series_loc[snode->num_series - 1].hunk_number = HUNK_PTR_NULL;
	snode->series_loc[snode->num_series - 1].node_number = HUNK_PTR_NULL;
    }

    (snode->num_series)--;
    if (DB_WriteStudyNode(&studyadd, snode) != DB_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelSeries: HF_WriteRecord failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    free(snode);

    (void) HF_IncrementUpdateFlag();
    return (DB_UnLock(DB_NORMAL));
}
/* DB_DelImage
**
** Purpose:
**	Deletes the image from the database.
**
** Parameter Dictionary:
**	short dbid:
**		The database identifier.
**	char *patid:
**		Identifies the patient to be deleted.
**	char *studyuid:
**		Identifies the study to be deleted.
**	char *seriesuid:
**		Identifies the series to be deleted.
**	char *seriesuid:
**		Identifies the image to be deleted.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADPATIENT:	The specified patient could not be found.
**	DB_READERROR:	The database record could not be read from the hunk file.
**	DB_BADSTUDY:	The specified study could not be found.
**	DB_BADSERIES:	The specified series could not be found.
**	DB_BADIMAGE:	The specified image could not be found.
**	DB_WRITERROR:	The modified record could not be rewritten.
**	DB_?:		Any valid return from DB_DelSeries
**
** Algorithm:
**	This routine makes the normal checks for database not open, then tries
**	to locate the patient record specified by patid, the study record
**	indicated by studyuid, the series record indicated by seriesuid, and the image
**	record specified by imageuid.  Inability to retrieve these records is an
**	error.  Once found, the routine deletes the image record and, if this was
**	the last image of the series, it calls DB_DelSeries, which may call
**	DB_DelStudy, and which may call DB_DelPatient.
*/
CONDITION
DB_DelImage(short dbid, char *patid, char *studyuid, char *seriesuid, char *imageuid)
{

    int
        i,
        bad_file_delete,
        found,
        image_inx = 0;
    PatientNode
        pnode;
    StudyNode
       *snode;
    SeriesNode
       *sernode;
    HunkBufAdd
        patadd,
        studyadd,
        seriesadd,
        imageadd;
    ImageNode
        inode;



    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelImage: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_ExclusiveLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_DelImage: HF_ExclusiveLock failed");
	return (DB_LOCKERROR);
    }
    /*
     * Check for the Patient...
     */
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    patadd = GS_root.patient_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_DelImage: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    /*
     * Read the Patient Record and remove all the studies for that patient.
     */
    if (HF_ReadRecord(&patadd, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &(pnode.study_loc[i])) == DB_MATCH) {
	    studyadd = pnode.study_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_DelImage: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&studyadd, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_DelImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < snode->num_series; i++) {
	if (DB_CompareSeriesUID(seriesuid, &(snode->series_loc[i])) == DB_MATCH) {
	    seriesadd = snode->series_loc[i];
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	free(snode);
	(void) COND_PushCondition(DB_BADSERIES, "DB_DelImage: Can't find series %s", seriesuid);
	return (DB_UnLock(DB_BADSERIES));
    }
    if (DB_ReadSeriesNode(&seriesadd, &sernode) != DB_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_DelImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < sernode->num_images; i++) {
	if (DB_CompareImageUID(imageuid, &(sernode->image_loc[i])) == DB_MATCH) {
	    imageadd = sernode->image_loc[i];
	    image_inx = i;
	    found = 1;
	    break;
	}
    }

    if (found == 0) {
    	free(sernode);
    	free(snode);
    	(void) COND_PushCondition(DB_BADIMAGE, "DB_DelImage: Can't find image %s", imageuid);
    	return (DB_UnLock(DB_BADIMAGE));
    }
    if (HF_ReadRecord(&imageadd, sizeof(inode), (void *) &inode) != HF_NORMAL) {
    	(void) COND_PushCondition(DB_READERROR, "DB_DelImage: HF_ReadRecord failed");
    	return (DB_UnLock(DB_READERROR));
    }
    /*
     * Delete the image file from the filesystem...
     */
    bad_file_delete = 0;
    if (unlink(inode.image.FileName) != 0) {
    	(void) COND_PushCondition(DB_FILEDELERROR, "DB_DelImage: Image file delete failed");
    	bad_file_delete = 1;
    }
    (void) HF_DeallocateRecord(&imageadd);

    if (image_inx == (sernode->num_images - 1)) {
    	sernode->image_loc[image_inx].hunk_number = HUNK_PTR_NULL;
    	sernode->image_loc[image_inx].node_number = HUNK_PTR_NULL;
    } else {
    	for (i = image_inx; i < (sernode->num_images - 1); i++) {
    		sernode->image_loc[i] = sernode->image_loc[i + 1];
    	}
    	sernode->image_loc[sernode->num_images - 1].hunk_number = HUNK_PTR_NULL;
    	sernode->image_loc[sernode->num_images - 1].node_number = HUNK_PTR_NULL;
    }

    (sernode->num_images)--;

    if (DB_WriteSeriesNode(&seriesadd, sernode) != DB_NORMAL) {
	free(snode);
	free(sernode);
	(void) COND_PushCondition(DB_WRITERROR, "DB_DelImage: HF_WriteSeriesNode failed");
	return (DB_UnLock(DB_WRITERROR));
    }
    free(snode);
    free(sernode);
    (void) HF_IncrementUpdateFlag();
    if (bad_file_delete)
	return (DB_UnLock(DB_FILEDELERROR));
    else
	return (DB_UnLock(DB_NORMAL));
}
/* DB_DelBelowStudy
**
** Purpose:
**	Deletes everything below the study node specified.
**
** Parameter Dictionary:
**	StudyNode *snode:
**		Identifies the study node.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**	DB_?:		Any valid return from DB_ReadSeriesNode
**	DB_?:		Any valid return from DB_DelBelowSeries
**
** Algorithm:
**	This is a utility routine for the main delete interface.  This routine
**	simply locates each series node for the specified study and calls
**	DB_DelBelowSeries to remove each series and any descendents.
*/
CONDITION
DB_DelBelowStudy(StudyNode *snode)
{

    CONDITION
	retval;
    SeriesNode
       *sernode;
    int
        i;

    for (i = 0; i < snode->num_series; i++) {
	if ((retval = DB_ReadSeriesNode(&(snode->series_loc[i]), &sernode)) != DB_NORMAL) {
	    return (retval);
	}
	if ((retval = DB_DelBelowSeries(sernode)) != DB_NORMAL) {
	    return (retval);
	}
	(void) HF_DeallocateRecord(&(snode->series_loc[i]));
	free(sernode);
    }

    return (DB_NORMAL);
}
/* DB_DelBelowSeries
**
** Purpose:
**	Deletes everything below the series node specified.
**
** Parameter Dictionary:
**	SeriesNode *snode:
**		Identifies the series node.
**
** Return Values:
**	DB_NORMAL:	The delete operation succeeded.
**
** Algorithm:
**	This is a utility routine for the main delete interface.  This routine
**	simply locates each image node for the specified series and deletes
**	the image node from the database.  It has the additional effect of removing
**	the image file from the filesystem.
*/
CONDITION
DB_DelBelowSeries(SeriesNode *snode)
{
    ImageNode
        inode;
    int
        i;

    for (i = 0; i < snode->num_images; i++) {
	if (HF_ReadRecord(&(snode->image_loc[i]),
			  sizeof(inode), (void *) &inode) != HF_NORMAL) {
	    (void) COND_PushCondition(DB_READERROR, "DB_DelBelowSeries: HF_ReadRecord failed");
	    return (DB_READERROR);
	}
	/*
	 * Delete the image file from the filesystem...we're not going to
	 * return an error here if the unlink fails...it would get just too
	 * messy... It's not that big a deal anyway...it has nothing to do
	 * with database integrity and the database shouldn't be doing this
	 * stuff anyway...
	 */
	(void) unlink(inode.image.FileName);
	(void) HF_DeallocateRecord(&(snode->image_loc[i]));
    }

    return (DB_NORMAL);
}
/* DB_GetPatient
**
** Purpose:
**	Retrieves the first patient record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	PatientLevel *patient:
**		A pointer to an existing patient level struct.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_NOPATIENTS:	There are no patients in the database.
**	DB_READERROR:	Could not read the patient records.
**
** Algorithm:
**	This routine is used in conjuction with DB_GetNextPatient.  Calling this
**	routine will return the first patient record to the caller, and then
**	will save context for subsequent calls to DB_GetNextPatient.
*/
CONDITION
DB_GetPatient(short dbid, PatientLevel *patient)
{

    PatientNode
        pnode;
    HunkBufAdd
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetPatient: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (GS_root.num_patients <= 0) {
	(void) COND_PushCondition(DB_NOPATIENTS, "DB_GetPatient: No patients");
	return (DB_NOPATIENTS);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetPatient: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    ploc = GS_root.patient_loc[0];

    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetPatient: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *patient = pnode.pat;

    DB_UpdatePatientContext(dbid, &ploc, 0, DB_PATIENTCONTEXT);
    ploc.hunk_number = HUNK_PTR_NULL;
    ploc.node_number = HUNK_PTR_NULL;
    DB_UpdatePatientContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
    DB_UpdatePatientContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
    DB_UpdatePatientContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    DB_SetChangeFlag(dbid);

    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetImage
**
** Purpose:
**	Retrieves the first image record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	char *patid:
**		The patient ID to retrieve.
**	char *studyuid:
**		The study UID to retrieve.
**	char *seriesuid:
**		The series UID to retrieve.
**	ImageLevel *image:
**		A pointer to the image level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_BADPATIENT:	There are no patients that match patid.
**	DB_BADSTUDY:	There are no studies that match studyuid.
**	DB_READERROR:	Could not read required records.
**	DB_NOMORE:	No images match the selection criteria.
**
** Algorithm:
**	This routine is used in conjuction with DB_GetNextImage.  Calling this
**	routine will return the first image record to the caller, and then
**	will save context for subsequent calls to DB_GetNextImage.
*/
CONDITION
DB_GetImage(short dbid, char *patid, char *studyuid, char *seriesuid, ImageLevel *image)
{

    int i,
        patindex = 0,
        studyindex = 0,
        seriesindex = 0,
        found;
    PatientNode
        pnode;
    ImageNode
        inode;
    StudyNode
       *snode;
    SeriesNode
       *sernode;
    HunkBufAdd
        iloc,
        serloc,
        sloc,
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetImage: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetImage: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    ploc = GS_root.patient_loc[i];
	    patindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_GetImage: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &pnode.study_loc[i]) == DB_MATCH) {
	    sloc = pnode.study_loc[i];
	    studyindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_GetImage: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&sloc, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if (snode->num_series <= 0) {
	free(snode);
	(void) COND_PushCondition(DB_NOSERIES, "DB_GetImage: No series found");
	return (DB_UnLock(DB_NOSERIES));
    }
    found = 0;
    for (i = 0; i < snode->num_series; i++) {
	if (DB_CompareSeriesUID(seriesuid, &snode->series_loc[i]) == DB_MATCH) {
	    serloc = snode->series_loc[i];
	    seriesindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSERIES, "DB_GetImage: Can't find series %s", seriesuid);
	return (DB_UnLock(DB_BADSERIES));
    }
    if (DB_ReadSeriesNode(&serloc, &sernode) != DB_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_GetImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if (sernode->num_images <= 0) {
	free(snode);
	return (DB_UnLock(DB_NOMORE));
    }
    iloc = sernode->image_loc[0];

    if (HF_ReadRecord(&iloc, sizeof(inode), (void *) &inode) != HF_NORMAL) {
	free(snode);
	free(sernode);
	(void) COND_PushCondition(DB_READERROR, "DB_GetImage: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *image = inode.image;

    DB_UpdateImageContext(dbid, &ploc, patindex, DB_PATIENTCONTEXT);
    DB_UpdateImageContext(dbid, &sloc, studyindex, DB_STUDYCONTEXT);
    DB_UpdateImageContext(dbid, &serloc, seriesindex, DB_SERIESCONTEXT);
    DB_UpdateImageContext(dbid, &iloc, 0, DB_IMAGECONTEXT);
    DB_SetChangeFlag(dbid);

    free(snode);
    free(sernode);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetSeries
**
** Purpose:
**	Retrieves the first series record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	char *patid:
**		The patient ID to retrieve.
**	char *studyuid:
**		The study UID to retrieve.
**	SeriesLevel *series:
**		A pointer to the series level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_BADPATIENT:	There are no patients that match patid.
**	DB_BADSTUDY:	There are no studies that match studyuid.
**	DB_NOSERIES:	There are no series for this patient/study pair.
**	DB_READERROR:	Could not read required records.
**
** Algorithm:
**	This routine is used in conjuction with DB_GetNextSeries.  Calling this
**	routine will return the first series record to the caller, and then
**	will save context for subsequent calls to DB_GetNextSeries.
*/
CONDITION
DB_GetSeries(short dbid, char *patid, char *studyuid, SeriesLevel *series)
{

    int i,
        patindex = 0,
        studyindex = 0,
        found;
    PatientNode
        pnode;
    StudyNode
       *snode;
    SeriesNode
        sernode;
    HunkBufAdd
        serloc,
        sloc,
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetSeries: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetSeries: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    ploc = GS_root.patient_loc[i];
	    patindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_GetSeries: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    found = 0;
    for (i = 0; i < pnode.num_studies; i++) {
	if (DB_CompareStudyUID(studyuid, &pnode.study_loc[i]) == DB_MATCH) {
	    sloc = pnode.study_loc[i];
	    studyindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADSTUDY, "DB_GetSeries: Can't find study %s", studyuid);
	return (DB_UnLock(DB_BADSTUDY));
    }
    if (DB_ReadStudyNode(&sloc, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if (snode->num_series <= 0) {
	free(snode);
	(void) COND_PushCondition(DB_NOSERIES, "DB_GetSeries: No series available");
	return (DB_UnLock(DB_NOSERIES));
    }
    serloc = snode->series_loc[0];

    if (HF_ReadRecord(&serloc, sizeof(sernode), (void *) &sernode) != HF_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_GetSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *series = sernode.series;

    DB_UpdateSeriesContext(dbid, &ploc, patindex, DB_PATIENTCONTEXT);
    DB_UpdateSeriesContext(dbid, &sloc, studyindex, DB_STUDYCONTEXT);
    DB_UpdateSeriesContext(dbid, &serloc, 0, DB_SERIESCONTEXT);
    ploc.hunk_number = HUNK_PTR_NULL;
    ploc.node_number = HUNK_PTR_NULL;
    DB_UpdateSeriesContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    DB_SetChangeFlag(dbid);

    free(snode);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetStudy
**
** Purpose:
**	Retrieves the first study record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	char *patid:
**		The patient ID to retrieve.
**	StudyLevel *study:
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_BADPATIENT:	There are no patients that match patid.
**	DB_NOSTUDIES:	There are no studies for this patient.
**	DB_READERROR:	Could not read required records.
**
** Algorithm:
**	This routine is used in conjuction with DB_GetNextStudy.  Calling this
**	routine will return the first study record to the caller, and then
**	will save context for subsequent calls to DB_GetNextStudy.
*/
CONDITION
DB_GetStudy(short dbid, char *patid, StudyLevel *study)
{

    int i,
        patindex = 0,
        found;
    PatientNode
        pnode;
    StudyNode
        snode;
    HunkBufAdd
        sloc,
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetStudy: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetStudy: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    found = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	if (DB_ComparePatID(patid, &GS_root.patient_loc[i]) == DB_MATCH) {
	    ploc = GS_root.patient_loc[i];
	    patindex = i;
	    found = 1;
	    break;
	}
    }
    if (found == 0) {
	(void) COND_PushCondition(DB_BADPATIENT, "DB_GetStudy: Can't find patient %s", patid);
	return (DB_UnLock(DB_BADPATIENT));
    }
    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    if (pnode.num_studies <= 0) {
	(void) COND_PushCondition(DB_NOSTUDIES, "DB_GetStudy: No studies available");
	return (DB_UnLock(DB_NOSTUDIES));
    }
    sloc = pnode.study_loc[0];

    if (HF_ReadRecord(&sloc, sizeof(snode), (void *) &snode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *study = snode.study;

    DB_UpdateStudyContext(dbid, &ploc, patindex, DB_PATIENTCONTEXT);
    DB_UpdateStudyContext(dbid, &sloc, 0, DB_STUDYCONTEXT);
    ploc.hunk_number = HUNK_PTR_NULL;
    ploc.node_number = HUNK_PTR_NULL;
    DB_UpdateStudyContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
    DB_UpdateStudyContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    DB_SetChangeFlag(dbid);

    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetNextImage
**
** Purpose:
**	Retrieves the next image record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	ImageLevel *image:
**		A pointer to the image level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_NOMORE:	No more image records are available.
**	DB_READERROR:	Could not read required records.
**	DB_CHANGED:	The database has changed since the initial
**			DB_GetImage call.
**
** Algorithm:
**	This routine is generally called after DB_GetImage.  It uses the context
**	from the previous call to determine which record to return.  It then
**	updates the context in preparation for the next call or nulls it out if
**	there are no more records left to retreive.
*/
CONDITION
DB_GetNextImage(short dbid, ImageLevel *image)
{

    HunkBufAdd
        iloc,
        serloc,
        sloc,
        ploc;
    int
        imageindex,
        serindex,
        sindex,
        pindex;
    ImageNode
        inode;
    SeriesNode
       *sernode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetNextImage: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetNextImage: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    if (DB_CompareChangeFlag(dbid) != DB_MATCH) {
	(void) COND_PushCondition(DB_CHANGED, "DB_GetNextImage: Database has been changed");
	return (DB_UnLock(DB_CHANGED));
    }
    DB_GetImageContext(dbid, &ploc, &pindex, DB_PATIENTCONTEXT);
    DB_GetImageContext(dbid, &sloc, &sindex, DB_STUDYCONTEXT);
    DB_GetImageContext(dbid, &serloc, &serindex, DB_SERIESCONTEXT);
    DB_GetImageContext(dbid, &iloc, &imageindex, DB_IMAGECONTEXT);

    if (ploc.hunk_number == HUNK_PTR_NULL ||
	ploc.node_number == HUNK_PTR_NULL ||
	sloc.hunk_number == HUNK_PTR_NULL ||
	sloc.node_number == HUNK_PTR_NULL ||
	serloc.hunk_number == HUNK_PTR_NULL ||
	serloc.node_number == HUNK_PTR_NULL ||
	iloc.hunk_number == HUNK_PTR_NULL ||
	iloc.node_number == HUNK_PTR_NULL ||
	pindex == -1 || sindex == -1 || serindex == -1 || imageindex == -1)
	return (DB_UnLock(DB_NOMORE));

    if (DB_ReadSeriesNode(&serloc, &sernode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextImage: DB_ReadSeriesNode failed");
	return (DB_UnLock(DB_READERROR));
    }
    imageindex++;
    if (imageindex >= sernode->num_images) {
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateImageContext(dbid, &ploc, -1, DB_PATIENTCONTEXT);
	DB_UpdateImageContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
	DB_UpdateImageContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdateImageContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
	free(sernode);
	return (DB_UnLock(DB_NOMORE));
    }
    iloc = sernode->image_loc[imageindex];

    if (HF_ReadRecord(&iloc, sizeof(inode), (void *) &inode) != HF_NORMAL) {
	free(sernode);
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextImage: HF_ReadRecord failed");
	return (DB_READERROR);
    }
    *image = inode.image;

    DB_UpdateImageContext(dbid, &iloc, imageindex, DB_IMAGECONTEXT);

    free(sernode);
    return (DB_NORMAL);
}
/* DB_GetNextSeries
**
** Purpose:
**	Retrieves the next series record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	SeriesLevel *series:
**		A pointer to the series level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_NOMORE:	No more series records are available.
**	DB_READERROR:	Could not read required records.
**	DB_CHANGED:	The database has changed since the initial
**			DB_GetSeries call.
**
** Algorithm:
**	This routine is generally called after DB_GetSeries.  It uses the context
**	from the previous call to determine which record to return.  It then
**	updates the context in preparation for the next call or nulls it out if
**	there are no more records left to retreive.
*/
CONDITION
DB_GetNextSeries(short dbid, SeriesLevel *series)
{

    HunkBufAdd
        serloc,
        sloc,
        ploc;
    int
        serindex,
        sindex,
        pindex;
    SeriesNode
        sernode;
    StudyNode
       *snode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetNextSeries: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetNextSeries: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    if (DB_CompareChangeFlag(dbid) != DB_MATCH) {
	(void) COND_PushCondition(DB_CHANGED, "DB_GetNextSeries: Database has been changed");
	return (DB_UnLock(DB_CHANGED));
    }
    DB_GetSeriesContext(dbid, &ploc, &pindex, DB_PATIENTCONTEXT);
    DB_GetSeriesContext(dbid, &sloc, &sindex, DB_STUDYCONTEXT);
    DB_GetSeriesContext(dbid, &serloc, &serindex, DB_SERIESCONTEXT);

    if (ploc.hunk_number == HUNK_PTR_NULL ||
	ploc.node_number == HUNK_PTR_NULL ||
	sloc.hunk_number == HUNK_PTR_NULL ||
	sloc.node_number == HUNK_PTR_NULL ||
	serloc.hunk_number == HUNK_PTR_NULL ||
	serloc.node_number == HUNK_PTR_NULL ||
	pindex == -1 || sindex == -1 || serindex == -1)
	return (DB_UnLock(DB_NOMORE));

    if (DB_ReadStudyNode(&sloc, &snode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextSeries: DB_ReadStudyNode failed");
	return (DB_UnLock(DB_READERROR));
    }
    serindex++;
    if (serindex >= snode->num_series) {
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateSeriesContext(dbid, &ploc, -1, DB_PATIENTCONTEXT);
	DB_UpdateSeriesContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
	DB_UpdateSeriesContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdateSeriesContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
	free(snode);
	return (DB_UnLock(DB_NOMORE));
    }
    serloc = snode->series_loc[serindex];

    if (HF_ReadRecord(&serloc, sizeof(sernode), (void *) &sernode) != HF_NORMAL) {
	free(snode);
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextSeries: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *series = sernode.series;

    DB_UpdateSeriesContext(dbid, &sloc, sindex, DB_STUDYCONTEXT);
    DB_UpdateSeriesContext(dbid, &serloc, serindex, DB_SERIESCONTEXT);

    free(snode);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetNextStudy
**
** Purpose:
**	Retrieves the next study record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	StudyLevel *study:
**		A pointer to the study level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_NOMORE:	No more study records are available.
**	DB_READERROR:	Could not read required records.
**	DB_CHANGED:	The database has changed since the initial
**			DB_GetStudy call.
**
** Algorithm:
**	This routine is generally called after DB_GetStudy.  It uses the context
**	from the previous call to determine which record to return.  It then
**	updates the context in preparation for the next call or nulls it out if
**	there are no more records left to retreive.
*/
CONDITION
DB_GetNextStudy(short dbid, StudyLevel *study)
{

    HunkBufAdd
        sloc,
        ploc;
    int
        sindex,
        pindex;
    StudyNode
        snode;
    PatientNode
        pnode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetNextStudy: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetNextStudy: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    if (DB_CompareChangeFlag(dbid) != DB_MATCH) {
	(void) COND_PushCondition(DB_CHANGED, "DB_GetNextStudy: Database has been changed");
	return (DB_UnLock(DB_CHANGED));
    }
    DB_GetStudyContext(dbid, &ploc, &pindex, DB_PATIENTCONTEXT);
    DB_GetStudyContext(dbid, &sloc, &sindex, DB_STUDYCONTEXT);

    if (ploc.hunk_number == HUNK_PTR_NULL ||
	ploc.node_number == HUNK_PTR_NULL ||
	sloc.hunk_number == HUNK_PTR_NULL ||
	sloc.node_number == HUNK_PTR_NULL ||
	pindex == -1 || sindex == -1)
	return (DB_UnLock(DB_NOMORE));

    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    sindex++;
    if (sindex >= pnode.num_studies) {
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateStudyContext(dbid, &ploc, -1, DB_PATIENTCONTEXT);
	DB_UpdateStudyContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
	DB_UpdateStudyContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdateStudyContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
	return (DB_UnLock(DB_NOMORE));
    }
    sloc = pnode.study_loc[sindex];

    if (HF_ReadRecord(&sloc, sizeof(snode), (void *) &snode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextStudy: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *study = snode.study;

    DB_UpdateStudyContext(dbid, &sloc, sindex, DB_STUDYCONTEXT);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_GetNextPatient
**
** Purpose:
**	Retrieves the next patient record from the database specified.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	PatientLevel *patient:
**		A pointer to the patient level struct to fill.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_NOMORE:	No more patient records are available.
**	DB_READERROR:	Could not read required records.
**	DB_CHANGED:	The database has changed since the initial
**			DB_GetPatient call.
**
** Algorithm:
**	This routine is generally called after DB_GetPatient.  It uses the context
**	from the previous call to determine which record to return.  It then
**	updates the context in preparation for the next call or nulls it out if
**	there are no more records left to retreive.
*/
CONDITION
DB_GetNextPatient(short dbid, PatientLevel *patient)
{

    HunkBufAdd
        ploc;
    int
        pindex;
    PatientNode
        pnode;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetNextPatient: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetNextPatient: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    if (DB_CompareChangeFlag(dbid) != DB_MATCH) {
	(void) COND_PushCondition(DB_CHANGED, "DB_GetNextPatient: Database has been changed");
	return (DB_UnLock(DB_CHANGED));
    }
    DB_GetPatientContext(dbid, &ploc, &pindex, DB_PATIENTCONTEXT);

    if (ploc.hunk_number == HUNK_PTR_NULL ||
	ploc.node_number == HUNK_PTR_NULL ||
	pindex == -1)
	return (DB_UnLock(DB_NOMORE));

    pindex++;
    if (pindex >= GS_root.num_patients) {
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdatePatientContext(dbid, &ploc, -1, DB_PATIENTCONTEXT);
	DB_UpdatePatientContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
	DB_UpdatePatientContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdatePatientContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
	return (DB_UnLock(DB_NOMORE));
    }
    ploc = GS_root.patient_loc[pindex];

    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_GetNextPatient: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    *patient = pnode.pat;

    DB_UpdatePatientContext(dbid, &ploc, pindex, DB_PATIENTCONTEXT);
    return (DB_UnLock(DB_NORMAL));
}
/* DB_UpdatePatientContext
**
*/
void
DB_UpdatePatientContext(short dbid, HunkBufAdd *p, int index, int level)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		temp->pacontxt.last_patient = *p;
		temp->pacontxt.last_patientindex = index;
		break;
	    case DB_STUDYCONTEXT:
		temp->pacontxt.last_study = *p;
		temp->pacontxt.last_studyindex = index;
		break;
	    case DB_SERIESCONTEXT:
		temp->pacontxt.last_series = *p;
		temp->pacontxt.last_seriesindex = index;
		break;
	    case DB_IMAGECONTEXT:
		temp->pacontxt.last_image = *p;
		temp->pacontxt.last_imageindex = index;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_UpdateStudyContext
**
*/
void
DB_UpdateStudyContext(short dbid, HunkBufAdd *p, int index, int level)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		temp->stcontxt.last_patient = *p;
		temp->stcontxt.last_patientindex = index;
		break;
	    case DB_STUDYCONTEXT:
		temp->stcontxt.last_study = *p;
		temp->stcontxt.last_studyindex = index;
		break;
	    case DB_SERIESCONTEXT:
		temp->stcontxt.last_series = *p;
		temp->stcontxt.last_seriesindex = index;
		break;
	    case DB_IMAGECONTEXT:
		temp->stcontxt.last_image = *p;
		temp->stcontxt.last_imageindex = index;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_UpdateSeriesContext
**
*/
void
DB_UpdateSeriesContext(short dbid, HunkBufAdd *p, int index, int level)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		temp->secontxt.last_patient = *p;
		temp->secontxt.last_patientindex = index;
		break;
	    case DB_STUDYCONTEXT:
		temp->secontxt.last_study = *p;
		temp->secontxt.last_studyindex = index;
		break;
	    case DB_SERIESCONTEXT:
		temp->secontxt.last_series = *p;
		temp->secontxt.last_seriesindex = index;
		break;
	    case DB_IMAGECONTEXT:
		temp->secontxt.last_image = *p;
		temp->secontxt.last_imageindex = index;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_UpdateQueryContext
**
*/
void
DB_UpdateQueryContext(short dbid, HunkBufAdd *p, int index, int level)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		temp->querycontxt.last_patient = *p;
		temp->querycontxt.last_patientindex = index;
		break;
	    case DB_STUDYCONTEXT:
		temp->querycontxt.last_study = *p;
		temp->querycontxt.last_studyindex = index;
		break;
	    case DB_SERIESCONTEXT:
		temp->querycontxt.last_series = *p;
		temp->querycontxt.last_seriesindex = index;
		break;
	    case DB_IMAGECONTEXT:
		temp->querycontxt.last_image = *p;
		temp->querycontxt.last_imageindex = index;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_UpdateImageContext
**
*/
void
DB_UpdateImageContext(short dbid, HunkBufAdd *p, int index, int level)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		temp->imcontxt.last_patient = *p;
		temp->imcontxt.last_patientindex = index;
		break;
	    case DB_STUDYCONTEXT:
		temp->imcontxt.last_study = *p;
		temp->imcontxt.last_studyindex = index;
		break;
	    case DB_SERIESCONTEXT:
		temp->imcontxt.last_series = *p;
		temp->imcontxt.last_seriesindex = index;
		break;
	    case DB_IMAGECONTEXT:
		temp->imcontxt.last_image = *p;
		temp->imcontxt.last_imageindex = index;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetPatientContext
**
*/
void
DB_GetPatientContext(short dbid, HunkBufAdd *loc, int *index, int level)
{

    DBidstruct
       *temp;

    loc->hunk_number = HUNK_PTR_NULL;
    loc->node_number = HUNK_PTR_NULL;
    *index = -1;
    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		*loc = temp->pacontxt.last_patient;
		*index = temp->pacontxt.last_patientindex;
		break;
	    case DB_STUDYCONTEXT:
		*loc = temp->pacontxt.last_study;
		*index = temp->pacontxt.last_studyindex;
		break;
	    case DB_SERIESCONTEXT:
		*loc = temp->pacontxt.last_series;
		*index = temp->pacontxt.last_seriesindex;
		break;
	    case DB_IMAGECONTEXT:
		*loc = temp->pacontxt.last_image;
		*index = temp->pacontxt.last_imageindex;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetStudyContext
**
*/
void
DB_GetStudyContext(short dbid, HunkBufAdd *loc, int *index, int level)
{

    DBidstruct
       *temp;

    loc->hunk_number = HUNK_PTR_NULL;
    loc->node_number = HUNK_PTR_NULL;
    *index = -1;
    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		*loc = temp->stcontxt.last_patient;
		*index = temp->stcontxt.last_patientindex;
		break;
	    case DB_STUDYCONTEXT:
		*loc = temp->stcontxt.last_study;
		*index = temp->stcontxt.last_studyindex;
		break;
	    case DB_SERIESCONTEXT:
		*loc = temp->stcontxt.last_series;
		*index = temp->stcontxt.last_seriesindex;
		break;
	    case DB_IMAGECONTEXT:
		*loc = temp->stcontxt.last_image;
		*index = temp->stcontxt.last_imageindex;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetSeriesContext
**
*/
void
DB_GetSeriesContext(short dbid, HunkBufAdd *loc, int *index, int level)
{

    DBidstruct
       *temp;

    loc->hunk_number = HUNK_PTR_NULL;
    loc->node_number = HUNK_PTR_NULL;
    *index = -1;
    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		*loc = temp->secontxt.last_patient;
		*index = temp->secontxt.last_patientindex;
		break;
	    case DB_STUDYCONTEXT:
		*loc = temp->secontxt.last_study;
		*index = temp->secontxt.last_studyindex;
		break;
	    case DB_SERIESCONTEXT:
		*loc = temp->secontxt.last_series;
		*index = temp->secontxt.last_seriesindex;
		break;
	    case DB_IMAGECONTEXT:
		*loc = temp->secontxt.last_image;
		*index = temp->secontxt.last_imageindex;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetImageContext
**
*/
void
DB_GetImageContext(short dbid, HunkBufAdd *loc, int *index, int level)
{

    DBidstruct
       *temp;

    loc->hunk_number = HUNK_PTR_NULL;
    loc->node_number = HUNK_PTR_NULL;
    *index = -1;
    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		*loc = temp->imcontxt.last_patient;
		*index = temp->imcontxt.last_patientindex;
		break;
	    case DB_STUDYCONTEXT:
		*loc = temp->imcontxt.last_study;
		*index = temp->imcontxt.last_studyindex;
		break;
	    case DB_SERIESCONTEXT:
		*loc = temp->imcontxt.last_series;
		*index = temp->imcontxt.last_seriesindex;
		break;
	    case DB_IMAGECONTEXT:
		*loc = temp->imcontxt.last_image;
		*index = temp->imcontxt.last_imageindex;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetQueryContext
**
*/
void
DB_GetQueryContext(short dbid, HunkBufAdd *loc, int *index, int level)
{

    DBidstruct
       *temp;

    loc->hunk_number = HUNK_PTR_NULL;
    loc->node_number = HUNK_PTR_NULL;
    *index = -1;
    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    switch (level) {
	    case DB_PATIENTCONTEXT:
		*loc = temp->querycontxt.last_patient;
		*index = temp->querycontxt.last_patientindex;
		break;
	    case DB_STUDYCONTEXT:
		*loc = temp->querycontxt.last_study;
		*index = temp->querycontxt.last_studyindex;
		break;
	    case DB_SERIESCONTEXT:
		*loc = temp->querycontxt.last_series;
		*index = temp->querycontxt.last_seriesindex;
		break;
	    case DB_IMAGECONTEXT:
		*loc = temp->querycontxt.last_image;
		*index = temp->querycontxt.last_imageindex;
		break;
	    }
	}
	temp = temp->next;
    }
    return;
}
/* DB_GetNumberofStudies
**
** Purpose:
**	Retrieves the total number of studies in the database.
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**	char *numstudies:
**		Receives the total number of studies in the database.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_READERROR:	Could not read required records.
**
** Algorithm:
**	Very simple routine to look at all the patients and retrieve the
**	number of studies each has.  Accumulating this value retreives the
**	the total number of studies in the database.
*/
CONDITION
DB_GetNumberofStudies(short dbid, long *numstudies)
{

    int i,
        num;
    PatientNode
        pnode;
    HunkBufAdd
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_GetNumberofStudies: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_GetNumberofStudies: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    num = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	ploc = GS_root.patient_loc[i];
	if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	    (void) COND_PushCondition(DB_READERROR, "DB_GetNumberofStudies: HF_ReadRecord failed");
	    return (DB_UnLock(DB_READERROR));
	}
	num += pnode.num_studies;
    }

    *numstudies = num;

    return (DB_UnLock(DB_NORMAL));
}
/* DB_DelOldestStudy
**
** Purpose:
**	Deletes the oldest study in the database..
**
** Parameter Dictionary:
**	short dbid:
**		Identifies the database to use.
**
** Return Values:
**	DB_NORMAL:	The Get operation succeeded.
**	DB_LOCKERROR:	Could not acquire a requested lock
**	DB_NOTOPENED:	The database specified by dbid is not open.
**	DB_READERROR:	Could not read required records.
**	DB_?:		Any valid return from DB_DelStudy.
**
** Algorithm:
**	Very simple routine to look at all the studies and delete the oldest one.
*/
CONDITION
DB_DelOldestStudy(short dbid)
{

    CONDITION
	ret_val;
    char
        patid[DB_MAXKEYLENGTH],
        studyuid[DB_MAXKEYLENGTH];
    int
        delpat,
        i,
        j;
    PatientNode
        pnode;
    StudyNode
        snode;
    HunkBufAdd
        sloc,
        ploc;
    time_t
	ts;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_DelOldestStudy: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_DelOldestStudy: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    delpat = 0;
    ts = 0;
    for (i = 0; i < GS_root.num_patients; i++) {
	ploc = GS_root.patient_loc[i];
	if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	    (void) COND_PushCondition(DB_READERROR, "DB_DelOldestStudy: HF_ReadRecord failed");
	    return (DB_UnLock(DB_READERROR));
	}
	if (pnode.num_studies == 1)
	    delpat = 1;
	for (j = 0; j < pnode.num_studies; j++) {
	    sloc = pnode.study_loc[j];
	    if (HF_ReadRecord(&sloc, sizeof(snode), (void *) &snode) != HF_NORMAL) {
		(void) COND_PushCondition(DB_READERROR, "DB_DelOldestStudy: HF_ReadRecord failed");
		return (DB_UnLock(DB_READERROR));
	    }
	    if (ts == 0) {
		ts = snode.time_stamp;
		strcpy(patid, pnode.pat.PatID);
		strcpy(studyuid, snode.study.StudyUID);
	    } else {
		if (ts > snode.time_stamp) {
		    ts = snode.time_stamp;
		    strcpy(patid, pnode.pat.PatID);
		    strcpy(studyuid, snode.study.StudyUID);
		}
	    }
	}
    }

    (void) HF_UnLock();

    if ((ret_val = DB_DelStudy(dbid, patid, studyuid)) != DB_NORMAL)
	return (ret_val);

    if (delpat)
	return (DB_DelPatient(dbid, patid));

    return (DB_NORMAL);
}
/* DB_UnLock
**
** Purpose:
**	Unlock the db file and return the proper return code.
**
** Parameter Dictionary:
**	CONDITION ret_val
**		The return value to return after unlocking...
**
** Return Values:
**	DB_?:	The return value will always be what was passed.
**
** Algorithm:
**	Just a utility routine to keep the routines less messy.
*/
CONDITION
DB_UnLock(CONDITION ret_val)
{

    (void) HF_UnLock();
    return (ret_val);
}

/* DB_RegexMatch
**
** Purpose:
**	Perform a DICOM regular expression match with the specified string, stm.
**
** Parameter Dictionary:
**	char *regex:
**		The DICOM regular expression to try and match.
**	char *stm:
**		The input string to match.
**
** Return Values:
**	DB_MATCH:	The input string matched the regular expression.
**	DB_NOMATCH:	The input string did not match the regular expression.
**
** Algorithm:
**	A simple function to perform a DICOM regular expression match with the
**	specified  string, stm.  The sematics of the DICOM patterns must be altered
**	slightly to work correctly with regex under unix...more information may
**	be found below.
**
*/
char *re_comp(char *);
int re_exec(char *);

CONDITION
DB_RegexMatch(char *regex, char *stm)
{

    CONDITION cond;

    cond = UTL_RegexMatch(regex, stm);
    return (cond == UTL_MATCH) ? DB_MATCH : DB_NOMATCH;

/* This code is better replaced by the UTL regular expression matching.
 * It constrains all regular expression matching to one place.
 */
#ifdef OLDREGEX
    int
        ret;
    char
       *new_rstring;

/*lint -e527 Turn off message for unreachable statement (return/break) */
    new_rstring = DB_ConvertRegex(regex);
    if (re_comp(new_rstring) != (char *) 0) {
	free(new_rstring);
	return (DB_NOMATCH);
    } else {
	ret = re_exec(stm);
	switch (ret) {
	case 0:
	case -1:
	    free(new_rstring);
	    return (DB_NOMATCH);
	    break;
	case 1:
	    free(new_rstring);
	    return (DB_MATCH);
	    break;
	}
    }
#endif
    return DB_NOMATCH;		/* We won't get here, but this helps lint */
/*lint +e527 Turn on checking for unreachable statements again */
}

/* DB_ConvertRegex
**
** Purpose:
**	This function converts a DICOM "regular expression" to the proper
**	regex semantics under unix.
**
** Parameter Dictionary:
**	char *regex:
**		The DICOM regular expression to convert.
**
** Return Values:
**	char *:	The converted regular expression which expresses DICOM pattern
**		matching in regex semantics.
**
** Algorithm:
**	Simple function to convert a DICOM "regular expression" to the proper
**	regex semantics under unix.  DICOM has only 2 meta characters, "*" for 0
**	or more occurrences, and "?" for a single character.  The "*" must be
**	converted to ".*" for regex while the "?" must be converted to ".".
**	Other special characters to regex like "[", "]", and "." must also
**	be escaped with the "\".  The DICOM escape character is assumed to be "\".
*/
#ifdef OLDREGEX
#define OFF	0
#define ON	1
char *
DB_ConvertRegex(char *regex)
{

    char
       *new_regex = (char *) NULL;
    int
        malloced_size = 0;
    int
        i,
        j,
        escape_on;

    if (new_regex == (char *) NULL) {
	malloced_size = REGEX_SIZE;
	if ((new_regex = (char *) malloc(malloced_size)) == (char *) NULL) {
	    return ((char *) NULL);
	}
    }
    i = j = 0;
    escape_on = OFF;
    new_regex[j++] = '^';
    while (regex[i] != '\000') {
	switch (regex[i]) {
	case '*':		/* Transform the "*" to ".*" or "\*" if
				 * escaped */
	    switch (escape_on) {
	    case OFF:
		new_regex[j++] = '.';
		break;
	    case ON:
		new_regex[j++] = '\\';
		escape_on = OFF;
		break;
	    }
	    new_regex[j++] = '*';
	    i++;
	    break;
	case '?':		/* Transform the "?" to "." or "?" if escaped */
	    switch (escape_on) {
	    case OFF:
		new_regex[j++] = '.';
		break;
	    case ON:
		new_regex[j++] = '?';
		escape_on = OFF;
		break;
	    }
	    i++;
	    break;
	case '\\':		/* Note that we have seen the escape
				 * character */
	    switch (escape_on) {
	    case OFF:
		escape_on = ON;
		break;
	    case ON:
		escape_on = OFF;
		new_regex[j++] = '\\';
		new_regex[j++] = '\\';
		break;
	    }
	    i++;
	    break;
	case '.':
	case '[':		/* These are special to regex and need to be
				 * escaped */
	case ']':
	    new_regex[j++] = '\\';
	    new_regex[j++] = regex[i++];
	    escape_on = OFF;
	    break;
	default:		/* Leave the "\" in at this juncture */
	    switch (escape_on) {
	    case ON:
		new_regex[j++] = '\\';
		escape_on = OFF;
		break;
	    case OFF:
		break;
	    }
	    new_regex[j++] = regex[i++];
	    break;
	}
	if (j >= (malloced_size - 2)) {
	    malloced_size += REGEX_SIZE;
	    if ((new_regex = (char *) realloc(new_regex, malloced_size)) ==
		(char *) NULL) {
		return ((char *) NULL);
	    }
	}
    }
    new_regex[j++] = '$';
    new_regex[j] = '\000';
    return (new_regex);
}
#endif

/* DB_Query
**
** Purpose:
**	This function performs the a generalized query of the specified database.
**
** Parameter Dictionary:
**	short dbid:
**		The database to query.
**	Query *qstruct:
**		The structure which specifies which fields to match against.
**	Query *retinfo:
**		The structure that receives the retreived data.
**
** Return Values:
**	DB_NORMAL:	The retreive operation succeeded.
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADQUERY:	The query was mal-formed.
**	DB_LOCKERROR:	Requested locks could not be granted.
**	DB_READERROR:	Records could not be read successfully.
**	DB_NOMORE:	No records matched the specifications in qstruct.
**
** Algorithm:
**	This function is always used in conjuction with DB_NextQuery.  It performs
**	a search at the proper level using the DICOM pattern matching specifications.
**	This function sets up the inital context, and returns the first record found
**	in the database with a call to DB_NextQuery.  There is a special flag used to
**	alert DB_NextQuery that this is the initial invocation.  Subsequent records
**	based on this query are returned with DB_NextQuery.
*/
CONDITION
DB_Query(short dbid, Query *qstruct, Query *retinfo)
{
    HunkBufAdd
        ploc;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_Query: DB_Findid failed");
	return (DB_NOTOPENED);
    }
    if (GS_root.num_patients <= 0) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_Query: HF_SharedLock failed");
	return (DB_NOPATIENTS);
    }
    if (qstruct->QueryState & DB_K_CLASSPATSTUDY) {
	if (!(qstruct->QueryState & DB_K_LEVELPAT) &&
	    !(qstruct->QueryState & DB_K_LEVELSTUDY)) {
	    (void) COND_PushCondition(DB_BADQUERY, "DB_Query: Bad state flags in query struct");
	    return (DB_BADQUERY);
	}
    }
    ploc.hunk_number = 0;
    ploc.node_number = 0;
    if (qstruct->QueryState & DB_K_LEVELPAT) {
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_PATIENTCONTEXT);
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_STUDYCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    } else if (qstruct->QueryState & DB_K_LEVELSTUDY) {
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_PATIENTCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_STUDYCONTEXT);
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_SERIESCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    } else if (qstruct->QueryState & DB_K_LEVELSERIES) {
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_PATIENTCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_STUDYCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_SERIESCONTEXT);
	ploc.hunk_number = HUNK_PTR_NULL;
	ploc.node_number = HUNK_PTR_NULL;
	DB_UpdateQueryContext(dbid, &ploc, -1, DB_IMAGECONTEXT);
    } else if (qstruct->QueryState & DB_K_LEVELIMAGE) {
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_PATIENTCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_STUDYCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_SERIESCONTEXT);
	DB_UpdateQueryContext(dbid, &ploc, DB_QUERYFIRST, DB_IMAGECONTEXT);
    }
    DB_SetChangeFlag(dbid);
    return (DB_NextQuery(dbid, qstruct, retinfo));

}

/* DB_SetChangeFlag
**
*/
void
DB_SetChangeFlag(short dbid)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    temp->dbchanged = HF_ReadUpdateFlag();
	    break;
	}
	temp = temp->next;
    }
    return;
}

/* DB_CompareChangeFlag
**
*/
CONDITION
DB_CompareChangeFlag(short dbid)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return DB_NOMATCH;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    if (temp->dbchanged == HF_ReadUpdateFlag())
		return (DB_MATCH);
	    break;
	}
	temp = temp->next;
    }
    return (DB_NOMATCH);
}

/* DB_ResetQueryContext
**
*/
void
DB_ResetQueryContext(short dbid)
{

    DBidstruct
       *temp;

    if (GS_context == (DBidstruct *) NULL)
	return;

    temp = GS_context;
    while (temp != (DBidstruct *) NULL) {
	if (temp->dbid == dbid) {
	    HunkBufAdd tadd;
	    tadd.hunk_number = HUNK_PTR_NULL;
	    tadd.node_number = HUNK_PTR_NULL;
	    temp->querycontxt.last_patient = tadd;
	    temp->querycontxt.last_patientindex = -1;
	    temp->querycontxt.last_study = tadd;
	    temp->querycontxt.last_studyindex = -1;
	    temp->querycontxt.last_series = tadd;
	    temp->querycontxt.last_seriesindex = -1;
	    temp->querycontxt.last_image = tadd;
	    temp->querycontxt.last_imageindex = -1;
	    break;
	}
	temp = temp->next;
    }
    return;
}

/* DB_NextQuery
**
** Purpose:
**	This function performs the a generalized query of the specified database.
**
** Parameter Dictionary:
**	short dbid:
**		The database to query.
**	Query *qstruct:
**		The structure which specifies which fields to match against.
**	Query *retinfo:
**		The structure that receives the retreived data.
**
** Return Values:
**	DB_NORMAL:	The retreive operation succeeded.
**	DB_NOTOPENED:	The database has not been opened.
**	DB_BADQUERY:	The query was mal-formed.
**	DB_LOCKERROR:	Requested locks could not be granted.
**	DB_READERROR:	Records could not be read successfully.
**	DB_NOMORE:	No records matched the specifications in qstruct.
**	DB_CHANGED:	The database has changed since the initial DB_Query call.
**
** Algorithm:
**	This function is always used in conjuction with DB_Query.  It performs
**	a search at the proper level using the DICOM pattern matching specifications.
**	This function is responsible for getting the next record in the sequence
**	of records that match the selection criteria.  DB_Query must always be called
**	first to intialize the context, and then DB_NextQuery is called to retreive
**	additional records until it returns something other than DB_NORMAL.
*/
CONDITION
DB_NextQuery(short dbid, Query *qstruct, Query *retinfo)
{

    int
        pindex,
        sindex,
        serindex,
        iindex,
        i,
        j,
        k,
        l;

    PatientNode
        pnode;
    StudyNode
       *psnode,
        snode;
    SeriesNode
       *psernode,
        sernode;
    ImageNode
        inode;

    HunkBufAdd
        ploc,
        sloc,
        serloc,
        iloc;

    retinfo->QueryState = qstruct->QueryState;

    if (DB_Findid(dbid) != DB_NORMAL) {
	(void) COND_PushCondition(DB_NOTOPENED, "DB_NextQuery: DB_Findid failed");
	DB_ResetQueryContext(dbid);
	return (DB_NOTOPENED);
    }
    if (GS_root.num_patients <= 0) {
	DB_ResetQueryContext(dbid);
	(void) COND_PushCondition(DB_NOPATIENTS, "DB_NextQuery: No patients");
	return (DB_NOPATIENTS);
    }
    if (qstruct->QueryState & DB_K_CLASSPATSTUDY) {
	if (!(qstruct->QueryState & DB_K_LEVELPAT) &&
	    !(qstruct->QueryState & DB_K_LEVELSTUDY)) {
	    DB_ResetQueryContext(dbid);
	    (void) COND_PushCondition(DB_BADQUERY, "DB_NextQuery: Bad state flags in query struct");
	    return (DB_BADQUERY);
	}
    }
    if (HF_SharedLock() != HF_NORMAL) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_NextQuery: HF_SharedLock failed");
	return (DB_LOCKERROR);
    }
    if (DB_CompareChangeFlag(dbid) != DB_MATCH) {
	(void) COND_PushCondition(DB_LOCKERROR, "DB_NextQuery: Database has changed");
	return (DB_UnLock(DB_CHANGED));
    }
    /*
     * Work on the patient context first.
     */
    DB_GetQueryContext(dbid, &ploc, &pindex, DB_PATIENTCONTEXT);

    if (ploc.hunk_number == HUNK_PTR_NULL ||
	ploc.node_number == HUNK_PTR_NULL ||
	pindex == -1) {
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    if (qstruct->QueryState & DB_K_LEVELPAT) {
	if (pindex != DB_QUERYFIRST)
	    pindex++;
	else
	    pindex = 0;
	for (i = pindex; i < GS_root.num_patients; i++) {
	    ploc = GS_root.patient_loc[i];
	    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
		(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
		return (DB_UnLock(DB_READERROR));
	    }
	    if (DB_ComparePat(&(pnode.pat), qstruct) == DB_MATCH) {
		retinfo->Patient = pnode.pat;
		retinfo->Patient.Query_Flag = qstruct->Patient.Query_Flag;
		retinfo->Study.Query_Flag = qstruct->Study.Query_Flag;
		retinfo->Series.Query_Flag = qstruct->Series.Query_Flag;
		retinfo->Image.Query_Flag = qstruct->Image.Query_Flag;
		if (qstruct->Image.Query_Flag & DB_K_QIMAGEMULTUID) {
		    retinfo->Image.Query_Flag |= DB_K_QIMAGEUID;
		    retinfo->Image.Query_Flag &= ~DB_K_QIMAGEMULTUID;
		}
		DB_UpdateQueryContext(dbid, &ploc, i, DB_PATIENTCONTEXT);
		return (DB_UnLock(DB_NORMAL));
	    }
	}
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    retinfo->Patient = pnode.pat;
    if (pindex == DB_QUERYFIRST)
	pindex = 0;

    /*
     * Now work on the study context.
     */
    DB_GetQueryContext(dbid, &sloc, &sindex, DB_STUDYCONTEXT);

    if (sloc.hunk_number == HUNK_PTR_NULL ||
	sloc.node_number == HUNK_PTR_NULL ||
	sindex == -1) {
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    if (qstruct->QueryState & DB_K_LEVELSTUDY) {
	if (sindex != DB_QUERYFIRST)
	    sindex++;
	else
	    sindex = 0;
	for (i = pindex; i < GS_root.num_patients; i++) {
	    ploc = GS_root.patient_loc[i];
	    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
		(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
		return (DB_UnLock(DB_READERROR));
	    }
	    if (DB_ComparePat(&(pnode.pat), qstruct) == DB_MATCH) {
		for (j = sindex; j < pnode.num_studies; j++) {
		    sloc = pnode.study_loc[j];
		    if (HF_ReadRecord(&sloc, sizeof(snode), (void *) &snode) != HF_NORMAL) {
			(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
			return (DB_UnLock(DB_READERROR));
		    }
		    if (DB_CompareStudy(&(snode.study), qstruct) == DB_MATCH) {
			retinfo->Study = snode.study;
			retinfo->Patient = pnode.pat;
			retinfo->Patient.Query_Flag = qstruct->Patient.Query_Flag;
			retinfo->Study.Query_Flag = qstruct->Study.Query_Flag;
			retinfo->Series.Query_Flag = qstruct->Series.Query_Flag;
			retinfo->Image.Query_Flag = qstruct->Image.Query_Flag;
			if (qstruct->Image.Query_Flag & DB_K_QIMAGEMULTUID) {
			    retinfo->Image.Query_Flag |= DB_K_QIMAGEUID;
			    retinfo->Image.Query_Flag &= ~DB_K_QIMAGEMULTUID;
			}
			DB_UpdateQueryContext(dbid, &sloc, j, DB_STUDYCONTEXT);
			DB_UpdateQueryContext(dbid, &ploc, i, DB_PATIENTCONTEXT);
			return (DB_UnLock(DB_NORMAL));
		    }
		}
	    }
	    sindex = 0;
	}
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    if (DB_ReadStudyNode(&sloc, &psnode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    retinfo->Study = psnode->study;
    if (sindex == DB_QUERYFIRST)
	sindex = 0;

    /*
     * Now work on the series context.
     */
    DB_GetQueryContext(dbid, &serloc, &serindex, DB_SERIESCONTEXT);

    if (serloc.hunk_number == HUNK_PTR_NULL ||
	serloc.node_number == HUNK_PTR_NULL ||
	serindex == -1) {
	DB_ResetQueryContext(dbid);
	free(psnode);
	return (DB_UnLock(DB_NOMORE));
    }
    if (qstruct->QueryState & DB_K_LEVELSERIES) {
	if (serindex != DB_QUERYFIRST)
	    serindex++;
	else
	    serindex = 0;
	for (i = pindex; i < GS_root.num_patients; i++) {
	    ploc = GS_root.patient_loc[i];
	    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
		free(psnode);
		(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
		return (DB_UnLock(DB_READERROR));
	    }
	    if (DB_ComparePat(&(pnode.pat), qstruct) == DB_MATCH) {
		for (j = sindex; j < pnode.num_studies; j++) {
		    sloc = pnode.study_loc[j];
		    free(psnode);
		    if (DB_ReadStudyNode(&sloc, &psnode) != DB_NORMAL) {
			(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
			return (DB_UnLock(DB_READERROR));
		    }
		    if (DB_CompareStudy(&(psnode->study), qstruct) == DB_MATCH) {
			for (k = serindex; k < psnode->num_series; k++) {
			    serloc = psnode->series_loc[k];
			    if (HF_ReadRecord(&serloc, sizeof(sernode), (void *) &sernode) != HF_NORMAL) {
				free(psnode);
				(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
				return (DB_UnLock(DB_READERROR));
			    }
			    if (DB_CompareSeries(&(sernode.series), qstruct) == DB_MATCH) {
				retinfo->Series = sernode.series;
				retinfo->Study = psnode->study;
				retinfo->Patient = pnode.pat;
				retinfo->Patient.Query_Flag = qstruct->Patient.Query_Flag;
				retinfo->Study.Query_Flag = qstruct->Study.Query_Flag;
				retinfo->Series.Query_Flag = qstruct->Series.Query_Flag;
				retinfo->Image.Query_Flag = qstruct->Image.Query_Flag;
				if (qstruct->Image.Query_Flag & DB_K_QIMAGEMULTUID) {
				    retinfo->Image.Query_Flag |= DB_K_QIMAGEUID;
				    retinfo->Image.Query_Flag &= ~DB_K_QIMAGEMULTUID;
				}
				DB_UpdateQueryContext(dbid, &serloc, k, DB_SERIESCONTEXT);
				DB_UpdateQueryContext(dbid, &sloc, j, DB_STUDYCONTEXT);
				DB_UpdateQueryContext(dbid, &ploc, i, DB_PATIENTCONTEXT);
				free(psnode);
				return (DB_UnLock(DB_NORMAL));
			    }
			}
		    }
		    serindex = 0;
		}
	    }
	    sindex = 0;
	}
	free(psnode);
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    if (DB_ReadSeriesNode(&serloc, &psernode) != DB_NORMAL) {
	(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
	return (DB_UnLock(DB_READERROR));
    }
    retinfo->Series = psernode->series;
    if (serindex == DB_QUERYFIRST)
	serindex = 0;

    /*
     * Now work on the image context.
     */
    DB_GetQueryContext(dbid, &iloc, &iindex, DB_IMAGECONTEXT);

    if (iloc.hunk_number == HUNK_PTR_NULL ||
	iloc.node_number == HUNK_PTR_NULL ||
	iindex == -1) {
	DB_ResetQueryContext(dbid);
	free(psnode);
	free(psernode);
	return (DB_UnLock(DB_NOMORE));
    }
    if (qstruct->QueryState & DB_K_LEVELIMAGE) {
	if (iindex != DB_QUERYFIRST)
	    iindex++;
	else
	    iindex = 0;
	for (i = pindex; i < GS_root.num_patients; i++) {
	    ploc = GS_root.patient_loc[i];
	    if (HF_ReadRecord(&ploc, sizeof(pnode), (void *) &pnode) != HF_NORMAL) {
		free(psnode);
		free(psernode);
		(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
		return (DB_UnLock(DB_READERROR));
	    }
	    if (DB_ComparePat(&(pnode.pat), qstruct) == DB_MATCH) {
		for (j = sindex; j < pnode.num_studies; j++) {
		    sloc = pnode.study_loc[j];
		    free(psnode);
		    if (DB_ReadStudyNode(&sloc, &psnode) != DB_NORMAL) {
			free(psernode);
			(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
			return (DB_UnLock(DB_READERROR));
		    }
		    if (DB_CompareStudy(&(psnode->study), qstruct) == DB_MATCH) {
			for (k = serindex; k < psnode->num_series; k++) {
			    serloc = psnode->series_loc[k];
			    free(psernode);
			    if (DB_ReadSeriesNode(&serloc, &psernode) != DB_NORMAL) {
				free(psnode);
				(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
				return (DB_UnLock(DB_READERROR));
			    }
			    if (DB_CompareSeries(&(psernode->series), qstruct) == DB_MATCH) {
				for (l = iindex; l < psernode->num_images; l++) {
				    iloc = psernode->image_loc[l];
				    if (HF_ReadRecord(&iloc, sizeof(inode), (void *) &inode) != HF_NORMAL) {
					free(psnode);
					free(psernode);
					(void) COND_PushCondition(DB_READERROR, "DB_NextQuery: HF_ReadRecord failed");
					return (DB_UnLock(DB_READERROR));
				    }
				    if (DB_CompareImage(&(inode.image), qstruct) == DB_MATCH) {
					retinfo->Image = inode.image;
					retinfo->Series = psernode->series;
					retinfo->Study = psnode->study;
					retinfo->Patient = pnode.pat;
					retinfo->Patient.Query_Flag = qstruct->Patient.Query_Flag;
					retinfo->Study.Query_Flag = qstruct->Study.Query_Flag;
					retinfo->Series.Query_Flag = qstruct->Series.Query_Flag;
					retinfo->Image.Query_Flag = qstruct->Image.Query_Flag;
					if (qstruct->Image.Query_Flag & DB_K_QIMAGEMULTUID) {
					    retinfo->Image.Query_Flag |= DB_K_QIMAGEUID;
					    retinfo->Image.Query_Flag &= ~DB_K_QIMAGEMULTUID;
					}
					DB_UpdateQueryContext(dbid, &iloc, l, DB_IMAGECONTEXT);
					DB_UpdateQueryContext(dbid, &serloc, k, DB_SERIESCONTEXT);
					DB_UpdateQueryContext(dbid, &sloc, j, DB_STUDYCONTEXT);
					DB_UpdateQueryContext(dbid, &ploc, i, DB_PATIENTCONTEXT);
					free(psnode);
					free(psernode);
					return (DB_UnLock(DB_NORMAL));
				    }
				}
			    }
			    iindex = 0;
			}
		    }
		    serindex = 0;
		}
	    }
	    sindex = 0;
	}
	free(psnode);
	free(psernode);
	DB_ResetQueryContext(dbid);
	return (DB_UnLock(DB_NOMORE));
    }
    free(psnode);
    free(psernode);
    return (DB_UnLock(DB_NOMORE));
}
/* DB_ComparePat
**
*/
CONDITION
DB_ComparePat(PatientLevel *pat, Query *qstruct)
{


    if (qstruct->Patient.Query_Flag & DB_K_QBIRTHDATE) {
	if (strcmp(qstruct->Patient.BirthDate, "") != 0) {
	    if (DB_DateMatch(qstruct->Patient.BirthDate, pat->BirthDate) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Patient.Query_Flag & DB_K_QNAME) {
	if (strcmp(qstruct->Patient.Name, "") != 0) {
	    if (DB_RegexMatch(qstruct->Patient.Name, pat->Name) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Patient.Query_Flag & DB_K_QID) {
	if (strcmp(qstruct->Patient.PatID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Patient.PatID, pat->PatID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    return (DB_MATCH);
}
/* DB_CompareStudy
**
*/
CONDITION
DB_CompareStudy(StudyLevel *study, Query *qstruct)
{

    if (qstruct->Study.Query_Flag & DB_K_QSTUDYDATE) {
	if (strcmp(qstruct->Study.StudyDate, "") != 0) {
	    if (DB_DateMatch(qstruct->Study.StudyDate, study->StudyDate) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QSTUDYTIME) {
	if (strcmp(qstruct->Study.StudyTime, "") != 0) {
	    if (DB_TimeMatch(qstruct->Study.StudyTime, study->StudyTime) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QSTUDYID) {
	if (strcmp(qstruct->Study.StudyID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.StudyID, study->StudyID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QACCESSIONNUMBER) {
	if (strcmp(qstruct->Study.AccessionNumber, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.AccessionNumber, study->AccessionNumber) ==
		DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QSTUDYUID) {
	if (strcmp(qstruct->Study.StudyUID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.StudyUID, study->StudyUID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QREFERRINGPHYSNAME) {
	if (strcmp(qstruct->Study.ReferringPhysName, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.ReferringPhysName,
			      study->ReferringPhysName) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QINTERPRETPHYSNAME) {
	if (strcmp(qstruct->Study.InterpretingPhysName, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.InterpretingPhysName,
			      study->InterpretingPhysName) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QPROCEDUREDESCRIPTION) {
	if (strcmp(qstruct->Study.ProcedureDescription, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.ProcedureDescription,
			      study->ProcedureDescription) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Study.Query_Flag & DB_K_QADMITTINGDIAGNOSEDDESCRIPTION) {
	if (strcmp(qstruct->Study.AdmittingDiagnosedDescription, "") != 0) {
	    if (DB_RegexMatch(qstruct->Study.AdmittingDiagnosedDescription,
			study->AdmittingDiagnosedDescription) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    return (DB_MATCH);
}
/* DB_CompareSeries
**
*/
CONDITION
DB_CompareSeries(SeriesLevel *series, Query *qstruct)
{

    if (qstruct->Series.Query_Flag & DB_K_QMODALITY) {
	if (strcmp(qstruct->Series.Modality, "") != 0) {
	    if (DB_RegexMatch(qstruct->Series.Modality, series->Modality) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Series.Query_Flag & DB_K_QSERIESNUMBER) {
	if (strcmp(qstruct->Series.SeriesNumber, "") != 0) {
	    if (DB_RegexMatch(qstruct->Series.SeriesNumber, series->SeriesNumber) ==
		DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Series.Query_Flag & DB_K_QSERIESUID) {
	if (strcmp(qstruct->Series.SeriesUID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Series.SeriesUID, series->SeriesUID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    return (DB_MATCH);
}
/* DB_CompareImage
**
*/
CONDITION
DB_CompareImage(ImageLevel *image, Query *qstruct)
{

    int
        found_match,
        i;

    if (qstruct->Image.Query_Flag & DB_K_QIMAGENUMBER) {
	if (strcmp(qstruct->Image.ImageNumber, "") != 0) {
	    if (DB_RegexMatch(qstruct->Image.ImageNumber, image->ImageNumber) ==
		DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Image.Query_Flag & DB_K_QIMAGEUID) {
	if (strcmp(qstruct->Image.ImageUID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Image.ImageUID, image->ImageUID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    if (qstruct->Image.Query_Flag & DB_K_QCLASSUID) {
	if (strcmp(qstruct->Image.ClassUID, "") != 0) {
	    if (DB_RegexMatch(qstruct->Image.ClassUID, image->ClassUID) == DB_NOMATCH)
		return (DB_NOMATCH);
	}
    }
    found_match = 0;
    if (qstruct->Image.Query_Flag & DB_K_QIMAGEMULTUID) {
	for (i = 0; i < qstruct->Image.ImageMultUIDCount; i++) {
	    if (DB_RegexMatch(qstruct->Image.ImageMultUID[i], image->ImageUID) == DB_MATCH) {
		found_match = 1;
		break;
	    }
	}
	if (!found_match)
	    return (DB_NOMATCH);
    }
    return (DB_MATCH);
}

/* DB_ConvertDatetoLong
**	Convert a dicom date to a long for comparision ease.
*/
long
DB_ConvertDatetoLong(char *date)
{

    char
        year[5],
        month[3],
        day[3];

    strncpy(year, date, 4);
    year[4] = '\000';
    strncpy(month, date + 4, 2);
    month[2] = '\000';
    strncpy(day, date + 6, 2);
    day[2] = '\000';

    return ((atol(year) * 10000) + (atol(month) * 100) + atol(day));
}


/* DB_ConvertTimetoFloat
**	Convert a dicom time to a floating point number for comparision ease.
*/
double
DB_ConvertTimetoFloat(char *tm)
{

    int
        i;
    char
        hour[3],
        minute[3],
        second[3],
        fracsec[5],
       *p;
    double
        divisor,
        hh,
        mm,
        ss,
        fs;

    hh = mm = ss = fs = 0.0;
    hour[0] = minute[0] = second[0] = fracsec[0] = '\000';

    p = tm;
    /*
     * Just a brute force way to tear down a dicom time...not very pretty,
     * but it works... We are not guaranteed to have every field present as
     * we are in the date...
     */
    hour[0] = *p++;
    hour[1] = *p++;
    hour[2] = '\000';
    if (isdigit(*p)){
	minute[0] = *p++;
	minute[1] = *p++;
	minute[2] = '\000';
		if (isdigit(*p)){
			second[0] = *p++;
			second[1] = *p++;
			second[2] = '\000';
			if (*p == '.'){
				p++;
				fracsec[0] = *p++;
				if ((*p != '\000') && (isdigit(*p))){
					fracsec[1] = *p++;
					if ((*p != '\000') && (isdigit(*p))){
						fracsec[2] = *p++;
						if ((*p != '\000') && (isdigit(*p))){
							fracsec[3] = *p++;
						}else{
							fracsec[3] = '\000';
						}
					}else{
						fracsec[2] = '\000';
					}
				}else{
					fracsec[1] = '\000';
				}
			}
		}
    }
    hh = atof(hour);
    mm = atof(minute);
    ss = atof(second);
    divisor = 10;
    for (i = 0; i < strlen(fracsec) - 1; i++)
	divisor *= 10;
    fs = atof(fracsec) / divisor;

    return ((hh * 3600) + (mm * 60) + ss + fs);
}
/* DB_SqueezeBlanks
**
*/
void
DB_SqueezeBlanks(char *s)
{

    char
       *t1,
       *t2;

    t1 = t2 = s;
    while (*t2 != '\000') {
	if (*t2 != ' ') {
	    *t1 = *t2;
	    t1++;
	}
	t2++;
    }
    *t1 = '\000';

    return;
}
/* DB_DateMatch
**	Match a date range as specified in the dicom standard
*/
CONDITION
DB_DateMatch(char *datestring, char *stm)
{

    int
        match;
    char
       *ndate;
    long
        start_date,
        end_date,
        date_in_question;

    if ((ndate = (char *) malloc(strlen(datestring) + 1)) == (char *) NULL)
	return (DB_NOMATCH);

    strcpy(ndate, datestring);
    DB_SqueezeBlanks(ndate);
    DB_SqueezeBlanks(stm);

    match = 0;
    if (strchr(ndate, (int) '-') == (char *) NULL) {
	if (strcmp(ndate, stm) == 0)
	    match = 1;
    } else {
	date_in_question = DB_ConvertDatetoLong(stm);
	if (ndate[0] == '-') {
	    end_date = DB_ConvertDatetoLong(ndate + 1);
	    if (date_in_question <= end_date)
		match = 1;
	} else if (ndate[strlen(ndate) - 1] == '-') {
	    start_date = DB_ConvertDatetoLong(ndate);
	    if (date_in_question >= start_date)
		match = 1;
	} else {
	    start_date = DB_ConvertDatetoLong(ndate);
	    end_date = DB_ConvertDatetoLong(strchr(ndate, (int) '-') + 1);
	    if ((date_in_question >= start_date) &&
		(date_in_question <= end_date))
		match = 1;
	}
    }
    free(ndate);
    if (match)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
/* DB_TimeMatch
**	Match a time range as specified in the dicom standard
*/
CONDITION
DB_TimeMatch(char *timestring, char *stm)
{

    int
        match;
    char
       *ntime;
    double
        start_time,
        end_time,
        time_in_question;

    if ((ntime = (char *) malloc(strlen(timestring) + 2)) == (char *) NULL)
	return (DB_NOMATCH);

    strcpy(ntime, timestring);
    DB_SqueezeBlanks(ntime);
    DB_SqueezeBlanks(stm);

    match = 0;
    if (strchr(ntime, (int) '-') == (char *) NULL) {
	if (strcmp(ntime, stm) == 0)
	    match = 1;
    } else {
	time_in_question = DB_ConvertTimetoFloat(stm);
	if (ntime[0] == '-') {
	    end_time = DB_ConvertTimetoFloat(ntime + 1);
	    if (time_in_question <= end_time)
		match = 1;;
	} else if (ntime[strlen(ntime) - 1] == '-') {
	    start_time = DB_ConvertTimetoFloat(ntime);
	    if (time_in_question >= start_time)
		match = 1;
	} else {
	    start_time = DB_ConvertTimetoFloat(ntime);
	    end_time = DB_ConvertTimetoFloat(strchr(ntime, (int) '-') + 1);
	    if ((time_in_question >= start_time) &&
		(time_in_question <= end_time))
		match = 1;;
	}
    }
    free(ntime);
    if (match)
	return (DB_MATCH);
    else
	return (DB_NOMATCH);
}
