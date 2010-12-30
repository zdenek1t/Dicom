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
**				DICOM 94
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	idb.h
** Author, Date:	David E. Beecher, 14-Mar-94
** Intent:		Primary include file for idb.c, the image
**			database access facility.
** Last Update:		$Author: smm $, $Date: 1999/09/24 03:02:15 $
** Source File:		$RCSfile: idb.h,v $
** Revision:		$Revision: 1.21 $
** Status:		$State: Exp $
*/


#ifndef _IDB_IS_IN
#define _IDB_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

#define ZERO_LENGTH_NUM 999999999

#define IDB_ERROR(a) (a), IDB_Message((a))

#define IDB_PATIENT_LEVEL	1
#define IDB_STUDY_LEVEL		2
#define IDB_SERIES_LEVEL	3
#define IDB_IMAGE_LEVEL		4

#define IDB_OWNER_LENGTH	16
#define IDB_GROUP_LENGTH	16
#define IDB_PRIV_LENGTH		9

#define	IDB_AS_QLENGTH		(DICOM_AS_LENGTH*2)
#define	IDB_CS_QLENGTH		(DICOM_CS_LENGTH*2)
#define	IDB_DS_QLENGTH		(DICOM_DS_LENGTH*2)
#define	IDB_IS_QLENGTH		(DICOM_IS_LENGTH*2)
#define	IDB_PN_QLENGTH		(DICOM_PN_LENGTH*8)
#define	IDB_LO_QLENGTH		(DICOM_LO_LENGTH*2)
#define	IDB_DA_QLENGTH		((DICOM_DA_LENGTH*2)+1)
#define	IDB_TM_QLENGTH		((DICOM_TM_LENGTH*2)+1)
#define	IDB_UI_QLENGTH		(DICOM_UI_LENGTH*2)
#define	IDB_SH_QLENGTH		(DICOM_SH_LENGTH*2)
#define	IDB_AE_QLENGTH		(DICOM_AE_LENGTH*2)
#define	IDB_ST_QLENGTH		DICOM_ST_LENGTH
#define	IDB_LT_QLENGTH		DICOM_LT_LENGTH

#define IDB_OWNER_QLENGTH	(IDB_OWNER_LENGTH*2)
#define IDB_GROUP_QLENGTH	(IDB_GROUP_LENGTH*2)
#define IDB_PRIV_QLENGTH	(IDB_PRIV_LENGTH*2)

typedef struct _IDB_PatientQuery {
    char
        PatNam[IDB_PN_QLENGTH + 1],	/* Secondary index */
        PatID[IDB_LO_QLENGTH + 1],	/* Unique index */
        PatBirDat[IDB_DA_QLENGTH + 1],
        PatBirTim[IDB_TM_QLENGTH + 1],
        PatSex[IDB_CS_QLENGTH + 1];
    long
        NumPatRelStu,
        NumPatRelSer,
        NumPatRelIma;
    char
        InsertDate[IDB_DA_QLENGTH + 1],
        InsertTime[IDB_TM_QLENGTH + 1],

        Owner[IDB_OWNER_QLENGTH + 1],
        GroupName[IDB_GROUP_QLENGTH + 1],
        Priv[IDB_PRIV_QLENGTH + 1];
}   IDB_PatientQuery;

typedef struct _IDB_StudyQuery {
    char
        StuDat[IDB_DA_QLENGTH + 1],
        StuTim[IDB_TM_QLENGTH + 1],
        AccNum[IDB_SH_QLENGTH + 1],
        StuID[IDB_SH_QLENGTH + 1],	/* Secondary index */
        StuInsUID[IDB_UI_QLENGTH + 1],	/* Unique index */
        RefPhyNam[IDB_PN_QLENGTH + 1],
        StuDes[IDB_LO_QLENGTH + 1],
        PatAge[IDB_AS_QLENGTH + 1],
        PatSiz[IDB_DS_QLENGTH + 1],
        PatWei[IDB_DS_QLENGTH + 1];
    int
        NumStuRelSer,
        NumStuRelIma;
    char
        InsertDate[IDB_DA_QLENGTH + 1],
        InsertTime[IDB_TM_QLENGTH + 1],

        Owner[IDB_OWNER_QLENGTH + 1],
        GroupName[IDB_GROUP_QLENGTH + 1],
        Priv[IDB_PRIV_QLENGTH + 1];
    char
        PatParent[DICOM_UI_LENGTH + 1],
    	ModsInStudy[IDB_CS_QLENGTH + 1];
}   IDB_StudyQuery;

typedef struct _IDB_SeriesQuery {
    char
        Mod[IDB_CS_QLENGTH + 1],
        SerNum[IDB_IS_QLENGTH + 1],
        SerInsUID[IDB_UI_QLENGTH + 1],	/* Unique index */
        ProNam[IDB_LO_QLENGTH + 1],
        SerDes[IDB_LO_QLENGTH + 1],
        BodParExa[IDB_CS_QLENGTH + 1],
        ViePos[IDB_CS_QLENGTH + 1];
    int
        NumSerRelIma;
    char
        InsertDate[IDB_DA_QLENGTH + 1],
        InsertTime[IDB_TM_QLENGTH + 1],

        Owner[IDB_OWNER_QLENGTH + 1],
        GroupName[IDB_GROUP_QLENGTH + 1],
        Priv[IDB_PRIV_QLENGTH + 1];
    char
        StuParent[DICOM_UI_LENGTH + 1];
}   IDB_SeriesQuery;

typedef struct _IDB_UIDListElement {
    void
       *ptr1,
       *ptr2;
    char
        UID[IDB_UI_QLENGTH + 1];
}   IDB_UIDListElement;

typedef struct _IDB_InstanceListElement {
    void
       *ptr1,
       *ptr2;
    char
        RespondingTitle[17],
        Medium[33],
        Path[256],
        Transfer[65];
    int
        Size;
}   IDB_InstanceListElement;

typedef struct _IDB_ImageQuery {
    char
        ImaNum[IDB_IS_QLENGTH + 1],
        SOPInsUID[IDB_UI_QLENGTH + 1],	/* Unique index */
        SOPClaUID[IDB_UI_QLENGTH + 1],
        PhoInt[IDB_CS_QLENGTH + 1],
        PatOri[IDB_CS_QLENGTH + 1];
    int
        SamPerPix,
        Row,
        Col,
        BitAll,
        BitSto,
        PixRep;
    char
        InsertDate[IDB_DA_QLENGTH + 1],
        InsertTime[IDB_TM_QLENGTH + 1],

        Owner[IDB_OWNER_QLENGTH + 1],
        GroupName[IDB_GROUP_QLENGTH + 1],
        Priv[IDB_PRIV_QLENGTH + 1];
    char
        SerParent[DICOM_UI_LENGTH + 1];
        LST_HEAD
    *   ImageUIDList;
        LST_HEAD
    *   InstanceList;
}   IDB_ImageQuery;

typedef struct _IDB_Query {
    IDB_PatientQuery patient;
    IDB_StudyQuery study;
    IDB_SeriesQuery series;
    IDB_ImageQuery image;
    long
        PatientQFlag,
        StudyQFlag,
        SeriesQFlag,
        ImageQFlag;
    long
        PatientNullFlag,
        StudyNullFlag,
        SeriesNullFlag,
        ImageNullFlag;
}   IDB_Query;

#ifdef CTN_IDBV2
/* These structures/flags are for version 2 of the Image Database.  It
** includes values with the size of the database (bytes of images) and
** other counts for quick reference.
*/

typedef struct {
    long DBSize;		/* Current size of database in bytes */
    long DBLimit;		/* Limit on database size in MB */
    long PatientCount;		/* Number of patients in the database */
    long StudyCount;		/* Number of studies in the database */
    long ImageCount;		/* Number of images in the database */
} IDB_Limits;


#endif
/*
 * Query Flags for IDB_Select--Patient Level
 */
#define	QF_PAT_PatNam		0x00000001
#define	QF_PAT_PatID		0x00000002
#define	QF_PAT_PatBirDat	0x00000004
#define	QF_PAT_PatBirTim	0x00000008
#define	QF_PAT_PatSex		0x00000010
#define	QF_PAT_NumPatRelStu	0x00000020
#define	QF_PAT_NumPatRelSer	0x00000040
#define	QF_PAT_NumPatRelIma	0x00000080
#define	QF_PAT_InsertDate	0x00000100
#define	QF_PAT_InsertTime	0x00000200
#define	QF_PAT_Owner		0x00000400
#define	QF_PAT_GroupName	0x00000800
#define	QF_PAT_Priv			0x00001000

/*
 * Query Flags for IDB_Select--Study Level
 */
#define	QF_STU_StuDat		0x00000001
#define	QF_STU_StuTim		0x00000002
#define	QF_STU_AccNum		0x00000004
#define	QF_STU_StuID		0x00000008
#define	QF_STU_StuInsUID	0x00000010
#define	QF_STU_RefPhyNam	0x00000020
#define	QF_STU_StuDes		0x00000040
#define	QF_STU_PatAge		0x00000080
#define	QF_STU_PatSiz		0x00000100
#define	QF_STU_PatWei		0x00000200
#define	QF_STU_NumStuRelSer	0x00000300
#define	QF_STU_NumStuRelIma	0x00000400
#define	QF_STU_InsertDate	0x00000800
#define	QF_STU_InsertTime	0x00001000
#define	QF_STU_Owner		0x00002000
#define	QF_STU_GroupName	0x00004000
#define	QF_STU_Priv			0x00008000
#define QF_STU_ModsInStudy	0x00010000

/*
 * Query Flags for IDB_Select--Series Level
 */
#define	QF_SER_Mod			0x00000001
#define	QF_SER_SerNum		0x00000002
#define	QF_SER_SerInsUID	0x00000004
#define	QF_SER_ProNam		0x00000008
#define	QF_SER_SerDes		0x00000010
#define	QF_SER_BodParExa	0x00000020
#define	QF_SER_NumSerRelIma	0x00000040
#define	QF_SER_InsertDate	0x00000080
#define	QF_SER_InsertTime	0x00000100
#define	QF_SER_Owner		0x00000200
#define	QF_SER_GroupName	0x00000400
#define	QF_SER_Priv			0x00000800
#define	QF_SER_ViePos		0x00001000

/*
 * Query Flags for IDB_Select--Image Level
 */
#define	QF_IMA_ImaNum			0x00000001
#define	QF_IMA_SOPInsUID		0x00000002
#define	QF_IMA_SOPClaUID		0x00000004
#define	QF_IMA_SamPerPix		0x00000008
#define	QF_IMA_PhoInt			0x00000010
#define	QF_IMA_Row				0x00000020
#define	QF_IMA_Col				0x00000040
#define	QF_IMA_BitAll			0x00000080
#define	QF_IMA_BitSto			0x00000100
#define	QF_IMA_PixRep			0x00000200
#define	QF_IMA_InsertDate		0x00000400
#define	QF_IMA_InsertTime		0x00000800
#define	QF_IMA_Owner			0x00001000
#define	QF_IMA_GroupName		0x00002000
#define	QF_IMA_Priv				0x00004000
#define	QF_IMA_SOPInsUIDList	0x00008000
#define	QF_IMA_PatOri			0x00010000
#define	QF_IMA_CharSet			0x00020000


typedef struct _IDB_PatientNode {
    char
        PatNam[DICOM_PN_LENGTH + 1],	/* Secondary index */
        PatID[DICOM_LO_LENGTH + 1],	/* Unique index */
        PatBirDat[DICOM_DA_LENGTH + 1],
        PatBirTim[DICOM_TM_LENGTH + 1],
        PatSex[DICOM_CS_LENGTH + 1];
    char
        Owner[IDB_OWNER_LENGTH + 1],
        GroupName[IDB_GROUP_LENGTH + 1],
        Priv[IDB_PRIV_LENGTH + 1];
}   IDB_PatientNode;

typedef struct _IDB_StudyNode {
    char
        StuDat[DICOM_DA_LENGTH + 1],
        StuTim[DICOM_TM_LENGTH + 1],
        AccNum[DICOM_SH_LENGTH + 1],
        StuID[DICOM_SH_LENGTH + 1],	/* Secondary index */
        StuInsUID[DICOM_UI_LENGTH + 1],	/* Unique index */
        RefPhyNam[DICOM_PN_LENGTH + 1],
        StuDes[DICOM_LO_LENGTH + 1],
        PatAge[DICOM_AS_LENGTH + 1],
        PatSiz[DICOM_DS_LENGTH + 1],
        PatWei[DICOM_DS_LENGTH + 1];
    char
        Owner[IDB_OWNER_LENGTH + 1],
        GroupName[IDB_GROUP_LENGTH + 1],
        Priv[IDB_PRIV_LENGTH + 1];
}   IDB_StudyNode;

typedef struct _IDB_SeriesNode {
    char
        Mod[DICOM_CS_LENGTH + 1],
        SerNum[DICOM_IS_LENGTH + 1],
        SerInsUID[DICOM_UI_LENGTH + 1],	/* Unique index */
        ProNam[DICOM_LO_LENGTH + 1],
        SerDes[DICOM_LO_LENGTH + 1],
        BodParExa[DICOM_CS_LENGTH + 1],
        ViePos[DICOM_CS_LENGTH + 1];
    char
        Owner[IDB_OWNER_LENGTH + 1],
        GroupName[IDB_GROUP_LENGTH + 1],
        Priv[IDB_PRIV_LENGTH + 1];
}   IDB_SeriesNode;

typedef struct _IDB_ImageNode {
    char
        ImaNum[DICOM_IS_LENGTH + 1],
        SOPInsUID[DICOM_UI_LENGTH + 1],	/* Unique index */
        SOPClaUID[DICOM_UI_LENGTH + 1],
        PhoInt[DICOM_CS_LENGTH + 1],	/* Unique index */
        PatOri[DICOM_CS_LENGTH + 1];
    int
        SamPerPix,
        Row,
        Col,
        BitAll,
        BitSto,
        PixRep;
    char
        Owner[IDB_OWNER_LENGTH + 1],
        GroupName[IDB_GROUP_LENGTH + 1],
        Priv[IDB_PRIV_LENGTH + 1];

    char
        RespondingTitle[17],
        Medium[33],
        Path[256],
        Transfer[65],
        CharSet[DICOM_CS_LENGTH + 1];
    int
        Size;
}   IDB_ImageNode;

typedef struct _IDB_Insertion {
    IDB_PatientNode 	patient;
    IDB_StudyNode 		study;
    IDB_SeriesNode 		series;
    IDB_ImageNode 		image;
}   IDB_Insertion;


typedef struct _IDB_CONTEXT {
    int		refCount;	/* Reference count, allows multiple opens */
    char
		   *databaseName;
			TBL_HANDLE
			*PatNodes,
			*SerNodes,
			*PatStuNodes,
			*StuNodes,
			*ImaNodes,
			*InsNodes;
#ifdef CTN_IDBV2
    TBL_HANDLE *Limits;
#endif
    struct _IDB_CONTEXT  *next;
}   IDB_CONTEXT;

typedef enum {
    PATIENT_ROOT,
    STUDY_ROOT,
    PATIENTSTUDY_ONLY
}   IDB_QUERY_MODEL;

/*
 * Function Prototypes...
 */
typedef void IDB_HANDLE;

CONDITION
IDB_Open(const char *databaseName, IDB_HANDLE ** handle);
CONDITION
IDB_Close(IDB_HANDLE ** handle);
CONDITION
IDB_InsertImage(IDB_HANDLE ** handle, IDB_Insertion * pssi);
CONDITION
IDB_InsertImageInstance(IDB_HANDLE ** handle, char *imageuid, IDB_InstanceListElement * iie);
CONDITION
IDB_Delete(IDB_HANDLE ** handle, long level, char *uid, CTNBOOLEAN flag);
CONDITION
IDB_Select(IDB_HANDLE ** handle, IDB_QUERY_MODEL model, long begin_level,
		   long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx);
CONDITION
IDB_Select_NoView(IDB_HANDLE ** handle, long begin_level, long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx);
CONDITION
IDB_Select_View(IDB_HANDLE ** handle, long begin_level, long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx);
CONDITION
IDB_Select_ImageLevel(IDB_HANDLE ** handle, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx);
#ifdef CTN_IDBV2
CONDITION
IDB_SelectLimits(IDB_HANDLE **handle, IDB_Limits *limits);
#endif

CONDITION
CBDel_CollectStudies(TBL_FIELD * field, int count, void *ctx);
CONDITION
CBDel_CollectSeries(TBL_FIELD * field, int count, void *ctx);
CONDITION
CBDel_CollectImages(TBL_FIELD * field, int count, void *ctx);

CONDITION
CBIns_CollectInstances(TBL_FIELD * field, int count, void *ctx);

void
IDB_DestroyGlobalLists(void);

CONDITION
CBSel_CollectPatients(TBL_FIELD * fp, int count, void *ctx);
CONDITION
CBSel_CollectStudies(TBL_FIELD * fp, int count, void *ctx);
CONDITION
CBSel_CollectPatientsStudies(TBL_FIELD * fp, int count, void *ctx);
CONDITION
CBSel_CollectSeries(TBL_FIELD * fp, int count, void *ctx);
CONDITION
CBSel_CollectImages(TBL_FIELD * fp, int count, void *ctx);
CONDITION
CBSel_CollectInstances(TBL_FIELD * fp, int count, void *ctx);

long
IDB_ConvertDicomQuerytoSQL(IDB_Query * pssi, long level);
long
IDB_ConvertDicomQuerytoSQL_PSView(IDB_Query * pssi);
char
*IDB_ConvertDicomMetatoSQL(char *str);

#define FREE_STRINGS(index,list,limit)                  \
        for(index=0;index<limit;index++)                \
            if( list[index].Value.Type == TBL_STRING ) free(list[index].Value.Value.String);

char *IDB_Message(CONDITION condition);

/* Define condition values */

#define	IDB_NORMAL				FORM_COND(FAC_IDB, SEV_SUCC, 1)
#define	IDB_UNIMPLEMENTED		FORM_COND(FAC_IDB, SEV_ERROR, 2)
#define	IDB_ALREADYOPENED		FORM_COND(FAC_IDB, SEV_ERROR, 3)
#define	IDB_BADDBTABPAIR		FORM_COND(FAC_IDB, SEV_ERROR, 4)
#define	IDB_NOMEMORY			FORM_COND(FAC_IDB, SEV_ERROR, 5)
#define	IDB_CLOSERROR			FORM_COND(FAC_IDB, SEV_ERROR, 6)
#define	IDB_BADHANDLE			FORM_COND(FAC_IDB, SEV_ERROR, 7)
#define	IDB_BADLEVEL			FORM_COND(FAC_IDB, SEV_ERROR, 8)
#define	IDB_NULLUID				FORM_COND(FAC_IDB, SEV_ERROR, 9)
#define	IDB_BADPATUID			FORM_COND(FAC_IDB, SEV_ERROR, 10)
#define	IDB_BADSTUUID			FORM_COND(FAC_IDB, SEV_ERROR, 11)
#define	IDB_BADSERUID			FORM_COND(FAC_IDB, SEV_ERROR, 12)
#define	IDB_BADIMAUID			FORM_COND(FAC_IDB, SEV_ERROR, 13)
#define	IDB_BADLISTENQ			FORM_COND(FAC_IDB, SEV_ERROR, 14)
#define	IDB_NOINSERTDATA		FORM_COND(FAC_IDB, SEV_ERROR, 15)
#define	IDB_BADLEVELSEQ			FORM_COND(FAC_IDB, SEV_ERROR, 16)
#define	IDB_NOMATCHES			FORM_COND(FAC_IDB, SEV_ERROR, 17)
#define	IDB_EARLYEXIT			FORM_COND(FAC_IDB, SEV_ERROR, 18)
#define	IDB_DUPINSTANCE			FORM_COND(FAC_IDB, SEV_ERROR, 19)
#define	IDB_PATIDMISMATCH		FORM_COND(FAC_IDB, SEV_ERROR, 20)
#define	IDB_DELETEFAILED		FORM_COND(FAC_IDB, SEV_ERROR, 21)
#define	IDB_INSERTFAILED		FORM_COND(FAC_IDB, SEV_ERROR, 22)
#define	IDB_FILEDELETEFAILED	FORM_COND(FAC_IDB, SEV_ERROR, 23)


#ifdef  __cplusplus
}
#endif

#endif
