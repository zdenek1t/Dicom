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
** Module Name(s):	IDB_Open,IDB_Close,IDB_InsertImage,IDB_InsertInstance,
**			IDB_Delete, IDB_Select
** Author, Date:	David E. Beecher, 14-Mar-94
** Intent:		Provides access functions to the Dicom
**			Image database.  Relies quite heavily
**			on the TBL and UTL facility.
** Last Update:		$Author: smm $, $Date: 1999/12/30 07:39:04 $
** Source File:		$RCSfile: idb.c,v $
** Revision:		$Revision: 1.48 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.48 $ $RCSfile: idb.c,v $";
#include <stdio.h>
#include <stdlib.h>
#ifndef IRIX
/*#include <unistd.h> */
#endif
#include <string.h>
#include <ctype.h>
#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/time.h>
#endif

#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "../idb/idb.h"
#include "../utility/utility.h"
#include "../condition/condition.h"
#include "../thread/ctnthread.h"

#define SCALING_FACTOR 1000

static IDB_CONTEXT
*   GS_ContextHead = (IDB_CONTEXT *) NULL;

static LST_HEAD
*   GS_pats,
   *GS_stus,
   *GS_sers,
   *GS_imas,
   *GS_instances;

/*
 * These statics are needed by the Select routine to ensure that
 * the storage hangs around for a while... and so we don't have to malloc
 * and free it...  These are actually used for the criteria lists that need
 * to get built.
 */
static long
    GS_PAT_PatBirDatB,
    GS_PAT_PatBirDatE,
    GS_PAT_InsertDateB,
    GS_PAT_InsertDateE,
    GS_STU_StuDatB,
    GS_STU_StuDatE,
    GS_STU_InsertDateB,
    GS_STU_InsertDateE,
    GS_SER_InsertDateB,
    GS_SER_InsertDateE,
    GS_IMA_SamPerPix,
    GS_IMA_Row,
    GS_IMA_Col,
    GS_IMA_BitAll,
    GS_IMA_BitSto,
    GS_IMA_PixRep,
    GS_IMA_InsertDateB,
    GS_IMA_InsertDateE;

static float
    GS_PAT_PatBirTimB,
    GS_PAT_PatBirTimE,
    GS_PAT_InsertTimeB,
    GS_PAT_InsertTimeE,
    GS_STU_StuTimB,
    GS_STU_StuTimE,
    GS_STU_InsertTimeB,
    GS_STU_InsertTimeE,
    GS_SER_InsertTimeB,
    GS_SER_InsertTimeE,
    GS_IMA_InsertTimeB,
    GS_IMA_InsertTimeE;

static TBL_CRITERIA
    GS_patcl[20],
    GS_stucl[24],
    GS_patstucl[44],
    GS_sercl[17],
    GS_imacl[21];
/*
 * These statics are needed by the Select routine to ensure that
 * the storage hangs around for a while... and so we don't have to malloc
 * and free it...  These are actually used for the field lists that need
 * to get built.
 */

static char
    GS_PATSEL_PatNam[DICOM_PN_LENGTH + 1],
    GS_PATSEL_PatID[DICOM_LO_LENGTH + 1],
    GS_PATSEL_PatSex[DICOM_CS_LENGTH + 1],
    GS_PATSEL_Owner[IDB_OWNER_LENGTH + 1],
    GS_PATSEL_GroupName[IDB_GROUP_LENGTH + 1],
    GS_PATSEL_Priv[IDB_PRIV_LENGTH + 1];
static long
    GS_PATSEL_NumPatRelStu,
    GS_PATSEL_NumPatRelSer,
    GS_PATSEL_NumPatRelIma;
static long
    GS_PATSEL_PatBirDat,
    GS_PATSEL_InsertDate;
static float
    GS_PATSEL_InsertTime,
    GS_PATSEL_PatBirTim;

static char
    GS_STUSEL_AccNum[DICOM_SH_LENGTH + 1],
    GS_STUSEL_StuID[DICOM_SH_LENGTH + 1],
    GS_STUSEL_StuInsUID[DICOM_UI_LENGTH + 1],
    GS_STUSEL_RefPhyNam[DICOM_PN_LENGTH + 1],
    GS_STUSEL_StuDes[DICOM_LO_LENGTH + 1],
    GS_STUSEL_PatAge[DICOM_AS_LENGTH + 1],
    GS_STUSEL_PatSiz[DICOM_DS_LENGTH + 1],
    GS_STUSEL_PatWei[DICOM_DS_LENGTH + 1],
    GS_STUSEL_Owner[IDB_OWNER_LENGTH + 1],
    GS_STUSEL_GroupName[IDB_GROUP_LENGTH + 1],
    GS_STUSEL_Priv[IDB_PRIV_LENGTH + 1],
    GS_STUSEL_PatParent[DICOM_UI_LENGTH + 1],
	GS_STUSEL_ModsInStudy[DICOM_CS_LENGTH + 1];
static long
    GS_STUSEL_NumStuRelSer,
    GS_STUSEL_NumStuRelIma;
static long
    GS_STUSEL_StuDat,
    GS_STUSEL_InsertDate;
static float
    GS_STUSEL_StuTim,
    GS_STUSEL_InsertTime;

static char
    GS_SERSEL_Mod[DICOM_CS_LENGTH + 1],
    GS_SERSEL_SerNum[DICOM_IS_LENGTH + 1],
    GS_SERSEL_SerInsUID[DICOM_UI_LENGTH + 1],
    GS_SERSEL_ProNam[DICOM_LO_LENGTH + 1],
    GS_SERSEL_SerDes[DICOM_LO_LENGTH + 1],
    GS_SERSEL_BodParExa[DICOM_CS_LENGTH + 1],
    GS_SERSEL_ViePos[DICOM_CS_LENGTH + 1],
    GS_SERSEL_Owner[IDB_OWNER_LENGTH + 1],
    GS_SERSEL_GroupName[IDB_GROUP_LENGTH + 1],
    GS_SERSEL_Priv[IDB_PRIV_LENGTH + 1],
    GS_SERSEL_StuParent[DICOM_UI_LENGTH + 1];
static long
    GS_SERSEL_NumSerRelIma;
static long
    GS_SERSEL_InsertDate;
static float
    GS_SERSEL_InsertTime;

static char
    GS_IMASEL_ImaNum[DICOM_IS_LENGTH + 1],
    GS_IMASEL_SOPInsUID[DICOM_UI_LENGTH + 1],
    GS_IMASEL_SOPClaUID[DICOM_UI_LENGTH + 1],
    GS_IMASEL_PhoInt[DICOM_CS_LENGTH + 1],
    GS_IMASEL_PatOri[DICOM_CS_LENGTH + 1],
    GS_IMASEL_Owner[IDB_OWNER_LENGTH + 1],
    GS_IMASEL_GroupName[IDB_GROUP_LENGTH + 1],
    GS_IMASEL_Priv[IDB_PRIV_LENGTH + 1],
    GS_IMASEL_SerParent[DICOM_UI_LENGTH + 1],
    GS_IMASEL_SOPInsUIDList[2048];
static long
    GS_IMASEL_SamPerPix,
    GS_IMASEL_Row,
    GS_IMASEL_Col,
    GS_IMASEL_BitAll,
    GS_IMASEL_BitSto,
    GS_IMASEL_PixRep;
static long
    GS_IMASEL_InsertDate;
static float
    GS_IMASEL_InsertTime;

static char
    GS_INSSEL_RespondingTitle[17],
    GS_INSSEL_Medium[33],
    GS_INSSEL_Path[256],
    GS_INSSEL_Transfer[65];
static long
    GS_INSSEL_Size;

static TBL_FIELD
GS_PATSEL_Field[] =
	{"PatNam", TBL_STRING, DICOM_PN_LENGTH + 1, DICOM_PN_LENGTH + 1, 0, GS_PATSEL_PatNam,
    "PatID", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_PATSEL_PatID,
    "PatSex", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_PATSEL_PatSex,
    "Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_PATSEL_Owner,
    "GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_PATSEL_GroupName,
    "Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_PATSEL_Priv,

    "PatBirDat", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_PatBirDat,
    "PatBirTim", TBL_FLOAT4, 4, 4, 0, &GS_PATSEL_PatBirTim,
    "InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_InsertDate,
    "InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_PATSEL_InsertTime,
    "NumPatRelStu", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelStu,
    "NumPatRelSer", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelSer,
    "NumPatRelIma", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelIma,
    0
};

static TBL_FIELD
GS_STUSEL_Field[] =
{
    "AccNum", TBL_STRING, DICOM_SH_LENGTH + 1, DICOM_SH_LENGTH + 1, 0, GS_STUSEL_AccNum,
    "StuID", TBL_STRING, DICOM_SH_LENGTH + 1, DICOM_SH_LENGTH + 1, 0, GS_STUSEL_StuID,
    "StuInsUID", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_STUSEL_StuInsUID,
    "RefPhyNam", TBL_STRING, DICOM_PN_LENGTH + 1, DICOM_PN_LENGTH + 1, 0, GS_STUSEL_RefPhyNam,
    "StuDes", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_STUSEL_StuDes,
    "PatAge", TBL_STRING, DICOM_AS_LENGTH + 1, DICOM_AS_LENGTH + 1, 0, GS_STUSEL_PatAge,
    "PatSiz", TBL_STRING, DICOM_DS_LENGTH + 1, DICOM_DS_LENGTH + 1, 0, GS_STUSEL_PatSiz,
    "PatWei", TBL_STRING, DICOM_DS_LENGTH + 1, DICOM_DS_LENGTH + 1, 0, GS_STUSEL_PatWei,
    "Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_STUSEL_Owner,
    "GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_STUSEL_GroupName,
    "Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_STUSEL_Priv,
    "PatParent", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_STUSEL_PatParent,

    "StuDat", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_StuDat,
    "StuTim", TBL_FLOAT4, 4, 4, 0, &GS_STUSEL_StuTim,
    "InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_InsertDate,
    "InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_STUSEL_InsertTime,
    "NumStuRelSer", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_NumStuRelSer,
    "NumStuRelIma", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_NumStuRelIma,
    0
};

/* PatientStudyView */
static TBL_FIELD
GS_PATSTUSEL_Field[] =
	{"Pat_PatNam", TBL_STRING, DICOM_PN_LENGTH + 1, DICOM_PN_LENGTH + 1, 0, GS_PATSEL_PatNam,
     "Pat_PatID", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_PATSEL_PatID,
     "Pat_PatSex", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_PATSEL_PatSex,
     "Pat_Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_PATSEL_Owner,
     "Pat_GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_PATSEL_GroupName,
     "Pat_Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_PATSEL_Priv,

     "Pat_PatBirDat", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_PatBirDat,
     "Pat_PatBirTim", TBL_FLOAT4, 4, 4, 0, &GS_PATSEL_PatBirTim,
     "Pat_InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_InsertDate,
     "Pat_InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_PATSEL_InsertTime,
     "Pat_NumPatRelStu", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelStu,
     "Pat_NumPatRelSer", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelSer,
     "Pat_NumPatRelIma", TBL_SIGNED4, 4, 4, 0, &GS_PATSEL_NumPatRelIma,

     "Stu_AccNum", TBL_STRING, DICOM_SH_LENGTH + 1, DICOM_SH_LENGTH + 1, 0, GS_STUSEL_AccNum,
     "Stu_StuID", TBL_STRING, DICOM_SH_LENGTH + 1, DICOM_SH_LENGTH + 1, 0, GS_STUSEL_StuID,
     "Stu_StuInsUID", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_STUSEL_StuInsUID,
     "Stu_RefPhyNam", TBL_STRING, DICOM_PN_LENGTH + 1, DICOM_PN_LENGTH + 1, 0, GS_STUSEL_RefPhyNam,
     "Stu_StuDes", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_STUSEL_StuDes,
     "Stu_PatAge", TBL_STRING, DICOM_AS_LENGTH + 1, DICOM_AS_LENGTH + 1, 0, GS_STUSEL_PatAge,
     "Stu_PatSiz", TBL_STRING, DICOM_DS_LENGTH + 1, DICOM_DS_LENGTH + 1, 0, GS_STUSEL_PatSiz,
     "Stu_PatWei", TBL_STRING, DICOM_DS_LENGTH + 1, DICOM_DS_LENGTH + 1, 0, GS_STUSEL_PatWei,
     "Stu_Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_STUSEL_Owner,
     "Stu_GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_STUSEL_GroupName,
     "Stu_Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_STUSEL_Priv,
     "Ser_ModsInStudy", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_STUSEL_ModsInStudy,

     "Stu_StuDat", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_StuDat,
     "Stu_StuTim", TBL_FLOAT4, 4, 4, 0, &GS_STUSEL_StuTim,
     "Stu_InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_InsertDate,
     "Stu_InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_STUSEL_InsertTime,
     "Stu_NumStuRelSer", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_NumStuRelSer,
     "Stu_NumStuRelIma", TBL_SIGNED4, 4, 4, 0, &GS_STUSEL_NumStuRelIma,
    0
};

static TBL_FIELD
GS_SERSEL_Field[] =
{
    "Mod", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_SERSEL_Mod,
    "SerNum", TBL_STRING, DICOM_IS_LENGTH + 1, DICOM_IS_LENGTH + 1, 0, GS_SERSEL_SerNum,
    "SerInsUID", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_SERSEL_SerInsUID,
    "ProNam", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_SERSEL_ProNam,
    "SerDes", TBL_STRING, DICOM_LO_LENGTH + 1, DICOM_LO_LENGTH + 1, 0, GS_SERSEL_SerDes,
    "BodParExa", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_SERSEL_BodParExa,
    "ViePos", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_SERSEL_ViePos,
    "Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_SERSEL_Owner,
    "GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_SERSEL_GroupName,
    "Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_SERSEL_Priv,
    "StuParent", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_SERSEL_StuParent,

    "InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_SERSEL_InsertDate,
    "InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_SERSEL_InsertTime,
    "NumSerRelIma", TBL_SIGNED4, 4, 4, 0, &GS_SERSEL_NumSerRelIma,
    0
};

static TBL_FIELD
    GS_IMASEL_Field[] =
{
    "ImaNum", TBL_STRING, DICOM_IS_LENGTH + 1, DICOM_IS_LENGTH + 1, 0, GS_IMASEL_ImaNum,
    "SOPInsUID", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_IMASEL_SOPInsUID,
    "SOPClaUID", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_IMASEL_SOPClaUID,
    "PhoInt", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_IMASEL_PhoInt,
    "PatOri", TBL_STRING, DICOM_CS_LENGTH + 1, DICOM_CS_LENGTH + 1, 0, GS_IMASEL_PatOri,
    "Owner", TBL_STRING, IDB_OWNER_LENGTH + 1, IDB_OWNER_LENGTH + 1, 0, GS_IMASEL_Owner,
    "GroupName", TBL_STRING, IDB_GROUP_LENGTH + 1, IDB_GROUP_LENGTH + 1, 0, GS_IMASEL_GroupName,
    "Priv", TBL_STRING, IDB_PRIV_LENGTH + 1, IDB_PRIV_LENGTH + 1, 0, GS_IMASEL_Priv,
    "SerParent", TBL_STRING, DICOM_UI_LENGTH + 1, DICOM_UI_LENGTH + 1, 0, GS_IMASEL_SerParent,

    "InsertDate", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_InsertDate,
    "InsertTime", TBL_FLOAT4, 4, 4, 0, &GS_IMASEL_InsertTime,
    "SamPerPix", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_SamPerPix,
    "Row", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_Row,
    "Col", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_Col,
    "BitAll", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_BitAll,
    "BitSto", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_BitSto,
    "PixRep", TBL_SIGNED4, 4, 4, 0, &GS_IMASEL_PixRep,
    0,
};

static TBL_FIELD
    GS_INSSEL_Field[] =
{
    "RespondingTitle", TBL_STRING, 17, 17, 0, GS_INSSEL_RespondingTitle,
    "Medium", TBL_STRING, 33, 33, 0, GS_INSSEL_Medium,
    "Path", TBL_STRING, 256, 256, 0, GS_INSSEL_Path,
    "Transfer", TBL_STRING, 65, 65, 0, GS_INSSEL_Transfer,
    "Size", TBL_SIGNED4, 4, 4, 0, &GS_INSSEL_Size,
    0,
};

static IDB_PatientQuery
*   GS_PatStuNodes,
   *GS_PatNodes;
static long
   *GS_NullPatFlag,
    GS_NumPatNodes;
static IDB_StudyQuery
*   GS_StuNodes;
static long
   *GS_NullStuFlag,
    GS_NumStuNodes;
static IDB_SeriesQuery
*   GS_SerNodes;
static long
   *GS_NullSerFlag,
    GS_NumSerNodes;
static IDB_ImageQuery
*   GS_ImaNodes;
static long
   *GS_NullImaFlag,
    GS_NumImaNodes;

static TBL_OPERATOR likeOrEqualOperator(const char* c)
{
  TBL_OPERATOR op = TBL_EQUAL;

    while (*c != '\0'){
    	if (*c == '*' || *c == '?') {
    		op = TBL_LIKE;
    		break;
    	}else{
    		c++;
    	}
    }
    return op;
}

static CONDITION
deleteImageFiles(TBL_HANDLE ** handle, TBL_CRITERIA * criteria, long *bytesFreed);

static void
replaceBadFields(TBL_FIELD * imafld, char *replacementString)
{
    for (; imafld->FieldName != 0; imafld++){
    	if (imafld->Value.Type == TBL_STRING){
    		if (strcmp(imafld->Value.Value.String, "\\") == 0) imafld->Value.Value.String = replacementString;
    	}
    }
}

#if 0
static CONDITION
localIncrement(TBL_HANDLE ** h, TBL_CRITERIA * crit, TBL_UPDATE * update)
{
    int 		value;
    long 		count;
    TBL_FIELD 	field[2];
    CONDITION 	cond;
    TBL_UPDATE 	localUpdate[2];

    field[0].FieldName = update->FieldName;
    field[0].Value.AllocatedSize = 4;
    field[0].Value.Type = TBL_SIGNED4;
    field[0].Value.Value.Signed4 = &value;

    field[1].FieldName = 0;

    count = 0;

    cond = TBL_Select(h, crit, field, &count, NULL, 0);
    if (cond != TBL_NORMAL)	return cond;
    if (count != 1)	return 0;

    value++;
    localUpdate[0] = *update;
    localUpdate[0].Function = TBL_SET;
    localUpdate[0].Value.Type = TBL_SIGNED4;
    localUpdate[0].Value.Value.Signed4 = (int *) &value;
    localUpdate[1].FieldName = 0;

    cond = TBL_Update(h, crit, localUpdate);
    return cond;
}
#endif
static CONDITION
localIncrement(TBL_HANDLE ** h, TBL_CRITERIA * crit, TBL_UPDATE * update)
{
    long 		value[10];
    long 		count;
    TBL_FIELD 	field[11];
    CONDITION 	cond;
    TBL_UPDATE 	localUpdate[11];
    int 		i;

    for (i = 0; i < 10 && update[i].FieldName != 0; i++){
    	field[i].FieldName = update[i].FieldName;
    	field[i].Value.AllocatedSize = 4;
    	field[i].Value.Type = TBL_SIGNED4;
    	field[i].Value.Value.Signed4 = (int *) (&value[i]);
    }

    if (i >= 10){
    	fprintf(stderr, "IDB:localIncrement too many fields to update\n");
    	exit(1);
    }
    field[i].FieldName = 0;

    count = 0;

    cond = TBL_Select(h, crit, field, &count, NULL, 0);
    if (cond != TBL_NORMAL)	return cond;
    if (count != 1)	return 0;

    for (i = 0; update[i].FieldName != 0; i++) {
    	if (update[i].Function == TBL_ADD){
    		value[i] += *update[i].Value.Value.Signed4;
    	}else if (update[i].Function == TBL_INCREMENT){
    		value[i]++;
    	}else{
    		fprintf(stderr, "IDB:localIncrement unsupported function: %d\n", update[i].Function);
    		exit(1);
    	}
    	localUpdate[i] = update[i];
    	localUpdate[i].Function = TBL_SET;
    	localUpdate[i].Value.Type = TBL_SIGNED4;
    	localUpdate[i].Value.Value.Signed4 = (int *) (&value[i]);
    }
    localUpdate[i].FieldName = 0;

    cond = TBL_Update(h, crit, localUpdate);
    return cond;
}

static CONDITION
localDecrement(TBL_HANDLE ** h, TBL_CRITERIA * crit, TBL_UPDATE * update)
{
    long 		value[10];
    long 		count;
    TBL_FIELD 	field[11];
    CONDITION 	cond;
    TBL_UPDATE 	localUpdate[11];
    int 		i;

    for (i = 0; i < 10 && update[i].FieldName != 0; i++){
    	field[i].FieldName = update[i].FieldName;
    	field[i].Value.AllocatedSize = 4;
    	field[i].Value.Type = TBL_SIGNED4;
    	field[i].Value.Value.Signed4 = (int *) (&value[i]);
    }
    if (i >= 10) {
    	fprintf(stderr, "IDB:localDecrement too many fields to update\n");
    	exit(1);
    }
    field[i].FieldName = 0;

    count = 0;
    cond = TBL_Select(h, crit, field, &count, NULL, 0);
    if (cond != TBL_NORMAL) return cond;
    if (count != 1)	return 0;

    for (i = 0; update[i].FieldName != 0; i++){
    	if (update[i].Function == TBL_SUBTRACT){
    		value[i] -= *update[i].Value.Value.Signed4;
    	}else if (update[i].Function == TBL_DECREMENT){
    		value[i]--;
    	}else{
    		fprintf(stderr, "IDB:localDecrement unsupported function: %d\n", update[i].Function);
    		exit(1);
    	}
    	localUpdate[i] = update[i];
    	localUpdate[i].Function = TBL_SET;
    	localUpdate[i].Value.Type = TBL_SIGNED4;
    	localUpdate[i].Value.Value.Signed4 = (int *) (&value[i]);
    }
    localUpdate[i].FieldName = 0;

    cond = TBL_Update(h, crit, localUpdate);
    return cond;
}

#ifdef CTN_IDBV2
static void
updateLimitsTable(TBL_HANDLE * limits, int imageSize, int patientCountDelta, int studyCountDelta, int imageCountDelta)
{
    TBL_UPDATE 	update[10];
    int 		i = 0;
    CONDITION 	cond = TBL_NORMAL;

    if (imageCountDelta != 0){
    	update[i].FieldName = "ImageCount";
    	update[i].Function = TBL_ADD;
    	update[i].Value.AllocatedSize = 4;
    	update[i].Value.Type = TBL_SIGNED4;
    	update[i].Value.Value.Signed4 = &imageCountDelta;
    	i++;
    }
    if (imageSize != 0) {
    	update[i].FieldName = "DBSize";
    	update[i].Function = TBL_ADD;
    	update[i].Value.AllocatedSize = 4;
    	update[i].Value.Type = TBL_SIGNED4;
    	update[i].Value.Value.Signed4 = &imageSize;
    	i++;
    }
    if (studyCountDelta) {
    	update[i].FieldName = "StudyCount";
    	update[i].Function = TBL_ADD;
    	update[i].Value.AllocatedSize = 4;
    	update[i].Value.Type = TBL_SIGNED4;
    	update[i].Value.Value.Signed4 = &studyCountDelta;
    	i++;
    }
    if (patientCountDelta) {
    	update[i].FieldName = "PatientCount";
    	update[i].Function = TBL_ADD;
    	update[i].Value.AllocatedSize = 4;
    	update[i].Value.Type = TBL_SIGNED4;
    	update[i].Value.Value.Signed4 = &patientCountDelta;
    	i++;
    }
    if (i == 0)	return;

    update[i].FieldName = 0;

    if (TBL_HasUpdateIncrement()){
    	cond = TBL_Update(&limits, NULL, update);
    }else{
    	cond = localIncrement(&limits, NULL, update);
    }
}
#endif

/* IDB_Open
**
** Purpose:
**	This routine attempts to open for access the database pointed to by
**	the input string databaseName.
**
** Parameter Dictionary:
**	char *databaseName:
**		the name of the database to open
**	IDB_HANDLE **handle:
**		will contain the newly database handle upon success
**
**
** Return Values:
**	IDB_NORMAL: 	The open succeeded and a new database has been opened.
**	IDB_ALREADYOPENED: The database specified is already opened.
**	IDB_BADDBTABPAIR: For each database, a certain number of tables must
**		successfully be opened, a failure in any one of the pairs results
**		in this return code.
**	IDB_NOMEMORY:	No memory is available to maintain the internal open
**		database list.
**
** Algorithm:
**	Open uses the TBL facility extensively to determine if the needed tables
**	can be opened and accessed.  If so, this routine allocated a context which
**	contains pointers to the tables just opened and saves this context in a
**	linked list maintained by IDB_Open and IDB_Close.
**
*/
CONDITION
IDB_Open(const char *databaseName, IDB_HANDLE ** handle)
{

    IDB_CONTEXT		* idbc;
    char		    *tdb;
    TBL_HANDLE		* pathandle, *stuhandle, *patstuhandle,	*serhandle,	*imahandle,	*inshandle;

    THR_ObtainMutex(FAC_IDB);

    (*handle) = (void *) NULL;

    idbc = GS_ContextHead;
    while (idbc != (IDB_CONTEXT *) NULL){
    	if ((strcmp(idbc->databaseName, databaseName) == 0)){
#if 1
    		/* Changed this on 1-Feb-1998.  Use reference counting to allow multiple users of the IDB facility/tables */
    		idbc->refCount++;
    		(*handle) = (void *) GS_ContextHead;
    		THR_ReleaseMutex(FAC_IDB);
    		return IDB_NORMAL;
#else
    		return COND_PushCondition(IDB_ERROR(IDB_ALREADYOPENED), databaseName, "IDB_Open");
#endif
    	}
    	idbc = idbc->next;
    }
    /*
     * Now the database has been found...let's open up all the tables to see
     * if everything is ok.
     */
    if (TBL_Open(databaseName, "Patient", &pathandle) != TBL_NORMAL){
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Patient", "IDB_Open");
    }
    if (TBL_HasViews()) {
    	if (TBL_Open(databaseName, "PatientStudyView", &patstuhandle) != TBL_NORMAL) {
    		TBL_Close(&pathandle);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "PatientStudyView", "IDB_Open");
    	}
    }
    if (TBL_Open(databaseName, "Study", &stuhandle) != TBL_NORMAL){
    	TBL_Close(&pathandle);
    	if (TBL_HasViews())	TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Study", "IDB_Open");
    }
    if (TBL_Open(databaseName, "Series", &serhandle) != TBL_NORMAL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Series", "IDB_Open");
    }
    if (TBL_Open(databaseName, "Image", &imahandle) != TBL_NORMAL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	TBL_Close(&serhandle);
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Image", "IDB_Open");
    }
    if (TBL_Open(databaseName, "Instance", &inshandle) != TBL_NORMAL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	TBL_Close(&serhandle);
    	TBL_Close(&imahandle);
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Instance", "IDB_Open");
    }
#ifdef CTN_IDBV2
    if (TBL_Open(databaseName, "Limits", &limitshandle) != TBL_NORMAL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	TBL_Close(&serhandle);
    	TBL_Close(&imahandle);
    	TBL_Close(&inshandle);
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADDBTABPAIR), databaseName, "Instance", "IDB_Open");
    }
#endif
    /*
     * We have successfully opened all the pairs...now we need to add them to
     * our list of open databases.
     */
    if ((idbc = (IDB_CONTEXT *) malloc(sizeof(IDB_CONTEXT))) == (IDB_CONTEXT *) NULL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	TBL_Close(&serhandle);
    	TBL_Close(&imahandle);
    	TBL_Close(&inshandle);
#ifdef CTN_IDBV2
    	TBL_Close(&limitshandle);
#endif
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Open");
    }
    idbc->refCount = 1;

    if ((tdb = (char *) malloc(strlen(databaseName) + 1)) == (char *) NULL) {
    	TBL_Close(&pathandle);
    	TBL_Close(&stuhandle);
    	TBL_Close(&serhandle);
    	TBL_Close(&imahandle);
    	TBL_Close(&inshandle);
#ifdef CTN_IDBV2
    	TBL_Close(&limitshandle);
#endif
    	if (TBL_HasViews()) TBL_Close(&patstuhandle);
    	free(idbc);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Open");
    }

    strcpy(tdb, databaseName);
    idbc->databaseName = tdb;
    idbc->PatNodes = pathandle;
    if (TBL_HasViews())	idbc->PatStuNodes = patstuhandle;
    idbc->StuNodes = stuhandle;
    idbc->SerNodes = serhandle;
    idbc->ImaNodes = imahandle;
    idbc->InsNodes = inshandle;
#ifdef CTN_IDBV2
    idbc->Limits = limitshandle;
#endif
    idbc->next = GS_ContextHead;
    GS_ContextHead = idbc;

    (*handle) = (void *) GS_ContextHead;

    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;

}


/* IDB_Close
**
** Purpose:
**	This routine closes a previously opened database.
**
** Parameter Dictionary:
**	IDB_HANDLE **handle:
**		the database handle.
**
**
** Return Values:
**
**	IDB_NORMAL: 	The close command succeeded
**	IDB_CLOSERROR:	The database handle passed could not be found.
**
** Algorithm:
**	This routine attempts to find the handle in it's internal table
**	of opened database descriptors and closes all the tables associated
**	with that descriptor.
**
*/
CONDITION
IDB_Close(IDB_HANDLE ** handle)
{
    IDB_CONTEXT		* previdbc,	*idbc, *hc;


    THR_ObtainMutex(FAC_IDB);

    hc = (IDB_CONTEXT *) (*handle);
    previdbc = idbc = GS_ContextHead;

    while (idbc != (IDB_HANDLE *) NULL){
    	if (hc == idbc){
    		idbc->refCount--;
    		if (idbc->refCount > 0){
    			THR_ReleaseMutex(FAC_IDB);
    			return IDB_NORMAL;
    		}
    		free(idbc->databaseName);
    		TBL_Close(&(idbc->PatNodes));
    		TBL_Close(&(idbc->StuNodes));
    		TBL_Close(&(idbc->SerNodes));
    		TBL_Close(&(idbc->ImaNodes));
    		TBL_Close(&(idbc->InsNodes));
    		if (TBL_HasViews()) TBL_Close(&(idbc->PatStuNodes));
    		if (idbc == GS_ContextHead){
    			GS_ContextHead = idbc->next;
    		}else{
    			previdbc->next = idbc->next;
    		}
#ifdef CTN_IDBV2
    		TBL_Close(&(idbc->Limits));
#endif
    		free(idbc);
    		(*handle) = (IDB_HANDLE *) NULL;
    		THR_ReleaseMutex(FAC_IDB);
    		return IDB_NORMAL;
    	}
    	previdbc = idbc;
    	idbc = idbc->next;
    }
    THR_ReleaseMutex(FAC_IDB);
    return COND_PushCondition(IDB_ERROR(IDB_CLOSERROR), "IDB_Close");
}


/* IDB_Delete
**
** Purpose:
**	This routine deletes node(s) in the heirarchy starting
**	from the selected uid.
**
** Parameter Dictionary:
**	IDB_HANDLE **handle:
**		the database identifier.
**	long level:
**		The level in the hierarchy specifiying
**		where the next parameter, uid, will be found.  level
**		must be one of the predefined constants,
**		IDB_PATIENT_LEVEL, IDB_STUDY_LEVEL, IDB_SERIES_LEVEL,
**		or IDB_IMAGE_LEVEL.
**	char *uid:
**		Specifies the uid of the node at which to begin
**		the deletion.
**	CTNBOOLEAN flag
**		Specifies if instances (files) should actually be deleted.
**
** Return Values:
**	IDB_NORMAL: 	The close command succeeded
**	IDB_BADHANDLE:	The handle passed is invalid
**	IDB_BADLEVEL:	The level passed is invalid
**	IDB_NULLUID:	The uid passed is null
**	IDB_BADPATUID:	The uid passed is a Patient ID is either
**		not present in the database or is duplicated in the
**		database.
**	IDB_BADSTUUID:	The uid passed is a Study UID is either
**		not present in the database or is duplicated in the
**		database.
**	IDB_BADSERUID:	The uid passed is a Series UID is either
**		not present in the database or is duplicated in the
**		database.
**	IDB_BADIMAUID:	The uid passed is an Image UID is either
**		not present in the database or is duplicated in the
**		database.
**	IDB_NOMEMORY:	No memory available for this routine.
**	IDB_BADLISTENQ: The attempt to add to a list failed
**
** Algorithm:
**	IDB_Delete creates lists of all the uid's to be deleted and
**	then simply issues the appropriate TBL_Delete calls to perform
**	that task.  It also updates counts in undeleted nodes where
**	appropriate.
**
*/
CONDITION
IDB_Delete(IDB_HANDLE ** handle, long level, char *uid, CTNBOOLEAN flag)
{
    TBL_UPDATE		update[10];
    TBL_FIELD		field[10];
    TBL_CRITERIA	criteria[10];
    TBL_HANDLE		* pathandle, *stuhandle, *serhandle, *imahandle, *inshandle;
#ifdef CTN_IDBV2
    TBL_HANDLE 		*limitsHandle;
#endif
    char	        patientuid[DICOM_UI_LENGTH + 1], studyuid[DICOM_UI_LENGTH + 1], seriesuid[DICOM_UI_LENGTH + 1], buf[DICOM_LO_LENGTH + 1];
    LST_NODE		* temp_node;
    char	        *temp_string;
    IDB_CONTEXT		* idbc;
    int		        PatientsDeleted, StudiesDeleted, SeriesDeleted, ImagesDeleted;
    long	        count, foundit,
					storageSpaceReleased = 0;	/* Bytes freed by deleting images */
    CONDITION		ret_val;

    THR_ObtainMutex(FAC_IDB);

    idbc = GS_ContextHead;
    foundit = 0;
    while (idbc != (IDB_CONTEXT *) NULL){
    	if (idbc == (IDB_CONTEXT *) (*handle)){
    		pathandle = idbc->PatNodes;
    		stuhandle = idbc->StuNodes;
    		serhandle = idbc->SerNodes;
    		imahandle = idbc->ImaNodes;
    		inshandle = idbc->InsNodes;
#ifdef CTN_IDBV2
    		limitsHandle = idbc->Limits;
#endif
    		foundit = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_Delete");
    }
    if (level != IDB_PATIENT_LEVEL && level != IDB_STUDY_LEVEL && level != IDB_SERIES_LEVEL && level != IDB_IMAGE_LEVEL){
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVEL), "IDB_Delete");
    }
    if (uid == (char *) NULL){
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_NULLUID), "IDB_Delete");
    }
    /*
     * Now we have the handle...we need to determine which nodes to delete.
     */
    GS_pats = LST_Create();
    GS_stus = LST_Create();
    GS_sers = LST_Create();
    GS_imas = LST_Create();
    count = 0;
    temp_node = (LST_NODE *) malloc((2 * sizeof(void *)) + (strlen(uid) +1));
    temp_string = ((char *) temp_node) + (2 * sizeof(void *));

    if (temp_node == (LST_NODE *) NULL) {
    	LST_Destroy(&GS_pats);
    	LST_Destroy(&GS_stus);
    	LST_Destroy(&GS_sers);
    	LST_Destroy(&GS_imas);
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Delete");
    }
    strcpy(temp_string, uid);

    if (level == IDB_PATIENT_LEVEL) {
    	field[0].FieldName = "PatID";
    	field[0].Value.AllocatedSize = DICOM_LO_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = patientuid;
    	field[1].FieldName = 0;

    	criteria[0].FieldName = "PatID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = uid;
    	criteria[1].FieldName = 0;
    	ret_val = TBL_Select(&pathandle, criteria, field, &count, NULL, NULL);
    	if (ret_val != TBL_NORMAL || count != 1){
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADPATUID), uid, "IDB_Delete");
    	}
    	if (LST_Enqueue(&GS_pats, temp_node) != LST_NORMAL) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    	}
	/*
	 * No update of statistics is needed here...
	 */
    }else if (level == IDB_STUDY_LEVEL){
    	criteria[0].FieldName = "StuInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = uid;
    	criteria[1].FieldName = 0;

    	field[0].FieldName = "PatParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = patientuid;

    	field[1].FieldName = "NumStuRelSer";
    	field[1].Value.AllocatedSize = 4;
    	field[1].Value.Type = TBL_SIGNED4;
    	field[1].Value.Value.Signed4 = &SeriesDeleted;

    	field[2].FieldName = "NumStuRelIma";
    	field[2].Value.AllocatedSize = 4;
    	field[2].Value.Type = TBL_SIGNED4;
    	field[2].Value.Value.Signed4 = &ImagesDeleted;

    	field[3].FieldName = 0;

    	ret_val = TBL_Select(&stuhandle, criteria, field, &count, NULL, NULL);
    	if (ret_val != TBL_NORMAL || count != 1) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADSTUUID), uid, "IDB_Delete");
    	}
    	if (LST_Enqueue(&GS_stus, temp_node) != LST_NORMAL) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    	}
	/*
	 * Must update the statistics of the patient node here... Pulling an
	 * error here is difficult...since statistics will be updated before
	 * the node is actually deleted...we will probably do a transaction
	 * here (eventually).
	 */
    	criteria[0].FieldName = "PatID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = patientuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumPatRelStu";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = "NumPatRelSer";
    	update[1].Function = TBL_SUBTRACT;
    	update[1].Value.AllocatedSize = 4;
    	update[1].Value.Type = TBL_SIGNED4;
    	update[1].Value.Value.Signed4 = &SeriesDeleted;

    	update[2].FieldName = "NumPatRelIma";
    	update[2].Function = TBL_SUBTRACT;
    	update[2].Value.AllocatedSize = 4;
    	update[2].Value.Type = TBL_SIGNED4;
    	update[2].Value.Value.Signed4 = &ImagesDeleted;

    	update[3].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&pathandle, criteria, update);
    	}else{
    		localDecrement(&pathandle, criteria, update);
    	}
    }else if (level == IDB_SERIES_LEVEL){
    	criteria[0].FieldName = "SerInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = uid;

    	criteria[1].FieldName = 0;

    	field[0].FieldName = "StuParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = studyuid;

    	field[1].FieldName = "NumSerRelIma";
    	field[1].Value.AllocatedSize = 4;
    	field[1].Value.Type = TBL_SIGNED4;
    	field[1].Value.Value.Signed4 = &ImagesDeleted;

    	field[2].FieldName = 0;

    	ret_val = TBL_Select(&serhandle, criteria, field, &count, NULL, NULL);
    	if (ret_val != TBL_NORMAL || count != 1) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    	}
    	if (LST_Enqueue(&GS_sers, temp_node) != LST_NORMAL) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    	}
    	/* Must update the statistics of the patient and study nodes here... */

    	criteria[0].FieldName = "StuInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = studyuid;

    	criteria[1].FieldName = 0;

    	field[0].FieldName = "PatParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = patientuid;

    	field[1].FieldName = 0;

    	TBL_Select(&stuhandle, criteria, field, &count, NULL, NULL);

    	criteria[0].FieldName = "StuInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = studyuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumStuRelSer";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = "NumStuRelIma";
    	update[1].Function = TBL_SUBTRACT;
    	update[1].Value.AllocatedSize = 4;
    	update[1].Value.Type = TBL_SIGNED4;
    	update[1].Value.Value.Signed4 = &ImagesDeleted;

    	update[2].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&stuhandle, criteria, update);
    	}else{
    		localDecrement(&stuhandle, criteria, update);
    	}

    	criteria[0].FieldName = "PatID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = patientuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumPatRelSer";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = "NumPatRelIma";
    	update[1].Function = TBL_SUBTRACT;
    	update[1].Value.AllocatedSize = 4;
    	update[1].Value.Type = TBL_SIGNED4;
    	update[1].Value.Value.Signed4 = &ImagesDeleted;

    	update[2].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&pathandle, criteria, update);
    	}else{
    		localDecrement(&pathandle, criteria, update);
    	}

    }else if (level == IDB_IMAGE_LEVEL){

    	criteria[0].FieldName = "SOPInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = uid;

    	criteria[1].FieldName = 0;

    	field[0].FieldName = "SerParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = seriesuid;

    	field[1].FieldName = 0;

    	ret_val = TBL_Select(&imahandle, criteria, field, &count, NULL, NULL);
    	if (ret_val != TBL_NORMAL || count != 1){
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADSERUID), uid, "IDB_Delete");
    	}
    	if (LST_Enqueue(&GS_imas, temp_node) != LST_NORMAL) {
    		LST_Destroy(&GS_pats);
    		LST_Destroy(&GS_stus);
    		LST_Destroy(&GS_sers);
    		LST_Destroy(&GS_imas);
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    	}
    	/* Must update the statistics of the patient, study, and series nodes here... */

    	criteria[0].FieldName = "SerInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = seriesuid;

    	criteria[1].FieldName = 0;

    	field[0].FieldName = "StuParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = studyuid;

    	field[1].FieldName = 0;

    	TBL_Select(&serhandle, criteria, field, &count, NULL, NULL);

    	criteria[0].FieldName = "StuInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = studyuid;

    	criteria[1].FieldName = 0;

    	field[0].FieldName = "PatParent";
    	field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = patientuid;

    	field[1].FieldName = 0;

    	TBL_Select(&stuhandle, criteria, field, &count, NULL, NULL);

    	criteria[0].FieldName = "SerInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = seriesuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumSerRelIma";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&serhandle, criteria, update);
    	}else{
    		localDecrement(&serhandle, criteria, update);
    	}

    	criteria[0].FieldName = "StuInsUID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = studyuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumStuRelIma";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&stuhandle, criteria, update);
    	}else{
    		localDecrement(&stuhandle, criteria, update);
    	}

    	criteria[0].FieldName = "PatID";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = patientuid;
    	criteria[1].FieldName = 0;

    	update[0].FieldName = "NumPatRelIma";
    	update[0].Function = TBL_DECREMENT;

    	update[1].FieldName = 0;

    	if (TBL_HasUpdateIncrement()){
    		TBL_Update(&pathandle, criteria, update);
    	}else{
    		localDecrement(&pathandle, criteria, update);
    	}
    }
    /*
     * Now we have initial seed for the nodes we need to delete...now all we
     * need to do is find the decendents.  We are going to make lists of uids
     * here and then just delete all the nodes with the corresponding uid.
     */

    if (LST_Count(&GS_pats) != 0) {	/* We have a patient uid...find the studies */
    	temp_node = LST_Head(&GS_pats);
    	temp_string = (char *) temp_node + (2 * sizeof(void *));

    	field[0].FieldName = "StuInsUID";
    	field[0].Value.AllocatedSize = DICOM_LO_LENGTH + 1;
    	field[0].Value.Type = TBL_STRING;
    	field[0].Value.Value.String = buf;
    	field[1].FieldName = 0;

    	criteria[0].FieldName = "PatParent";
    	criteria[0].Operator = TBL_EQUAL;
    	criteria[0].Value.Type = TBL_STRING;
    	criteria[0].Value.Value.String = temp_string;
    	criteria[1].FieldName = 0;

    	ret_val = TBL_Select(&stuhandle, criteria, field, NULL, CBDel_CollectStudies, NULL);

    	if (ret_val != TBL_NORMAL) {
    		IDB_DestroyGlobalLists();
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    	}
    }
    if (LST_Count(&GS_stus) != 0){
    	temp_node = LST_Head(&GS_stus);
    	(void) LST_Position(&GS_stus, temp_node);

    	do {
    		temp_string = (char *) temp_node + (2 * sizeof(void *));
    		field[0].FieldName = "SerInsUID";
    		field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    		field[0].Value.Type = TBL_STRING;
    		field[0].Value.Value.String = buf;
    		field[1].FieldName = 0;

    		criteria[0].FieldName = "StuParent";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_string;
    		criteria[1].FieldName = 0;

    		ret_val = TBL_Select(&serhandle, criteria, field, NULL, CBDel_CollectSeries, NULL);
    		if (ret_val != TBL_NORMAL) {
    			IDB_DestroyGlobalLists();
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    		}
    	} while ((temp_node = LST_Next(&GS_stus)) != NULL);
    }

    if (LST_Count(&GS_sers) != 0){
    	temp_node = LST_Head(&GS_sers);
    	(void) LST_Position(&GS_sers, temp_node);
    	do {
    		temp_string = (char *) temp_node + (2 * sizeof(void *));

    		field[0].FieldName = "SOPInsUID";
    		field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    		field[0].Value.Type = TBL_STRING;
    		field[0].Value.Value.String = buf;
    		field[1].FieldName = 0;

    		criteria[0].FieldName = "SerParent";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_string;
    		criteria[1].FieldName = 0;

    		ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBDel_CollectImages, NULL);
    		if (ret_val != TBL_NORMAL){
    			IDB_DestroyGlobalLists();
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_BADLISTENQ), "IDB_Delete");
    		}
    	} while ((temp_node = LST_Next(&GS_sers)) != NULL);
    }
    /*
     * Now we have lists with all the node to be deleted...now we just delete them. */
    PatientsDeleted = StudiesDeleted = SeriesDeleted = ImagesDeleted = 0;
    if (LST_Count(&GS_pats) != 0) {
    	temp_node = LST_Dequeue(&GS_pats);
    	while (temp_node != (LST_NODE *) NULL) {

    		criteria[0].FieldName = "PatID";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_node + (2 * sizeof(void *));
    		criteria[1].FieldName = 0;

    		TBL_Delete(&pathandle, criteria);
    		PatientsDeleted++;
    		free(temp_node);
    		temp_node = LST_Dequeue(&GS_pats);
    	}
    }
    if (LST_Count(&GS_stus) != 0) {
    	temp_node = LST_Dequeue(&GS_stus);
    	while (temp_node != (LST_NODE *) NULL) {

    		criteria[0].FieldName = "StuInsUID";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_node + (2 * sizeof(void *));
    		criteria[1].FieldName = 0;

    		TBL_Delete(&stuhandle, criteria);
    		StudiesDeleted++;
    		free(temp_node);
    		temp_node = LST_Dequeue(&GS_stus);
    	}
    }
    if (LST_Count(&GS_sers) != 0) {
    	temp_node = LST_Dequeue(&GS_sers);
    	while (temp_node != (LST_NODE *) NULL) {

    		criteria[0].FieldName = "SerInsUID";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_node + (2 * sizeof(void *));
    		criteria[1].FieldName = 0;

    		TBL_Delete(&serhandle, criteria);
    		SeriesDeleted++;
    		free(temp_node);
    		temp_node = LST_Dequeue(&GS_sers);
    	}
    }
    if (LST_Count(&GS_imas) != 0) {
    	temp_node = LST_Dequeue(&GS_imas);
    	while (temp_node != (LST_NODE *) NULL) {
    		criteria[0].FieldName = "SOPInsUID";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_node + (2 * sizeof(void *));
    		criteria[1].FieldName = 0;
    		TBL_Delete(&imahandle, criteria);
    		/* And while we have the Image UID...use it to delete the Instances as well */

    		criteria[0].FieldName = "ImageUID";
    		criteria[0].Operator = TBL_EQUAL;
    		criteria[0].Value.Type = TBL_STRING;
    		criteria[0].Value.Value.String = (char *) temp_node + (2 * sizeof(void *));
    		criteria[1].FieldName = 0;

    		if (flag) {
    			if (deleteImageFiles(&inshandle, criteria, &storageSpaceReleased) != IDB_NORMAL) {
    				THR_ReleaseMutex(FAC_IDB);
    				return COND_PushCondition(IDB_ERROR(IDB_DELETEFAILED), "IDB_Delete");
    			}
    		}
    		TBL_Delete(&inshandle, criteria);

    		ImagesDeleted++;
    		free(temp_node);
    		temp_node = LST_Dequeue(&GS_imas);
    	}
    }
    IDB_DestroyGlobalLists();

#ifdef CTN_IDBV2
    updateLimitsTable(limitsHandle,
		      -storageSpaceReleased,
		      -PatientsDeleted,
		      -StudiesDeleted,
		      -ImagesDeleted);
#endif

    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;
}

static CONDITION
deleteInstanceFileCallback(TBL_FIELD * fp, int count, void *ctx)
{

    long 		*bytesFreed;

    bytesFreed = (long *) ctx;

    while (fp->FieldName != 0) {
    	if (strcmp(fp->FieldName, "Path") == 0){
    		if (unlink(fp->Value.Value.String) != 0)
    			return COND_PushCondition(IDB_FILEDELETEFAILED, IDB_Message(IDB_FILEDELETEFAILED), fp->Value.Value.String, "(IDB)deleteInstanceFileCallback");
    	}else if (strcmp(fp->FieldName, "Size") == 0){
    		*bytesFreed += (*(fp->Value.Value.Signed4)) / SCALING_FACTOR;
    	}
    	fp++;
    }
    return TBL_NORMAL;
}

static CONDITION
deleteImageFiles(TBL_HANDLE ** handle, TBL_CRITERIA * criteria, long *bytesFreed)
{
    CONDITION 		cond;

    cond = TBL_Select(handle, criteria, GS_INSSEL_Field, NULL, deleteInstanceFileCallback, bytesFreed);
    if (cond != TBL_NORMAL)	return 0;

    return IDB_NORMAL;
}

/*
 * Destroy the linked lists of uids we gathered...
 */
void
IDB_DestroyGlobalLists()
{

    LST_NODE	    * foo;

    while (LST_Count(&GS_pats) != 0){
    	foo = LST_Dequeue(&GS_pats);
    	free(foo);
    }
    LST_Destroy(&GS_pats);

    while (LST_Count(&GS_stus) != 0){
    	foo = LST_Dequeue(&GS_stus);
    	free(foo);
    }
    LST_Destroy(&GS_stus);

    while (LST_Count(&GS_sers) != 0) {
    	foo = LST_Dequeue(&GS_sers);
    	free(foo);
    }
    LST_Destroy(&GS_sers);

    while (LST_Count(&GS_imas) != 0) {
    	foo = LST_Dequeue(&GS_imas);
    	free(foo);
    }
    LST_Destroy(&GS_imas);

    return;
}
/*
 * Callback routines for the Delete function...
 */

CONDITION
CBDel_CollectImages(TBL_FIELD * field, int count, void *ctx)
{
    LST_NODE		* temp_node;
    char	        *temp_string;

    temp_node = (LST_NODE *) malloc((2 * sizeof(void *)) + strlen(field->Value.Value.String) + 1);
    if (temp_node == (LST_NODE *) NULL) return ~TBL_NORMAL;

    temp_string = (char *) temp_node + (2 * sizeof(void *));
    strcpy(temp_string, field->Value.Value.String);
    LST_Enqueue(&GS_imas, temp_node);
    return TBL_NORMAL;
}

CONDITION
CBDel_CollectSeries(TBL_FIELD * field, int count, void *ctx)
{
    LST_NODE	* temp_node;
    char	    *temp_string;

    temp_node = (LST_NODE *) malloc((2 * sizeof(void *)) + strlen(field->Value.Value.String) + 1);
    if (temp_node == (LST_NODE *) NULL) return ~TBL_NORMAL;

    temp_string = (char *) temp_node + (2 * sizeof(void *));
    strcpy(temp_string, field->Value.Value.String);
    LST_Enqueue(&GS_sers, temp_node);
    return TBL_NORMAL;
}

CONDITION
CBDel_CollectStudies(TBL_FIELD * field, int count, void *ctx)
{
    LST_NODE	* temp_node;
    char        *temp_string;

    temp_node = (LST_NODE *) malloc((2 * sizeof(void *)) + strlen(field->Value.Value.String) + 1);
    if (temp_node == (LST_NODE *) NULL)	return ~TBL_NORMAL;

    temp_string = (char *) temp_node + (2 * sizeof(void *));
    strcpy(temp_string, field->Value.Value.String);
    LST_Enqueue(&GS_stus, temp_node);
    return TBL_NORMAL;
}


/* IDB_Select
**
** Purpose:
**	This routine selects records from the database and uses
**	the Dicom matching specifications.
**
** Parameter Dictionary:
**	IDB_HANDLE **handle:
**		the database identifier.
**	IDB_QUERY_MODEL model:
**		The Dicom model for this request.
**	long begin_level:
**	long end_level:
**		The levels in the heirarchy specifying where the
**		search for records will begin and end.
**		begin_level and end_level must be one of the
**		predefined constants:
**		IDB_PATIENT_LEVEL, IDB_STUDY_LEVEL, IDB_SERIES_LEVEL,
**		or IDB_IMAGE_LEVEL.
**	IDB_Query *pssi:
**		The structure that contains the specifications for
**		record retrieval on a per-node basis.
**	long *count:
**		This parameter will contains the number of records matched
**		upon return.
**	CONDITION (*callback()):
**		The callback function invoked when a matching record is
**		found.  It is invoked as follows:
**			callback(IDB_Query *pssi,long count, void *ctx);
**      void *ctx: Ancillary data passed through to the callback function
**              and untouched by this routine.
**
** Return Values:
**	IDB_NORMAL: 	The close command succeeded
**	IDB_BADHANDLE:	The handle passed is invalid
**	IDB_BADLEVEL:	The level passed is invalid
**	IDB_BADLEVELSEQ: The level sequence passed is invalid, for example,
**		the begin_level passed is Study, and the end_level passed
**		is Patient.
**	IDB_NOMATCHES:	No matches were found for the query in question.
**	IDB_EARLYEXIT:	The callback routine provided by the user returned
**		something other that TBL_NORMAL and forced this routine
**		to exit early.
**	IDB_NOMEMORY:	No memory is available to build the temporary lists
**		needed...this is very bad.
**
** Algorithm:
**      As each record is retreived from the
**      database, the fields requested by the user (contained in
**      pssi), are filled with the informatiton retreived from
**      the database and a pointer to the list is passed to the
**      callback routine designated by the input parameter callback.
**      The callback routine is invoked as follows:
**
**              callback(IDB_Query *pssi, long count, void *ctx)
**
**      The count contains the number of records retreived to this point.
**      ctx contains any additional information the user originally passed
**      to this select function.  If callback returns any value other
**      than IDB_NORMAL, it is assumed that this function should terminate
**      (i.e. cancel the current db operation), and return an abnormal
**      termination message (IDB_EARLYEXIT) to the routine which
**      originally invoked the select.
**
** Notes:
**	This routine contains the use of a "go to" to implement the
**	structured construct known as a multi-level break statement.  'c'
**	has a single level break statement in the language but no facility
**	to implement a multi-level break statement.  This algorithm could
**	well have been implemented without using the actual "go to", but
**	the resulting code would have been more difficult to read and maintain
**	in my opinion.  I am not fond of using "go to"'s, and rarely ever
**	do it, but I do find that every 100 thousand lines or so that the
**	need surfaces...
**
** *New* Notes:
**	In addition, this routine is now simply placeholder to determine
**	whether or not we are dealing with the STUDY_ROOT Dicom Query Model.
**	If so, then we will utilize the routine IDB_Select_View, which will
**	perform the initial select on a view to get better response times
**	from the database.  If not, then we will use the old, original routine
**	IDB_Select_NoView.
**
*/

CONDITION
IDB_Select(IDB_HANDLE ** handle, IDB_QUERY_MODEL model, long begin_level, long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx)
{
    if ((model == STUDY_ROOT) && (TBL_HasViews())){
    	return IDB_Select_View(handle, begin_level, end_level, pssi, count, callback, ctx);
    }else{
    	return IDB_Select_NoView(handle, begin_level, end_level, pssi, count, callback, ctx);
    }
}

CONDITION
IDB_Select_View(IDB_HANDLE ** handle, long begin_level,	long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx)
{
    IDB_Query					ret_query;
    TBL_FIELD					* field;
    TBL_CRITERIA				inscrit[2],	*criteria;
    TBL_HANDLE					* pathandle, *stuhandle, *patstuhandle, *serhandle,	*imahandle,	*inshandle;
    IDB_CONTEXT					* idbc;
    long				        i, ipat, istu, iser = 0, iima, tempcount, numpatcrits, numstucrits, numpatstucrits, numsercrits,  numimacrits, foundit;
    void				        *temp;
    CONDITION					ret_val = 0;

    THR_ObtainMutex(FAC_IDB);

    /* Locate the handle in the open table...  */
    idbc = GS_ContextHead;
    foundit = 0;

    while (idbc != (IDB_CONTEXT *) NULL){
    	if (idbc == (IDB_CONTEXT *) (*handle)){
    		pathandle 	 = idbc->PatNodes;
    		stuhandle 	 = idbc->StuNodes;
    		patstuhandle = idbc->PatStuNodes;
    		serhandle 	 = idbc->SerNodes;
    		imahandle 	 = idbc->ImaNodes;
    		inshandle 	 = idbc->InsNodes;
    		foundit 	 = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_Select");
    }
    if (begin_level != IDB_PATIENT_LEVEL && begin_level != IDB_STUDY_LEVEL && begin_level != IDB_SERIES_LEVEL && begin_level != IDB_IMAGE_LEVEL &&
					   end_level != IDB_PATIENT_LEVEL && end_level != IDB_STUDY_LEVEL && end_level != IDB_SERIES_LEVEL && end_level != IDB_IMAGE_LEVEL){
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVEL), "IDB_Select");
    }
    if (begin_level == IDB_STUDY_LEVEL && end_level == IDB_PATIENT_LEVEL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }
    if (begin_level == IDB_SERIES_LEVEL && (end_level == IDB_PATIENT_LEVEL || end_level == IDB_STUDY_LEVEL)) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }
    if (begin_level == IDB_IMAGE_LEVEL && (end_level == IDB_PATIENT_LEVEL || end_level == IDB_STUDY_LEVEL || end_level == IDB_SERIES_LEVEL)){
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }

    numpatstucrits = numpatcrits = numstucrits = numsercrits = numimacrits = 0;
    GS_PatStuNodes = (IDB_PatientQuery *) NULL;
    GS_PatNodes	   = (IDB_PatientQuery *) NULL;
    GS_StuNodes    = (IDB_StudyQuery *) NULL;
    GS_NullPatFlag = (long *) NULL;
    GS_NullStuFlag = (long *) NULL;
    GS_NumPatNodes = 0;
    GS_NumStuNodes = 0;

    if (begin_level == IDB_PATIENT_LEVEL || begin_level == IDB_STUDY_LEVEL){
    	criteria = GS_patstucl;
    	if ((numpatstucrits = IDB_ConvertDicomQuerytoSQL_PSView(pssi)) == 0) criteria = (TBL_CRITERIA *) NULL;
    	field = GS_PATSTUSEL_Field;
    	ret_val = TBL_Select(&patstuhandle, criteria, field, NULL, CBSel_CollectPatientsStudies, NULL);
    	if (GS_NumPatNodes == 0 || GS_NumStuNodes == 0 || ret_val != TBL_NORMAL) {

    		if (!GS_NumPatNodes || !GS_NumStuNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}

    		FREE_STRINGS(i, GS_patstucl, numpatstucrits);
    		if (GS_NumPatNodes)	free(GS_PatNodes);
    		if (GS_NullPatFlag)	free(GS_NullPatFlag);
    		if (GS_NumStuNodes)	free(GS_StuNodes);
    		if (GS_NullStuFlag)	free(GS_NullStuFlag);
    	}
    }
    GS_SerNodes = (IDB_SeriesQuery *) NULL;
    GS_NumSerNodes = 0;
    GS_NullSerFlag = (long *) NULL;

    if (begin_level == IDB_SERIES_LEVEL || (begin_level == IDB_PATIENT_LEVEL && end_level != IDB_PATIENT_LEVEL && end_level != IDB_STUDY_LEVEL) ||
					   (begin_level == IDB_STUDY_LEVEL && end_level != IDB_STUDY_LEVEL)){
    	criteria = GS_sercl;
    	field = GS_SERSEL_Field;

    	if ((numsercrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_SERIES_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;

    	if (GS_NumStuNodes != 0){
    		criteria = GS_sercl;
    		criteria[numsercrits].FieldName = "StuParent";
    		criteria[numsercrits].Value.Type = TBL_STRING;
    		criteria[numsercrits].Value.IsNull = 0;
    		criteria[numsercrits].Operator = TBL_EQUAL;
    		criteria[numsercrits + 1].FieldName = 0;

    		temp = (char *) NULL;

    		for (i = 0; i < GS_NumStuNodes; i++){ /* Get study UID and add to criteria list at numitems... */
    			if (temp != (char *) NULL) free(temp);

    			if ((temp = (char *) malloc(strlen(GS_StuNodes[i].StuInsUID) + 1)) == (char *) NULL) {
    				ret_val = ~TBL_NORMAL;
    				break;
    			}
    			criteria[numsercrits].Value.Value.String = temp;
    			strcpy(criteria[numsercrits].Value.Value.String, GS_StuNodes[i].StuInsUID);
    			criteria[numsercrits].Value.AllocatedSize = strlen(GS_StuNodes[i].StuInsUID);
    			criteria[numsercrits].Value.Size = strlen(GS_StuNodes[i].StuInsUID);
    			ret_val = TBL_Select(&serhandle, criteria, field, NULL, CBSel_CollectSeries, NULL);
    		}
    		numsercrits++;
    	}else{
    		ret_val = TBL_Select(&serhandle, criteria, field, NULL, CBSel_CollectSeries, NULL);
    	}

    	if (GS_NumSerNodes == 0 || ret_val != TBL_NORMAL) {
    		FREE_STRINGS(i, GS_sercl, numsercrits);
    		if (GS_NumPatNodes) {
    			FREE_STRINGS(i, GS_patcl, numpatcrits);
    			free(GS_PatNodes);
    			free(GS_NullPatFlag);
    		}
    		if (GS_NumStuNodes) {
    			FREE_STRINGS(i, GS_stucl, numstucrits);
    			free(GS_StuNodes);
    			free(GS_NullStuFlag);
    		}
    		if (GS_NumSerNodes) {
    			free(GS_SerNodes);
    			free(GS_NullSerFlag);
    		}
    		if (!GS_NumSerNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    }

    GS_ImaNodes = (IDB_ImageQuery *) NULL;
    GS_NumImaNodes = 0;
    GS_NullImaFlag = (long *) NULL;

    if (begin_level == IDB_IMAGE_LEVEL || end_level == IDB_IMAGE_LEVEL){
    	criteria = GS_imacl;
    	field = GS_IMASEL_Field;
    	if ((numimacrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_IMAGE_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;

    	if (GS_NumSerNodes != 0) {
    		criteria = GS_imacl;
    		criteria[numimacrits].FieldName = "SerParent";
    		criteria[numimacrits].Value.Type = TBL_STRING;
    		criteria[numimacrits].Value.IsNull = 0;
    		criteria[numimacrits].Operator = TBL_EQUAL;
    		criteria[numimacrits + 1].FieldName = 0;

    		temp = (char *) NULL;

    		for (i = 0; i < GS_NumSerNodes; i++) {/* Get series UID and add to criteria list at numitems... */
    			if (temp != (char *) NULL) free(temp);

    			if ((temp = (char *) malloc(strlen(GS_SerNodes[i].SerInsUID) + 1)) == (char *) NULL) {
    				ret_val = ~TBL_NORMAL;
    				break;
    			}
    			criteria[numimacrits].Value.Value.String = temp;
    			strcpy(criteria[numimacrits].Value.Value.String, GS_SerNodes[i].SerInsUID);
    			criteria[numimacrits].Value.AllocatedSize = strlen(GS_SerNodes[i].SerInsUID);
    			criteria[numimacrits].Value.Size = strlen(GS_SerNodes[i].SerInsUID);
    			ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
    		}
    		numimacrits++;
    	}else{
    		ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
    	}

    	if (GS_NumImaNodes == 0 || ret_val != TBL_NORMAL) {
    		FREE_STRINGS(i, GS_imacl, numimacrits);
    		if (GS_NumPatNodes) {
    			FREE_STRINGS(i, GS_patcl, numpatcrits);
    			free(GS_PatNodes);
    			free(GS_NullPatFlag);
    		}
    		if (GS_NumStuNodes) {
    			FREE_STRINGS(i, GS_stucl, numstucrits);
    			free(GS_StuNodes);
    			free(GS_NullStuFlag);
    		}
    		if (GS_NumSerNodes) {
    			FREE_STRINGS(i, GS_sercl, numsercrits);
    			free(GS_SerNodes);
    			free(GS_NullSerFlag);
    		}
    		if (GS_NumImaNodes) {
    			free(GS_ImaNodes);
    			free(GS_NullImaFlag);
    		}
    		if (!GS_NumImaNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    	/* Now go and get the instances associated with each image node. Cycle through all the GS_ImaNodes and fill up each with an
    	 * instances list. GS_ImaNodes[i].InstanceList...and allocate a bunch of Instance List Elements.
    	 *
    	 * Pass along the head pointer as the void * argument. */

    	inscrit[1].FieldName = 0;

    	for (i = 0; i < GS_NumImaNodes; i++){
    		GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
    		TBL_CRITERIA_LOAD_BYTE(inscrit[0], "ImageUID", GS_ImaNodes[i].SOPInsUID, TBL_STRING, TBL_EQUAL);
    		field = GS_INSSEL_Field;
    		ret_val = TBL_Select(&inshandle, inscrit, field, NULL, CBSel_CollectInstances, (void *) &(GS_ImaNodes[i].InstanceList));
    		if (ret_val != TBL_NORMAL) GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
    	}
    }

    if (numpatcrits) FREE_STRINGS(i, GS_patcl, numpatcrits);
    if (numstucrits) FREE_STRINGS(i, GS_stucl, numstucrits);
    if (numsercrits) FREE_STRINGS(i, GS_sercl, numsercrits);
    if (numimacrits) FREE_STRINGS(i, GS_imacl, numimacrits);

    /*
     * Now we have lists of all the nodes of interest that match the criteria
     * finished...now cycle through them and return the correct information
     * via the users callback routine....
     * 
     * This is the section that uses the dreaded "goto"....ich.
     */
    memset(&ret_query, 0, sizeof(ret_query));
    ret_query.PatientQFlag = ret_query.StudyQFlag = ret_query.SeriesQFlag = ret_query.ImageQFlag = 0;
    tempcount = 0;

    if (GS_NumPatNodes != 0){
    	for (ipat = 0, istu = 0; ipat < GS_NumPatNodes; ipat++, istu++){
    		ret_query.patient = GS_PatNodes[ipat];
    		ret_query.PatientQFlag = pssi->PatientQFlag;
    		ret_query.PatientNullFlag = GS_NullPatFlag[ipat];
    		ret_query.study = GS_StuNodes[istu];
    		ret_query.StudyQFlag = pssi->StudyQFlag;
    		ret_query.StudyNullFlag = GS_NullStuFlag[istu];

    		if (GS_NumSerNodes != 0) {
    			for (iser = 0; iser < GS_NumSerNodes; iser++) {
    				if (strcmp(GS_StuNodes[istu].StuInsUID, GS_SerNodes[iser].StuParent) == 0) {
    					ret_query.series = GS_SerNodes[iser];
    					ret_query.SeriesQFlag = pssi->SeriesQFlag;
    					ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    					if (GS_NumImaNodes != 0) {
    						for (iima = 0; iima < GS_NumImaNodes; iima++) {
    							if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    								ret_query.image = GS_ImaNodes[iima];
    								ret_query.ImageQFlag = pssi->ImageQFlag;
    								ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    								tempcount++;
    								if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)	goto EarlyExit;
    							}
    						}
    					}else{
    						tempcount++;
    						if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    					}
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }else if (GS_NumStuNodes != 0){
    	for (istu = 0; istu < GS_NumStuNodes; istu++) {
    		ret_query.study = GS_StuNodes[istu];
    		ret_query.StudyQFlag = pssi->StudyQFlag;
    		ret_query.StudyNullFlag = GS_NullStuFlag[istu];
    		if (GS_NumSerNodes != 0) {
    			for (iser = 0; iser < GS_NumSerNodes; iser++) {
    				if (strcmp(GS_StuNodes[istu].StuInsUID, GS_SerNodes[iser].StuParent) == 0) {
    					ret_query.series = GS_SerNodes[iser];
    					ret_query.SeriesQFlag = pssi->SeriesQFlag;
    					ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    					if (GS_NumImaNodes != 0) {
    						for (iima = 0; iima < GS_NumImaNodes; iima++) {
    							if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    								ret_query.image = GS_ImaNodes[iima];
    								ret_query.ImageQFlag = pssi->ImageQFlag;
    								ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    								tempcount++;
    								if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)	goto EarlyExit;
    							}
    						}
    					}else{
    						tempcount++;
    						if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)	goto EarlyExit;
    					}
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }else if (GS_NumSerNodes != 0){
    	for (iser = 0; iser < GS_NumSerNodes; iser++) {
    		ret_query.series = GS_SerNodes[iser];
    		ret_query.SeriesQFlag = pssi->SeriesQFlag;
    		ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    		if (GS_NumImaNodes != 0) {
    			for (iima = 0; iima < GS_NumImaNodes; iima++) {
    				if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    					ret_query.image = GS_ImaNodes[iima];
    					ret_query.ImageQFlag = pssi->ImageQFlag;
    					ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    					tempcount++;
    					if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }else if (GS_NumImaNodes != 0){
    	for (iima = 0; iima < GS_NumImaNodes; iima++) {
    		if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    			ret_query.image = GS_ImaNodes[iima];
    			ret_query.ImageQFlag = pssi->ImageQFlag;
    			ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }
    if (count != NULL) *count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement	    * insnode;

    	for (i = 0; i < GS_NumImaNodes; i++) {
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;

EarlyExit:

    if (count != NULL) *count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement	    * insnode;

    	for (i = 0; i < GS_NumImaNodes; i++) {
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return COND_PushCondition(IDB_ERROR(IDB_EARLYEXIT), "IDB_Select");
}

CONDITION
IDB_Select_NoView(IDB_HANDLE ** handle, long begin_level, long end_level, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx)
{
    IDB_Query					ret_query;
    TBL_FIELD					* field;
    TBL_CRITERIA				inscrit[2], *criteria;
    TBL_HANDLE					* pathandle, *stuhandle, *serhandle, *imahandle, *inshandle;
    IDB_CONTEXT					* idbc;
    long				        i, ipat, istu, iser = 0, iima, tempcount, numpatcrits, numstucrits, numsercrits, numimacrits, foundit;
    void				       *temp;
    CONDITION					ret_val;

    THR_ObtainMutex(FAC_IDB);
    THR_ReleaseMutex(FAC_IDB);
    /*
     * Locate the handle in the open table...
     */
    idbc = GS_ContextHead;
    foundit = 0;

    while (idbc != (IDB_CONTEXT *) NULL) {
    	if (idbc == (IDB_CONTEXT *) (*handle)) {
    		pathandle = idbc->PatNodes;
    		stuhandle = idbc->StuNodes;
    		serhandle = idbc->SerNodes;
    		imahandle = idbc->ImaNodes;
    		inshandle = idbc->InsNodes;
    		foundit = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_Select");
    }
    if (begin_level != IDB_PATIENT_LEVEL && begin_level != IDB_STUDY_LEVEL && begin_level != IDB_SERIES_LEVEL && begin_level != IDB_IMAGE_LEVEL &&
					   end_level != IDB_PATIENT_LEVEL && end_level != IDB_STUDY_LEVEL && end_level != IDB_SERIES_LEVEL && end_level != IDB_IMAGE_LEVEL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVEL), "IDB_Select");
    }
    if (begin_level == IDB_STUDY_LEVEL && end_level == IDB_PATIENT_LEVEL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }
    if (begin_level == IDB_SERIES_LEVEL && (end_level == IDB_PATIENT_LEVEL || end_level == IDB_STUDY_LEVEL)) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }
    if (begin_level == IDB_IMAGE_LEVEL && (end_level == IDB_PATIENT_LEVEL || end_level == IDB_STUDY_LEVEL || end_level == IDB_SERIES_LEVEL)) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADLEVELSEQ), "IDB_Select");
    }

    numpatcrits = numstucrits = numsercrits = numimacrits = 0;
    GS_PatNodes = (IDB_PatientQuery *) NULL;
    GS_NullPatFlag = (long *) NULL;
    GS_NumPatNodes = 0;

    if (begin_level == IDB_PATIENT_LEVEL) {
    	criteria = GS_patcl;
    	if ((numpatcrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_PATIENT_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;
    	field = GS_PATSEL_Field;

    	ret_val = TBL_Select(&pathandle, criteria, field, NULL, CBSel_CollectPatients, NULL);
    	if (GS_NumPatNodes == 0 || ret_val != TBL_NORMAL) {
    		FREE_STRINGS(i, GS_patcl, numpatcrits);
    		if (GS_NumPatNodes)	free(GS_PatNodes);
    		if (GS_NullPatFlag)	free(GS_NullPatFlag);
    		if (!GS_NumPatNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    }
    GS_StuNodes = (IDB_StudyQuery *) NULL;
    GS_NumStuNodes = 0;
    GS_NullStuFlag = (long *) NULL;

    if (begin_level == IDB_STUDY_LEVEL || (begin_level == IDB_PATIENT_LEVEL && end_level != IDB_PATIENT_LEVEL)) {
    	criteria = GS_stucl;
    	field = GS_STUSEL_Field;
    	if ((numstucrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_STUDY_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;

    	if (GS_NumPatNodes != 0) {
    		criteria = GS_stucl;
    		criteria[numstucrits].FieldName = "PatParent";
    		criteria[numstucrits].Value.Type = TBL_STRING;
    		criteria[numstucrits].Value.IsNull = 0;
    		criteria[numstucrits].Operator = TBL_EQUAL;
    		criteria[numstucrits + 1].FieldName = 0;
    		temp = (char *) NULL;

    		for (i = 0; i < GS_NumPatNodes; i++) {
    			/* Get patient UID and add to criteria list at numitems... */
    			if (temp != (char *) NULL) free(temp);
				if ((temp = (char *) malloc(strlen(GS_PatNodes[i].PatID) + 1)) == (char *) NULL) {
					ret_val = ~TBL_NORMAL;
					break;
				}
				criteria[numstucrits].Value.Value.String = temp;
				strcpy(criteria[numstucrits].Value.Value.String, GS_PatNodes[i].PatID);
				criteria[numstucrits].Value.AllocatedSize = strlen(GS_PatNodes[i].PatID);
				criteria[numstucrits].Value.Size = strlen(GS_PatNodes[i].PatID);
				ret_val = TBL_Select(&stuhandle, criteria, field, NULL, CBSel_CollectStudies, NULL);
    		}
    		numstucrits++;
    	}else{
    		ret_val = TBL_Select(&stuhandle, criteria, field, NULL, CBSel_CollectStudies, NULL);
    	}

    	if ((GS_NumStuNodes == 0) || (ret_val != TBL_NORMAL)) {
    		FREE_STRINGS(i, GS_stucl, numstucrits);
    		if (GS_NumPatNodes) {
    			FREE_STRINGS(i, GS_patcl, numpatcrits);
    			free(GS_PatNodes);
    			free(GS_NullPatFlag);
    		}
    		if (GS_NumStuNodes) {
    			free(GS_StuNodes);
    			free(GS_NullStuFlag);
    		}
    		if (!GS_NumStuNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    }
    GS_SerNodes = (IDB_SeriesQuery *) NULL;
    GS_NumSerNodes = 0;
    GS_NullSerFlag = (long *) NULL;

    if (begin_level == IDB_SERIES_LEVEL || (begin_level == IDB_PATIENT_LEVEL && end_level != IDB_PATIENT_LEVEL && end_level != IDB_STUDY_LEVEL) ||
					   (begin_level == IDB_STUDY_LEVEL && end_level != IDB_STUDY_LEVEL)) {
    	criteria = GS_sercl;
    	field = GS_SERSEL_Field;
    	if ((numsercrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_SERIES_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;

    	if (GS_NumStuNodes != 0) {
    		criteria = GS_sercl;
    		criteria[numsercrits].FieldName = "StuParent";
    		criteria[numsercrits].Value.Type = TBL_STRING;
    		criteria[numsercrits].Value.IsNull = 0;
    		criteria[numsercrits].Operator = TBL_EQUAL;
    		criteria[numsercrits + 1].FieldName = 0;

    		temp = (char *) NULL;

    		for (i = 0; i < GS_NumStuNodes; i++) {
    			/* Get study UID and add to criteria list at numitems... */
    			if (temp != (char *) NULL) free(temp);
    			if ((temp = (char *) malloc(strlen(GS_StuNodes[i].StuInsUID) + 1)) == (char *) NULL) {
    				ret_val = ~TBL_NORMAL;
    				break;
    			}
    			criteria[numsercrits].Value.Value.String = temp;
    			strcpy(criteria[numsercrits].Value.Value.String, GS_StuNodes[i].StuInsUID);
    			criteria[numsercrits].Value.AllocatedSize = strlen(GS_StuNodes[i].StuInsUID);
    			criteria[numsercrits].Value.Size = strlen(GS_StuNodes[i].StuInsUID);
    			ret_val = TBL_Select(&serhandle, criteria, field, NULL, CBSel_CollectSeries, NULL);
    		}
    		numsercrits++;
    	}else{
    		ret_val = TBL_Select(&serhandle, criteria, field, NULL, CBSel_CollectSeries, NULL);
    	}

    	if (GS_NumSerNodes == 0 || ret_val != TBL_NORMAL) {
    		FREE_STRINGS(i, GS_sercl, numsercrits);
    		if (GS_NumPatNodes) {
    			FREE_STRINGS(i, GS_patcl, numpatcrits);
    			free(GS_PatNodes);
    			free(GS_NullPatFlag);
    		}
    		if (GS_NumStuNodes) {
    			FREE_STRINGS(i, GS_stucl, numstucrits);
    			free(GS_StuNodes);
    			free(GS_NullStuFlag);
    		}
    		if (GS_NumSerNodes) {
    			free(GS_SerNodes);
    			free(GS_NullSerFlag);
    		}
    		if (!GS_NumSerNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    }
    GS_ImaNodes = (IDB_ImageQuery *) NULL;
    GS_NumImaNodes = 0;
    GS_NullImaFlag = (long *) NULL;

    if (begin_level == IDB_IMAGE_LEVEL || end_level == IDB_IMAGE_LEVEL) {
    	criteria = GS_imacl;
    	field = GS_IMASEL_Field;
    	if ((numimacrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_IMAGE_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;
    	if (GS_NumSerNodes != 0) {
    		criteria = GS_imacl;
    		criteria[numimacrits].FieldName = "SerParent";
    		criteria[numimacrits].Value.Type = TBL_STRING;
    		criteria[numimacrits].Value.IsNull = 0;
    		criteria[numimacrits].Operator = TBL_EQUAL;
    		criteria[numimacrits + 1].FieldName = 0;

    		temp = (char *) NULL;

    		for (i = 0; i < GS_NumSerNodes; i++) {
    			/* Get series UID and add to criteria list at numitems... */
    			if (temp != (char *) NULL) free(temp);
    			if ((temp = (char *) malloc(strlen(GS_SerNodes[i].SerInsUID) + 1)) == (char *) NULL) {
    				ret_val = ~TBL_NORMAL;
    				break;
    			}
    			criteria[numimacrits].Value.Value.String = temp;
    			strcpy(criteria[numimacrits].Value.Value.String, GS_SerNodes[i].SerInsUID);
    			criteria[numimacrits].Value.AllocatedSize = strlen(GS_SerNodes[i].SerInsUID);
    			criteria[numimacrits].Value.Size = strlen(GS_SerNodes[i].SerInsUID);
    			ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
    		}
    		numimacrits++;
    	}else{
    		ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
    	}

    	if (GS_NumImaNodes == 0 || ret_val != TBL_NORMAL) {
    		FREE_STRINGS(i, GS_imacl, numimacrits);
    		if (GS_NumPatNodes) {
    			FREE_STRINGS(i, GS_patcl, numpatcrits);
    			free(GS_PatNodes);
    			free(GS_NullPatFlag);
    		}
    		if (GS_NumStuNodes) {
    			FREE_STRINGS(i, GS_stucl, numstucrits);
    			free(GS_StuNodes);
    			free(GS_NullStuFlag);
    		}
    		if (GS_NumSerNodes) {
    			FREE_STRINGS(i, GS_sercl, numsercrits);
    			free(GS_SerNodes);
    			free(GS_NullSerFlag);
    		}
    		if (GS_NumImaNodes) {
    			free(GS_ImaNodes);
    			free(GS_NullImaFlag);
    		}
    		if (!GS_NumImaNodes) {
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
    		}else{
    			THR_ReleaseMutex(FAC_IDB);
    			return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
    		}
    	}
    	/*
    	 * Now go and get the instances associated with each image node.
    	 * Cycle through all the GS_ImaNodes and fill up each with an
    	 * instances list. GS_ImaNodes[i].InstanceList...and allocate a bunch
    	 * of Instance List Elements.
    	 *
    	 * Pass along the head pointer as the void * argument.
    	 */

    	inscrit[1].FieldName = 0;

    	for (i = 0; i < GS_NumImaNodes; i++) {
    		GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
    		TBL_CRITERIA_LOAD_BYTE(inscrit[0], "ImageUID", GS_ImaNodes[i].SOPInsUID, TBL_STRING, TBL_EQUAL);
    		field = GS_INSSEL_Field;
    		ret_val = TBL_Select(&inshandle, inscrit, field, NULL, CBSel_CollectInstances, (void *) &(GS_ImaNodes[i].InstanceList));
    		if (ret_val != TBL_NORMAL) GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
    	}
    }

    if (numpatcrits) FREE_STRINGS(i, GS_patcl, numpatcrits);
    if (numstucrits) FREE_STRINGS(i, GS_stucl, numstucrits);
    if (numsercrits) FREE_STRINGS(i, GS_sercl, numsercrits);
    if (numimacrits) FREE_STRINGS(i, GS_imacl, numimacrits);

    /*
     * Now we have lists of all the nodes of interest that match the criteria
     * finished...now cycle through them and return the correct information
     * via the users callback routine....
     * 
     * This is the section that uses the dreaded "goto"....ich.
     */
    memset(&ret_query, 0, sizeof(ret_query));
    ret_query.PatientQFlag = ret_query.StudyQFlag = ret_query.SeriesQFlag = ret_query.ImageQFlag = 0;
    tempcount = 0;

    if (GS_NumPatNodes != 0) {
    	for (ipat = 0; ipat < GS_NumPatNodes; ipat++) {
    		ret_query.patient = GS_PatNodes[ipat];
    		ret_query.PatientQFlag = pssi->PatientQFlag;
    		ret_query.PatientNullFlag = GS_NullPatFlag[ipat];
    		if (GS_NumStuNodes != 0) {
    			for (istu = 0; istu < GS_NumStuNodes; istu++) {
    				if (strcmp(GS_PatNodes[ipat].PatID, GS_StuNodes[istu].PatParent) == 0) {
    					ret_query.study = GS_StuNodes[istu];
    					ret_query.StudyQFlag = pssi->StudyQFlag;
    					ret_query.StudyNullFlag = GS_NullStuFlag[istu];
    					if (GS_NumSerNodes != 0) {
    						for (iser = 0; iser < GS_NumSerNodes; iser++) {
    							if (strcmp(GS_StuNodes[istu].StuInsUID, GS_SerNodes[iser].StuParent) == 0) {
    								ret_query.series = GS_SerNodes[iser];
    								ret_query.SeriesQFlag = pssi->SeriesQFlag;
    								ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    								if (GS_NumImaNodes != 0) {
    									for (iima = 0; iima < GS_NumImaNodes; iima++) {
    										if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    											ret_query.image = GS_ImaNodes[iima];
    											ret_query.ImageQFlag = pssi->ImageQFlag;
    											ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    											tempcount++;
    											if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    										}
    									}
    								}else{
    									tempcount++;
    									if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)	goto EarlyExit;
    								}
    							}
    						}
    					}else{
    						tempcount++;
    						if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    					}
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }else if (GS_NumStuNodes != 0){
    	for (istu = 0; istu < GS_NumStuNodes; istu++) {
    		ret_query.study = GS_StuNodes[istu];
    		ret_query.StudyQFlag = pssi->StudyQFlag;
    		ret_query.StudyNullFlag = GS_NullStuFlag[istu];
    		if (GS_NumSerNodes != 0) {
    			for (iser = 0; iser < GS_NumSerNodes; iser++) {
    				if (strcmp(GS_StuNodes[istu].StuInsUID, GS_SerNodes[iser].StuParent) == 0) {
    					ret_query.series = GS_SerNodes[iser];
    					ret_query.SeriesQFlag = pssi->SeriesQFlag;
    					ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    					if (GS_NumImaNodes != 0) {
    						for (iima = 0; iima < GS_NumImaNodes; iima++) {
    							if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    								ret_query.image = GS_ImaNodes[iima];
    								ret_query.ImageQFlag = pssi->ImageQFlag;
    								ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    								tempcount++;
    								if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    							}
    						}
    					}else{
    						tempcount++;
    						if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    					}
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }else if (GS_NumSerNodes != 0){
    	for (iser = 0; iser < GS_NumSerNodes; iser++) {
    		ret_query.series = GS_SerNodes[iser];
    		ret_query.SeriesQFlag = pssi->SeriesQFlag;
    		ret_query.SeriesNullFlag = GS_NullSerFlag[iser];
    		if (GS_NumImaNodes != 0) {
    			for (iima = 0; iima < GS_NumImaNodes; iima++) {
    				if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    					ret_query.image = GS_ImaNodes[iima];
    					ret_query.ImageQFlag = pssi->ImageQFlag;
    					ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    					tempcount++;
    					if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)  goto EarlyExit;
    				}
    			}
    		}else{
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL)	goto EarlyExit;
    		}
    	}
    }else if (GS_NumImaNodes != 0){
    	for (iima = 0; iima < GS_NumImaNodes; iima++) {
    		if (strcmp(GS_SerNodes[iser].SerInsUID, GS_ImaNodes[iima].SerParent) == 0) {
    			ret_query.image = GS_ImaNodes[iima];
    			ret_query.ImageQFlag = pssi->ImageQFlag;
    			ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }
    if (count != NULL)	*count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement		    * insnode;
    	for (i = 0; i < GS_NumImaNodes; i++) {
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;

EarlyExit:

    if (count != NULL)	*count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement		    * insnode;
    	for (i = 0; i < GS_NumImaNodes; i++) {
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return COND_PushCondition(IDB_ERROR(IDB_EARLYEXIT), "IDB_Select");
}

CONDITION
IDB_Select_ImageLevel(IDB_HANDLE ** handle, IDB_Query * pssi, long *count, CONDITION(callback()), void *ctx)
{
    IDB_Query					ret_query;
    TBL_FIELD					* field;
    TBL_CRITERIA				inscrit[2],	*criteria;
    TBL_HANDLE					* pathandle, *stuhandle, *serhandle, *imahandle, *inshandle;
    IDB_CONTEXT					* idbc;
    long				        i, iima, tempcount, numpatcrits, numstucrits, numsercrits = 0, numimacrits, foundit;
    void				        *temp;
    CONDITION					ret_val = 0;

    THR_ObtainMutex(FAC_IDB);
    /*
     * Locate the handle in the open table...
     */
    idbc = GS_ContextHead;
    foundit = 0;

    while (idbc != (IDB_CONTEXT *) NULL) {
    	if (idbc == (IDB_CONTEXT *) (*handle)) {
    		pathandle = idbc->PatNodes;
    		stuhandle = idbc->StuNodes;
    		serhandle = idbc->SerNodes;
    		imahandle = idbc->ImaNodes;
    		inshandle = idbc->InsNodes;
    		foundit = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_Select");
    }
    numimacrits = 0;
    GS_PatNodes = (IDB_PatientQuery *) NULL;
    GS_NullPatFlag = (long *) NULL;
    GS_NumPatNodes = 0;

    GS_StuNodes = (IDB_StudyQuery *) NULL;
    GS_NumStuNodes = 0;
    GS_NullStuFlag = (long *) NULL;

    GS_SerNodes = (IDB_SeriesQuery *) NULL;
    GS_NumSerNodes = 0;
    GS_NullSerFlag = (long *) NULL;

    GS_ImaNodes = (IDB_ImageQuery *) NULL;
    GS_NumImaNodes = 0;
    GS_NullImaFlag = (long *) NULL;
    {
	criteria = GS_imacl;
	field = GS_IMASEL_Field;
	if ((numimacrits = IDB_ConvertDicomQuerytoSQL(pssi, IDB_IMAGE_LEVEL)) == 0) criteria = (TBL_CRITERIA *) NULL;
	if (GS_NumSerNodes != 0){
		criteria = GS_imacl;
	    criteria[numimacrits].FieldName = "SerParent";
	    criteria[numimacrits].Value.Type = TBL_STRING;
	    criteria[numimacrits].Value.IsNull = 0;
	    criteria[numimacrits].Operator = TBL_EQUAL;
	    criteria[numimacrits + 1].FieldName = 0;

	    temp = (char *) NULL;

	    for (i = 0; i < GS_NumSerNodes; i++) {
	    	/* Get series UID and add to criteria list at numitems... */
	    	if (temp != (char *) NULL) free(temp);
	    	if ((temp = (char *) malloc(strlen(GS_SerNodes[i].SerInsUID) + 1)) == (char *) NULL) {
	    		ret_val = ~TBL_NORMAL;
				break;
	    	}
	    	criteria[numimacrits].Value.Value.String = temp;
	    	strcpy(criteria[numimacrits].Value.Value.String, GS_SerNodes[i].SerInsUID);
	    	criteria[numimacrits].Value.AllocatedSize = strlen(GS_SerNodes[i].SerInsUID);
	    	criteria[numimacrits].Value.Size = strlen(GS_SerNodes[i].SerInsUID);
	    	ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
	    }
	    numimacrits++;
	}else{
		ret_val = TBL_Select(&imahandle, criteria, field, NULL, CBSel_CollectImages, NULL);
	}

	if (GS_NumImaNodes == 0 || ret_val != TBL_NORMAL){
		FREE_STRINGS(i, GS_imacl, numimacrits);
		if (GS_NumPatNodes) {
			FREE_STRINGS(i, GS_patcl, numpatcrits);
			free(GS_PatNodes);
			free(GS_NullPatFlag);
	    }
	    if (GS_NumStuNodes) {
	    	FREE_STRINGS(i, GS_stucl, numstucrits);
	    	free(GS_StuNodes);
	    	free(GS_NullStuFlag);
	    }
	    if (GS_NumSerNodes) {
	    	FREE_STRINGS(i, GS_sercl, numsercrits);
	    	free(GS_SerNodes);
	    	free(GS_NullSerFlag);
	    }
	    if (GS_NumImaNodes) {
	    	free(GS_ImaNodes);
	    	free(GS_NullImaFlag);
	    }
	    if (!GS_NumImaNodes) {
	    	THR_ReleaseMutex(FAC_IDB);
	    	return COND_PushCondition(IDB_ERROR(IDB_NOMATCHES), "IDB_Select");
	    }else{
	    	THR_ReleaseMutex(FAC_IDB);
	    	return COND_PushCondition(IDB_ERROR(IDB_NOMEMORY), "IDB_Select");
	    }
	}
	/*
	 * Now go and get the instances associated with each image node.
	 * Cycle through all the GS_ImaNodes and fill up each with an
	 * instances list. GS_ImaNodes[i].InstanceList...and allocate a bunch
	 * of Instance List Elements.
	 * 
	 * Pass along the head pointer as the void * argument.
	 */

	inscrit[1].FieldName = 0;

	for (i = 0; i < GS_NumImaNodes; i++){
		GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
		TBL_CRITERIA_LOAD_BYTE(inscrit[0], "ImageUID", GS_ImaNodes[i].SOPInsUID, TBL_STRING, TBL_EQUAL);
		field = GS_INSSEL_Field;

	    ret_val = TBL_Select(&inshandle, inscrit, field, NULL, CBSel_CollectInstances, (void *) &(GS_ImaNodes[i].InstanceList));
	    if (ret_val != TBL_NORMAL) GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;
	}

    }
    if (numpatcrits) FREE_STRINGS(i, GS_patcl, numpatcrits);
    if (numstucrits) FREE_STRINGS(i, GS_stucl, numstucrits);
    if (numsercrits) FREE_STRINGS(i, GS_sercl, numsercrits);
    if (numimacrits) FREE_STRINGS(i, GS_imacl, numimacrits);

    /*
     * Now we have lists of all the nodes of interest that match the criteria
     * finished...now cycle through them and return the correct information
     * via the users callback routine....
     * 
     * This is the section that uses the dreaded "goto"....ich.
     */
    memset(&ret_query, 0, sizeof(ret_query));
    ret_query.PatientQFlag = ret_query.StudyQFlag = ret_query.SeriesQFlag = ret_query.ImageQFlag = 0;
    tempcount = 0;

    if (GS_NumPatNodes != 0){

    }else if (GS_NumStuNodes != 0){

    }else if (GS_NumSerNodes != 0){

    }else if (GS_NumImaNodes != 0){
    	for (iima = 0; iima < GS_NumImaNodes; iima++) {
    		{
    			ret_query.image = GS_ImaNodes[iima];
    			ret_query.ImageQFlag = pssi->ImageQFlag;
    			ret_query.ImageNullFlag = GS_NullImaFlag[iima];
    			tempcount++;
    			if (callback(&ret_query, tempcount, ctx) != IDB_NORMAL) goto EarlyExit;
    		}
    	}
    }

    if (count != NULL) *count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement		    * insnode;
    	for (i = 0; i < GS_NumImaNodes; i++){
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;

EarlyExit:

    if (count != NULL) *count = tempcount;
    if (GS_PatNodes) {
    	free(GS_PatNodes);
    	free(GS_NullPatFlag);
    }
    if (GS_StuNodes) {
    	free(GS_StuNodes);
    	free(GS_NullStuFlag);
    }
    if (GS_SerNodes) {
    	free(GS_SerNodes);
    	free(GS_NullSerFlag);
    }
    if (GS_ImaNodes) {
    	IDB_InstanceListElement		    * insnode;
    	for (i = 0; i < GS_NumImaNodes; i++) {
    		if (GS_ImaNodes[i].InstanceList != (LST_HEAD *) NULL) {
    			while (LST_Count(&(GS_ImaNodes[i].InstanceList)) != 0) {
    				insnode = LST_Dequeue(&(GS_ImaNodes[i].InstanceList));
    				free(insnode);
    			}
    			LST_Destroy(&(GS_ImaNodes[i].InstanceList));
    		}
    	}
    	free(GS_ImaNodes);
    	free(GS_NullImaFlag);
    }
    THR_ReleaseMutex(FAC_IDB);
    return COND_PushCondition(IDB_ERROR(IDB_EARLYEXIT), "IDB_Select");

}

#ifdef CTN_IDBV2
CONDITION
IDB_SelectLimits(IDB_HANDLE ** handle, IDB_Limits * limits)
{
    long 			count = 0;
    CONDITION 		cond;
    TBL_HANDLE 		*limitsHandle;
    int 			foundit = 0;
    IDB_CONTEXT 	*idbc;

    TBL_FIELD fields[] = {
    		{"DBSize", TBL_SIGNED4, 4, 4, 0, NULL},
    		{"DBLimit", TBL_SIGNED4, 4, 4, 0, NULL},
    		{"PatientCount", TBL_SIGNED4, 4, 4, 0, NULL},
    		{"StudyCount", TBL_SIGNED4, 4, 4, 0, NULL},
    		{"ImageCount", TBL_SIGNED4, 4, 4, 0, NULL},
    		{NULL, TBL_OTHER, 0, 0, 0, NULL}
    };

    THR_ObtainMutex(FAC_IDB);

    fields[0].Value.Value.Signed4 = &limits->DBSize;
    fields[1].Value.Value.Signed4 = &limits->DBLimit;
    fields[2].Value.Value.Signed4 = &limits->PatientCount;
    fields[3].Value.Value.Signed4 = &limits->StudyCount;
    fields[4].Value.Value.Signed4 = &limits->ImageCount;

    idbc = GS_ContextHead;
    foundit = 0;
    while (idbc != (IDB_CONTEXT *) NULL) {
    	if (idbc == (IDB_CONTEXT *) (*handle)) {
    		limitsHandle = idbc->Limits;
    		foundit = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_SelectLimits");
    }
    cond = TBL_Select(&limitsHandle, NULL, fields, &count, NULL, NULL);
    if (cond != TBL_NORMAL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return 0;		/* repair */
    }
    if (count != 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return 0;		/* repair */
    }

    THR_ReleaseMutex(FAC_IDB);
    return IDB_NORMAL;
}
#endif

CONDITION
CBSel_CollectPatientsStudies(TBL_FIELD * fp, int count, void *ctx)
{

    int	        i;

    GS_NumPatNodes++;
    if (GS_PatNodes == (IDB_PatientQuery *) NULL) {
    	if ((GS_PatNodes = (IDB_PatientQuery *) malloc(sizeof(IDB_PatientQuery) * GS_NumPatNodes)) == (IDB_PatientQuery *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_PatNodes = (IDB_PatientQuery *) realloc(GS_PatNodes, sizeof(IDB_PatientQuery) * GS_NumPatNodes)) == (IDB_PatientQuery *) NULL){
		return ~TBL_NOMEMORY;
    }
    if (GS_NullPatFlag == (long *) NULL){
    	if ((GS_NullPatFlag = (long *) malloc(sizeof(long) * GS_NumPatNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_NullPatFlag = (long *) realloc(GS_NullPatFlag, sizeof(long) * GS_NumPatNodes)) == (long *) NULL){
		return ~TBL_NOMEMORY;
    }

    GS_NumStuNodes++;
    if (GS_StuNodes == (IDB_StudyQuery *) NULL){
    	if ((GS_StuNodes = (IDB_StudyQuery *) malloc(sizeof(IDB_StudyQuery) * GS_NumStuNodes)) == (IDB_StudyQuery *) NULL) return ~TBL_NOMEMORY;
    }else if ((GS_StuNodes = (IDB_StudyQuery *) realloc(GS_StuNodes, sizeof(IDB_StudyQuery) * GS_NumStuNodes)) == (IDB_StudyQuery *) NULL){
    	return ~TBL_NOMEMORY;
    }
    if (GS_NullStuFlag == (long *) NULL) {
    	if ((GS_NullStuFlag = (long *) malloc(sizeof(long) * GS_NumStuNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_NullStuFlag = (long *) realloc(GS_NullStuFlag, sizeof(long) * GS_NumStuNodes)) == (long *) NULL){
		return ~TBL_NOMEMORY;
    }

    i = GS_NumPatNodes - 1;
    GS_NullPatFlag[i] = 0;
    GS_NullStuFlag[i] = 0;
    while (fp->FieldName != 0) {
    	if (strcmp(fp->FieldName, "Pat_PatNam") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatNam;
    		}else{
    			strcpy(GS_PatNodes[i].PatNam, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_PatID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatID;
    		}else{
    			strcpy(GS_PatNodes[i].PatID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_PatSex") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatSex;
    		}else{
    			strcpy(GS_PatNodes[i].PatSex, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_Owner;
    		}else{
    			strcpy(GS_PatNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_GroupName;
    		}else{
    			strcpy(GS_PatNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_Priv;
    		}else{
    			strcpy(GS_PatNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_PatBirDat") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatBirDat;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_PatNodes[i].PatBirDat);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_PatBirTim") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatBirTim;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_PatNodes[i].PatBirTim);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_PatNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_PatNodes[i].InsertTime);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_NumPatRelStu") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelStu;
    		}else{
    			GS_PatNodes[i].NumPatRelStu = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_NumPatRelSer") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelSer;
    		}else{
    			GS_PatNodes[i].NumPatRelSer = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Pat_NumPatRelIma") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelIma;
    		}else{
    			GS_PatNodes[i].NumPatRelIma = *(fp->Value.Value.Signed4);
    		}
    	}

    	if (strcmp(fp->FieldName, "Stu_AccNum") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_AccNum;
    		}else{
    			strcpy(GS_StuNodes[i].AccNum, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_StuID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuID;
    		}else{
    			strcpy(GS_StuNodes[i].StuID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_StuInsUID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuInsUID;
    		}else{
    			strcpy(GS_StuNodes[i].StuInsUID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_RefPhyNam") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_RefPhyNam;
    		}else{
    			strcpy(GS_StuNodes[i].RefPhyNam, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_StuDes") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuDes;
    		}else{
    			strcpy(GS_StuNodes[i].StuDes, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_PatAge") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatAge;
    		}else{
    			strcpy(GS_StuNodes[i].PatAge, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_PatSiz") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatSiz;
    		}else{
    			strcpy(GS_StuNodes[i].PatSiz, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_PatWei") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatWei;
    		}else{
    			strcpy(GS_StuNodes[i].PatWei, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_Owner;
    		}else{
    			strcpy(GS_StuNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_GroupName;
    		}else{
    			strcpy(GS_StuNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_Priv;
    		}else{
    			strcpy(GS_StuNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_StuDat") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuDat;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_StuNodes[i].StuDat);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_StuTim") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuTim;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_StuNodes[i].StuTim);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_StuNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_StuNodes[i].InsertTime);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_NumStuRelSer") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_NumStuRelSer;
    		}else{
    			GS_StuNodes[i].NumStuRelSer = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Stu_NumStuRelIma") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_NumStuRelIma;
    		}else{
    			GS_StuNodes[i].NumStuRelIma = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Ser_ModsInStudy") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_ModsInStudy;
    		}else{
    			strcpy(GS_StuNodes[i].ModsInStudy, fp->Value.Value.String);
    		}
    	}
    	fp++;
    }
    return TBL_NORMAL;
}

CONDITION
CBSel_CollectPatients(TBL_FIELD * fp, int count, void *ctx)
{

    int	        i;

    GS_NumPatNodes++;
    if (GS_PatNodes == (IDB_PatientQuery *) NULL) {
    	if ((GS_PatNodes = (IDB_PatientQuery *) malloc(sizeof(IDB_PatientQuery) * GS_NumPatNodes)) == (IDB_PatientQuery *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_PatNodes = (IDB_PatientQuery *) realloc(GS_PatNodes, sizeof(IDB_PatientQuery) * GS_NumPatNodes)) == (IDB_PatientQuery *) NULL){
		return ~TBL_NOMEMORY;
    }
    if (GS_NullPatFlag == (long *) NULL){
    	if ((GS_NullPatFlag = (long *) malloc(sizeof(long) * GS_NumPatNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_NullPatFlag = (long *) realloc(GS_NullPatFlag, sizeof(long) * GS_NumPatNodes)) == (long *) NULL){
		return ~TBL_NOMEMORY;
    }

    i = GS_NumPatNodes - 1;
    GS_NullPatFlag[i] = 0;
    while (fp->FieldName != 0){
    	if (strcmp(fp->FieldName, "PatNam") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatNam;
    		}else{
    			strcpy(GS_PatNodes[i].PatNam, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatID;
    		}else{
    			strcpy(GS_PatNodes[i].PatID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatSex") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatSex;
    		}else{
    			strcpy(GS_PatNodes[i].PatSex, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_Owner;
    		}else{
    			strcpy(GS_PatNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_GroupName;
    		}else{
    			strcpy(GS_PatNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_Priv;
    		}else{
    			strcpy(GS_PatNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatBirDat") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatBirDat;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_PatNodes[i].PatBirDat);
    		}
    	}else if (strcmp(fp->FieldName, "PatBirTim") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_PatBirTim;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_PatNodes[i].PatBirTim);
    		}
    	}else if (strcmp(fp->FieldName, "InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_PatNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_PatNodes[i].InsertTime);
    		}
    	}else if (strcmp(fp->FieldName, "NumPatRelStu") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelStu;
    		}else{
    			GS_PatNodes[i].NumPatRelStu = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "NumPatRelSer") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelSer;
    		}else{
    			GS_PatNodes[i].NumPatRelSer = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "NumPatRelIma") == 0){
    		if (fp->Value.IsNull){
    			GS_NullPatFlag[i] |= QF_PAT_NumPatRelIma;
    		}else{
    			GS_PatNodes[i].NumPatRelIma = *(fp->Value.Value.Signed4);
    		}
    	}
    	fp++;
    }
    return TBL_NORMAL;
}

CONDITION
CBSel_CollectStudies(TBL_FIELD * fp, int count, void *ctx)
{
    int        i;

    GS_NumStuNodes++;
    if (GS_StuNodes == (IDB_StudyQuery *) NULL){
    	if ((GS_StuNodes = (IDB_StudyQuery *) malloc(sizeof(IDB_StudyQuery) * GS_NumStuNodes)) == (IDB_StudyQuery *) NULL) return ~TBL_NOMEMORY;
    }else if ((GS_StuNodes = (IDB_StudyQuery *) realloc(GS_StuNodes, sizeof(IDB_StudyQuery) * GS_NumStuNodes)) == (IDB_StudyQuery *) NULL){
    	return ~TBL_NOMEMORY;
    }
    if (GS_NullStuFlag == (long *) NULL) {
    	if ((GS_NullStuFlag = (long *) malloc(sizeof(long) * GS_NumStuNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_NullStuFlag = (long *) realloc(GS_NullStuFlag, sizeof(long) * GS_NumStuNodes)) == (long *) NULL){
		return ~TBL_NOMEMORY;
    }

    i = GS_NumStuNodes - 1;
    GS_NullStuFlag[i] = 0;
    while (fp->FieldName != 0){
    	if (strcmp(fp->FieldName, "AccNum") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_AccNum;
    		}else{
    			strcpy(GS_StuNodes[i].AccNum, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "StuID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuID;
    		}else{
    			strcpy(GS_StuNodes[i].StuID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "StuInsUID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuInsUID;
    		}else{
    			strcpy(GS_StuNodes[i].StuInsUID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "RefPhyNam") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_RefPhyNam;
    		}else{
    			strcpy(GS_StuNodes[i].RefPhyNam, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "StuDes") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuDes;
    		}else{
    			strcpy(GS_StuNodes[i].StuDes, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatAge") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatAge;
    		}else{
    			strcpy(GS_StuNodes[i].PatAge, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatSiz") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatSiz;
    		}else{
    			strcpy(GS_StuNodes[i].PatSiz, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatWei") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_PatWei;
    		}else{
    			strcpy(GS_StuNodes[i].PatWei, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_Owner;
    		}else{
    			strcpy(GS_StuNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_GroupName;
    		}else{
    			strcpy(GS_StuNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_Priv;
    		}else{
    			strcpy(GS_StuNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatParent") == 0){
    		strcpy(GS_StuNodes[i].PatParent, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "StuDat") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuDat;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_StuNodes[i].StuDat);
    		}
    	}else if (strcmp(fp->FieldName, "StuTim") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_StuTim;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_StuNodes[i].StuTim);
    		}
    	}else if (strcmp(fp->FieldName, "InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_StuNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_StuNodes[i].InsertTime);
    		}
    	}else if (strcmp(fp->FieldName, "NumStuRelSer") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_NumStuRelSer;
    		}else{
    			GS_StuNodes[i].NumStuRelSer = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "NumStuRelIma") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_STU_NumStuRelIma;
    		}else{
    			GS_StuNodes[i].NumStuRelIma = *(fp->Value.Value.Signed4);
    		}
    	}
    	fp++;
    }
    return TBL_NORMAL;
}

CONDITION
CBSel_CollectSeries(TBL_FIELD * fp, int count, void *ctx)
{
    int	        i;

    GS_NumSerNodes++;
    if (GS_SerNodes == (IDB_SeriesQuery *) NULL) {
    	if ((GS_SerNodes = (IDB_SeriesQuery *) malloc(sizeof(IDB_SeriesQuery) * GS_NumSerNodes)) == (IDB_SeriesQuery *) NULL) return ~TBL_NOMEMORY;
	}else if ((GS_SerNodes = (IDB_SeriesQuery *) realloc(GS_SerNodes, sizeof(IDB_SeriesQuery) * GS_NumSerNodes)) == (IDB_SeriesQuery *) NULL){
		return ~TBL_NOMEMORY;
    }
    if (GS_NullSerFlag == (long *) NULL) {
    	if ((GS_NullSerFlag = (long *) malloc(sizeof(long) * GS_NumSerNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
    }else if ((GS_NullSerFlag = (long *) realloc(GS_NullSerFlag, sizeof(long) * GS_NumSerNodes)) == (long *) NULL){
    	return ~TBL_NOMEMORY;
    }

    i = GS_NumSerNodes - 1;
    GS_NullSerFlag[i] = 0;
    while (fp->FieldName != 0) {
    	if (strcmp(fp->FieldName, "Mod") == 0) {
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_Mod;
    		}else{
    			strcpy(GS_SerNodes[i].Mod, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SerNum") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_SerNum;
    		}else{
    			strcpy(GS_SerNodes[i].SerNum, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SerInsUID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_SerInsUID;
    		}else{
    			strcpy(GS_SerNodes[i].SerInsUID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "ProNam") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_ProNam;
    		}else{
    			strcpy(GS_SerNodes[i].ProNam, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SerDes") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_SerDes;
    		}else{
    			strcpy(GS_SerNodes[i].SerDes, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "BodParExa") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_BodParExa;
    		}else{
    			strcpy(GS_SerNodes[i].BodParExa, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "ViePos") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_ViePos;
    		}else{
    			strcpy(GS_SerNodes[i].ViePos, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_Owner;
    		}else{
    			strcpy(GS_SerNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_GroupName;
    		}else{
    			strcpy(GS_SerNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_Priv;
    		}else{
    			strcpy(GS_SerNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "StuParent") == 0){
    		strcpy(GS_SerNodes[i].StuParent, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_SerNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullSerFlag[i] |= QF_SER_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_SerNodes[i].InsertTime);
    		}
    	}else if (strcmp(fp->FieldName, "NumSerRelIma") == 0){
    		if (fp->Value.IsNull){
    			GS_NullStuFlag[i] |= QF_SER_NumSerRelIma;
    		}else{
    			GS_SerNodes[i].NumSerRelIma = *(fp->Value.Value.Signed4);
    		}
    	}
    	fp++;
    }
    return TBL_NORMAL;
}

CONDITION
CBSel_CollectImages(TBL_FIELD * fp, int count, void *ctx)
{

    int	        i;

    GS_NumImaNodes++;
    if (GS_ImaNodes == (IDB_ImageQuery *) NULL) {
    	if ((GS_ImaNodes = (IDB_ImageQuery *) malloc(sizeof(IDB_ImageQuery) * GS_NumImaNodes)) == (IDB_ImageQuery *) NULL) return ~TBL_NOMEMORY;
    }else if ((GS_ImaNodes = (IDB_ImageQuery *) realloc(GS_ImaNodes, sizeof(IDB_ImageQuery) * GS_NumImaNodes)) == (IDB_ImageQuery *) NULL) {
    	return ~TBL_NOMEMORY;
    }
    if (GS_NullImaFlag == (long *) NULL) {
    	if ((GS_NullImaFlag = (long *) malloc(sizeof(long) * GS_NumImaNodes)) == (long *) NULL) return ~TBL_NOMEMORY;
    }else if ((GS_NullImaFlag = (long *) realloc(GS_NullImaFlag, sizeof(long) * GS_NumImaNodes)) == (long *) NULL){
    	return ~TBL_NOMEMORY;
    }

    i = GS_NumImaNodes - 1;
    GS_NullImaFlag[i] = 0;
    while (fp->FieldName != 0){
    	if (strcmp(fp->FieldName, "ImaNum") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_ImaNum;
    		}else{
    			strcpy(GS_ImaNodes[i].ImaNum, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SOPInsUID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_SOPInsUID;
    		}else{
    			strcpy(GS_ImaNodes[i].SOPInsUID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SOPClaUID") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_SOPClaUID;
    		}else{
    			strcpy(GS_ImaNodes[i].SOPClaUID, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SamPerPix") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_SamPerPix;
    		}else{
    			GS_ImaNodes[i].SamPerPix = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "PhoInt") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_PhoInt;
    		}else{
    			strcpy(GS_ImaNodes[i].PhoInt, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "PatOri") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_PatOri;
    		}else{
    			strcpy(GS_ImaNodes[i].PatOri, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Row") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_Row;
    		}else{
    			GS_ImaNodes[i].Row = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Col") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_Col;
    		}else{
    			GS_ImaNodes[i].Col = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "BitAll") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_BitAll;
    		}else{
    			GS_ImaNodes[i].BitAll = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "BitSto") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_BitSto;
    		}else{
    			GS_ImaNodes[i].BitSto = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "PixRep") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_PixRep;
    		}else{
    			GS_ImaNodes[i].PixRep = *(fp->Value.Value.Signed4);
    		}
    	}else if (strcmp(fp->FieldName, "Owner") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_Owner;
    		}else{
    			strcpy(GS_ImaNodes[i].Owner, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "GroupName") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_GroupName;
    		}else{
    			strcpy(GS_ImaNodes[i].GroupName, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "Priv") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_Priv;
    		}else{
    			strcpy(GS_ImaNodes[i].Priv, fp->Value.Value.String);
    		}
    	}else if (strcmp(fp->FieldName, "SerParent") == 0){
    		strcpy(GS_ImaNodes[i].SerParent, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "InsertDate") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_InsertDate;
    		}else{
    			UTL_ConvertLongtoDate(*(fp->Value.Value.Signed4), GS_ImaNodes[i].InsertDate);
    		}
    	}else if (strcmp(fp->FieldName, "InsertTime") == 0){
    		if (fp->Value.IsNull){
    			GS_NullImaFlag[i] |= QF_IMA_InsertTime;
    		}else{
    			UTL_ConvertFloattoTime(*(fp->Value.Value.Float4), GS_ImaNodes[i].InsertTime);
    		}
    	}
    	fp++;
    }
    GS_ImaNodes[i].InstanceList = (LST_HEAD *) NULL;

    return TBL_NORMAL;
}

CONDITION
CBSel_CollectInstances(TBL_FIELD * fp, int count, void *ctx)
{

    LST_HEAD					** temp;
    IDB_InstanceListElement		* ile;

    temp = (LST_HEAD **) ctx;

    if (*temp == (LST_HEAD *) NULL)	*temp = LST_Create();
    if ((ile = (IDB_InstanceListElement *) malloc(sizeof(IDB_InstanceListElement))) == (IDB_InstanceListElement *) NULL) return ~TBL_NORMAL;

    while (fp->FieldName != 0){
    	if (strcmp(fp->FieldName, "RespondingTitle") == 0) {
    		strcpy(ile->RespondingTitle, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "Medium") == 0){
    		strcpy(ile->Medium, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "Path") == 0){
    		strcpy(ile->Path, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "Transfer") == 0){
    		strcpy(ile->Transfer, fp->Value.Value.String);
    	}else if (strcmp(fp->FieldName, "Size") == 0){
    		ile->Size = *(fp->Value.Value.Signed4);
    	}
    	fp++;
    }
    LST_Enqueue(temp, (void *) ile);

    return TBL_NORMAL;
}

long
IDB_ConvertDicomQuerytoSQL_PSView(IDB_Query * pssi)
{
    char       		*cs;
    int        		i, j;
    TBL_OPERATOR 	op;

    i = 0;
    if (pssi->PatientQFlag & QF_PAT_PatNam){
    	if (!(pssi->PatientNullFlag & QF_PAT_PatNam)){
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatNam)) == (char *) NULL){
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.PatNam);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_PatNam", cs, TBL_STRING, op);
    		i++;
    	}
	}
    if (pssi->PatientQFlag & QF_PAT_PatID){
    	if (!(pssi->PatientNullFlag & QF_PAT_PatID)){
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatID)) == (char *) NULL){
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.PatID);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_PatID", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_PatBirTim){
    	if (!(pssi->PatientNullFlag & QF_PAT_PatBirTim)){
    		UTL_SqueezeBlanks(pssi->patient.PatBirTim);
    		if (strchr(pssi->patient.PatBirTim, (int) '-') == (char *) NULL) {
    			GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat(pssi->patient.PatBirTim);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_EQUAL);
    		}else{
    			if (pssi->patient.PatBirTim[0] == '-') {
    				GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}else if (pssi->patient.PatBirTim[strlen(pssi->patient.PatBirTim) - 1] == '-'){
    				GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    			}else{
    				GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				i++;
    				GS_PAT_PatBirTimE = UTL_ConvertTimetoFloat(strchr(pssi->patient.PatBirTim, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirTim", GS_PAT_PatBirTimE, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_PatBirDat){
    	if (!(pssi->PatientNullFlag & QF_PAT_PatBirDat)){
    		UTL_SqueezeBlanks(pssi->patient.PatBirDat);
    		if (strchr(pssi->patient.PatBirDat, (int) '-') == (char *) NULL){
    			GS_PAT_PatBirDatB = UTL_ConvertDatetoLong(pssi->patient.PatBirDat);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_EQUAL);
    		}else{
    			if (pssi->patient.PatBirDat[0] == '-'){
    				GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}else if (pssi->patient.PatBirDat[strlen(pssi->patient.PatBirDat) - 1] == '-'){
    				GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    			}else{
    				GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				i++;
    				GS_PAT_PatBirDatE = UTL_ConvertDatetoLong(strchr(pssi->patient.PatBirDat, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_PatBirDat", GS_PAT_PatBirDatE, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_PatSex){
    	if (!(pssi->PatientNullFlag & QF_PAT_PatSex)){
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatSex)) == (char *) NULL){
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.PatSex);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_PatSex", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_InsertDate) {
    	if (!(pssi->PatientNullFlag & QF_PAT_InsertDate)) {
    		UTL_SqueezeBlanks(pssi->patient.InsertDate);
    		if (strchr(pssi->patient.InsertDate, (int) '-') == (char *) NULL) {
    			GS_PAT_InsertDateB = UTL_ConvertDatetoLong(pssi->patient.InsertDate);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
    		}else{
    			if (pssi->patient.InsertDate[0] == '-') {
    				GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}else if (pssi->patient.InsertDate[strlen(pssi->patient.InsertDate) - 1] == '-'){
    				GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    			}else{
    				GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				i++;
    				GS_PAT_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->patient.InsertDate, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertDate", GS_PAT_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_InsertTime) {
    	if (!(pssi->PatientNullFlag & QF_PAT_InsertTime)) {
    		UTL_SqueezeBlanks(pssi->patient.InsertTime);
    		if (strchr(pssi->patient.InsertTime, (int) '-') == (char *) NULL) {
    			GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat(pssi->patient.InsertTime);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
    		}else{
    			if (pssi->patient.InsertTime[0] == '-'){
    				GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}else if (pssi->patient.InsertTime[strlen(pssi->patient.InsertTime) - 1] == '-'){
    				GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    			}else{
    				GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				i++;
    				GS_PAT_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->patient.InsertTime, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Pat_InsertTime", GS_PAT_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_Owner) {
    	if (!(pssi->PatientNullFlag & QF_PAT_Owner)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.Owner)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.Owner);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_Owner", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_GroupName) {
    	if (!(pssi->PatientNullFlag & QF_PAT_GroupName)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.GroupName)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.GroupName);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_GroupName", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->PatientQFlag & QF_PAT_Priv) {
    	if (!(pssi->PatientNullFlag & QF_PAT_Priv)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.Priv)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->patient.Priv);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Pat_Priv", cs, TBL_STRING, op);
    		i++;
    	}
    }
    /*
     * Now do the study portion of the list...
     */
    if (pssi->StudyQFlag & QF_STU_StuDat) {
    	if (!(pssi->StudyNullFlag & QF_STU_StuDat)) {
    		UTL_SqueezeBlanks(pssi->study.StuDat);
    		if (strchr(pssi->study.StuDat, (int) '-') == (char *) NULL) {
    			GS_STU_StuDatB = UTL_ConvertDatetoLong(pssi->study.StuDat);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_EQUAL);
    		}else{
    			if (pssi->study.StuDat[0] == '-'){
    				GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}else if (pssi->study.StuDat[strlen(pssi->study.StuDat) - 1] == '-'){
    				GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    			}else{
    				GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				i++;
    				GS_STU_StuDatE = UTL_ConvertDatetoLong(strchr(pssi->study.StuDat, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuDat", GS_STU_StuDatE, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_StuTim) {
    	if (!(pssi->StudyNullFlag & QF_STU_StuTim)) {
    		UTL_SqueezeBlanks(pssi->study.StuTim);
    		if (strchr(pssi->study.StuTim, (int) '-') == (char *) NULL){
    			GS_STU_StuTimB = UTL_ConvertTimetoFloat(pssi->study.StuTim);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_EQUAL);
    		}else{
    			if (pssi->study.StuTim[0] == '-') {
    				GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}else if (pssi->study.StuTim[strlen(pssi->study.StuTim) - 1] == '-'){
    				GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    			}else{
    				GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				i++;
    				GS_STU_StuTimE = UTL_ConvertTimetoFloat(strchr(pssi->study.StuTim, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_StuTim", GS_STU_StuTimE, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
	}
    if (pssi->StudyQFlag & QF_STU_AccNum){
    	if (!(pssi->StudyNullFlag & QF_STU_AccNum)){
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.AccNum)) == (char *) NULL){
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.AccNum);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_AccNum", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_StuID) {
    	if (!(pssi->StudyNullFlag & QF_STU_StuID)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuID)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.StuID);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_StuID", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_StuInsUID) {
    	if (!(pssi->StudyNullFlag & QF_STU_StuInsUID)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuInsUID)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.StuInsUID);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_StuInsUID", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_RefPhyNam) {
    	if (!(pssi->StudyNullFlag & QF_STU_RefPhyNam)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.RefPhyNam)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.RefPhyNam);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_RefPhyNam", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_StuDes) {
    	if (!(pssi->StudyNullFlag & QF_STU_StuDes)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuDes)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.StuDes);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_StuDes", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_PatAge) {
    	if (!(pssi->StudyNullFlag & QF_STU_PatAge)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatAge)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.PatAge);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_PatAge", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_PatSiz) {
    	if (!(pssi->StudyNullFlag & QF_STU_PatSiz)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatSiz)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.PatSiz);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_PatSiz", cs, TBL_STRING, op);
    		i++;
    	}
	}
    if (pssi->StudyQFlag & QF_STU_PatWei) {
    	if (!(pssi->StudyNullFlag & QF_STU_PatWei)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatWei)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.PatWei);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_PatWei", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_InsertDate) {
    	if (!(pssi->StudyNullFlag & QF_STU_InsertDate)) {
    		UTL_SqueezeBlanks(pssi->study.InsertDate);
    		if (strchr(pssi->study.InsertDate, (int) '-') == (char *) NULL) {
    			GS_STU_InsertDateB = UTL_ConvertDatetoLong(pssi->study.InsertDate);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
    		}else{
    			if (pssi->study.InsertDate[0] == '-') {
    				GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}else if (pssi->study.InsertDate[strlen(pssi->study.InsertDate) - 1] == '-'){
    				GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    			}else{
    				GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				i++;
    				GS_STU_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->study.InsertDate, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertDate", GS_STU_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_InsertTime) {
    	if (!(pssi->StudyNullFlag & QF_STU_InsertTime)) {
    		UTL_SqueezeBlanks(pssi->study.InsertTime);
    		if (strchr(pssi->study.InsertTime, (int) '-') == (char *) NULL) {
    			GS_STU_InsertTimeB = UTL_ConvertTimetoFloat(pssi->study.InsertTime);
    			TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
    		}else{
    			if (pssi->study.InsertTime[0] == '-') {
    				GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime) + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}else if (pssi->study.InsertTime[strlen(pssi->study.InsertTime) - 1] == '-'){
    				GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    			}else{
    				GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime));
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				i++;
    				GS_STU_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->study.InsertTime, (int) '-') + 1);
    				TBL_CRITERIA_LOAD_NUM(GS_patstucl[i], "Stu_InsertTime", GS_STU_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
    			}
    		}
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_Owner) {
    	if (!(pssi->StudyNullFlag & QF_STU_Owner)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.Owner)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.Owner);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_Owner", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_GroupName) {
    	if (!(pssi->StudyNullFlag & QF_STU_GroupName)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.GroupName)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.GroupName);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_GroupName", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_Priv) {
    	if (!(pssi->StudyNullFlag & QF_STU_Priv)) {
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.Priv)) == (char *) NULL) {
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
    		op = likeOrEqualOperator(pssi->study.Priv);
    		TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Stu_Priv", cs, TBL_STRING, op);
    		i++;
    	}
    }
    if (pssi->StudyQFlag & QF_STU_ModsInStudy){
    	if (!(pssi->StudyNullFlag & QF_STU_ModsInStudy)){
    		if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.ModsInStudy)) == (char *) NULL){
    			FREE_STRINGS(j, GS_patstucl, i);
    			return (0);
    		}
			char *tok_cs = strtok(cs, "\\");
    		while (tok_cs != NULL) {
    			char *tmp_cs = (char *)malloc(strlen(tok_cs) + 2);
    			strcpy(tmp_cs,"%");
    			strcat(tmp_cs, tok_cs);
    			strcat(tmp_cs,"%");
    			TBL_CRITERIA_LOAD_BYTE(GS_patstucl[i], "Ser_ModsInStudy", tmp_cs, TBL_STRING, TBL_LIKE);
    			tok_cs = strtok(NULL, "\\");
    			if(tok_cs == NULL) free(tmp_cs);
    			i++;
    		}
    	}
	}
    if (i != 0) GS_patstucl[i].FieldName = 0;	/* Terminate List */

    return (i);
}

long
IDB_ConvertDicomQuerytoSQL(IDB_Query * pssi, long level)
{

    char       		*cs;
    int        		i, j;
    TBL_OPERATOR 	op;

    if (level == IDB_PATIENT_LEVEL){
    	i = 0;
    	if (pssi->PatientQFlag & QF_PAT_PatNam) {
    		if (!(pssi->PatientNullFlag & QF_PAT_PatNam)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatNam)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.PatNam);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "PatNam", cs, TBL_STRING, op);
    			/*
    			 * TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "PatNam", cs,
    			 * TBL_STRING, TBL_EQUAL);
    			 */
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_PatID) {
    		if (!(pssi->PatientNullFlag & QF_PAT_PatID)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatID)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.PatID);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "PatID", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_PatBirTim) {
    		if (!(pssi->PatientNullFlag & QF_PAT_PatBirTim)) {
    			UTL_SqueezeBlanks(pssi->patient.PatBirTim);
    			if (strchr(pssi->patient.PatBirTim, (int) '-') == (char *) NULL) {
    				GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat(pssi->patient.PatBirTim);
    				TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_EQUAL);
    			}else{
    				if (pssi->patient.PatBirTim[0] == '-') {
    					GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}else if (pssi->patient.PatBirTim[strlen(pssi->patient.PatBirTim) - 1] == '-'){
    					GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				}else{
    					GS_PAT_PatBirTimB = UTL_ConvertTimetoFloat((pssi->patient.PatBirTim));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirTim", GS_PAT_PatBirTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    					i++;
    					GS_PAT_PatBirTimE = UTL_ConvertTimetoFloat(strchr(pssi->patient.PatBirTim, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirTim", GS_PAT_PatBirTimE, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_PatBirDat) {
    		if (!(pssi->PatientNullFlag & QF_PAT_PatBirDat)) {
    			UTL_SqueezeBlanks(pssi->patient.PatBirDat);
    			if (strchr(pssi->patient.PatBirDat, (int) '-') == (char *) NULL) {
    				GS_PAT_PatBirDatB = UTL_ConvertDatetoLong(pssi->patient.PatBirDat);
    				TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_EQUAL);
    			}else{
    				if (pssi->patient.PatBirDat[0] == '-') {
    					GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}else if (pssi->patient.PatBirDat[strlen(pssi->patient.PatBirDat) - 1] == '-') {
    					GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				}else{
    					GS_PAT_PatBirDatB = UTL_ConvertDatetoLong((pssi->patient.PatBirDat));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirDat", GS_PAT_PatBirDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    					i++;
    					GS_PAT_PatBirDatE = UTL_ConvertDatetoLong(strchr(pssi->patient.PatBirDat, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "PatBirDat", GS_PAT_PatBirDatE, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_PatSex) {
    		if (!(pssi->PatientNullFlag & QF_PAT_PatSex)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.PatSex)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.PatSex);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "PatSex", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_InsertDate) {
    		if (!(pssi->PatientNullFlag & QF_PAT_InsertDate)) {
    			UTL_SqueezeBlanks(pssi->patient.InsertDate);
    			if (strchr(pssi->patient.InsertDate, (int) '-') == (char *) NULL) {
    				GS_PAT_InsertDateB = UTL_ConvertDatetoLong(pssi->patient.InsertDate);
    				TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
    			}else{
    				if (pssi->patient.InsertDate[0] == '-') {
    					GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}else if (pssi->patient.InsertDate[strlen(pssi->patient.InsertDate) - 1] == '-') {
    					GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				}else{
    					GS_PAT_InsertDateB = UTL_ConvertDatetoLong((pssi->patient.InsertDate));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertDate", GS_PAT_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    					i++;
    					GS_PAT_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->patient.InsertDate, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertDate", GS_PAT_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_InsertTime) {
    		if (!(pssi->PatientNullFlag & QF_PAT_InsertTime)) {
    			UTL_SqueezeBlanks(pssi->patient.InsertTime);
    			if (strchr(pssi->patient.InsertTime, (int) '-') == (char *) NULL) {
    				GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat(pssi->patient.InsertTime);
    				TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
    			}else{
    				if (pssi->patient.InsertTime[0] == '-') {
    					GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}else if (pssi->patient.InsertTime[strlen(pssi->patient.InsertTime) - 1] == '-') {
    					GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				}else{
    					GS_PAT_InsertTimeB = UTL_ConvertTimetoFloat((pssi->patient.InsertTime));
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertTime", GS_PAT_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    					i++;
    					GS_PAT_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->patient.InsertTime, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_patcl[i], "InsertTime", GS_PAT_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_Owner) {
    		if (!(pssi->PatientNullFlag & QF_PAT_Owner)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.Owner)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.Owner);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "Owner", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_GroupName) {
    		if (!(pssi->PatientNullFlag & QF_PAT_GroupName)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.GroupName)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.GroupName);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "GroupName", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->PatientQFlag & QF_PAT_Priv) {
    		if (!(pssi->PatientNullFlag & QF_PAT_Priv)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->patient.Priv)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_patcl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->patient.Priv);
    			TBL_CRITERIA_LOAD_BYTE(GS_patcl[i], "Priv", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (i != 0) GS_patcl[i].FieldName = 0;	/* Terminate List */

    }else if (level == IDB_STUDY_LEVEL){
    	i = 0;
    	if (pssi->StudyQFlag & QF_STU_StuDat) {
    		if (!(pssi->StudyNullFlag & QF_STU_StuDat)) {
    			UTL_SqueezeBlanks(pssi->study.StuDat);
    			if (strchr(pssi->study.StuDat, (int) '-') == (char *) NULL) {
    				GS_STU_StuDatB = UTL_ConvertDatetoLong(pssi->study.StuDat);
    				TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_EQUAL);
    			}else{
    				if (pssi->study.StuDat[0] == '-') {
    					GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}else if (pssi->study.StuDat[strlen(pssi->study.StuDat) - 1] == '-') {
    					GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				}else{
    					GS_STU_StuDatB = UTL_ConvertDatetoLong((pssi->study.StuDat));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuDat", GS_STU_StuDatB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    					i++;
    					GS_STU_StuDatE = UTL_ConvertDatetoLong(strchr(pssi->study.StuDat, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuDat", GS_STU_StuDatE, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_StuTim) {
    		if (!(pssi->StudyNullFlag & QF_STU_StuTim)) {
    			UTL_SqueezeBlanks(pssi->study.StuTim);
    			if (strchr(pssi->study.StuTim, (int) '-') == (char *) NULL) {
    				GS_STU_StuTimB = UTL_ConvertTimetoFloat(pssi->study.StuTim);
    				TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_EQUAL);
    			}else{
    				if (pssi->study.StuTim[0] == '-') {
    					GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}else if (pssi->study.StuTim[strlen(pssi->study.StuTim) - 1] == '-') {
    					GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				}else{
    					GS_STU_StuTimB = UTL_ConvertTimetoFloat((pssi->study.StuTim));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuTim", GS_STU_StuTimB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    					i++;
    					GS_STU_StuTimE = UTL_ConvertTimetoFloat(strchr(pssi->study.StuTim, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "StuTim", GS_STU_StuTimE, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_AccNum) {
    		if (!(pssi->StudyNullFlag & QF_STU_AccNum)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.AccNum)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.AccNum);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "AccNum", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_StuID) {
    		if (!(pssi->StudyNullFlag & QF_STU_StuID)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuID)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.StuID);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "StuID", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_StuInsUID) {
    		if (!(pssi->StudyNullFlag & QF_STU_StuInsUID)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuInsUID)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.StuInsUID);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "StuInsUID", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_RefPhyNam) {
    		if (!(pssi->StudyNullFlag & QF_STU_RefPhyNam)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.RefPhyNam)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.RefPhyNam);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "RefPhyNam", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_StuDes) {
    		if (!(pssi->StudyNullFlag & QF_STU_StuDes)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.StuDes)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.StuDes);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "StuDes", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_PatAge) {
    		if (!(pssi->StudyNullFlag & QF_STU_PatAge)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatAge)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.PatAge);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "PatAge", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_PatSiz) {
    		if (!(pssi->StudyNullFlag & QF_STU_PatSiz)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatSiz)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.PatSiz);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "PatSiz", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_PatWei) {
    		if (!(pssi->StudyNullFlag & QF_STU_PatWei)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.PatWei)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.PatWei);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "PatWei", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_InsertDate) {
    		if (!(pssi->StudyNullFlag & QF_STU_InsertDate)) {
    			UTL_SqueezeBlanks(pssi->study.InsertDate);
    			if (strchr(pssi->study.InsertDate, (int) '-') == (char *) NULL) {
    				GS_STU_InsertDateB = UTL_ConvertDatetoLong(pssi->study.InsertDate);
    				TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
    			}else{
    				if (pssi->study.InsertDate[0] == '-') {
    					GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}else if (pssi->study.InsertDate[strlen(pssi->study.InsertDate) - 1] == '-') {
    					GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    				}else{
    					GS_STU_InsertDateB = UTL_ConvertDatetoLong((pssi->study.InsertDate));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
    					i++;
    					GS_STU_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->study.InsertDate, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertDate", GS_STU_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_InsertTime) {
    		if (!(pssi->StudyNullFlag & QF_STU_InsertTime)) {
    			UTL_SqueezeBlanks(pssi->study.InsertTime);
    			if (strchr(pssi->study.InsertTime, (int) '-') == (char *) NULL) {
    				GS_STU_InsertTimeB = UTL_ConvertTimetoFloat(pssi->study.InsertTime);
    				TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
    			}else{
    				if (pssi->study.InsertTime[0] == '-') {
    					GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime) + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}else if (pssi->study.InsertTime[strlen(pssi->study.InsertTime) - 1] == '-') {
    					GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    				}else{
    					GS_STU_InsertTimeB = UTL_ConvertTimetoFloat((pssi->study.InsertTime));
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertTime", GS_STU_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
    					i++;
    					GS_STU_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->study.InsertTime, (int) '-') + 1);
    					TBL_CRITERIA_LOAD_NUM(GS_stucl[i], "InsertTime", GS_STU_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
    				}
    			}
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_Owner) {
    		if (!(pssi->StudyNullFlag & QF_STU_Owner)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.Owner)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.Owner);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "Owner", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_GroupName) {
    		if (!(pssi->StudyNullFlag & QF_STU_GroupName)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.GroupName)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.GroupName);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "GroupName", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (pssi->StudyQFlag & QF_STU_Priv) {
    		if (!(pssi->StudyNullFlag & QF_STU_Priv)) {
    			if ((cs = IDB_ConvertDicomMetatoSQL(pssi->study.Priv)) == (char *) NULL) {
    				FREE_STRINGS(j, GS_stucl, i);
    				return (0);
    			}
    			op = likeOrEqualOperator(pssi->study.Priv);
    			TBL_CRITERIA_LOAD_BYTE(GS_stucl[i], "Priv", cs, TBL_STRING, op);
    			i++;
    		}
    	}
    	if (i != 0) GS_stucl[i].FieldName = 0;	/* Terminate List */
	}else if (level == IDB_SERIES_LEVEL){
		i = 0;
		if (pssi->SeriesQFlag & QF_SER_Mod) {
			if (!(pssi->SeriesNullFlag & QF_SER_Mod)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.Mod)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.Mod);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "Mod", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_SerNum) {
			if (!(pssi->SeriesNullFlag & QF_SER_SerNum)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.SerNum)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.SerNum);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "SerNum", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_SerInsUID) {
			if (!(pssi->SeriesNullFlag & QF_SER_SerInsUID)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.SerInsUID)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.SerInsUID);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "SerInsUID", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_ProNam) {
			if (!(pssi->SeriesNullFlag & QF_SER_ProNam)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.ProNam)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.ProNam);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "ProNam", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_SerDes) {
			if (!(pssi->SeriesNullFlag & QF_SER_SerDes)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.SerDes)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.SerDes);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "SerDes", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_BodParExa) {
			if (!(pssi->SeriesNullFlag & QF_SER_BodParExa)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.BodParExa)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.BodParExa);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "BodParExa", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_ViePos) {
			if (!(pssi->SeriesNullFlag & QF_SER_ViePos)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.ViePos)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.ViePos);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "ViePos", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_InsertDate) {
			if (!(pssi->SeriesNullFlag & QF_SER_InsertDate)) {
				UTL_SqueezeBlanks(pssi->series.InsertDate);
				if (strchr(pssi->series.InsertDate, (int) '-') == (char *) NULL) {
					GS_SER_InsertDateB = UTL_ConvertDatetoLong(pssi->series.InsertDate);
					TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertDate", GS_SER_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
				}else{
					if (pssi->series.InsertDate[0] == '-') {
						GS_SER_InsertDateB = UTL_ConvertDatetoLong((pssi->series.InsertDate) + 1);
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertDate", GS_SER_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
					}else if (pssi->series.InsertDate[strlen(pssi->series.InsertDate) - 1] == '-') {
						GS_SER_InsertDateB = UTL_ConvertDatetoLong((pssi->series.InsertDate));
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertDate", GS_SER_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
					}else{
						GS_SER_InsertDateB = UTL_ConvertDatetoLong((pssi->series.InsertDate));
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
						i++;
						GS_SER_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->series.InsertDate, (int) '-') + 1);
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertDate", GS_SER_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
					}
				}
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_InsertTime) {
			if (!(pssi->SeriesNullFlag & QF_SER_InsertTime)) {
				UTL_SqueezeBlanks(pssi->series.InsertTime);
				if (strchr(pssi->series.InsertTime, (int) '-') == (char *) NULL) {
					GS_SER_InsertTimeB = UTL_ConvertTimetoFloat(pssi->series.InsertTime);
					TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertTime", GS_SER_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
				}else{
					if (pssi->series.InsertTime[0] == '-') {
						GS_SER_InsertTimeB = UTL_ConvertTimetoFloat((pssi->series.InsertTime) + 1);
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertTime", GS_SER_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
					}else if (pssi->series.InsertTime[strlen(pssi->series.InsertTime) - 1] == '-') {
						GS_SER_InsertTimeB = UTL_ConvertTimetoFloat((pssi->series.InsertTime));
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertTime", GS_SER_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
					}else{
						GS_SER_InsertTimeB = UTL_ConvertTimetoFloat((pssi->series.InsertTime));
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertTime", GS_SER_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
						i++;
						GS_SER_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->series.InsertTime, (int) '-') + 1);
						TBL_CRITERIA_LOAD_NUM(GS_sercl[i], "InsertTime", GS_SER_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
					}
				}
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_Owner) {
			if (!(pssi->SeriesNullFlag & QF_SER_Owner)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.Owner)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.Owner);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "Owner", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_GroupName) {
			if (!(pssi->SeriesNullFlag & QF_SER_GroupName)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.GroupName)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.GroupName);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "GroupName", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->SeriesQFlag & QF_SER_Priv) {
			if (!(pssi->SeriesNullFlag & QF_SER_Priv)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->series.Priv)) == (char *) NULL) {
					FREE_STRINGS(j, GS_sercl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->series.Priv);
				TBL_CRITERIA_LOAD_BYTE(GS_sercl[i], "Priv", cs, TBL_STRING, op);
				i++;
			}
		}
		if (i != 0) GS_sercl[i].FieldName = 0;	/* Terminate List */
	} else if (level == IDB_IMAGE_LEVEL) {
		i = 0;
		if (pssi->ImageQFlag & QF_IMA_ImaNum) {
			if (!(pssi->ImageNullFlag & QF_IMA_ImaNum)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.ImaNum)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.ImaNum);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "ImaNum", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_SOPInsUID) {
			if (!(pssi->ImageNullFlag & QF_IMA_SOPInsUID)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.SOPInsUID)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.SOPInsUID);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "SOPInsUID", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_SOPClaUID) {
			if (!(pssi->ImageNullFlag & QF_IMA_SOPClaUID)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.SOPClaUID)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.SOPClaUID);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "SOPClaUID", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_PhoInt) {
			if (!(pssi->ImageNullFlag & QF_IMA_PhoInt)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.PhoInt)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.PhoInt);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "PhoInt", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_PatOri) {
			if (!(pssi->ImageNullFlag & QF_IMA_PatOri)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.PatOri)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.PatOri);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "PatOri", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_SamPerPix) {
			if (!(pssi->ImageNullFlag & QF_IMA_SamPerPix)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "SamPerPix", GS_IMA_SamPerPix, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_SamPerPix = pssi->image.SamPerPix;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_Row) {
			if (!(pssi->ImageNullFlag & QF_IMA_Row)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "Row", GS_IMA_Row, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_Row = pssi->image.Row;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_Col) {
			if (!(pssi->ImageNullFlag & QF_IMA_Col)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "Col", GS_IMA_Col, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_Col = pssi->image.Col;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_BitAll) {
			if (!(pssi->ImageNullFlag & QF_IMA_BitAll)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "BitAll", GS_IMA_BitAll, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_BitAll = pssi->image.BitAll;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_BitSto) {
			if (!(pssi->ImageNullFlag & QF_IMA_BitSto)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "BitSto", GS_IMA_BitSto, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_BitSto = pssi->image.BitSto;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_PixRep) {
			if (!(pssi->ImageNullFlag & QF_IMA_PixRep)) {
				TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "PixRep", GS_IMA_PixRep, TBL_SIGNED4, TBL_EQUAL);
				GS_IMA_SamPerPix = pssi->image.SamPerPix;
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_InsertDate) {
			if (!(pssi->ImageNullFlag & QF_IMA_InsertDate)) {
				UTL_SqueezeBlanks(pssi->image.InsertDate);
				if (strchr(pssi->image.InsertDate, (int) '-') == (char *) NULL) {
					GS_IMA_InsertDateB = UTL_ConvertDatetoLong(pssi->image.InsertDate);
					TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertDate", GS_IMA_InsertDateB, TBL_SIGNED4, TBL_EQUAL);
				}else{
					if (pssi->image.InsertDate[0] == '-') {
						GS_IMA_InsertDateB = UTL_ConvertDatetoLong((pssi->image.InsertDate) + 1);
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertDate", GS_IMA_InsertDateB, TBL_SIGNED4, TBL_LESS_EQUAL);
					}else if (pssi->image.InsertDate[strlen(pssi->image.InsertDate) - 1] == '-') {
						GS_IMA_InsertDateB = UTL_ConvertDatetoLong((pssi->image.InsertDate));
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertDate", GS_IMA_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
					}else{
						GS_IMA_InsertDateB = UTL_ConvertDatetoLong((pssi->image.InsertDate));
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertDate", GS_STU_InsertDateB, TBL_SIGNED4, TBL_GREATER_EQUAL);
						i++;
						GS_IMA_InsertDateE = UTL_ConvertDatetoLong(strchr(pssi->image.InsertDate, (int) '-') + 1);
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertDate", GS_IMA_InsertDateE, TBL_SIGNED4, TBL_LESS_EQUAL);
					}
				}
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_InsertTime) {
			if (!(pssi->ImageNullFlag & QF_IMA_InsertTime)) {
				UTL_SqueezeBlanks(pssi->image.InsertTime);
				if (strchr(pssi->image.InsertTime, (int) '-') == (char *) NULL) {
					GS_IMA_InsertTimeB = UTL_ConvertTimetoFloat(pssi->image.InsertTime);
					TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertTime", GS_IMA_InsertTimeB, TBL_FLOAT4, TBL_EQUAL);
				}else{
					if (pssi->image.InsertTime[0] == '-') {
						GS_IMA_InsertTimeB = UTL_ConvertTimetoFloat((pssi->image.InsertTime) + 1);
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertTime", GS_IMA_InsertTimeB, TBL_FLOAT4, TBL_LESS_EQUAL);
					}else if (pssi->image.InsertTime[strlen(pssi->image.InsertTime) - 1] == '-') {
						GS_IMA_InsertTimeB = UTL_ConvertTimetoFloat((pssi->image.InsertTime));
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertTime", GS_IMA_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
					}else{
						GS_IMA_InsertTimeB = UTL_ConvertTimetoFloat((pssi->image.InsertTime));
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertTime", GS_IMA_InsertTimeB, TBL_FLOAT4, TBL_GREATER_EQUAL);
						i++;
						GS_IMA_InsertTimeE = UTL_ConvertTimetoFloat(strchr(pssi->image.InsertTime, (int) '-') + 1);
						TBL_CRITERIA_LOAD_NUM(GS_imacl[i], "InsertTime", GS_IMA_InsertTimeE, TBL_FLOAT4, TBL_LESS_EQUAL);
					}
				}
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_Owner) {
			if (!(pssi->ImageNullFlag & QF_IMA_Owner)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.Owner)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.Owner);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "Owner", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_GroupName) {
			if (!(pssi->ImageNullFlag & QF_IMA_GroupName)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.GroupName)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.GroupName);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "GroupName", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_Priv) {
			if (!(pssi->ImageNullFlag & QF_IMA_Priv)) {
				if ((cs = IDB_ConvertDicomMetatoSQL(pssi->image.Priv)) == (char *) NULL) {
					FREE_STRINGS(j, GS_imacl, i);
					return (0);
				}
				op = likeOrEqualOperator(pssi->image.Priv);
				TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "Priv", cs, TBL_STRING, op);
				i++;
			}
		}
		if (pssi->ImageQFlag & QF_IMA_SOPInsUIDList) {
			LST_NODE		* temp;
			int		        count;

			strcpy(GS_IMASEL_SOPInsUIDList, "( SOPInsUID in ( ");
			count = LST_Count(&(pssi->image.ImageUIDList));

			temp = LST_Head(&(pssi->image.ImageUIDList));
			LST_Position(&(pssi->image.ImageUIDList), temp);

			while (count != 0) {
				strcat(GS_IMASEL_SOPInsUIDList, "\"");
				strcat(GS_IMASEL_SOPInsUIDList, "temp->UID");
				strcat(GS_IMASEL_SOPInsUIDList, "\"");
				count--;
				if (count) strcat(GS_IMASEL_SOPInsUIDList, ",");
				temp = LST_Next(&(pssi->image.ImageUIDList));
			}

			strcat(GS_IMASEL_SOPInsUIDList, " )) ");

	    /*
	     * This is a faked up Name...there really is no field name of
	     * SOPInsUIDList, but the type of TBL_NOP doesn't use it
	     * anyway....it just plops the string passed into the where
	     * clause of the sybase statement...
	     */
	    TBL_CRITERIA_LOAD_BYTE(GS_imacl[i], "SOPInsUIDList", GS_IMASEL_SOPInsUIDList, TBL_STRING, TBL_NOP);
	    i++;
		}
		if (i != 0) GS_imacl[i].FieldName = 0;	/* Terminate List */
    }else{
    	return (0);
    }
	return (i);
}
/* IDB_ConvertDicomMetatoSQL
**
** Purpose:
**	This function converts a DICOM "regular expression" to the proper
**	meta characters sequences for use under SQL.
**
** Parameter Dictionary:
**	char *regex:
**		The DICOM regular expression to convert.
**
** Return Values:
**	char *:	The converted regular expression which expresses DICOM pattern
**		matching in SQL semantics.
**
** Algorithm:
**	Simple function to convert a DICOM "regular expression" to the proper
**	regex semantics under SQL.  DICOM has only 2 meta characters, "*" for 0
**	or more occurrences, and "?" for a single character.  The "*" must be
**	converted to "%" for SQL while the "?" must be converted to "_".
**	Other special characters to SQL like "[", and "^" must also
**	be escaped.
*/
char *
IDB_ConvertDicomMetatoSQL(char *regex)
{

    char       *new_regex = (char *) NULL;
    int        malloced_size = 0;
    int        i, j;

    if (new_regex == (char *) NULL) {
    	malloced_size = 128;
    	if ((new_regex = (char *) malloc(malloced_size)) == (char *) NULL) return ((char *) NULL);
    }
    i = j = 0;
    while (regex[i] != '\000') {
    	switch (regex[i]) {
			case '*':		/* Transform the "*" to "%" */
							new_regex[j++] = '%';
							break;
			case '?':		/* Transform the "?" to "_" */
							new_regex[j++] = '_';
							break;
			case '%':		/* Change to [%] */
							new_regex[j++] = '[';
							new_regex[j++] = '%';
							new_regex[j++] = ']';
							break;
			case '_':		/* Change to [_] */
							new_regex[j++] = '[';
							new_regex[j++] = '_';
							new_regex[j++] = ']';
							break;
			case '[':		/* Change to [[] */
							new_regex[j++] = '[';
							new_regex[j++] = '[';
							new_regex[j++] = ']';
							break;
#ifdef TBL_REQUIRES_HAT_ESCAPE
			case '^':		/* Change to */
							new_regex[j++] = '\\';
							new_regex[j++] = '\\';
							new_regex[j++] = '^';
							break;
#endif
			default:		/* Just copy the character */
							new_regex[j++] = regex[i];
							break;
    	}
    	if (j >= (malloced_size - 4)) {
    		malloced_size += 128;
    		if ((new_regex = (char *) realloc(new_regex, malloced_size)) == (char *) NULL) return ((char *) NULL);
    	}
    	i++;
    }
    new_regex[j] = '\000';
    return (new_regex);
}


/* IDB_InsertImage
**
** Purpose:
**	This routine inserts records into the database
**
** Parameter Dictionary:
**	IDB_HANDLE **handle:
**		the database identifier.
**	IDB_Insertion *pssi:
**		The structure that contains the new record to be
**		inserted into the database.
**
** Return Values:
**	IDB_NORMAL: 	The insert command succeeded
**	IDB_NOINSERTDATA: The pssi pointer was null, no data was
**		provided to insert.
**	IDB_BADPATUID: The patient UID occurs multiple times in the db
**	IDB_BADSTUUID: The study UID occurs multiple times in the db
**	IDB_BADSERUID: The study UID occurs multiple times in the db
**	IDB_BADIMAUID: The image UID occurs multiple times in the db
**	IDB_DUPINSTANCE: The image file of an old instance could not
**		be deleted, and this warning is returned.
**
** Algorithm:
**	The insertion algorithm first checks to determine if any of
**	the uid's passed in pssi are contained in the database.  If
**	so, then these Levels need not be replaced...simply updated
**	with new counts for the number of decendents.  If multiple
**	records with that UID exist, a database integrity problem
**	exists. This routine generates the appropriate error and the
**	insertion is aborted.
**
*/
CONDITION
IDB_InsertImage(IDB_HANDLE ** handle, IDB_Insertion * pssi)
{
    TBL_FIELD		patfld[20],	stufld[19],	serfld[15],	imafld[19],	insfld[7], field[5];
    char 			nullString[] = "";
    TBL_CRITERIA	crit[5];
    TBL_UPDATE		update[2];
    TBL_HANDLE		* pathandle, *stuhandle, *serhandle, *imahandle, *inshandle;
#ifdef CTN_IDBV2
    TBL_HANDLE 		*limits;
#endif
    char	        temp_date[DICOM_DA_LENGTH + 1], temp_time[DICOM_TM_LENGTH + 1], path_buffer[256], buf[DICOM_LO_LENGTH + 1];
    IDB_CONTEXT		* idbc;
    long	        NeedPatientInsert, NeedStudyInsert, NeedSeriesInsert, NeedImageInsert, InstanceNotDeleted, count, foundit;
    long	        PAT_PatBirDat, PAT_NumPatRelStu, PAT_NumPatRelSer, PAT_NumPatRelIma, PAT_InsertDate, STU_StuDat, STU_NumStuRelSer, STU_NumStuRelIma,
					STU_InsertDate, SER_NumSerRelIma, SER_InsertDate, IMA_InsertDate;
    float	        PAT_PatBirTim, PAT_InsertTime, STU_StuTim, STU_InsertTime, SER_InsertTime, IMA_InsertTime;
    CONDITION		ret_valb, ret_val, cond;
    int		        i;

    THR_ObtainMutex(FAC_IDB);

    idbc = GS_ContextHead;
    foundit = 0;
    while (idbc != (IDB_CONTEXT *) NULL) {
    	if (idbc == (IDB_CONTEXT *) (*handle)) {
    		pathandle = idbc->PatNodes;
    		stuhandle = idbc->StuNodes;
    		serhandle = idbc->SerNodes;
    		imahandle = idbc->ImaNodes;
    		inshandle = idbc->InsNodes;
#ifdef CTN_IDBV2
    		limits = idbc->Limits;
#endif
    		foundit = 1;
    	}
    	idbc = idbc->next;
    }
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_InsertImage");
    }
    if (pssi == (IDB_Insertion *) NULL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_NOINSERTDATA), "IDB_InsertImage");
    }

    cond = TBL_SetEncoding(&pathandle, pssi->image.CharSet);
    if (cond != TBL_NORMAL) fprintf(stderr, "PatHandle - Unable to set Character set %s \n" ,pssi->image.CharSet);
    cond = TBL_SetEncoding(&stuhandle, pssi->image.CharSet);
    if (cond != TBL_NORMAL) fprintf(stderr, "StuHandle - Unable to set Character set %s \n" ,pssi->image.CharSet);
    cond = TBL_SetEncoding(&serhandle, pssi->image.CharSet);
    if (cond != TBL_NORMAL) fprintf(stderr, "SerHandle - Unable to set Character set %s \n" ,pssi->image.CharSet);
    cond = TBL_SetEncoding(&imahandle, pssi->image.CharSet);
    if (cond != TBL_NORMAL) fprintf(stderr, "ImaHandle - Unable to set Character set %s \n" ,pssi->image.CharSet);


    NeedPatientInsert = NeedStudyInsert = NeedSeriesInsert = NeedImageInsert = 0;
    field[0].FieldName = "PatID";
    field[0].Value.AllocatedSize = DICOM_LO_LENGTH + 1;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = buf;
    field[1].FieldName = 0;

    crit[0].FieldName = "PatID";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = pssi->patient.PatID;
    crit[0].Value.Type = TBL_STRING;
    crit[1].FieldName = 0;
    count = 0;

    ret_val = TBL_Select(&pathandle, crit, field, &count, NULL, NULL);
    if (count > 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADPATUID), buf, "IDB_InsertImage");
    }else if (count < 1){
    	/* Need to insert this node into the patient level... */
    	TBL_FIELD_LOAD_BYTE(patfld[0], "PatNam", pssi->patient.PatNam, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(patfld[1], "PatID", pssi->patient.PatID, TBL_STRING);
    	TBL_FIELD_LOAD_NUM(patfld[2], "PatBirDat", PAT_PatBirDat, TBL_SIGNED4);
    	PAT_PatBirDat = UTL_ConvertDatetoLong(pssi->patient.PatBirDat);
    	TBL_FIELD_LOAD_NUM(patfld[3], "PatBirTim", PAT_PatBirTim, TBL_FLOAT4);
    	PAT_PatBirTim = UTL_ConvertTimetoFloat(pssi->patient.PatBirTim);

    	TBL_FIELD_LOAD_BYTE(patfld[4], "PatSex", pssi->patient.PatSex, TBL_STRING);

    	TBL_FIELD_LOAD_NUM(patfld[5], "NumPatRelStu", PAT_NumPatRelStu, TBL_SIGNED4);
    	PAT_NumPatRelStu = 0;
    	TBL_FIELD_LOAD_NUM(patfld[6], "NumPatRelSer", PAT_NumPatRelSer, TBL_SIGNED4);
    	PAT_NumPatRelSer = 0;
    	TBL_FIELD_LOAD_NUM(patfld[7], "NumPatRelIma", PAT_NumPatRelIma, TBL_SIGNED4);
    	PAT_NumPatRelIma = 0;

    	TBL_FIELD_LOAD_NUM(patfld[8], "InsertDate", PAT_InsertDate, TBL_SIGNED4);
    	UTL_GetDicomDate(temp_date);
    	PAT_InsertDate = UTL_ConvertDatetoLong(temp_date);

    	TBL_FIELD_LOAD_NUM(patfld[9], "InsertTime", PAT_InsertTime, TBL_FLOAT4);
    	UTL_GetDicomTime(temp_time);
    	PAT_InsertTime = UTL_ConvertTimetoFloat(temp_time);

    	TBL_FIELD_LOAD_BYTE(patfld[10], "Owner", pssi->patient.Owner, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(patfld[11], "GroupName", pssi->patient.GroupName, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(patfld[12], "Priv", pssi->patient.Priv, TBL_STRING);

    	patfld[13].FieldName = 0;	/* Terminate the list */

    	NeedPatientInsert = 1;
    }

    field[0].FieldName = "PatParent";
    field[0].Value.AllocatedSize = DICOM_LO_LENGTH + 1;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = buf;
    field[1].FieldName = 0;

    crit[0].FieldName = "StuInsUID";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = pssi->study.StuInsUID;
    crit[0].Value.Type = TBL_STRING;
    crit[1].FieldName = 0;
    count = 0;

    ret_val = TBL_Select(&stuhandle, crit, field, &count, NULL, NULL);
    if (count > 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADSTUUID), buf, "IDB_InsertImage");
    }else if (count == 1) {
    	if (strcmp(buf, pssi->patient.PatID) != 0) {
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_PATIDMISMATCH), pssi->study.StuInsUID, pssi->patient.PatID, buf, "IDB_InsertImage");
    	}
    }else{			/* Need Study Insertion */
    	TBL_FIELD_LOAD_NUM(stufld[0], "StuDat", STU_StuDat, TBL_SIGNED4);
    	STU_StuDat = UTL_ConvertDatetoLong(pssi->study.StuDat);
    	TBL_FIELD_LOAD_NUM(stufld[1], "StuTim", STU_StuTim, TBL_FLOAT4);
    	STU_StuTim = UTL_ConvertTimetoFloat(pssi->study.StuTim);
    	TBL_FIELD_LOAD_BYTE(stufld[2], "AccNum", pssi->study.AccNum, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[3], "StuID", pssi->study.StuID, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[4], "StuInsUID", pssi->study.StuInsUID, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[5], "RefPhyNam", pssi->study.RefPhyNam, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[6], "StuDes", pssi->study.StuDes, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[7], "PatAge", pssi->study.PatAge, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[8], "PatSiz", pssi->study.PatSiz, TBL_STRING);
    	TBL_FIELD_LOAD_BYTE(stufld[9], "PatWei", pssi->study.PatWei, TBL_STRING);

    	TBL_FIELD_LOAD_NUM(stufld[10], "NumStuRelSer", STU_NumStuRelSer, TBL_SIGNED4);
		STU_NumStuRelSer = 0;
		TBL_FIELD_LOAD_NUM(stufld[11], "NumStuRelIma", STU_NumStuRelIma, TBL_SIGNED4);
		STU_NumStuRelIma = 0;

		TBL_FIELD_LOAD_NUM(stufld[12], "InsertDate", STU_InsertDate, TBL_SIGNED4);
		UTL_GetDicomDate(temp_date);
		STU_InsertDate = UTL_ConvertDatetoLong(temp_date);

		TBL_FIELD_LOAD_NUM(stufld[13], "InsertTime", STU_InsertTime, TBL_FLOAT4);
		UTL_GetDicomTime(temp_time);
		STU_InsertTime = UTL_ConvertTimetoFloat(temp_time);

		TBL_FIELD_LOAD_BYTE(stufld[14], "Owner", pssi->study.Owner, TBL_STRING);
		TBL_FIELD_LOAD_BYTE(stufld[15], "GroupName", pssi->study.GroupName, TBL_STRING);
		TBL_FIELD_LOAD_BYTE(stufld[16], "Priv", pssi->study.Priv, TBL_STRING);

		TBL_FIELD_LOAD_BYTE(stufld[17], "PatParent", pssi->patient.PatID, TBL_STRING);

		stufld[18].FieldName = 0;	/* Terminate the list */

		NeedStudyInsert = 1;
    }

    field[0].FieldName = "StuParent";
    field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = buf;
    field[1].FieldName = 0;

    crit[0].FieldName = "SerInsUID";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = pssi->series.SerInsUID;
    crit[0].Value.Type = TBL_STRING;
    crit[1].FieldName = 0;
    count = 0;

    ret_val = TBL_Select(&serhandle, crit, field, &count, NULL, NULL);
    if (count > 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADSERUID), buf, "IDB_InsertImage");
    }else if (count == 1) {
    	if (strcmp(buf, pssi->study.StuInsUID) != 0) {
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADSERUID), buf, "IDB_InsertImage");
    	}
    }else{			/* Need Series Insertion */
    	i = 0;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "Mod", pssi->series.Mod, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "SerNum", pssi->series.SerNum, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "SerInsUID", pssi->series.SerInsUID, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "ProNam", pssi->series.ProNam, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "SerDes", pssi->series.SerDes, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "BodParExa", pssi->series.BodParExa, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "ViePos", pssi->series.ViePos, TBL_STRING);

    	i++;
    	TBL_FIELD_LOAD_NUM(serfld[i], "NumSerRelIma", SER_NumSerRelIma, TBL_SIGNED4);
    	SER_NumSerRelIma = 0;

    	i++;
    	TBL_FIELD_LOAD_NUM(serfld[i], "InsertDate", SER_InsertDate, TBL_SIGNED4);
    	UTL_GetDicomDate(temp_date);
    	SER_InsertDate = UTL_ConvertDatetoLong(temp_date);

    	i++;
    	TBL_FIELD_LOAD_NUM(serfld[i], "InsertTime", SER_InsertTime, TBL_FLOAT4);
    	UTL_GetDicomTime(temp_time);
    	SER_InsertTime = UTL_ConvertTimetoFloat(temp_time);

    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "Owner", pssi->series.Owner, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "GroupName", pssi->series.GroupName, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "Priv", pssi->series.Priv, TBL_STRING);

    	i++;
    	TBL_FIELD_LOAD_BYTE(serfld[i], "StuParent", pssi->study.StuInsUID, TBL_STRING);

    	i++;
    	serfld[i].FieldName = 0;/* Terminate the list */

    	NeedSeriesInsert = 1;
    }

    field[0].FieldName = "SerParent";
    field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = buf;
    field[1].FieldName = 0;

    crit[0].FieldName = "SOPInsUID";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = pssi->image.SOPInsUID;
    crit[0].Value.Type = TBL_STRING;
    crit[1].FieldName = 0;
    count = 0;

    ret_val = TBL_Select(&imahandle, crit, field, &count, NULL, NULL);
    if (count > 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADIMAUID), buf, "IDB_InsertImage");
    }else if (count == 1) {
    	if (strcmp(buf, pssi->series.SerInsUID) != 0) {
    		THR_ReleaseMutex(FAC_IDB);
    		return COND_PushCondition(IDB_ERROR(IDB_BADIMAUID), buf, "IDB_InsertImage");
    	}
    }else{			/* Need Image Insertion */
    	i = 0;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "ImaNum", pssi->image.ImaNum, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "SOPInsUID", pssi->image.SOPInsUID, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "SOPClaUID", pssi->image.SOPClaUID, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "PhoInt", pssi->image.PhoInt, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "PatOri", pssi->image.PatOri, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "SamPerPix", (pssi->image.SamPerPix), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "Row", (pssi->image.Row), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "Col", (pssi->image.Col), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "BitAll", (pssi->image.BitAll), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "BitSto", (pssi->image.BitSto), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "PixRep", (pssi->image.PixRep), TBL_SIGNED4);
    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "InsertDate", IMA_InsertDate, TBL_SIGNED4);
    	UTL_GetDicomDate(temp_date);
    	IMA_InsertDate = UTL_ConvertDatetoLong(temp_date);

    	i++;
    	TBL_FIELD_LOAD_NUM(imafld[i], "InsertTime", IMA_InsertTime, TBL_FLOAT4);
    	UTL_GetDicomTime(temp_time);
    	IMA_InsertTime = UTL_ConvertTimetoFloat(temp_time);

    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "CharSet", pssi->image.CharSet, TBL_STRING);

    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "Owner", pssi->image.Owner, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "GroupName", pssi->image.GroupName, TBL_STRING);
    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "Priv", pssi->image.Priv, TBL_STRING);

    	i++;
    	TBL_FIELD_LOAD_BYTE(imafld[i], "SerParent", pssi->series.SerInsUID, TBL_STRING);

    	i++;
    	imafld[i].FieldName = 0;/* Terminate the list */

    	replaceBadFields(imafld, nullString);

    	NeedImageInsert = 1;
    }
    /*
     * We need to try and delete the duplicate instances here....(along with
     * the image files)
     */
    InstanceNotDeleted = 0;
    field[0].FieldName = "Path";
    field[0].Value.AllocatedSize = 256;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = path_buffer;
    field[1].FieldName = 0;

    crit[0].FieldName = "Medium";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = pssi->image.Medium;
    crit[0].Value.Type = TBL_STRING;

    crit[1].FieldName = "Transfer";
    crit[1].Operator = TBL_EQUAL;
    crit[1].Value.Value.String = pssi->image.Transfer;
    crit[1].Value.Type = TBL_STRING;

    crit[2].FieldName = "ImageUID";
    crit[2].Operator = TBL_EQUAL;
    crit[2].Value.Value.String = pssi->image.SOPInsUID;
    crit[2].Value.Type = TBL_STRING;

    crit[3].FieldName = 0;
    count = 0;
    GS_instances = LST_Create();

    ret_val = TBL_Select(&inshandle, crit, field, &count, CBIns_CollectInstances, NULL);
    if (count != 0) {
    	char 		*tc;
    	int 		i;

    	LST_NODE *temp_node;
    	crit[0].FieldName = "Transfer";
    	crit[0].Operator = TBL_EQUAL;
    	crit[0].Value.Type = TBL_STRING;
    	crit[0].Value.Value.String = pssi->image.Transfer;

    	crit[1].FieldName = "Medium";
    	crit[1].Operator = TBL_EQUAL;
    	crit[1].Value.Type = TBL_STRING;
    	crit[1].Value.Value.String = pssi->image.Medium;

    	crit[2].FieldName = "Path";
    	crit[2].Operator = TBL_EQUAL;
    	crit[2].Value.Type = TBL_STRING;
    	/*	File in pointer to value in loop below */

    	crit[3].FieldName = "ImageUID";
    	crit[3].Operator = TBL_EQUAL;
    	crit[3].Value.Type = TBL_STRING;
    	crit[3].Value.Value.String = pssi->image.SOPInsUID;

    	crit[4].FieldName = 0;

    	if (LST_Count(&GS_instances) != 0) {
    		for (i = 0; i < count; i++) {
    			temp_node = LST_Dequeue(&GS_instances);
    			tc = (char *) temp_node + (2 * sizeof(void *));
    			if (unlink(tc) == 0) {
    				crit[2].Value.Value.String = tc;
    				TBL_Delete(&inshandle, crit);
    			}else{
    				InstanceNotDeleted = 1;
    			}
    			free(temp_node);
    		}
    	}
	}
    LST_Destroy(&GS_instances);

    /*
     * We always need to do the instance insertion....so do the right thing
     * here...
     */
    TBL_FIELD_LOAD_BYTE(insfld[0], "ImageUID", pssi->image.SOPInsUID, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[1], "RespondingTitle", pssi->image.RespondingTitle, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[2], "Medium", pssi->image.Medium, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[3], "Path", pssi->image.Path, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[4], "Transfer", pssi->image.Transfer, TBL_STRING);
    TBL_FIELD_LOAD_NUM(insfld[5],  "Size", (pssi->image.Size), TBL_SIGNED4);

    insfld[6].FieldName = 0;	/* Terminate the list */

    /* BEGIN THE INSERT TRANSACTION--IF WE NEED TOO... */
    if (NeedPatientInsert || NeedStudyInsert || NeedSeriesInsert || NeedPatientInsert) TBL_BeginInsertTransaction();
    ret_val = ret_valb = TBL_NORMAL;

    if (NeedPatientInsert) {
    	ret_val = TBL_Insert(&pathandle, patfld);
    	if (ret_val != TBL_NORMAL) COND_DumpConditions();
    }
    if (ret_val == TBL_NORMAL){
    	if (NeedStudyInsert) {
    		ret_val = TBL_Insert(&stuhandle, stufld);
    		if (ret_val != TBL_NORMAL) COND_DumpConditions();
    	}
    	if (ret_val == TBL_NORMAL) {
    		if (NeedSeriesInsert) {
    			ret_val = TBL_Insert(&serhandle, serfld);
    			if (ret_val != TBL_NORMAL) COND_DumpConditions();
    		}
    		if (ret_val == TBL_NORMAL) {
    			if (NeedImageInsert) ret_val = TBL_Insert(&imahandle, imafld);
    			if (ret_val != TBL_NORMAL) COND_DumpConditions();

    			ret_valb = TBL_Insert(&inshandle, insfld);
    			if (ret_valb != TBL_NORMAL)	COND_DumpConditions();

    			if ((ret_val != TBL_NORMAL) || (ret_valb != TBL_NORMAL)) {
    				/* rollback the transaction */
    				TBL_RollbackInsertTransaction();
    			}
    		}else{
    			/* rollback the transaction */
    			TBL_RollbackInsertTransaction();
    		}
    	}else{
    		/* rollback the transaction */
    		TBL_RollbackInsertTransaction();
    	}
    } else {
    	/* rollback the transaction */
    	TBL_RollbackInsertTransaction();
    }

    if ((ret_val == TBL_NORMAL) && (ret_valb == TBL_NORMAL)) {
    	/* COMMIT the INSERT TRANSACTION--IF WE IN FACT STARTED ONE... */
    	if (NeedPatientInsert || NeedStudyInsert || NeedSeriesInsert || NeedPatientInsert) TBL_CommitInsertTransaction();

    	/* Update the counts in each of patient, study, series, and image */
    	if (NeedStudyInsert) { /* StudyInsert */
    		crit[0].FieldName = "PatID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->patient.PatID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumPatRelStu";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;
    		if (TBL_HasUpdateIncrement()){
    			ret_val = TBL_Update(&pathandle, crit, update);
    		}else{
    			ret_val = localIncrement(&pathandle, crit, update);
    		}
    	}
    	if (NeedSeriesInsert && ret_val == TBL_NORMAL) { /* StudyInsert */
    		crit[0].FieldName = "PatID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->patient.PatID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumPatRelSer";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;

    		if (TBL_HasUpdateIncrement()){
    			ret_val = TBL_Update(&pathandle, crit, update);
    		}else{
    			ret_val = localIncrement(&pathandle, crit, update);
    		}
    		crit[0].FieldName = "StuInsUID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->study.StuInsUID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumStuRelSer";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;

    		if (ret_val == TBL_NORMAL) {
    			if (TBL_HasUpdateIncrement()){
    				ret_val = TBL_Update(&stuhandle, crit, update);
    			}else{
    				ret_val = localIncrement(&stuhandle, crit, update);
    			}
    		}
    	}
    	if (NeedImageInsert && ret_val == TBL_NORMAL) { /* ImageInsert */
    		crit[0].FieldName = "PatID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->patient.PatID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumPatRelIma";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;

    		if (TBL_HasUpdateIncrement()){
    			ret_val = TBL_Update(&pathandle, crit, update);
    		}else{
    			ret_val = localIncrement(&pathandle, crit, update);
    		}
    		crit[0].FieldName = "StuInsUID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->study.StuInsUID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumStuRelIma";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;

    		if (ret_val == TBL_NORMAL) {
    			if (TBL_HasUpdateIncrement()){
    				ret_val = TBL_Update(&stuhandle, crit, update);
    			}else{
    				ret_val = localIncrement(&stuhandle, crit, update);
    			}
    		}
    		crit[0].FieldName = "SerInsUID";
    		crit[0].Operator = TBL_EQUAL;
    		crit[0].Value.Type = TBL_STRING;
    		crit[0].Value.Value.String = pssi->series.SerInsUID;
    		crit[1].FieldName = 0;

    		update[0].FieldName = "NumSerRelIma";
    		update[0].Function = TBL_INCREMENT;

    		update[1].FieldName = 0;

    		if (ret_val == TBL_NORMAL) {
    			if (TBL_HasUpdateIncrement()){
    				ret_val = TBL_Update(&serhandle, crit, update);
    			}else{
    				ret_val = localIncrement(&serhandle, crit, update);
    			}
    		}
    	}
#ifdef CTN_IDBV2
    	if ((ret_val == TBL_NORMAL) && (ret_valb == TBL_NORMAL)) {
    		updateLimitsTable(limits, (NeedImageInsert == 1) ? pssi->image.Size / SCALING_FACTOR : 0, (NeedPatientInsert == 1) ? 1 : 0,
									  (NeedStudyInsert == 1) ? 1 : 0, (NeedImageInsert == 1) ? 1 : 0);
    	}
#endif
    }
    if (ret_val != TBL_NORMAL || ret_valb != TBL_NORMAL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_INSERTFAILED), "Image", "IDB_InsertImage");
    }
    if (InstanceNotDeleted) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_DUPINSTANCE), "IDB_InsertImage");
    }else{
    	THR_ReleaseMutex(FAC_IDB);
    	return IDB_NORMAL;
    }
}

CONDITION
CBIns_CollectInstances(TBL_FIELD * field, int count, void *ctx)
{
    LST_NODE 		* temp_node;
    char	    	*temp_string;

    temp_node = (LST_NODE *) malloc((2 * sizeof(void *)) + strlen(field->Value.Value.String) + 1);
    if (temp_node == (LST_NODE *) NULL) return ~TBL_NORMAL;

    temp_string = (char *) temp_node + (2 * sizeof(void *));
    strcpy(temp_string, field->Value.Value.String);
    LST_Enqueue(&GS_instances, temp_node);
    return TBL_NORMAL;
}


/* IDB_InsertInstance
**
** Purpose:
**	This routine inserts an image instance record into the database
**
** Parameter Dictionary:
**	IDB_HANDLE **handle:
**		the database identifier.
**	char *imageuid:
**		The image uid for which the instances will be inserted.
**	IDB_InstanceListElement *iie:
**		The instance to be inserted.
**
** Return Values:
**	IDB_NORMAL: 	The insert command succeeded
**	IDB_BADHANDLE:	The handle passed is invalid
**	IDB_BADIMAUID: The image UID occurs multiple times in the db
**
** Algorithm:
**	The image uid passed must exist.  The routine then inserts the
**	instance into the image instance table.
**
*/
CONDITION
IDB_InsertImageInstance(IDB_HANDLE ** handle, char *imageuid, IDB_InstanceListElement * iie)
{

    IDB_CONTEXT		* idbc;
    TBL_HANDLE		* pathandle, *stuhandle, *serhandle, *imahandle, *inshandle;
    TBL_FIELD		insfld[7], field[2];
    TBL_CRITERIA	crit[2];
    char	        buf[DICOM_LO_LENGTH + 1];
    CONDITION		ret_val;
    long	        count, foundit;

    THR_ObtainMutex(FAC_IDB);

    idbc = GS_ContextHead;
    foundit = 0;
    while (idbc != (IDB_CONTEXT *) NULL) {
    	if (idbc == (IDB_CONTEXT *) (*handle)) {
    		pathandle = idbc->PatNodes;
    		stuhandle = idbc->StuNodes;
    		serhandle = idbc->SerNodes;
    		imahandle = idbc->ImaNodes;
    		inshandle = idbc->InsNodes;
    		foundit = 1;
    	}
    	idbc = idbc->next;
	}
    if (!foundit) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADHANDLE), "IDB_InsertImageInstance");
    }

    field[0].FieldName = "SOPInsUID";
    field[0].Value.AllocatedSize = DICOM_UI_LENGTH + 1;
    field[0].Value.Type = TBL_STRING;
    field[0].Value.Value.String = buf;
    field[1].FieldName = 0;

    crit[0].FieldName = "SOPInsUID";
    crit[0].Operator = TBL_EQUAL;
    crit[0].Value.Value.String = imageuid;
    crit[0].Value.Type = TBL_STRING;
    crit[1].FieldName = 0;
    count = 0;

    ret_val = TBL_Select(&imahandle, crit, field, &count, NULL, NULL);
    if (count != 1) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_BADIMAUID), buf, "IDB_InsertImageInstance");
    }

    TBL_FIELD_LOAD_BYTE(insfld[0], "ImageUID", imageuid, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[1], "RespondingTitle", iie->RespondingTitle, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[2], "Medium", iie->Medium, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[3], "Path", iie->Path, TBL_STRING);
    TBL_FIELD_LOAD_BYTE(insfld[4], "Transfer", iie->Transfer, TBL_STRING);
    TBL_FIELD_LOAD_NUM(insfld[5],  "Size", (iie->Size), TBL_SIGNED4);

    insfld[6].FieldName = 0;	/* Terminate the list */

    ret_val = TBL_Insert(&inshandle, insfld);
    if (ret_val != TBL_NORMAL) {
    	THR_ReleaseMutex(FAC_IDB);
    	return COND_PushCondition(IDB_ERROR(IDB_INSERTFAILED), "image instance", "IDB_InsertImageInstance");
    }else{
    	THR_ReleaseMutex(FAC_IDB);
    	return IDB_NORMAL;
    }
}
