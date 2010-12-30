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
** Module Name(s):	dicom_ie.h
** Author, Date:	Stephen M. Moore, 30-May-93
** Intent:		Header file for the IE_facility which provides
**                      subroutines to manipulate information entities
**                      in DICOM V3 information objects.
** Last Update:		$Author: smm $, $Date: 1996/08/23 19:42:20 $
** Source File:		$RCSfile: dicom_ie.h,v $
** Revision:		$Revision: 1.15 $
** Status:		$State: Exp $
*/

#ifndef IE_IS_IN
#define IE_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
    IE_K_INFORMATIONOBJECT,
    IE_K_INFORMATIONENTITY,
    IE_K_MODULE,
    IE_K_ATTRIBUTE
}   IE_STRUCTURETYPE;

typedef enum {
    IE_K_REQUIRED,
    IE_K_OPTIONAL
}   IE_IEREQUIREMENT;

typedef enum {
    IE_K_TYPE1,
    IE_K_TYPE1C,
    IE_K_TYPE2,
    IE_K_TYPE2C,
    IE_K_TYPE3,
    IE_K_TYPE3C
}   IE_ATTRIBUTEREQUIREMENT;

typedef enum {
    IE_K_PATIENTIE,
    IE_K_STUDYIE,
    IE_K_SERIESIE,
    IE_K_FRAMEOFREFERENCEIE,
    IE_K_EQUIPMENTIE,
    IE_K_IMAGEIE,
    IE_K_OVERLAYIE,
    IE_K_CURVEIE
}   IE_IETYPE;

typedef enum {
    IE_K_PATIENTMODULE,
    IE_K_GENERALSTUDYMODULE,
    IE_K_PATIENTSTUDYMODULE,
    IE_K_GENERALSERIESMODULE,
    IE_K_CRSERIESMODULE,
    IE_K_NMSERIESMODULE,
#if STANDARD_VERSION < VERSION_APR1995
    IE_K_NMSPECTACQSERIESMODULE,
    IE_K_NMMULTIACQSERIESMODULE,
#endif
    IE_K_USSERIESMODULE,
    IE_K_USLOOPSERIESMODULE,
    IE_K_REGIONCALIBRATIONMODULE,
    IE_K_FRAMEOFREFERENCEMODULE,
    IE_K_GENERALEQUIPMENTMODULE,
#if STANDARD_VERSION < VERSION_APR1995
    IE_K_NMEQUIPMENTMODULE,
#endif
    IE_K_SCEQUIPMENTMODULE,
    IE_K_SOPCOMMONMODULE,
    IE_K_GENERALIMAGEMODULE,
    IE_K_IMAGEPLANEMODULE,
    IE_K_IMAGEPIXELMODULE,
    IE_K_CINEMODULE,
    IE_K_MULTIFRAMEMODULE,
    IE_K_CONTRASTMODULE,
    IE_K_CRIMAGEMODULE,
    IE_K_CTIMAGEMODULE,
    IE_K_MRIMAGEMODULE,
    IE_K_NMIMAGEMODULE,
    IE_K_USIMAGEMODULE,
    IE_K_USMULTIFRAMEIMAGEMODULE,
    IE_K_SCIMAGEMODULE,
    IE_K_OVERLAYPLANEMODULE,
    IE_K_LOOKUPTABLEMODULE,
    IE_K_CURVEMODULE,
    IE_K_OVERLAYIDENTIFICATIONMODULE,
    IE_K_CURVEIDENTIFICATIONMODULE,
    IE_K_CURVEPLANEMODULE,
#if STANDARD_VERSION >= VERSION_APR1995
    IE_K_NMIMAGEPIXELMODULE,
    IE_K_NMMULTIFRAMEIMAGEMODULE,
    IE_K_NMISOTOPEIMAGEMODULE,
    IE_K_NMDETECTORIMAGEMODULE,
    IE_K_NMTOMOACQIMAGEMODULE,
    IE_K_NMMULTIGATEDACQIMAGEMODULE,
    IE_K_NMPHASEIMAGEMODULE,
    IE_K_NMRECONSTRUCTIONIMAGEMODULE,
    IE_K_MULTIFRAMEOVERLAYPLANEMODULE,
    IE_K_VOILUTIMAGEMODULE
#endif
}   IE_MODULETYPE;

typedef enum {
    IE_MISSING,
    IE_INCOMPLETE,
    IE_COMPLETE
}   IE_STATUS;

typedef struct {
    void *reserved[2];
    IE_STRUCTURETYPE structureType;
    DCM_ELEMENT element;
    IE_ATTRIBUTEREQUIREMENT requirement;
    IE_STATUS status;
}   IE_ATTRIBUTE;

typedef struct {
    void *reserved[2];
    IE_STRUCTURETYPE structureType;	/* Module, information entity */
    IE_MODULETYPE moduleType;	/* The type of module */
    char moduleDescription[48];
    IE_IEREQUIREMENT requirement;
    IE_STATUS status;
    LST_HEAD *attributeList;	/* A list of IE_ATTRIBUTEs */
}   IE_MODULE;

typedef struct {
    void *reserved[2];
    IE_STRUCTURETYPE structureType;	/* Module, information entity */
    IE_IETYPE ieType;		/* The type of information entity */
    char ieDescription[64];
    IE_IEREQUIREMENT requirement;
    IE_STATUS status;
    LST_HEAD *moduleList;	/* A list of IE_MODULEs */
}   IE_INFORMATIONENTITY;

typedef struct {
    void *reserved[2];
    IE_STRUCTURETYPE structureType;	/* Object, module, IE */
    char classUID[DICOM_UI_LENGTH + 1];
    char objectDescription[64];
    IE_STATUS status;
    LST_HEAD *ieList;		/* A list of IE_INFORMATIONENTITYs */
}   IE_OBJECT;


CONDITION
IE_ExamineObject(DCM_OBJECT ** dcmObject, IE_OBJECT ** ieObject);
CONDITION
IE_ExamineInformationEntity(DCM_OBJECT ** dcmObject, IE_IETYPE ieType, IE_INFORMATIONENTITY ** ieEntity);
CONDITION
IE_ExamineModule(DCM_OBJECT ** dcmObject, IE_IETYPE ieType, IE_MODULETYPE moduleType, IE_MODULE ** ieModule);
CONDITION
IE_ObjectRequirements(char *classUID, IE_OBJECT ** object);
CONDITION
IE_IERequirements(char *classUID, IE_IETYPE ieType, IE_INFORMATIONENTITY ** ieEntity);
CONDITION
IE_ModuleRequirements(char *classUID, IE_IETYPE ieType, IE_MODULETYPE moduleType, IE_MODULE ** ieModule);
CONDITION
IE_Free(void **object);
char
*IE_Message(CONDITION cond);

#define IE_NORMAL	/* Normal return from IE routine */ \
	FORM_COND(FAC_IE, SEV_SUCC, 1)
#define IE_OBJECTINCOMPLETE /* Object Incomplete return from IE routine */ \
        FORM_COND(FAC_IE, SEV_WARN, 2)
#define IE_IEINCOMPLETE	/* IE Incomplete return from IE routine */ \
        FORM_COND(FAC_IE, SEV_WARN, 3)
#define IE_IEMISSING		/* IE missing return from IE routine */ \
	FORM_COND(FAC_IE, SEV_ERROR, 4)
#define IE_MODULEINCOMPLETE	/* Module Incomplete return from IE routine */ \
        FORM_COND(FAC_IE, SEV_WARN, 5)
#define IE_MODULEMISSING	/* Module missing return from IE routine */ \
	FORM_COND(FAC_IE, SEV_ERROR, 6)
#define IE_ILLEGALDCMOBJECT	/* Illegal DICOM Object passed to IE routine*/ \
	FORM_COND(FAC_IE, SEV_ERROR, 7)
#define IE_LISTFAILURE  /* unable to perform list operations */ \
        FORM_COND(FAC_IE, SEV_ERROR, 8)
#define IE_MALLOCFAILURE        /* malloc() failure */ \
        FORM_COND(FAC_IE, SEV_ERROR, 9)

#ifdef  __cplusplus
}
#endif

#endif
