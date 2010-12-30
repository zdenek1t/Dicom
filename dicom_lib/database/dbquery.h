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
** Module Name(s):	dbquery.h
** Author, Date:	David E. Beecher, 5-May-93
** Intent:		Include file for database queries.
** Last Update:		$Author: smm $, $Date: 1998/02/18 19:38:09 $
** Source File:		$RCSfile: dbquery.h,v $
** Revision:		$Revision: 1.24 $
** Status:		$State: Exp $
*/

#ifndef DBQUERY_IS_IN
#define DBQUERY_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Query flags for database searches...
 */
#define DB_K_QBIRTHDATE		0x0001
#define DB_K_QNAME		0x0002
#define DB_K_QID		0x0004

#define DB_K_QSTUDYDATE		0x0001
#define DB_K_QSTUDYTIME		0x0002
#define DB_K_QSTUDYID		0x0004
#define DB_K_QACCESSIONNUMBER	0x0008
#define DB_K_QSTUDYUID		0x0010
#define DB_K_QREFERRINGPHYSNAME			0x0020
#define DB_K_QINTERPRETPHYSNAME			0x0040
#define DB_K_QPROCEDUREDESCRIPTION		0x0080
#define DB_K_QADMITTINGDIAGNOSEDDESCRIPTION	0x0100

#define DB_K_QMODALITY		0x0001
#define DB_K_QSERIESNUMBER	0x0002
#define DB_K_QSERIESUID		0x0004

#define DB_K_QIMAGENUMBER	0x0001
#define DB_K_QIMAGEUID		0x0002
#define DB_K_QIMAGEMULTUID	0x0004
#define DB_K_QCLASSUID		0x0008

/*
 * Bit values for the query state in Query
 */
#define DB_K_CLASSPAT		0x0001
#define DB_K_CLASSSTUDY		0x0002
#define DB_K_CLASSPATSTUDY	0x0004
#define DB_K_LEVELPAT		0x0008
#define DB_K_LEVELSTUDY		0x0010
#define DB_K_LEVELSERIES	0x0020
#define DB_K_LEVELIMAGE		0x0040

    typedef struct {
	char
	    BirthDate[2 * DICOM_DA_LENGTH + 2],	/* Required	 */
	    Name[DICOM_PN_LENGTH + 1],	/* Required	 */
	    PatID[DICOM_LO_LENGTH + 1];	/* Unique	 */
	long Query_Flag;
    }   PatientLevel;

    typedef struct {
	char
	    StudyDate[2 * DICOM_DA_LENGTH + 2],	/* Required	 */
	    StudyTime[2 * DICOM_TM_LENGTH + 2],	/* Required	 */
	    StudyID[DICOM_CS_LENGTH + 1],	/* Required	 */
	    AccessionNumber[DICOM_IS_LENGTH + 1],	/* Required	 */
	    StudyUID[DICOM_UI_LENGTH + 1],	/* Unique	 */
	    ReferringPhysName[DICOM_PN_LENGTH + 1],	/* Optional	 */
	    InterpretingPhysName[DICOM_PN_LENGTH + 1],	/* Optional	 */
	    ProcedureDescription[DICOM_LO_LENGTH + 1],	/* Optional	 */
	    AdmittingDiagnosedDescription[DICOM_LO_LENGTH + 1];	/* Optional */
	long Query_Flag;
    }   StudyLevel;

    typedef struct {
	char
	    Modality[DICOM_CS_LENGTH + 1],	/* Required	 */
	    SeriesNumber[DICOM_IS_LENGTH + 1],	/* Required	 */
	    SeriesUID[DICOM_UI_LENGTH + 1];	/* Required	 */
	long Query_Flag;
    }   SeriesLevel;

    typedef struct {
	char
	    ImageNumber[DICOM_IS_LENGTH + 1],	/* Required	 	 */
	    ClassUID[DICOM_UI_LENGTH + 1],	/* Helpful for Demo	 */
	    ImageUID[DICOM_UI_LENGTH + 1],	/* Unique		 */
	  **ImageMultUID;
	long ImageMultUIDCount;	/* For UID lists	 */
	char FileName[1024];	/* For image access	 */
	long Query_Flag;
    }   ImageLevel;

    typedef struct {
	long QueryState;
	PatientLevel Patient;
	StudyLevel Study;
	SeriesLevel Series;
	ImageLevel Image;
    }   Query;

/*
 * Function Prototypes...
 */

    CONDITION DB_CreateDB(char *dbkey);
    CONDITION DB_DeleteDB(char *dbkey);
    CONDITION DB_Open(char *dbkey, short *dbid);
    CONDITION DB_Close(short dbid);

    CONDITION DB_AddPatient(short dbid, PatientLevel *pat);
    CONDITION DB_AddStudy(short dbid, char *PatID, StudyLevel *study);
        CONDITION
        DB_AddSeries(short dbid, char *PatID, char *studyuid,
		         SeriesLevel *series);
        CONDITION
        DB_AddImage(short dbid, char *patid, char *studyuid,
		        char *seriesuid, ImageLevel *image);

    CONDITION DB_DelPatient(short dbid, char *patid);
    CONDITION DB_DelStudy(short dbid, char *patid, char *studyuid);
        CONDITION
        DB_DelSeries(short dbid, char *patid, char *studyuid,
		         char *seriesuid);
        CONDITION
        DB_DelImage(short dbid, char *patid, char *studyuid,
		        char *seriesuid, char *imageuid);

        CONDITION
        DB_GetPatient(short dbid, PatientLevel *patient);
        CONDITION
        DB_GetNextPatient(short dbid, PatientLevel *patient);

        CONDITION
        DB_GetStudy(short dbid, char *patid, StudyLevel *study);
        CONDITION
        DB_GetNextStudy(short dbid, StudyLevel *study);

        CONDITION
        DB_GetSeries(short dbid, char *patid, char *studyuid, SeriesLevel *series);
        CONDITION
        DB_GetNextSeries(short dbid, SeriesLevel *series);

        CONDITION
        DB_GetImage(short dbid, char *patid, char *studyuid,
		        char *seriesuid, ImageLevel *image);
    CONDITION DB_GetNextImage(short dbid, ImageLevel *image);

    CONDITION DB_Query(short dbid, Query *qstruct, Query *retinfo);
    CONDITION DB_NextQuery(short dbid, Query *qstruct, Query *retinfo);

    CONDITION DB_GetNumberofStudies(short dbid, long *numstudies);

    CONDITION DB_DelOldestStudy(short dbid);

#define	DB_NORMAL	FORM_COND(FAC_DB,SEV_SUCC,1)
#define	DB_OK		FORM_COND(FAC_DB,SEV_SUCC,1)
#define	DB_CREATERROR	FORM_COND(FAC_DB,SEV_ERROR,2)
#define	DB_OPENERROR	FORM_COND(FAC_DB,SEV_ERROR,3)
#define	DB_ALLOCATERROR	FORM_COND(FAC_DB,SEV_ERROR,4)
#define	DB_WRITERROR	FORM_COND(FAC_DB,SEV_ERROR,5)
#define	DB_CLOSERROR	FORM_COND(FAC_DB,SEV_ERROR,6)
#define	DB_IDREMERROR	FORM_COND(FAC_DB,SEV_ERROR,7)
#define	DB_READERROR	FORM_COND(FAC_DB,SEV_ERROR,8)
#define	DB_NOMEMORY	FORM_COND(FAC_DB,SEV_ERROR,9)
#define	DB_TOOMANYPATS	FORM_COND(FAC_DB,SEV_ERROR,10)
#define	DB_DUPATIENT	FORM_COND(FAC_DB,SEV_ERROR,11)
#define	DB_ALLOCERROR	FORM_COND(FAC_DB,SEV_ERROR,12)
#define	DB_IDNOTHERE	FORM_COND(FAC_DB,SEV_ERROR,13)
#define DB_NOTOPENED	FORM_COND(FAC_DB,SEV_ERROR,14)
#define DB_BADPATIENT	FORM_COND(FAC_DB,SEV_ERROR,15)
#define DB_TOOMANYSTUDS	FORM_COND(FAC_DB,SEV_ERROR,16)
#define DB_BADSTUDY	FORM_COND(FAC_DB,SEV_ERROR,17)
#define DB_BADSERIES	FORM_COND(FAC_DB,SEV_ERROR,18)
#define DB_DUPSTUDY	FORM_COND(FAC_DB,SEV_ERROR,19)
#define DB_DUPSERIES	FORM_COND(FAC_DB,SEV_ERROR,20)
#define DB_DELERROR	FORM_COND(FAC_DB,SEV_ERROR,21)
#define DB_BADIMAGE	FORM_COND(FAC_DB,SEV_ERROR,22)
#define DB_NOPATIENTS	FORM_COND(FAC_DB,SEV_ERROR,23)
#define DB_NOMORE	FORM_COND(FAC_DB,SEV_ERROR,24)
#define DB_NOSTUDIES	FORM_COND(FAC_DB,SEV_ERROR,25)
#define DB_NOSERIES	FORM_COND(FAC_DB,SEV_ERROR,26)
#define DB_EXISTS	FORM_COND(FAC_DB,SEV_ERROR,27)
#define DB_LOCKERROR	FORM_COND(FAC_DB,SEV_ERROR,28)
#define DB_BADQUERY	FORM_COND(FAC_DB,SEV_ERROR,29)
#define DB_DUPIMAGE	FORM_COND(FAC_DB,SEV_ERROR,30)
#define DB_FILEDELERROR	FORM_COND(FAC_DB,SEV_WARN,31)
#define DB_CHANGED	FORM_COND(FAC_DB,SEV_WARN,32)

#ifdef  __cplusplus
}
#endif

#endif
