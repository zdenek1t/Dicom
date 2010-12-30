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
** Author, Date:	Stephen M. Moore, 11-Jun-93
** Intent:
** Last Update:		$Author: smm $, $Date: 2003/03/11 20:15:32 $
** Source File:		$RCSfile: iap.h,v $
** Revision:		$Revision: 1.12 $
** Status:		$State: Exp $
*/

#include "../database/dbquery.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    DCM_ELEMENT e;
    long flag;
    long *flagAddress;
}   IAP_ELEMENT_MAP;

CONDITION
IAP_SendImage(DUL_ASSOCIATIONKEY ** association,
	      DUL_ASSOCIATESERVICEPARAMETERS * params,
	    char *fileName, char *moveAETitle, unsigned short moveMessageID,
	      CONDITION(*callback) (MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response,
	     unsigned long transmitted, unsigned long total, char *string), 
		 void *callbackCtx);
CONDITION
IAP_SendInfoObject(DUL_ASSOCIATIONKEY ** association,
		   DUL_ASSOCIATESERVICEPARAMETERS * params,
		   const char *fileName, const char* fileXferSyntax,
		   const char *moveAETitle, unsigned short moveMessageID,
		   CONDITION(*callback) (), void *callbackCtx);

CONDITION
IAP_ObjectToQuery(DCM_OBJECT ** object, char *SOPClass,
		  Query * query, int *elementCount);

CONDITION
IAP_QueryToObject(Query * query, DCM_OBJECT ** object,
		  char *SOPClass, int *elementCount);

char *IAP_Message(CONDITION cond);

#define	IAP_NORMAL	FORM_COND(FAC_IAP, SEV_SUCC, 1)
#define	IAP_ILLEGALOBJECT	FORM_COND(FAC_IAP, SEV_ERROR, 2)
#define	IAP_QUERYLEVELMISSING	FORM_COND(FAC_IAP, SEV_WARN, 3)
#define	IAP_INCOMPLETEOBJECT	FORM_COND(FAC_IAP, SEV_WARN, 4)
#define	IAP_ILLEGALSOPCLASS	FORM_COND(FAC_IAP, SEV_ERROR, 5)
#define	IAP_OBJECTCREATEFAILED	FORM_COND(FAC_IAP, SEV_ERROR, 6)
#define	IAP_INCOMPLETEQUERY	FORM_COND(FAC_IAP, SEV_WARN, 7)
#define IAP_SOPCLASSMISSING	FORM_COND(FAC_IAP, SEV_WARN, 8)
#define IAP_OBJECTACCESSFAILED	FORM_COND(FAC_IAP, SEV_WARN, 9)
#define IAP_SENDFAILED	FORM_COND(FAC_IAP, SEV_ERROR, 10)
#define IAP_MALLOCFAILURE	FORM_COND(FAC_IAP, SEV_WARN, 11)

#ifdef  __cplusplus
}
#endif
