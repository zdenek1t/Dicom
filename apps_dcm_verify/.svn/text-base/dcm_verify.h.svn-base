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
**                              DICOM 93
**                   Electronic Radiology Laboratory
**                 Mallinckrodt Institute of Radiology
**              Washington University School of Medicine
**
** Module Name(s):	dcm_verify.h
** Author, Date:        Pei Weng, 2-June-93
** Intent:		This file contains definition of structures
**			which contain the ASCII messages that go with
**			IE_STATUS, IE_STRUCTURETYPE and REQUIREMENT.
**			By searching the table using the variable
**			of the above type as the key, the programmer
**			can find the corresponding description instead
**			of the enumerated values.
** Last Update:         $Author: smm $, $Date: 1994/12/30 18:36:44 $
** Source File:         $RCSfile: dcm_verify.h,v $
** Revision:            $Revision: 1.5 $
** Status:              $State: Exp $
*/

typedef struct {
    IE_IEREQUIREMENT 	requirement;
    char 				*string;
}   REQUIREMENTID;

typedef struct {
    IE_ATTRIBUTEREQUIREMENT 	requirement;
    char 						*string;
}   ATTR_REQID;

typedef struct {
    IE_STRUCTURETYPE 	type;
    char 				*string;
}   STRUCTUREID;

typedef struct {
    IE_STATUS 		status;
    char 			*string;
}   STATUSID;

/* Table for IE_IEREQUIREMENT */
static REQUIREMENTID requirement[] = {
    {IE_K_REQUIRED, "Mandatory"},
    {IE_K_OPTIONAL, "Optional"}
};

/* Table for IE_STRUCTURETYPE */
static STRUCTUREID structure[] = {
    {IE_K_INFORMATIONOBJECT, "IE_OBJECT"},
    {IE_K_INFORMATIONENTITY, "IE_IE"},
    {IE_K_MODULE, "IE_MODULE"},
    {IE_K_ATTRIBUTE, "IE_ATTRIBUTE"}
};

/* Table for IE_STATUS */
static STATUSID status[] = {
    {IE_COMPLETE, "Complete"},
    {IE_INCOMPLETE, "Incomplete"},
    {IE_MISSING, "Missing"},
};
