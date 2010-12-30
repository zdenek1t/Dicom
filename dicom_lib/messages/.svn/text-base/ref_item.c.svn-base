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
** Module Name(s):	MSG_BuildReferencedItemSequence
** Author, Date:	Stephen M. Moore, 24-Jun-93
** Intent:		This module contains functions for accessing
**			attributes which are sequences of "referenced
**			items".
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:06 $
** Source File:		$RCSfile: ref_item.c,v $
** Revision:		$Revision: 1.10 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.10 $ $RCSfile: ref_item.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#include <stdarg.h>
#include <sys/types.h>
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "msgprivate.h"


/* MSG_BuildReferenceItemSequence
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSG_BuildReferencedItemSequence(MSG_REFERENCED_ITEM * item, LST_HEAD ** list)
{
    CONDITION			cond;
    DCM_SEQUENCE_ITEM	* sq;

    if (*list == NULL) {
    	*list = LST_Create();
    	if (*list == NULL) return COND_PushCondition(MSG_LISTFAILURE, MSG_Message(MSG_LISTFAILURE), "MSG_BuildReferencedItemSequence");
    }
    sq = CTN_MALLOC(sizeof(*sq));
    if (sq == NULL)	return COND_PushCondition(MSG_MALLOCFAILURE, MSG_Message(MSG_MALLOCFAILURE), sizeof(*sq), "MSG_BuildReferencedItemSequence");

    cond = MSG_BuildCommand(item, &sq->object);
    if (cond != MSG_NORMAL)	return cond;

    cond = LST_Enqueue(list, item);
    if (cond != LST_NORMAL)	return COND_PushCondition(MSG_LISTFAILURE, MSG_Message(MSG_LISTFAILURE), "MSG_BuildReferencedItemSequence");

    return MSG_NORMAL;
}
