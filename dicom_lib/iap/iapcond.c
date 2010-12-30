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
** Intent:		Define the ASCIZ messages that go with condition
**			codes and provide a function that returns a pointer
**			to a particular message (given a code).
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:04 $
** Source File:		$RCSfile: iapcond.c,v $
** Revision:		$Revision: 1.10 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.10 $ $RCSfile: iapcond.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#endif

#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../dulprotocol/dulprotocol.h"
#include "../database/dbquery.h"
#include "../iap/iap.h"

typedef struct vector {
    CONDITION cond;
    char *message;
}   VECTOR;

static VECTOR messageVector[] = {
    {IAP_NORMAL, "Normal return from IAP routine"},
    {IAP_ILLEGALOBJECT, "IAP Illegal object detected in %s"},
    {IAP_QUERYLEVELMISSING, "IAP Query level missing from query in %s"},
    {IAP_INCOMPLETEOBJECT, "IAP Query object incomplete in %s"},
    {IAP_OBJECTCREATEFAILED, "IAP Failed to create DCM object in %s"},
    {IAP_INCOMPLETEQUERY, "IAP Incomplete query detected in %s"},
    {IAP_SOPCLASSMISSING, "SOP Class missing from Object"},
    {IAP_OBJECTACCESSFAILED, "IAP Failed to access DCM object in %s"},
    {IAP_SENDFAILED, "IAP Failed to send image in %s"},
    {IAP_MALLOCFAILURE, "IAP Failed to malloc %d bytes in %s"},
    {0, NULL}
};

char *
IAP_Message(CONDITION condition)
{
    int        index;

    for (index = 0; index < (int) DIM_OF(messageVector); index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
