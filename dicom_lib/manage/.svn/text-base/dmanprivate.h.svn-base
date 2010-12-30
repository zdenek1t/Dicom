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
** Author, Date:	Stephen M. Moore, 20-May-1994
** Intent:		Provide private structure defintions for facility.
** Last Update:		$Author: smm $, $Date: 1996/08/23 19:45:33 $
** Source File:		$RCSfile: dmanprivate.h,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    TBL_HANDLE 				*applicationEntityHandle;
    TBL_HANDLE 				*groupNamesHandle;
    TBL_HANDLE 				*securityMatrixHandle;
    TBL_HANDLE 				*storageAccessHandle;
    TBL_HANDLE 				*storageControlHandle;
    TBL_HANDLE 				*FISAccessHandle;
    TBL_HANDLE 				*printServerCFGHandle;
    TBL_HANDLE 				*VideoImageDestHandle;
    char 					requestingTitle[17];
    char 					respondingTitle[17];
    CTNBOOLEAN 				storageAccess;
    CTNBOOLEAN 				readAccess;
    CTNBOOLEAN 				writeAccess;
    DMAN_STORAGECONTROL 	*storage;
}   PRIVATE_HANDLE;

typedef struct {
    char 	*FieldName;
    int 	Flag;
}   FLAG_MAP;

#ifdef  __cplusplus
}
#endif
