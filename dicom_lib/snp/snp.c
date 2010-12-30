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
** Module Name(s):	SNP_Init
**			SNP_Terminate
**			SNP_RegisterCallback
**			SNP_Start
**			SNP_Stop
**			SNP_Debug
**			streamSetup
			paddr
** Author, Date:	Nilesh R. Gohel, 23-Aug-94
** Intent:		This file contains functions for snooping
**			on a TCP/IP connection over a shared media network.
** Last Update:		$Author: smm $, $Date: 1995/04/07 19:43:47 $
** Source File:		$RCSfile: snp.c,v $
** Revision:		$Revision: 1.20 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.20 $ $RCSfile: snp.c,v $";


#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../snp/snp.h"

#ifdef SNOOP

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <sys/bufmod.h>
#include <sys/pfmod.h>


#include "decode.h"


/* Structure of state message vector */
typedef struct {
    int 	state;
    char 	*message;
}   STATE_VECTOR;

static STATE_VECTOR message_st[] = {
    {NORMAL, "All is well"},
    {END_ASSOC, "End of association"},
    {DATA_OVERFLOW, "Data overflow error"},
    {GETMSG_FAIL, "getmsg() failure"},
    {RESET_ASSOC_INI, "Association aborted by initiator"},
    {RESET_ASSOC_ACC, "Association aborted by acceptor"},
    {NONCONTIGDATA, "Non contiguous data passed to application"},
    {WRITECALLBACKFAIL, "Failure in callback to write data"},
    {LSTINSFAIL, "Failure using LST facility Insert"},
    {DROPPEDPACKETS, "Kernel processing has dropped packets"},
    {BAD_END_ASSOC, "Bad end of association - was not able to capture all data"},
    {CON_TIMEOUT, "Connection timed out (with segments still to be ack'ed)"},
    {STRGETMSG_TIMEOUT, "strgetmsg() timed out (in STREAM setup)"},
};


static struct strbuf data;

static CONDITION
streamSetup(int ppa, char *device, int sap, int bufferSpace, int timeOutBuf);

static void
sigpoll_func();

static void
sigalrm();

static int
strioctl(int fd, int cmd, int timout, int len, char *dp);

static CTNBOOLEAN debug = FALSE;

static void
paddr(char *addr);


/* SNP_Init
**
** Purpose:
**	This function initializes snooping on a TCP/IP connection (SNP).
**
** Parameter Dictionary:
**
**
** Return Values:
**
**	SNP_NORMAL
**  	SNP_MALLOCERROR
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SNP_Init()
{

#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In SNP_Init\n");
#endif
    handle = (SNP_HANDLE *) malloc(sizeof(SNP_HANDLE));
    if (handle == NULL)	return COND_PushCondition(SNP_MALLOCERROR, SNP_Message(SNP_MALLOCERROR), sizeof(SNP_HANDLE), "SNP_Init");
    return SNP_NORMAL;
}


/* SNP_Terminate
**
** Purpose:
**	This function terminates snooping on a TCP/IP connection (SNP).
**
** Parameter Dictionary:
**
**
** Return Values:
**	SNP_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SNP_Terminate()
{
#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In SNP_Terminate\n");
#endif
    free(handle);
    return SNP_NORMAL;
}


/* SNP_RegisterCallback
**
** Purpose:
**	To register the callback functions when TCP/IP snoop facilities have data
**	or state information to deliver to calling application.
**
** Parameter Dictionary:
**	callback 	function to be called to send data or state information back
**			to calling functions
**	callbackType	specifies that the callback function should be called for
**			data in a particular direction or on state information
**
**			SNP_CALLBACK_ITOA	on data from Initiator to
**						Acceptor
**			SNP_CALLBACK_ATOI	on data from Acceptor to
**						Initiator
**			SNP_CALLBACK_STATE	on state information
**
**	ctx		context pointer used by calling functions
**
** Return Values:
**	SNP_NORMAL
**
** Notes:
**	Need to be used atleast once after SNP_Init before SNP_Start
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SNP_RegisterCallback(CONDITION(*callback) (), int callbackType, void *ctx) {
#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In SNP_RegisterCallback\n");
#endif
    switch (callbackType) {
		case SNP_CALLBACK_ITOA:
								handle->callback_itoa = callback;
								handle->itoa_ctx = ctx;
								break;
		case SNP_CALLBACK_ATOI:
								handle->callback_atoi = callback;
								handle->atoi_ctx = ctx;
								break;
		case SNP_CALLBACK_STATE:
								handle->callback_state = callback;
								handle->state_ctx = ctx;
								break;
	}
    return SNP_NORMAL;
}


/* SNP_Start
**
** Purpose:
**	To start the snooping activities of the TCP/IP snoop facility
**
** Parameter Dictionary:
**	device 		shared media network device driver file name on which
**			to be snooping
**				e.g.  Ethernet interface: "/dev/le"
**	ppa		Physical Point of Access (PPA)
**	initiator 	host name of TCP/IP communication initiator
**	acceptor	host name of TCP/IP communication acceptor
**	port 		port number on acceptor that will be used
**	timeOutCon	number of seconds for timeout on connection for which
**			there is no traffic and there are outstanding
**			acknowledgements
**	timeOutBuf	number of seconds for timeout by STREAMS buffer module in
**			kernel
**	bufferSpace	number of bytes of space used for chunks by STREAMS kernel
**			buffer module
**
** Return Values:
**	SNP_NORMAL
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SNP_Start(char *device, int ppa, char *initiator, char *acceptor, int port, int timeOutCon, int timeOutBuf, int bufferSpace)
{
    struct hostent 	*host;
    int 			sap;

#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In SNP_Start\n");
#endif
    /*
     * Make sure callbacks are registered
     */
    if ((handle->callback_state == NULL) || (handle->callback_itoa == NULL) || (handle->callback_atoi == NULL))
    	return COND_PushCondition(SNP_CALLBACKSMISSING, SNP_Message(SNP_CALLBACKSMISSING));

    /*
     * Validate arguments.
     */
    if (ppa < 0) return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR),"ppa", "SNP_Start", "Must be 0 or greater");
    if (!(timeOutCon > 0)) return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "timeOutCon", "SNP_Start", "Must be > 0");
    if (!(timeOutBuf > 0)) return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "timeOutBuf", "SNP_Start", "Must be > 0");
    if (!(bufferSpace > 0))	return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "bufferSpace", "SNP_Start", "Must be > 0");
    if (!(timeOutCon > timeOutBuf))
    	return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "timeOutCon and timeOutBuf", "SNP_Start", "timeOutCon must be > timeOutBuf");

    /*
     * Perform the initialization
     */

    host = gethostbyname(initiator);
    if (host == NULL) return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "initiator", "SNP_Start", "Can not find IP address");
#ifdef DEBUG
    if (debug) {
    	printf("Initiator address = ");
    	paddr(host->h_addr);
    	printf("\n");
    }
#endif
    memcpy(handle->ini_addr, host->h_addr, 4);
    host = gethostbyname(acceptor);
    if (host == NULL) return COND_PushCondition(SNP_ARGERROR, SNP_Message(SNP_ARGERROR), "acceptor", "SNP_Start", "Can not find IP address");
#ifdef DEBUG
    if (debug) {
    	printf("Acceptor address = ");
    	paddr(host->h_addr);
    	printf("\n");
    }
#endif
    memcpy(handle->acc_addr, host->h_addr, 4);
    handle->port = port;
    sap = SAP;
    handle->timeOutCon = timeOutCon;

    /*
     * Open the device.
     */
    if ((handle->fd_in = open(device, 2)) < 0) return COND_PushCondition(SNP_OPENERROR, SNP_Message(SNP_OPENERROR), "DLPI STREAM", "SNP_Start");

    /*
     * Set aside space for buffers
     */
    handle->buffer = (long *) malloc(bufferSpace * 4);
    if (handle->buffer == NULL) return COND_PushCondition(SNP_MALLOCERROR, SNP_Message(SNP_MALLOCERROR), bufferSpace, "SNP_Start");
    handle->bufsize = bufferSpace;

    /*
     * Set up the stream for incoming data
     */
    if ((streamSetup(ppa, device, sap, bufferSpace, timeOutBuf)) != SNP_NORMAL)
    	return COND_PushCondition(SNP_STREAMSETUP, SNP_Message(SNP_STREAMSETUP), "SNP_Start");

    /*
     * Create LSTs to use for segments awaiting acknowledgements
     */
    if ((handle->tcp_list[ITOA] = LST_Create()) == NULL) return COND_PushCondition(SNP_LSTCREATFAIL, SNP_Message(SNP_LSTCREATFAIL), "ITOA", "SNP_Start");
    if ((handle->tcp_list[ATOI] = LST_Create()) == NULL) return COND_PushCondition(SNP_LSTCREATFAIL, SNP_Message(SNP_LSTCREATFAIL), "ATOI", "SNP_Start");

    /*
     * Set up for asynchronous I/O  - interrupt driven on data presence on
     * DLPI stream
     */
    sigset(SIGPOLL, sigpoll_func);

    if (ioctl(handle->fd_in, I_SETSIG, S_RDNORM | S_HIPRI) < 0)	return COND_PushCondition(SNP_SIGSETERROR, SNP_Message(SNP_SIGSETERROR), "SIGPOLL", "SNP_Start");

    data.buf = (char *) handle->buffer;
    data.maxlen = bufferSpace * 4;

    /*
     * Set alarm signal callback to sigalrm in this file
     */
    (void) sigset(SIGALRM, sigalrm);
#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "Leaving SNP_Start - STREAM set up, ready for monitoring\n");
#endif

    return SNP_NORMAL;
}



/* SNP_Stop
**
** Purpose:
**	To stop the snooping activities of the TCP/IP snoop facility
**
** Parameter Dictionary:
**
**
** Return Values:
**	SNP_NORMAL
**	SNP_CLOSEERROR
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
SNP_Stop()
{

    CONDITION 	cond;


#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In SNP_Stop - closing STREAM and cleaning up\n");
#endif
    /*
     * Close incoming DLPI stream
     */
    if ((close(handle->fd_in)) == -1) return COND_PushCondition(SNP_CLOSEERROR, SNP_Message(SNP_CLOSEERROR), "DLPI STREAM", "SNP_Stop");

    /*
     * Cancel the time out condition
     */
    alarm(0);

    /*
     * Free up kernel's memory space for buffering
     */
    free(handle->buffer);

    /*
     * Remove items in the LSTs and destroy them
     */
    cleanLists();

    LST_Destroy(&(handle->tcp_list[ITOA]));
    LST_Destroy(&(handle->tcp_list[ATOI]));

    return SNP_NORMAL;
}



/* sigpoll_func
**
** Purpose:
**	Callback function for asynchronous reading of buffers of IP packets
**	from kernel upon indication that buffer is full or has timed out. If
**	kernel has dropped a packet, it is indicated in headers attached
**	to IP packets.
**
** Parameter Dictionary:
**
**
** Return Values:
**	None
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
sigpoll_func()
{

    int 					flags;
    register char 			*p;
    register struct sb_hdr 	*bp;
    static int 				sigflag;
    int 					check;
    static CONDITION 		cond = SNP_NORMAL;


#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "In sigpoll_func()\n");
#endif
    if (sigflag) {
    	/* callback STATE - data overflow error */
    	cond = handle->callback_state(DATA_OVERFLOW, handle->state_ctx);
#ifdef DEBUG
    	if (debug) fprintf(DEBUG_ERR_DEVICE, "DATA OVERFLOW\n");
#endif
    }
    if (cond == SNP_NORMAL) {
    	/* Setup for reads */
    	data.len = 0;
    	flags = 0;

    	/*
    	 * Set flag such that if SIGPOLL arrives again during this segment,
    	 * can identify an overflow situation
    	 */
    	sigflag = 1;
    	alarm(0);
    	if ((check = getmsg(handle->fd_in, NULL, &data, &flags)) == 0) {
#ifdef DEBUG
    		if (debug) fprintf(DEBUG_ERR_DEVICE, "Data length of buffer = %d\n", data.len);
#endif
    		if (data.len) {
    			for (p = data.buf; p < (char *) (data.buf + data.len); p += bp->sbh_totlen) {
    				bp = (struct sb_hdr *) p;
    				if (bp->sbh_drops) {
    					/* Call CALLBACK STATE and report packets dropped */
    					cond = handle->callback_state(DROPPEDPACKETS, handle->state_ctx);
#ifdef DEBUG
    					if (debug) fprintf(DEBUG_ERR_DEVICE, "Dropped packets\n");
#endif
    				}else{
    					/* Pass the packet for processing */
    					cond = decode(p + sizeof(struct sb_hdr), bp->sbh_msglen);
    				}
    				if (cond == SNP_DONE) p = (char *) (data.buf + data.len);
    			}
    		}
    	}else{
    		/* Insert CALLBACK STATE overflow */
    		cond = handle->callback_state(GETMSG_FAIL, handle->state_ctx);
    	}
    	/*
    	 * Set up alarm signal for connection time out - only set when have
    	 * outstanding segments - else may time out communications that are
    	 * just idle for the time being
    	 */
    	if (((handle->tcp_list[ITOA]) && (LST_Count(&(handle->tcp_list[ITOA])))) || ((handle->tcp_list[ATOI]) && (LST_Count(&(handle->tcp_list[ATOI]))))) {
#ifdef DEBUG
    		if (debug) fprintf(DEBUG_ERR_DEVICE, "Connection time out alarm set\tTime : %d\n", handle->timeOutCon);
#endif
    		alarm(handle->timeOutCon);
    	}
#ifdef DEBUG
    	if (debug) fprintf(DEBUG_ERR_DEVICE, "Made it through sigpoll_func()\n");
#endif
    	/*
    	 * Turn flag off as SIGPOLL did not arrive while servicing interrupt,
    	 * did not run into overflow situation
    	 */
    } else {
    	alarm(0);
    }
    sigflag = 0;
}



/* streamSetup
**
** Purpose:
**	Sets up STREAMS processing from the data link layer in the kernel.
**	Uses the Data Link Provider Interface for STREAMS interface to network
**	device driver. Data in a STREAM from the data link interface in
**	promiscuous mode is processed in the kernel. In the kernel filtering is
**	performed on packets for TCP data of the connection being sought to be
**	snooped on. Packets which make it through the filter are placed in a
**	buffer in the kernel for delivery to the buffer reading funtion when
**	it fills up or times out.
**
** Parameter Dictionary:
**	ppa		Physical point of access
**	device		Network device driver file used for snooping
**	sap 		Service access point for Data Link Provider
**	bufferSpace	Amount of buffer space to be used by kernel (in longs)
**	timeOutBuf	Time before buffer in kernel times out (in seconds)
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
streamSetup(int ppa, char *device, int sap, int bufferSpace, int timeOutBuf)
{
    /*
     * Packet filter design for TCP indication in protocol feild of IP hdr
     */
    struct packetfilt 	for_tcp;
    u_short 			tcp_filt[] = {ENF_PUSH00FF, ENF_PUSHWORD + 11, ENF_AND, ENF_PUSHLIT, TCP_PROTO, ENF_EQ};

    /*
     * Packet filter design for port number in SRC and DST port feilds of TCP
     * hdr
     */
    struct packetfilt 	for_port;
    u_short 			port_filt[] = {ENF_PUSHLIT, ACC_PORT, ENF_PUSHWORD + 17, ENF_COR, ENF_PUSHLIT, ACC_PORT, ENF_PUSHWORD + 18, ENF_CAND};
    /*
     * Packet filter for 2nd half of SRC and DST address with 2nd half of INI
     * address
     */
    struct packetfilt 	for_ini_add2;
    u_short 			ini_add2_filt[] = {ENF_PUSHLIT, 0 /* INI_ADD2 */ , ENF_PUSHWORD + 14, ENF_COR, ENF_PUSHLIT, 0 /* INI_ADD2 */ , ENF_PUSHWORD + 16, ENF_CAND};

    /*
     * Packet filter for 2nd half of SRC and DST address with 2nd half of ACC
     * address
     */
    struct packetfilt 	for_acc_add2;
    u_short 			acc_add2_filt[] = {ENF_PUSHLIT, 0 /* ACC_ADD2 */ , ENF_PUSHWORD + 14, ENF_COR, ENF_PUSHLIT, 0 /* ACC_ADD2 */ , ENF_PUSHWORD + 16, ENF_CAND};

    /*
     * Packet filter for 1st half of SRC and DST address with 1st half of INI
     * address
     */
    struct packetfilt 	for_ini_add1;
    u_short 			ini_add1_filt[] = {ENF_PUSHLIT, 0 /* INI_ADD1 */ , ENF_PUSHWORD + 13, ENF_COR, ENF_PUSHLIT, 0 /* INI_ADD1 */ , ENF_PUSHWORD + 15, ENF_CAND};
    /*
     * Packet filter for 1st half of SRC and DST address with 1st half of ACC
     * address
     */
    struct packetfilt 	for_acc_add1;
    u_short 			acc_add1_filt[] = {ENF_PUSHLIT, 0 /* ACC_ADD1 */ , ENF_PUSHWORD + 13, ENF_COR, ENF_PUSHLIT, 0 /* ACC_ADD1 */ , ENF_PUSHWORD + 15, ENF_CAND};


    u_int 		chunksize;
    u_long 		flag;
    struct 		timeval t;
    CONDITION 	cond;


#ifdef DEBUG
    if (debug) fprintf(DEBUG_ERR_DEVICE, "Performing streamSetup\n");
#endif
    chunksize = bufferSpace * 4;

    /*
     * Attach.
     */
    cond = dlattachreq(handle->fd_in, ppa);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");

    cond = dlokack(handle->fd_in, (char *) handle->buffer);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");

    /*
     * Enable promiscuous mode.
     */
    cond = dlpromisconreq(handle->fd_in, DL_PROMISC_PHYS);
    if (cond != SNP_NORMAL) return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");

    cond = dlokack(handle->fd_in, (char *) handle->buffer);
    if (cond != SNP_NORMAL) return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");


    /*
     * Bind.
     */
    cond = dlbindreq(handle->fd_in, sap, 0, DL_CLDLS, 0, 0);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");

    cond = dlbindack(handle->fd_in, (char *) handle->buffer);
    if (cond != SNP_NORMAL)	return COND_PushCondition(SNP_DLPIFAIL, SNP_Message(SNP_DLPIFAIL), "streamSetup");

    /*
     * Issue DLIOCRAW ioctl
     */
    if (strioctl(handle->fd_in, DLIOCRAW, -1, 0, NULL) < 0)	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "DLIOCRAW", "streamSetup");


    /* Implementation of STREAMS kernel-level packet filter modules  */

    /* Filter on SRC and DST ports with acceptor port number provided */

    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0) return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for port", "streamSetup");

    for_port.Pf_Priority = 0;
    for_port.Pf_FilterLen = 8;
    port_filt[1] = (u_short) handle->port;
    port_filt[5] = (u_short) handle->port;
    memcpy(for_port.Pf_Filter, port_filt, 16);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_port), (char *) &for_port) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for port", "streamSetup");


    /*
     * Filter on 2nd half of SRC and DST addresses with 2nd half of initiator
     * address
     */

    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0) return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for 2nd half ini address", "streamSetup");

    for_ini_add2.Pf_Priority = 0;
    for_ini_add2.Pf_FilterLen = 8;
    memcpy(&ini_add2_filt[1], &(handle->ini_addr[2]), 2);
    memcpy(&ini_add2_filt[5], &(handle->ini_addr[2]), 2);
    memcpy(for_ini_add2.Pf_Filter, ini_add2_filt, 16);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_ini_add2), (char *) &for_ini_add2) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for 2nd half ini address", "streamSetup");


    /*
     * Filter on 2nd half of SRC and DST addresses with 2nd half of acceptor
     * address
     */
    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for 2nd half acc address", "streamSetup");

    for_acc_add2.Pf_Priority = 0;
    for_acc_add2.Pf_FilterLen = 8;
    memcpy(&acc_add2_filt[1], &(handle->acc_addr[2]), 2);
    memcpy(&acc_add2_filt[5], &(handle->acc_addr[2]), 2);
    memcpy(for_acc_add2.Pf_Filter, acc_add2_filt, 16);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_acc_add2), (char *) &for_acc_add2) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for 2nd half acc address", "streamSetup");


    /*
     * Filter on 1st half of SRC and DST addresses with 1st half of initiator
     * address
     */
    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for 1st half ini address", "streamSetup");

    for_ini_add1.Pf_Priority = 0;
    for_ini_add1.Pf_FilterLen = 8;
    memcpy(&ini_add1_filt[1], &(handle->ini_addr[0]), 2);
    memcpy(&ini_add1_filt[5], &(handle->ini_addr[0]), 2);
    memcpy(for_ini_add1.Pf_Filter, ini_add1_filt, 16);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_ini_add1), (char *) &for_ini_add1) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for 1st half ini address", "streamSetup");


    /*
     * Filter on 1st half of SRC and DST addresses with 1st half of acceptor
     * address
     */
    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for 1st half acc address", "streamSetup");

    for_acc_add1.Pf_Priority = 0;
    for_acc_add1.Pf_FilterLen = 8;
    memcpy(&acc_add1_filt[1], &(handle->acc_addr[0]), 2);
    memcpy(&acc_add1_filt[5], &(handle->acc_addr[0]), 2);
    memcpy(for_acc_add1.Pf_Filter, acc_add1_filt, 16);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_acc_add1), (char *) &for_acc_add1) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for 1st half of acc address", "streamSetup");

    /* Filter for TCP segements in the stream */
    if (ioctl(handle->fd_in, I_PUSH, "pfmod") < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH pfmod for TCP", "streamSetup");

    for_tcp.Pf_Priority = 0;
    for_tcp.Pf_FilterLen = 6;
    memcpy(for_tcp.Pf_Filter, tcp_filt, 12);

    if (strioctl(handle->fd_in, PFIOCSETF, -1, sizeof(for_tcp), (char *) &for_tcp) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "PFIOCSETF for TCP", "streamSetup");


    /* Implementation of STREAMS buffer module - bufmod for buffering of data */

    /*
     * Push and configure buffer module.
     */
    if (ioctl(handle->fd_in, I_PUSH, "bufmod") < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_PUSH bufmod", "streamSetup");

    /*
     * Set up timer for chunksize to fill up
     */
    t.tv_sec = timeOutBuf;
    t.tv_usec = 0;
    if (strioctl(handle->fd_in, SBIOCSTIME, -1, sizeof(t), (char *) &t) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "SBIOCSTIME", "streamSetup");

    /*
     * Set chunksize
     */
    if (strioctl(handle->fd_in, SBIOCSCHUNK, -1, sizeof(u_int), (char *) &chunksize) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "SBIOCSCHUNK", "streamSetup");

    /*
     * * Set up the bufmod flags
     */
    if (strioctl(handle->fd_in, SBIOCGFLAGS, -1, sizeof(u_long), (char *) &flag) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "SBIOCGFLAGS", "streamSetup");

    flag |= SB_NO_DROPS;

    if (strioctl(handle->fd_in, SBIOCSFLAGS, -1, sizeof(u_long), (char *) &flag) != 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "SBIOCSFLAGS", "streamSetup");

    /*
     * Flush the read side of the Stream.
     */
    if (ioctl(handle->fd_in, I_FLUSH, FLUSHR) < 0)
    	return COND_PushCondition(SNP_IOCTLFAIL, SNP_Message(SNP_IOCTLFAIL), "I_FLUSH", "streamSetup");

    return SNP_NORMAL;
}



/* sigalrm
**
** Purpose:
**	Callback to timeout of connection
**
** Parameter Dictionary:
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
sigalrm()
{
    /* Call CALLBACK STATE time out condition */
    handle->callback_state(CON_TIMEOUT, handle->state_ctx);
}


/* SNP_StateMsg
**
** Purpose:
**	Find the ASCII message that goes with a state number and
**	return a pointer to static memory containing that message.
**
** Parameter Dictionary:
**	state	The state number
**
** Return Values:
**	The message corresponding to the state number. If no such
**	state exists, a NULL message is returned.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
char *
SNP_StateMsg(int state)
{
    int        index;

    for (index = 0; message_st[index].message != NULL; index++){
    	if (state == message_st[index].state) return message_st[index].message;
    }
    return NULL;
}



/* strioctl
**
** Purpose:
**	To simplify calling of ioctls
**
** Parameter Dictionary:
**	fd		file descriptor on which to call it
**	cmd		command
**	timout		time out for ack
**	len		length of data
**	dp		ptr to data
**
** Return Values:
**	Return value of ioctl on error or length of data on success
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
strioctl(int fd, int cmd, int timout, int len, char *dp)
{
    struct strioctl 	sioc;
    int 				rc;

    sioc.ic_cmd = cmd;
    sioc.ic_timout = timout;
    sioc.ic_len = len;
    sioc.ic_dp = dp;
    rc = ioctl(fd, I_STR, &sioc);

    if (rc < 0){
    	return (rc);
    }else{
    	return (sioc.ic_len);
    }
}


/* SNP_Debug
**
** Purpose:
**	Set debug flag in this module and in the other modules.
**
** Parameter Dictionary:
**	flag	The boolean variable to set the debug facility.
**
** Return Values:
**	None
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
void
SNP_Debug(CTNBOOLEAN flag)
{
    debug = flag;
    decodeDebug(flag);
}


static void
paddr(char *addr)
{
    printf("%u.", addr[0] & 0xff);
    printf("%u.", addr[1] & 0xff);
    printf("%u.", addr[2] & 0xff);
    printf("%u ", addr[3] & 0xff);
}



#else				/* If SNOOP is not defined .... just return the unimplemented code */

static CTNBOOLEAN debug = FALSE;

CONDITION
SNP_Init()
{
    return COND_PushCondition(SNP_UNIMPLEMENTED, SNP_Message(SNP_UNIMPLEMENTED), "SNP_Init");
}


CONDITION
SNP_Terminate()
{
    return COND_PushCondition(SNP_UNIMPLEMENTED, SNP_Message(SNP_UNIMPLEMENTED), "SNP_Terminate");
}


CONDITION
SNP_RegisterCallback(CONDITION(*callback) (), int callbackType, void *ctx)
{
    return COND_PushCondition(SNP_UNIMPLEMENTED, SNP_Message(SNP_UNIMPLEMENTED), "SNP_RegisterCallback");
}


CONDITION
SNP_Start(char *device, int ppa, char *initiator, char *acceptor, int port, int timeOut1, int timeOut2, int bufferSpace)
{

    return COND_PushCondition(SNP_UNIMPLEMENTED,
			      SNP_Message(SNP_UNIMPLEMENTED), "SNP_Start");

}

CONDITION
SNP_Stop()
{
    return COND_PushCondition(SNP_UNIMPLEMENTED, SNP_Message(SNP_UNIMPLEMENTED), "SNP_Stop");
}


char *
SNP_StateMsg(int state)
{
    return NULL;
}

void
SNP_Debug(CTNBOOLEAN flag)
{
    debug = flag;
}
#endif
