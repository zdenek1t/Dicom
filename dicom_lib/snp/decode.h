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
** Module Name(s):
** Author, Date:	Nilesh R. Gohel, 23-Aug-94
** Intent:		This include file defines constants, structures,
**			and prototypes for the SNP facility.
** Last Update:		$Author: smm $, $Date: 1997/09/01 21:21:45 $
** Source File:		$RCSfile: decode.h,v $
** Revision:		$Revision: 1.13 $
** Status:		$State: Exp $
*/

#ifndef DECODE_H_IS_IN
#define DECODE_H_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif


#define u_char 	unsigned char
#define u_short unsigned short
#define u_long  unsigned long
#define u_int   unsigned int

#define ETHER_HDR_LEN 14


/*
 * TCP header. Per RFC 793, September, 1981.
 */
struct tcphdr {
    u_char pad[2];		/* IP info including version, hdr length, TOS */
    u_short len;		/* Total length of IP packet in bytes */
    u_char ip[8];		/* Other IP specific info */
    u_char iph_src[4];		/* IP source address */
    u_char iph_dst[4];		/* IP destination address */
    u_short th_sport;		/* source port */
    u_short th_dport;		/* destination port */
    u_short th_seq1;		/* sequence number */
    u_short th_seq2;		/* Shorts for long word boundry */
    u_short th_ack1;		/* Acknowledgement number */
    u_short th_ack2;		/* Shorts for long word boundry */
    u_char th_off;		/* data offset */
    u_char th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
    u_short th_win;		/* window */
    u_short th_sum;		/* checksum */
    u_short th_urp;		/* urgent pointer */
    char data[4];		/* first word of data */
};

/*
 * Maximum number of seconds we'll wait for any particular DLPI
 * acknowledgment from the provider after issuing a request.
 */
#define		MAXWAIT		15

/*
 * Maximum address buffer length.
 */
#define		MAXDLADDR	1024


/*
 * Handy macros
 */
#define		OFFADDR(s, n)	(u_char*)((char*)(s) + (int)(n))

#define		CASERET(s)	case s:  return ("s")


/*
 * Often used port numbers
 */
#define TELNET_PORT 23
#define SMTP_PORT 	25
#define RLOGIN_PORT 513

#define TCP_PROTO 0x0006
#define ACC_PORT  0x0000		/* Dummy space filler - should be filled with acceptor port */

#define IP_HDR_LEN 20

#define TCP_WIN 16384

/*
 * The IEE802.3 SAP to filter for TCP/IP
 */
#define SAP 2048


/* Defining TCP states (only a subset for snooping purposes) */

#define CLOSED 		1
#define LISTEN 		2
#define SYN_SENT 	3
#define ESTABLISHED 	4
#define FIN_RCVD 	5

#if 0
#define SHORTSIZE 	16
#define LONGSIZE 	32
#endif

/*
	Structure used for storage of TCP segments awaiting ACKs
*/
typedef struct {
    void 	*reserved[2];		/* Reserved */
    u_long 	seg_seq;			/* Segment's sequence number for 1st byte of data */
    u_long 	seg_ack;			/* Segment's acknowledgement number expected for last byte of data */
    int 	d_len;				/* Length of data in segement */
    char 	*data;				/* Pointer to segment's data */
}   TCP_LST_ITEM;


/*
	Handle structure used thoughout SNP facility
*/
typedef struct {
    int 						fd_in;			/* File descriptor for incoming data STREAM */
    CONDITION(*callback_itoa) 	();				/* Callback function for delivery of segment ini->acc */
    void 						*itoa_ctx;		/* Context pointer for callback_itoa call */
    CONDITION(*callback_atoi) 	();				/* Callback function for delivery of segment acc->ini */
    void 						*atoi_ctx;		/* Context pointer for callback_atoi call */
    CONDITION(*callback_state) 	();				/* Callback function for reporting state information */
    void 						*state_ctx;		/* Context pointer for callback_state call */
    int 						bufsize;		/* Size of packet being processed */
    long 						*buffer;		/* Pointer to packet being processed */
    LST_HEAD 					*tcp_list[2];	/* Headers for LSTs */
    u_char 						ini_addr[4];	/* Initiator IP address */
    u_char 						acc_addr[4];	/* Acceptor IP address */
    int 						port;			/* Port being used on acceptor */
    int 						timeOutCon;		/* Time out for connection */
}   SNP_HANDLE;

SNP_HANDLE *handle;		/* Global pointer to handle structure */

CONDITION
decode(char *ptr, int length);
void
cleanLists();
void
decodeDebug(CTNBOOLEAN flag);

/*
	Prototypes for DLPI operations found in dlroutines.c
*/

CONDITION
dlbindreq(int fd, u_long sap, u_long max_conind, u_long service_mode, u_long conn_mgmt, u_long xidtest);
CONDITION
dlunbindreq(int fd);
CONDITION
dldetachreq(int fd);
CONDITION
dlbindack(int fd, char *bufp);
CONDITION
dlattachreq(int fd, u_long ppa);
CONDITION
dlokack(int fd, char *bufp);
CONDITION
dlpromisconreq(int fd, u_long level);

#ifdef  __cplusplus
}
#endif

#endif
