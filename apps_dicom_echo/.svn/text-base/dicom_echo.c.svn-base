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
** Module Name(s):	main
**			usageerror
**			myExit
**			echoCallback
** Author, Date:	Stephen M. Moore, 1-May-93
** Intent:		This program is an example and test program which
**			demonstrates the DICOM packages developed at MIR.
**			It establishes an Association with a server and uses
**			the verification service class (ECHO command).
**  Usage:
**	dicom_echo [-a title] [-d] [-c title] [-m mode] [-n number] [-p] [-r repeat] [-v] [-x] node port
**  Options:
**	a	Application title of this application
**	c	Called AP title to use during Association setup
**	d	Drop Association after echo requests
**	m	Mode for SCU/SCP negotiation (SCU, SCP, SCUSCP)
**	n	Number of network connections
**	p	Dump service parameters after Association Request
**	r	Number of times to repeat echo request
**	v	Verbose mode for DUL facility
**	x	Don't release Associations
** Last Update:		$Author: smm $, $Date: 2001/12/26 16:17:18 $
** Source File:		$RCSfile: dicom_echo.c,v $
** Revision:		$Revision: 1.26 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.26 $ $RCSfile: dicom_echo.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#include <winsock.h>
#else
#include <sys/file.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/thread/ctnthread.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"

#define PRESENTATION_CONTEXT_ID	5

static void
usageerror();
static void
myExit(DUL_ASSOCIATIONKEY ** association);
static CONDITION
echoCallback(MSG_C_ECHO_REQ * request, MSG_C_ECHO_RESP * response, void *string);

int
main(int argc, char **argv)
{

    CONDITION						cond;					/* Return values from DUL and ACR routines */
    DUL_NETWORKKEY					* network = NULL;		/* Used to initialize our network */
    DUL_ASSOCIATIONKEY				* association = NULL;	/* Describes the Association with the Acceptor */
    DUL_ASSOCIATESERVICEPARAMETERS							/* The items which describe this Association */
									params = {DICOM_STDAPPLICATIONCONTEXT, "DICOM_TEST", "DICOM_VERIFY", "", 16384, 0, 0, 0, "calling presentation addr",
											 "called presentation addr", NULL, NULL, 0, 0, MIR_IMPLEMENTATIONCLASSUID, MIR_IMPLEMENTATIONVERSIONNAME, "", "" };
    char       						*calledAPTitle = "DICOM_STORAGE", *callingAPTitle = "DICOM_ECHO";
    char					        localHost[40];
    int						        port;					/* TCP port used to establish Association */
    char					        *node;					/* The node we are calling */

    CTNBOOLEAN						verbose = FALSE, abortFlag = FALSE, paramsFlag = FALSE, drop = FALSE, noRelease = FALSE;
    int						        repeat = 1, connections = 1, repeatCopy, sleepTime = 0;
    DUL_SC_ROLE						role = DUL_SC_ROLE_DEFAULT;
    MSG_C_ECHO_RESP					response;

    while (--argc > 0 && (*++argv)[0] == '-') {
    	switch (*(argv[0] + 1)) {
			case 'a':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						callingAPTitle = *argv;
						break;
			case 'c':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						calledAPTitle = *argv;
						break;
			case 'd':
						drop = TRUE;
						break;
			case 'm':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						if (strcmp(*argv, "SCU") == 0){
							role = DUL_SC_ROLE_SCU;
						}else if (strcmp(*argv, "SCP") == 0){
							role = DUL_SC_ROLE_SCP;
						}else if (strcmp(*argv, "SCUSCP") == 0){
							role = DUL_SC_ROLE_SCUSCP;
						}else{
							usageerror();
						}
						break;
			case 'n':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						if (sscanf(*argv, "%d", &connections) != 1) usageerror();
						break;
			case 'p':
						paramsFlag = TRUE;
						break;
			case 'r':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						if (sscanf(*argv, "%d", &repeat) != 1) usageerror();
						if (repeat <= 0) repeat = 32 * 1024 * 1024;	/* A special case */
						break;
			case 's':
						argc--;
						argv++;
						if (argc <= 0) usageerror();
						if (sscanf(*argv, "%d", &sleepTime) != 1) usageerror();
						break;
			case 'v':
						verbose = TRUE;
						break;
			case 'x':
						noRelease = TRUE;
						break;
			default:
						break;
    	}
    }
    if (argc < 2) usageerror();

    THR_Init();
    DUL_Debug(verbose);
    SRV_Debug(verbose);

    node = *argv;
    if (sscanf(*++argv, "%d", &port) != 1) usageerror();
    (void) gethostname(localHost, sizeof(localHost));

    cond = DUL_InitializeNetwork(DUL_NETWORK_TCP, DUL_AEREQUESTOR, NULL, 10, DUL_ORDERBIGENDIAN, &network);
    if (cond != DUL_NORMAL)	myExit(&association);

    while (connections-- > 0) {
    	sprintf(params.calledPresentationAddress, "%s:%s", node, *argv);
    	strcpy(params.callingPresentationAddress, localHost);
    	strcpy(params.calledAPTitle, calledAPTitle);
    	strcpy(params.callingAPTitle, callingAPTitle);

    	cond = SRV_RequestServiceClass(DICOM_SOPCLASSVERIFICATION, role, &params);
    	if (cond != SRV_NORMAL) {
    		COND_DumpConditions();
    		THR_Shutdown();
    		exit(1);
    	}

    	cond = DUL_RequestAssociation(&network, &params, &association);
    	if (cond != DUL_NORMAL) {
    		if (cond == DUL_ASSOCIATIONREJECTED) {
    			fprintf(stderr, "Association Rejected\n");
    			fprintf(stderr, " Result: %2x Source %2x Reason %2x\n",	params.result, params.resultSource, params.diagnostic);
    		}
    		myExit(&association);
    	}

    	if (verbose || paramsFlag) {
    		(void) printf("Association accepted, parameters:\n");
    		DUL_DumpParams(&params);
    	}
    	repeatCopy = repeat;

    	while (repeatCopy-- > 0) {
    		MSG_C_ECHO_REQ echoRequest = {MSG_K_C_ECHO_REQ, 0, 0, DCM_CMDDATANULL, DICOM_SOPCLASSVERIFICATION};
    		echoRequest.messageID = SRV_MessageIDOut();

    		cond = SRV_CEchoRequest(&association, &params, &echoRequest, &response, echoCallback, "Context", "");
    		if (!(CTN_SUCCESS(cond))) {
    			(void) printf("Verification Request unsuccessful\n");
    			COND_DumpConditions();
    		}else{
    			MSG_DumpMessage(&response, stdout);
    		}

    		SRV_MessageIDIn(echoRequest.messageID);
#ifdef _MSC_VER
    		if (sleepTime > 0) Sleep(sleepTime * 1000);
#else
    		if (sleepTime > 0) sleep(sleepTime);
#endif
    	}

    	if (abortFlag){
    		cond = DUL_AbortAssociation(&association);
    	}else if (drop){
    		cond = DUL_DropAssociation(&association);
    	}else if (noRelease == FALSE) {
    		cond = DUL_ReleaseAssociation(&association);
    		if (cond != DUL_RELEASECONFIRMED) {
    			(void) COND_PopCondition(TRUE);
#if 0
    			(void) fprintf(stderr, "%x\n", cond);
    			COND_DumpConditions();
#endif
    		}
    	}
    	(void) DUL_ClearServiceParameters(&params);
    }
    (void) DUL_DropNetwork(&network);
    THR_Shutdown();
    return 0;
}


/* usageerror
**
** Purpose:
**	Print the usage message for this program and exit.
**
** Parameter Dictionary:
**	None
**
** Return Values:
**	None
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static void
usageerror()
{
    char msg[] = "\
dicom_echo [-a title] [-d] [-c title] [-m mode] [-n num] [-p] [-r repeat] [-s sleeptime] [-v] [-x] node port\n\
\n\
    a     Application title of this application\n\
    c     Called AP title to use during Association setup\n\
    d     Drop Association after echo requests\n\
    m     Mode for SCU/SCP negotiation (SCU, SCP, SCUSCP)\n\
    n     Number of network connections\n\
    p     Dump service parameters after Association Request\n\
    r     Number of times to repeat echo request\n\
    s     Time to sleep after each echo request\n\
    v     Verbose mode for DUL/SRV facilities\n\
    x     Do not release Associations when finished with echo\n\
\n\
    node  Node name of server\n\
    port  Port number of server\n";

    fprintf(stderr, msg);
    exit(1);
}


/* myExit
**
** Purpose:
**	Exit routines which closes network connections, dumps error
**	messages and exits.
**
** Parameter Dictionary:
**	association	A handle for an association which is possibly open.
**
** Return Values:
**	None
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static void
myExit(DUL_ASSOCIATIONKEY ** association)
{
    fprintf(stderr, "Abnormal exit\n");
    COND_DumpConditions();

    if (association != NULL){
    	if (*association != NULL) (void) DUL_DropAssociation(association);
    }
    THR_Shutdown();
    exit(1);
}

/* echoCallback
**
** Purpose:
**	Call back routine for the C-ECHO primitive
**
** Parameter Dictionary:
**	request		Pointer to request message
**	response	Pointer to response message
**	string		Context information
**
** Return Values:
**	SRV_NORMAL
**
** Notes:
**	The context value is an asciiz string that we passed from the
**	main loop.  In order to satisfy function prototypes, we have to
**	declare it as void* and cast it to get the char*.
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
echoCallback(MSG_C_ECHO_REQ * request, MSG_C_ECHO_RESP * response, void *string) /* MSG_C_ECHO_RESP * response, char *string) */

{
    printf("Echo context: %s\n", (char *) string);
    (void) printf("Verification Response\n");
    (void) printf("  Message ID Responded to: %4d\n", response->messageIDRespondedTo);
    (void) printf("  Verification Status:     %04x\n", response->status);
    return SRV_NORMAL;
}
