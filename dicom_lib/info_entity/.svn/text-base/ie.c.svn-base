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
** Module Name(s):
**			IE_ExamineObject
**			IE_ExamineInformationEntity
**			IE_ExamineModule
**			IE_ObjectRequirements
**			IE_IERequirements
**			IE_ModuleRequirements
**			IE_Free
**			findModality
**			findSOPClassUID
**			lookupModality
**			findElement
** Author, Date:	Stephen M. Moore, and Pei Weng, 30-May-93
** Intent:		Provide subroutine facility for manipulating the
**			information entities in DICOM information objects.
** Last Update:		$Author: asg $, $Date: 1995/06/22 17:48:12 $
** Source File:		$RCSfile: ie.c,v $
** Revision:		$Revision: 1.24 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.24 $ $RCSfile: ie.c,v $";

#include <stdio.h>
#include <sys/types.h>
#ifndef MACOS
#include <stdlib.h>
#endif
#include <string.h>

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "../lst/lst.h"
#include "../objects/dicom_objects.h"
#include "dicom_ie.h"
#include "../uid/dicom_uids.h"
#include "tables.h"

/* Prototypes for internal functions
 */
static CTNBOOLEAN
findElement(DCM_OBJECT * object, DCM_TAG tag, DCM_ELEMENT * element);
static SOP_CLASS
findModality(DCM_OBJECT * object);
static char
*findSOPClassUID(DCM_OBJECT * object);
static SOP_CLASS
lookupModality(char *classUID);

/* IE_ExamineObject
**
** Purpose:
**	IE_ExamineObject examines a DICOM information object and creates
**	an IE_OBJECT structure defined by the IE facility. The structure
**	contains a description of the object and a list of Information
**	Entities which should be present in this DICOM object(as defined
**	in Part 3 of the DICOM standard).
**
**	Each Information Entity in the IE_OBJECT list contains a
**	description of that Information Entity, a flag which indicates
**	if it is optional or required,  and a flag indicating if the
**	Information Entity is complete, incomplete or missing. This
**	function does not fill in the list of Modules in each Information
**	Entity structure. The IE_OBJECT is tagged "complete" if all
**	required Information Entities are present.
**
** Parameter Dictionary:
**	dcmObject: Address of caller's pointer to the input DICOM object.
**	ieObject:  Address of caller's pointer to an IE_OBJECT. This
**		   function allocates an IE_OBJECT structure and places
**		   the address of the structure in the caller's pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_OBJECTINCOMPLETE
**	IE_ILLEGALDCMOBJECT
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality of the DCM object.
**	Allocate memory for the IE_OBJECT and fill in the structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**	modality.
**	While( there are unexamined IE on the IE_FIELDS table) {
**		while(there are unexamined modules in the MOD_FIELDS table) {
**			Check all the attributes in the module.
**		   If at least one of the attributes(any type) is present,
**			then set ieExists flag to TRUE.
**		   If there are required(type1, type2) attributes
**			missing, then set ieIncomplete and objectIncomplete
**			flags to TRUE.
**		   }
**		If ieExists is TRUE, push this IE into the ieList in the
**		IE_OBJECT structure.
**		If ieIncomplete is TRUE, push IE_IEINCOMPLETE into the
**		error stack.
**		}
**	If objectIncomplete is TRUE, return IE_OBJECTINCOMPLETE.
**	Else return IE_NORMAL(object is complete).
**
*/

CONDITION
IE_ExamineObject(DCM_OBJECT ** dcmObject, IE_OBJECT ** ieObject)
{
    CONDITION				cond;						/* Return value from DUL and ACR routines */
	CTNBOOLEAN				flag;						/* Return value of findElement routine */
	CTNBOOLEAN				objectIncomplete = FALSE;	/* Flag indicates the object is incomplete */
	CTNBOOLEAN				ieIncomplete = FALSE;		/* Flag which indicates an IE is incomplete */
	CTNBOOLEAN				ieExists = FALSE;			/* Flag which indicates that an IE exists */
	DCM_ELEMENT				element;					/* Handle to DCM_ELEMENT */
	TAGS					* tags_tbl;					/* Handle to table of TAGS */
	U32						length;						/* Length of the data field of DCM_ELEMENT */
	IE_OBJECT				* ie_object = NULL;			/* Handle to the IE_OBJECT */
	IE_INFORMATIONENTITY	* ie_ie;					/* Handle to the IE_INFORMATIONENTITY */
	SOP_CLASS				SOPClass;					/* Return value of the findModality routine */
	IE_FIELDS				* table;					/* Handle to the IE_FIELDS table */
	char					*UID;						/* The SOPClassUID of the DCM object */
    int						size;						/* Size of the table */
    int						loop, k, j;					/* Iteration variables */

    /*
     * Find the SOPClassUID of the information object.
     */
    UID = findSOPClassUID(*dcmObject);
    /*
     * If the UID is not there, find the modality of the information object.
     */
    if (UID == NULL){
    	SOPClass = findModality(*dcmObject);
    }else{
    	SOPClass = lookupModality(UID);
    }
    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL) return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_ExamineObject");
    /*
     * Allocate memory for the IE_OBJECT, fill in the structure.
     */
    ie_object = malloc(sizeof(IE_OBJECT));
    if (ie_object == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ExamineObject");
    ie_object->structureType = IE_K_INFORMATIONOBJECT;
    ie_object->status = IE_COMPLETE;
    ie_object->ieList = LST_Create();
    if (ie_object->ieList == NULL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineObject");
    /*
     * Get the handle to the IE_FIELDS table.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;

    /* Fill in the classUID and objectDescription fields */
    strcpy(ie_object->classUID, Images[loop].classUID);
    strcpy(ie_object->objectDescription, Images[loop].obj_description);

    /*
     * Check all the attributes in an IE, if not all the attributes are
     * present, then push this IE into the information entity list in the
     * IE_OBJECT structure and push IE_INCOMPLETEIE warning into the error
     * stack. Repeat this procedure until all the IEs are examined.
     */
    for (loop = 0; loop < size; loop++) {
    	for (k = 0; k < table[loop].size_tbl; k++) {
    		tags_tbl = table[loop].mod_fields[k].tag_tbl;
    		for (j = 0; j < table[loop].mod_fields[k].size_tbl; j++) {
    			flag = findElement((*dcmObject), tags_tbl[j].tag, &element);
    			if (flag && (element.d.ot != NULL)) {
    				free(element.d.ot);
    				element.d.ot = NULL;
    			}
    			/*
    			 * Arrribute exists in the DCM object, this IE exists.
    			 */
    			if (flag){
    				ieExists = TRUE;
    				/*
    				 * Could not find the value of this attribute, and it's of
    				 * IE_K_TYPE1 which means that it's a violation not to have
    				 * value field, set ieIncomplete to TRUE.
    				 */
    			}else if (!flag && (tags_tbl[j].requirement == IE_K_TYPE1) && (table[loop].requirement == IE_K_REQUIRED) && (table[loop].mod_fields[k].requirement == IE_K_REQUIRED)) {
    				objectIncomplete = TRUE;
    				ieIncomplete = TRUE;
    			}
    			/*
    			 * Could not find the value of this attribute, and it's of
    			 * IE_K_TYPE2 which means that it's ok to have no value
    			 * field,
    			 */
    			else if (!flag && tags_tbl[j].requirement == IE_K_TYPE2) {
    				cond = DCM_GetElementSize(dcmObject, tags_tbl[j].tag, &length);
    				/*
    				 * The attribute doesn't exists at all, set ieIncomplete
    				 * to TRUE.
    				 */
    				if (cond != DCM_NORMAL) {
    					if ((table[loop].requirement == IE_K_REQUIRED) && (table[loop].mod_fields[k].requirement == IE_K_REQUIRED)) {
    						objectIncomplete = TRUE;
    						ieIncomplete = TRUE;
    					}
    				}else{
    					/*
    					 * The attribute tag exists in the object which is
    					 * OK.
    					 */
    					ieExists = TRUE;
    				}
    			}else{
    				/* Optional attribute, pop the error cond from the stack */
    				cond = COND_PopCondition(FALSE);
    			}
    		}			/* finish checking one module */
    	}			/* finish checking one IE */

	/*
	 * Push warning message into the error stack.
	 */
	ie_ie = malloc(sizeof(IE_INFORMATIONENTITY));
	if (ie_ie == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ExamineObject");
	ie_ie->structureType = IE_K_INFORMATIONENTITY;
	ie_ie->ieType = table[loop].type;
	strcpy(ie_ie->ieDescription, table[loop].ie_description);
	ie_ie->requirement = table[loop].requirement;
	if (!ieExists) ie_ie->status = IE_MISSING;
	if (ieExists && ieIncomplete) ie_ie->status = IE_INCOMPLETE;
	if (ieExists && !ieIncomplete) ie_ie->status = IE_COMPLETE;

	/*
	 * If this IE is missing, and it is not mandatory, we don't need to
	 * push it into the list.
	 */
	if ((ie_ie->requirement != IE_K_REQUIRED) && (ie_ie->status == IE_MISSING)){
	    free(ie_ie);
	}else{			/* Push this IE into the list if it is not the above case */
	    cond = LST_Enqueue(&(ie_object->ieList), ie_ie);
	    if (cond != LST_NORMAL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineObject");
	    if (ieIncomplete) cond = COND_PushCondition(IE_IEINCOMPLETE, IE_Message(IE_IEINCOMPLETE), table[loop].ie_description);
	    ieIncomplete = FALSE;
	    ieExists = FALSE;
	}
    }
    if (objectIncomplete) ie_object->status = IE_INCOMPLETE;
    *ieObject = ie_object;

    /*
     * Push Object Incomplete warning into the stack if ieIncomplete is TRUE.
     */
    if (objectIncomplete){
    	return COND_PushCondition(IE_OBJECTINCOMPLETE, IE_Message(IE_OBJECTINCOMPLETE), "IE_ExamineObject");
    }else{
    	return IE_NORMAL;
    }
}

/* IE_ExamineInformationEntity
**
** Purpose:
**	IE_ExamineInformationEntity examines one Information Entity
**	in a DICOM object and creates an IE_INFORMATIONENTITY structure
** 	defined by the IE facility. The structure contains a description
**	of the Information Entity, a flag indicating if the Information
**	Entity is required or optional and a list of Modules which should
**	be present in this Information Entity.
**
**	Each Module in the IE_INFORMATIONENTITY list contains a description
**	of that Module, a flag indicating if it is optional or required,
**	and a flag which indicates if the Module is complete, incomplete, or
**	missing. The function does not fill in the list of Attributes in
**	each Module structure. The IE_INFORMATIONENTITY structure is tagged
**	"complete" if all required Modules are complete.
**
**	The ieType argument is used to specify the type of the Information
**	Entity the caller wants to examine. It should take on values such
**	as IE_PATIENTIE, IE_STUDYIE, IE_SERIESIE, IE_FRAMEOFREFERENCEIE,
**	IE_EQUIPMENTIE, IE_IMAGEIE, IE_OVERLAYIE and IE_CURVEIE.
**
** Parameter Dictionary:
**	dcmObject: Address of caller's pointer to the input DICOM object.
**	ieType:    Type of the particular Information Entity which the
**		   caller wants to examine.
**	ieEntity:  Address of caller's pointer to an IE_INFORMATIONENTITY.
**		   This function allocates an IE_INFORMATIONENTITY
**		   structure and places the address of the structure in
**		   caller's pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**	IE_IEINCOMPLETE
**	IE_IEMISSING
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality of the DCM object.
**	Allocate memory  for the IE_INFORMATIONENTITY and fill in the
**	structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**      modality.
**	Find the handle to the MOD_FIELDS table(handle to an array of modules
**	which consists of an IE).
**	while( there are unexamined modules ) {
**	   Repeat {
**		If at least one of the attributes(any type) is present,
**		then set moduleExists flag to TRUE and ieMissing to FALSE.
**		If there are required(type1, type2) attributes missing,
**		then set ieIncomplete and moduleIncomplete flags to TRUE.
**		} until all attributes in a module are examined.
**	   If moduleIncomplete flag is TRUE, then push IE_MODULEINCOMPLETE
**	   warning message into the error stack.
**	   If moduleExists flag is TRUE, then push this module into the
**	   moduleList in the IE_INFORMATIONENTITY structure.
**	   }
**	If ieIncomplete flag is TRUE and ieMissing flag is FALSE, return
**	IE_IEINCOMPLETE.
**	Else if ieMissing is TRUE, return IE_IEMISSING.
**	Else return IE_NORMAL(IE is complete).
**
*/

CONDITION
IE_ExamineInformationEntity(DCM_OBJECT ** dcmObject, IE_IETYPE ieType, IE_INFORMATIONENTITY ** ieEntity)
{
    CONDITION			/* Return value from DUL and ACR routines */
	cond;
    DCM_ELEMENT			/* Handle to DCM element */
	element;
    U32				/* Length of the data of the DCM element */
	length;
    IE_INFORMATIONENTITY	/* Handle to the information entity */
	* ie_ie = NULL;
    IE_MODULE			/* Handle to the module */
	* ie_module;
    SOP_CLASS			/* Return value from findModality routine */
	SOPClass;
    IE_FIELDS			/* Handle to the IE_FIELDS table */
	* table;
    CTNBOOLEAN			/* Return value of the findElement */
	flag;
    CTNBOOLEAN			/* Flag which indicates if IE is missing */
	ieMissing = TRUE;
    CTNBOOLEAN			/* Flag which indicates IE is incomplete */
	ieIncomplete = FALSE;
    CTNBOOLEAN			/* Flag which indicates a module exists */
	moduleExists = FALSE;
    CTNBOOLEAN			/* Flag indicates a module is incomplete */
	moduleIncomplete = FALSE;
    MOD_FIELDS			/* Handle to the MOD_FIELDS table */
	* modFields_tbl;
    TAGS			/* Handle to the TAGS table */
	* tags_tbl;
    char			/* SOPClassUID of the DCM object */
       *UID;
    int				/* Size of the table */
        size;
    int				/* Iteration variables */
        loop,
        k;


    /*
     * Find the SOPClassUID of the information object.
     */
    UID = findSOPClassUID(*dcmObject);
    /*
     * If the UID is not there, find the modality of the information object.
     */
    if (UID == NULL)
	SOPClass = findModality(*dcmObject);
    else
	SOPClass = lookupModality(UID);

    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL)
	return COND_PushCondition(IE_ILLEGALDCMOBJECT,
	    IE_Message(IE_ILLEGALDCMOBJECT), "IE_ExamineInformationEntity");
    /*
     * Allocate memory for the IE_INFORMATIONENTITY, fill in the structure.
     */
    ie_ie = malloc(sizeof(IE_INFORMATIONENTITY));
    if (ie_ie == NULL)		/* malloc failed, return... */
	return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ExamineInformationEntity");
    ie_ie->structureType = IE_K_INFORMATIONENTITY;
    ie_ie->ieType = ieType;
    ie_ie->status = IE_COMPLETE;
    ie_ie->moduleList = LST_Create();
    if (ie_ie->moduleList == NULL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineInformationEntity");
    /*
     * Find the handle to the IE_FIELDS table for the corresponding modality.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;

    /*
     * Find the handle to the MOD_FIELDS table for the corresponding
     * information entity.
     */
    loop = 0;
    while (loop < size && ieType != table[loop].type)
	loop++;

    modFields_tbl = table[loop].mod_fields;
    size = table[loop].size_tbl;
    /*
     * Fill in the requirement, and description fields.
     */
    strcpy(ie_ie->ieDescription, table[loop].ie_description);
    ie_ie->requirement = table[loop].requirement;
    /*
     * Examine all the modules within the information entity. If all
     * attributes in a module are present in the DCM object, then push this
     * module into the module list in the IE_INFORMATIONENTITY structure,
     * repeat this procedure until all the modules are examined.
     */
    for (loop = 0; loop < size; loop++) {
	tags_tbl = modFields_tbl[loop].tag_tbl;
	for (k = 0; k < modFields_tbl[loop].size_tbl; k++) {
	    flag = findElement((*dcmObject), tags_tbl[k].tag, &element);
	    if (flag && (element.d.ot != NULL)) {
		free(element.d.ot);
		element.d.ot = NULL;
	    }
	    /*
	     * Arrribute exists in the DCM object, this IE exists.
	     */
	    if (flag) {
		moduleExists = TRUE;
		ieMissing = FALSE;
	    }
	    /*
	     * Could not find the value of this attribute, and it's of
	     * IE_K_TYPE1 which means that it's a violation not to have value
	     * field, set ieIncomplete to TRUE.
	     */
	    else if (!flag && (tags_tbl[k].requirement == IE_K_TYPE1) && (modFields_tbl[loop].requirement == IE_K_REQUIRED)) {
		ieIncomplete = TRUE;
		moduleIncomplete = TRUE;
	    }
	    /*
	     * Could not find the value of this attribute, and it's of
	     * IE_K_TYPE2 which means that it's ok to have no value field,
	     */
	    else if (!flag && tags_tbl[k].requirement == IE_K_TYPE2) {
		cond = DCM_GetElementSize(dcmObject, tags_tbl[k].tag, &length);
		/*
		 * The attribute doesn't exists at all, set ieIncomplete to
		 * TRUE.
		 */
		if (cond != DCM_NORMAL) {
		    if (modFields_tbl[loop].requirement == IE_K_REQUIRED) {
			ieIncomplete = TRUE;
			moduleIncomplete = TRUE;
		    }
		} else {
		    /*
		     * The attribute tag exists in the object which is OK.
		     */
		    moduleExists = TRUE;
		    ieMissing = FALSE;
		}
	    } else		/* Optional attribute, pop the error cond off
				 * the stack */
		cond = COND_PopCondition(FALSE);
	}			/* Finish checking one module */

	ie_module = malloc(sizeof(IE_MODULE));
	if (ie_module == NULL)
	    return COND_PushCondition(IE_MALLOCFAILURE, IE_Message
			 (IE_MALLOCFAILURE), "IE_ExamineInformationEntity");
	ie_module->structureType = IE_K_MODULE;
	ie_module->moduleType = modFields_tbl[loop].type;
	strcpy(ie_module->moduleDescription,
	       modFields_tbl[loop].mod_description);
	ie_module->requirement = modFields_tbl[loop].requirement;
	if (!moduleExists) ie_module->status = IE_MISSING;
	if (moduleExists && moduleIncomplete) ie_module->status = IE_INCOMPLETE;
	if (moduleExists && !moduleIncomplete) ie_module->status = IE_COMPLETE;
	/*
	 * If this Module is missing, and it isn't madatory, we don't need to
	 * push it into the list.
	 */
	if ((ie_module->requirement != IE_K_REQUIRED) && (ie_module->status == IE_MISSING))
	    free(ie_module);
	else {			/* Push it into the list if it isn't the
				 * above vase */
	    cond = LST_Enqueue(&(ie_ie->moduleList), ie_module);
	    if (cond != LST_NORMAL)
		return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineInformationEntity");
	    if (moduleIncomplete) cond = COND_PushCondition(IE_MODULEINCOMPLETE, IE_Message(IE_MODULEINCOMPLETE), modFields_tbl[loop].mod_description);
	    moduleIncomplete = FALSE;	/* reset the flag for later use */
	    moduleExists = FALSE;	/* reset the flag for later use */
	}
    }

    if (ieIncomplete && !ieMissing) {
	ie_ie->status = IE_INCOMPLETE;
	*ieEntity = ie_ie;
	return COND_PushCondition(IE_IEINCOMPLETE, IE_Message(IE_IEINCOMPLETE), ie_ie->ieDescription);
    } else if (ieMissing) {
	ie_ie->status = IE_MISSING;
	*ieEntity = ie_ie;
	return COND_PushCondition(IE_IEMISSING, IE_Message(IE_IEMISSING), ie_ie->ieDescription);
    } else {
	*ieEntity = ie_ie;
	return IE_NORMAL;
    }
}

/* IE_ExamineModule
**
** Purpose:
**	IE_ExamineModule examines one Module in a DICOM object and
**	creates an IE_MODULE structure defined by the IE facility.
**	The structure contains a description of the Module, a flag
**	indicating if this Module is required or optional and a list
**	of Attributes which should be present in the Module.
**
**	Each Attribute in the IE_MODULE list contains a DCM_ELEMENT
**	structure, a flag which indicates if this Attribute is optional
**	or required, and a flag indicating if this attribute is missing
**	or complete. The IE_MODULE structure is tagged "complete" if all
**	required Attributes are present.
**
**	The ieType argument is used to specify the type of the Information
**	Entity which contains the module the caller wants to examine.
**
**	The moduleType argument is used to specify the type of Module the
**	caller wants to examine.
**
** Parameter Dictionary:
**	dcmObject:  Address of caller's pointer to the input DICOM object.
**	ieType:     Type of the Information Entity which contains the
**		    Module the caller wants to examine.
**	moduleType: Type of the Module the caller wants to examine.
**	ieModule:   Address of caller's pointer to an IE_MODULE. This
**		    function allocates an IE_MODULE structure and places
**		    the address of the structure in caller's pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**	IE_MODULEINCOMPLETE
**	IE_MODULEMISSING
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality of the DCM object.
**	Allocate memory for the IE_MODULE and fill in the structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**      modality.
**	Find the handle to the module tag table corresponding to the
**	module type.
**	while( there are unexamined attributes ) {
**		Examine each attribute in the module.
**		If the attribute(any type) is found, set the attrExists
**		flag to TRUE and modMissing flag to FALSE.
**		If any required(type1, type2) attribute is missing, set
**		modIncomplete flag to TRUE.
**		If attrExists flag is TRUE, then push this attribute into
**		the attributeList in the IE_MODULE structure.
**		}
**	If modIncomplete flag is TRUE and modMissing flag is FALSE, return
**	IE_MODULEINCOMPLETE.
**	Else if modMissing flag is TRUE, return IE_MODULEMISSING.
**	Else return IE_NORMAL(the module is complete).
**
*/

CONDITION
IE_ExamineModule(DCM_OBJECT ** dcmObject, IE_IETYPE ieType, IE_MODULETYPE moduleType, IE_MODULE ** ieModule)
{
    CONDITION			/* Return value from DUL and ACR routines */
	cond;
    CTNBOOLEAN			/* Return value of findElement routine */
	flag;
    CTNBOOLEAN			/* Flag indicates if the module is missing */
	modMissing = TRUE;
    CTNBOOLEAN			/* Flag indicates an module is incomplete */
	moduleIncomplete = FALSE;
    CTNBOOLEAN			/* Flag indicates if the attribute exists */
	attrExists = FALSE;
    U32				/* Length of the data field of DCM_ELEMENT */
	length;
    IE_MODULE			/* Handle to the IE_MODULE */
	* ie_module;
    IE_ATTRIBUTE		/* Handle to the IE_ATTRIBUTE */
	* ie_attr;
    IE_FIELDS			/* Handle to the IE_FIELDS table */
	* table;
    SOP_CLASS			/* Return value of the findModality */
	SOPClass;
    MOD_FIELDS			/* Handle to the MOD_FIELDS table */
	* modFields_tbl;
    TAGS			/* Handle to the TAGS table */
	* tag_tbl = NULL;
    char			/* SOPClassUID of the DCM object */
       *UID;
    int				/* Size of the table */
        size;
    int				/* Iteration variable */
        loop;

    /*
     * Find the SOPClassUID of the information object.
     */
    UID = findSOPClassUID(*dcmObject);
    /*
     * If the UID is not there, find the SOP Classof the information object.
     */
    if (UID == NULL)
    	SOPClass = findModality(*dcmObject);
    else
    	SOPClass = lookupModality(UID);

    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL)
	return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_ExamineModule");
    /*
     * Allocate memory for the IE_MODULE, fill in the sturcture.
     */
    ie_module = malloc(sizeof(IE_MODULE));
    if (ie_module == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ExamineModule");
    ie_module->structureType = IE_K_MODULE;
    ie_module->moduleType = moduleType;
    ie_module->status = IE_COMPLETE;
    ie_module->attributeList = LST_Create();
    if (ie_module->attributeList == NULL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineModule");
    /*
     * Get the handle to the IE_FIELDS table corresponding to the SOP Class.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;

    /*
     * Find the handle to the MOD_FIELDS table corresponding to the ieType.
     */
    loop = 0;
    while (loop < size && ieType != table[loop].type)
	loop++;
    modFields_tbl = table[loop].mod_fields;
    size = table[loop].size_tbl;
    /*
     * Find the handle to the TAGS table corresponding to the moduleType.
     */
    loop = 0;
    while (loop < size && moduleType != modFields_tbl[loop].type)
	loop++;
    tag_tbl = modFields_tbl[loop].tag_tbl;
    size = modFields_tbl[loop].size_tbl;

    /*
     * Fill in the requirement, and description fields.
     */
    strcpy(ie_module->moduleDescription, modFields_tbl[loop].mod_description);
    ie_module->requirement = modFields_tbl[loop].requirement;

    /*
     * Check all the attributes in the module, push each attribute into the
     * attributeList in the IE_MODULE  structure, repeat this procedure until
     * all the attributes are examined.
     */
    for (loop = 0; loop < size; loop++) {
	ie_attr = malloc(sizeof(IE_ATTRIBUTE));
	if (ie_attr == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ExamineModule");
	ie_attr->structureType = IE_K_ATTRIBUTE;
	ie_attr->requirement = tag_tbl[loop].requirement;
	flag = findElement(*dcmObject, tag_tbl[loop].tag, &(ie_attr->element));
	if (flag) {
	    attrExists = TRUE;
	    modMissing = FALSE;
	} else if (!flag && tag_tbl[loop].requirement == IE_K_TYPE1)
	    moduleIncomplete = TRUE;
	else if (!flag && tag_tbl[loop].requirement == IE_K_TYPE2) {
	    cond = DCM_GetElementSize(dcmObject, tag_tbl[loop].tag, &length);
	    if (cond == DCM_NORMAL) {
		modMissing = FALSE;
		attrExists = TRUE;
	    } else
		moduleIncomplete = TRUE;
	} else			/* optional attribute, pop the error cond off
				 * the stack */
	    cond = COND_PopCondition(FALSE);
	if (attrExists)
	    ie_attr->status = IE_COMPLETE;
	else
	    ie_attr->status = IE_MISSING;
	/*
	 * If this attribute is missing, and it is not of type1 and type2, we
	 * don't need to push it into the list.
	 */
	if ((ie_attr->status == IE_MISSING) && (ie_attr->requirement != IE_K_TYPE1 || ie_attr->requirement != IE_K_TYPE2))
	    free(ie_attr);
	else {
	    cond = LST_Enqueue(&(ie_module->attributeList), ie_attr);
	    if (cond != LST_NORMAL)
		return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ExamineModule");
	    attrExists = FALSE;
	}
    }

    if (moduleIncomplete && !modMissing) {
	ie_module->status = IE_INCOMPLETE;
	*ieModule = ie_module;
	return COND_PushCondition(IE_MODULEINCOMPLETE, IE_Message(IE_MODULEINCOMPLETE), ie_module->moduleDescription);
    } else if (modMissing) {
	ie_module->status = IE_MISSING;
	*ieModule = ie_module;
	return COND_PushCondition(IE_MODULEMISSING,	IE_Message(IE_MODULEMISSING), ie_module->moduleDescription);
    } else {
	*ieModule = ie_module;
	return IE_NORMAL;
    }
}

/* IE_ObjectRequirements
**
** Purpose:
**	IE_ObjectRequirements determines what Information Entities
**	are required for a DICOM information object belonging to an
**	SOP Class(such as MR, CT, Secondary Capture) and returns an
**	IE_OBJECT structure which contains those requirements. The
**	structure contains a description of the object and a list of
**	Information Entities required(mandatory) for the object.
**
**	Each Information Entity in the IE_OBJECT list contains a
**	description of this Information Entity, and a flag which
**	indicates it is required. This function does not fill in the
**	list of Modules in the IE_INFORMATIONENTITY structure.
**
**	This function is used to determine requirements for any image
**	belonging to an SOP Class as defined by the caller's UID argument.
**	This function does not examine a DICOM information object.
**
** Parameter Dictionary:
**	classUID: An ascii string which identifies the SOP Class of
**		  the DICOM information object.
**	object:	  Address of caller's pointer to an IE_OBJECT. This
**                function allocates an IE_OBJECT structure and places
**                the address of the structure in the caller's pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality according to classUID.
**	Allocate memory for the IE_OBJECT and fill in the structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**	modality.
**	Push each required IE into the ieList in the IE_OBJECT
**	structure.
**	Return IE_NORMAL.
**
*/

CONDITION
IE_ObjectRequirements(char *classUID,
		      IE_OBJECT ** object)
{
    CONDITION			/* Return value from DUL and ACR routines */
	cond;
    IE_OBJECT			/* Handle to the IE_OBJECT */
	* ie_object = NULL;
    IE_INFORMATIONENTITY	/* Handle to the IE_INFORMATIONENTITY */
	* ie_ie;
    SOP_CLASS			/* Return value of the findModality routine */
	SOPClass;
    IE_FIELDS			/* Handle to the IE_FIELDS table */
	* table;
    int				/* Size of the table */
        size;
    int				/* Iteration variables */
        loop;

    /*
     * Find out the SOP Class of the information object.
     */
    SOPClass = lookupModality(classUID);

    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL) return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_ObjectRequirements");
    /*
     * Allocate memory for the IE_OBJECT, fill in the structure.
     */
    ie_object = malloc(sizeof(IE_OBJECT));
    if (ie_object == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ObjectRequirements");
    ie_object->structureType = IE_K_INFORMATIONOBJECT;
    ie_object->ieList = LST_Create();
    if (ie_object->ieList == NULL)
	return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ObjectRequirements");
    /*
     * Get the handle to the table of the information entities.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;

    /* Fill in the classUID and objectDescription fields */
    strcpy(ie_object->classUID, Images[loop].classUID);
    strcpy(ie_object->objectDescription, Images[loop].obj_description);

    /*
     * Push every required IE into the information entity list in the
     * IE_OBJECT structure, repeat this procedure until all the required IEs
     * are examined.
     */
    for (loop = 0; loop < size; loop++) {
	if (table[loop].requirement == IE_K_REQUIRED) {
	    ie_ie = malloc(sizeof(IE_INFORMATIONENTITY));
	    if (ie_ie == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ObjectRequirements");
	    ie_ie->structureType = IE_K_INFORMATIONENTITY;
	    ie_ie->ieType = table[loop].type;
	    strcpy(ie_ie->ieDescription, table[loop].ie_description);
	    ie_ie->requirement = table[loop].requirement;
	    cond = LST_Enqueue(&(ie_object->ieList), ie_ie);

	    if (cond != LST_NORMAL)	return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ObjectRequirements");
	}
    }

    *object = ie_object;
    return IE_NORMAL;
}

/* IE_IERequirements
**
** Purpose:
**	IE_IERequirements determines which Modules are required for
**	an Information Entity in a DICOM object and returns an
**	IE_INFORMATIONENTITY structure which contains those requirements.
**	The structure contains a description of this Information Entity,
**	a flag indicating if it is required or optional and a list of
**	Modules required for this Information Entity.
**
**	Each Module in the IE_INFORMATIONENTITY list contains a description
**	of the Module and a flag which indicates it is required. This
**	function does not fill in the list of Attributes in each IE_MODULE
**	structure.
**
**	This function is used to determine requirements for an Information
**	Entity in an image belonging to the SOP Calss defined by the
**	caller's classUID argument.
**
**	The ieType argument is used to specify the type of the Information
**	Entity the caller wants to examine.
**
**	This function does not examine a DICOM information object.
**
** Parameter Dictionary:
**	classUID: An ascii string which identifies the SOP Class of the
**		  DICOM information object.
**	ieType:   Identifies the type of the Information Entity the
**		  caller wants to examine.
**	ieEntity: Address of caller's pointer to an IE_INFORMATIONENTITY.
**		  This function allocates an IE_INFORMATIONENTITY
**		  structure and places the address of the structure in
**		  the caller's pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality according to classUID.
**	Allocate memory for the IE_INFORMATIONENTITY and fill in the
**	structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**	modality.
**	Find the handle of the MOD_FIELDS table corresponding to the
**	ieType.
**	Push each required MODULE into the moduleList in the
**	IE_INFORMATIONENTITY structure.
**	Return IE_NORMAL.
**
*/

CONDITION
IE_IERequirements(char *classUID, IE_IETYPE ieType, IE_INFORMATIONENTITY ** ieEntity)
{
    CONDITION			/* Return value from DUL and ACR routines */
	cond;
    IE_INFORMATIONENTITY	/* Handle to the IE_INFORMATIONENTITY */
	* ie_ie = NULL;
    IE_MODULE			/* Handle to the IE_MODULE */
	* ie_module;
    SOP_CLASS			/* Return value of the findModality routine */
	SOPClass;
    IE_FIELDS			/* Handle to the IE_FIELDS table */
	* table;
    MOD_FIELDS			/* Handle to the MOD_FIELDS table */
	* modFields_tbl;
    int				/* Size of the table */
        size;
    int				/* Iteration variables */
        loop;

    /*
     * Find out the modality of the information object.
     */
    SOPClass = lookupModality(classUID);

    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL) return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_IERequirements");
    /*
     * Allocate memory for the IE_INFORMATIONENTITY, fill in the structure.
     */
    ie_ie = malloc(sizeof(IE_INFORMATIONENTITY));
    if (ie_ie == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_IERequirements");
    ie_ie->structureType = IE_K_INFORMATIONENTITY;
    ie_ie->ieType = ieType;
    ie_ie->moduleList = LST_Create();
    if (ie_ie->moduleList == NULL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_IERequirements");
    /*
     * Get the handle to the table of the information entities.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;

    /*
     * Find the handle to the MOD_FIELDS table corresponding to the ieType.
     */
    loop = 0;
    while (loop < size && ieType != table[loop].type)
	loop++;
    modFields_tbl = table[loop].mod_fields;
    size = table[loop].size_tbl;

    /*
     * Fill in the requirement, and description fields.
     */
    strcpy(ie_ie->ieDescription, table[loop].ie_description);
    ie_ie->requirement = table[loop].requirement;

    /*
     * Push every required module into the module list in the
     * IE_INFORMATIONENTITY structure, repeat this procedure until all the
     * required modules are examined.
     */
    for (loop = 0; loop < size; loop++) {
	if (modFields_tbl[loop].requirement == IE_K_REQUIRED) {
	    ie_module = malloc(sizeof(IE_MODULE));
	    if (ie_module == NULL)
		return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_IERequirements");
	    ie_module->structureType = IE_K_MODULE;
	    ie_module->moduleType = modFields_tbl[loop].type;
	    strcpy(ie_module->moduleDescription, modFields_tbl[loop].mod_description);
	    ie_module->requirement = modFields_tbl[loop].requirement;
	    cond = LST_Enqueue(&(ie_ie->moduleList), ie_module);
	    if (cond != LST_NORMAL)	return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_IERequirements");
	}
    }

    *ieEntity = ie_ie;
    return IE_NORMAL;
}

/* IE_ModuleRequirements
**
** Purpose:
**	IE_ModuleRequirements determines which Attributes are required
**	for a Module in a DICOM object and returns an IE_MODULE
**	structure which contains those requirements. The structure
**	contains a description of this Module, a flag indicating if it
**	is required or optional and a list of Attributes required
**	(type1, type2) for this Module.
**
**	Each Attribute in the IE_MODULE list contains a DCM_ELEMENT
**	structure, and a flag indicating if it is optional or required.
**	This function fills in the DCM_TAG field in the DCM_ELEMENT
**	structure, other fields of the structure are empty.
**
**	This function is used to determine requirements for a Module
**	in an image in the SOP Class defined by the caller's classUID
**	argument.
**
** 	The ieType argument is used to specify the type of the Information
**	Entity which contains the Module the caller wants to examine.
**
**	The moduleType argument is used to specify the type of Module
**	the caller wants to examine.
**
**	This function does not examine a DICOM Information Entity.
**
** Parameter Dictionary:
**	classUID:   A string which identifies the the type of the
**		    input DICOM object.
**	ieType:	    Identifies the type of the Information Entity.
**	moduleType: Identifies the type of the Module the caller
**		    wants to examine.
**	ieModule:   Address of caller's pointer to an IE_MODULE.
**	  	    This function allocates an IE_MODULE structure and
**		    places the address of the structure in the caller's
**		    pointer.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**	IE_LISTFAILURE
**	IE_MALLOCFAILURE
**
** Algorithm:
**	Find the modality according to classUID.
**	Allocate memory for the IE_MODULE and fill in the structure.
**	Find the handle of the IE_FIELDS table corresponding to the
**      modality.
**	Find the handle of the MOD_FIELDS table corresponding to the
**	moduleType.
**	Push each required attribute into the attributeList in the
**	IE_MODULE structure.
**	Return IE_NORMAL.
**
*/

CONDITION
IE_ModuleRequirements(char *classUID, IE_IETYPE ieType, IE_MODULETYPE moduleType, IE_MODULE ** ieModule)
{
    CONDITION			/* Return value from DUL and ACR routines */
	cond;
    IE_MODULE			/* Handle to the IE_MODULE */
	* ie_module = NULL;
    IE_ATTRIBUTE		/* Handle to the IE_ATTRIBUTE */
	* ie_attr;
    SOP_CLASS			/* Return value of the findModality routine */
	SOPClass;
    IE_FIELDS			/* Handle to the IE_FIELDS table */
	* table;
    MOD_FIELDS			/* Handle to the MOD_FIELDS table */
	* modFields_tbl;
    TAGS			/* Handle to the TAGS table */
	* tags_tbl;
    int				/* Size of the table */
        size;
    int				/* Iteration variables */
        loop;

    /*
     * Find out the SOP Class of the information object.
     */
    SOPClass = lookupModality(classUID);

    /*
     * Returns if it is not a legal DCM object.
     */
    if (SOPClass == ILLEGAL)
	return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_ModuleRequirements");
    /*
     * Allocate memory for the IE_INFORMATIONENTITY, fill in the structure.
     */
    ie_module = malloc(sizeof(IE_MODULE));
    if (ie_module == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ModuleRequirement");
    ie_module->structureType = IE_K_MODULE;
    ie_module->moduleType = moduleType;
    ie_module->attributeList = LST_Create();
    if (ie_module->attributeList == NULL) return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_ModuleRequirements");
    /*
     * Get the handle to the table of the information entities.
     */
    loop = 0;
    while (loop < (int) DIM_OF(Images) && SOPClass != Images[loop].SOPClass)
	loop++;
    table = Images[loop].fields;
    size = Images[loop].size_tbl;
    /*
     * Find the handle to the MOD_FIELDS table corresponding to the ieType.
     */
    loop = 0;
    while (loop < size && ieType != table[loop].type)
	loop++;
    modFields_tbl = table[loop].mod_fields;
    size = table[loop].size_tbl;

    /*
     * Find the handle to the TAGS table corresponding to the moduleType.
     */
    loop = 0;
    while (loop < size && moduleType != modFields_tbl[loop].type)
	loop++;
    tags_tbl = modFields_tbl[loop].tag_tbl;
    size = modFields_tbl[loop].size_tbl;

    /*
     * Fill in the requirement, and description fields.
     */
    strcpy(ie_module->moduleDescription, modFields_tbl[loop].mod_description);
    ie_module->requirement = modFields_tbl[loop].requirement;

    /*
     * Push every required attribute into the attribute list in the IE_MODULE
     * structure, repeat this procedure until all the required attributes are
     * examined.
     */
    for (loop = 0; loop < size; loop++) {
	/*
	 * Right now, I will check IE_K_TYPE1 and IE_K_TYPE2 for mandantory
	 * only. Treat IE_K_TYPE1C and IE_K_TYPE2C as optional.
	 */
	if (tags_tbl[loop].requirement == IE_K_TYPE1 ||
	    tags_tbl[loop].requirement == IE_K_TYPE2) {
	    ie_attr = malloc(sizeof(IE_ATTRIBUTE));
	    if (ie_attr == NULL) return COND_PushCondition(IE_MALLOCFAILURE, IE_Message(IE_MALLOCFAILURE), "IE_ModuleRequirements");
	    ie_attr->structureType = IE_K_ATTRIBUTE;
	    ie_attr->requirement = tags_tbl[loop].requirement;
	    ie_attr->element.tag = tags_tbl[loop].tag;
	    cond = LST_Enqueue(&(ie_module->attributeList), ie_attr);
	    if (cond != LST_NORMAL)	return COND_PushCondition(IE_LISTFAILURE, IE_Message(IE_LISTFAILURE), "IE_IERequirements");
	}
    }

    *ieModule = ie_module;
    return IE_NORMAL;
}

/* IE_Free
**
** Purpose:
**	IE_Free frees any structure that has been created by a function
**	in the IE_facility. This includes functions:
**		IE_ExamineObject
**		IE_ExamineInformationEntity
**		IE_ExamineModule
**		IE_ObjectRequirements
**		IE_IERequirements
**		IE_ModuleRequirements
**
**	The function determines the type of structure passed by the caller
**	and frees any lists contained in the structure. After the lists
**	are free, the function frees the structure itself.
**
**	The caller passes the address of a pointer to an IE structure.
**	After the structure is freed, this function destroys the caller's
**	reference to the structure by writing NULL into the caller's
**	pointer.
**
** Parameter Dictionary:
**	object: Caller's handle to the IE structure.
**
** Return Values:
**	IE_NORMAL
**	IE_ILLEGALDCMOBJECT
**
** Algorithm:
**	switch(the structure type)
**		free the memory allocated for each element on the list
**		(if there exists such a list) in the structure.
**		free the structure itself.
**	Destroy the list created by LST_Create(if there is a list).
**	Set the reference to the structure to NULL.
**	Return IE_NORMAL.
**
*/

CONDITION
IE_Free(void **object)
{
    CONDITION			/* Return value from ACR and DUL routines */
	cond;
    int        i, loop;
    LST_HEAD	* head;
    LST_NODE	* node;
    IE_ATTRIBUTE	* attribute;


    if (((IE_OBJECT *) * object)->structureType == IE_K_INFORMATIONOBJECT) {
	head = ((IE_OBJECT *) * object)->ieList;
	loop = LST_Count(&head);
	for (i = 0; i < loop; i++) {
	    node = LST_Pop(&head);
	    free(node);
	}
	cond = LST_Destroy(&head);
	if (cond == LST_NORMAL)
	    free(*object);
    } else if (((IE_INFORMATIONENTITY *) * object)->structureType == IE_K_INFORMATIONENTITY) {
	head = ((IE_INFORMATIONENTITY *) * object)->moduleList;
	loop = LST_Count(&head);
	for (i = 0; i < loop; i++) {
	    node = LST_Pop(&head);
	    free(node);
	}
	cond = LST_Destroy(&head);
	if (cond == LST_NORMAL)
	    free(*object);
    } else if (((IE_MODULE *) * object)->structureType == IE_K_MODULE) {
	head = ((IE_MODULE *) * object)->attributeList;
	loop = LST_Count(&head);
	for (i = 0; i < loop; i++) {
	    node = LST_Pop(&head);
	    free(node);
	}
	cond = LST_Destroy(&head);
	if (cond == LST_NORMAL)
	    free(*object);
    } else if (((IE_ATTRIBUTE *) * object)->structureType == IE_K_ATTRIBUTE) {
	attribute = (IE_ATTRIBUTE *) * object;
	if (attribute->element.d.ot != NULL) {
	    free(attribute->element.d.ot);
	    attribute->element.d.ot = NULL;
	}
	free(*object);
    } else
	return COND_PushCondition(IE_ILLEGALDCMOBJECT, IE_Message(IE_ILLEGALDCMOBJECT), "IE_Free");

    *object = NULL;
    return IE_NORMAL;
}

/* findElement
**
** Purpose:
**      This routine fills in the DCM_ELEMENT structure according to the
**	tag. Returns TRUE if the element is present in the object,
**	FALSE otherwise.
**
** Parameter Dictionary:
**	object:  Caller's handle to the DCM information object.
**      tag:	 Element tag.
**	element: Handle to the DCM_ELEMENT element returned to the caller.
**
** Return Values:
**	TRUE is returned if the element is found in the DCM object,
**	FALSE otherwise.
**
** Algorithm:
**      Call DCM_LookupElement() to find the representation of the
**      attribute according to the tag.
**      If the representaion is found,
**              then length field is filled and the memory to hold the
**              data is assigned.
**      else
**              Call DCM_GetElementSize(), if the return condition
**              is DCM_NORMAL, then return TRUE.
**      Call DCM_GetElementValue() to retrieve the data for the
**      attribute.
**
*/

static CTNBOOLEAN
findElement(DCM_OBJECT * object, DCM_TAG tag, DCM_ELEMENT * element)
{
    CONDITION			/* Return value from DUL and ACR routines */
    cond;
    DCM_ELEMENT			/* Handle to the dicom data element */
	dcm_element;
    void			/* Context variable used by
				 * DCM_GetElementValue */
       *ctx;
    CTNBOOLEAN			/* See Return Values in the header */
	flag = TRUE;
    CTNBOOLEAN			/* Flag indicates if value is of string type */
	isString = FALSE;

    /*
     * Find the group and element fields in the tag.
     */
    dcm_element.tag = tag;
    dcm_element.d.ot = NULL;
    element->tag = dcm_element.tag;
    element->d.ot = NULL;
    /*
     * Find the representation of the data in the element.
     */
    cond = DCM_LookupElement(&dcm_element);
    if (cond != DCM_NORMAL) {
	cond = COND_PopCondition(FALSE);
	return FALSE;
    }
    /*
     * Fill the length field and assign the memory for the data.
     */
/*lint -e527  Turn off checking for unreachable statements (return/break) */
    switch (dcm_element.representation) {
    case DCM_LO:		/* Long string */
    case DCM_UI:		/* UID */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_DA:		/* Date */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_CS:		/* Control string */
    case DCM_TM:		/* Time */
    case DCM_SH:		/* Short string */
    case DCM_DS:		/* Decimal string */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_IS:		/* Integer string */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_ST:		/* Short text */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_LT:		/* Long text */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_PN:		/* Person name */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_AS:		/* Age string */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else {
	    dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
	    isString = TRUE;
	}
	break;
    case DCM_SS:		/* Signed short */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else
	    dcm_element.d.ss = malloc((dcm_element.length + 1) * sizeof(char));
	break;
    case DCM_SL:		/* signed long */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else
	    dcm_element.d.sl = malloc((dcm_element.length + 1) * sizeof(char));
	break;
    case DCM_US:		/* unsigned short */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else
	    dcm_element.d.us = malloc((dcm_element.length + 1) * sizeof(char));
	break;
    case DCM_AT:		/* Attribute Tag */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else
	    dcm_element.d.at = malloc((dcm_element.length + 1) * sizeof(char));
	break;
    case DCM_UL:		/* unsigned long */
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond != DCM_NORMAL) {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	} else
	    dcm_element.d.ul = malloc((dcm_element.length + 1) * sizeof(char));
	break;
    case DCM_SQ:		/* Sequence of items */
	/* we cannot get DCM_SQ value. So we return with false */
	*element = dcm_element;
	flag = FALSE;
	return flag;
	break;
    default:
	cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
	if (cond == DCM_NORMAL)
	    return flag;
	else {
	    cond = COND_PopCondition(FALSE);
	    return FALSE;
	}
	break;
    }
/*lint +e527 */
    /*
     * The following is necessary when made the first call to
     * DCM_GetElementValue().
     */
    ctx = NULL;
    cond = DCM_GetElementValue(&object, &dcm_element, &(dcm_element.length), &ctx);
    if (cond != DCM_NORMAL) {	/* check for error */
	/*
	 * the attribute is not found
	 */
	flag = FALSE;
	cond = COND_PopCondition(FALSE);
    } else {
	if (isString) dcm_element.d.string[dcm_element.length] = '\0';
	*element = dcm_element;
    }
    return flag;
}


/* findModality
**
** Purpose:
**      This routine examines the information object to find out its
**	modality. The modality is returned to the caller.
**
** Parameter Dictionary:
**      object: Caller's handle to the DCM information object.
**
** Return Values:
**      The modality of the information object is returned if the data field
**	of attribute(0008, 0060) is listed on the modality_tbl, ILLEGAL is
**	returned otherwise.
**
** Algorithm:
**
*/

static SOP_CLASS
findModality(DCM_OBJECT * object)
{
    CONDITION			/* Return values from DUL and ACR routines */
    cond;
    DCM_ELEMENT			/* Handle to the DCM_ELEMENT */
	element;
    int				/* Size of the table */
        size;
    int				/* Iteration variable */
        loop = 0;
    char
        modality[DICOM_CS_LENGTH + 1];

    /*
     * Examine the modality attribute in the information object.
     */
    element.tag = DCM_IDMODALITY;
    cond = DCM_LookupElement(&element);
    element.length = sizeof(modality);
    element.d.string = modality;
    /*
     * Get the value of the attribute which is themodality of the information
     * object.
     */
    cond = DCM_ParseObject(&object, &element, 1, NULL, 0, NULL);

    if (cond != DCM_NORMAL)	return ILLEGAL;

    size = (int) DIM_OF(modality_tbl);
    while (loop < size && strcmp(element.d.string, modality_tbl[loop].key))
	loop++;

    if (loop < size)
	return modality_tbl[loop].SOPClass;
    else
	return ILLEGAL;
}

/* findSOPClassUID
**
** Purpose:
**      This routine examines the information object to find out its
**	SOPCalssUID. The UID is returned to the caller.
**
** Parameter Dictionary:
**      object: Caller's handle to the DCM information object.
**
** Return Values:
**      The SOPClassUID of the information object is returned if the
**	data field of attribute (0008, 0016) is listed on the
**      modality_tbl, NULL is returned otherwise.
**
** Algorithm:
**
*/

static char *
findSOPClassUID(DCM_OBJECT * object)
{
    CONDITION			/* Return values from DUL and ACR routines */
    cond;
    DCM_ELEMENT			/* Handle to the DCM_ELEMENT */
	element;
    int				/* Size of the table */
        size;
    int				/* Iteration variable */
        loop = 0;
    char
        uidString[DICOM_UI_LENGTH + 1];

    /*
     * Examine the modality attribute in the information object.
     */
    element.tag = DCM_IDSOPCLASSUID;
    cond = DCM_LookupElement(&element);
    element.length = sizeof(uidString);
    element.d.string = uidString;
    /*
     * Get the value of the attribute which is themodality of the information
     * object.
     */
    cond = DCM_ParseObject(&object, &element, 1, NULL, 0, NULL);

    if (cond != DCM_NORMAL)	return NULL;

    size = (int) DIM_OF(modality_tbl);
    while (loop < size && strcmp(element.d.string, modality_tbl[loop].classUID))
	loop++;

    if (loop < size)
	return modality_tbl[loop].classUID;
    else
	return NULL;
}

/* lookupModality
**
** Purpose:
**      This routine finds the modality in the modality table according
**	to its classUID. The modality is returned.
**
** Parameter Dictionary:
**      classUID: String to identify the type of the DCM information object.
**
** Return Values:
**      The modality is returned if the classUID is on the list, ILLEGAL is
**	returned otherwise.
**
** Algorithm:
**
*/

static SOP_CLASS
lookupModality(char *classUID)
{
    int				/* Iteration variable */
        loop = 0;

    while (loop < (int) DIM_OF(modality_tbl) && strcmp(classUID, modality_tbl[loop].classUID))
	loop++;

    if (loop < (int) DIM_OF(modality_tbl))
	return modality_tbl[loop].SOPClass;
    else
	return ILLEGAL;
}
