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
** Module Name(s):
** Author, Date:	Nilesh R. Gohel, 23-Aug-94
** Intent:		This include file defines constants, structures,
**			and prototypes for the SNP facility.
** Last Update:		$Author: smm $, $Date: 1996/08/23 20:12:23 $
** Source File:		$RCSfile: snp.h,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

#ifndef DICOM_SNP_IS_IN
#define DICOM_SNP_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif


/* Directions */

#define ITOA 		0
#define ATOI 		1

/* Additinal constant for use in header information */

#define SNP_EOA 		2

/* Callback types */

#define SNP_CALLBACK_ITOA	1
#define SNP_CALLBACK_ATOI 	2
#define SNP_CALLBACK_STATE	3


CONDITION
SNP_Init(void);
CONDITION
SNP_Terminate(void);
CONDITION
SNP_RegisterCallback(CONDITION(*callback) (), int callbackType, void *ctx);
CONDITION
SNP_Start(char *device, int ppa, char *initiator, char *acceptor, int port, int timeOut1, int timeOut2, int bufferSpace);
CONDITION
SNP_Stop(void);
char
*SNP_Message(CONDITION cond);
char
*SNP_StateMsg(int state);
void
SNP_Debug(CTNBOOLEAN flag);

#define	SNP_NORMAL			FORM_COND(FAC_SNP, SEV_SUCC, 1)
#define SNP_MALLOCERROR 	FORM_COND(FAC_SNP, SEV_ERROR, 2)
#define SNP_CLOSEERROR 		FORM_COND(FAC_SNP, SEV_ERROR, 3)
#define SNP_OPENERROR 		FORM_COND(FAC_SNP, SEV_ERROR, 4)
#define SNP_SIGSETERROR 	FORM_COND(FAC_SNP, SEV_ERROR, 5)
#define SNP_STREAMSETUP		FORM_COND(FAC_SNP, SEV_ERROR, 6)
#define SNP_LSTCREATFAIL 	FORM_COND(FAC_SNP, SEV_ERROR, 7)
#define SNP_CALLBACKSMISSING	FORM_COND(FAC_SNP, SEV_ERROR, 8)
#define SNP_CALLBACKFAIL	FORM_COND(FAC_SNP, SEV_ERROR, 9)
#define SNP_ARGERROR		FORM_COND(FAC_SNP, SEV_ERROR, 10)
#define SNP_IOCTLFAIL		FORM_COND(FAC_SNP, SEV_ERROR, 11)
#define SNP_UNIMPLEMENTED	FORM_COND(FAC_SNP, SEV_ERROR, 12)
#define SNP_PUTMSGFAIL		FORM_COND(FAC_SNP, SEV_ERROR, 13)
#define SNP_DLPIFAIL		FORM_COND(FAC_SNP, SEV_ERROR, 14)
#define SNP_DLPIEXPECT		FORM_COND(FAC_SNP, SEV_ERROR, 15)
#define SNP_ALARMSET 		FORM_COND(FAC_SNP, SEV_ERROR, 16)
#define SNP_GETMSGFAIL 		FORM_COND(FAC_SNP, SEV_ERROR, 17)
#define SNP_DONE 		FORM_COND(FAC_SNP, SEV_SUCC, 18)



/* State definitions */

#define NORMAL 			1
#define END_ASSOC		2
#define DATA_OVERFLOW		3
#define GETMSG_FAIL 		4
#define RESET_ASSOC_INI		5
#define RESET_ASSOC_ACC		6
#define NONCONTIGDATA		7
#define WRITECALLBACKFAIL 	8
#define LSTINSFAIL 		9
#define DROPPEDPACKETS		10
#define BAD_END_ASSOC		11
#define CON_TIMEOUT		12
#define STRGETMSG_TIMEOUT	13

/* Header used when storing TCP data buffers to files */

typedef struct {
    u_long 	type;
    u_long 	seq;
    u_long 	len;
}   TCP_BUF_HEAD;


#ifdef  __cplusplus
}
#endif

#endif
