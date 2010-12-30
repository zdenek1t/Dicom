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
** Module Name(s):	SRV_Message
** Author, Date:	Stephen M. Moore, 1-May-93
** Intent:		Define the ASCIZ messages for errors and provide
**			a function for table lookup of the error messages.
** Last Update:		$Author: smm $, $Date: 2001/12/21 16:52:11 $
** Source File:		$RCSfile: srvcond.c,v $
** Revision:		$Revision: 1.24 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.24 $ $RCSfile: srvcond.c,v $";

#include "../dicom/ctn_os.h"

#if 0
#include <stdio.h>
#include <sys/types.h>
#endif

#include "../dicom/dicom.h"
#include "../uid/dicom_uids.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../dulprotocol/dulprotocol.h"
#include "../objects/dicom_objects.h"
#include "../messages/dicom_messages.h"
#include "../services/dicom_services.h"


typedef struct vector {
    CONDITION cond;
    char *message;
}   VECTOR;

static VECTOR messageVector[] = {
    {SRV_NORMAL, "Normal return from SRV routine"},
    {SRV_UNSUPPORTEDSERVICE, "SRV Request for unsupported SOP Class (%s) in %s"},
    {SRV_UNSUPPORTEDTRANSFERSYNTAX, "SRV No transfer syntaxes supported for SOP class (%s) in %s"},
    {SRV_PEERREQUESTEDRELEASE, "SRV Peer requested release in %s"},
    {SRV_PEERABORTEDASSOCIATION, "SRV Peer Aborted Association in %s"},
    {SRV_RECEIVEFAILED, "SRV Failed to receive fragment in %s"},
    {SRV_ILLEGALASSOCIATION, "SRV Illegal Association in routine: %s"},
    {SRV_SENDFAILED, "SRV Send (%s) failed in %s"},
    {SRV_OBJECTTOOLARGE, "SRV Object received too large in %s"},
    {SRV_UNEXPECTEDPRESENTATIONCONTEXTID, "SRV Unexpected presentation context ID: %x in %s"},
    {SRV_READPDVFAILED, "SRV Read PDV Failed in %s"},
    {SRV_UNEXPECTEDPDVTYPE, "SRV Unexpected PDV type (%d) in %s"},
    {SRV_FILECREATEFAILED, "SRV Failed to create file %s in %s"},
    {SRV_NOSERVICEINASSOCIATION, "SRV Service not supported in Association: %s"},
    {SRV_LISTFAILURE, "SRV List Failure in %s"},
    {SRV_MALLOCFAILURE, "SRV Failed to malloc %d bytes in %s"},
    {SRV_PRESENTATIONCONTEXTERROR, "SRV Presentation context error in %s"},
    {SRV_PARSEFAILED, "SRV Command Parse failed in %s"},
    {SRV_UNSUPPORTEDCOMMAND, "SRV Received unsupported DICOM Command (%04x) in %s"},
    {SRV_NOTRANSFERSYNTAX, "SRV Transfer Syntax (%s) is not supported for abstract syntax (%s) in %s"},
    {SRV_NOCALLBACK, "SRV No callback function defined in routine %s"},
    {SRV_ILLEGALPARAMETER, "SRV Illegal %s parameter in %s structure in %s"},
    {SRV_OBJECTBUILDFAILED, "SRV Failed to build object of type %s in %s"},
    {SRV_REQUESTFAILED, "SRV Request failed in %s"},
    {SRV_UNEXPECTEDCOMMAND, "SRV Unexpected DICOM Command (%04x) received in %s"},
    {SRV_CALLBACKABORTEDSERVICE, "SRV User callback function aborted service in %s"},
    {SRV_RESPONSEFAILED, "SRV Response failed in %s"},
    {SRV_OBJECTACCESSFAILED, "SRV Failed to access object of type %s in %s"},
    {SRV_QUERYLEVELATTRIBUTEMISSING, "SRV Failed to find Query Level Attribute in %s"},
    {SRV_ILLEGALQUERYLEVELATTRIBUTE, "SRV Illegal Query Level Attribute (%s) in %s"},
    {SRV_SYSTEMERROR, "SRV System error (%s) occurred in %s"},
    {SRV_LISTCREATEFAILURE, "SRV Failed to create a list in %s"},
    {SRV_SUSPICIOUSRESPONSE, "SRV In %s response, suspicious combination of status (%s) and data set type (%s) in %s"},
    {SRV_OPERATIONCANCELLED, "SRV Callback or peer cancelled operation"},
    {SRV_PDVRECEIVEDOUTOFSEQUENCE, "SRV PDV Received out of sequence.  Expected (%s), Received (%s %d) in %s"},
    {SRV_RECEIVEDATASETFAILED, "SRV Receive Data Set failed in %s"},
    {0, NULL}
};


/* SRV_Message
**
** Purpose:
**	Issue an error message depending on the condition.
**
** Parameter Dictionary:
**	condition	Condition indicating type of error.
**
** Return Values:
**	message
**	NULL, if condition doesn't exist.
**
** Algorithm:
**	Description of the algorithm (optional) and any other notes.
*/

char *
SRV_Message(CONDITION condition)
{
    int		index;

    for (index = 0; messageVector[index].message != NULL; index++){
    	if (condition == messageVector[index].cond) return messageVector[index].message;
    }
    return NULL;
}
