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
** Module Name(s):	DMAN_Message
** Author, Date:	Steve Moore, Summer 1994
** Intent:		This file contains the function that maps DMAN
**			conditions into text that describes the condition.
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:44:08 $
** Source File:		$RCSfile: mancond.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: mancond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../tbl/tbl.h"
#include "manage.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {DMAN_NORMAL, "Normal return from DMAN routine"},
    {DMAN_UNIMPLEMENTED, "Function %s is not implemented"},
    {DMAN_MALLOCFAILED, "Failed to malloc %d bytes in %s"},
    {DMAN_TABLEOPENFAILED, "Failed to open table %s in function %s"},
    {DMAN_APPLICATIONVERIFICATIONFAILED, "Failed to verify application (%s, %s) in %s"},
    {DMAN_TITLENOTFOUND, "Application title %s not found in database in %s"},
    {DMAN_APPLICATIONNODEMISMATCH, "Application title (%s, %s) not found on expected node (%s) in %s"},
    {DMAN_ILLEGALCONNECTION, "Illegal connection (%s, %s) request in %s"},
    {DMAN_STORAGEACCESSDENIED, "Database access denied (%s, %s) by %s"},
    {DMAN_FILEGENERATIONFAILED, "Failed to generate a file name for (%s, %s) in %s"},
    {DMAN_FILENAMETOOLONG, "Generated file name length (%d) greater than allocated length (%d) in %s"},
    {DMAN_PATHNOTDIR, "Path name (%s) is not a directory as expected"},
    {DMAN_FILECREATEFAILED, "Failed to create file (%s) of type %s in %s"},
    {DMAN_APPLICATIONLOOKUPFAILED, "Failed to lookup application (%s) in %s"},
    {DMAN_STORAGELOOKUPFAILED, "Failed to lookup storage for application (%s) in %s"},
    {DMAN_ILLEGALPRINTSERVERCONFIGURATION, "Illegal Print Server Configuration (%s, %s, %s) request in %s"},
    {DMAN_FISACCESSLOOKUPFAILED, "Failed to lookup FIS Access for application (%s) in %s"},
    {DMAN_ILLEGALHANDLE, "DMAN Illegal handle passed to function %s"},
    {0, NULL}
};


/* DMAN_Message
**
** Purpose:
**	Find the ASCIZ message that goes with an DMAN error number and
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
DMAN_Message(CONDITION condition)
{
    int        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
