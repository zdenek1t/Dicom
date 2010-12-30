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
** Module Name(s):
** Author, Date:	Stephen M. Moore, 27-Apr-93
** Intent:		Define any constants and function prototypes for
**			functions/features that are private to the
**			services facility.
** Last Update:		$Author: smm $, $Date: 1996/08/23 20:06:07 $
** Source File:		$RCSfile: private.h,v $
** Revision:		$Revision: 1.10 $
** Status:		$State: Exp $
*/

#ifdef  __cplusplus
extern "C" {
#endif

#define	DEBUG_DEVICE	stdout

DUL_PRESENTATIONCONTEXT *
SRVPRV_PresentationContext(
		   DUL_ASSOCIATESERVICEPARAMETERS * params, char *classUID);
CONDITION
SRVPRV_ReadNextPDV(DUL_ASSOCIATIONKEY ** association,
		   DUL_BLOCKOPTIONS block, int timeout, DUL_PDV * pdv);

#ifdef  __cplusplus
}
#endif
