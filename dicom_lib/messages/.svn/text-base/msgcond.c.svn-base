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
** @$=@$=@$=
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
** Author, Date:	Stephen M. Moore, 10-May-93
** Intent:		Define the ASCIZ messages that go with each MSG
**			error number and provide a function for looking up
**			the error message.
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:44:14 $
** Source File:		$RCSfile: msgcond.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: msgcond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {MSG_NORMAL, "MSG Normal return from MSG routine"},
    {MSG_ZEROLENGTHCLASSUID, "MSG Zero Length Class UID in routine: %s"},
    {MSG_ZEROLENGTHINSTANCEUID, "MSG Zero Length SOP Instance UID in routine: %s"},
    {MSG_ILLEGALMESSAGETYPE, "MSG Illegal message type (%d) in routine %s"},
    {MSG_NOCOMMANDELEMENT, "MSG COMMAND element missing in Information Object in %s"},
    {MSG_UNSUPPORTEDCOMMAND, "MSG DICOM command not supported by MSG facility (%04x) in %s"},
    {MSG_MALLOCFAILURE, "MSG Failed to malloc %d bytes in routine %s"},
    {MSG_PARSEFAILED, "MSG Failed to parse object of type %d %s"},
    {MSG_OBJECTACCESSERROR, "MSG Failed to access DCM Information Object of type %d in %s"},
    {MSG_OBJECTCREATEFAILED, "MSG Failed to create DCM Information Object in %s"},
    {MSG_MODIFICATIONFAILURE, "MSG Failed to modify DCM Information Object in %s"},
    {MSG_LISTFAILURE, "MSG List failure in %s"},
    {MSG_STATUSCODENOTFOUND, "MSG Status Code (%04x) not found in %s"},
    {MSG_MUTEXFAILED, "MSG Mutex operation failed in %s"},
    {0, NULL}
};


/* MSG_Message
**
** Purpose:
**	Find the ASCIZ message that goes with an DCM error number and
**	return a pointer to static memory containing that error message.
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

char *
MSG_Message(CONDITION condition)
{
    int	        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond)	return messageVector[index].message;
    }
    return NULL;
}
