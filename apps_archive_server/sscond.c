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
** Module Name(s):	APP_Message
** Author, Date:	Stephen M. Moore, 20-Jul-1993
** Intent:		Define the ASCIZ messages that go with condition codes
**			and provide a function that returns a pointer to the
**			messages.
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:37:42 $
** Source File:		$RCSfile: sscond.c,v $
** Revision:		$Revision: 1.5 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.5 $ $RCSfile: sscond.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <time.h>
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"
#ifdef CTN_MULTIBYTE
#include "tblmb.h"
#include "idbmb.h"
#else
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/idb/idb.h"
#endif
#include "../dicom_lib/manage/manage.h"

#include "image_archive.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {APP_NORMAL, "Normal return from application routine"},
    {APP_ILLEGALSERVICEPARAMETER, "APP Illegal service parameter (name %s, value %s)"},
    {APP_MISSINGSERVICEPARAMETER, "APP The %s parameter is missing"},
    {APP_PARAMETERWARNINGS, "APP One or more service parameters were incorrect"},
    {APP_PARAMETERFAILURE, "APP One or more service parameters were incorrect"},
    {APP_FAILURE, "APP Failure: %s in %s"},
    {APP_ASSOCIATIONRQFAILED, "APP Failed to establish Association"},
    {APP_IMAGEPARSEFAILED, "APP failed to parse an image for insertion into database"},
    {APP_GENERALMSG, "APP General msg: %s"},
    {APP_ASSOCIATIONINITIATIONFAILED, "APP Failed to establish Association with %s %s %s"},
    {APP_STARTUPFAILURE, "APP Failed during startup: %s"}
};


/* APP_Message
**
** Purpose:
**	This function accepts a CONDITION as an input parameter and finds
**	the ASCIZ message that is defined for that CONDITION.  If the
**	CONDITION is defined for this facility, this function returns
**	a pointer to the ASCIZ string which describes the condition.
**	If the CONDITION is not found, the function returns NULL.
**
** Parameter Dictionary:
**	condition	The CONDITION used to search the dictionary.
**
** Return Values:
**	ASCIZ string which describes the condtion requested by the caller
**	NULL if the condition is not found
**
*/
char *
APP_Message(CONDITION condition)
{
    int        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
