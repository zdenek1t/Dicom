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
** Module Name(s):	SRVPRV_PresentationContext
** Author, Date:	Stephen M. Moore, 27-Apr-93
** Intent: 		This module contains private functions that are
** 			used by the SRV facility. These functions provide
**			support for the services we have written, but are not
**			public to the user.
**
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:10 $
** Source File:		$RCSfile: private.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: private.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#endif

#include "../dicom/dicom.h"
#include "../uid/dicom_uids.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"
#include "../objects/dicom_objects.h"


/* SRVPRV_PresentationContext
**
** Purpose:
**	Obtain the presentation context for the specified SOP class UID
**	from the service parameters
**
** Parameter Dictionary:
**	params		Service parameters for the Association
**	classUID	SOP class UID for which the presentation context
**			is required
**
** Return Values:
**	Handle to the presentation context if found, else NULL.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

DUL_PRESENTATIONCONTEXT *
SRVPRV_PresentationContext(DUL_ASSOCIATESERVICEPARAMETERS * params, char *classUID)
{
    DUL_PRESENTATIONCONTEXT    	* presentationCtx;/* Presentation context for this service */

#if 0
    fprintf(stderr, "<%s>\n", classUID);
#endif

    if (params->acceptedPresentationContext == NULL) return NULL;

    presentationCtx = LST_Head(&params->acceptedPresentationContext);
    if (presentationCtx == NULL) return NULL;

    (void) LST_Position(&params->acceptedPresentationContext, presentationCtx);

    while (presentationCtx != NULL){
#if 0
    	fprintf(stderr, "%d <%s> <%s>\n", presentationCtx->result, classUID, presentationCtx->abstractSyntax);
#endif
    	if ((strcmp(classUID, presentationCtx->abstractSyntax) == 0) && (presentationCtx->result == DUL_PRESENTATION_ACCEPT)) return presentationCtx;
    	presentationCtx = LST_Next(&params->acceptedPresentationContext);
    }

    if (strcmp(classUID, DICOM_SOPCLASSDETACHEDINTERPRETMGMT) == 0){
    	return SRVPRV_PresentationContext(params, DICOM_SOPCLASSDETACHEDRESULTSMGMTMETA);
    }else if (strcmp(classUID, DICOM_SOPCLASSDETACHEDINTERPRETMGMT) == 0){
    	return SRVPRV_PresentationContext(params, DICOM_SOPCLASSDETACHEDRESULTSMGMTMETA);
    }
    return NULL;
}
