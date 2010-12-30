/*
          Copyright (C) 1994, RSNA and Washington University

          The software and supporting documentation for the Radiological
          Society of North America (RSNA) 1994 Digital Imaging and
          Communications in Medicine (DICOM) Demonstration were developed
          at the
                  Electronic Radiology Laboratory
                  Mallinckrodt Institute of Radiology
                  Washington University School of Medicine
                  510 S. Kingshighway Blvd.
                  St. Louis, MO 63110
          as part of the 1994 DICOM Central Test Node project for, and
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
/*
** @$=@$=@$=
*/
/*
**				DICOM 94
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	dlbindreq
**			dlunbindreq
**			dldetachreq
**			dlbindack
**			dlattachreq
**			dlokack
**			dlpromisconreq
** Author, Date:	Neal Nuckolls of Sun Microsystems, Inc.
**			(nn@eng.sun.com), 30-Jun-1992
**			Modified by Nilesh R. Gohel, 23-Aug-94
** Intent:		Generic Data Link Provider Interface (DLPI)
**			routines for snooping on a Solaris 2.X system.
** Last Update:		$Author: nilesh $, $Date: 1995/03/22 01:39:39 $
** Source File:		$RCSfile: dlroutines.c,v $
** Revision:		$Revision: 1.6 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.6 $ $RCSfile: dlroutines.c,v $";

#ifdef SNOOP

#include	<sys/types.h>
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/dlpi.h>
#include	<sys/signal.h>
#include	<stdio.h>
#include	<string.h>
#include 	"dicom.h"
#include 	"condition.h"
#include 	"lst.h"
#include 	"decode.h"
#include	"snp.h"

/* Prototypes defined here */


static void sigalrm();
static CONDITION strgetmsg(int fd, struct strbuf * ctlp, struct strbuf * datap, int *flagsp, char *caller);
static CONDITION expecting(int prim, union DL_primitives * dlp);
static char *dlprim(u_long prim);


/* dlbindreq
**
** Purpose:
**	To request binding a datalink-level service access point to the STREAM
**
** Parameter Dictionary:
**	fd		File descriptor
**	sap 		Service access point
**	max_conind	Maximum number of outstanding connection indications
**	service_mode	service mode requested
**	conn_mgmt	if non-zero indicates cannection mgmt stream
**	xidtest		Flag to indicate XID and/or TEST responses
**
** Return Values:
**	SNP_NORMAL
**	SNP_PUTMSGFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlbindreq(int fd, u_long sap, u_long max_conind, u_long service_mode, u_long conn_mgmt, u_long xidtest)
{
    dl_bind_req_t 	bind_req;
    struct strbuf 	ctl;
    int 			flags;

    bind_req.dl_primitive = DL_BIND_REQ;
    bind_req.dl_sap = sap;
    bind_req.dl_max_conind = max_conind;
    bind_req.dl_service_mode = service_mode;
    bind_req.dl_conn_mgmt = conn_mgmt;
    bind_req.dl_xidtest_flg = xidtest;

    ctl.maxlen = 0;
    ctl.len = sizeof(bind_req);
    ctl.buf = (char *) &bind_req;

    flags = 0;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0){
    	return COND_PushCondition(SNP_PUTMSGFAIL, SNP_Message(SNP_PUTMSGFAIL), "dlbindreq");
    }else{
    	return SNP_NORMAL;
    }
}

/* dlunbindreq
**
** Purpose:
**	To request unbinding a datalink-level service access point
**	to the STREAM
**
** Parameter Dictionary:
**	fd		File descriptor
**
** Return Values:
**	SNP_NORMAL
**	SNP_PUTMSGFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlunbindreq(int fd)
{
    dl_unbind_req_t 	unbind_req;
    struct strbuf 		ctl;
    int 				flags;

    unbind_req.dl_primitive = DL_UNBIND_REQ;

    ctl.maxlen = 0;
    ctl.len = sizeof(unbind_req);
    ctl.buf = (char *) &unbind_req;

    flags = 0;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0){
    	return COND_PushCondition(SNP_PUTMSGFAIL, SNP_Message(SNP_PUTMSGFAIL), "dlunbindreq");
    }else{
    	return SNP_NORMAL;
    }
}

/* dldetachreq
**
** Purpose:
**	To request the datalink-level service (DLS) provider to detach
**	a physical point of attachment (PPA)
**
** Parameter Dictionary:
**	fd		File descriptor
**
** Return Values:
**	SNP_NORMAL
**	SNP_PUTMSGFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dldetachreq(int fd)
{
    dl_detach_req_t 	detach_req;
    struct strbuf 		ctl;
    int 				flags;

    detach_req.dl_primitive = DL_DETACH_REQ;

    ctl.maxlen = 0;
    ctl.len = sizeof(detach_req);
    ctl.buf = (char *) &detach_req;

    flags = 0;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0){
    	return COND_PushCondition(SNP_PUTMSGFAIL, SNP_Message(SNP_PUTMSGFAIL), "dldetachreq");
    }else{
    	return SNP_NORMAL;
    }
}


/* dlbindack
**
** Purpose:
**	To get response from bind request
**
** Parameter Dictionary:
**	fd		File descriptor
**	bufp		Pointer to buffer for response
**
** Return Values:
**	SNP_NORMAL
**	SNP_DLPIFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlbindack(int fd, char *bufp)
{
    union DL_primitives 	*dlp;
    struct strbuf 			ctl;
    int 					flags;
    CONDITION 				cond;

    ctl.maxlen = handle->bufsize;
    ctl.len = 0;
    ctl.buf = bufp;

    strgetmsg(fd, &ctl, (struct strbuf *) NULL, &flags, "dlbindack");

    dlp = (union DL_primitives *) ctl.buf;

    cond = expecting(DL_BIND_ACK, dlp);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "dlbindack");

    /*
     * For debugging purposes
     * 
     * if (flags != RS_HIPRI) fprintf(stderr, "dlbindack:  DL_OK_ACK was not
     * M_PCPROTO\n");
     * 
     * if (ctl.len < sizeof(dl_bind_ack_t)) fprintf(stderr, "dlbindack:  short
     * response ctl.len:  %d\n", ctl.len);
     */

    return SNP_NORMAL;
}


/* dlattachreq
**
** Purpose:
**	To request the datalink-level service (DLS) provider to attach
**	a physical point of attachment (PPA)
**
** Parameter Dictionary:
**	fd		File descriptor
**	bufp		Pointer to buffer for response
**
** Return Values:
**	SNP_NORMAL
**	SNP_DLPIFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlattachreq(int fd, u_long ppa)
{
    dl_attach_req_t 	attach_req;
    struct strbuf 		ctl;
    int 				flags;

    attach_req.dl_primitive = DL_ATTACH_REQ;
    attach_req.dl_ppa = ppa;

    ctl.maxlen = 0;
    ctl.len = sizeof(attach_req);
    ctl.buf = (char *) &attach_req;

    flags = 0;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0){
    	return COND_PushCondition(SNP_PUTMSGFAIL, SNP_Message(SNP_PUTMSGFAIL), "dlattachreq");
    }else{
    	return SNP_NORMAL;
    }
}

/* dlokack
**
** Purpose:
**	To get response from most datalink-level service requests
**	in the form of the primitive DL_OK_ACK
**
** Parameter Dictionary:
**	fd		File descriptor
**	bufp		Pointer to buffer for response
**
** Return Values:
**	SNP_NORMAL
**	SNP_DLPIFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlokack(int fd, char *bufp)
{
    union DL_primitives 	*dlp;
    struct strbuf 			ctl;
    int 					flags;
    CONDITION 				cond;

    ctl.maxlen = handle->bufsize;
    ctl.len = 0;
    ctl.buf = bufp;

    strgetmsg(fd, &ctl, (struct strbuf *) NULL, &flags, "dlokack");

    dlp = (union DL_primitives *) ctl.buf;

    cond = expecting(DL_OK_ACK, dlp);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "dlokack");

    /*
     * For debugging purposes
     * 
     * if (ctl.len < sizeof(dl_ok_ack_t)) fprintf(stderr, "dlokack:  response
     * ctl.len too short:  %d\n", ctl.len);
     * 
     * if (flags != RS_HIPRI) fprintf(stderr, "dlokack:  DL_OK_ACK was not
     * M_PCPROTO\n");
     * 
     * if (ctl.len < sizeof(dl_ok_ack_t)) fprintf(stderr, "dlokack:  short
     * response ctl.len:  %d\n", ctl.len);
     */

    return SNP_NORMAL;
}


/* dlpromisconreq
**
** Purpose:
**	To request the datalink-service provider to enable promiscuous
**	mode at the physical or SAP level
**
** Parameter Dictionary:
**	fd		File descriptor
**	level		Level at which DLS provider placed in
**			promiscuous mode
**
** Return Values:
**	SNP_NORMAL
**	SNP_PUTMSGFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
dlpromisconreq(int fd, u_long level)
{
    dl_promiscon_req_t 	promiscon_req;
    struct strbuf 		ctl;
    int 				flags;

    promiscon_req.dl_primitive = DL_PROMISCON_REQ;
    promiscon_req.dl_level = level;

    ctl.maxlen = 0;
    ctl.len = sizeof(promiscon_req);
    ctl.buf = (char *) &promiscon_req;

    flags = 0;

    if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0){
    	return COND_PushCondition(SNP_PUTMSGFAIL, SNP_Message(SNP_PUTMSGFAIL), "dlpromisconreq");
    }else{
    	return SNP_NORMAL;
    }
}


/* sigalrm
**
** Purpose:
**	On SIGALRM signal indicating getmsg timeout, send appropriate
**	SNP state info
**
** Parameter Dictionary:
**
**
** Return Values:
**
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
sigalrm()
{
    /* Call CALLBACK STATE time out condition */
    handle->callback_state(STRGETMSG_TIMEOUT);
}


/* strgetmsg
**
** Purpose:
**	A more elaborate getmsg function with timeout facility
**
** Parameter Dictionary:
**	fd		File descriptor
**	ctlp		Pointer to buffer for control info
**	datap		Pointer to buffer for data
**	flagsp		Flags
**
**
** Return Values:
**	SNP_NORMAL
**	SNP_DLPIFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
strgetmsg(int fd, struct strbuf * ctlp, struct strbuf * datap, int *flagsp, char *caller)
{
    int 			rc;
    static char 	errmsg[80];

    /*
     * Start timer.
     */
    (void) signal(SIGALRM, sigalrm);
    if (alarm(MAXWAIT) < 0)	return COND_PushCondition(SNP_ALARMSET, SNP_Message(SNP_ALARMSET), "strgetmsg");

    /*
     * Set flags argument and issue getmsg().
     */
    *flagsp = 0;
    if ((rc = getmsg(fd, ctlp, datap, flagsp)) < 0)	return COND_PushCondition(SNP_GETMSGFAIL, SNP_Message(SNP_GETMSGFAIL), "strgetmsg");

    /*
     * Stop timer.
     */
    if (alarm(0) < 0) return COND_PushCondition(SNP_ALARMSET, SNP_Message(SNP_ALARMSET), "strgetmsg");

    /*
     * Check for MOREDATA and/or MORECTL on debug
     * 
     * if ((rc & (MORECTL | MOREDATA)) == (MORECTL | MOREDATA)) fprintf(stderr,
     * "%s:  MORECTL|MOREDATA\n", caller); if (rc & MORECTL) fprintf(stderr,
     * "%s:  MORECTL\n", caller); if (rc & MOREDATA) fprintf(stderr, "%s:
     * MOREDATA\n", caller);
     */

    /*
     * Check for at least sizeof (long) control data portion on debug
     * 
     * if (ctlp->len < sizeof(long)) fprintf(stderr, "getmsg:  control portion
     * length < sizeof (long):  %d\n", ctlp->len);
     */

    return SNP_NORMAL;
}


/* expecting
**
** Purpose:
**	To inspect return primitive and report error if does not match
**
** Parameter Dictionary:
**	prim		primitive expected
**	dlp		structure holding primitive returned
**
**
** Return Values:
**	SNP_NORMAL
**	SNP_DLPIEXPECT
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
expecting(int prim, union DL_primitives * dlp)
{
    if (dlp->dl_primitive != (u_long) prim)	return COND_PushCondition(SNP_DLPIEXPECT, SNP_Message(SNP_DLPIEXPECT), dlprim(prim), dlprim(dlp->dl_primitive));
    return SNP_NORMAL;
}



/* dlprim
**
** Purpose:
**	To return ASCII representation of primitive number
**
** Parameter Dictionary:
**	prim		primitive
**
** Return Values:
**	Pointer to character representation of primitive number
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static char *
dlprim(u_long prim)
{
    static char primbuf[80];

    switch ((int) prim) {
		CASERET(DL_ATTACH_REQ);
		CASERET(DL_DETACH_REQ);
		CASERET(DL_BIND_REQ);
		CASERET(DL_BIND_ACK);
		CASERET(DL_UNBIND_REQ);
		CASERET(DL_OK_ACK);
		CASERET(DL_ERROR_ACK);
		CASERET(DL_SUBS_BIND_REQ);
		CASERET(DL_SUBS_BIND_ACK);
		CASERET(DL_UDERROR_IND);
		CASERET(DL_UDQOS_REQ);
		CASERET(DL_CONNECT_REQ);
		CASERET(DL_CONNECT_IND);
		CASERET(DL_CONNECT_RES);
		CASERET(DL_CONNECT_CON);
		CASERET(DL_TOKEN_REQ);
		CASERET(DL_TOKEN_ACK);
		CASERET(DL_DISCONNECT_REQ);
		CASERET(DL_DISCONNECT_IND);
		CASERET(DL_RESET_REQ);
		CASERET(DL_RESET_IND);
		CASERET(DL_RESET_RES);
		CASERET(DL_RESET_CON);

		default:
					(void) sprintf(primbuf, "unknown primitive 0x%x", prim);
					return (primbuf);
    }
}

#endif
