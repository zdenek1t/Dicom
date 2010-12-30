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
** @$ = @$ = @$ =
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	main
**			usageerror
**			myExit
**			sendImage
**			sendImageSet
**			sendCallback
**			requestAssociation
** Author, Date:	Stephen M. Moore, 6-May-93
** Intent:		This program is an example and test program which
**			demonstrates the DICOM packages developed at MIR.
**			It establishes an Association with a server and sends
**			one or more images to the server.
**
** Last Update:		$Author: smm $, $Date: 2005-02-12 04:42:43 $
** Source File:		$RCSfile: send_image.c,v $
** Revision:		$Revision: 1.46 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.46 $ $RCSfile: send_image.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
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
#include "../dicom_lib/utility/utility.h"

static void usageerror();
static void myExit(DUL_ASSOCIATIONKEY ** association, CONDITION cond);
static void
sendImageSet(int argc, char **argv, DUL_NETWORKKEY ** network, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPName, int limit, CTNBOOLEAN trimPixels, CTNBOOLEAN expandPixels,
			 CTNBOOLEAN eFilmFlag, CTNBOOLEAN timeTransfer, CTNBOOLEAN allowRepeatedElements, CTNBOOLEAN allowVRMismatch, const char* sopInstanceUID, char** xferSyntaxes, int xferSyntaxCount);
static CONDITION
sendImage(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DCM_OBJECT ** object, char *SOPClass, char *instanceUID, char *moveAETitle);
static int
requestAssociation(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, char** xferSyntaxes, int xferSyntaxCount);
static CONDITION
sendCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response, unsigned long transmitted, unsigned long total, void *string);

static CTNBOOLEAN responseSensitive = FALSE;
static CTNBOOLEAN silent = FALSE;


static void
fillFileList(const char* f, LST_HEAD** lst)
{
  if (UTL_IsDirectory(f)) {
    LST_HEAD* 		l = 0;
    UTL_FILEITEM* 	item;

    UTL_ScanDirectory(f, &l);
    item = (UTL_FILEITEM*)LST_Dequeue(&l);

    while(item != NULL) {
    	char p2[1024];
    	if ((strcmp(item->path, ".") != 0) && (strcmp(item->path, "..") != 0)) {
    		strcpy(p2, f);
    		strcat(p2, "/");
    		strcat(p2, item->path);
    		fillFileList(p2, lst);
    	}
    	free(item);
    	item = (UTL_FILEITEM*)LST_Dequeue(&l);
    }
  }else{
	  UTL_FILEITEM * p;
	  p = malloc(sizeof(*p));
	  strcpy(p->path, f);
	  LST_Enqueue(lst, p);
  }
}

main(int argc, char **argv)
{
    CONDITION 		cond;			/* Return values from DUL and ACR routines */
    DUL_NETWORKKEY 	* network = NULL;		/* Used to initialize our network */

    DUL_ASSOCIATESERVICEPARAMETERS	/* The items which describe this Association */
		params = { DICOM_STDAPPLICATIONCONTEXT, "DICOM_TEST", "DICOM_STORAGE", "", 16384, 0, 0, 0, "calling addr", "called addr", NULL, NULL, 0, 0, MIR_IMPLEMENTATIONCLASSUID, MIR_IMPLEMENTATIONVERSIONNAME, "", ""};
    char
       *calledAPTitle = "DICOM_STORAGE",
       *callingAPTitle = "DICOM_TEST",
        localHost[40],
       *node,			/* The node we are calling */
       *port,			/* ASCIIZ representation of TCP port */
       *SOPName = NULL;
    int
        scratch;		/* Used to check syntax of port number */
    unsigned long   maxPDU = 16384;

    CTNBOOLEAN
		verboseDCM 				= FALSE,
		verboseDUL 				= FALSE,
		verboseSRV 				= FALSE,
		trimPixelData 			= FALSE,
		expandPixelData 		= FALSE,
		timeTransfer 			= FALSE,
		eFilmFlag 				= FALSE,
		allowRepeatedElements 	= FALSE,
		allowVRMismatch 		= FALSE;

    int 	limit = 1000 * 1000;	/* Limit on number of images to send */
    char* 	sopInstanceUID = 0;
    char** 	xferSyntaxes;
    int 	xferSyntaxCount = 0;
    char 	xferSyntaxBuf[1024] = "";

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
    		case 'e':
    					expandPixelData = TRUE;
    					break;
    		case 'E':
    					eFilmFlag = TRUE;
    					break;
    		case 'l':
    					argc--;
    					argv++;
    					if (argc <= 0) usageerror();
    					if (sscanf(*argv, "%d", &limit) != 1) usageerror();
    					break;
    		case 'm':
    					argc--;
    					argv++;
    					if (argc <= 0) usageerror();
    					if (sscanf(*argv, "%lu", &maxPDU) != 1) usageerror();
    					break;
    		case 'p':
    					trimPixelData = TRUE;
    					break;
    		case 'q':
    					silent = TRUE;
    					break;
    		case 'r':
    					responseSensitive = TRUE;
    					break;
    		case 's':
    					argc--;
    					argv++;
    					if (argc <= 0) usageerror();
    					SOPName = *argv;
    					break;
    		case 't':
    					timeTransfer = TRUE;
    					break;
    		case 'u':
    					argc--;
    					argv++;
    					if (argc <= 0) usageerror();
    					sopInstanceUID = *argv;
    					break;
    		case 'v':
    					verboseDUL = TRUE;
    					verboseSRV = TRUE;
    					break;
    		case 'w':
    					argc--; argv++;
    					if (argc < 1) usageerror();
    					if (strcmp(*argv, "REPEAT") == 0) allowRepeatedElements = TRUE;
    					break;
    		case 'x':
    					argc--;
    					argv++;
    					if (argc <= 0) usageerror();
    					if (strcmp(*argv, "DCM") == 0)
    						verboseDCM = TRUE;
    					else if (strcmp(*argv, "DUL") == 0)
    						verboseDUL = TRUE;
    					else if (strcmp(*argv, "SRV") == 0)
    						verboseSRV = TRUE;
    					else
    						usageerror();
    					break;
    		case 'X':
    					argc--; argv++;
    					if (argc <= 0) usageerror();
    					if (xferSyntaxCount++ != 0) strcat(xferSyntaxBuf, ";");
    					strcat(xferSyntaxBuf, UID_Translate(*argv));
    					break;
    		case 'Z':
    					allowVRMismatch = TRUE;
    					break;
    		default:
    					break;
    		}
    	}
    if (argc < 3) usageerror();

    node = *argv++;
    argc--;
    port = *argv++;
    argc--;
    if (sscanf(port, "%d", &scratch) != 1) usageerror();

    THR_Init();
    DCM_Debug(verboseDCM);
    DUL_Debug(verboseDUL);
    SRV_Debug(verboseSRV);
    UTL_SetConfigFile(0);

    cond = DUL_InitializeNetwork(DUL_NETWORK_TCP, DUL_AEREQUESTOR, NULL, DUL_TIMEOUT, DUL_ORDERBIGENDIAN, &network);
    if (cond != DUL_NORMAL)	myExit(NULL, 0);

    (void) gethostname(localHost, sizeof(localHost));
    sprintf(params.calledPresentationAddress, "%s:%s", node, port);
    strcpy(params.callingPresentationAddress, localHost);
    strcpy(params.calledAPTitle, calledAPTitle);
    strcpy(params.callingAPTitle, callingAPTitle);
    params.maxPDU = maxPDU;

    if (xferSyntaxCount != 0) xferSyntaxes = UTL_ExpandToPointerArray(xferSyntaxBuf, ";", &xferSyntaxCount);

    sendImageSet(argc, argv, &network, &params, SOPName, limit, trimPixelData, expandPixelData, eFilmFlag, timeTransfer, allowRepeatedElements, allowVRMismatch, sopInstanceUID, xferSyntaxes, xferSyntaxCount);
    THR_Shutdown();
    return 0;
}

static void replaceSOPInstanceUID(DCM_OBJECT** iod, const char* sopInstanceUID)
{
  char 			uid[DICOM_UI_LENGTH+1];
  DCM_ELEMENT 	e = { DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, 0, 0 };

  strcpy(uid, sopInstanceUID);
  e.d.string = uid;

  DCM_ModifyElements(iod, &e, 1, 0, 0, 0);
}

static void
replacePixels(DCM_OBJECT ** iod)
{
    static U32 		pixel = 0;
    DCM_ELEMENT 	e = {DCM_PXLPIXELDATA, DCM_OB, "", 1, sizeof(pixel), (void *) &pixel};

    DCM_RemoveElement(iod, DCM_PXLPIXELDATA);
    DCM_AddElement(iod, &e);
}

static void
inflatePixels(DCM_OBJECT ** iod)
{
  static U16 	rows = 0;
  static U16 	cols = 0;
  static U16 	bitsAllocated = 0;

  DCM_ELEMENT e1[] = {
    { DCM_IMGROWS, DCM_US, "", 1, sizeof(U16), (void*)&rows },
    { DCM_IMGCOLUMNS, DCM_US, "", 1, sizeof(U16), (void*)&cols },
    { DCM_IMGBITSALLOCATED, DCM_US, "", 1, sizeof(U16), (void*)&bitsAllocated }
  };

  CONDITION cond;

  DCM_ELEMENT e = {DCM_PXLPIXELDATA, DCM_OB, "", 1, 0, 0 };

  cond = DCM_ParseObject(iod, e1, (int)DIM_OF(e1), 0, 0, 0);

  if (cond != DCM_NORMAL) {
    fprintf(stderr, "Could not parse image for pixel parameters\n");
    COND_DumpConditions();
    exit(1);
  }

  e.length = rows * cols * (bitsAllocated/8);
  e.d.ot = calloc(e.length, 1);

  DCM_RemoveElement(iod, DCM_PXLPIXELDATA);
  DCM_AddElement(iod, &e);

  free (e.d.ot);
}

static void getXferSyntax(DCM_OBJECT** iod, char* fileXferSyntax)
{
  CONDITION 	cond;
  DCM_ELEMENT 	elements[] = {{DCM_METATRANSFERSYNTAX, DCM_UI, "", 1, 65, 0}};

  elements[0].d.string = fileXferSyntax;

  cond = DCM_ParseObject(iod, elements, DIM_OF(elements), NULL, 0, NULL);
  if (cond != DCM_NORMAL) {
    printf("Unable to get Transfer Syntax from Part 10 file\n");
    exit(1);
  }
}

static int
testForEncapsulatedXferSyntax(const char* fileXferSyntax)
{
  char* 	xfer[] = {DICOM_TRANSFERLITTLEENDIAN, DICOM_TRANSFERLITTLEENDIANEXPLICIT, DICOM_TRANSFERBIGENDIANEXPLICIT};
  int 		index;

  if (fileXferSyntax == 0) return 0;

  for (index = 0; index < 3; index++) {
	  if (strcmp(xfer[index], fileXferSyntax) == 0) return 0;
  }

  return 1;
}

/* sendImageSet
**
** Purpose:
**	Send a set of images to the destination
**
** Parameter Dictionary:
**	argc		Number of command line arguments
**	argv		Actual arguments
**	network		Key to the network connection
**	params		Service parameters describing the Association
**	SOPName		SOP Class ID of the image to be sent
**
** Return Values:
**	None
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static void
sendImageSet(int argc, char **argv, DUL_NETWORKKEY ** network, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPName, int limit, CTNBOOLEAN trimPixels, CTNBOOLEAN expandPixels, CTNBOOLEAN eFilmFlag,
	     	 CTNBOOLEAN timeTransfer, CTNBOOLEAN allowRepeatedElements, CTNBOOLEAN allowVRMismatch, const char* sopInstanceUID, char** xferSyntaxes, int xferSyntaxCount)
{
    DUL_ASSOCIATIONKEY		* association = NULL;	/* Describes the Association with the Acceptor */

    static char
    	SOPClass[DICOM_UI_LENGTH + 1] = "",
        lastSOPClass[DICOM_UI_LENGTH + 1] = "";

    CONDITION		cond;
    DCM_OBJECT 		*iod;
    static char     instanceUID[DICOM_UI_LENGTH + 1] = "";
    DCM_ELEMENT 	elements[] = {{DCM_IDSOPCLASSUID, DCM_UI, "", 1, sizeof(SOPClass), (void *) SOPClass}, {DCM_IDSOPINSTANCEUID, DCM_UI, "", 1, sizeof(instanceUID), (void *) instanceUID}};
    CTNBOOLEAN 		part10File;
    void 			*timeStamp = NULL;
    unsigned long 	objectLength = 0;
    double 			deltaTime = 0.;
    LST_HEAD* 		fileNames = 0;
    UTL_FILEITEM* 	p = NULL;
    char 			fileXferSyntax[65] = "";
    int 			isEncapsulatedXferSyntax = 0;
    int 			status;

  if (limit < argc) argc = limit;

  fileNames = LST_Create();
  while (argc-- > 0) {
	  fillFileList(*argv, &fileNames);
	  argv++;
  }

  p = LST_Dequeue(&fileNames);
  while (p != NULL) {
	  unsigned long options = DCM_ORDERLITTLEENDIAN;
	  if (timeTransfer) timeStamp = UTL_GetTimeStamp();

	  part10File = FALSE;
	  if (eFilmFlag) {
		  options = DCM_PART10FILE | DCM_EFILM;
		  part10File = TRUE;
	  }
	  if (allowRepeatedElements) options |= DCM_ALLOWREPEATELEMENTS;
	  if (allowVRMismatch) options |= DCM_ACCEPTVRMISMATCH;

	  printf("%s\n", p->path);

	  cond = DCM_OpenFile(p->path, options, &iod);
	  if (cond != DCM_NORMAL){
		  unsigned long optionsPart10 = DCM_ORDERLITTLEENDIAN | DCM_PART10FILE;
		  if (allowRepeatedElements) optionsPart10 |= DCM_ALLOWREPEATELEMENTS;
		  if (allowVRMismatch) optionsPart10 |= DCM_ACCEPTVRMISMATCH;

		  part10File = TRUE;
		  cond = DCM_OpenFile(p->path, optionsPart10, &iod);
		  if (cond != DCM_NORMAL) myExit(&association, 0);
	  }
	  (void) COND_PopCondition(TRUE);

	  cond = DCM_ParseObject(&iod, elements, DIM_OF(elements), NULL, 0, NULL);
	  if (cond != DCM_NORMAL) myExit(&association, 0);

	  if (part10File) {
		  getXferSyntax(&iod, fileXferSyntax);
		  isEncapsulatedXferSyntax = testForEncapsulatedXferSyntax(fileXferSyntax);
	  }

	  if (strcmp(SOPClass, lastSOPClass) != 0) {
		  if (strlen(lastSOPClass) != 0) {
			  (void) DUL_ReleaseAssociation(&association);
			  (void) DUL_DropAssociation(&association);
		  }
		  (void) DUL_ClearServiceParameters(params);
		  if (isEncapsulatedXferSyntax) {
			  char* xfer[1];
			  xfer[0] = fileXferSyntax;
			  status = requestAssociation(network, &association, params, SOPClass, xfer, 1);
		  }else{
			  status = requestAssociation(network, &association, params, SOPClass, xferSyntaxes, xferSyntaxCount);
		  }
		  if (status == 0) myExit(NULL, 0);
      }
	  if (part10File) {
		  cond = DCM_RemoveGroup(&iod, DCM_GROUPFILEMETA);
		  if (cond != DCM_NORMAL) myExit(&association, 0);
		  (void) DCM_RemoveGroup(&iod, DCM_GROUPPAD);
		  (void) COND_PopCondition(TRUE);
	  }
	  if (trimPixels) replacePixels(&iod);
	  if (expandPixels) inflatePixels(&iod);
	  if (timeTransfer) (void) DCM_GetObjectSize(&iod, &objectLength);
    {
      if (sopInstanceUID != 0) replaceSOPInstanceUID(&iod, sopInstanceUID);
    }

    cond = sendImage(&association, params, &iod, SOPClass, instanceUID, "");
    if (cond != SRV_NORMAL) myExit(&association, cond);

    strcpy(lastSOPClass, SOPClass);
    (void) DCM_CloseObject(&iod);
    if (timeTransfer) {
    	deltaTime = UTL_DeltaTime(timeStamp);
    	printf("%10d bytes transfer in %7.3f seconds (%f kB/s)\n", objectLength, deltaTime, (((float) objectLength) / 1000.) / deltaTime);
    	UTL_ReleaseTimeStamp(timeStamp);
    }
    CTN_FREE(p);
    p = LST_Dequeue(&fileNames);
  }

  (void) DUL_ReleaseAssociation(&association);
  (void) DUL_DropAssociation(&association);
  (void) DUL_ClearServiceParameters(params);
  (void) DUL_DropNetwork(network);
}


/* sendImage
**
** Purpose:
**	Send a single image over the Association
**
** Parameter Dictionary:
**	assocition		Handle to the Association
**	params			Service parameters describing the Association
**	object			Handle to the DICOM object holding the image
**	SOPClass		SOP Class UID
**	instanceUID		SOP Instance UID of the image
**	moveAETitle		Name of the Application Entity
**
** Return Values:
**
**	SRV_ILLEGALPARAMETER
**	SRV_NOCALLBACK
**	SRV_NORMAL
**	SRV_NOSERVICEINASSOCIATION
**	SRV_OBJECTBUILDFAILED
**	SRV_REQUESTFAILED
**	SRV_UNEXPECTEDCOMMAND
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static CONDITION
sendImage(DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, DCM_OBJECT ** object, char *SOPClass, char *instanceUID, char *moveAETitle)
{
    MSG_C_STORE_REQ    	request;
    MSG_C_STORE_RESP	response;
    CONDITION			cond;

    request.type = MSG_K_C_STORE_REQ;
    request.messageID = SRV_MessageIDOut();
    request.priority = 0;
    request.dataSetType = DCM_CMDDATAIMAGE;
    request.dataSet = *object;
    request.fileName = NULL;
    strcpy(request.classUID, SOPClass);
    strcpy(request.instanceUID, instanceUID);
    strcpy(request.moveAETitle, moveAETitle);

    cond = SRV_CStoreRequest(association, params, &request, &response, sendCallback, "context string", "");
    if (cond == SRV_NORMAL) MSG_DumpMessage(&response, stdout);

    if (responseSensitive && response.status != MSG_K_SUCCESS) cond = 0;

    return cond;
}


/* sendCallback
**
** Purpose:
**	Callback routine
**
** Parameter Dictionary:
**	request		Pointer to request message
**	response	Pointer to response message
**	transmitted	Number of bytes sent
**	total		Total number of bytes sent
**	string		Context Information
**
** Return Values:
**	SRV_NORMAL
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/
static CONDITION
sendCallback(MSG_C_STORE_REQ * request, MSG_C_STORE_RESP * response, unsigned long transmitted, unsigned long total, void *string)
{
    if (transmitted == 0 && !silent) printf("Initial call to sendCallback\n");

    if (!silent) printf("%8d bytes transmitted of %8d (%s)\n", transmitted, total, (char *) string);

    if (response != NULL && !silent) MSG_DumpMessage(response, stdout);

    return SRV_NORMAL;
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
send_image [-a application] [-c called] [-m maxPDU] [-p] [-q] [-r] [-s SOPName] [-t] [-x FAC] [-X xfer] [-v] [-w flag] [-Z] node port image [image...]\n\
    \n\
    -a    Set application title of this (calling) application\n\
    -c    Set called AE title to title in Association RQ\n\
    -m    Set maximum PDU in Association RQ to maxPDU\n\
    -p    Alter image by sending minimal pixel data\n\
    -q    Quiet mode.  Suppresses some messages to stdout\n\
    -r    Make program sensitve to response status.  If not success, stop\n\
    -s    Force an initial Association using one SOP Class based on SOPName\n\
          (CR, CT, MR, NM, SC, US)\n\
    -t    Time the image transfer.  Print elapsed time and transfer rate.\n\
    -v    Place DUL and SRV facilities in verbose mode\n\
    -x    Place one facility(DCM, DUL, SRV) in verbose mode\n\
    -X    Specify a transfer syntax to be proposed; may repeat this switch\n\
    -w    Set open options; flag can be REPEAT \n\
    -Z    Allow VR mismatch in input files\n\
  \n\
    node  Node name for network connection\n\
    port  TCP / IP port number of server application\n\
    image A list of one or more images to send\n";

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
myExit(DUL_ASSOCIATIONKEY ** association, CONDITION cond)
{
    fprintf(stderr, "Abnormal exit\n");
    if (cond == SRV_NOSERVICEINASSOCIATION) {
    	fprintf(stderr,	"The returned condition implies the Storage SCP accepted your association \n request but rejected the SOP class. This could mean it does not support that\n class or that it rejected the proposed transfer syntax(es).\n");
    }
    COND_DumpConditions();

    if (association != NULL)
    	if (*association != NULL) (void) DUL_DropAssociation(association);
    THR_Shutdown();
    exit(1);
}



/* requestAssociation
**
** Purpose:
**	Request for a Association establishment
**
** Parameter Dictionary:
**	network		Key to the network connection
**	association	Handle to the Association (to be returned)
**	params		Service parameters describing the Association
**	SOPClass	SOPClass for which the service is to be requested
**
** Return Values:
**	1 => success
**	0 => failure
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

static int
requestAssociation(DUL_NETWORKKEY ** network, DUL_ASSOCIATIONKEY ** association, DUL_ASSOCIATESERVICEPARAMETERS * params, char *SOPClass, char** xferSyntaxes, int xferSyntaxCount)
{
  CONDITION cond;

  cond = SRV_ProposeSOPClassWithXfer(SOPClass, DUL_SC_ROLE_SCU, xferSyntaxes, xferSyntaxCount, 1, params);

  if (cond != SRV_NORMAL) {
	  COND_DumpConditions();
	  return 0;
  }

  cond = DUL_RequestAssociation(network, params, association);
  if (cond != DUL_NORMAL) {
	  if (cond == DUL_ASSOCIATIONREJECTED) {
		  fprintf(stderr, "Association Rejected\n");
		  fprintf(stderr, " Result: %2x Source %2x Reason %2x\n", params->result, params->resultSource, params->diagnostic);
    }
    return 0;
  }
  if (!silent) {
	  (void) printf("Association accepted, parameters:\n");
	  DUL_DumpParams(params);
  }
  return 1;
}
