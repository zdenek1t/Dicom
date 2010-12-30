/*
+-+-+-+-+-+-+-+-+-
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
** Author, Date:	Stephen M. Moore, 15-Jun-93
** Intent:		This module contains support routines for the MSG
**			facility.  These support routines are private to
**			the MSG facility.
** Last Update:		$Author: smm $, $Date: 1993/06/22 02:10:25 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/messages/msgsupport.c,v $
** Revision:		$Revision: 1.3 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.3 $ $RCSfile: msgsupport.c,v $";

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "msgprivate.h"

/* MSGPRV_BuildObject
**
** Purpose:
**	Describe the purpose of the function
**
** Parameter Dictionary:
**	Define the parameters to the function
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
MSGPRV_BuildObject(DCM_OBJECT ** object, DCM_ELEMENT * required, int requiredCount, DCM_FLAGGED_ELEMENT * conditional, int conditionalCount)
{
    int	        	index;
    CONDITION		cond;
    DCM_ELEMENT		e;

    cond = DCM_CreateObject(object);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    cond = DCM_ModifyElements(object, required, requiredCount, conditional, conditionalCount, NULL);
    if (cond != DCM_NORMAL)	return COND_PushCondition(0, "");	/* repair */

    return MSG_NORMAL;
}
