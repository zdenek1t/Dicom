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
** Module Name(s):	decode
**			tcp_parse
**			cleanLists
** Author, Date:	Nilesh R. Gohel, 23-Aug-94
** Intent:		Packet decoding and FSM routines for TCP/IP
**			level communications.
** Last Update:		$Author: nilesh $, $Date: 1995/04/14 17:18:28 $
** Source File:		$RCSfile: decode.c,v $
** Revision:		$Revision: 1.23 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.23 $ $RCSfile: decode.c,v $";

#ifdef SNOOP

#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/dlpi.h>
#include	<sys/bufmod.h>
#include	<stdio.h>
#include	<string.h>
#include	<syslog.h>
#include 	<fcntl.h>
#include 	<sys/stat.h>
#include 	<unistd.h>
#include	"dicom.h"
#include 	"lst.h"
#include 	"condition.h"
#include 	"decode.h"
#include 	"snp.h"



/*
	Private functions used by SNP facility
*/

static CONDITION
tcp_parse(register int data_len, register int dir, register u_short tcp_win, char *tcp_data, register u_long tcp_flags, register u_long seq, register u_long ack, register u_long itoa_seq, register u_long atoi_seq, int *isn);
static int
in_between(u_long top, u_long bottom, u_long between);

static CTNBOOLEAN debug = FALSE;
static u_long chk_seq[2];

/* decode
**
** Purpose:
**	Given IP header, extracts TCP segment, and serves as the TCP FSM for snooping
**	functions using the flags that are found in the TCP headers. Selected information
**	including data and ack numbers are passed on for further processing.
**
** Parameter Dictionary:
**	ptr 	Pointer to TCP segment
**	length 	Length of TCP segment
**
** Return Values:
**	SNP_NORMAL 		everything okay .... keep processing
**	SNP_DONE 		end processing ... either due to error or completed
**				monitoring required number of associations
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
decode(char *ptr, int length)
{
    struct tcphdr 	*tp;
    int 			data_len;
    register 		u_long seq, ack;
    static u_long 	itoa_seq, atoi_seq;			/* SYN sequence numbers for conn. setup */
    register int 	direction;					/* Direction of flow ITOA or ATOI */
    static int 		tcp_initiator_st = CLOSED;	/* Initiator state */
    static int 		tcp_acceptor_st = LISTEN;	/* Acceptor state */
    static int 		isn;
    static 			u_short acc_port;
    int 			i, j;
    TCP_LST_ITEM 	*tcp_item;
    CONDITION 		cond;


    tp = (struct tcphdr *) (ptr + ETHER_HDR_LEN);

    /* Calculate data length */

    data_len = (int) tp->len - (int) IP_HDR_LEN - ((int) (tp->th_off >> 4) * 4);

    seq = (((u_long) tp->th_seq1) << 16) | ((u_long) tp->th_seq2);

    ack = (((u_long) tp->th_ack1) << 16) | ((u_long) tp->th_ack2);

    if (!memcmp(tp->iph_src, handle->ini_addr, 4)){
    	direction = ITOA;
    }else{
    	direction = ATOI;
    }
    /*
     * TCP Finite State Machine for Connection Establishment and Termination
     */
    /*
     * Check for abort connection flag received
     */
    if ((!acc_port) || (tp->th_sport == acc_port) || (tp->th_dport == acc_port)) {
    	if (tp->th_flags & TH_RST) {
#ifdef DEBUG
    		if (debug) fprintf(DEBUG_ERR_DEVICE, "\nTCP: RST received ");
#endif
    		if (direction == ITOA) {
    			cond = handle->callback_state(RESET_ASSOC_INI, handle->state_ctx);
#ifdef DEBUG
    			if (debug) fprintf(DEBUG_ERR_DEVICE, "from initiator\n");
#endif
    		}else{
    			cond = handle->callback_state(RESET_ASSOC_ACC, handle->state_ctx);
#ifdef DEBUG
    			if (debug) fprintf(DEBUG_ERR_DEVICE, "from acceptor\n");
#endif
    		}
    		if (cond == SNP_DONE) {
    			/* Remove items in the LSTs */
    			cleanLists();
    			/* Reset the states */
    			tcp_initiator_st = CLOSED;
    			tcp_acceptor_st = LISTEN;
    			return SNP_DONE;
    		}
    	}
    }
    /*
     * For all other flags
     */

    /* Check for connection setup flags first */
    if ((tcp_initiator_st == CLOSED) || (tcp_acceptor_st == LISTEN)	|| (tcp_initiator_st == SYN_SENT)) {
    	if (direction == ITOA) {
    		/* Initiator to acceptor Originally CLOSED, initiator moves to SYN_SENT on SYN or to ESTABLISHED on ACK of acceptor's SYN + ACK */
    		if (tcp_initiator_st == CLOSED) {
    			if ((tp->th_flags & TH_SYN) && (!(tp->th_flags & TH_ACK))) {
    				tcp_initiator_st = SYN_SENT;
    				itoa_seq = seq;
#ifdef DEBUG
    				if (debug) fprintf(DEBUG_ERR_DEVICE, "\nTCP: Initiator sends SYN\n");
#endif
    			}
    		}else{
    			if (tp->th_flags & TH_ACK) {
    				/* Check ACK is in response to SYN + ACK */
    				if (ack == atoi_seq + 1) {
#ifdef DEBUG
    					if (debug) fprintf(DEBUG_ERR_DEVICE, "\nTCP: Initiator's final ACK for ESTABLISHED state\n");
#endif
    					tcp_initiator_st = ESTABLISHED;
    					itoa_seq = seq;
    					atoi_seq = ack;
    					isn = 1;
    					acc_port = tp->th_dport;
    				}
    			}
    		}
    	}else{
    		/* Acceptor to initiator Originally LISTENing, moves to ESTABLISHED on sending out SYN + ACK on initiator's SYN */
    		if (tp->th_flags & TH_SYN) {
    			/* Check that returning SYN + ACK are in response to our original SYN */
    			if (ack == itoa_seq + 1) {
#ifdef DEBUG
    				if (debug) fprintf(DEBUG_ERR_DEVICE, "\nTCP: Acceptor returns SYN + ACK\n");
#endif
    				tcp_acceptor_st = ESTABLISHED;
    				atoi_seq = seq;
    			}
    		}
    	}
    }else{
    	if ((tp->th_sport == acc_port) || (tp->th_dport == acc_port)) {
    		if ((tcp_initiator_st == ESTABLISHED) || (tcp_acceptor_st == ESTABLISHED)) {
    			/* Data transfer mode Use seq and ack numbers to resequence segments and to discard duplicate segments Ship segment* off for parsing */
    			cond = tcp_parse(data_len, direction, tp->th_win, tp->data, tp->th_flags, seq, ack, itoa_seq, atoi_seq, &isn);
    			if (cond == SNP_DONE) {
    				/* Remove items in the LSTs */
    				cleanLists();
    				/* Reset the states */
    				tcp_initiator_st = CLOSED;
    				tcp_acceptor_st = LISTEN;
    				return SNP_DONE;
    			}
    		}
    		if (tp->th_flags & TH_FIN) {
    			if (direction == ITOA) {
    				tcp_initiator_st = FIN_RCVD;
#ifdef DEBUG
    				if (debug) fprintf(DEBUG_ERR_DEVICE, "ITOA FIN: Data len = %d\n", data_len);
#endif
    				itoa_seq = seq;
    			}else{
    				tcp_acceptor_st = FIN_RCVD;
#ifdef DEBUG
    				if (debug) fprintf(DEBUG_ERR_DEVICE, "ATOI FIN: Data len = %d\n", data_len);
#endif
    				atoi_seq = seq;
    			}
#ifdef DEBUG
    			if (debug) {
    				fprintf(DEBUG_ERR_DEVICE, "\nTCP: FIN flag received:\n");
    				fprintf(DEBUG_ERR_DEVICE, "Direction = %d\n", direction);
    				fprintf(DEBUG_ERR_DEVICE, "Current seq = %u\n", seq);
    				fprintf(DEBUG_ERR_DEVICE, "Current ack = %u\n", ack);
    				fprintf(DEBUG_ERR_DEVICE, "Data len = %d\n", data_len);
    				fprintf(DEBUG_ERR_DEVICE, "ITOA LST count %d\n", LST_Count(&(handle->tcp_list[ITOA])));
    				fprintf(DEBUG_ERR_DEVICE, "ATOI LST count %d\n\n", LST_Count(&(handle->tcp_list[ATOI])));
    			}
#endif
    		}
    	}
    }
    if ((tcp_initiator_st == FIN_RCVD) && (tcp_acceptor_st == FIN_RCVD)) {
#ifdef DEBUG
    	if (debug) {
    		printf("ITOA FIN: Check = %u, Actual = %u\n", chk_seq[ITOA], itoa_seq);
    		printf("ATOI FIN: Check = %u, Actual = %u\n", chk_seq[ATOI], atoi_seq);
    	}
#endif
    	if ((chk_seq[ITOA] != itoa_seq) || (chk_seq[ATOI] != atoi_seq)) {
    		/* Call CALLBACK STATE - bad end assoc...segments left still to be ack'ed */
	    	cond = handle->callback_state(BAD_END_ASSOC, handle->state_ctx);
    	}else{
    		/* Call CALLBACK STATE to inform of end of an association */
    		cond = handle->callback_state(END_ASSOC, handle->state_ctx);
    	}
    	/* Reset the states */
    	tcp_initiator_st = CLOSED;
    	tcp_acceptor_st = LISTEN;
    	if (cond == SNP_DONE) {
    		cleanLists();
    		return SNP_DONE;
    	}
    }
    return SNP_NORMAL;
}



/* tcp_parse
**
** Purpose:
**	To parse TCP segments - re-odering segments, discarding duplicate
**	segments and so on. Only accepts data on ACK from receiving end.
**	Uses DICOM LST facility for temporary storage of TCP data segments
**	awaiting ACKs.
**
** Parameter Dictionary:
**	data_len 	Length of data in TCP segment
**	dir		Flag indicating direction of segment
**	tcp_win		Window of outstanding data negotiated by TCP engines
**	tcp_data 	Pointer to data carried by TCP segment
**	tcp_flags 	Flags carried by TCP segment
**	seq		Sequence number of TCP segment
**	ack		Ack number of TCP segment
**	itoa_seq	Initial sequence number in ITOA direction
**	atoi_seq 	Initial sequence number in ATOI direction
**	isn		Flag indicating that above two are initial sequence numbers
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
tcp_parse(register int data_len, register int dir, register u_short tcp_win, char *tcp_data, register u_long tcp_flags, register u_long seq, register u_long ack, register u_long itoa_seq, register u_long atoi_seq, int *isn)
{
    register int 			lst_count, i, where;
    register int 			opp_dir;
	register TCP_LST_ITEM 	*tcp_item;
    CONDITION 				cond;
    register u_long 		ack_chk, seq_chk;
    register u_long 		offset;

    if (*isn) {
    	chk_seq[ITOA] = itoa_seq;
    	chk_seq[ATOI] = atoi_seq;
    	*isn = 0;
    }
    if (dir == ITOA){
    	opp_dir = ATOI;
    }else{
    	opp_dir = ITOA;
    }

    /* If segment carries ACKs ... take care of them first */
    if (tcp_flags & TH_ACK) {
    	/* Check if have a list to check against for ACK */
    	if (LST_Count(&(handle->tcp_list[opp_dir]))) {
    		/*
    		 * If have list ... assume current node is at head end and
    		 * traverse list to find end sequence number that matches ACK.
    		 * Data taken from that node and output to corresponding file.
    		 * Upon extracting data remove node from list. Only keep checking
    		 * while end sequence number of node is less than ACK otherwise
    		 * gone too far
    		 */
    		ack_chk = ack - TCP_WIN;
    		tcp_item = LST_Head(&(handle->tcp_list[opp_dir]));
    		tcp_item = LST_Position(&(handle->tcp_list[opp_dir]), tcp_item);
#ifdef DEBUG
    		if (debug) {
    			fprintf(DEBUG_ERR_DEVICE, "\n\nCheck for ACK :\n");
    			fprintf(DEBUG_ERR_DEVICE, "ack = %u\n", ack);
    			fprintf(DEBUG_ERR_DEVICE, "ack_chk = %u\n", ack_chk);
    			fprintf(DEBUG_ERR_DEVICE, "chk_seq = %u\n", chk_seq[opp_dir]);
    			fprintf(DEBUG_ERR_DEVICE, "1st seg. seq = %u\n", tcp_item->seg_seq);
    			fprintf(DEBUG_ERR_DEVICE, "1st seg. ack = %u\n", tcp_item->seg_ack);
    		}
#endif
    		/*
    		 * Add some leeway such that if ack of previous segment is in
    		 * between seq and ack of this segment ... take only pertinent
    		 * section
    		 */
    		while ((tcp_item != NULL) && (in_between(ack, ack_chk, tcp_item->seg_ack)) && (in_between(tcp_item->seg_ack, tcp_item->seg_seq, chk_seq[opp_dir]))) {
    			offset = chk_seq[opp_dir] - tcp_item->seg_seq;
#ifdef DEBUG
    			if (debug) {
    				if (offset) (void) fprintf(DEBUG_ERR_DEVICE, "offset = %u\n", offset);
    			}
#endif
    			tcp_item = LST_Remove(&(handle->tcp_list[opp_dir]), LST_K_AFTER);
    			if (opp_dir == ITOA){
    				cond = handle->callback_itoa(&(tcp_item->data[offset]), tcp_item->d_len - offset, handle->itoa_ctx);
    			}else{
    				cond = handle->callback_atoi(&(tcp_item->data[offset]), tcp_item->d_len - offset, handle->atoi_ctx);
    			}
    			if (cond != SNP_NORMAL) {
    				cond = handle->callback_state(WRITECALLBACKFAIL, handle->state_ctx);
    				if (cond == SNP_DONE) return SNP_DONE;
    			}
    			chk_seq[opp_dir] = tcp_item->seg_ack;
    			free(tcp_item->data);
    			free((char *) tcp_item);
    			tcp_item = LST_Current(&(handle->tcp_list[opp_dir]));
    			if (tcp_item != NULL) {
#ifdef DEBUG
    				if (debug) {
    					(void) fprintf(DEBUG_ERR_DEVICE, "Others :\n");
    					(void) fprintf(DEBUG_ERR_DEVICE, "seg_seq = %u\n", tcp_item->seg_seq);
    					(void) fprintf(DEBUG_ERR_DEVICE, "seg_ack = %u\n", tcp_item->seg_ack);
    				}
#endif
    			}
    		}
    	}
    }
    /*
     * TCP segments carrying data need to be added to appropriate list If
     * duplicate or out of window, then need to be ignored
     */
    if (data_len) {
#ifdef DEBUG
    	if (debug) {
    		(void) fprintf(DEBUG_ERR_DEVICE, "\nCheck for insert :\n");
    		(void) fprintf(DEBUG_ERR_DEVICE, "seq = %u\n", seq);
    		(void) fprintf(DEBUG_ERR_DEVICE, "chk_seq = %u\n", chk_seq[dir]);
    		(void) fprintf(DEBUG_ERR_DEVICE, "data_len = %d\n\n", data_len);
    	}
#endif
	/*
	 * Check to see if sequence number is within chk_seq[dir] and chk_seq
	 * + TCP_WIN
	 */
    	where = LST_K_AFTER;
    	seq_chk = chk_seq[dir] + TCP_WIN;
    	if (in_between(seq_chk, chk_seq[dir], seq)) {
    		lst_count = (int) LST_Count(&(handle->tcp_list[dir]));
    		if (lst_count) {
    			tcp_item = LST_Head(&(handle->tcp_list[dir]));
    			tcp_item = LST_Position(&(handle->tcp_list[dir]), tcp_item);
    			for (i = 0; i < lst_count; i++) {
    				tcp_item = LST_Current(&(handle->tcp_list[dir]));
#ifdef DEBUG
    				if (debug) (void) fprintf(DEBUG_ERR_DEVICE, "Check with seq = %u\n", tcp_item->seg_seq);
#endif
    				if (seq == tcp_item->seg_seq) {
    					/*
    					 * If segment with seq number found in list, remove
    					 * corresponding segments and position to add the new
    					 * version of segments
    					 */
    					if (tcp_flags & TH_FIN){
    						ack = seq + (u_long) data_len + 1;
    					}else{
    						ack = seq + (u_long) data_len;
    					}
#ifdef DEBUG
    					if (debug) {
    						(void) fprintf(DEBUG_ERR_DEVICE, "Replacement seq = %u\n", seq);
    						(void) fprintf(stderr, "Replacement ack = %u\n\n", ack);
    					}
#endif
    					ack_chk = ack - (u_long) TCP_WIN;
    					/*
    					 * Remove TCP LST items that the current segment
    					 * replaces
    					 */
    					while ((tcp_item != NULL) && (in_between(ack, ack_chk, tcp_item->seg_ack))) {
    						tcp_item = LST_Remove(&(handle->tcp_list[dir]), LST_K_AFTER);
    						lst_count--;
#ifdef DEBUG
    						if (debug) {
    							(void) fprintf(DEBUG_ERR_DEVICE, "Replacing seq = %u\n", tcp_item->seg_seq);
    							(void) fprintf(DEBUG_ERR_DEVICE, "Replacing ack = %u\n\n", tcp_item->seg_ack);
    						}
#endif
    						free(tcp_item->data);
    						free((char *) tcp_item);
    						tcp_item = LST_Current(&(handle->tcp_list[opp_dir]));
    					}
    					/* Setup for insertion */
    					if (i == 0) {
    						if (!lst_count){
    							where = LST_K_AFTER;
    						}else{
    							where = LST_K_BEFORE;
    						}
    					}else{
    						if (lst_count > i){
    							where = LST_K_BEFORE;
    						}else{
    							tcp_item = LST_Head(&(handle->tcp_list[dir]));
    							LST_Position(&(handle->tcp_list[dir]), tcp_item);
    							while (i > 1) {
    								LST_Next(&(handle->tcp_list[dir]));
    								i--;
    							}
    							where = LST_K_AFTER;
    						}
    					}
    					i = lst_count;
    				}else{
    					/*
    					 * Find position for insertion otherwise insert at
    					 * end of list
    					 */
    					seq_chk = tcp_item->seg_seq - TCP_WIN;
    					if (in_between(tcp_item->seg_seq, seq_chk, seq)) {
    						where = LST_K_BEFORE;
    						i = lst_count;
    					}else{
    						where = LST_K_AFTER;
    						if (i < lst_count - 1) tcp_item = LST_Next(&(handle->tcp_list[dir]));
    					}
    				}
    			}
    		}
    		/*
    		 * Add segment to list in appropriate position
    		 */
    		tcp_item = LST_Current(&(handle->tcp_list[dir]));
    		tcp_item = (TCP_LST_ITEM *) malloc(sizeof(TCP_LST_ITEM));
    		tcp_item->data = (char *) malloc(data_len);
    		memcpy(tcp_item->data, tcp_data, data_len);
    		tcp_item->d_len = data_len;
    		tcp_item->seg_seq = seq;
    		if (tcp_flags & TH_FIN) {
    			tcp_item->seg_ack = seq + (u_long) data_len + 1;
    		}else{
    			tcp_item->seg_ack = seq + (u_long) data_len;
    		}

    		cond = LST_Insert(&(handle->tcp_list[dir]), tcp_item, where);
    		if (cond != LST_NORMAL) {
    			cond = handle->callback_state(LSTINSFAIL, handle->state_ctx);
    			if (cond == SNP_DONE) return SNP_DONE;
    		}
    	}
    }
    return SNP_NORMAL;
}

/* cleanLists
**
** Purpose:
**	To clean up lists of outstanding TCP segements
**
** Parameter Dictionary:
**
**
** Return Values:
**
**
** Notes:
**
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
void
cleanLists()
{
    int 			i, j, k;
    TCP_LST_ITEM 	*tcp_item;

#ifdef DEBUG
    if (debug) (void) fprintf(DEBUG_ERR_DEVICE, "In SNP's cleanLists()\n");
#endif
    /* Remove items in the LSTs */
    for (i = 0; i < 2; i++) {
    	if (handle->tcp_list[i] != NULL) {
    		j = LST_Count(&(handle->tcp_list[i]));
    		tcp_item = LST_Head(&(handle->tcp_list[i]));
    		tcp_item = LST_Position(&(handle->tcp_list[i]), tcp_item);

    		while (j) {
    			tcp_item = LST_Current(&(handle->tcp_list[i]));
#ifdef DEBUG
    			if (debug) {
    				(void) fprintf(DEBUG_ERR_DEVICE, "TCP LST item : direction = %d\n\n", i);
    				(void) fprintf(DEBUG_ERR_DEVICE, "seq = %u\n", tcp_item->seg_seq);
    				(void) fprintf(DEBUG_ERR_DEVICE, "ack = %u\n", tcp_item->seg_ack);
    				(void) fprintf(DEBUG_ERR_DEVICE, "d_len = %d\n\n", tcp_item->d_len);
    			}
#endif
    			tcp_item = LST_Remove(&(handle->tcp_list[i]), LST_K_AFTER);
    			free(tcp_item->data);
    			free((char *) tcp_item);
    			j--;
    		}
    	}
    }
}


/* in_between
**
** Purpose:
**	Is a given unsigned long value between two others where one is the top
**	and the other bottom
**
** Parameter Dictionary:
**	top		value considered to be the top (may not be
**			  	due if goes over MAX_U_LONG)
**	bottom		value considered to be the bottom
**	between 	value to be checked as to whether in between
**
** Return Values:
**	1	True
**	0	False
**
**
** Notes:
**
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
in_between(u_long top, u_long bottom, u_long between)
{
    if (top > bottom) {
    	if (!((between > top) || (top < bottom))){
    		return 1;
    	}else{
    		return 0;
    	}
    }else{
    	if (!((between > top) && (between < bottom))){
    		return 1;
    	}else{
    		return 0;
    	}
    }
}

/* decodeDebug
**
** Purpose:
**	Set debug flag in this module.
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
decodeDebug(CTNBOOLEAN flag)
{
    debug = flag;
}

#endif
