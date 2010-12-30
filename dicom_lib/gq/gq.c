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
** Module Name(s):	GQ_KillQueue, GQ_InitQueue, GQ_Enqueue, GQ_Dequeue,
**			GQ_GetQueue, GQ_PrintQueue, GQ_Wait, GQ_Signal,
**			GQ_MakeFilename
** Author, Date:	David E. Beecher, 18-June-93
** Intent:		These routines implement the generalized queueing mechanism
**			to allow multiple processes to communicate information that
**			needs to be shared between them.
** Last Update:		$Author: smm $, $Date: 1999/12/29 05:01:43 $
** Source File:		$RCSfile: gq.c,v $
** Revision:		$Revision: 1.20 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.20 $ $RCSfile: gq.c,v $";

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#endif
#include 	<sys/stat.h>
#ifndef _MSC_VER
#include 	<sys/file.h>
#endif

#include	"../dicom/dicom.h"
#include	"../condition/condition.h"
#include 	"gq.h"

/*
 * static structures for the queue...
 */
static void 			*GQ;		/* Pointer to the queue elements	 */
static QUEUE_Pointers 	*QPTRS;		/* Pointer to the head, tail, and size	 */
static int 				SHM_ID;		/* the shared memory id	*/
static int 				SEM_ID;		/* the semaphore id	*/
static char 			*SHM_ADD;	/* the address of the shared memory	 */
/*
 * A flag to remember if we did a GetQueue yet...
 */
static int SM_attached = FALSE;

#if defined(SHARED_MEMORY) && defined(SEMAPHORE) && !defined(LINUX)
static union semun2 {		/* the struct needed for semaphore ops	 */
    int 				val;
    struct semid_ds 	*buf;
    ushort 				*array;
}   sem_buf;
#endif

#if defined(SHARED_MEMORY) && defined(SEMAPHORE) && defined(LINUX)
#if defined (_SEM_SEMUN_UNDEFINED)
  union semun {
    int 					val;	/* <= value for SETVAL */
    struct semid_ds 		*buf;	/* <= buffer for IPC_STAT & IPC_SET */
    unsigned short int 		*array;	/* <= array for GETALL & SETALL */
    struct seminfo 			*__buf;	/* <= buffer for IPC_INFO */
  } sem_buf;
#else
union semun sem_buf;
#endif
#endif

#define GQ_UNIMPLEMENTED_TXT "GQ Routine %s is unimplemented on this machine architecture"

static CONDITION
access_queue(int qid);

/* GQ_KillQueue
**
** Purpose:
**	This function removes a previously allocated queue from the system
**
** Parameter Dictionary:
**	int qid:	The queue to kill
**
** Return Values:
**	GQ_NORMAL:		Operation completed successfully
**	GQ_SHAREDMEMORYFAIL:	Couldn't access shared memory
**	GQ_SEMAPHOREFAIL:	Couldn't access semaphore resources
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**
** Algorithm:
**	This routine operates on the currently active queue.  It attempts
**	to detach the shared memory segment holding the queue and then
**	frees up the associated semaphore.  As a final cleanup it removes
**	the small ascii file used to communicate between processes.
*/
CONDITION
GQ_KillQueue(int qid)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    CONDITION 			ret;
    struct shmid_ds 	shm_buf;

    ret = access_queue(qid);

    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_KillQueue: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if ((shmdt(SHM_ADD)) == -1) {
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "GQ_KillQueue: shmdt failed");
    	return (GQ_SHAREDMEMORYFAIL);
    }
    SM_attached = FALSE;

    if ((shmctl(SHM_ID, IPC_RMID, &shm_buf)) == -1) {
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "GQ_KillQueue: shmctl failed");
    	return (GQ_SHAREDMEMORYFAIL);
    }
    if ((semctl(SEM_ID, 0, IPC_RMID, sem_buf)) == -1) {
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_SemaphoreRelease: semctl failed");
    	return (GQ_SEMAPHOREFAIL);
    }
    unlink(GQ_MakeFilename(qid));

    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_KillQueue");
#endif
}



/* GQ_InitQueue
**
** Purpose:
**	This function initializes a new queue for the system
**
** Parameter Dictionary:
**	int qid:		The queue to initialize
**	int num_elements:	the number of queue elements to allocate
**	int element_size:	the size of the queue element to allocate
**
** Return Values:
**	GQ_NORMAL:		Operation completed successfully
**	GQ_SHAREDMEMORYFAIL:	Couldn't access shared memory resources
**	GQ_SEMAPHOREFAIL:	Couldn't access semaphore resources
**	GQ_FILEACCESSFAIL:	Couldn't access communication file
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**	None
**
** Algorithm:
**	This routine attempts to create a new queue with the specified
**	queue id (qid), with num_elements each of size element_size.  It
**	needs to allocate a chunk of shared memory the correct size, and
**	allocate and initialize a semaphore for exclusive access, either
**	of which may fail.  If all these succeed, it creates the communications
**	file and does a GQ_GetQueue before returning success.
*/

CONDITION
GQ_InitQueue(int qid, int num_elements, int element_size)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    int 	q_size;			/* size of the queue and pointers	 */
    key_t 	key;			/* key for the shared mem		 */
    char 	*shmadd,		/* temp shared mem address		 */
		    *fname;			/* possibly long filename for queue	 */
    FILE 	*fopen(),		/* for file access			 */
    		*fd;
    struct shmid_ds 	shm_buf; /* In case we need to abort...		 */
    struct stat	        sbuf; 	 /* for getting file status information */

/* check whether a generalized queue already exists for the specified queue  ID */

    fname = GQ_MakeFilename(qid);
    if (fname == NULL) {
    	COND_PushCondition(GQ_FILEACCESSFAIL, "%s", "GQ_InitQueue: Cannot access GQ_QUEUEDIRECTORY/gq???.dat");
    	return (GQ_FILEACCESSFAIL);
    }
    if (stat(fname, &sbuf) < 0) {
    	if (errno == ENOENT) { /* file does not exist and we need to create it */
    		if ((fd = fopen(fname, "w")) == NULL) {
    			COND_PushCondition(GQ_FILECREATEFAILED, "%s: %s", "GQ_InitQueue: Cannot create", fname);
    			return (GQ_FILECREATEFAILED);
    		}
    	}else{
    		return COND_PushCondition(GQ_FILEACCESSFAIL, "%s", "GQ_InitQueue: Cannot access GQ_QUEUEDIRECTORY/gq???.dat");
    	}
	}else{	/* GQ already exists for the given identifier */
		return COND_PushCondition(GQ_MULTCREATEREQUEST, "GQ_InitQueue: GQ with ID (%d) already exists", qid);
    }
/*
 * Acquire and attach to the shared memory...
 */
    key = IPC_PRIVATE;

    q_size = (num_elements * element_size) + sizeof(QUEUE_Pointers);

    if ((SHM_ID = shmget(key, (int) q_size, IPC_EXCL | IPC_CREAT | 0777)) == -1) {
    	perror("GQ_InitQueue: shmget failed");
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "GQ_InitQueue: shmget failed");
    	unlink(fname);
    	return (GQ_SHAREDMEMORYFAIL);
    }
    if (SM_attached == TRUE) {
    	shmdt(SHM_ADD);
    	SM_attached = FALSE;
    }
    SHM_ADD = 0;

    if ((shmadd = (char *) shmat(SHM_ID, SHM_ADD, SHM_RND)) == (char *) -1) {
    	shmctl(SHM_ID, IPC_RMID, &shm_buf);
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "GQ_InitQueue: shmat failed");
    	unlink(fname);
    	return (GQ_SHAREDMEMORYFAIL);
    }
/*
 * Save for later detaching if needed
 */
    SHM_ADD = shmadd;

/*
 * Acquire and initialize the semaphore...
 */
    if ((SEM_ID = semget(key, 1, IPC_EXCL | IPC_CREAT | 0777)) == -1) {
    	perror("GQ_InitQueue: semget failed");
    	shmdt(SHM_ADD);
    	shmctl(SHM_ID, IPC_RMID, &shm_buf);
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_InitQueue: semget failed");
    	unlink(fname);
    	return (GQ_SEMAPHOREFAIL);
    }
    sem_buf.val = 1;
    if ((semctl(SEM_ID, 0, SETVAL, sem_buf)) == 1) {
    	shmdt(SHM_ADD);
    	shmctl(SHM_ID, IPC_RMID, &shm_buf);
    	semctl(SEM_ID, 0, IPC_RMID, sem_buf);
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_InitQueue: semctl failed");
    	unlink(fname);
    	return (GQ_SEMAPHOREFAIL);
    }
/*
 * Map structure for head/tail pointers into shared memory
 */
    QPTRS = (QUEUE_Pointers *) shmadd;
    shmadd += sizeof(QUEUE_Pointers);

/*
 * Map structure for queue into shared memory
 */
    GQ = (void *) shmadd;

    QPTRS->GQ_head = QPTRS->GQ_tail = 0;
    QPTRS->GQ_numelements = num_elements;
    QPTRS->GQ_elementsize = element_size;
/*
 * Include the shared memory and semaphore ids in the GQ file created
 * above
 */

    fprintf(fd, "%d\n", SEM_ID);
    fprintf(fd, "%d\n", SHM_ID);
    fclose(fd);

    SM_attached = TRUE;
    return (GQ_GetQueue(qid, QPTRS->GQ_elementsize));
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_InitQueue");
#endif
}


/* GQ_Enqueue
**
** Purpose:
**	This function places a new element on the rear of the queue
**
** Parameter Dictionary:
**	void *element:		a pointer to the new element to store
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_SEMAPHOREFAIL:	Could not access semaphore resource
**	GQ_QUEUEFULL:		The current queue is full
**	GQ_NOPENQUEUE:		No queue is currently open
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**	It is the users responsibility to ensure that the size of the
**	element referred to with the pointer parameter is the correct
**	size for this particular queue.
**
** Algorithm:
**	The element pointed to by the input parameter, element, is
**	copied to the tail of the currently selected queue.
**
*/

CONDITION
GQ_Enqueue(int qid, void *element)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    CONDITION		ret;
    char		    *ep;


    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Enqueue: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Enqueue: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Enqueue: Semaphore failure");
    	return (ret);
    }
    if ((QPTRS->GQ_head == (QPTRS->GQ_tail + 1)) ||	((QPTRS->GQ_head == 0) && (QPTRS->GQ_tail == (QPTRS->GQ_numelements - 1)))) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_Enqueue: Semaphore failure");
    		return (ret);
    	}
    	COND_PushCondition(GQ_QUEUEFULL, "GQ_Enqueue: Queue (ID %d) full", qid);
    	return (GQ_QUEUEFULL);
    }
    ep = (char *) GQ;
    ep += (QPTRS->GQ_tail * QPTRS->GQ_elementsize);
    memcpy(ep, (char *) element, QPTRS->GQ_elementsize);

    if (++QPTRS->GQ_tail >= QPTRS->GQ_numelements)
	QPTRS->GQ_tail = 0;

    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Enqueue: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_Enqueue");
#endif
}



/* GQ_Dequeue
**
** Purpose:
**	This function removes an element from the head of the queue
**
** Parameter Dictionary:
**	void *element:		storage for the retreived element
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_SEMAPHOREFAIL:	Could not access semaphore resource
**	GQ_QUEUEMPTY:		The current queue is empty
**	GQ_NOPENQUEUE:		No queue is currently open
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**	It is the users responsibility to ensure that the size of the
**	element referred to with the pointer parameter is the correct
**	size for this particular queue.
**
** Algorithm:
**	The element pointed to by the input parameter, element, is
**	replaced by the head of the queue.
**
*/

CONDITION
GQ_Dequeue(int qid, void *element)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    CONDITION		ret;
    char	        *ep;

/*
 * Acquire exclusive access to the queue and make sure we have an
 * element to retreive.
 */
    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Dequeue: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Dequeue: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Dequeue: Semaphore failure");
    	return (ret);
    }
    if (QPTRS->GQ_head == QPTRS->GQ_tail) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_Dequeue: Semaphore failure");
    		return (ret);
    	}
    	COND_PushCondition(GQ_QUEUEEMPTY, "GQ_Dequeue: Queue (ID %d) empty", qid);
    	return (GQ_QUEUEEMPTY);
    }
/*
 * Copy the next item out of the queue
 */
    ep = (char *) GQ;
    ep += (QPTRS->GQ_head * QPTRS->GQ_elementsize);
    memcpy((char *) element, ep, QPTRS->GQ_elementsize);

/*
 * Check for wrap-around condition
 */
    if (++QPTRS->GQ_head >= QPTRS->GQ_numelements) QPTRS->GQ_head = 0;

    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Dequeue: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_Dequeue");
#endif
}



/* GQ_GetQueue
**
** Purpose:
**	Attach to a new queue
**
** Parameter Dictionary:
**	int qid:		attempt to attach to this queue
**	int element_size:	the element size of this queue.
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_SHAREDMEMORYFAIL:	Could not access shared memory resource
**	GQ_FILEACCESSFAIL:	Could not access communications file
**	GQ_BADELEMSIZE:		Bad element size specification
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**
** Algorithm:
**
*/

CONDITION
GQ_GetQueue(int qid, int element_size)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    FILE 		*fopen(), *fd;
    char 		*shmadd, *fname;

    if ((fname = GQ_MakeFilename(qid)) == NULL) {
    	COND_PushCondition(GQ_FILEACCESSFAIL, "%s", "GQ_GetQueue: Cannot access GQ_QUEUEDIRECTORY/gq???.dat");
    	return (GQ_FILEACCESSFAIL);
    }
    if ((fd = fopen(fname, "r")) == NULL) {
    	COND_PushCondition(GQ_FILEACCESSFAIL, "%s", "GQ_GetQueue: Cannot access GQ_QUEUEDIRECTORY/gq???.dat");
    	return (GQ_FILEACCESSFAIL);
    }
    fscanf(fd, "%d", &SEM_ID);
    fscanf(fd, "%d", &SHM_ID);
    fclose(fd);

    if (SM_attached == TRUE) {
    	shmdt(SHM_ADD);
    	SM_attached = FALSE;
    }
    SHM_ADD = 0;
    if ((shmadd = (char *) shmat(SHM_ID, SHM_ADD, SHM_RND)) == (char *) -1) {
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "GQ_GetQueue: shmat failed");
    	return (GQ_SHAREDMEMORYFAIL);
    }
    SHM_ADD = shmadd;
/*
 * Map structure for head/tail pointers into shared memory
 */
    QPTRS = (QUEUE_Pointers *) shmadd;
    shmadd += sizeof(QUEUE_Pointers);

    if (QPTRS->GQ_elementsize != element_size) {
    	shmdt(SHM_ADD);
    	COND_PushCondition(GQ_BADELEMSIZE, "GQ_GetQueue: Bad element size specification");
    	return (GQ_BADELEMSIZE);
    }
/*
 * Map structure for queue into shared memory
 */
    GQ = (void *) shmadd;

    SM_attached = TRUE;
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_GetQueue");
#endif
}


/* GQ_PrintQueue
**
** Purpose:
**	Print all the elements of a queue to stdout
**
** Parameter Dictionary:
**	void (print_func(void *)):	the printing function
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_NOPENQUEUE:		No queue is currently open
**	GQ_SEMAPHOREFAIL:	Could not access semaphore resources
**	GQ_QUEUEEMPTY:		This queue is empty
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**
** Algorithm:
**	Just a utility routine meant for developers.  Since the queueing mechanisms
**	knows nothing about what an element looks like (except it's size),
**	a printing routine must be passed as an input parameter so that each
**	element can be successfully printed.
**
*/
CONDITION
GQ_PrintQueue(int qid, void (print_func(void *)))
{
#if defined(SHARED_MEMORY) && defined(SEMAPHORE)
    CONDITION		ret;
    int		        ploc;
    char		    *ep;

    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_PrintQueue: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_PrintQueue: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_PrintQueue: Semaphore failure");
    	return (ret);
    }
    if (QPTRS->GQ_head == QPTRS->GQ_tail) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_PrintQueue: Semaphore failure");
    		return (ret);
    	}
    	return (GQ_QUEUEEMPTY);
    }
    printf("<<<< HEAD >>>>\n");
    ploc = QPTRS->GQ_head;

    while (ploc != QPTRS->GQ_tail) {
    	ep = (char *) GQ;
    	ep += (ploc * QPTRS->GQ_elementsize);
    	print_func((void *) ep);
    	/* Check for wrap-around condition */
    	if (++ploc >= QPTRS->GQ_numelements) ploc = 0;
    }

    printf("<<<< TAIL >>>>\n");

    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_PrintQueue: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_PrintQueue");
#endif
}


CONDITION
GQ_PeekQueue(int qid, void *element)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    CONDITION		ret;
    char	        *ep;

/*
 * Acquire exclusive access to the queue and make sure we have an
 * element to retreive.
 */
    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Peek: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Peek: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Peek: Semaphore failure");
    	return (ret);
    }
    if (QPTRS->GQ_head == QPTRS->GQ_tail) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_Peek: Semaphore failure");
    		return (ret);
    	}
    	return (GQ_QUEUEEMPTY);
    }

    /* Copy the next item out of the queue */
    if (!element) return COND_PushCondition(GQ_NOMEMORY, "%s", "GQ_PeekQueue, No memory for allocated for element");

    ep = (char *) GQ;
    ep += (QPTRS->GQ_head * QPTRS->GQ_elementsize);
    memcpy((char *) element, ep, QPTRS->GQ_elementsize);

    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Peek: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_Peek");
#endif
}

CONDITION
GQ_ModifyHeadElement(int qid, void *element, void (*func) (void *element))
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    CONDITION	ret;
    char        *ep;

/*
 * Acquire exclusive access to the queue and make sure we have an
 * element to retreive.
 */
    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Modify: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_Modify: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Modify: Semaphore failure");
    	return (ret);
    }
    if (QPTRS->GQ_head == QPTRS->GQ_tail) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_Modify: Semaphore failure");
    		return (ret);
    	}
    	return (GQ_QUEUEEMPTY);
    }

    /* Copy the next item out of the queue */
    if (!element) return COND_PushCondition(GQ_NOMEMORY, "%s", "GQ_ModifyHeadElement: No memory for element");

    ep = (char *) GQ;
    ep += (QPTRS->GQ_head * QPTRS->GQ_elementsize);
    memcpy((char *) element, ep, QPTRS->GQ_elementsize);

    /* operate on this element */
    if (func) func(element); /* user defined function */

    /* copy the element back */
    memcpy(ep, (char *) element, QPTRS->GQ_elementsize);
    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_Modify: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_Modify");
#endif
}
/* GQ_Wait
**
** Purpose:
**	Wait for the semaphore of the current queue to be free and acquire it
**
** Parameter Dictionary:
**	none
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_SEMPHOREFAIL:	Could not access semaphore resources
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**
** Algorithm:
**	Just wait for the semaphore to become free and acquire it...this
**	is all accomplished as a non-interruptible operation using unix
**	semaphore facilities
**
*/

CONDITION
GQ_Wait(void)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = SEM_UNDO;
    if ((semop(SEM_ID, &op, (int) 1)) == -1) {
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_Wait: semop failed");
    	return (GQ_SEMAPHOREFAIL);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_Wait");
#endif
}



/* GQ_Signal
**
** Purpose:
**	Let the semaphore go.
**
** Parameter Dictionary:
**	none
**
** Return Values:
**	GQ_NORMAL:		Enqueue operation successful
**	GQ_SEMPHOREFAIL:	Could not access semaphore resources
**	GQ_UNIMPLEMENTED:	This feature is unimplemented on this architecture
**
**
** Notes:
**
** Algorithm:
**	Release the currently held semaphore
**
*/
CONDITION
GQ_Signal(void)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

#ifdef ASG
    sem_buf.val = 1;		/* this is the global buffer 	 */

    if ((semctl(SEM_ID, 0, SETVAL, sem_buf)) == -1) {
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_Signal: semctl failed");
    	return (GQ_SEMAPHOREFAIL);
    }
#endif
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = SEM_UNDO;
    if ((semop(SEM_ID, &op, (int) 1)) == -1) {
    	COND_PushCondition(GQ_SEMAPHOREFAIL, "%s", "GQ_Wait: semop failed");
    	return (GQ_SEMAPHOREFAIL);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED,
			      GQ_UNIMPLEMENTED_TXT, "GQ_Signal");
#endif
}


/* GQ_MakeFilename
**
*/

char *
GQ_MakeFilename(int qid)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    static char 	buf[GQ_MAXSTRINGLENGTH];
    char 			*p, temp[10];

    if (qid > GQ_MAXNAMEDQUEUES) {
    	(void) COND_PushCondition(GQ_MAXQUEUEEXCEEDED, "GQ QID Exceeded maximum allowed queue number (%d %d)", GQ_MAXNAMEDQUEUES, qid);
    	return (NULL);
    }
    if ((p = getenv(GQ_QUEUEDIRECTORY)) == NULL) return (NULL);

    strcpy(buf, p);
    if (buf[strlen(buf) - 1] != '/') strcat(buf, "/");

    strcat(buf, GQ_QUEUEFILESUFFIX);
    sprintf(temp, "%d", qid);
    strcat(buf, temp);

    return (buf);

#else
    return NULL;
#endif
}

/* GQ_GetQueueSize
**
** Purpose:
**	Return the number of elements in the queue
**
** Parameter Dictionary:
**	gqID	: Queue Identifier
**	size	: Pointer to user defined location where the size
**		  is returned
**
** Return Values:
**
** Notes:
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
CONDITION
GQ_GetQueueSize(int qid, int *size)
{
#if defined(SHARED_MEMORY) && defined(SEMAPHORE)
    CONDITION		ret;
    int		        ploc;
    char	        *ep;

    if (!size) return COND_PushCondition(GQ_NOMEMORY, "GQ_GetQueueSize: No memory allocated for returning length");
    *size = 0;
    ret = access_queue(qid);
    if (ret != GQ_NORMAL) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_GetQueueSize: No open queue (ID : %d)", qid);
    	return (GQ_NOPENQUEUE);
    }
    if (SM_attached == FALSE) {
    	COND_PushCondition(GQ_NOPENQUEUE, "GQ_GetQueueSize: No open queue");
    	return (GQ_NOPENQUEUE);
    }
    if ((ret = GQ_Wait()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_GetQueueSize: Semaphore failure");
    	return (ret);
    }
    if (QPTRS->GQ_head == QPTRS->GQ_tail) {
    	if ((ret = GQ_Signal()) != GQ_NORMAL) {
    		COND_PushCondition(ret, "%s", "GQ_GetQueueSize: Semaphore failure");
    		return (ret);
    	}
    	return (GQ_QUEUEEMPTY);
    }
    ploc = QPTRS->GQ_head;

    while (ploc != QPTRS->GQ_tail) {
    	ep = (char *) GQ;
    	ep += (ploc * QPTRS->GQ_elementsize);
    	*size = *size + 1;

    	/* Check for wrap-around condition */
    	if (++ploc >= QPTRS->GQ_numelements) ploc = 0;
    }

    if ((ret = GQ_Signal()) != GQ_NORMAL) {
    	COND_PushCondition(ret, "%s", "GQ_GetQueueSize: Semaphore failure");
    	return (ret);
    }
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "GQ_GetQueueSize");
#endif
}

static CONDITION
access_queue(int qid)
{

#if defined(SHARED_MEMORY) && defined(SEMAPHORE)

    FILE 		*fopen(), *fd;
    char 		*shmadd, *fname;

    if ((fname = GQ_MakeFilename(qid)) == NULL) {
    	COND_PushCondition(GQ_FILEACCESSFAIL, "%s", "access_queue: Cannot access GQ_QUEUEDIRECTORY/gq???.dat");
    	return (GQ_FILEACCESSFAIL);
    }
    if ((fd = fopen(fname, "r")) == NULL) {
    	COND_PushCondition(GQ_FILEACCESSFAIL, "access_queue: Cannot access file (%s)", fname);
    	return (GQ_FILEACCESSFAIL);
    }
    fscanf(fd, "%d", &SEM_ID);
    fscanf(fd, "%d", &SHM_ID);
    fclose(fd);

    if (SM_attached == TRUE) {
    	shmdt(SHM_ADD);
    	SM_attached = FALSE;
    }
    SHM_ADD = 0;
    if ((shmadd = (char *) shmat(SHM_ID, SHM_ADD, SHM_RND)) == (char *) -1) {
    	COND_PushCondition(GQ_SHAREDMEMORYFAIL, "%s", "access_queue: shmat failed");
    	return (GQ_SHAREDMEMORYFAIL);
    }
    SHM_ADD = shmadd;
/*
 * Map structure for head/tail pointers into shared memory
 */
    QPTRS = (QUEUE_Pointers *) shmadd;
    shmadd += sizeof(QUEUE_Pointers);
/*
 * Map structure for queue into shared memory
 */
    GQ = (void *) shmadd;

    SM_attached = TRUE;
    return (GQ_NORMAL);
#else
    return COND_PushCondition(GQ_UNIMPLEMENTED, GQ_UNIMPLEMENTED_TXT, "access_queue");
#endif
}
