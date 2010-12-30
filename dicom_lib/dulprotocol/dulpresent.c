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
** Module Name(s):	DUL_MakePresentationCtx
** Author, Date:	Stephen M. Moore, 15-Apr-93
** Intent:		This module contains routines for the user to
**			build and manipulate the public DUL structures.
** Last Update:		$Author: smm $, $Date: 2001/12/26 16:12:18 $
** Source File:		$RCSfile: dulpresent.c,v $
** Revision:		$Revision: 1.18 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.18 $ $RCSfile: dulpresent.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#ifdef _MSC_VER
#include <winsock.h>
#else
#include <sys/socket.h>
#endif
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifdef _MSC_VER
#else
#include <netinet/in.h>
#include <netdb.h>
#endif
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#endif

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "dulprotocol.h"
#include "dulstructures.h"
#include "dulprivate.h"
#include "dulfsm.h"


/* DUL_MakePresentationCtx
**
** Purpose:
**	Build a Presentation Context from the specified parameters
**
** Parameter Dictionary:
**	ctx		Pointer to the presentation context that is to
**			be built.
**	proposedSCRole	Proposed role played by the caller
**	acceptedSCRole	Accepted role (after negotiation)
**	ctxID		Unique ID for this presentation context
**	result
**	abstarctSyntax
**
** Return Values:
**	DUL_NORMAL
**	DUL_LISTCREATEFAILED
**	DUL_LISTERROR
**	DUL_MALLOCERROR
**
**
** Notes:
**	The transfer syntax argument allows the caller to specify one
**	or more transfer syntaxes.  The function expects the caller to
**	terminate the set of transfer syntaxes with a NULL pointer.
**
**	Transfer syntaxes of 0 length are not considered an error and/but
**	are ignored.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

CONDITION
DUL_MakePresentationCtx(DUL_PRESENTATIONCONTEXT ** ctx,
						DUL_SC_ROLE proposedSCRole, DUL_SC_ROLE acceptedSCRole,
						DUL_PRESENTATIONCONTEXTID ctxID, unsigned char result,
						const char *abstractSyntax, const char *transferSyntax,...)
{
    va_list					args;
    LST_HEAD				* lst;
    DUL_TRANSFERSYNTAX		* transfer;
#ifdef lint
    char 					__builtin_va_alist;
#endif

    *ctx = (DUL_PRESENTATIONCONTEXT *) CTN_MALLOC(sizeof(**ctx));
    if (*ctx == NULL) return COND_PushCondition(DUL_MALLOCERROR, DUL_Message(DUL_MALLOCERROR), "DUL_MakePresentationCtx", sizeof(**ctx));

    (void) memset(*ctx, 0, sizeof(**ctx));
    lst = LST_Create();
    if (lst == NULL)
	return COND_PushCondition(DUL_LISTCREATEFAILED, DUL_Message(DUL_LISTCREATEFAILED), "DUL_MakePresentationCtx");

    (*ctx)->presentationContextID = ctxID;
    (*ctx)->result = result;
    (*ctx)->proposedSCRole = proposedSCRole;
    (*ctx)->acceptedSCRole = acceptedSCRole;
    strcpy((*ctx)->abstractSyntax, abstractSyntax);

    va_start(args, transferSyntax);
    strcpy((*ctx)->acceptedTransferSyntax, transferSyntax);

    while ((transferSyntax = va_arg(args, char *)) != NULL) {
    	if (strlen(transferSyntax) != 0) {
    		transfer = CTN_MALLOC(sizeof(*transfer));
    		if (transfer == NULL) return COND_PushCondition(DUL_MALLOCERROR, DUL_Message(DUL_MALLOCERROR), "DUL_MakePresentationCtx", sizeof(*transfer));

    		strcpy(transfer->transferSyntax, transferSyntax);
    		if (LST_Enqueue(&lst, transfer) != LST_NORMAL) return COND_PushCondition(DUL_LISTERROR, DUL_Message(DUL_LISTERROR), "DUL_MakePresentationCtx");
    	}
    }
    va_end(args);
    (*ctx)->proposedTransferSyntax = lst;
    return DUL_NORMAL;
}

CONDITION
DUL_AddSinglePresentationCtx(DUL_ASSOCIATESERVICEPARAMETERS* params,
							 DUL_SC_ROLE proposedRole, DUL_SC_ROLE acceptedRole,
							 DUL_PRESENTATIONCONTEXTID contextID,
							 unsigned char result, const char* abstractSyntax,
							 const char** xferSyntaxes, int xferSyntaxCount)
{
  LST_HEAD* 				 lst;
  DUL_TRANSFERSYNTAX* 		 transfer;
  DUL_PRESENTATIONCONTEXT* 	 ctx;
  CONDITION 				 cond;


  ctx = (DUL_PRESENTATIONCONTEXT *) CTN_MALLOC(sizeof(*ctx));
  if (ctx == NULL) return COND_PushCondition(DUL_MALLOCERROR, DUL_Message(DUL_MALLOCERROR), "DUL_AddSinglePresentationCtx", sizeof(*ctx));

  (void) memset(ctx, 0, sizeof(*ctx));
  lst = LST_Create();
  if (lst == NULL) return COND_PushCondition(DUL_LISTCREATEFAILED, DUL_Message(DUL_LISTCREATEFAILED), "DUL_AddSinglePresentationCtx");

  (ctx)->presentationContextID = contextID;
  (ctx)->result = result;
  (ctx)->proposedSCRole = proposedRole;
  (ctx)->acceptedSCRole = acceptedRole;
  strcpy((ctx)->abstractSyntax, abstractSyntax);

  strcpy((ctx)->acceptedTransferSyntax, "");

  for (; xferSyntaxCount-- > 0; xferSyntaxes++) {
	  if (strlen(*xferSyntaxes) != 0) {
		  transfer = CTN_MALLOC(sizeof(*transfer));
		  if (transfer == NULL)
			  return COND_PushCondition(DUL_MALLOCERROR, DUL_Message(DUL_MALLOCERROR), "DUL_AddSinglePresentationCtx", sizeof(*transfer));

		  strcpy(transfer->transferSyntax, *xferSyntaxes);
		  if (LST_Enqueue(&lst, transfer) != LST_NORMAL)
			  return COND_PushCondition(DUL_LISTERROR, DUL_Message(DUL_LISTERROR), "DUL_AddSinglePresentationCtx");
	  }
  }

  (ctx)->proposedTransferSyntax = lst;
  cond = LST_Enqueue(&params->requestedPresentationContext,ctx);
  if (cond != LST_NORMAL) return COND_PushCondition(DUL_LISTERROR, DUL_Message(DUL_LISTERROR), "DUL_AddSinglePresentationCtx");
    /* Memory Leak here */

  return DUL_NORMAL;

}

CONDITION
DUL_AddMultiplePresentationCtx(DUL_ASSOCIATESERVICEPARAMETERS* params,
							   DUL_SC_ROLE proposedRole, DUL_SC_ROLE acceptedRole,
							   DUL_PRESENTATIONCONTEXTID contextID,
							   unsigned char reason, const char* abstractSyntax,
							   const char** xferSyntaxes, int xferSyntaxCount)
{
  CONDITION 	cond;

  for (; xferSyntaxCount-- > 0; xferSyntaxes++, contextID += 2) {
	  if (strlen(*xferSyntaxes) != 0) {
		  cond = DUL_AddSinglePresentationCtx(params, proposedRole, acceptedRole, contextID, reason, abstractSyntax, xferSyntaxes, 1);
		  if (cond != DUL_NORMAL) return cond;
    }
  }

  return DUL_NORMAL;
}
