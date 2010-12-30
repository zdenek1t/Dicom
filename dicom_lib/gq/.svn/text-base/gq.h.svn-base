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
** Module Name(s):	gq.h
** Author, Date:	David E. Beecher, 23-June-93
** Intent:		Generalized queue routines to allow multiple processes
**			to use shared queues.  The data element for the
**			any given queue is defined at create time and is
**			completely user defined.
** Last Update:		$Author: smm $, $Date: 1996/08/23 19:32:03 $
** Source File:		$RCSfile: gq.h,v $
** Revision:		$Revision: 1.14 $
** Status:		$State: Exp $
*/

#ifndef _GQ_IS_IN
#define _GQ_IS_IN

#ifdef  __cplusplus
extern "C" {
#endif

/* DEFINITIONS OF CONSTANTS */
#define		GQ_MAXSTRINGLENGTH	256
#define		GQ_QUEUEDIRECTORY	"QUEUE_DIRECTORY"
#define		GQ_QUEUEFILESUFFIX	"gq.dat"
#define		GQ_MAXNAMEDQUEUES	200

typedef struct _QUEUE_Pointers {
    int GQ_head,
        GQ_tail,
        GQ_numelements,
        GQ_elementsize;
}   QUEUE_Pointers;

/*
 * Error Conditions for functions
 */
#define GQ_NORMAL		FORM_COND(FAC_GQ, SEV_SUCC,  1)
#define GQ_QUEUEFULL		FORM_COND(FAC_GQ, SEV_WARN,  2)
#define GQ_QUEUEEMPTY		FORM_COND(FAC_GQ, SEV_WARN,  3)
#define GQ_SHAREDMEMORYFAIL	FORM_COND(FAC_GQ, SEV_FATAL, 4)
#define GQ_SEMAPHOREFAIL	FORM_COND(FAC_GQ, SEV_FATAL, 5)
#define GQ_FILEACCESSFAIL	FORM_COND(FAC_GQ, SEV_FATAL, 6)
#define GQ_NOMEMORY		FORM_COND(FAC_GQ, SEV_WARN,  7)
#define	GQ_UNIMPLEMENTED	FORM_COND(FAC_GQ, SEV_ERROR, 8)
#define	GQ_BADELEMSIZE		FORM_COND(FAC_GQ, SEV_ERROR, 9)
#define	GQ_NOPENQUEUE		FORM_COND(FAC_GQ, SEV_ERROR, 10)
#define	GQ_MAXQUEUEEXCEEDED	FORM_COND(FAC_GQ, SEV_ERROR, 11)
#define GQ_FILECREATEFAILED	FORM_COND(FAC_GQ, SEV_FATAL, 12)
#define GQ_MULTCREATEREQUEST	FORM_COND(FAC_GQ, SEV_ERROR, 13)

/*
 * Function Prototypes
 */
CONDITION
GQ_InitQueue(int qid, int num_elements, int element_size);
CONDITION
GQ_KillQueue(int qid);
CONDITION
GQ_Enqueue(int qid, void *element);
CONDITION
GQ_Dequeue(int qid, void *element);
CONDITION
GQ_GetQueue(int qid, int element_size);
CONDITION
GQ_PrintQueue(int qid, void (print_func(void *)));
CONDITION
GQ_PeekQueue(int qid, void *element);
CONDITION
GQ_ModifyHeadElement(int qid, void *element, void (*func) (void *element));
CONDITION
GQ_GetQueueSize(int qid, int *size);
CONDITION
GQ_Wait(void);
CONDITION
GQ_Signal(void);
char
*GQ_MakeFilename(int qid);


#ifdef  __cplusplus
}
#endif

#endif
