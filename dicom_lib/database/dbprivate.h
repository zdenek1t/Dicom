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
** Module Name(s):	dbprivate.h
** Author, Date:	David E. Beecher, 12-May-93
** Intent:		Private data structure for the database access
**			routines.
** Last Update:		$Author: smm $, $Date: 1998/02/18 19:38:35 $
** Source File:		$RCSfile: dbprivate.h,v $
** Revision:		$Revision: 1.26 $
** Status:		$State: Exp $
*/
#ifndef DBPRIVATE_IS_IN
#define DBPRIVATE_IS_IN

#ifdef  __cplusplus
extern "C" {
#endif

#define DB_MAXKEYLENGTH		1024
#define	DB_MAXPATIENTS		60
#define DB_MAXSTUDIES		60
#define DB_INITSIZE		10
#define DB_EXPANDSIZE		5

#define DB_QUERYFIRST		-2

#define DB_HUNKRECORDSIZE		600
#define DB_NUMRECSPERHUNK		100
#define DB_HUNKINITIALALLOCATION	5

#define DB_MATCH	1
#define DB_NOMATCH	0

#define REGEX_SIZE	128

#define DB_PATIENTCONTEXT	1
#define DB_STUDYCONTEXT		2
#define DB_SERIESCONTEXT	3
#define DB_IMAGECONTEXT		4

    typedef struct _Root {
	char
	    dbkey[DB_MAXKEYLENGTH];
	int
	    num_patients;
	    HunkBufAdd
	    patient_loc[DB_MAXPATIENTS];
    }   Root;

    typedef struct _PatientNode {
	time_t
	time_stamp;
	PatientLevel
	pat;
	int
	    num_studies;
	HunkBufAdd
	    study_loc[DB_MAXSTUDIES];
    }   PatientNode;

    typedef struct _StudyNode {
	time_t
	    time_stamp;
	    StudyLevel
	    study;
	int
	    num_allocated,
	    num_series;
	HunkBufAdd
	    series_loc[DB_INITSIZE];
    }   StudyNode;

    typedef struct _SeriesNode {
	time_t
	    time_stamp;
	    SeriesLevel
	    series;
	int
	    num_allocated,
	    num_images;
	HunkBufAdd
	    image_loc[DB_INITSIZE];
    }   SeriesNode;

    typedef struct _ImageNode {
	time_t
	    time_stamp;
	    ImageLevel
	    image;
    }   ImageNode;

    typedef struct _DBcontext {
	HunkBufAdd
	    last_patient,
	    last_study,
	    last_series,
	    last_image;
	int
	    last_patientindex,
	    last_studyindex,
	    last_seriesindex,
	    last_imageindex;
    }   DBcontext;

    typedef struct _DBidstruct {
	char dbkey[DB_MAXKEYLENGTH];
	short dbid;
	long dbchanged;
	DBcontext
	    pacontxt,
	    stcontxt,
	    secontxt,
	    imcontxt,
	    querycontxt;
	struct _DBidstruct
	   *next;
    }   DBidstruct;

    CONDITION DB_Init(char *dbkey);
    CONDITION DB_Findid(short dbid);
    CONDITION DB_Removeid(short dbid, char *dbkey);
    CONDITION DB_Addid(short *dbid);

    CONDITION DB_ComparePatID(char *patid, HunkBufAdd *pat_loc);
    CONDITION DB_CompareStudyUID(char *studyuid, HunkBufAdd *study_loc);
    CONDITION DB_CompareSeriesUID(char *seriesuid, HunkBufAdd *series_loc);
    CONDITION DB_CompareImageUID(char *imageuid, HunkBufAdd *image_loc);

    CONDITION DB_ComparePat(PatientLevel *pat, Query *qstruct);
    CONDITION DB_CompareStudy(StudyLevel *study, Query *qstruct);
    CONDITION DB_CompareSeries(SeriesLevel *series, Query *qstruct);
    CONDITION DB_CompareImage(ImageLevel *image, Query *qstruct);

    CONDITION DB_ReadStudyNode(HunkBufAdd *add, StudyNode **snode);
    CONDITION DB_WriteStudyNode(HunkBufAdd *add, StudyNode *snode);
    CONDITION DB_ReadSeriesNode(HunkBufAdd *add, SeriesNode **snode);
    CONDITION DB_WriteSeriesNode(HunkBufAdd *add, SeriesNode *snode);
    void DB_DumpDB(short dbid);
    void DB_PrintPatient(PatientNode p);
    void DB_PrintStudy(StudyNode *s);
    void DB_PrintSeries(SeriesNode *s);
    void DB_PrintImage(ImageNode i);
    void DB_UpdatePatientContext(short dbid, HunkBufAdd *loc, int index, int level);
    void DB_UpdateStudyContext(short dbid, HunkBufAdd *loc, int index, int level);
    void DB_UpdateSeriesContext(short dbid, HunkBufAdd *loc, int index, int level);
    void DB_UpdateImageContext(short dbid, HunkBufAdd *loc, int index, int level);
    void DB_UpdateQueryContext(short dbid, HunkBufAdd *loc, int index, int level);
    void DB_GetPatientContext(short dbid, HunkBufAdd *loc, int *index, int level);
    void DB_GetStudyContext(short dbid, HunkBufAdd *loc, int *index, int level);
    void DB_GetSeriesContext(short dbid, HunkBufAdd *loc, int *index, int level);
    void DB_GetImageContext(short dbid, HunkBufAdd *loc, int *index, int level);
    void DB_GetQueryContext(short dbid, HunkBufAdd *loc, int *index, int level);
    void DB_ResetQueryContext(short dbid);
    void DB_SetChangeFlag(short dbid);
    CONDITION DB_CompareChangeFlag(short dbid);
    CONDITION DB_DelBelowStudy(StudyNode *snode);
    CONDITION DB_DelBelowSeries(SeriesNode *snode);
    CONDITION DB_UnLock(CONDITION ret_val);
    CONDITION DB_RegexMatch(char *regex, char *stm);
    CONDITION DB_DateMatch(char *datestring, char *stm);
    CONDITION DB_TimeMatch(char *timestring, char *stm);
    char *DB_ConvertRegex(char *regex);
    long DB_ConvertDatetoLong(char *date);
    double DB_ConvertTimetoFloat(char *time);
    void DB_SqueezeBlanks(char *s);

#ifdef  __cplusplus
}
#endif

#endif
