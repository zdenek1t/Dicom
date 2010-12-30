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
 **				DINPACS 1997
 **		     Electronic Radiology Laboratory
 **		   Mallinckrodt Institute of Radiology
 **		Washington University School of Medicine
 **
 ** Module Name(s):
 ** Author, Date:	Stephen Moore, March-1997
 ** Intent:		The archive_server application is an extension of
 **			the CTN image_server application.  This application
 **			adds support for the DICOM Storage Commitment Push Model
 **			SOP class.
 ** Last Update:		$Author: smm $, $Date: 2002/02/26 20:32:24 $
 ** Source File:		$RCSfile: archive_server.c,v $
 ** Revision:		$Revision: 1.15 $
 ** Status:		$State: Exp $
 */

static char rcsid[] = "$Revision: z1.0 $ $RCSfile: archive_server.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"
#include <stdlib.h>
#include <signal.h>

#if 0
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/param.h>
#endif
#ifdef SOLARIS
#include <netdb.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
/*lint -e46*/
#include <sys/wait.h>
/*lint +e46*/
#endif
#ifdef _MSC_VER
#include <windows.h>
#include <process.h>
#include <winsock.h>
#endif
#endif

#ifdef CTN_USE_THREADS
#ifdef _MSC_VER
#include <process.h>
#else
#include <pthread.h>
#endif
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"
#ifdef CTN_MULTIBYTE
#include "tblmb.h"
#include "idbmb.h"
#else
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/idb/idb.h"
#endif
#include "../dicom_lib/manage/manage.h"
#include "../dicom_lib/utility/utility.h"
#include "../dicom_lib/thread/ctnthread.h"
#include "../dicom_lib/gq/gq.h"

#include "../dicom_lib/apps_include/iqueues.h"
#include "image_archive.h"
#include "archive_queue.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

int         maxPDU = 16384;
CTNBOOLEAN  verboseDUL = FALSE;
CTNBOOLEAN  verboseTBL = FALSE;
CTNBOOLEAN  verboseSRV = FALSE;
CTNBOOLEAN  silent = FALSE;
CTNBOOLEAN  doVerification = FALSE;

char        *controlDatabase = "DCM_SRV";
static char *logFile = NULL;
static      CTNBOOLEAN forgiveFlag = FALSE;
int         queueNumber = -1;
char*       queueMapFile = 0;


#ifdef CTN_USE_THREADS
#ifndef _MSC_VER
typedef struct {
	void* reserved[2];
	pthread_t tid;
	int done;
}THREAD_HOLDER;

LST_HEAD* threadList = 0;
#endif
#endif

static void usageerror() {
	char
			msg[] =
					"\
Usage: [-e] [-f db] [-g generic] [-C config_file] [-i] [-l logfile] [-n node] [-o max] [-q] [-r] [-t] [-v] [-z queue] port\n\
\n\
    -b    \n\
    -e    Examine received images and do SOP validation\n\
    -f    Use db as control database instead of default (DCM_SRV)\n\
    -g    An override in the Security Matrix.  If an application is\n\
          configured to connect to <generic>, it has access to all \n\
          applications on this server.\n\
    -i    Ignore some problems in Association Request\n\
    -l    Place log of association requests in <logfile>\n\
    -n    Use <node> as name of server rather than hostname\n\
    -m    trip max\n\
    -o    Allow <max> simultaneous connections for an organization\n\
    -r    Reduced capability.  Turn on anything using FIS database\n\
    -s    Single user mode\n\
    -q    Quiet mode, don't dump a lot of messages to terminal\n\
    -t    Use threaded version.\n\
    -v    Place DUL and SRV facilities in verbose mode\n\
    -z    Turn on queuing of status information\n\
    -x    Verbose TBL/SRV\n\
    -C    Config file\n\
	\n\
    port  TCP/IP port address\n";

	fprintf(stderr, msg);
	exit(5);
}

static CONDITION
addChildProcess(PROCESS_ELEMENT * processElement, LST_HEAD ** list) {
	PROCESS_ELEMENT 	* e;

	if ((e = malloc(sizeof(*e))) == NULL) return 0;
	*e = *processElement;
	if (LST_Enqueue(list, e) != LST_NORMAL) return 0;

	return APP_NORMAL;
}

static CONDITION
harvestChildrenProcesses(LST_HEAD ** list) {
#ifndef _MSC_VER
	int               pid;
	PROCESS_ELEMENT   * e;
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0){
		e = LST_Head(list);
		if (e != NULL) (void) LST_Position(list, e);

		while (e != NULL){
			if (!silent) printf("Pid: %5d Called: %s\n", e->pid, e->calledAPTitle);
			if (e->pid == pid){
				  (void) LST_Remove(list, LST_K_BEFORE);
				  free(e);
				  e = NULL;
			}else{
				  e = LST_Next(list);
			}
		}
	}
#endif
	return 1;
}

static void
releaseProcessList(LST_HEAD ** list){
	PROCESS_ELEMENT  	*e;

	while ((e = LST_Dequeue(list)) != NULL){
		  free(e);
	}
	LST_Destroy(list);
}

static CONDITION
checkOrganizationConnections(char *organization,	LST_HEAD ** l, int connections) {
	PROCESS_ELEMENT	 	*e;

	e = LST_Head(l);
	(void) LST_Position(l, e);

	while (e != NULL && connections > 0) {
		if (strcmp(e->organization, organization) == 0)  connections--;
		e = LST_Next(l);
	}

	if (connections > 0){
		  return APP_NORMAL;
	}else{
		  return 0;
	}
}

void
logMessage(char *controlString, ...) {
	FILE     *fp;
	va_list  args;

	if (logFile == NULL){
		  fp = stdout;
	}else{
		  fp = fopen(logFile, "a");
	}
	if (fp == NULL) return;

	/*lint -e40 -e50 */
	va_start(args, controlString);
	if (controlString == NULL) controlString = "NULL Control string passedto PushCondition";

	(void) vfprintf(fp, controlString, args);
	(void) fflush(fp);
	va_end(args);
	/*lint +e40 +e50 */
	if (logFile != NULL) (void) fclose(fp);
}

typedef struct {
	DUL_NETWORKKEY 					**network;
	DUL_ASSOCIATIONKEY 				*association;
	DUL_ASSOCIATESERVICEPARAMETERS 	*service;
	CTNBOOLEAN 						reducedCapability;
} THREAD_STRUCT;

#ifdef _MSC_VER
 static void	 runThread(void *arg)
#else
 static void *   runThread(void *arg)
#endif
{
#ifdef CTN_USE_THREADS
	CONDITION      cond;
	THREAD_STRUCT  *s;

	s = (THREAD_STRUCT *) arg;
	cond = serviceRequests(s->network, &s->association, s->service, s->reducedCapability);
	if (cond == SRV_PEERREQUESTEDRELEASE) cond = SRV_NORMAL;
	if (cond != SRV_NORMAL) COND_DumpConditions();

	free(s);

 #ifdef _MSC_VER
	_endthread();
	return;
 #else
	{	THREAD_HOLDER* t;
		pthread_t   tid;

		tid = pthread_self();
		THR_ObtainMutex(FAC_UTL);
		t = LST_Head(&threadList);
		(void)LST_Position(&threadList, t);

		while (t != NULL) {
			if (pthread_equal(tid, t->tid)) {
				  t->done = 1;
				  break;
			}
			t = LST_Next(&threadList);
		}
		THR_ReleaseMutex(FAC_UTL);
	}
	/*pthread_exit(NULL);*/
	return 0;
 #endif
#else
 #ifdef _MSC_VER
	return;
 #else
	return 0;
 #endif
#endif
}

#ifdef CTN_USE_THREADS
#ifndef _MSC_VER
static void collectThreads(){
	THREAD_HOLDER*   t;

	t = LST_Head(&threadList);
	(void)LST_Position(&threadList, t);

	while (t != NULL) {
		if (t->done != 0){
			 THREAD_HOLDER* toDelete;
			 void* status = 0;
			 pthread_join(t->tid, &status);
			 toDelete = LST_Remove(&threadList, LST_K_BEFORE);
			 if (toDelete->done == 0) {
			 		 fprintf(stderr, "Collect threads logic error on\n");
					 exit(1);
				}
				free(toDelete);
		}
		t = LST_Next(&threadList);
	}
}
#endif
#endif

static void
startThread(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY * association, DUL_ASSOCIATESERVICEPARAMETERS * service, CTNBOOLEAN reducedCapability) {
#ifdef CTN_USE_THREADS
	CONDITION      cond;
	THREAD_STRUCT  *s;

	cond = DUL_AcknowledgeAssociationRQ(&association, service);
	if (cond != DUL_NORMAL) {
		  COND_DumpConditions();
		  exit(1);
	}
	s = malloc(sizeof(*s));
	if (s == NULL) {
		  fprintf(stderr, "Unable to malloc structure to start a thread\n");
		  exit(1);
	}
	s->network = network;
	s->association = association;
	s->service = service;
	s->reducedCapability = reducedCapability;

#ifdef _MSC_VER
	{
		unsigned long   threadID;
		threadID = _beginthread(runThread, 0, s);
		/* CloseHandle(threadID); */
	}
#else
	{
		pthread_t   threadID;
		static      pthread_attr_t attr;
		static int  firstTrip = 1;

		if (firstTrip) {
			  pthread_attr_init(&attr);
			  /*pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);*/
			  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
			  firstTrip = 0;
		}
		THR_ObtainMutex(FAC_UTL);
		pthread_create(&threadID, &attr, runThread, s);
#ifndef _MSC_VER
		{	THREAD_HOLDER* t;
			t = malloc(sizeof(*t));
			t->tid = threadID;
			t->done = 0;
			(void)LST_Enqueue(&threadList, t);
		}
		collectThreads();
#endif
		THR_ReleaseMutex(FAC_UTL);
	}
#endif
#endif
}

void child_died(int sig){
    printf("Child died\n");
};

void handle_kill(int sig){
    printf("Kill Fork\n");
    TBL_CloseDB();
    exit(sig);
};

int
main(int argc, char **argv) {
	CONDITION            			 cond;
	DUL_NETWORKKEY       			 * network;
	DUL_ASSOCIATIONKEY   			 * association = NULL;
	DUL_ASSOCIATESERVICEPARAMETERS   *service;
	int                  			 pid, port = 4006;
#ifdef _MSC_VER
	char                 			 node[512 + 1] = "";
#else
	char                 			 node[MAXHOSTNAMELEN + 1] = "";
#endif
	CTNBOOLEAN    				     singleUserMode = FALSE;
	CTNBOOLEAN           			 useThreads = FALSE;
	LST_HEAD             			 * processList = NULL;
	char                 			 currentDate[DICOM_DA_LENGTH + 1];
	char                 			 currentTime[DICOM_TM_LENGTH + 1];
	char                 			 *logString = "%-10s (%s %s %s) (%s %s) %s %s %d\n";
	int                  			 maxPerOrganization = 1000;
	PROCESS_ELEMENT      			 processElement;
	CTNBOOLEAN           			 doBLG = FALSE;
	CTNBOOLEAN           			 reducedCapability = TRUE;
	char                 			 *genericAE = "";
	int                  			 tripCount = 0;
	int                  			 tripMax = -1;
	char*                			 configFile = 0;
	char 							 uidError[] = "To use this port (%d), you must run as root or the application\n must be setuid root (see chmod)\n";

	(void) gethostname(node, sizeof(node) - 1);

	while (--argc > 0 && *(++argv)[0] == '-') {
		switch ((*argv)[1]) {
		 case 'b':
			         doBLG = TRUE;
			         break;
		 case 'C':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         configFile = *argv;
			         break;
		 case 'e':
			         doVerification = TRUE;
			         break;
		 case 'f':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         controlDatabase = *argv;
			         break;
		 case 'g':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         genericAE = *argv;
			         break;
		 case 'l':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         logFile = *argv;
			         break;
		 case 'i':
			         forgiveFlag = TRUE;
			         break;
		 case 'm':
			         if (argc < 1)  usageerror();
			         argc--;
			         argv++;
			         if (sscanf(*argv, "%d", &tripMax) != 1) usageerror();
			         break;
		 case 'n':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         strcpy(node, *argv);
			         break;
		 case 'o':
			         if (argc < 1) usageerror();
			         argc--;
			         argv++;
			         if (sscanf(*argv, "%d", &maxPerOrganization) != 1) usageerror();
			         break;
		 case 'r':
			         reducedCapability = FALSE;
			         break;
		 case 'q':
			         silent = TRUE;
			         break;
		 case 's':
			         singleUserMode = TRUE;
			         break;
		 case 't':
#ifndef CTN_USE_THREADS
			         fprintf(stderr, "This version was not compiled for threads\n");
			         return 1;
#else
			         useThreads = TRUE;
#endif
			         break;
		 case 'v':
			         verboseDUL = TRUE;
			         break;
		 case 'x':
			         if (--argc < 1) usageerror();
			         argv++;
			         if (strcmp(*argv, "TBL") == 0){
			        	 verboseTBL = TRUE;
			         }else if (strcmp(*argv, "SRV") == 0){
			        	 verboseSRV = TRUE;
			         }else{
			        	 usageerror();
			         }
			         break;
		 case 'z':
			         if (argc < 2) usageerror();
			         argc--;
			         argv++;
			         if (sscanf(*argv, "%d", &queueNumber) != 1) usageerror();
			         argc--;
			         argv++;
			         queueMapFile = *argv;
			         break;
		 default:
			         printf("Unrecognized option: %s\n", *argv);
			         break;
		}
	}

	if (configFile != 0){
		UTL_SetConfigFile(configFile);
		printf("Config file: %s \n",configFile);
		if (UTL_GetConfigParameter("DB_NAME") != NULL)		controlDatabase = UTL_GetConfigParameter("DB_NAME");
		if (UTL_GetConfigParameter("PORT") != NULL)   		port = atoi(UTL_GetConfigParameter("PORT"));
		if (UTL_GetConfigParameter("NODE") != NULL) 		strcpy(node, UTL_GetConfigParameter("NODE"));
		if (UTL_GetConfigParameter("LOG_FILE") != NULL) 	logFile = UTL_GetConfigParameter("LOG_FILE");

  		if (UTL_GetConfigParameter("IGNORE") != NULL)  		forgiveFlag = TRUE;

		if (UTL_GetConfigParameter("SILENT") != NULL ){
			silent = TRUE;
			verboseDUL = FALSE;
		}else{
			if (UTL_GetConfigParameter("VERBOSE") != NULL) 	verboseDUL = TRUE;
			printf("Control database: %s \n",controlDatabase);
			printf("Port: %i \n",port);
			printf("Node: %s \n",node);
			printf("Log file: %s \n", logFile);
			printf("Ignore: %i \n",(int)forgiveFlag);
			printf("Silent: %i \n", (int)silent);
			printf("Verbose: %i \n", (int)verboseDUL);
		}
	}else{
		if (argc < 1) usageerror();
		if (sscanf(*argv++, "%d", &port) != 1) usageerror();
	}

#ifdef CTN_USE_THREADS
	THR_Init();
 #ifndef _MSC_VER
	if (useThreads) threadList = LST_Create();
 #endif
#endif

#ifndef _MSC_VER
	(void) signal(SIGCHLD, child_died);
	(void) signal(SIGKILL, handle_kill);
	(void) signal(SIGILL, handle_kill);
	(void) signal(SIGQUIT, handle_kill);
	(void) signal(SIGINT, handle_kill);
	(void) signal(SIGUSR1, SIG_IGN);

	if (port < 1024) {
		if (geteuid() != 0) {
			fprintf(stderr, uidError, port);
			exit(1);
		}
	}
#endif
/* DEBUG */
	(void) TBL_Debug(verboseTBL);
	DUL_Debug(verboseDUL);
	DUL_Blog(doBLG);
	SRV_Debug(verboseSRV);
//	DCM_Debug(TRUE);



	cond = TBL_ConnectDB();
	if (cond != TBL_NORMAL){
		fprintf(stderr, "Could not connect to database - Exiting \n");
		exit(2);
	}

	cond = DUL_InitializeNetwork(DUL_NETWORK_TCP, DUL_AEBOTH, (void *) &port,  DUL_TIMEOUT, DUL_ORDERBIGENDIAN | DUL_FULLDOMAINNAME, &network);
	if (cond != DUL_NORMAL) {
		fprintf(stderr, "Could not initialize network - Exiting \n");
		COND_DumpConditions();
		exit(2);
	}
#ifndef _MSC_VER
	setuid(getuid()); /* Return to proper uid */
#endif

	if ((processList = LST_Create()) == NULL) {
		  fprintf(stderr, "Unable to create list for process elements\n");
		  exit(3);
	}

	queueInitialize();

	while (!CTN_ERROR(cond)) {
		service = malloc(sizeof(*service));
		if (service == NULL) {
			  fprintf(stderr, "Could not allocate DICOM services structure\n");
			  return 1;
		}
		cond = nextAssociationRequest(node, &network, service, maxPDU, forgiveFlag, genericAE, &association, &processElement, useThreads);
		if (CTN_FATAL(cond)) {
			  fprintf(stderr, "Fatal error during startup\n");
			  COND_DumpConditions();
			  break;
		}
		UTL_GetDicomDate(currentDate);
		UTL_GetDicomTime(currentTime);

		if (!singleUserMode && !useThreads) (void) harvestChildrenProcesses(&processList);

		if (cond == APP_NORMAL && !useThreads) {
			  DUL_ABORTITEMS 	abortItems;
			  cond = checkOrganizationConnections(processElement.organization, &processList, maxPerOrganization);
			  if (cond != APP_NORMAL) {
				    abortItems.result = DUL_REJECT_TRANSIENT;
				    abortItems.source = DUL_ULSU_REJECT;
				    abortItems.reason = DUL_ULSU_REJ_NOREASON;
				    (void) DUL_RejectAssociationRQ(&association, &abortItems);
			  }
		}
		if (cond == APP_NORMAL) {
			pid = 0;
#ifdef _MSC_VER
			if (useThreads) startThread(&network, association, service, reducedCapability);

#else
			if (useThreads){
				startThread(&network, association, service, reducedCapability);
			}else if (!singleUserMode){
				pid = fork();
			}
#endif

			if (pid == 0 && !useThreads) {
				if (!silent) printf(">> ====== Forked child ====== <<\n");

				cond = DUL_AcknowledgeAssociationRQ(&association, service);
				if (cond != DUL_NORMAL) continue;

				if(!singleUserMode) TBL_ConnectDB();

				cond = serviceRequests(&network, &association, service,	reducedCapability);
				if (cond == SRV_PEERREQUESTEDRELEASE) cond = SRV_NORMAL;

				if (!singleUserMode) break;

			}else{
				logMessage(logString, "ACCEPTED", service->callingAPTitle, service->callingPresentationAddress,
					       processElement.organization, service->calledAPTitle, service->calledPresentationAddress, currentDate, currentTime, pid);

				if (!useThreads) {
					DUL_ClearServiceParameters(service);
					free(service);
					(void) DUL_DropAssociation(&association);
					processElement.pid = pid;
					(void) addChildProcess(&processElement, &processList);
				}
				if (!silent) printf(">> ====== Parent (%i) ====== <<\n", pid);
			}
		}else{
			logMessage(logString, "REJECTED", service->callingAPTitle, service->callingPresentationAddress,
				       processElement.organization, service->calledAPTitle, service->calledPresentationAddress, currentDate, currentTime, 0);

			DUL_ABORTITEMS 		abortItems;
			abortItems.result = DUL_REJECT_TRANSIENT;
			abortItems.source = DUL_ULSU_REJECT;
			abortItems.reason = DUL_ULSU_REJ_UNREC_CALLING_TITLE;
			(void) DUL_RejectAssociationRQ(&association, &abortItems);
			COND_DumpConditions();
		}
		tripCount++;
		if ((tripMax > 0) && (tripCount >= tripMax)) break;
	}

	releaseProcessList(&processList);
	DUL_DropNetwork(&network);
	COND_DumpConditions();
	TBL_CloseDB();

	if (!silent) printf("Exiting\n");

	#ifdef MALLOC_DEBUG
		malloc_verify(0);
		malloc_shutdown();
	#endif
	return 0;
}
