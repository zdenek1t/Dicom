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
**                              DICOM 93
**                   Electronic Radiology Laboratory
**                 Mallinckrodt Institute of Radiology
**              Washington University School of Medicine
**
** Module Name(s):	main
**			findAsciiC
**			findAsciiR
**			findAsciiS
**			printObject
**			printIE
**			printModule
**			printIEAttribute
**			printElement
**			findElement
**			usageerror
** Author, Date:	Pei Weng, 9-June-1993
** Intent:		This program opens an image file which contains
**			a DICOM V3 object, dumps the contents of the file
**			into the standard output in hierarchical order.
**			Then the program will find all the required
**			Information Entitites, required Modules which form
**			the Information Entity and all the missing required
**			atributes(if exist) within the module and print the
**			above information to the standard output.
**
** Last Update:         $Author: smm $, $Date: 1998/08/03 21:27:19 $
** Source File:         $RCSfile: dcm_verify.c,v $
** Revision:            $Revision: 1.17 $
** Status:              $State: Exp $
*/

static char rcsid[] = "$Revision: 1.17 $ $RCSfile: dcm_verify.c,v $";

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "../dicom_lib/dicom/dicom.h"
#include "../dicom_lib/thread/ctnthread.h"
#include "../dicom_lib/condition/condition.h"
#include "../dicom_lib/lst/lst.h"
#include "../dicom_lib/objects/dicom_objects.h"
#include "../dicom_lib/messages/dicom_messages.h"
#include "../dicom_lib/info_entity/dicom_ie.h"
#include "../dicom_lib/uid/dicom_uids.h"
#include "dcm_verify.h"

/* Prototypes for internal functions
 */
static void usageerror();
static char *findAsciiC(STATUSID * table, IE_STATUS id, int loop);
static char *findAsciiR(REQUIREMENTID * table, IE_IEREQUIREMENT id, int loop);
static char *findAsciiS(STRUCTUREID * table, IE_STRUCTURETYPE id, int loop);
static void printObject(IE_OBJECT * ieObject);
static void printIE(IE_INFORMATIONENTITY * ieIE);
static void printModule(IE_MODULE * ieModule);
static void printIEAttribute(IE_ATTRIBUTE * ieAttr);
static void printElement(DCM_ELEMENT * element);

static CTNBOOLEAN
findElement(DCM_OBJECT * object, DCM_TAG tag, DCM_ELEMENT * element);

int
main(int argc, char **argv)
{
    CONDITION				cond;								/* Return value from DUL and ACR routines */
    DCM_OBJECT				* object;							/* Handle to the information object */
    DCM_ELEMENT				element;							/* Handle to the DCM_ELEMENT */
    IE_OBJECT				* ieObject;							/* Handle to the IE_OBJECT object */
    IE_INFORMATIONENTITY	* ieIE, *ie_node;					/* Handle to IE_INFORMATIONENTITY */
    LST_HEAD				* ie_head, *mod_head, *attr_head;	/* Handle to the LST_HEAD */
    IE_MODULE				* ieModule, *mod_node;				/* Handle to IE_MODULE */
    IE_ATTRIBUTE			* attr_node;						/* Handle to IE_ATTRIBUTE */
    CTNBOOLEAN				verbose = FALSE;					/* For debugging purpose */
    CTNBOOLEAN				flag;								/* Return value from findElement routine */
    unsigned long			options = DCM_ORDERLITTLEENDIAN;	/* Byte order in data streams */
    char					*file;								/* The image file name */
    char					UID[90];							/* The SOP Class UID of the image file */
    U32						length;								/* Length of the data field of DCM_ELEMENT */
    int						ie_loop, mod_loop, attr_loop, j, k, i;/* Iteration variables */


    while (--argc > 0 && (*++argv)[0] == '-') {
    	switch (*(argv[0] + 1)) {
			case 'v':
						verbose = TRUE;
						break;
			case 'b':
						options &= ~DCM_ORDERMASK;
						options |= DCM_ORDERBIGENDIAN;
						break;
			case 't':
						options &= ~DCM_FILEFORMATMASK;
						options |= DCM_PART10FILE;
						break;
			default:
						break;
    	}
    }

    if (argc < 1) usageerror();

    file = *argv;
    THR_Init();
    DCM_Debug(verbose);

    /* Open a DICOM object file and put the contents into the memory represented by the information object. */
    cond = DCM_OpenFile(file, options, &object);


    if (cond != DCM_NORMAL && ((options & DCM_PART10FILE) == 0)) {
    	COND_DumpConditions();
    	(void) DCM_CloseObject(&object);
    	(void) COND_PopCondition(TRUE);
    	fprintf(stderr, "Could not open %s as expected.  Trying Part 10 format.\n", file);
    	cond = DCM_OpenFile(file, options | DCM_PART10FILE, &object);
    }

    if (cond != DCM_NORMAL) {
     	COND_DumpConditions();
     	THR_Shutdown();
     	return 1;
    }else{
    	printf("file is successfully opened!\n");
    	/* Call IE_ExamineObject to examine this DCM object. */
    	cond = IE_ExamineObject(&object, &ieObject);
    	if (cond == IE_ILLEGALDCMOBJECT || cond == IE_LISTFAILURE || cond == IE_MALLOCFAILURE){
    		COND_DumpConditions();
    	}else{
    		/* Print the IE_OBJECT object.  */
    		strcpy(UID, ieObject->classUID);
    		printObject(ieObject);

    		/* Examine each IE on the list.  */
    		ie_head = ieObject->ieList;
    		ie_loop = LST_Count(&ie_head);

    		for (i = 0; i < ie_loop; i++) {
    			ie_node = LST_Pop(&ie_head);
    			cond = IE_ExamineInformationEntity(&object, ie_node->ieType, &ieIE);

    			/* Print each IE_IE. */
    			printIE(ieIE);

    			/* Examine each module on the list. */
    			mod_head = ieIE->moduleList;
    			mod_loop = LST_Count(&mod_head);

    			for (k = 0; k < mod_loop; k++) {
    				mod_node = LST_Pop(&mod_head);
    				cond = IE_ExamineModule(&object, ieIE->ieType, mod_node->moduleType, &ieModule);
    				printModule(ieModule);

    				/* Print each IE_ATTRIBUTE.  */
    				attr_head = ieModule->attributeList;
    				attr_loop = LST_Count(&attr_head);

    				for (j = 0; j < attr_loop; j++) {
    					attr_node = LST_Pop(&attr_head);
    					printIEAttribute(attr_node);
    					free(attr_node);
    				}
    				free(mod_node);

    				cond = IE_Free((void **) &ieModule);
    			}
    			free(ie_node);

    			cond = IE_Free((void **) &ieIE);
    		}
    		cond = IE_Free((void **) &ieObject);

	    /* Check to see the status of the Information Entities. */
	    cond = IE_ExamineObject(&object, &ieObject);
	    printf("\n%s requirements:\n", ieObject->objectDescription);
	    ie_head = ieObject->ieList;
	    ie_loop = LST_Count(&ie_head);

	    for (i = 0; i < ie_loop; i++) {
	    	ie_node = LST_Pop(&ie_head);
	    	if (ie_node->requirement == IE_K_REQUIRED) printIE(ie_node);
	    	free(ie_node);
	    }

	    cond = IE_Free((void **) &ieObject);

	    /* Check to see the status of the Information Entity and status of the Modules within them. */
	    cond = IE_ExamineObject(&object, &ieObject);
	    printf("\n%s requirements:\n", ieObject->objectDescription);
	    ie_head = ieObject->ieList;
	    ie_loop = LST_Count(&ie_head);

	    for (i = 0; i < ie_loop; i++) {
	    	ie_node = LST_Pop(&ie_head);
	    	cond = IE_ExamineInformationEntity(&object, ie_node->ieType, &ieIE);
	    	if (ie_node->requirement == IE_K_REQUIRED) {
	    		printf("\n");
	    		printIE(ieIE);
	    		mod_head = ieIE->moduleList;
	    		mod_loop = LST_Count(&mod_head);

	    		for (k = 0; k < mod_loop; k++) {
	    			mod_node = LST_Pop(&mod_head);
	    			if (mod_node->requirement == IE_K_REQUIRED) printModule(mod_node);
	    			free(mod_node);
	    		}
	    	}
	    	free(ie_node);
	    	cond = IE_Free((void **) &ieIE);
	    }
	    cond = IE_Free((void **) &ieObject);

	    /* Check to see the missing attributes if there is any. */
	    cond = IE_ObjectRequirements(UID, &ieObject);
	    printf("\n  Missing required(type1 and type2) attributes: \n");
	    ie_head = ieObject->ieList;
	    ie_loop = LST_Count(&ie_head);

	    for (i = 0; i < ie_loop; i++) {
	    	ie_node = LST_Pop(&ie_head);
	    	cond = IE_IERequirements(UID, ie_node->ieType, &ieIE);
	    	mod_head = ieIE->moduleList;
	    	mod_loop = LST_Count(&mod_head);

	    	for (k = 0; k < mod_loop; k++) {
	    		mod_node = LST_Pop(&mod_head);
	    		cond = IE_ModuleRequirements(UID, ie_node->ieType, mod_node->moduleType, &ieModule);
	    		printf("  %s\n", ieModule->moduleDescription);
	    		attr_head = ieModule->attributeList;
	    		attr_loop = LST_Count(&attr_head);

	    		for (j = 0; j < attr_loop; j++) {
	    			attr_node = LST_Pop(&attr_head);
	    			flag = findElement(object, attr_node->element.tag, &element);
	    			cond = DCM_LookupElement(&element);
	    			if (cond != DCM_NORMAL) cond = COND_PopCondition(FALSE);

	    			if (!flag) {
	    				if (attr_node->requirement == IE_K_TYPE1){
	    					printf("    %08x, %s\n", element.tag, element.description);
	    				}else if (attr_node->requirement == IE_K_TYPE2){
	    					cond = DCM_GetElementSize(&object, attr_node->element.tag, &length);
	    					if (cond != DCM_NORMAL){
	    						cond = COND_PopCondition(FALSE);
	    						printf("    %08x, %s\n", element.tag, element.description);
	    					}
	    				}
	    			}
	    		}		/* finish one module */
	    		free(mod_node);
	    		cond = IE_Free((void **) &ieModule);
	    	}
	    	free(ie_node);
	    	cond = IE_Free((void **) &ieIE);
	    }
	    cond = IE_Free((void **) &ieObject);
    	}
    }

    /* Free the memory and remove the object handle. */
    cond = DCM_CloseObject(&object);
    if (cond != DCM_NORMAL){
    	COND_DumpConditions();
    }else{
    	printf("The object  is closed successfully.\n");
    }
    THR_Shutdown();
    return 0;
}

/* findAsciiR
**
** Purpose:
**      Find the ascii description of the IE_IEREQUIREMENT requirement.
**
** Parameter Dictionary:
**      table: Handle to the requirement table.
**
** Return Values:
**	The ascii description of the requirement is returned.
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static char *
findAsciiR(REQUIREMENTID * table, IE_IEREQUIREMENT id, int loop)
{
    int        i = 0;

    while (i < loop && id != table[i].requirement){
    	i++;
    }
    return table[i].string;
}

/* findAsciiS
**
** Purpose:
**      Find the ascii description of the IE_STRUCTURE type.
**
** Parameter Dictionary:
**      table: Handle to the structure table.
**
** Return Values:
**	The ascii description of the structure type is returned.
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static char *
findAsciiS(STRUCTUREID * table, IE_STRUCTURETYPE id, int loop)
{
    int        i = 0;

    while ((i < loop) && (id != table[i].type)){
    	i++;
    }
    return table[i].string;
}

/* findAsciiC
**
** Purpose:
**      Find the ascii description of the IE_STATUS status.
**
** Parameter Dictionary:
**      table: Handle to the IE_STATUS table.
**
** Return Values:
**	The ascii description of the status is returned.
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static char *
findAsciiC(STATUSID * table, IE_STATUS id, int loop)
{
    int        i = 0;

    while (i < loop && id != table[i].status){
    	i++;
    }
    return table[i].string;
}

/* printObject
**
** Purpose:
**      Print every field in the IE_OBJECT structure.
**
** Parameter Dictionary:
**      ieObject: Handle to the IE_OBJECT object.
**      cond:     Return condition of the IE routine.
**
** Return Values:
**	None
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static void
printObject(IE_OBJECT * ieObject)
{
    printf("%9s,  %12s, %12s,   %8s\n", findAsciiS(structure, ieObject->structureType, (int) DIM_OF(structure)), ieObject->classUID,
    		ieObject->objectDescription, findAsciiC(status, ieObject->status, (int) DIM_OF(status)));
}

/* printIE
**
** Purpose:
**      Print every field in the IE_IE structure.
**
** Parameter Dictionary:
**      ieIE: Handle to the IE_IE information entity.
**      cond: Return condition of the IE routine.
**
** Return Values:
**	None
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static void
printIE(IE_INFORMATIONENTITY * ieIE)
{
    printf("  %5s, %18s, %10s, %16s\n",	findAsciiS(structure, ieIE->structureType, (int) DIM_OF(structure)), ieIE->ieDescription, findAsciiR(requirement,
		   ieIE->requirement, (int) DIM_OF(requirement)), findAsciiC(status, ieIE->status, (int) DIM_OF(status)));
}

/* printModule
**
** Purpose:
**      Print every field in the IE_MODULE structure.
**
** Parameter Dictionary:
**      ieModule: Handle to the IE_MODULE module.
**      cond:     Return condition of the IE routine.
**
** Return Values:
**	None
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static void
printModule(IE_MODULE * ieModule)
{
    printf("    %9s, %20s, %10s, %8s\n", findAsciiS(structure, ieModule->structureType, (int) DIM_OF(structure)), ieModule->moduleDescription,
    		findAsciiR(requirement, ieModule->requirement, (int) DIM_OF(requirement)), findAsciiC(status, ieModule->status, (int) DIM_OF(status)));
}

/* printIEAttribute
**
** Purpose:
**      Print every field in the IE_ATTRIBUTE structure.
**
** Parameter Dictionary:
**      ieAttr: Handle to the IE_ATTRIBUTE attribute.
**
** Return Values:
**	None
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static void
printIEAttribute(IE_ATTRIBUTE * ieAttr)
{
/*    printf("\t %10s,  %8s,           %04x,         %04x\n",
      findAsciiS(structure, ieAttr->structureType, (int)DIM_OF(structure)),
           findAsciiAR(attr_req, ieAttr->requirement, (int)DIM_OF(attr_req)),
	   ieAttr->element.group, ieAttr->element.element);   */
    printElement(&(ieAttr->element));
}


/* printElement
**
** Purpose:
**      Dump the contents of the dicom element to the standard output.
**
** Parameter Dictionary:
**      element: Handle to the dicom data element.
**
** Return Values:
**	None
**
** Algorithm:
**	switch(dicom element value representation)
**		print the tag, length, description, and value to the
**		standard output.
**
*/

static void
printElement(DCM_ELEMENT * element)
{
    switch (element->representation) {
		case DCM_AS:		/* Age string */
		case DCM_CS:		/* control string */
		case DCM_DA:		/* Date */
		case DCM_DS:		/* Decimal string */
		case DCM_IS:		/* Integer string */
		case DCM_LO:		/* Long string */
		case DCM_LT:		/* Long text */
		case DCM_ST:		/* Short text */
		case DCM_SH:		/* Short string */
		case DCM_TM:		/* Time */
		case DCM_UI:		/* UID */
		case DCM_PN:		/* Person name */
							printf("      %08x, %2d, %s, %s\n", element->tag, element->length, element->description, element->d.string);
							break;
		case DCM_SS:		/* Signed short */
							printf("      %08x, %2d, %s, %d\n", element->tag, element->length, element->description, *element->d.us);
							break;
		case DCM_SL:		/* signed long */
							printf("      %08x, %2d, %s, %ld\n", element->tag, element->length, element->description, *element->d.sl);
							break;
		case DCM_US:		/* unsigned short */
							printf("      %08x, %2d, %s, %u\n", element->tag, element->length, element->description, *element->d.us);
							break;
		case DCM_UL:		/* unsigned long */
							printf("      %08x, %2d, %s, %lu\n", element->tag, element->length, element->description, *element->d.ul);
							break;
		case DCM_AT:		/* attribute Tag  */
							{
								unsigned long i;
								printf("      %08x, %2d, %s,", element->tag, element->length, element->description);
								for (i = 0; i < (element->length / sizeof(DCM_TAG)); i++) {
									printf("<%04x,%04x> ", DCM_TAG_GROUP(element->d.at[i]), DCM_TAG_ELEMENT(element->d.at[i]));
								}
								printf("\n");
							}
							break;
		default:
							break;
    }
}

/* findElement
**
** Purpose:
**      Fill in the DCM_ELEMENT structure according to the tag. Returns
**      TRUE if the element is present in the object, FALSE otherwise.
**
** Parameter Dictionary:
**      object:  Handle to the DCM information object.
**      tag:     tag of the element.
**      element: Handle to the DCM_ELEMENT element.
**
** Return Values:
**      A boolean flag is returned TRUE to indicate the attribute is
**      found, and FALSE otherwise.
**
** Algorithm:
**      Call DCM_LookupElement() to find the representation of the
**      attribute according to the tag.
**      If the representaion is found,
**		then length field is filled and the memory to hold the
**      	data is assigned.
**	else
**		Call DCM_GetElementSize(), if the return condition
**		is DCM_NORMAL, then return TRUE.
**      Call DCM_GetElementValue() to retrieve the data for the
**      attribute.
**
*/

static CTNBOOLEAN
findElement(DCM_OBJECT * object, DCM_TAG tag, DCM_ELEMENT * element)
{
    CONDITION		cond;				/* Return value from DUL and ACR routines */
    DCM_ELEMENT		dcm_element;		/* Handle to the dicom data element */
    void			*ctx;				/* Context variable used by DCM_GetElementValue */
    CTNBOOLEAN		flag = TRUE;		/* See Return Values in the header */
    CTNBOOLEAN		isString = FALSE;	/* Flag indicates if value is of string type */


    /* Find the group and element fields in the tag. */
    dcm_element.tag = tag;
    element->tag = dcm_element.tag;

    /* Find the representation of the data in the element. */
    cond = DCM_LookupElement(&dcm_element);
    if (cond != DCM_NORMAL) {
    	cond = COND_PopCondition(FALSE);
    	return FALSE;
    }

    /* Fill the length field and assign the memory for the data. */
    switch (dcm_element.representation) {
		case DCM_LO:		/* Long string */
		case DCM_UI:		/* UID */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_DA:		/* Date */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_CS:		/* Control string */
		case DCM_TM:		/* Time */
		case DCM_SH:		/* Short string */
		case DCM_DS:		/* Decimal string */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_IS:		/* Integer string */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_ST:		/* Short text */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_LT:		/* Long text */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_PN:		/* Person name */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_AS:		/* Age string */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.string = malloc((dcm_element.length + 1) * sizeof(char));
								isString = TRUE;
							}
							break;
		case DCM_SS:		/* Signed short */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.ss = malloc((dcm_element.length + 1) * sizeof(char));
							}
							break;
		case DCM_SL:		/* signed long */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.sl = malloc((dcm_element.length + 1) * sizeof(char));
							}
							break;
		case DCM_US:		/* unsigned short */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.us = malloc((dcm_element.length + 1) * sizeof(char));
							}
							break;
		case DCM_UL:		/* unsigned long */
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond != DCM_NORMAL){
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}else{
								dcm_element.d.ul = malloc((dcm_element.length + 1) * sizeof(char));
							}
							break;
		case DCM_SQ:		/* Sequence of items */
							break;
		default:
							cond = DCM_GetElementSize(&object, tag, &(dcm_element.length));
							if (cond == DCM_NORMAL){
								return flag;
							}else{
								cond = COND_PopCondition(FALSE);
								return FALSE;
							}
    }

    /* The following is necessary when made the first call to DCM_GetElementValue(). */
    ctx = NULL;
    cond = DCM_GetElementValue(&object, &dcm_element, &(dcm_element.length), &ctx);
    if (cond != DCM_NORMAL) {	/* check for error */
    	/* the attribute is not found */
    	flag = FALSE;
    	cond = COND_PopCondition(FALSE);
    }else{
    	if (isString) dcm_element.d.string[dcm_element.length] = '\0';
    	*element = dcm_element;
    }
    return flag;
}

/* usageerror
**
** Purpose:
**      Print the usage message for this program and exit.
**
** Parameter Dictionary:
**	None
**
** Return Values:
**	None
**
** Algorithm:
**      Description of the algorithm (optional) and any other notes.
*/

static void
usageerror()
{
    char msg[] = "\
Usage: object_verify [-b] [-t] [-v] filename\n\
\n\
  -b  Read file with big endian transfer syntax\n\
  -t  Read file with DICOM Part 10 format\n\
  -v  Place DCM facility in verbose mode\n\
\n\
  filename    Of the image to be verified\n";

    fprintf(stderr, msg);
    exit(1);
}
