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
** Module Name(s):	iqueues.h
** Author, Date:	David E. Beecher, 28-Jun-93
** Intent:		Queue definitions for communications
**			between clients/servers and ctndisp and
**			ctnnetwork.
** Last Update:		$Author: smm $, $Date: 1994/12/30 00:12:40 $
** Source File:		$RCSfile: iqueues.h,v $
** Revision:		$Revision: 1.2 $
** Status:		$State: Exp $
*/

#define CONN_INOPEN	-1
#define CONN_INCLOSE	-2
#define CONN_INXFER	-3
#define CONN_INDISPLAY  -4
#define CONN_OUTOPEN	-5
#define CONN_OUTCLOSE	-6
#define CONN_OUTXFER	-7
#define CONN_OUTDISPLAY -8

/*
 * Semantics for CTNDISP_Queue
 *
 *	If connection is CONN_INOPEN (CONN_OUTOPEN) then
 *		An INCOMING (OUTGOING) connection has been opened.
 *      If connection is CONN_INCLOSE (CONN_OUTCLOSE) then
 *		An INCOMING (OUTGOING) connection has been closed.
 *      If connection is CONN_INXFER (CONN_OUTXFER) then
 *		Update INCOMING (OUTGOING) image number with inumber.
 *      If connection is CONN_INDISPLAY (CONN_OUTDISPLAY) then
 *		Display the image identified by imagefile.
 *
 */

typedef struct _CTNDISP_Queue {
    char
        imagefile[256],
        dpnid[256];
    short
        connection,
        inumber;
}   CTNDISP_Queue;

/*
 * Semantics for CTNNETWORK_Queue
 *
 *	If connection is CONN_INOPEN (CONN_OUTOPEN) then
 *		An INCOMING (OUTGOING) connection has been opened.
 *      If connection is CONN_INCLOSE (CONN_OUTCLOSE) then
 *		An INCOMING (OUTGOING) connection has been closed.
 *      If connection is CONN_INXFER (CONN_OUTXFER) then
 *		Update INCOMING (OUTGOING) percentage with percentage.
 *      If connection is CONN_INDISPLAY (CONN_OUTDISPLAY) then
 *		CONN_INDISPLAY (CONN_OUTDISPLAY) has no meaning here.
 *	The connection is uniquely identified by association_id.
 *
 */
typedef struct _CTNNETWORK_Queue {
    char
        vendorid[64],
        dpnid[64];
    long
        association_id;
    short
        connection,
        percentage;
}   CTNNETWORK_Queue;
