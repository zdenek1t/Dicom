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
** Module Name(s):
** Author, Date:
** Intent:
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:46:23 $
** Source File:		$RCSfile: tblcond.c,v $
** Revision:		$Revision: 1.8 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.8 $ $RCSfile: tblcond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "tblprivate.h"
#include "tbl.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {TBL_NORMAL, 		 	"Normal return from TBL routine"},
    {TBL_UNIMPLEMENTED,  	"Function %s is not implemented"},
    {TBL_MALLOCFAILURE,  	"Function %s failed to malloc %d bytes"},
    {TBL_OPENFAILED, 	 	"Function failed to open table: %s"},
    {TBL_FILEOPENFAILED, 	"Function %s failed to open file: %s"},
    {TBL_ILLEGALFORMAT, 	"Illegal table format detected (%s) in routine %s"},
    {TBL_LISTCREATEFAILURE, "List create failed in %s"},
    {TBL_LISTFAILURE, 		"List function failed in %s"},
    {TBL_DBINITFAILED, 		"%s dbinit()/dblogin() failed in %s"},
    {TBL_ALREADYOPENED, 	"DB/Table pair [%s/%s] has already been opened in %s"},
    {TBL_TBLNOEXIST, 		"Table %s does not exist in %s"},
    {TBL_NOMEMORY, 			"No memory available in %s"},
    {TBL_CLOSERROR, 		"Database handle could not be found in %s"},
    {TBL_BADHANDLE, 		"Database handle could not be found in %s"},
    {TBL_DBNOEXIST, 		"Database %s does not exist in %s"},
    {TBL_NOFIELDLIST, 		"No field list was supplied to %s"},
    {TBL_SELECTFAILED, 		"The %s select operation failed in %s"},
    {TBL_EARLYEXIT, 		"The callback routine returned something other than TBL_NORMAL in %s"},
    {TBL_UPDATEFAILED, 		"The %s update operation failed in %s"},
    {TBL_INSERTFAILED, 		"The %s insert operation failed in %s"},
    {TBL_DELETEFAILED, 		"The %s delete operation failed in %s"},
    {TBL_NOCALLBACK, 		"No callback function was supplied to %s"},
    {TBL_NOCOLUMNS, 		"No columns were found in %s"},
    {TBL_DBSPECIFIC, 		"DB specific error (%s:%s) in %s"},
    {TBL_CHARSETFAILED,		"Character set %s failed in %s"},
    {0, NULL}
};


/* TBL_Message
**
** Purpose:
**	Find the ASCIZ message that goes with an TBL error number and
**	return a pointer to static memory containing that error message.
**
** Parameter Dictionary:
**	condition	The error condition for which the message is to be
**			returned
**
** Return Values:
**	The error message if a valid error condition was reported else NULL.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

char *
TBL_Message(CONDITION condition)
{
    int		index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
