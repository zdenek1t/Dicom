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
** Module Name(s):	UID_Message
** Author, Date:	Stephen M. Moore, 29-Jun-93
** Intent:		Define the ASCIZ messages that go with each UID
**			error number and provide a function for looking up
**			the error message.
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:46:56 $
** Source File:		$RCSfile: uidcond.c,v $
** Revision:		$Revision: 1.7 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.7 $ $RCSfile: uidcond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "dicom_uids.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {UID_NORMAL, "UID Normal return from UID routine"},
    {UID_NOUIDFILENAME, "UID No environment variable (UIDFILE) defined"},
    {UID_GENERATEFAILED, "UID Failed to generate a uniqe identifier"},

    {UID_FILEOPENFAILURE, "UID Failed to open file (%s) for read"},
    {UID_FILECREATEFAILURE, "UID Failed to create file (%s) for write"},
    {UID_ILLEGALROOT, "UID Illegal root in line: %s"},
    {UID_ILLEGALNUMERIC, "UID Illegal numeric in line: %s"},
    {UID_NODEVICETYPE, "UID No device type specified in file (%s)"},
    {UID_NOROOT, "UID No root value specified in file (%s)"},
    {UID_UIDNOTFOUND, "UID Could not find UID %s in %s"},
    {0, NULL}
};


/* UID_Message
**
** Purpose:
**	Find the ASCIZ message that goes with an UID error number and
**	return a pointer to static memory containing that error message.
**
** Parameter Dictionary:
**	condition	The error condition number
**
** Return Values:
**	Error message corresponding to the condition number. If no such
**	condition exists, a NULL message is returned.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

char *
UID_Message(CONDITION condition)
{
    int		index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
