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
**                              DICOM 94
**                   Electronic Radiology Laboratory
**                 Mallinckrodt Institute of Radiology
**              Washington University School of Medicine
**
** Module Name(s):	DUL_NetworkSnoop
**			DUL_FileSnoop
**			DUL_RegPDUCall
** Author, Date:	Nilesh R. Gohel, 14-Sep-1994
** Intent:		These modules allow the user to snoop on one or more
**			associations at the DICOM level on a shared media
**			network given the name of the initiator, name of the
**			acceptor, and the port number on the acceptor. Output of
**			the modules provide addresses of DICOM PDUs in callback
**			functions along with state information of initiator and
**			acceptor. A variety of parameters of the communication
**			are also tracked and provided in a
**			DUL_ASSOCIATESERVICEPARAMETERS structure.
**			Additional information required includes information on
**			the network interface to be used and size of buffers for
**			temporary storage of data. Conversely, parsed TCP/IP
**			data of DICOM associations stored in files, may be read
**			and data and parsed for DICOM PDUs.
**			The facility uses the SNP facility for snooping.
**
** Last Update:         $Author: smm $, $Date: 2001/12/21 16:52:03 $
** Source File:         $RCSfile: dulsnoop.c,v $
** Revision:            $Revision: 1.38 $
** Status:              $State: Exp $
*/

static char rcsid[] = "$Revision: 1.38 $ $RCSfile: dulsnoop.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#ifdef SUNOS
#include <sys/types.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "../dicom/dicom.h"
#include "../lst/lst.h"
#include "../condition/condition.h"
#include "../uid/dicom_uids.h"
#include "../snp/snp.h"
#include "dulprotocol.h"
#include "dulfsm.h"
#include "dulstructures.h"
#include "dulprivate.h"

#ifdef SNOOP

#include "dulsnoop.h"

#define RING_SIZE 32768		/* Size of ring buffers to be used */
#define BUFFERSIZE 2048		/* Size of buffers for reads from files */

/*
	Global structure used to register callbacks for DICOM PDUs
*/
static struct {
    void (*callITOA) ();
    void *ITOActx;
    void (*callATOI) ();
    void *ATOIctx;
}   callPDU;


/*
	Prototypes for internal functions
*/
static CONDITION callbackState(int state, void *ctx);
static CONDITION callbackITOA(char *buffer, int bufsize, void *ctx);
static CONDITION callbackATOI(char *buffer, int bufsize, void *ctx);
static void PDU_FSM_init(char *intiator, char *acceptor);
static CONDITION fill_rq_params(u_char * buf, u_long pdulen);
static CONDITION fill_ac_params(u_char * buf, u_long pdulen);
static CONDITION fill_rj_params(u_char * buf);

static int 			current_state = NORMAL;	/* Current state reported by SNP facilities */
volatile int 		ini_state = STATE4;	/* Initial states for DUL monitoring FSM */
volatile int 		acc_state = STATE2;
static DUL_ASSOCIATESERVICEPARAMETERS 	params;
static char 		*ini, *acc;
volatile int 		assoc;
static CTNBOOLEAN 	debug = FALSE;

static PDU_FSM_ENTRY pduStateTable[NO_OF_PDU_EVENTS][DUL_NUMBER_OF_STATES] = {
    {
	{A_ASSOC_RQ_SENT, STATE1, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE2, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE3, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE4, STATE5},
	{A_ASSOC_RQ_SENT, STATE5, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE6, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE7, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE8, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE9, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE10, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE11, NOSTATE},
	{A_ASSOC_RQ_SENT, STATE12, NOSTATE},
    {A_ASSOC_RQ_SENT, STATE13, NOSTATE}},

    {
	{A_ASSOC_RQ_RCVD, STATE1, NOSTATE},
	{A_ASSOC_RQ_RCVD, STATE2, STATE3},
	{A_ASSOC_RQ_RCVD, STATE3, STATE13},
	{A_ASSOC_RQ_RCVD, STATE4, NOSTATE},
	{A_ASSOC_RQ_RCVD, STATE5, STATE13},
	{A_ASSOC_RQ_RCVD, STATE6, STATE13},
	{A_ASSOC_RQ_RCVD, STATE7, STATE13},
	{A_ASSOC_RQ_RCVD, STATE8, STATE13},
	{A_ASSOC_RQ_RCVD, STATE9, STATE13},
	{A_ASSOC_RQ_RCVD, STATE10, STATE13},
	{A_ASSOC_RQ_RCVD, STATE11, STATE13},
	{A_ASSOC_RQ_RCVD, STATE12, STATE13},
    {A_ASSOC_RQ_RCVD, STATE13, STATE13}},
    {
	{A_ASSOC_AC_SENT, STATE1, NOSTATE},
	{A_ASSOC_AC_SENT, STATE2, NOSTATE},
	{A_ASSOC_AC_SENT, STATE3, STATE6},
	{A_ASSOC_AC_SENT, STATE4, NOSTATE},
	{A_ASSOC_AC_SENT, STATE5, NOSTATE},
	{A_ASSOC_AC_SENT, STATE6, NOSTATE},
	{A_ASSOC_AC_SENT, STATE7, NOSTATE},
	{A_ASSOC_AC_SENT, STATE8, NOSTATE},
	{A_ASSOC_AC_SENT, STATE9, NOSTATE},
	{A_ASSOC_AC_SENT, STATE10, NOSTATE},
	{A_ASSOC_AC_SENT, STATE11, NOSTATE},
	{A_ASSOC_AC_SENT, STATE12, NOSTATE},
    {A_ASSOC_AC_SENT, STATE13, NOSTATE}},
    {
	{A_ASSOC_AC_RCVD, STATE1, NOSTATE},
	{A_ASSOC_AC_RCVD, STATE2, STATE13},
	{A_ASSOC_AC_RCVD, STATE3, STATE13},
	{A_ASSOC_AC_RCVD, STATE4, NOSTATE},
	{A_ASSOC_AC_RCVD, STATE5, STATE6},
	{A_ASSOC_AC_RCVD, STATE6, STATE13},
	{A_ASSOC_AC_RCVD, STATE7, STATE13},
	{A_ASSOC_AC_RCVD, STATE8, STATE13},
	{A_ASSOC_AC_RCVD, STATE9, STATE13},
	{A_ASSOC_AC_RCVD, STATE10, STATE13},
	{A_ASSOC_AC_RCVD, STATE11, STATE13},
	{A_ASSOC_AC_RCVD, STATE12, STATE13},
    {A_ASSOC_AC_RCVD, STATE13, STATE13}},
    {
	{A_ASSOC_RJ_SENT, STATE1, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE2, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE3, STATE13},
	{A_ASSOC_RJ_SENT, STATE4, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE5, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE6, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE7, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE8, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE9, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE10, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE11, NOSTATE},
	{A_ASSOC_RJ_SENT, STATE12, NOSTATE},
    {A_ASSOC_RJ_SENT, STATE13, NOSTATE}},
    {
	{A_ASSOC_RJ_RCVD, STATE1, NOSTATE},
	{A_ASSOC_RJ_RCVD, STATE2, STATE13},
	{A_ASSOC_RJ_RCVD, STATE3, STATE13},
	{A_ASSOC_RJ_RCVD, STATE4, NOSTATE},
	{A_ASSOC_RJ_RCVD, STATE5, STATE1},
	{A_ASSOC_RJ_RCVD, STATE6, STATE13},
	{A_ASSOC_RJ_RCVD, STATE7, STATE13},
	{A_ASSOC_RJ_RCVD, STATE8, STATE13},
	{A_ASSOC_RJ_RCVD, STATE9, STATE13},
	{A_ASSOC_RJ_RCVD, STATE10, STATE13},
	{A_ASSOC_RJ_RCVD, STATE11, STATE13},
	{A_ASSOC_RJ_RCVD, STATE12, STATE13},
    {A_ASSOC_RJ_RCVD, STATE13, STATE13}},
    {
	{P_DATA_SENT, STATE1, NOSTATE},
	{P_DATA_SENT, STATE2, NOSTATE},
	{P_DATA_SENT, STATE3, NOSTATE},
	{P_DATA_SENT, STATE4, NOSTATE},
	{P_DATA_SENT, STATE5, NOSTATE},
	{P_DATA_SENT, STATE6, STATE6},
	{P_DATA_SENT, STATE7, NOSTATE},
	{P_DATA_SENT, STATE8, STATE8},
	{P_DATA_SENT, STATE9, NOSTATE},
	{P_DATA_SENT, STATE10, NOSTATE},
	{P_DATA_SENT, STATE11, NOSTATE},
	{P_DATA_SENT, STATE12, NOSTATE},
    {P_DATA_SENT, STATE13, NOSTATE}},
    {
	{P_DATA_RCVD, STATE1, NOSTATE},
	{P_DATA_RCVD, STATE2, STATE2},
	{P_DATA_RCVD, STATE3, STATE3},
	{P_DATA_RCVD, STATE4, NOSTATE},
	{P_DATA_RCVD, STATE5, STATE13},
	{P_DATA_RCVD, STATE6, STATE6},
	{P_DATA_RCVD, STATE7, STATE7},
	{P_DATA_RCVD, STATE8, STATE13},
	{P_DATA_RCVD, STATE9, STATE13},
	{P_DATA_RCVD, STATE10, STATE13},
	{P_DATA_RCVD, STATE11, STATE13},
	{P_DATA_RCVD, STATE12, STATE13},
    {P_DATA_RCVD, STATE13, STATE13}},
    {
	{A_REL_RQ_SENT, STATE1, NOSTATE},
	{A_REL_RQ_SENT, STATE2, NOSTATE},
	{A_REL_RQ_SENT, STATE3, NOSTATE},
	{A_REL_RQ_SENT, STATE4, NOSTATE},
	{A_REL_RQ_SENT, STATE5, NOSTATE},
	{A_REL_RQ_SENT, STATE6, STATE7},
	{A_REL_RQ_SENT, STATE7, NOSTATE},
	{A_REL_RQ_SENT, STATE8, NOSTATE},
	{A_REL_RQ_SENT, STATE9, NOSTATE},
	{A_REL_RQ_SENT, STATE10, NOSTATE},
	{A_REL_RQ_SENT, STATE11, NOSTATE},
	{A_REL_RQ_SENT, STATE12, NOSTATE},
    {A_REL_RQ_SENT, STATE13, NOSTATE}},
    {
	{A_REL_RQ_RCVD, STATE1, NOSTATE},
	{A_REL_RQ_RCVD, STATE2, STATE13},
	{A_REL_RQ_RCVD, STATE3, STATE13},
	{A_REL_RQ_RCVD, STATE4, NOSTATE},
	{A_REL_RQ_RCVD, STATE5, STATE13},
	{A_REL_RQ_RCVD, STATE6, STATE8},
	{A_REL_RQ_RCVD, STATE7, STATE10},
	{A_REL_RQ_RCVD, STATE8, STATE13},
	{A_REL_RQ_RCVD, STATE9, STATE13},
	{A_REL_RQ_RCVD, STATE10, STATE13},
	{A_REL_RQ_RCVD, STATE11, STATE13},
	{A_REL_RQ_RCVD, STATE12, STATE13},
    {A_REL_RQ_RCVD, STATE13, STATE13}},
    {
	{A_REL_RP_SENT, STATE1, NOSTATE},
	{A_REL_RP_SENT, STATE2, NOSTATE},
	{A_REL_RP_SENT, STATE3, NOSTATE},
	{A_REL_RP_SENT, STATE4, NOSTATE},
	{A_REL_RP_SENT, STATE5, NOSTATE},
	{A_REL_RP_SENT, STATE6, NOSTATE},
	{A_REL_RP_SENT, STATE7, NOSTATE},
	{A_REL_RP_SENT, STATE8, STATE13},
	{A_REL_RP_SENT, STATE9, STATE11},
	{A_REL_RP_SENT, STATE10, NOSTATE},
	{A_REL_RP_SENT, STATE11, NOSTATE},
	{A_REL_RP_SENT, STATE12, STATE13},
    {A_REL_RP_SENT, STATE13, NOSTATE}},
    {
	{A_REL_RP_RCVD, STATE1, NOSTATE},
	{A_REL_RP_RCVD, STATE2, STATE13},
	{A_REL_RP_RCVD, STATE3, STATE13},
	{A_REL_RP_RCVD, STATE4, NOSTATE},
	{A_REL_RP_RCVD, STATE5, STATE13},
	{A_REL_RP_RCVD, STATE6, STATE13},
	{A_REL_RP_RCVD, STATE7, STATE1},
	{A_REL_RP_RCVD, STATE8, STATE13},
	{A_REL_RP_RCVD, STATE9, STATE13},
	{A_REL_RP_RCVD, STATE10, STATE13},
	{A_REL_RP_RCVD, STATE11, STATE13},
	{A_REL_RP_RCVD, STATE12, STATE13},
    {A_REL_RP_RCVD, STATE13, NOSTATE}},
    {
	{A_ABORT_SENT, STATE1, NOSTATE},
	{A_ABORT_SENT, STATE2, NOSTATE},
	{A_ABORT_SENT, STATE3, STATE13},
	{A_ABORT_SENT, STATE4, STATE1},
	{A_ABORT_SENT, STATE5, STATE13},
	{A_ABORT_SENT, STATE6, STATE13},
	{A_ABORT_SENT, STATE7, STATE13},
	{A_ABORT_SENT, STATE8, STATE13},
	{A_ABORT_SENT, STATE9, STATE13},
	{A_ABORT_SENT, STATE10, STATE13},
	{A_ABORT_SENT, STATE11, STATE13},
	{A_ABORT_SENT, STATE12, STATE13},
    {A_ABORT_SENT, STATE13, NOSTATE}},
    {
	{A_ABORT_RCVD, STATE1, NOSTATE},
	{A_ABORT_RCVD, STATE2, STATE1},
	{A_ABORT_RCVD, STATE3, STATE1},
	{A_ABORT_RCVD, STATE4, NOSTATE},
	{A_ABORT_RCVD, STATE5, STATE1},
	{A_ABORT_RCVD, STATE6, STATE1},
	{A_ABORT_RCVD, STATE7, STATE1},
	{A_ABORT_RCVD, STATE8, STATE1},
	{A_ABORT_RCVD, STATE9, STATE1},
	{A_ABORT_RCVD, STATE10, STATE1},
	{A_ABORT_RCVD, STATE11, STATE1},
	{A_ABORT_RCVD, STATE12, STATE1},
    {A_ABORT_RCVD, STATE13, STATE1}},
    {
	{INVALID_PDU_SENT, STATE1, NOSTATE},
	{INVALID_PDU_SENT, STATE2, NOSTATE},
	{INVALID_PDU_SENT, STATE3, NOSTATE},
	{INVALID_PDU_SENT, STATE4, NOSTATE},
	{INVALID_PDU_SENT, STATE5, NOSTATE},
	{INVALID_PDU_SENT, STATE6, NOSTATE},
	{INVALID_PDU_SENT, STATE7, NOSTATE},
	{INVALID_PDU_SENT, STATE8, NOSTATE},
	{INVALID_PDU_SENT, STATE9, NOSTATE},
	{INVALID_PDU_SENT, STATE10, NOSTATE},
	{INVALID_PDU_SENT, STATE11, NOSTATE},
	{INVALID_PDU_SENT, STATE12, NOSTATE},
    {INVALID_PDU_SENT, STATE13, NOSTATE}},
    {
	{INVALID_PDU_RCVD, STATE1, NOSTATE},
	{INVALID_PDU_RCVD, STATE2, STATE13},
	{INVALID_PDU_RCVD, STATE3, STATE13},
	{INVALID_PDU_RCVD, STATE4, NOSTATE},
	{INVALID_PDU_RCVD, STATE5, STATE13},
	{INVALID_PDU_RCVD, STATE6, STATE13},
	{INVALID_PDU_RCVD, STATE7, STATE13},
	{INVALID_PDU_RCVD, STATE8, STATE13},
	{INVALID_PDU_RCVD, STATE9, STATE13},
	{INVALID_PDU_RCVD, STATE10, STATE13},
	{INVALID_PDU_RCVD, STATE11, STATE13},
	{INVALID_PDU_RCVD, STATE12, STATE13},
    {INVALID_PDU_RCVD, STATE13, STATE13}}
};


/* DUL_FileSnoop
**
** Purpose:
**	Reads parsed TCP data buffers and their headers from files. Note
**	that the format of the headers may be found in snp.h . The data
**	is then parsed for DICOM PDUs. The DICOM PDUs are passed back to
**	calling entity via registered callbacks. Input parameters should
**	have a file name for data in each of the two directions. Initiator
**	and acceptor names may also be specified although these are only
**	used to fill in DUL_ASSOCIATESERVICEPARAMETERS structure
**
** Parameter Dictionary:
**	itoa_file	File name of parsed TCP data ini->acc
**	atoi_file	File name of parsed TCP data acc->ini
**	initiator 	(Optional) name / IP address of assoc. initiator
**	acceptor 	(Optional) name / IP address of assoc. acceptor
**
** Return Values:
**	DUL_NORMAL
**	DUL_SNPFILEREAD
**	DUL_SNPFILEOPEN
**	DUL_SNPCALLBACKUSE
**	DUL_SNPBADASSOCSTATE
**	DUL_SNPPREMATUREEOF
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
DUL_FileSnoop(char *itoa_file, char *atoi_file, char *initiator, char *acceptor)
{
    CONDITION 		cond;	/* Variable to track conditions returned */
    void 			*ctx;	/* Dummy context pointer */
    int 			read_atoi_flag;		/* Flag to indicate read from ini->acc file req'd */
    int 			read_itoa_flag;		/* Flag to indicate read from acc->ini file req'd */
    int 			read_itoa;		/* Bytes read from ini->acc file */
    int 			read_atoi;		/* Bytes read from acc->ini file */
    TCP_BUF_HEAD 	itoa_hdr;	/* Header of buffer ini->acc */
    TCP_BUF_HEAD 	atoi_hdr;	/* Header of buffer acc->ini */
    char 			buffer[BUFFERSIZE];	/* Buffer for reads from files */
    u_long 			seq_num;	/* Sequence number of TCP buffer sought */
    int 			fd_atoi;	/* File descriptor for initiator -> acceptor data */
    int 			fd_itoa;	/* File descriptor for acceptor -> initiator data */


    printf("Initiator -> Acceptor data file = %s\n", itoa_file);
    printf("Acceptor -> Initiator data file = %s\n", atoi_file);
    printf("\n\n");

    ini = initiator;
    acc = acceptor;

    /*
     * Open data files
     */
    if ((fd_itoa = open(itoa_file, O_RDONLY, 0)) == NULL)
    	return COND_PushCondition(DUL_SNPFILEOPEN, DUL_Message(DUL_SNPFILEOPEN), itoa_file, "DUL_FileSnoop");
    if ((fd_atoi = open(atoi_file, O_RDONLY, 0)) == NULL)
    	return COND_PushCondition(DUL_SNPFILEOPEN, DUL_Message(DUL_SNPFILEOPEN), atoi_file, "DUL_FileSnoop");
    /*
     * For lack of a better scheme Read READ_SIZE bytes from ini->acc file
     * and then from acc->ini alternatively for processing until out of data
     * in files
     */
    seq_num = 0;
    assoc = 0;
    read_itoa_flag = 1;
    read_atoi_flag = 1;
    PDU_FSM_init(ini, acc);
    do {
	/*
	 * Processing with ini->acc file
	 */

	/*
	 * Read header if required
	 */
	if (read_itoa_flag) {
	    read_itoa = read(fd_itoa, &itoa_hdr, 12);
	    read_itoa_flag = 0;
	    if (read_itoa && (read_itoa != 12))
	    	return COND_PushCondition(DUL_SNPFILEREAD, DUL_Message(DUL_SNPFILEREAD), itoa_file, "DUL_FileSnoop");
	}
	/*
	 * Only if have a good header check against sequence number being
	 * sought
	 */
	if (read_itoa == 12) {
	    /*
	     * Check for the header being an end of association indication
	     * (only in ini->acc data file)
	     */
	    if (itoa_hdr.type == SNP_EOA) {
	    	if (seq_num == itoa_hdr.seq) {
	    		assoc++;
	    		printf("\n** End of association %d**\n\n", assoc);
	    		PDU_FSM_init(ini, acc);
	    		read_itoa_flag = 1;
	    		seq_num++;
	    	}
	    }else{
		/*
		 * If seq number in header is the next one read, the data and
		 * call the callback function. Increment sequence number
		 * being sought. Setup for reading of next header.
		 */
	    	if (seq_num == itoa_hdr.seq) {
	    		if ((read(fd_itoa, buffer, itoa_hdr.len)) == itoa_hdr.len) {
	    			cond = callbackITOA(buffer, itoa_hdr.len, callPDU.ITOActx);
	    			if (cond != SNP_NORMAL)
	    				return COND_PushCondition(DUL_SNPCALLBACKUSE,
												  DUL_Message(DUL_SNPCALLBACKUSE),
												  "callbackITOA", "DUL_FileSnoop");
	    			seq_num++;
	    			read_itoa_flag = 1;
	    		}else{
	    			return COND_PushCondition(DUL_SNPFILEREAD, DUL_Message(DUL_SNPFILEREAD), itoa_file, "DUL_FileSnoop");
	    		}
	    	}
	    }
	}
	/*
	 * Processing with acc->ini file
	 */

	/*
	 * Read header if required
	 */
	if (read_atoi_flag) {
	    read_atoi = read(fd_atoi, &atoi_hdr, 12);
	    read_atoi_flag = 0;
	    if (read_atoi && (read_atoi != 12))
	    	return COND_PushCondition(DUL_SNPFILEREAD, DUL_Message(DUL_SNPFILEREAD), atoi_file, "DUL_FileSnoop");
	}
	/*
	 * Only if have a good header check against sequence number being
	 * sought
	 */
	if (read_atoi == 12) {
	    /*
	     * If seq number in header is the next one read, the data and
	     * call the callback function. Increment sequence number being
	     * sought. Setup for reading of next header.
	     */
	    if (seq_num == atoi_hdr.seq) {
	    	if ((read(fd_atoi, buffer, atoi_hdr.len)) == atoi_hdr.len) {
	    		cond = callbackATOI(buffer, atoi_hdr.len, callPDU.ATOIctx);
	    		if (cond != SNP_NORMAL)
	    			return COND_PushCondition(DUL_SNPCALLBACKUSE,
											  DUL_Message(DUL_SNPCALLBACKUSE),
											  "callbackATOI", "DUL_FileSnoop");
	    		seq_num++;
	    		read_atoi_flag = 1;
	    	}else{
	    		return COND_PushCondition(DUL_SNPFILEREAD, DUL_Message(DUL_SNPFILEREAD), atoi_file, "DUL_FileSnoop");
	    	}
	    }
	}
    } while ((read_itoa_flag || read_atoi_flag) && (ini_state > 0) && (acc_state > 0));
    if ((ini_state < 1) || (acc_state < 1)) {
    	return COND_PushCondition(DUL_SNPBADASSOCSTATE, DUL_Message(DUL_SNPBADASSOCSTATE));
    } else {
    	if (read_itoa || read_atoi)
    		return COND_PushCondition(DUL_SNPPREMATUREEOF, DUL_Message(DUL_SNPPREMATUREEOF), "DUL_FileSnoop");
    }
    /*
     * Close file descriptors
     */
    close(fd_itoa);
    close(fd_atoi);

    return DUL_NORMAL;

}



/* DUL_NetworkSnoop
**
** Purpose:
**	Given the appropriate parameters monitors DICOM associations based on TCP/IP
**	over shared media networks using the SNP facility in real-time. Through
**	callbacks, parsed TCP data is accumulated and DICOM PDUs parsed out.
**	The DICOM PDUs are delivered to the calling entity, again via callbacks.
**	In addition, the function also tracks DUL states for the associations, and
**	makes available the DUL_ASSOCIATESERVICEPARAMETERS structure for the
**	associations.
**
** Parameter Dictionary:
**	device 		shared media network device driver file name on which
**			to be snooping
**				e.g.  Ethernet interface: "/dev/le"
**	ppa		Physical Point of Access (PPA)
**	initiator 	host name / address of DICOM communication initiator
**	acceptor	host name / address of DICOM communication acceptor
**	port 		port number on acceptor that will be used
**	buffersize	number of bytes of space used for chunks by STREAMS kernel
**			buffer module
**	associations	number of associations to be tracked
**
** Return Values:
**	DUL_NORMAL
**	DUL_SNPBADSTATE
**	DUL_SNPBADASSOCSTATE
**	DUL_SNPCALLBACKREG
**
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
DUL_NetworkSnoop(char *device, int ppa, char *initiator, char *acceptor, int port, int bufsize, int associations)
{
    CONDITION 			cond;


    ini = initiator;
    acc = acceptor;
    assoc = associations;

    /*
     * Re-iterate user entries
     */
    printf("Device = %s\n", device);
    printf("PPA = %d\n", ppa);
    printf("Initiator = %s\n", initiator);
    printf("Acceptor = %s\n", acceptor);
    printf("Acceptor port = %d\n", port);
    printf("Buffer size (longs) = %d\n", bufsize);
    printf("Associations = %d\n", assoc);
    printf("\n\n");

    /*
     * Initialize SNP facilities
     */
    cond = SNP_Init();
    if (cond != SNP_NORMAL) return COND_PushCondition(DUL_SNPINIT, DUL_Message(DUL_SNPINIT), "DUL_NetworkSnoop");
    /*
     * Other initializations
     */
    PDU_FSM_init(ini, acc);

    /*
     * Register callback functions
     */
    cond = SNP_RegisterCallback(callbackState, SNP_CALLBACK_STATE, NULL);
    if (cond != SNP_NORMAL)
    	return COND_PushCondition(DUL_SNPCALLBACKREG, DUL_Message(DUL_SNPCALLBACKREG), callbackState, "DUL_NetworkSnoop");

    cond = SNP_RegisterCallback(callbackITOA, SNP_CALLBACK_ITOA, NULL);
    if (cond != SNP_NORMAL)
    	return COND_PushCondition(DUL_SNPCALLBACKREG, DUL_Message(DUL_SNPCALLBACKREG), callbackITOA, "DUL_NetworkSnoop");

    cond = SNP_RegisterCallback(callbackATOI, SNP_CALLBACK_ATOI, NULL);
    if (cond != SNP_NORMAL)
    	return COND_PushCondition(DUL_SNPCALLBACKREG, DUL_Message(DUL_SNPCALLBACKREG), callbackATOI, "DUL_NetworkSnoop");
    /*
     * Commence the snooping with given arguements
     */
    cond = SNP_Start(device, ppa, initiator, acceptor, port, 20, 2, bufsize);
    if (cond != SNP_NORMAL)
    	return COND_PushCondition(DUL_SNPSTART, DUL_Message(DUL_SNPSTART), "DUL_NetworkSnoop");
    /*
     * Until have monitored correct number of associations or until something
     * goes wrong keep snooping - update user with number of associations to
     * go
     */
    while ((assoc > 0) && (ini_state > 0) && (acc_state > 0)) {
    	sleep(1);
    }

    cond = SNP_Stop();
    if (cond != SNP_NORMAL) return cond;

    cond = SNP_Terminate();
    if (cond != SNP_NORMAL)	return cond;


    /*
     * If finished up with an unexpected state .... something went wrong
     */
    if (current_state != NORMAL)
    	return COND_PushCondition(DUL_SNPBADSTATE, DUL_Message(DUL_SNPBADSTATE), SNP_StateMsg(current_state), "DUL_NetworkSnoop");

    /*
     * If finished up in a bad DUL association state, report it
     */
    if ((ini_state < 1) || (acc_state < 1))
    	return COND_PushCondition(DUL_SNPBADASSOCSTATE, DUL_Message(DUL_SNPBADASSOCSTATE));

    return DUL_NORMAL;
}


/* DUL_RegPDUCall
**
** Purpose:
**	To register callbacks for DICOM PDUs.
**
** Parameter Dictionary:
**	callback	Callback function to register
**	callbackType	Type of callback for PDU (ITOA or ATOI)
**	ctx		Pointer to context data
**
** Return Values:
**
**	DUL_NORMAL		Success
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
DUL_RegPDUCall(void (*callback) (), int callbackType, void *ctx) {
    switch (callbackType) {
		case CALLBACK_ITOA_PDU:
								callPDU.callITOA = callback;
								callPDU.ITOActx = ctx;
								break;
		case CALLBACK_ATOI_PDU:
								callPDU.callATOI = callback;
								callPDU.ATOIctx = ctx;
								break;
    }
    return DUL_NORMAL;
}


/* callbackState
**
** Purpose:
**	Used to report the current state of the underlying SNP
**	facilities.
**
** Parameter Dictionary:
**	state	The state number
**	ctx	Context pointer passed back in callback
**
** Return Values:
**	SNP_NORMAL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes
*/
CONDITION
callbackState(int state, void *ctx)
{
    CONDITION 	cond;

    current_state = state;
    switch (state) {
		case END_ASSOC:
						PDU_FSM_init(ini, acc);
						current_state = NORMAL;
						assoc--;
						printf("\n** %d associations to go **\n\n", assoc);
						if (!assoc) return SNP_DONE;
						break;
		case NORMAL:
						break;
		default:
						assoc = 0;
						return SNP_DONE;
    }
    return SNP_NORMAL;
}


/* callbackITOA
**
** Purpose:
**	Used to accumulate data of DICOM PDUs and track DUL states. Once a PDU is
**	complete, appropriate callback is called to deliver PDU to calling entity.
**	Ring buffers used to store data temporarily. Routine only handles data in
**	initiator to acceptor direction. May be used as a callback by underlying SNP
**	facility.
**
** Parameter Dictionary:
**	buffer		Pointer to data being passed
**	bufsize 	Size of data buffer being passed
**	ctx		Context pointer passed back in callback
**
** Return Values:
**	SNP_NORMAL
**	SNP_CALLBACKFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
callbackITOA(char *buffer, int bufsize, void *ctx)
{
    static unsigned char 		ring_buf[RING_SIZE];
    static int 					read_ptr, write_ptr;
    u_long 						pdu_len;
    int 						avail_flag;
    register PDU_FSM_ENTRY 		*entry;
    CONDITION 					cond;

    /*
     * Write data to ring buffer
     */
    if (ring_write(ring_buf, buffer, bufsize, &read_ptr, &write_ptr)) {
#ifdef DEBUG
    	if (debug) fprintf(DEBUG_ERR_DEVICE, "Error: WRITE on ring buffer in callbackITOA\n");
#endif
    	return SNP_CALLBACKFAIL;
    }
    do {
	/*
	 * Read length of PDU
	 */
    	if (ring_read(ring_buf, (unsigned char *) &pdu_len, 2, 4, read_ptr)) {
#ifdef DEBUG
    		if (debug) fprintf(DEBUG_ERR_DEVICE, "Error: READ on ring buffer in callbackATOI\n");
#endif
    		return SNP_CALLBACKFAIL;
    	}
	/*
	 * Check if complete PDU is avaialbale
	 */
    	avail_flag = ring_avail(pdu_len + 6, &read_ptr, &write_ptr);
    	if (avail_flag || (ring_buf[read_ptr] > DUL_MAXTYPE) || (!ring_buf[read_ptr])) {
    		switch (ring_buf[read_ptr]) {
				case DUL_TYPEASSOCIATERQ:
											ini_state = (&pduStateTable[A_ASSOC_RQ_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_ASSOC_RQ_RCVD][acc_state - 1])->next_state;

											cond = fill_rq_params(&ring_buf[read_ptr], (int) pdu_len);
											if (cond != DUL_NORMAL) return cond;

											callPDU.callITOA(ini_state, acc_state, DUL_TYPEASSOCIATERQ, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPEASSOCIATEAC:
											ini_state = (&pduStateTable[A_ASSOC_AC_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_ASSOC_AC_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPEASSOCIATEAC, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPEASSOCIATERJ:
											ini_state = (&pduStateTable[A_ASSOC_RJ_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_ASSOC_RJ_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPEASSOCIATERJ, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPEDATA:
											ini_state = (&pduStateTable[P_DATA_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[P_DATA_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPEDATA, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPERELEASERQ:
											ini_state = (&pduStateTable[A_REL_RQ_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_REL_RQ_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPERELEASERQ, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPERELEASERP:
											ini_state = (&pduStateTable[A_REL_RP_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_REL_RP_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPERELEASERP, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				case DUL_TYPEABORT:
											ini_state = (&pduStateTable[A_ABORT_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[A_ABORT_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, DUL_TYPEABORT, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
											break;
				default:
											/* Unknown PDU */
											ini_state = (&pduStateTable[INVALID_PDU_SENT][ini_state - 1])->next_state;
											acc_state = (&pduStateTable[INVALID_PDU_RCVD][acc_state - 1])->next_state;
											callPDU.callITOA(ini_state, acc_state, ring_buf[read_ptr], NULL, 0, &params);
											/* Unknown PDU - don't know length - flush ring buffer */
											read_ptr = write_ptr;
											break;
	    }
	    if (avail_flag) {
	    	if (ring_free(pdu_len + 6, &read_ptr)) {
#ifdef DEBUG
	    		if (debug) fprintf(DEBUG_ERR_DEVICE, "Error: FREE on ring buffer in callbackITOA\n");
#endif
	    		return SNP_CALLBACKFAIL;
	    	}
	    }
	}
    } while ((avail_flag) && (read_ptr != write_ptr));
    return SNP_NORMAL;
}


/* callbackATOI
**
** Purpose:
**	Used to accumulate data of DICOM PDUs and track DUL states. Once a PDU is
**	complete, appropriate callback is called to deliver PDU to calling entity.
**	Ring buffers used to store data temporarily. Routine only handles data in
**	acceptor to initiator direction. May be used as a callback by underlying SNP
**	facility.
**
** Parameter Dictionary:
**	buffer		Pointer to data being passed
**	bufsize 	Size of data buffer being passed
**	ctx		Context pointer passed back in callback
**
** Return Values:
**	SNP_NORMAL
**	SNP_CALLBACKFAIL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
callbackATOI(char *buffer, int bufsize, void *ctx)
{
    static unsigned char 	ring_buf[RING_SIZE];
    static int 				read_ptr, write_ptr;
    u_long 					pdu_len;
    int 					avail_flag;
    CONDITION 				cond;


    /*
     * Write data to ring buffer
     */
    if (ring_write(ring_buf, buffer, bufsize, &read_ptr, &write_ptr)) {
#ifdef DEBUG
    	if (debug) fprintf(DEBUG_ERR_DEVICE, "Error: WRITE on ring buffer in callbackATOI\n");
#endif
    	return SNP_CALLBACKFAIL;
    }
    do {
	/*
	 * Read length of PDU
	 */
    	if (ring_read(ring_buf, (unsigned char *) &pdu_len, 2, 4, read_ptr)) {
#ifdef DEBUG
    		if (debug)fprintf(DEBUG_ERR_DEVICE, "Error: READ on ring buffer in callbackATOI\n");
#endif
    		return SNP_CALLBACKFAIL;
	}
	/*
	 * Check if complete PDU is avaialbale
	 */
	avail_flag = ring_avail(pdu_len + 6, &read_ptr, &write_ptr);
	if (avail_flag || (ring_buf[read_ptr] > DUL_MAXTYPE) || (!ring_buf[read_ptr])) {
	    switch (ring_buf[read_ptr]) {
			case DUL_TYPEASSOCIATERQ:
										ini_state = (&pduStateTable[A_ASSOC_RQ_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_ASSOC_RQ_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, DUL_TYPEASSOCIATERQ, &ring_buf[read_ptr], (int) pdu_len, &params);
										break;
			case DUL_TYPEASSOCIATEAC:
										ini_state = (&pduStateTable[A_ASSOC_AC_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_ASSOC_AC_SENT][acc_state - 1])->next_state;

										cond = fill_ac_params(&ring_buf[read_ptr], pdu_len);
										if (cond != DUL_NORMAL) return cond;

										callPDU.callATOI(ini_state, acc_state, DUL_TYPEASSOCIATEAC, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			case DUL_TYPEASSOCIATERJ:
										ini_state = (&pduStateTable[A_ASSOC_RJ_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_ASSOC_RJ_SENT][acc_state - 1])->next_state;

										cond = fill_rj_params(&ring_buf[read_ptr]);
										if (cond != DUL_NORMAL) return cond;

										callPDU.callATOI(ini_state, acc_state, DUL_TYPEASSOCIATERJ, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			case DUL_TYPEDATA:
										ini_state = (&pduStateTable[P_DATA_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[P_DATA_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, DUL_TYPEDATA, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			case DUL_TYPERELEASERQ:
										ini_state = (&pduStateTable[A_REL_RQ_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_REL_RQ_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, DUL_TYPERELEASERQ, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			case DUL_TYPERELEASERP:
										ini_state = (&pduStateTable[A_REL_RP_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_REL_RP_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, DUL_TYPERELEASERP, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			case DUL_TYPEABORT:
										ini_state = (&pduStateTable[A_ABORT_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[A_ABORT_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, DUL_TYPEABORT, &ring_buf[read_ptr], (int) pdu_len + 6, &params);
										break;
			default:
										/* Unknown PDU */
										ini_state = (&pduStateTable[INVALID_PDU_RCVD][ini_state - 1])->next_state;
										acc_state = (&pduStateTable[INVALID_PDU_SENT][acc_state - 1])->next_state;
										callPDU.callATOI(ini_state, acc_state, ring_buf[read_ptr], NULL, 0, &params);
										/* Unknown PDU - don't know length - flush ring buffer */
										read_ptr = write_ptr;
										break;
	    }
	    if (avail_flag) {
	    	if (ring_free(pdu_len + 6, &read_ptr)) {
#ifdef DEBUG
	    		if (debug) fprintf(DEBUG_ERR_DEVICE, "Error: FREE on ring buffer in callbackATOI\n");
#endif
	    		return SNP_CALLBACKFAIL;
	    	}
	    }
	}
    } while ((avail_flag) && (read_ptr != write_ptr));
    return SNP_NORMAL;
}


/* PDU_FSM_init
**
** Purpose:
**	To reset states and clear out DUL_ASSOCIATESERVICEPARAMETERS structure for next
**	association
**
** Parameter Dictionary:
**	initiator 	Name of initiator for DUL_ASSOCIATESERVICEPARAMETERS structure
**	acceptor 	Name of acceptor for DUL_ASSOCIATESERVICEPARAMETERS structure
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static void
PDU_FSM_init(char *initiator, char *acceptor)
{
    ini_state = STATE4;		/* Initial states for DUL monitoring FSM */
    acc_state = STATE2;
    memset(&params, 0, sizeof(DUL_ASSOCIATESERVICEPARAMETERS));
    DUL_ClearServiceParameters(&params);

    if ((initiator != NULL) && (acceptor != NULL)) {
    	(void) strcpy(params.callingPresentationAddress, initiator);
    	(void) strcpy(params.calledPresentationAddress, acceptor);
    }
}


/* ring_write
**
** Purpose:
**	Used to write data to a ring buffer.
**
** Parameter Dictionary:
**	ring_buf	Pointer to ring buffer
**	buffer		Pointer to data being passed
**	bufsize 	Size of data buffer being passed
**	read_ptr	Pointer of integer read pointer
**	write_ptr	Pointer of integer write pointer
**
** Return Values:
**	0		Success
**	1		Failure
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
ring_write(unsigned char *ring_buf, unsigned char *buffer, int bufsize, int *read_ptr, int *write_ptr)
{
    int 		check_ptr;

    check_ptr = *write_ptr + bufsize;
    if (check_ptr / RING_SIZE) {
    	if ((check_ptr / RING_SIZE) > 1){
    		return 1;
    	}else{
    		check_ptr = check_ptr % RING_SIZE;
    		if (check_ptr > *read_ptr){
    			return 1;
    		}else{
    			memcpy(&ring_buf[*write_ptr], buffer, RING_SIZE - *write_ptr);
    			memcpy(ring_buf, &buffer[RING_SIZE - *write_ptr], bufsize - (RING_SIZE - *write_ptr));
    			*write_ptr = check_ptr;
    			return 0;
    		}
    	}
    }else{
    	if ((*write_ptr < *read_ptr) && (check_ptr > *read_ptr)){
    		return 1;
		}else {
    		memcpy(&ring_buf[*write_ptr], buffer, bufsize);
    		*write_ptr = check_ptr;
    		return 0;
    	}
    }
}



/* ring_avail
**
** Purpose:
**	Used to check if data length of a PDU available in ring buffer.
**
** Parameter Dictionary:
**	pdu_len 	Sise of PDU being sought
**	read_ptr	Pointer of integer read pointer
**	write_ptr	Pointer of integer write pointer
**
** Return Values:
**	1		Success
**	0		Failure
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
ring_avail(u_long pdu_len, int *read_ptr, int *write_ptr)
{

    if (*read_ptr > *write_ptr) {
    	if (!(((RING_SIZE - *read_ptr) + *write_ptr) < pdu_len)){
    		return 1;		/* Have all data for PDU */
		}else{
    		return 0;		/* Do not have all data for PDU */
		}
    }else{
    	if (!((*write_ptr - *read_ptr) < pdu_len)){
    		return 1;		/* Have all data for PDU */
    	}else{
    		return 0;		/* Do not have all data for PDU */
    	}
    }
}



/* ring_read
**
** Purpose:
**	Used to read data in a ring buffer.
**
** Parameter Dictionary:
**	ring_buf	Pointer to ring buffer
**	buf 		Buffer into which place data read from ring buffer
**	offset		Offset from read pointer from which to read data
**	len		Length of data to the read
**	read_ptr	Pointer of integer read pointer
**
** Return Values:
**
**	0		Success
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
ring_read(unsigned char *ring_buf, unsigned char *buf, int offset, int len, int read_ptr)
{

    if ((read_ptr + offset + len) < RING_SIZE){
    	memcpy(buf, &ring_buf[read_ptr + offset], len);
    }else{
    	if ((read_ptr + offset) < RING_SIZE) {
    		memcpy(buf, &ring_buf[read_ptr + offset], RING_SIZE - (read_ptr + offset));
    		memcpy(buf, ring_buf, len - (RING_SIZE - (read_ptr + offset)));
    	}else{
    		memcpy(buf, &ring_buf[(read_ptr + offset) % RING_SIZE], len);
    	}
    }
    return 0;
}



/* ring_free
**
** Purpose:
**	Used to free data in a ring buffer.
**
** Parameter Dictionary:
**	len		Length of data to the read
**	read_ptr	Pointer of integer read pointer
**
** Return Values:
**
**	0		Success
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static int
ring_free(int len, int *read_ptr)
{
    *read_ptr = (*read_ptr + len) % RING_SIZE;
    return 0;
}



/* fill_rq_params
**
** Purpose:
**	Used to fill the request parameters of DUL_ASSOCIATESERVICEPARAMETERS
**	structure
**
** Parameter Dictionary:
**	buf		Pointer to associate request PDU
**	pdulen		Length of PDU
**
** Return Values:
**	DUL_NORMAL
**	DUL_LISTCREATEFAILED
**	DUL_PCTRANSLATIONFAILURE
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
fill_rq_params(u_char * buf, u_long pdulen)
{
    PRV_ASSOCIATEPDU 		assoc;
    CONDITION 				cond;

    cond = parseAssociate(buf, pdulen, &assoc);
    if (cond != DUL_NORMAL)	return cond;

    (void) strcpy(params.calledAPTitle, assoc.calledAPTitle);
    (void) strcpy(params.callingAPTitle, assoc.callingAPTitle);
    (void) strcpy(params.applicationContextName, assoc.applicationContext.data);
    if ((params.requestedPresentationContext = LST_Create()) == NULL)
    	return COND_PushCondition(DUL_LISTCREATEFAILED, DUL_Message(DUL_LISTCREATEFAILED), "fill_rq_params");

    if (translatePresentationContextList(&assoc.presentationContextList, &assoc.userInfo.SCUSCPRoleList, &params.requestedPresentationContext) != DUL_NORMAL)
    	return COND_PushCondition(DUL_PCTRANSLATIONFAILURE, DUL_Message(DUL_PCTRANSLATIONFAILURE), "fill_rq_params");

    params.peerMaxPDU = assoc.userInfo.maxLength.maxLength;
    strcpy(params.callingImplementationClassUID, assoc.userInfo.implementationClassUID.data);
    strcpy(params.callingImplementationVersionName, assoc.userInfo.implementationVersionName.data);

    if (assoc.userInfo.asyncOperations.length) {
    	params.maximumOperationsPerformed = assoc.userInfo.asyncOperations.maximumOperationsProvided;
    	params.maximumOperationsInvoked = assoc.userInfo.asyncOperations.maximumOperationsInvoked;
    }
    destroyPresentationContextList(&assoc.presentationContextList);
    destroyUserInformationLists(&assoc.userInfo);

    return DUL_NORMAL;
}

/* fill_ac_params
**
** Purpose:
**	Used to fill the accept parameters of DUL_ASSOCIATESERVICEPARAMETERS
**	structure
**
** Parameter Dictionary:
**	buf		Pointer to associate accept PDU
**	pdulen		Length of PDU
**
** Return Values:
**	DUL_NORMAL
**	DUL_LISTCREATEFAILED
**	DUL_MALLOCERROR
**	DUL_PEERILLEGALXFERSYNTAXCOUNT
**	DUL_LISTERROR
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
fill_ac_params(u_char * buf, u_long pdulen)
{

    PRV_ASSOCIATEPDU 				assoc;
    CONDITION 						cond;
    PRV_PRESENTATIONCONTEXTITEM 	*prvCtx;
    DUL_PRESENTATIONCONTEXT			* userPresentationCtx,	*requestedPresentationCtx;
    DUL_SUBITEM 					*subItem;
    PRV_SCUSCPROLE 					*scuscpRole;
    u_char 							buffer[8192];
    u_long 							pduLength;



    cond = parseAssociate(buf, pdulen, &assoc);
    if (cond != DUL_NORMAL)	return COND_PushCondition(DUL_ILLEGALPDU, DUL_Message(DUL_ILLEGALPDU));

    (void) strcpy(params.respondingAPTitle, assoc.calledAPTitle);
    (void) strcpy(params.callingAPTitle, assoc.callingAPTitle);
    (void) strcpy(params.applicationContextName, assoc.applicationContext.data);

    if ((params.acceptedPresentationContext = LST_Create()) == NULL)
    	return COND_PushCondition(DUL_LISTCREATEFAILED, DUL_Message(DUL_LISTCREATEFAILED), "fill_ac_params");

    prvCtx = LST_Head(&assoc.presentationContextList);
    if (prvCtx != NULL)	(void) LST_Position(&assoc.presentationContextList, prvCtx);

    while (prvCtx != NULL) {
    	userPresentationCtx = (DUL_PRESENTATIONCONTEXT *)
	    CTN_MALLOC(sizeof(*userPresentationCtx));

    	if (userPresentationCtx == NULL)
    		return COND_PushCondition(DUL_MALLOCERROR, DUL_Message(DUL_MALLOCERROR), "fill_ac_params", sizeof(*userPresentationCtx));

    	(void) memset(userPresentationCtx, 0, sizeof(userPresentationCtx));
    	userPresentationCtx->result = prvCtx->result;
    	userPresentationCtx->presentationContextID = prvCtx->contextID;
    	userPresentationCtx->proposedTransferSyntax = NULL;
    	requestedPresentationCtx = (DUL_PRESENTATIONCONTEXT *) findPresentationCtx(&params.requestedPresentationContext, prvCtx->contextID);

    	if (requestedPresentationCtx != NULL) {
    		strcpy(userPresentationCtx->abstractSyntax, requestedPresentationCtx->abstractSyntax);
    		userPresentationCtx->proposedSCRole = requestedPresentationCtx->proposedSCRole;
    	}

    	userPresentationCtx->acceptedSCRole = DUL_SC_ROLE_DEFAULT;
    	scuscpRole = (PRV_SCUSCPROLE *) findSCUSCPRole(&assoc.userInfo.SCUSCPRoleList, userPresentationCtx->abstractSyntax);

    	if (scuscpRole != NULL) {
    		if (scuscpRole->SCURole == scuscpRole->SCPRole){
    			userPresentationCtx->acceptedSCRole = DUL_SC_ROLE_SCUSCP;
    		}else if (scuscpRole->SCURole == 1){
    			userPresentationCtx->acceptedSCRole = DUL_SC_ROLE_SCU;
    		}else{
    			userPresentationCtx->acceptedSCRole = DUL_SC_ROLE_SCP;
    		}
    	}

    	if (prvCtx->transferSyntaxList == NULL)
    		return COND_PushCondition(DUL_PEERILLEGALXFERSYNTAXCOUNT, DUL_Message(DUL_PEERILLEGALXFERSYNTAXCOUNT), 0);

    	if ((prvCtx->result == DUL_PRESENTATION_ACCEPT) && (LST_Count(&prvCtx->transferSyntaxList) != 1))
    		return COND_PushCondition(DUL_PEERILLEGALXFERSYNTAXCOUNT, DUL_Message(DUL_PEERILLEGALXFERSYNTAXCOUNT), LST_Count(&prvCtx->transferSyntaxList));

    	subItem = LST_Head(&prvCtx->transferSyntaxList);

    	if (subItem != NULL)
    		(void) strcpy(userPresentationCtx->acceptedTransferSyntax, subItem->data);

    	if (LST_Enqueue(&params.acceptedPresentationContext, userPresentationCtx) != LST_NORMAL)
    		return COND_PushCondition(DUL_LISTERROR, DUL_Message(DUL_LISTERROR), "fill_ac_params");

    	prvCtx = LST_Next(&assoc.presentationContextList);

    }

    destroyPresentationContextList(&assoc.presentationContextList);
    destroyUserInformationLists(&assoc.userInfo);
    params.peerMaxPDU = assoc.userInfo.maxLength.maxLength;
    params.maxPDU = assoc.userInfo.maxLength.maxLength;

    strcpy(params.calledImplementationClassUID, assoc.userInfo.implementationClassUID.data);
    strcpy(params.calledImplementationVersionName, assoc.userInfo.implementationVersionName.data);

    if (assoc.userInfo.asyncOperations.length) {
    	params.maximumOperationsPerformed = assoc.userInfo.asyncOperations.maximumOperationsProvided;
    	params.maximumOperationsInvoked =  assoc.userInfo.asyncOperations.maximumOperationsInvoked;
    }
    return DUL_NORMAL;
}


/* fill_rj_params
**
** Purpose:
**	Used to fill the reject parameters of DUL_ASSOCIATESERVICEPARAMETERS
**	structure
**
** Parameter Dictionary:
**	buf		Pointer to associate reject PDU
**
** Return Values:
**	DUL_NORMAL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
fill_rj_params(u_char * buf)
{
    params.result = buf[7];
    params.resultSource = buf[8];
    params.diagnostic = buf[9];

    return DUL_NORMAL;
}


/* snoopDebug
**
** Purpose:
**	To turn on/off the debuging features of this module
**
** Parameter Dictionary:
**	flag 		Indication of whether to turn on/off debugging
**			features
**
** Return Values:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
void
snoopDebug(CTNBOOLEAN flag)
{
    debug = flag;
}

#else /* If SNOOP is not defined .... just return the unimplemented code */

CONDITION
DUL_FileSnoop(char *itoa_file, char *atoi_file, char *initiator, char *acceptor)
{
    return COND_PushCondition(DUL_SNPUNIMPLEMENTED, SNP_Message(DUL_SNPUNIMPLEMENTED), "DUL_FileSnoop");
}

CONDITION
DUL_NetworkSnoop(char *device, int ppa, char *initiator, char *acceptor, int port, int bufsize, int associations)
{
    return COND_PushCondition(DUL_SNPUNIMPLEMENTED, DUL_Message(DUL_SNPUNIMPLEMENTED), "DUL_NetworkSnoop");
}

CONDITION
DUL_RegPDUCall(void (*callback) (), int callbackType, void *ctx) {
    return COND_PushCondition(DUL_SNPUNIMPLEMENTED, DUL_Message(DUL_SNPUNIMPLEMENTED), "DUL_RegPDUCall");
}

#endif
