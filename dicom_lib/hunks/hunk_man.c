/*
          Copyright (C) 1993, RSNA and Washington University

          The software and supporting documentation for the Radiological
          Society of North America (RSNA) 1993 Digital Imaging and
          Communications in Medicine (DICOM) Demonstration were developed
          at the
                  Electronic Radiology Laboratory
                  Mallinckrodt Institute of Radiology
                  Washington University School of Medicine
                  510 S. Kingshighway Blvd.
                  St. Louis, MO 63110
          as part of the 1993 DICOM Central Test Node project for, and
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
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	HF_Create, HF_Open, HF_Close, HF_AddHunk,
**			HF_AllocateRecord, HF_DeallocateRecord,
**			HF_ReadRecord, HF_WriteRecord, HF_WriteCurrentHunk,
**			HF_ReadFileHeader, HF_WriteFileHeader,
**			HF_ReadCurrentHunk, HF_FindFreeNode, HF_Dumper
**			HF_AllocateStaticRecord, HF_DeallocateStaticRecord,
**			HF_ReadStaticRecord, HF_WriteStaticRecord
**			HF_InitAddresses, HF_ExclusiveLock, HF_SharedLock,
**			HF_Unlock, HF_ReadUpdateFlag, HF_IncrementUpdateFlag
**
** Author, Date:	David E. Beecher, 5-May-93
**
** Intent:		These routines provide an orderly access mechanism
**			for managing a "hunk" file.  This is the underlying
**			mechanism used for the implementation of the DICOM
**			database system.  It provides orderly access for
**			allocating and deallocating records, reading and writing
**			those records, initializing a new file, and other
**			maintenence features.  The locking mechanisms are one
**			level up in the database code.
**
** Last Update:		$Author: smm $, $Date: 1997/01/24 20:25:05 $
** Source File:		$RCSfile: hunk_man.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: hunk_man.c,v $";

#include "../dicom/ctn_os.h"
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>*/
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>
#include <fcntl.h>
#include <windows.h>
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#if defined(SOLARIS) || defined(GCCSUNOS) || defined(HPUX)
#include <unistd.h>
#endif
#include "hunk_man.h"

static int   			Opened = 0;		/* has the database been opened, initially no */
static HunkFileHeader   GS_hfh;			/* the hunk file header	 */
static Hunk				GS_curhunk;		/* the hunk currently in memory */
static int			    GS_hunkfd;		/* the hunk file file descriptor */
static void			    *GS_hunkstorage = NULL;	/* the storage for the hunk data */
static void				*GS_addstorage = NULL;	/* the storage for the address data */

/* HF_Create
**
** Purpose:
**	This routine creates and initializes a new hunk file most probably
**	for implementing a new database.
**
** Parameter Dictionary:
**	char *fname:
**		a null terminated string indicating the file name to use.
**
**	int num_hunks:
**		the number of "hunks" to initially allocate, the minimum
**		number for this is one.
**
**	int hunk_record_length:
**		the length of fixed length records in this hunk file.
**
**	int num_recs_per_hunk:
**		the number of records per hunk in this hunk file.
**
** Return Values:
**
**	HF_NORMAL: 	The creation command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_BADPARMS:	The parameter for hunk file creation are invalid
**	HF_CREATERROR:	An error in the open to create the file on disk
**	HF_NOMEMORY:	Can't allocate enough memory for hunk storage
**
** Algorithm:
**	A hunk file header starts every hunk file.  This contains information
**	including the length of hunks, the number currently allocated, and
**	an identification string to identify this file as a hunk file.
**
**	Each hunk can simplistically be viewed as a buffer pool with some
**	header information for management.  There are num_recs_per_hunk records
**	per hunk, where each record is hunk_record_length bytes in length.  Each
**	buffer in the pool has a corresponding node which indicates whether or
**	not it is currently being used.  Each hunk also contains header
**	information which includes the hunk number, a dirty bit, a static
**	pointer for this hunk, the number of free nodes (or buffers), and the
**	number of records per hunk and the size of the records in bytes.
**	This description is given more detail in the function descriptions
**	that follow.
*/

CONDITION
HF_Create(char *fname, int num_hunks, int hunk_record_length, int num_recs_per_hunk)
{

    int	        addsize,		/* the size of the addresses	 */
				hunksize,		/* the size of the hunk in memory */
				i,
				j,				/* temp indicies		 */
				openFlags = 0;
    CONDITION        retval;

    if (Opened == 1) return (HF_OPENERROR);
    if (num_hunks < 1 || hunk_record_length < 1 || num_recs_per_hunk < 1) return (HF_BADPARMS);

    openFlags = O_RDWR | O_CREAT;
#ifndef _MSC_VER
    openFlags |= O_SYNC;
#endif
    if ((GS_hunkfd = open(fname, openFlags, 0777)) < 0)	return (HF_CREATERROR);

/*
 * Compute the storage needed for addresses...
 */
    addsize = sizeof(HunkBuf) * num_recs_per_hunk;
/*
 * Compute the storage needed for the hunk in memory...
 */
    hunksize = sizeof(HunkHeader);
    hunksize += num_recs_per_hunk;	/* for the node_list */
    hunksize += ((hunk_record_length + sizeof(HunkBufAdd)) * num_recs_per_hunk);

    if ((GS_hunkstorage = (void *) malloc(hunksize)) == (void *) NULL) return (HF_NOMEMORY);
    if ((GS_addstorage = (void *) malloc(addsize)) == (void *) NULL) return (HF_NOMEMORY);

    GS_hfh.update_flag = 0;
    GS_hfh.hunks_allocated = num_hunks;
    GS_hfh.hunk_length = hunksize;
    GS_hfh.hunk_record_length = hunk_record_length;
    GS_hfh.num_recs_per_hunk = num_recs_per_hunk;
    strcpy(GS_hfh.hunk_file_id, HUNK_ID);

    HF_InitAddresses();

    Opened = 1;
    if ((retval = HF_WriteFileHeader()) != HF_NORMAL) {
    	Opened = 0;
    	close(GS_hunkfd);
    	return (retval);
    }

    for (i = 0; i < num_hunks; i++) {
    	GS_curhunk.hunk_head->hunk_number = i;
    	GS_curhunk.hunk_head->hunk_dirty = 1;
    	GS_curhunk.hunk_head->static_buf_node.hunk_number = HUNK_PTR_NULL;
    	GS_curhunk.hunk_head->static_buf_node.node_number = HUNK_PTR_NULL;
    	GS_curhunk.hunk_head->free_nodes = GS_hfh.num_recs_per_hunk;

    	for (j = 0; j < GS_hfh.num_recs_per_hunk; j++) {
    		GS_curhunk.node_list[j] = (char) HUNK_FREE;
    		GS_curhunk.buf_pool[j].nextp->hunk_number = HUNK_PTR_NULL;
    		GS_curhunk.buf_pool[j].nextp->node_number = HUNK_PTR_NULL;
    	}

    	if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) {
    		Opened = 0;
    		close(GS_hunkfd);
    		return (retval);
    	}
    }
    if (GS_hunkstorage != (void *) NULL) {
    	free(GS_hunkstorage);
    	GS_hunkstorage = (void *) NULL;
    }
    if (GS_addstorage != (void *) NULL) {
    	free(GS_addstorage);
    	GS_addstorage = (void *) NULL;
    }
    Opened = 0;
    close(GS_hunkfd);
    return (HF_NORMAL);
}
/* HF_ReadUpdateFlag
**
*/
long
HF_ReadUpdateFlag(void)
{
    HF_ReadFileHeader();
    return (GS_hfh.update_flag);
}
/* HF_IncrementUpdateFlag
**
*/
long
HF_IncrementUpdateFlag(void)
{
    GS_hfh.update_flag += 1;
    HF_WriteFileHeader();
    return (GS_hfh.update_flag);
}

/* HF_InitAddresses
**
** Purpose:
**	This function initailizes the address mapping for any hunk.
**
** Parameter Dictionary:
**	No formal parameters passed
**
** Return Values:
**	None.
**
** Algorithm:
**	This call is used to hook up all the nasty little pointers needed
**	to deal with hunks which can be sized at run time.  It's a little
**	esoteric but pretty straightforward if you study this along with the
**	data structure definitions in hunk_file.h
*/
void
HF_InitAddresses()
{
    int 		i;
    char 		*addp, *hunkp;

    addp = (char *) GS_addstorage;
    hunkp = (char *) GS_hunkstorage;
    GS_curhunk.hunk_head = (HunkHeader *) hunkp;
    hunkp += sizeof(HunkHeader);
    GS_curhunk.node_list = hunkp;
    hunkp += GS_hfh.num_recs_per_hunk;
    GS_curhunk.buf_pool = (HunkBuf *) addp;

    for (i = 0; i < GS_hfh.num_recs_per_hunk; i++) {
    	GS_curhunk.buf_pool[i].buf = hunkp;
    	hunkp += GS_hfh.hunk_record_length;
    	GS_curhunk.buf_pool[i].nextp = (HunkBufAdd *) hunkp;
    	hunkp += sizeof(HunkBufAdd);
    }

}
/* HF_Open
**
** Purpose:
**	This function opens a previously created hunk file.
**
** Parameter Dictionary:
**	char *fname:
**		a null terminated string indicating the file name to use.
**
** Return Values:
**
**	HF_NORMAL: 	The open command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened or can't be opened
**	HF_NOMEMORY:	Can't allocate enough memory for hunk storage
**	HF_READERROR:	Can't read the hunk from disk
**	HF_BADMAGIC:	Bad verification string in hunk header
**	HF_SEEKERROR:	Can't seek to a particular location
**	HF_BADHUNK:	Bad parameters in hunk header
**
** Algorithm:
**	This function attempts to open the specified hunk file.  It performs
**	some simple sanity checking to make sure that at the minimum it has the
**	proper hunk identification string in the header and that the number of
**	hunks allocated and the hunk size is a reasonable number.
*/

CONDITION
HF_Open(char *fname)
{
    int	        addsize, hunksize, hs, seek_pos, rb, openFlags = O_RDWR;

    if (Opened == 1) return (HF_OPENERROR);

#ifndef _MSC_VER
    openFlags |= O_SYNC;
#endif

    if ((GS_hunkfd = open(fname, openFlags)) < 0) return (HF_OPENERROR);

    if ((read(GS_hunkfd, (char *) &GS_hfh, sizeof(HunkFileHeader))) != sizeof(HunkFileHeader)) {
    	close(GS_hunkfd);
    	return (HF_READERROR);
    }
    if (strcmp(GS_hfh.hunk_file_id, HUNK_ID) != 0) {
    	close(GS_hunkfd);
    	return (HF_BADMAGIC);
    }
    hs = GS_hfh.hunk_length;
    if ((GS_hunkstorage == NULL) || (GS_addstorage == NULL)) {
    	/* Compute the storage needed for addresses... */
    	addsize = sizeof(HunkBuf) * GS_hfh.num_recs_per_hunk;
    	/* Compute the storage needed for the hunk in memory... */
    	hunksize = sizeof(HunkHeader);
    	hunksize += GS_hfh.num_recs_per_hunk;
    	hunksize += ((GS_hfh.hunk_record_length + sizeof(HunkBufAdd)) * GS_hfh.num_recs_per_hunk);

    	if ((GS_hunkstorage = (void *) malloc(hunksize)) == (void *) NULL) return (HF_NOMEMORY);
    	if ((GS_addstorage = (void *) malloc(addsize)) == (void *) NULL) return (HF_NOMEMORY);

    	HF_InitAddresses();
    }
    seek_pos = sizeof(HunkFileHeader);
    if ((lseek(GS_hunkfd, (off_t) seek_pos, SEEK_SET)) < 0) return (HF_SEEKERROR);

    rb = read(GS_hunkfd, (char *) GS_hunkstorage, GS_hfh.hunk_length);
    if (rb != GS_hfh.hunk_length) {
    	close(GS_hunkfd);
    	return (HF_READERROR);
    }
    if ((GS_hfh.hunks_allocated < 1) || (GS_hfh.hunk_length <= 0)) {
    	close(GS_hunkfd);
    	return (HF_BADHUNK);
    }
    GS_curhunk.hunk_head->hunk_dirty = 0;
    Opened = 1;
    return (HF_NORMAL);
}




/* HF_Close
**
** Purpose:
**	This function closes a previously opened hunk file.
**
** Parameter Dictionary:
**
**	char *fname:
**		a null terminated string indicating the file name to close.
**
** Return Values:
**
**	HF_NORMAL: 	The close command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened or can't be opened
**	HF_WRITERROR:	Can't write the hunk to disk
**	HF_CLOSERROR:	Can't close the file descriptor
**
** Algorithm:
**	This function attempts to close the specified hunk file.  It attempts
**	to write the file header and the current open hunk if need be.
*/

CONDITION
HF_Close(char *fname)
{
    CONDITION       retval;

    if (Opened == 0) return (HF_OPENERROR);

    if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) return (retval);
    if (HF_WriteFileHeader() != HF_NORMAL) return (HF_WRITERROR);

    if (GS_hunkstorage != (void *) NULL) {
    	free(GS_hunkstorage);
    	GS_hunkstorage = (void *) NULL;
    }
    if (GS_addstorage != (void *) NULL) {
    	free(GS_addstorage);
    	GS_addstorage = (void *) NULL;
    }
    if (close(GS_hunkfd) < 0) return (HF_CLOSERROR);

    Opened = 0;

    return (HF_NORMAL);
}




/* HF_AddHunk
**
** Purpose:
**	This function appends an additional hunk to an existing hunk file.
**
** Parameter Dictionary:
**	None
**
** Return Values:
**
**	HF_NORMAL: 	The add command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened or can't be opened
**	HF_SEEKERROR:	Can't seek to a particular location
**
** Algorithm:
**	This function is called internally by the HF functions when they
**	decide they need to have additional hunks...for instance when a record
**	cannot be allocated anywhere else, it must be placed in a new hunk.
*/


CONDITION
HF_AddHunk()
{

    int		        j;
    CONDITION	    retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL)	return (retval);
    if ((lseek(GS_hunkfd, (off_t) 0, SEEK_END)) < 0) return (HF_SEEKERROR);

    GS_curhunk.hunk_head->hunk_number = GS_hfh.hunks_allocated;
    GS_hfh.hunks_allocated++;
    GS_curhunk.hunk_head->hunk_dirty = 1;
    GS_curhunk.hunk_head->static_buf_node.hunk_number = HUNK_PTR_NULL;
    GS_curhunk.hunk_head->static_buf_node.node_number = HUNK_PTR_NULL;
    GS_curhunk.hunk_head->free_nodes = GS_hfh.num_recs_per_hunk;

    for (j = 0; j < GS_hfh.num_recs_per_hunk; j++) {
    	GS_curhunk.node_list[j] = HUNK_FREE;
    	GS_curhunk.buf_pool[j].nextp->hunk_number = HUNK_PTR_NULL;
    	GS_curhunk.buf_pool[j].nextp->node_number = HUNK_PTR_NULL;
    }
    if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) return (retval);
    if ((retval = HF_WriteFileHeader()) != HF_NORMAL) return (retval);

    return (HF_NORMAL);
}


/* HF_WriteCurrentHunk
**
** Purpose:
**	This function writes the current hunk out to the disk file.
**
** Parameter Dictionary:
**	None
**
** Return Values:
**
**	HF_NORMAL: 	The write current hunk command succeeded
**	HF_SEEKERROR:	Can't seek to a particular location
**	HF_WRITERROR:	Can't write the requested hunk
**
** Algorithm:
**	The hunk is only written to disk if it is dirty, that is, some
**	piece of data on the memory image of the hunk changed due to buffers
**	being allocated and deallocated, or written.
*/

CONDITION
HF_WriteCurrentHunk(void)
{

    int        wb, seek_pos;

    if (Opened == 0) return (HF_OPENERROR);

    if (GS_curhunk.hunk_head->hunk_dirty == 1) {	/* this page is dirty...write it out */
    	seek_pos = sizeof(HunkFileHeader) + (GS_curhunk.hunk_head->hunk_number * GS_hfh.hunk_length);
    	if ((lseek(GS_hunkfd, (off_t) seek_pos, SEEK_SET)) < 0) return (HF_SEEKERROR);

    	GS_curhunk.hunk_head->hunk_dirty = 0;
    	wb = write(GS_hunkfd, (char *) GS_hunkstorage, GS_hfh.hunk_length);
    	if (wb != GS_hfh.hunk_length) {
    		GS_curhunk.hunk_head->hunk_dirty = 1;
    		return (HF_WRITERROR);
    	}
    }
    return (HF_NORMAL);
}



/* HF_ReadFileHeader
**
** Purpose:
**	A convienience routine used to read the Hunk file header from the disk
**
** Parameter Dictionary:
**	None
**
** Return Values:
**
**	HF_NORMAL: 	The read file header command succeeded
**	HF_SEEKERROR:	Can't seek to a particular location
**	HF_READERROR:	Can't read the file header
**
** Algorithm:
**	N/A
*/

CONDITION
HF_ReadFileHeader()
{

    int        seek_pos;

    if (Opened == 0) return (HF_OPENERROR);

    seek_pos = 0;
    if ((lseek(GS_hunkfd, (off_t) seek_pos, SEEK_SET)) < 0) return (HF_SEEKERROR);
    if ((read(GS_hunkfd, (char *) &GS_hfh, sizeof(HunkFileHeader))) !=	sizeof(HunkFileHeader)) return (HF_READERROR);

    return (HF_NORMAL);

}
/* HF_WriteFileHeader
**
** Purpose:
**	A convienience routine used to write the Hunk file header to the disk
**
** Parameter Dictionary:
**	None
**
** Return Values:
**
**	HF_NORMAL: 	The read file header command succeeded
**	HF_SEEKERROR:	Can't seek to a particular location
**	HF_WRITERROR:	Can't write the file header
**
** Algorithm:
**	N/A
*/
CONDITION
HF_WriteFileHeader()
{

    int        seek_pos;

    if (Opened == 0) return (HF_OPENERROR);

    seek_pos = 0;
    if ((lseek(GS_hunkfd, (off_t) seek_pos, SEEK_SET)) < 0) return (HF_SEEKERROR);
    if ((write(GS_hunkfd, (char *) &GS_hfh, sizeof(HunkFileHeader))) != sizeof(HunkFileHeader)) return (HF_WRITERROR);

    return (HF_NORMAL);

}
/* HF_ReadCurrentHunk
**
** Purpose:
**	A routine used to read the requested hunk into the hunk memory space
**
** Parameter Dictionary:
**	int hunk_num:
*		the number of the hunk you want.
**
** Return Values:
**
**	HF_NORMAL: 	The read current hunk succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_BADHUNKNUM:	Invalid hunk number requested
**	HF_SEEKERROR:	Couldn't seek to requested location
**	HF_READERROR:	Couldn't read the hunk
**
** Algorithm:
**	The memory residient hunk is written if need be, then the requested
**	hunk is read from the disk given that it exists.
*/
CONDITION
HF_ReadCurrentHunk(int hunk_num)
{

    int        	rb, seek_pos;
    CONDITION   retval;

    if (Opened == 0) return (HF_OPENERROR);
    if (hunk_num >= GS_hfh.hunks_allocated)	return (HF_BADHUNKNUM);
    if (GS_curhunk.hunk_head->hunk_number == hunk_num) return (HF_NORMAL);

    if (GS_curhunk.hunk_head->hunk_dirty == 1){
    	if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) return (retval);
    }

    seek_pos = sizeof(HunkFileHeader) + (hunk_num * GS_hfh.hunk_length);
    if ((lseek(GS_hunkfd, (off_t) seek_pos, SEEK_SET)) < 0) return (HF_SEEKERROR);

    rb = read(GS_hunkfd, (char *) GS_hunkstorage, GS_hfh.hunk_length);
    if (rb != GS_hfh.hunk_length) return (HF_READERROR);

    GS_curhunk.hunk_head->hunk_dirty = 0;
    return (HF_NORMAL);
}


/* HF_AllocateStaticRecord
**
** Purpose:
**	A routine to allocate the static buffer associated with a hunk
**
** Parameter Dictionary:
**	int hunk_number:
**		the number of the hunk you want to use for the static alloc
**	int size:
**		the size of this static allocation.
**
** Return Values:
**
**	HF_NORMAL: 	The allocate command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_STATICINUSE:	The static buffer is already in use
**
** Algorithm:
**	Each hunk  can have a static buffer allocation associated with it so
**	that when the file is opened the first time, you have a "handle" to
**	go out and see what is where...this is really important if you are
**	implementing say a tree structure and need to find the root right off
**	the bat.
*/
CONDITION
HF_AllocateStaticRecord(int hunk_number, int size)
{

    HunkBufAdd 		add;
    CONDITION 		retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_AllocateRecord(size, &add)) != HF_NORMAL) return (retval);
    if ((retval = HF_ReadCurrentHunk(hunk_number)) != HF_NORMAL) return (retval);

    if (GS_curhunk.hunk_head->static_buf_node.hunk_number != HUNK_PTR_NULL && GS_curhunk.hunk_head->static_buf_node.node_number != HUNK_PTR_NULL) return (HF_STATICINUSE);

    GS_curhunk.hunk_head->static_buf_node.hunk_number = add.hunk_number;
    GS_curhunk.hunk_head->static_buf_node.node_number = add.node_number;
    GS_curhunk.hunk_head->hunk_dirty = 1;

    return (HF_NORMAL);
}

/* HF_AllocateRecord
**
** Purpose:
**	The general fixed length record allocator.
**
** Parameter Dictionary:
**	int size:
**		the size of this record allocation.
**	HunkBufAdd *address
**		Gets the address of the record just allocated.
**
** Return Values:
**
**	HF_NORMAL: 	The allocate command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_RECTOOBIG:	The record size requested is too big
**
** Algorithm:
**	This is the guts of the hunk system.  It allocates whatever size
**	record you want by linking together the proper number of buffers
**	from the buffer pool of a particular hunk.  There is waste here, but
**	it is simple to follow and not horribly slow.  One restriction is that
**	buffers cannot be larger than the size of a hunk.
*/
CONDITION
HF_AllocateRecord(int size, HunkBufAdd *address)
{

    int      	  num_bufs, i, free_index, done, prev_node, prev_hunk;
    CONDITION     retval;

    if (Opened == 0) return (HF_OPENERROR);

    if (size > (GS_hfh.hunk_record_length *	GS_hfh.num_recs_per_hunk)) return (HF_RECTOOBIG);

    num_bufs = size / GS_hfh.hunk_record_length;

    if ((size % GS_hfh.hunk_record_length) != 0) num_bufs++;
    if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) return (retval);

    done = 0;

    for (i = 0; i < GS_hfh.hunks_allocated; i++) {
    	if ((retval = HF_ReadCurrentHunk(i)) != HF_NORMAL) return (retval);

    	if (num_bufs <= GS_curhunk.hunk_head->free_nodes) {	/* we can service it here... */
    		free_index = HF_FindFreeNode();
    		GS_curhunk.buf_pool[free_index].nextp->hunk_number = -1;
    		GS_curhunk.buf_pool[free_index].nextp->node_number = -1;
    		address->hunk_number = i;
    		address->node_number = free_index;
    		num_bufs--;
    		prev_node = address->node_number;
    		prev_hunk = address->hunk_number;

    		while (num_bufs > 0) {
    			free_index = HF_FindFreeNode();
    			GS_curhunk.buf_pool[prev_node].nextp->hunk_number = prev_hunk;
    			GS_curhunk.buf_pool[prev_node].nextp->node_number = free_index;
    			GS_curhunk.buf_pool[free_index].nextp->hunk_number = -1;
    			GS_curhunk.buf_pool[free_index].nextp->node_number = -1;
    			prev_node = free_index;
    			num_bufs--;
    		}
    		done = 1;
    		break;
    	}
    }
    if (done == 0) {		/* Need to allocate a new hunk */
    	if ((retval = HF_AddHunk()) != HF_NORMAL){
    		return (retval);
    	}else{
    		if ((retval = HF_ReadCurrentHunk(GS_hfh.hunks_allocated - 1)) != HF_NORMAL){
    			return (retval);
    		}else{
    			free_index = HF_FindFreeNode();
    			GS_curhunk.buf_pool[free_index].nextp->hunk_number = -1;
    			GS_curhunk.buf_pool[free_index].nextp->node_number = -1;
    			address->hunk_number = i;
    			address->node_number = free_index;
    			num_bufs--;
    			prev_node = address->node_number;
    			prev_hunk = address->hunk_number;

    			while (num_bufs > 0) {
    				free_index = HF_FindFreeNode();
    				GS_curhunk.buf_pool[prev_node].nextp->hunk_number = prev_hunk;
    				GS_curhunk.buf_pool[prev_node].nextp->node_number = free_index;
    				GS_curhunk.buf_pool[free_index].nextp->hunk_number = -1;
    				GS_curhunk.buf_pool[free_index].nextp->node_number = -1;
    				prev_node = free_index;
    				num_bufs--;
    			}
    		}
    	}
    }
    return (HF_NORMAL);
}

/* HF_FindFreeNode
**
** Purpose:
**	A simple routine to find a free node in the buffer pool
**
** Parameter Dictionary:
**	None.
**
** Return Values:
**	A free node index.
**
** Algorithm:
**	This routine is never called unless a free node exists which is why
**	it doesn't have an error return.
**	And, yes, it is a linear search, again we could do more here but there
**	is the issue of time, and these things tend to be small.
*/
int
HF_FindFreeNode()
{
    int 	i;

    for (i = 0; i < GS_hfh.num_recs_per_hunk; i++){
    	if (GS_curhunk.node_list[i] == HUNK_FREE) {
    		GS_curhunk.node_list[i] = HUNK_USED;
    		GS_curhunk.hunk_head->free_nodes--;
    		GS_curhunk.hunk_head->hunk_dirty = 1;
    		return (i);
    	}
    }
}
/* HF_DeallocateRecord
**
** Purpose:
**	A routine to deallocate a previously allocated record.
**
** Parameter Dictionary:
**	HunkBufAdd *address:
**		The address of the buffer to deallocate.
**
** Return Values:
**
**	HF_NORMAL: 	The deallocate command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**
** Algorithm:
**	This routine simply frees the allocated buffer and any buffers
**	previously linked to it.
*/
CONDITION
HF_DeallocateRecord(HunkBufAdd *address)
{
    int	        cur_node;
    CONDITION   retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_ReadCurrentHunk(address->hunk_number)) != HF_NORMAL) return (retval);

    cur_node = address->node_number;

    do {
    	GS_curhunk.node_list[cur_node] = HUNK_FREE;
    	(GS_curhunk.hunk_head->free_nodes)++;
    	cur_node = GS_curhunk.buf_pool[cur_node].nextp->node_number;
		} while (cur_node != -1);

	GS_curhunk.hunk_head->hunk_dirty = 1;
	address->node_number = HUNK_PTR_NULL;
	address->hunk_number = HUNK_PTR_NULL;

	return (HF_NORMAL);
}
/* HF_DeallocateStaticRecord
**
** Purpose:
**	A routine to deallocate a previously allocated static record.
**
** Parameter Dictionary:
**	int hunk_number
**		The hunk_number of the statically allocated buffer.
**
** Return Values:
**
**	HF_NORMAL: 	The deallocate static command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**
** Algorithm:
**	This routine simply frees the staticly allocated buffer
**	associated with a hunk.  Static buffers have there addresses stored
**	with the database so they don't need to be passed.
*/
CONDITION
HF_DeallocateStaticRecord(int hunk_number)
{
    HunkBufAdd 		add;
    CONDITION 		retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_ReadCurrentHunk(hunk_number)) != HF_NORMAL) return (retval);

    add.hunk_number = GS_curhunk.hunk_head->static_buf_node.hunk_number;
    add.node_number = GS_curhunk.hunk_head->static_buf_node.node_number;
    GS_curhunk.hunk_head->static_buf_node.hunk_number = HUNK_PTR_NULL;
    GS_curhunk.hunk_head->static_buf_node.node_number = HUNK_PTR_NULL;
    GS_curhunk.hunk_head->hunk_dirty = 1;
    if ((retval = HF_DeallocateRecord(&add)) != HF_NORMAL) return (retval);

    return (HF_NORMAL);
}
/* HF_Dumper
**
** Purpose:
**	A test routine to dump the data records and see links
**
** Parameter Dictionary:
**
** Return Values:
**	None.
**
** Algorithm:
**	No real algorithm.
*/
void
HF_Dumper(void)
{

    int 	i, j, k;

    printf("Allocated: %d, Length: %d, Rec Len: %d Recs/hunk: %d ID: %s\n\n", GS_hfh.hunks_allocated, GS_hfh.hunk_length, GS_hfh.hunk_record_length, GS_hfh.num_recs_per_hunk, GS_hfh.hunk_file_id);

    for (i = 0; i < GS_hfh.hunks_allocated; i++) {
    	HF_ReadCurrentHunk(i);
    	printf("Hunk number: %d\n", GS_curhunk.hunk_head->hunk_number);
    	printf("Hunk dirty: %d\n", GS_curhunk.hunk_head->hunk_dirty);
    	printf("Hunk static_buf_node: %d %d\n", GS_curhunk.hunk_head->static_buf_node.hunk_number, GS_curhunk.hunk_head->static_buf_node.node_number);
    	printf("Hunk free_nodes: %d\n", GS_curhunk.hunk_head->free_nodes);

    	if (GS_curhunk.hunk_head->free_nodes < 100) {
    		for (j = 0; j < 100; j++) {
    			if (GS_curhunk.node_list[j] == HUNK_USED) {
    				printf("\tBuffer %3d [Link: %3d] ", j, GS_curhunk.buf_pool[j].nextp->node_number);

    				for (k = 0; k < 15; k++){
    					printf("%c", GS_curhunk.buf_pool[j].buf[k]);
    				}
    				printf(" ... ");

    				for (k = GS_hfh.hunk_record_length - 15; k < GS_hfh.hunk_record_length; k++){
    					printf("%c", GS_curhunk.buf_pool[j].buf[k]);
    				}
    				printf("\n");
    			}
    		}
    	}
    	printf("\n\n");
    }
}
/* HF_ReadStaticRecord
**
** Purpose:
**	A routine to read a previously allocated static record.
**
** Parameter Dictionary:
**	int hunk_number
**		The hunk_number of the statically allocated record.
**	int size:
**		The size of the user buffer.
**	void *buf:
**		The user buffer
**
** Return Values:
**
**	HF_NORMAL: 	The read static record command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**
** Algorithm:
**	Simply reads the data from the static buffer associated with a
**	particular hunk into the user buffer pointed to by buf.  It will
**	read size bytes regardless of the size of the static buffer which
**	is not retained by the system.
*/
CONDITION
HF_ReadStaticRecord(int hunk_number, int size, void *buf)
{
    HunkBufAdd	    add;
    CONDITION 		retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_ReadCurrentHunk(hunk_number)) != HF_NORMAL) return (retval);

    add.hunk_number = GS_curhunk.hunk_head->static_buf_node.hunk_number;
    add.node_number = GS_curhunk.hunk_head->static_buf_node.node_number;

    return (HF_ReadRecord(&add, size, buf));
}
/* HF_ReadRecord
**
** Purpose:
**	A general purpose routine to read a previously allocated record.
**
** Parameter Dictionary:
**	HunkBufAdd *add:
**		The address of the allocated record.
**	int size:
**		The size of the user buffer.
**	void *buf:
**		The user buffer
**
** Return Values:
**
**	HF_NORMAL: 	The read record command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_BADNODENUM:	Bad node number in address
**
** Algorithm:
**	Simply reads the data from the buffer associated with the address
**	passed into the user buffer pointed to by buf.  It will
**	read size bytes regardless of the size of the buffer which
**	is not retained by the system.
*/
CONDITION
HF_ReadRecord(HunkBufAdd *add, int bufsize, void *buffer)
{
    int        	left_to_transfer, current_node;
    char       	*pbuffer;
    CONDITION 	retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_ReadCurrentHunk(add->hunk_number)) != HF_NORMAL) return (retval);
    if (add->node_number < 0) return (HF_BADNODENUM);

    if (bufsize <= GS_hfh.hunk_record_length) {
    	memcpy((char *) buffer, (char *) GS_curhunk.buf_pool[add->node_number].buf, bufsize);
    }else{
    	left_to_transfer = bufsize;
    	current_node = add->node_number;
    	pbuffer = (char *) buffer;

    	while (left_to_transfer > GS_hfh.hunk_record_length) {
    		memcpy((char *) pbuffer, (char *) GS_curhunk.buf_pool[current_node].buf, GS_hfh.hunk_record_length);
    		left_to_transfer -= GS_hfh.hunk_record_length;
    		pbuffer += GS_hfh.hunk_record_length;
    		current_node = GS_curhunk.buf_pool[current_node].nextp->node_number;

    		if (current_node < 0) return (HF_BADNODENUM);
    	}
    	memcpy((char *) pbuffer, (char *) GS_curhunk.buf_pool[current_node].buf, left_to_transfer);
    }
    return (HF_NORMAL);
}

/* HF_WriteStaticRecord
**
** Purpose:
**	A general purpose routine to write a previously allocated static record.
**
** Parameter Dictionary:
**	int hunk_number
**		The hunk number of the statically allocated record.
**	int size:
**		The size of the user buffer.
**	void *buf:
**		The user buffer
**
** Return Values:
**
**	HF_NORMAL: 	The write static command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**
** Algorithm:
**	Simply writes the data to static hunk buffers specified  by the hunk
**	number passed.  It will write size bytes from the users buffer
**	regardless of the size of the buffer which is not retained by the
**	system.
*/
CONDITION
HF_WriteStaticRecord(int hunk_number, int size, void *buf)
{

    HunkBufAdd       add;
    CONDITION        retval;

    if (Opened == 0) return (HF_OPENERROR);
    if ((retval = HF_ReadCurrentHunk(hunk_number)) != HF_NORMAL) return (retval);

    add.hunk_number = GS_curhunk.hunk_head->static_buf_node.hunk_number;
    add.node_number = GS_curhunk.hunk_head->static_buf_node.node_number;

    return (HF_WriteRecord(&add, size, buf));
}

/* HF_WriteRecord
**
** Purpose:
**	A general purpose routine to write a previously allocated record.
**
** Parameter Dictionary:
**	HunkBufAdd *add:
**		The address of the allocated record.
**	int size:
**		The size of the user buffer.
**	void *buf:
**		The user buffer
**
** Return Values:
**
**	HF_NORMAL: 	The creation command succeeded
**	HF_OPENERROR: 	The hunk_file is already opened
**	HF_BADNODENUM:	Address contains a bad node number
**
** Algorithm:
**	Simply writes the data to hunk buffers specified  by the address
**	passed.  It will write size bytes from the users buffer regardless
**	regardless of the size of the buffer which is not retained by the
**	system.
*/
CONDITION
HF_WriteRecord(HunkBufAdd *add, int bufsize, void *buffer)
{
    int        	left_to_transfer, current_node;
    char       	*pbuffer;
    CONDITION 	retval;

    if (Opened == 0) return (HF_NORMAL);
    if ((retval = HF_ReadCurrentHunk(add->hunk_number)) != HF_NORMAL) return (retval);
    if (add->node_number < 0) return (HF_BADNODENUM);

    if (bufsize <= GS_hfh.hunk_record_length) {
    	memcpy((char *) GS_curhunk.buf_pool[add->node_number].buf, (char *) buffer, bufsize);
    }else{
    	left_to_transfer = bufsize;
    	current_node = add->node_number;
    	pbuffer = (char *) buffer;

    	while (left_to_transfer > GS_hfh.hunk_record_length) {
    		memcpy((char *) GS_curhunk.buf_pool[current_node].buf, pbuffer, GS_hfh.hunk_record_length);
    		left_to_transfer -= GS_hfh.hunk_record_length;
    		pbuffer += GS_hfh.hunk_record_length;
    		current_node = GS_curhunk.buf_pool[current_node].nextp->node_number;

    		if (current_node < 0) return (HF_BADNODENUM);
    	}
    	memcpy((char *) GS_curhunk.buf_pool[current_node].buf, pbuffer, left_to_transfer);
    }
    GS_curhunk.hunk_head->hunk_dirty = 1;
    if ((retval = HF_WriteCurrentHunk()) != HF_NORMAL) return (retval);

    return (HF_NORMAL);
}
CONDITION
HF_ExclusiveLock(void)
{
    int        retry;

    if (Opened == 0) return (HF_OPENERROR);

    retry = 0;
#if defined(SOLARIS) || defined(HPUX)
    while ((lockf(GS_hunkfd, F_TLOCK, 0) == -1) && (retry < 10)) {
#elif defined (_MSC_VER)
	while ((_locking(GS_hunkfd, LK_NBLCK, 0L) == -1) && (retry < 10)) {
#else
    while ((flock(GS_hunkfd, LOCK_EX) == -1) && (retry < 10)) {
#endif

#if defined _MSC_VER
    	Sleep(1000);
#else
    	sleep(1);
#endif
    	retry++;
    }
    if (retry >= 10) return (HF_LOCKERROR);

    return (HF_NORMAL);
}
CONDITION
HF_SharedLock(void)
{
#if defined _MSC_VER
    return HF_LOCKERROR;	/* Punt under MSC */
#else

    int        retry;

    if (Opened == 0) return (HF_OPENERROR);

    retry = 0;
#if defined(SOLARIS) || defined(HPUX)
    while ((lockf(GS_hunkfd, F_TLOCK, 0) == -1) && (retry < 10)) {
#else
    while ((flock(GS_hunkfd, LOCK_SH) == -1) && (retry < 10)) {
#endif
    	sleep(1);
    	retry++;
    }

    if (retry >= 10) return (HF_LOCKERROR);

    return (HF_NORMAL);
#endif
}
CONDITION
HF_UnLock(void)
{
    if (Opened == 0) return (HF_OPENERROR);

#if defined(SOLARIS) || defined(HPUX)
    if (lockf(GS_hunkfd, F_ULOCK, 0) == -1) {
#elif defined _MSC_VER
	if (_locking(GS_hunkfd, LK_UNLCK, 0L) == -1) {
#else
    if (flock(GS_hunkfd, LOCK_UN) == -1) {
#endif
    	return (HF_UNLOCKERROR);
    }
    return (HF_NORMAL);
}
