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
** Author, Date:	Stephen Moore, 6-Nov-94
** Intent:
** Last Update:		$Author: smm $, $Date: 2001-12-21 16:37:43 $
** Source File:		$RCSfile: fillImageDB.c,v $
** Revision:		$Revision: 1.11 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.11 $ $RCSfile: fillImageDB.c,v $";

#include "../dicom_lib/dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#ifdef SOLARIS
#include <netdb.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifdef MACH
#include <unistd.h>
#endif
#include <sys/wait.h>
#ifdef MALLOC_DEBUG
#include "malloc.h"
#endif
#endif

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/thread/ctnthread.h"
#include "../dicom_lib/tbl/tbl.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "../dicom_lib/dulprotocol/dulprotocol.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/services/dicom_services.h"
#include "../dicom_lib/manage/manage.h"
#include "../dicom_lib/idb/idb.h"

//#include "image_server.h"
#include "../apps_archive_server/image_archive.h"

static void usageerror();
static CONDITION
insertImage(DMAN_HANDLE ** handle, char *fileName, char *owner);


char        *controlDatabase = "DCM_SRV";
CTNBOOLEAN  verboseDUL = FALSE;
CTNBOOLEAN  verboseTBL = FALSE;
CTNBOOLEAN  verboseSRV = FALSE;
CTNBOOLEAN  silent = FALSE;
CTNBOOLEAN  doVerification = FALSE;

int
main(int argc, char **argv)
{
    CONDITION 		cond;
    CTNBOOLEAN 		verboseTBL = FALSE;
    IDB_HANDLE 		*IDBHandle;
    char 			*owner = "";

    while (--argc > 0 && *(++argv)[0] == '-') {
    	switch ((*argv)[1]) {
			case 'o':
						if (--argc < 1) usageerror();
						argv++;
						owner = *argv;
						break;
			case 'x':
						if (--argc < 1)	usageerror();
						argv++;
						if (strcmp(*argv, "TBL") == 0){
							verboseTBL = TRUE;
						}else{
							usageerror();
						}
						break;
			default:
						printf("Unrecognized option: %s\n", *argv);
						break;
    	}
    }

    if (argc < 2) usageerror();

    THR_Init();
    TBL_Debug(verboseTBL);

    if (IDB_Open(*argv, &IDBHandle) != IDB_NORMAL) {
    	printf("Error opening IDB file: %s\n", *argv);
    	COND_DumpConditions();
    	exit(1);
    }
    argc--;

    while (argc-- > 0) {
    	printf("%s\n", *++argv);
    	cond = insertImage(&IDBHandle, *argv, owner);
    	if (cond != APP_NORMAL) {
    		printf("Could not insert image: %s\n", *argv);
    		COND_DumpConditions();
    		THR_Shutdown();
    		exit(1);
    	}
    }

    THR_Shutdown();
    exit(0);
}

static void
usageerror()
{
    char msg[] = "\
Usage: [-o owner] [-x <FAC>] database file [file...]\n\
\n\
    -o    Set owner of patient, study, series, image\n\
    -x    Place facility <FAC> (TBL) in verbose mode\n\
\n\
    database The image database to be used\n\
    file     A list of one or more image files to be inserted\n";

    fprintf(stderr, msg);
    exit(1);
}

static int
fileSize(const char *fileName)
{
    int 		status;
    struct 		stat im_stat;
    int 		fd;

    fd = open(fileName, O_RDONLY);
    if (fd < 0)	return 0;

    status = fstat(fd, &im_stat);
    (void) close(fd);
    if (status < 0)	return 0;

    return im_stat.st_size;
}

static CONDITION
insertImage(DMAN_HANDLE ** handle, char *fileName, char *owner)
{
    IDB_Insertion 				Insertion;
    IDB_InstanceListElement 	imageInstance;
    CONDITION 					cond;
    DCM_OBJECT 					*obj;
    CTNBOOLEAN 					part10 = FALSE;

    memset(&Insertion, 0, sizeof(Insertion));

    cond = DCM_OpenFile(fileName, DCM_ORDERLITTLEENDIAN | DCM_FORMATCONVERSION,	&obj);
    if (cond != DCM_NORMAL) {
    	cond = DCM_OpenFile(fileName, DCM_PART10FILE | DCM_FORMATCONVERSION, &obj);
    	if (cond != DCM_NORMAL) return 0;
    	part10 = TRUE;
    }

    cond = parseImageForInsert(&obj, &Insertion);
    (void) DCM_CloseObject(&obj);
    if (cond != APP_NORMAL)	return 0;

    strcpy(Insertion.image.Path, fileName);
    if (part10){
    	extractTransferSyntax(&obj, Insertion.image.Transfer);
    }else{
    	strcpy(Insertion.image.Transfer, DICOM_TRANSFERLITTLEENDIAN);
    }

    strcpy(Insertion.patient.Owner, owner);
    strcpy(Insertion.study.Owner, owner);
    strcpy(Insertion.series.Owner, owner);
    strcpy(Insertion.image.Owner, owner);
    Insertion.image.Size = fileSize(fileName);

    cond = IDB_InsertImage(handle, &Insertion);
    if (cond != IDB_NORMAL) {
    	COND_DumpConditions();
    	return 0;
    }
    (void) COND_PopCondition(TRUE);
    return APP_NORMAL;
}
