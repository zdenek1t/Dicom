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
** Module Name(s):	SNP_Message
** Author, Date:	Nilesh R. Gohel, 23-Aug-94
** Intent:		Define the ASCII messages that go with each SNP
**			error number and provide a function for looking up
**			the error message.
** Last Update:		$Author: smm $, $Date: 1998/03/13 19:38:34 $
** Source File:		$RCSfile: snpcond.c,v $
** Revision:		$Revision: 1.5 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.5 $ $RCSfile: snpcond.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "decode.h"
#include "snp.h"

typedef struct vector {
    CONDITION 	cond;
    char 		*message;
}   VECTOR;

static VECTOR messageVector[] = {
    {SNP_NORMAL, "SNP Normal return from SNP routine"},
    {SNP_MALLOCERROR, "SNP could not malloc %d bytes in function %s"},
    {SNP_CLOSEERROR, "SNP could not close the file %s in function %s"},
    {SNP_OPENERROR, "SNP could not open the file %s in function %s"},
    {SNP_SIGSETERROR, "SNP error setting up for %s signal in function %s"},
    {SNP_STREAMSETUP, "SNP error setting up kernel level streams processing from %s"},
    {SNP_LSTCREATFAIL, "SNP error creating %s LST in function %s"},
    {SNP_CALLBACKSMISSING, "SNP error - all callbacks not registered"},
    {SNP_CALLBACKFAIL, "SNP error using callback function %s"},
    {SNP_ARGERROR, "Problem with argument %s to function %s : %s"},
    {SNP_IOCTLFAIL, "SNP %s ioctl failure in function %s"},
    {SNP_UNIMPLEMENTED, "SNP error unimplemented function %s"},
    {SNP_PUTMSGFAIL, "SNP putmsg failure in function %s"},
    {SNP_DLPIFAIL, "SNP failure in DLPI routine %s"},
    {SNP_DLPIEXPECT, "SNP DLPI function strgetmsg expected %s, got %s"},
    {SNP_ALARMSET, "SNP alarm set failure in function %s"},
    {SNP_GETMSGFAIL, "SNP getmsg failure in function %s"},
    {0, NULL}
};


/* SNP_Message
**
** Purpose:
**	Find the ASCII message that goes with an SNP error number and
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
SNP_Message(CONDITION condition)
{
    int        index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
