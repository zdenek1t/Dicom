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
** Author, Date:	Stephen Moore, 28-Oct-99
** Intent:
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:37:41 $
** Source File:		$RCSfile: queue.c,v $
** Revision:		$Revision: 1.3 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.3 $ $RCSfile: queue.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef _MSC_VER
#include <windows.h>
#endif
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/manage/manage.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/gq/gq.h"

#if 0
#include "image_archive.h"
#endif

#include "../dicom_lib/apps_include/iqueues.h"
#include "archive_queue.h"

extern char*		queueMapFile;
extern int 			queueNumber;

static LST_HEAD* 	queueList = 0;

static int findQueue(const char* title)
{
  QUEUE_ELEMENT*    	p;

  if (queueList == 0) return -1;

  p = LST_Head(&queueList);
  (void)LST_Position(&queueList, p);

  while (p != 0){
	  if (strcmp(p->title, title) == 0) return p->qid;
	  p = LST_Next(&queueList);
  }
  return -1;
}

void queueInitialize()
{
  FILE*       f;
  char        buf[1024];

  if (queueMapFile == 0) return;

  if (queueNumber >= 0) {
	  if (GQ_GetQueue(queueNumber, sizeof(CTNNETWORK_Queue)) != GQ_NORMAL) {
		  (void) COND_PopCondition(TRUE);
		  if (GQ_InitQueue(queueNumber, 100, sizeof(CTNNETWORK_Queue)) != GQ_NORMAL) {
			  fprintf(stderr, "Unable to initialize network queue\n");
			  COND_DumpConditions();
			  exit(1);
          }
      }
  }

  queueList = LST_Create();
  f = fopen(queueMapFile, "r");
  if (f == 0) return;

  while(fgets(buf, sizeof(buf), f) != 0) {
	  QUEUE_ELEMENT q, *p;
	  if (buf[0] == '#') continue;
	  if (buf[0] == '\r') continue;
	  if (buf[0] == '\n') continue;

	  if (sscanf(buf, "%s %d", q.title, &q.qid) != 2) continue;

	  if (GQ_GetQueue(q.qid, sizeof(CTNDISP_Queue)) != GQ_NORMAL) {
		  (void) COND_PopCondition(TRUE);
		  if (GQ_InitQueue(q.qid, 100, sizeof(CTNDISP_Queue)) != GQ_NORMAL) {
			  fprintf(stderr, "Unable to initialize display queue\n");
	          COND_DumpConditions();
	          exit(1);
		  }
	  }

	  p = malloc(sizeof(*p));
	  *p = q;
	  LST_Enqueue(&queueList, p);
  }
  fclose(f);
}


void
queueNewAssociation(const char* node)
{
  CONDITION       cond;
  CTNDISP_Queue   entry;
  int             imageQueue;

  imageQueue = findQueue(node);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
	  COND_DumpConditions();
      return;
  }

  entry.connection = CONN_INOPEN;
  strcpy(entry.dpnid,node);
  entry.inumber = 0;
  
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

void
queueClosedAssociation(const char* node, int count)
{
  CONDITION         cond;
  CTNDISP_Queue     entry;
  CTNNETWORK_Queue  network;
  int               imageQueue;

  imageQueue = findQueue(node);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
	  COND_DumpConditions();
      return;
  }

  entry.connection = CONN_INCLOSE;
  strcpy(entry.dpnid,node);
  
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();

  strcpy(network.vendorid, "vendor");
  strcpy(network.dpnid, node);
  network.association_id = getpid()*2;
  network.connection = CONN_INCLOSE;
  network.percentage = count;

  cond = GQ_Enqueue(queueNumber, &network);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

void
queueNewImage(const char* node, int count)
{
  CONDITION         cond;
  CTNDISP_Queue     entry;
  int               imageQueue;

  imageQueue = findQueue(node);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  entry.connection = CONN_INXFER;
  strcpy(entry.dpnid,node);
  entry.inumber = count;
  
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

int
queueDisplayImage(const char* node, const char* name)
{
  CONDITION       cond;
  CTNDISP_Queue   entry;
  int             imageQueue;

  imageQueue = findQueue(node);


  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  entry.connection = CONN_INDISPLAY;
  strcpy(entry.dpnid,node);
  entry.inumber = 0;
  strcpy(entry.imagefile, name);

  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}


void
networkqueuePartialImage(const char* node, int percentage)
{
  CONDITION         cond;
  CTNNETWORK_Queue  entry;
  int               imageQueue;

  imageQueue = findQueue(node);


  if (imageQueue < 0) return;

  cond = GQ_GetQueue(queueNumber, sizeof(CTNNETWORK_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  strcpy(entry.vendorid, "vendor");
  strcpy(entry.dpnid, node);
  entry.association_id = getpid()*2;
  entry.connection = CONN_INXFER;
  entry.percentage = percentage;

  cond = GQ_Enqueue(queueNumber, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

void
queueNetworkNewAssociation(const char* node)
{
  CONDITION         cond;
  CTNNETWORK_Queue  entry;
  int               imageQueue;

  imageQueue = findQueue(node);

  if (imageQueue < 0) return;


  cond = GQ_GetQueue(queueNumber, sizeof(CTNNETWORK_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  strcpy(entry.vendorid, "vendor");
  strcpy(entry.dpnid, node);
  entry.association_id = getpid()*2;
  entry.connection = CONN_INOPEN;
  entry.percentage = 50;

  cond = GQ_Enqueue(queueNumber, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();

}

/* These are for outgoing transmissions */

void
queueTransmitAssociation(const char* caller,
			 const char* destination)
{
  CONDITION         cond;
  CTNDISP_Queue     entry;
  CTNNETWORK_Queue  ctnnet;
  int               imageQueue;

  imageQueue = findQueue(destination);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  entry.connection = CONN_OUTOPEN;
  strcpy(entry.dpnid,destination);
  entry.inumber = 0;
  
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();


  /* Now do the part for the network queue */

  cond = GQ_GetQueue(queueNumber, sizeof(CTNNETWORK_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  strcpy(ctnnet.vendorid, "vendor");
  strcpy(ctnnet.dpnid, destination);
  ctnnet.association_id = getpid()*2 + 1;
  ctnnet.connection = CONN_OUTOPEN;
  ctnnet.percentage = 50;

  cond = GQ_Enqueue(queueNumber, &ctnnet);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

void
queueStartTransmit(const char* caller, const char* destination, int count)
{
  CONDITION       cond;
  CTNDISP_Queue   entry;
  int             imageQueue;

  imageQueue = findQueue(destination);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  entry.connection = CONN_OUTXFER;
  strcpy(entry.dpnid,destination);
  entry.inumber = count;
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}

void
queueTransmitImage(const char* caller, const char* destination, int percentage)
{
  CONDITION         cond;
  CTNNETWORK_Queue  entry;
  int               imageQueue;

  imageQueue = findQueue(destination);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(queueNumber, sizeof(CTNNETWORK_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  strcpy(entry.vendorid, "vendor");
  strcpy(entry.dpnid, destination);
  entry.association_id = getpid()*2 + 1;
  entry.connection = CONN_OUTXFER;
  entry.percentage = percentage;

  cond = GQ_Enqueue(queueNumber, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}


void
queueClosedTransmitAssociation(const char* caller, const char* destination, int count)
{
  CONDITION         cond;
  CTNDISP_Queue     entry;
  CTNNETWORK_Queue  network;
  int               imageQueue;

  imageQueue = findQueue(destination);

  if (imageQueue < 0) return;

  cond = GQ_GetQueue(imageQueue, sizeof(CTNDISP_Queue));
  if (cond != GQ_NORMAL) {
      COND_DumpConditions();
      return;
  }

  entry.connection = CONN_OUTCLOSE;
  strcpy(entry.dpnid,destination);
  
  cond = GQ_Enqueue(imageQueue, &entry);
  if (cond != GQ_NORMAL) COND_DumpConditions();

  strcpy(network.vendorid, "vendor");
  strcpy(network.dpnid, destination);
  network.association_id = getpid()*2+1;
  network.connection = CONN_OUTCLOSE;
  network.percentage = count;

  cond = GQ_Enqueue(queueNumber, &network);
  if (cond != GQ_NORMAL) COND_DumpConditions();
}
