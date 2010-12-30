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
** Module Name(s):
** Author, Date:
** Intent:
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:43:30 $
** Source File:		$RCSfile: idbcond.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: idbcond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "../idb/idb.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {IDB_NORMAL, "Normal return from IDB routine"},
    {IDB_UNIMPLEMENTED, "Function %s is not implemented"},
    {IDB_ALREADYOPENED, "Database %s is already opened in %s"},
    {IDB_BADDBTABPAIR, "Bad Database/Table [%s/%s] in %s"},
    {IDB_NOMEMORY, "No more memory in function %s"},
    {IDB_CLOSERROR, "%s: Cannot Close database (Can't find handle)"},
    {IDB_BADHANDLE, "Cannot find datbase handle in %s"},
    {IDB_NULLUID, "A NULL UID was given to %s"},
    {IDB_BADPATUID, "Patid: %s cannot be found in %s"},
    {IDB_BADLISTENQ, "A list enqueue operation failed in %s"},
    {IDB_BADSTUUID, "StudyUID: %s cannot be found in %s"},
    {IDB_BADSERUID, "SeriesUID: %s cannot be found in %s"},
    {IDB_BADLEVEL, "A bad Level combination was specified in %s"},
    {IDB_BADLEVELSEQ, "A bad Level sequence was specified in %s"},
    {IDB_NOMATCHES, "No matches were found in %s"},
    {IDB_EARLYEXIT, "The callback routine returned something other than IDB_NORMAL in %s"},
    {IDB_NOINSERTDATA, "No insert data was provided to %s"},
    {IDB_BADIMAUID, "ImageSOPInsUID: %s cannot be found in %s"},
    {IDB_DUPINSTANCE, "Attempt to insert a duplicate image instance in %s"},
    {IDB_PATIDMISMATCH, "Patient ID mismatch for Study UID %s, Pat ID (%s:%s) in %s"},
    {IDB_DELETEFAILED, "Failed to delete records as requested in %s"},
    {IDB_INSERTFAILED, "Failed to insert type (%s) in %s"},
    {IDB_FILEDELETEFAILED, "Failed to delete file (%s) in %s"},
    {0, NULL}
};


/* IDB_Message
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
IDB_Message(CONDITION condition)
{
    int        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
