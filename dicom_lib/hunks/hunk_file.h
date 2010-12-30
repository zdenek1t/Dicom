/*
+-+-+-+-+-+-+-+-+-
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	hunk_file.h
** Author, Date:	David E. Beecher, 5-May-93
** Intent:		Include file for Hunk file operations.
** Last Update:		$Author: beech $, $Date: 1993/05/12 22:11:45 $
** Source File:		$Source: /sw2/prj/ctn/cvs/facilities/hunks/hunk_file.h,v $
** Revision:		$Revision: 1.1 $
** Status:		$State: Exp $
*/
#include "dicom.h"
#ifndef HF_IS_IN
#define HF_IS_IN
#define OK				1
#define NOTOK			0
#define HUNK_ID 		"DICOM-RSNA-1993"
#define	HUNK_USED		1
#define HUNK_FREE		0
#define HUNK_PTR_NULL	-1

typedef struct _HunkFileHeader {
    int
        hunks_allocated,
        hunk_length,
        hunk_record_length,
        num_recs_per_hunk;
    char
        hunk_file_id[20];
} HunkFileHeader;

typedef struct _HunkBufAdd {
    int
        hunk_number,
        node_number;
} HunkBufAdd;

typedef struct _HunkBuf {
    char
       *buf;
    HunkBufAdd
       *nextp;
} HunkBuf;

typedef struct _HunkHeader {
    int
        hunk_number,
        hunk_dirty,
        free_nodes;
    HunkBufAdd
        static_buf_node;
} HunkHeader;

typedef struct _Hunk {
    HunkHeader
       *hunk_head;
    char
       *node_list;
    HunkBuf
       *buf_pool;
} Hunk;


CONDITION HF_Create(char *, int, int, int);
CONDITION HF_Open(char *);
CONDITION HF_Close(char *);
CONDITION HF_AddHunk(void);
CONDITION HF_AllocateRecord(int, HunkBufAdd *);
CONDITION HF_DeallocateRecord(HunkBufAdd *);
CONDITION HF_ReadRecord(HunkBufAdd *, int, void *);
CONDITION HF_WriteRecord(HunkBufAdd *, int, void *);
CONDITION HF_WriteCurrentHunk(void);
CONDITION HF_ReadFileHeader(void);
CONDITION HF_WriteFileHeader(void);
CONDITION HF_ReadCurrentHunk(int);
CONDITION HF_AllocateStaticRecord(int, int);
CONDITION HF_DeallocateStaticRecord(int);
CONDITION HF_ReadStaticRecord(int, int, void *);
CONDITION HF_WriteStaticRecord(int, int, void *);
int HF_FindFreeNode(void);
void HF_Dumper(void);
void HF_InitAddresses(void);

/*
 * define the error conditions...
 */
#define HF_NORMAL		FORM_COND(FAC_HUNK,SEV_SUCC,1)
#define HF_OK			FORM_COND(FAC_HUNK,SEV_SUCC,1)
#define HF_OPENERROR	FORM_COND(FAC_HUNK,SEV_ERROR,2)
#define HF_BADPARMS		FORM_COND(FAC_HUNK,SEV_ERROR,3)
#define HF_CREATERROR	FORM_COND(FAC_HUNK,SEV_ERROR,4)
#define HF_NOMEMORY		FORM_COND(FAC_HUNK,SEV_ERROR,5)
#define HF_READERROR	FORM_COND(FAC_HUNK,SEV_ERROR,6)
#define HF_BADMAGIC		FORM_COND(FAC_HUNK,SEV_ERROR,7)
#define HF_SEEKERROR	FORM_COND(FAC_HUNK,SEV_ERROR,8)
#define HF_BADHUNK		FORM_COND(FAC_HUNK,SEV_ERROR,9)
#define HF_WRITERROR	FORM_COND(FAC_HUNK,SEV_ERROR,10)
#define HF_CLOSERROR	FORM_COND(FAC_HUNK,SEV_ERROR,11)
#define HF_BADHUNKNUM	FORM_COND(FAC_HUNK,SEV_ERROR,12)
#define HF_BADNODENUM	FORM_COND(FAC_HUNK,SEV_ERROR,13)
#define HF_STATICINUSE	FORM_COND(FAC_HUNK,SEV_ERROR,14)
#define HF_RECTOOBIG	FORM_COND(FAC_HUNK,SEV_ERROR,15)
#endif
