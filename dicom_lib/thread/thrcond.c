/*
          Copyright (C) 1996 RSNA and Washington University

          The software and supporting documentation for the Radiological
          Society of North America (RSNA) 1993, 1994 Digital Imaging and
          Communications in Medicine (DICOM) Demonstration were developed
          at the
                  Electronic Radiology Laboratory
                  Mallinckrodt Institute of Radiology
                  Washington University School of Medicine
                  510 S. Kingshighway Blvd.
                  St. Louis, MO 63110
          as part of the 1993-1996 DICOM Central Test Node project for, and
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
**				DICOM 96
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	THR_Message
** Author, Date:	Stephen M. Moore, 16-Sep-96
** Intent:		Define the ASCIZ messages for errors and provide
**			a function for table lookup of the error messages.
** Last Update:		$Author: smm $, $Date: 1998/07/31 19:56:17 $
** Source File:		$RCSfile: thrcond.c,v $
** Revision:		$Revision: 1.3 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.3 $ $RCSfile: thrcond.c,v $";

#include <stdio.h>
#include <sys/types.h>

#include "../dicom/dicom.h"
#include "ctnthread.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {THR_NORMAL, 		 "Normal return from THR routine"},
    {THR_GENERICFAILURE, "THR Generic Failure: %s in %s"},
    {THR_NOTINITIALIZED, "THR Threads not initialized in call to %s"},
    {0, NULL}
};


/* THR_Message
**
** Purpose:
**	Issue an error message depending on the condition.
**
** Parameter Dictionary:
**	condition	Condition indicating type of error.
**
** Return Values:
**	message
**	NULL, if condition doesn't exist.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

char *
THR_Message(CONDITION condition)
{
    int        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
