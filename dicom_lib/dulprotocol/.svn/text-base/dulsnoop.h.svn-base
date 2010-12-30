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
** Author, Date:	Nilesh R. Gohel, 29-Sep-1994
** Intent:		This module defines structures and constants needed
**			to implement DUL snooping facilities.
** Last Update:		$Author: smm $, $Date: 1996/08/23 19:26:34 $
** Source File:		$RCSfile: dulsnoop.h,v $
** Revision:		$Revision: 1.6 $
** Status:		$State: Exp $
*/

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    int 	event;
    int 	state;
    int 	next_state;
}   PDU_FSM_ENTRY;

#define NO_OF_PDU_EVENTS 16

/* State numbers */

#define A_ASSOC_RQ_SENT		0
#define A_ASSOC_RQ_RCVD 	1
#define A_ASSOC_AC_SENT 	2
#define A_ASSOC_AC_RCVD 	3
#define A_ASSOC_RJ_SENT 	4
#define A_ASSOC_RJ_RCVD 	5
#define P_DATA_SENT 		6
#define P_DATA_RCVD			7
#define A_REL_RQ_SENT		8
#define A_REL_RQ_RCVD  		9
#define A_REL_RP_SENT		10
#define A_REL_RP_RCVD		11
#define A_ABORT_SENT		12
#define A_ABORT_RCVD		13
#define INVALID_PDU_SENT	14
#define INVALID_PDU_RCVD	15

#ifdef  __cplusplus
}
#endif
