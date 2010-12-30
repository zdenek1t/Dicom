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
** @$=@$=@$=
*/
/*
**				DICOM 93
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):
** Author, Date:	Stephen M. Moore, 19-Apr-93
** Intent:		This file contains constant and structure definitions
**			and function prototypes for the MSG facility.
** Last Update:		$Author: drm $, $Date: 2002/07/17 19:40:04 $
** Source File:		$RCSfile: dicom_messages.h,v $
** Revision:		$Revision: 1.34 $
** Status:		$State: Exp $
*/

#ifndef DICOM_MESSAGES_IS_IN
#define DICOM_MESSAGES_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

typedef unsigned short MSG_COMMAND;

/* Enumerate the type of messages (commands) that this facility supports.
*/

typedef enum {
    MSG_K_C_ECHO_REQ,
    MSG_K_C_ECHO_RESP,
    MSG_K_C_FIND_REQ,
    MSG_K_C_FIND_RESP,
    MSG_K_C_GET_REQ,
    MSG_K_C_GET_RESP,
    MSG_K_C_MOVE_REQ,
    MSG_K_C_MOVE_RESP,
    MSG_K_C_PRINT_REQ,
    MSG_K_C_PRINT_RESP,
    MSG_K_C_STORE_REQ,
    MSG_K_C_STORE_RESP,
    MSG_K_C_CANCEL_REQ,

    MSG_K_N_EVENT_REPORT_REQ,
    MSG_K_N_EVENT_REPORT_RESP,
    MSG_K_N_GET_REQ,
    MSG_K_N_GET_RESP,
    MSG_K_N_SET_REQ,
    MSG_K_N_SET_RESP,
    MSG_K_N_ACTION_REQ,
    MSG_K_N_ACTION_RESP,
    MSG_K_N_CREATE_REQ,
    MSG_K_N_CREATE_RESP,
    MSG_K_N_DELETE_REQ,
    MSG_K_N_DELETE_RESP,

    MSG_K_REFERENCED_ITEM,
    MSG_K_NONE
}   MSG_TYPE;

/* Define the set of status codes for each service class.  The
** first set of status codes are defined in Annex C of Part 7.
*/

#define	MSG_K_SUCCESS						0x0000
#define	MSG_K_CANCEL						0xfe00
#define	MSG_K_ATTRIBUTELISTERRORR			0x0107
#define	MSG_K_CLASSINSTANCECONFLICT			0x0119
#define	MSG_K_DUPLICATESOPINSTANCE			0x0111
#define	MSG_K_DUPLICATEINVOCATION			0x0210
#define	MSG_K_INVALIDARGUMENTVALUE			0x0115
#define	MSG_K_INVALIDATTRIBUTEVALUE			0x0106
#define	MSG_K_INVALIDOBJECTINSTANCE			0x0117
#define	MSG_K_MISSINGATTRIBUTE				0x0120
#define	MSG_K_MISSINGATTRIBUTEVALUE			0x0121
#define	MSG_K_MISTYPEDARGUMENT				0x0212
#define	MSG_K_NOSUCHARGUMENT				0x0114
#define	MSG_K_NOSUCHATTRIBUTE				0x0105
#define	MSG_K_NOSUCHEVENTTYPE				0x0113
#define	MSG_K_NOSUCHOBJECTINSTANCE			0x0112
#define	MSG_K_NOSUCHSOPCLASS				0x0118
#define	MSG_K_PROCESSINGFAILURE				0x0110
#define	MSG_K_RESOURCELIMITATION			0x0213
#define	MSG_K_UNRECOGNIZEDOPERATION			0x0211

/* Now define status codes that depend on the various SOP classes.
** These will be found in the various annexes of Part 4.
*/

#define	MSG_K_C_STORE_OUTOFRESOURCES				0xa700
#define	MSG_K_C_STORE_SOPCLASSNOTSUPPORTED			0xa800
#define	MSG_K_C_STORE_DATASETNOTMATCHSOPCLASSERROR	0xa900
#define	MSG_K_C_STORE_CANNOTUNDERSTAND				0xc000
#define	MSG_K_C_STORE_DATAELEMENTCOERCION			0xb000
#define	MSG_K_C_STORE_DATASETNOTMATCHSOPCLASSWARN	0xb007
#define	MSG_K_C_STORE_ELEMENTSDISCARDED				0xb006
#define	MSG_K_C_STORE_COMPLETE						0x0000

#define	MSG_K_C_FIND_OUTOFRESOURCES					0xa700
#define	MSG_K_C_FIND_SOPCLASSNOTSUPPORTED			0xa800
#define	MSG_K_C_FIND_IDENTIFIERNOTMATCHSOPCLASS		0xa900
#define	MSG_K_C_FIND_UNABLETOPROCESS				0xc000
#define	MSG_K_C_FIND_MATCHCANCELLED					0xfe00
#define	MSG_K_C_FIND_COMPLETE						0x0000
#define	MSG_K_C_FIND_MATCHCONTINUING				0xff00
#define	MSG_K_C_FIND_MATCHCONTINUINGWARN			0xff01

#define	MSG_K_C_MOVE_UNABLETOCACULATEMATCHCOUNT		0XA701
#define	MSG_K_C_MOVE_UNABLETOPERFORMSUBOPERATIONS	0XA702
#define	MSG_K_C_MOVE_SOPCLASSNOTSUPPORTED			0XA800
#define	MSG_K_C_MOVE_MOVEDESTINATIONUNKNOWN			0XA801
#define	MSG_K_C_MOVE_IDENTIFIERNOTMATCHSOPCLASS		0XA900
#define	MSG_K_C_MOVE_UNABLETOPROCESS				0XC000
#define	MSG_K_C_MOVE_SUBOPERATIONSCANCELLED			0XFE00
#define	MSG_K_C_MOVE_COMPLETEWITHFAILURES			0XB000
#define	MSG_K_C_MOVE_SUBOPERATIONSCONTINUING		0xFF00

#define	MSG_K_C_GET_UNABLETOCACULATEMATCHCOUNT		0XA701
#define	MSG_K_C_GET_UNABLETOPERFORMSUBOPERATIONS	0XA702
#define	MSG_K_C_GET_SOPCLASSNOTSUPPORTED			0XA800
#define	MSG_K_C_GET_IDENTIFIERNOTMATCHSOPCLASS		0XA900
#define	MSG_K_C_GET_UNABLETOPROCESS					0XC000
#define	MSG_K_C_GET_SUBOPERATIONSCANCELLED			0XFE00
#define	MSG_K_C_GET_COMPLETEWITHFAILURES			0XB000
#define	MSG_K_C_GET_SUBOPERATIONSCONTINUING			0xFF00

#define MSG_K_BFS_NCREATESUCCESS					0x0000
#define MSG_K_BFS_MEMORYALLOCATIONUNSUPPORTED		0xB600
#define MSG_K_BFS_FILMACCEPTEDFORPRINTING			0x0000
#define MSG_K_BFS_FILMCOLLATIONUNSUPPORTED			0xB601
#define MSG_K_BFS_NOIMAGEBOXSOPINSTANCES			0xB602
#define MSG_K_BFS_NOFILMBOXSOPINSTANCES				0xC600
#define MSG_K_BFS_UNABLETOCREATEPRINTJOBSOPINSTANCE	0xC601
#define MSG_K_BFS_IMAGEPOSITIONCOLLISION			0xC604
#define MSG_K_BFS_IMAGESIZELARGERTHANIMAGEBOXSIZE	0xC603

#define MSG_K_BFB_FILMACCEPTEDFORPRINTING			0x0000
#define MSG_K_BFB_NOIMAGEBOXSOPINSTANCES			0xB603
#define MSG_K_BFB_UNABLETOCREATEPRINTJOBSOPINSTANCE	0xC602
#define MSG_K_BFB_IMAGEPOSITIONCOLLISION			0xC604
#define MSG_K_BFB_IMAGESIZELARGERTHANIMAGEBOXSIZE	0xC603

#define MSG_K_BIB_INSUFFICIENTMEMORYINPRINTER		0xC605
#define MSG_K_BIB_MORETHANONEVOILUTBOXINIMAGE		0xC606

#define MSG_K_N_ACTION_UNABLETOUPDATE				0xA501
#define MSG_K_N_ACTION_WRONGTRANSACTIOUID			0xA502
#define MSG_K_N_ACTION_ALREADYINPROGRESS			0xA503

typedef enum {
    MSG_K_CLASS_SUCCESS,
    MSG_K_CLASS_PENDING,
    MSG_K_CLASS_CANCEL,
    MSG_K_CLASS_WARNING,
    MSG_K_CLASS_FAILURE,
    MSG_K_CLASS_REFUSED
}   MSG_STATUS_CLASS;

typedef struct {
    unsigned short 		code;			/* Code value defined in Standard */
    unsigned short 		mask;			/* Mask to be ANDed with code */
    MSG_TYPE 			messageType;	/* One of the enumerated message types */
    MSG_STATUS_CLASS 	statusClass;	/* One of the enumerated status classes */
    char 				description[64];
}   MSG_STATUS_DESCRIPTION;

/* Define all of the structures which define fields for attributes in
** the command group and may contain a DICOM Information Object for
** the data set or identifier in a message.
*/

typedef struct {
    MSG_TYPE 	type;
}   MSG_GENERAL;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
}   MSG_C_CANCEL_REQ;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    char 				classUID[DICOM_UI_LENGTH + 1];
}   MSG_C_ECHO_REQ;

#define	MSG_K_C_ECHORESP_CLASSUID	0x01
typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    char 				classUID[DICOM_UI_LENGTH + 1];
}   MSG_C_ECHO_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		priority;
    DCM_OBJECT 			*identifier;
    char 				classUID[DICOM_UI_LENGTH + 1];
}   MSG_C_FIND_REQ;

#define	MSG_K_C_FINDRESP_CLASSUID	0x01
#define	MSG_K_C_FINDRESP_ERRORCOMMENT	0x02
typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    DCM_OBJECT 			*identifier;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				errorComment[DICOM_LO_LENGTH + 1];
}   MSG_C_FIND_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		priority;
    DCM_OBJECT 			*identifier;
    char 				classUID[DICOM_UI_LENGTH + 1];
}   MSG_C_GET_REQ;

#define MSG_K_C_GET_REMAINING  			0x10
#define MSG_K_C_GET_COMPLETED  			0x20
#define MSG_K_C_GET_FAILED     			0x40
#define MSG_K_C_GET_WARNING    			0x80
#define MSG_K_C_GETRESP_CLASSUID       	0x100
#define	MSG_K_C_GETRESP_ERRORCOMMENT	0x200

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    unsigned short 		remainingSubOperations;
    unsigned short 		completedSubOperations;
    unsigned short 		failedSubOperations;
    unsigned short 		warningSubOperations;
    DCM_OBJECT 			*identifier;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				errorComment[DICOM_LO_LENGTH + 1];
}   MSG_C_GET_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		priority;
    char 				moveDestination[20];
    DCM_OBJECT 			*identifier;
    char 				classUID[DICOM_UI_LENGTH + 1];
}   MSG_C_MOVE_REQ;

typedef struct {
    void 				*reserved[2];
    char 				UID[DICOM_UI_LENGTH + 1];
}   MSG_UID_ITEM;

#if STANDARD_VERSION < VERSION_JUL1993
#define	MSG_K_C_MOVE_SUCCESSUID			0x01
#define	MSG_K_C_MOVE_FAILEDUID			0x02
#define	MSG_K_C_MOVE_WARNINGUID			0x04
#endif
#define	MSG_K_C_MOVE_REMAINING			0x10
#define	MSG_K_C_MOVE_COMPLETED			0x20
#define	MSG_K_C_MOVE_FAILED				0x40
#define	MSG_K_C_MOVE_WARNING			0x80
#define	MSG_K_C_MOVERESP_CLASSUID		0x100
#define	MSG_K_C_MOVERESP_ERRORCOMMENT	0x200

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    unsigned short 		remainingSubOperations;
    unsigned short 		completedSubOperations;
    unsigned short 		failedSubOperations;
    unsigned short 		warningSubOperations;
#if STANDARD_VERSION < VERSION_JUL1993
    LST_HEAD 			*successUIDList;
    LST_HEAD 			*failedUIDList;
    LST_HEAD 			*warningUIDList;
#else
    DCM_OBJECT 			*dataSet;
#endif
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				errorComment[DICOM_LO_LENGTH + 1];
}   MSG_C_MOVE_RESP;

typedef struct {
    MSG_TYPE 			type;
}   MSG_C_PRINT_REQ;
typedef struct {
    MSG_TYPE 			type;
    unsigned short 		dataSetType;
}   MSG_C_PRINT_RESP;

#define	MSG_K_C_STORE_MOVEMESSAGEID	0x01
#define	MSG_K_C_STORE_MOVEAETITLE	0x02
typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		priority;
    unsigned short 		moveMessageID;
    DCM_OBJECT 			*dataSet;
    char 				*fileName;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
    char				moveAETitle[20];
}   MSG_C_STORE_REQ;

#define	MSG_K_C_STORERESP_CLASSUID			0x01
#define	MSG_K_C_STORERESP_INSTANCEUID		0x02
#define	MSG_K_C_STORERESP_ERRORCOMMENT		0x04

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];

    /* Fields that are needed for the status information */
    char 				errorComment[DICOM_LO_LENGTH + 1];
}   MSG_C_STORE_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		eventTypeID;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				affectedInstanceUID[DICOM_UI_LENGTH + 1];
    DCM_OBJECT 			*dataSet;
}   MSG_N_EVENT_REPORT_REQ;

#define	MSG_K_N_EVENTREPORTRESP_EVENTTYPEID				0x01
#define MSG_K_N_EVENTREPORTRESP_EVENTINFORMATION		0x02
#define MSG_K_N_EVENTREPORTRESP_REQUESTEDCLASSUID		0x04
#define MSG_K_N_EVENTREPORTRESP_REQUESTEDINSTANCEUID	0x08
#define MSG_K_N_EVENTREPORTRESP_ERRORCOMMENT			0x10
#define MSG_K_N_EVENTREPORTRESP_ERRORID					0x20

typedef struct {
    MSG_TYPE		 	type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    unsigned short 		eventTypeID;
    DCM_OBJECT 			*dataSet;	/* event reply */
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				affectedInstanceUID[DICOM_UI_LENGTH + 1];

    /* all fields that are needed for the status information DCM_OBJECT *eventInformation; */
    char 				requestedClassUID[DICOM_UI_LENGTH + 1];
    char 				requestedInstanceUID[DICOM_UI_LENGTH + 1];
    char 				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
}   MSG_N_EVENT_REPORT_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				requestedInstanceUID[DICOM_UI_LENGTH + 1];
    DCM_TAG 			*attributeList;
    int 				attributeCount;
}   MSG_N_GET_REQ;

#define MSG_K_N_GETRESP_ATTRIBUTEIDENTIFIERLIST		0x01
#define MSG_K_N_GETRESP_REQUESTEDCLASSUID			0x02
#define MSG_K_N_GETRESP_REQUESTEDINSTANCEUID		0x04
#define MSG_K_N_GETRESP_ERRORCOMMENT				0x08
#define MSG_K_N_GETRESP_ERRORID						0x10

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				affectedInstanceUID[DICOM_UI_LENGTH + 1];
    DCM_OBJECT 			*dataSet;

    /* All fields that are needed for status information */
    DCM_TAG 			*attributeIdentifierList;
    int 				attributeCount;
    char 				requestedClassUID[DICOM_UI_LENGTH + 1];
    char 				requestedInstanceUID[DICOM_UI_LENGTH + 1];
    char 				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
}   MSG_N_GET_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    DCM_OBJECT 			*dataSet;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
}   MSG_N_SET_REQ;

#define MSG_K_N_SETRESP_ATTRIBUTEIDENTIFIERLIST		0x01
#define MSG_K_N_SETRESP_REQUESTEDCLASSUID			0x02
#define MSG_K_N_SETRESP_REQUESTEDINSTANCEUID		0x04
#define MSG_K_N_SETRESP_ERRORCOMMENT				0x08
#define MSG_K_N_SETRESP_ERRORID						0x10
#define MSG_K_N_SETRESP_MODIFICATIONLIST			0x20

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
    DCM_OBJECT 			*dataSet;

    /* All fields that are needed for status information */
    DCM_TAG 			*attributeIdentifierList;
    int 				attributeCount;
    char 				requestedClassUID[DICOM_UI_LENGTH + 1];
    char 				requestedInstanceUID[DICOM_UI_LENGTH + 1];
    char				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
/*    DCM_OBJECT *modificationList; */
}   MSG_N_SET_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    unsigned short 		actionTypeID;
    DCM_OBJECT 			*actionInformation;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
}   MSG_N_ACTION_REQ;

#define MSG_K_N_ACTIONRESP_ACTIONINFORMATION		0x01
/*#define MSG_K_N_ACTIONRESP_REQUESTEDCLASSUID		0x02 */
/*#define MSG_K_N_ACTIONRESP_REQUESTEDINSTANCEUID	0x04 */
#define MSG_K_N_ACTIONRESP_ERRORCOMMENT				0x08
#define MSG_K_N_ACTIONRESP_ERRORID					0x10
#define MSG_K_N_ACTIONRESP_ACTIONTYPEID				0x20
#define MSG_K_N_ACTIONRESP_AFFECTEDCLASSUID			0x40
#define MSG_K_N_ACTIONRESP_AFFECTEDINSTANCEUID		0x80

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    unsigned short 		actionTypeID;
    DCM_OBJECT 			*actionReply;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];

    /* All fields that are needed for status information */
    /* char requestedClassUID[DICOM_UI_LENGTH + 1]; */
    /* char requestedInstanceUID[DICOM_UI_LENGTH + 1]; */
    char 				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
}   MSG_N_ACTION_RESP;

#define MSG_K_N_CREATEREQ_INSTANCEUID	(1 << 0)

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    DCM_OBJECT 			*dataSet;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
}   MSG_N_CREATE_REQ;

#define MSG_K_N_CREATERESP_ATTRIBUTEIDENTIFIERLIST	0x01
/*#define MSG_K_N_CREATERESP_REQUESTEDCLASSUID		0x02 These are in error */
/*#define MSG_K_N_CREATERESP_REQUESTEDINSTANCEUID	0x04 These are in error */
#define MSG_K_N_CREATERESP_ERRORCOMMENT				0x08
#define MSG_K_N_CREATERESP_ERRORID					0x10
#define MSG_K_N_CREATERESP_ATTRIBUTELIST			0x20
#define MSG_K_N_CREATERESP_AFFECTEDCLASSUID			0x40
#define MSG_K_N_CREATERESP_AFFECTEDINSTANCEUID		0x80

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    DCM_OBJECT 			*dataSet;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];

    /* All fields that are needed for status information */
    DCM_TAG 			*attributeIdentifierList;
    int 				attributeCount;
    /* char requestedClassUID[DICOM_UI_LENGTH + 1]; */
    /* char requestedInstanceUID[DICOM_UI_LENGTH + 1]; */
    char 				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
/*    DCM_OBJECT *attributeList;*/
}   MSG_N_CREATE_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageID;
    unsigned short 		dataSetType;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];
}   MSG_N_DELETE_REQ;

/*#define MSG_K_N_DELETERESP_REQUESTEDCLASSUID		0x01 */
/*#define MSG_K_N_DELETERESP_REQUESTEDINSTANCEUID	0x02 */
#define MSG_K_N_DELETERESP_ERRORCOMMENT				0x04
#define MSG_K_N_DELETERESP_ERRORID					0x08
#define MSG_K_N_DELETERESP_AFFECTEDCLASSUID			0x10
#define MSG_K_N_DELETERESP_AFFECTEDINSTANCEUID		0x20

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned short 		messageIDRespondedTo;
    unsigned short 		dataSetType;
    unsigned short 		status;
    char 				classUID[DICOM_UI_LENGTH + 1];
    char 				instanceUID[DICOM_UI_LENGTH + 1];

    /* All fields that are needed for status information */
    /* char requestedClassUID[DICOM_UI_LENGTH + 1]; */
    /* char requestedInstanceUID[DICOM_UI_LENGTH + 1]; */
    char 				errorComment[DICOM_LO_LENGTH + 1];
    unsigned short 		errorID;
}   MSG_N_DELETE_RESP;

typedef struct {
    MSG_TYPE 			type;
    long 				conditionalFields;
    unsigned char 		classUID[DICOM_UI_LENGTH + 1];
    unsigned char 		instanceUID[DICOM_UI_LENGTH + 1];
}   MSG_REFERENCED_ITEM;

#define	MSG_K_N_PATIENT_CREATED				1
#define	MSG_K_N_PATIENT_DELETED				2
#define	MSG_K_N_PATIENT_UPDATED				3
#define	MSG_K_N_VISIT_CREATED				1
#define	MSG_K_N_VISIT_SCHEDULED				2
#define	MSG_K_N_PATIENT_ADMITTED			3
#define	MSG_K_N_PATIENT_TRANSFERRED			4
#define	MSG_K_N_PATIENT_DISCHARGED			5
#define	MSG_K_N_VISIT_DELETED				6
#define	MSG_K_N_VISIT_UPDATED				7
#define	MSG_K_N_STUDY_CREATED				1
#define	MSG_K_N_STUDY_SCHEDULED				2
#define	MSG_K_N_PATIENT_ARRIVED				3
#define	MSG_K_N_STUDY_STARTED				4
#define	MSG_K_N_STUDY_COMPLETED				5
#define	MSG_K_N_STUDY_VERIFIED				6
#define	MSG_K_N_STUDY_READ					7
#define	MSG_K_N_STUDY_DELETED				8
#define	MSG_K_N_STUDY_UPDATED				9
#define	MSG_K_N_RESULTS_CREATED				1
#define	MSG_K_N_RESULTS_DELETED				2
#define	MSG_K_N_RESULTS_UPDATED				3
#define	MSG_K_N_INTERPRETATION_CREATED		1
#define	MSG_K_N_INTERPRETATION_RECORDED		2
#define	MSG_K_N_INTERPRETATION_TRANSCRIBED 	3
#define	MSG_K_N_INTERPRETATION_APPROVED		4
#define	MSG_K_N_INTERPRETATION_DELETED		5
#define	MSG_K_N_INTERPRETATION_UPDATED		6

/* Define the function prototypes for this set of routines. */
CONDITION
MSG_BuildCommand(void *message, DCM_OBJECT ** obj);
CONDITION
MSG_Free(void **message);
CONDITION
MSG_ParseCommand(DCM_OBJECT ** obj, void **message);
void MSG_DumpMessage(void *message, FILE * f);
CONDITION
MSG_StatusLookup(unsigned short code, MSG_TYPE messageType, MSG_STATUS_DESCRIPTION * statusDescription);

char
*MSG_Message(CONDITION cond);

#define	MSG_NORMAL					FORM_COND(FAC_MSG, SEV_SUCC, 1)
#define MSG_PARSEFAILED				FORM_COND(FAC_MSG, SEV_ERROR, 2)
#define	MSG_ZEROLENGTHCLASSUID		FORM_COND(FAC_MSG, SEV_ERROR, 3)
#define	MSG_ZEROLENGTHINSTANCEUID	FORM_COND(FAC_MSG, SEV_ERROR, 4)
#define	MSG_ILLEGALMESSAGETYPE		FORM_COND(FAC_MSG, SEV_ERROR, 5)
#define	MSG_NOCOMMANDELEMENT		FORM_COND(FAC_MSG, SEV_ERROR, 6)
#define	MSG_UNSUPPORTEDCOMMAND		FORM_COND(FAC_MSG, SEV_ERROR, 7)
#define	MSG_MALLOCFAILURE			FORM_COND(FAC_MSG, SEV_ERROR, 9)
#define	MSG_OBJECTACCESSERROR		FORM_COND(FAC_MSG, SEV_ERROR,10)
#define	MSG_OBJECTCREATEFAILED		FORM_COND(FAC_MSG, SEV_ERROR,11)
#define	MSG_MODIFICATIONFAILURE		FORM_COND(FAC_MSG, SEV_ERROR,12)
#define	MSG_LISTFAILURE				FORM_COND(FAC_MSG, SEV_ERROR,13)
#define	MSG_STATUSCODENOTFOUND		FORM_COND(FAC_MSG, SEV_ERROR,14)
#define	MSG_MUTEXFAILED				FORM_COND(FAC_MSG, SEV_ERROR,15)

#ifdef  __cplusplus
}
#endif

#endif
