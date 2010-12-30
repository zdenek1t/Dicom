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
**		     Electronic Radiology Laboratory
**		   Mallinckrodt Institute of Radiology
**		Washington University School of Medicine
**
** Module Name(s):	TBL_Open
**			TBL_Close
**			TBL_Select
**			TBL_Update
**			TBL_Insert
**			TBL_Delete
**			TBL_Debug
** Author, Date:	Steve Moore, 30-Dec-1998
** Intent:		Provide a general set of functions to be performed
**			on tables in a relational database.
**			These are wrappers for the postgres DB.
** Last Update:		$Author: smm $, $Date: 2002/11/01 17:49:54 $
** Source File:		$RCSfile: tbl_psql.c,v $
** Revision:		$Revision: 1.7 $
** Status:		$State: Exp $
*/

static char rcsid[] = "$Revision: 1.7 $ $RCSfile: tbl_psql.c,v $";

#include "../dicom/ctn_os.h"

#include "../dicom/dicom.h"
#include "../condition/condition.h"
#include "tblprivate.h"
#include "tbl.h"
#include <stdio.h>
#include <ctype.h>


#include "../thread/ctnthread.h"


#ifdef PSQL

#include "libpq-fe.h"
#include "tbl_psql.h"

/*
** Static Globals for this file...
*/
static TBL_CONTEXT		*G_ContextHead = (TBL_CONTEXT *) NULL;
static int			    G_OpenFlag = 0;
static CTNBOOLEAN	    G_Verbose = FALSE;
static PGconn 			*G_DbConn;


static void remapLower(const char* in, char* out)
{
  while (*in != '\0') {
    if (isupper(*in)){
      *out++ = tolower(*in++);
    }else{
      *out++ = *in++;
    }
  }
  *out = '\0';
}

static void
addFieldNames(const TBL_FIELD* fp, char* command)
{
  strcat(command, "\"");
  strcat(command, fp->FieldName);
  strcat(command, "\"");
  fp++;
  while(fp->FieldName != NULL) {
	  strcat(command, " , ");
	  strcat(command, "\"");
	  strcat(command, fp->FieldName);
	  strcat(command, "\"");
	  fp++;
  }
}

static void
addOneFieldValue(const TBL_FIELD* fp, char* command)
{
  char 	temp[512];
  char 	*pField;
  char 	*pData;
  char 	quote[2];
  char 	*pQuote;

  switch(fp->Value.Type) {
	  case TBL_OTHER:
					  fprintf(stderr, "Not ready to handle TBL_OTHER\n");
					  exit(1);
					  break;
	  case TBL_UNSIGNED2:
					  sprintf(temp, "%u", *fp->Value.Value.Unsigned2);
					  strcat(command, temp);
					  break;
	  case TBL_UNSIGNED4:
					  sprintf(temp, "%u", *fp->Value.Value.Unsigned4);
					  strcat(command, temp);
					  break;
	  case TBL_SIGNED2:
					  sprintf(temp, "%d", *fp->Value.Value.Signed2);
					  strcat(command, temp);
					  break;
	  case TBL_SIGNED4:
					  sprintf(temp, "%d", *fp->Value.Value.Signed4);
					  strcat(command, temp);
					  break;
	  case TBL_FLOAT4:
					  sprintf(temp, "%f", *fp->Value.Value.Float4);
					  strcat(command, temp);
					  break;
	  case TBL_FLOAT8:
					  sprintf(temp, "%f", *fp->Value.Value.Float8);
					  strcat(command, temp);
					  break;
	  case TBL_STRING:
	  case TBL_TEXT:
					  strcpy(quote, "'");
					  pQuote = quote;
					  pField = fp->Value.Value.String;
					  pData = command + strlen(command);
					  *pData++ = *pQuote;

					  while (*pField != '\0') {
						  if (*pField == *pQuote) {
							  *pData++ = *pQuote;
							  *pData++ = *pField++;
						  } else {
							  *pData++ = *pField++;
						  }
					  }
					  *pData++ = *pQuote;
					  *pData = '\0';
					  break;
	  case TBL_BINARYDATA:
					  fprintf(stderr, "Not ready for TBL_BINARYDATA\n");
					  exit(1);
					  break;
	  default:
					  fprintf(stderr, "In TBL:PGSQL:addOneFieldValue, got to default data type\n");
					  exit(1);
					  break;
  }
}

static void
addFieldValues(const TBL_FIELD* fp, char* command)
{
  addOneFieldValue(fp, command);
  fp++;
  while(fp->FieldName != NULL) {
	  strcat(command, " , ");
	  addOneFieldValue(fp, command);
	  fp++;
  }
}

static void
addOneCriteria(const TBL_CRITERIA* cp, char* command)
{
  char 	*pField = 0;
  char 	*pData = 0;
  char 	quote[2];
  char 	*pQuote;

  if (cp->Operator != TBL_NOP){
	  strcat(command, "\"");
  	  strcat(command, cp->FieldName);
  	  strcat(command, "\"");
  };

  switch (cp->Operator) {
	  case TBL_EQUAL:
					  strcat(command, " = ");
					  break;
	  case TBL_LIKE:
					  strcat(command, " like ");
					  break;
	  case TBL_NOT_EQUAL:
					  strcat(command, " <> ");
					  break;
	  case TBL_GREATER:
					  strcat(command, " > ");
					  break;
	  case TBL_GREATER_EQUAL:
					  strcat(command, " >= ");
					  break;
	  case TBL_LESS:
					  strcat(command, " < ");
					  break;
	  case TBL_LESS_EQUAL:
					  strcat(command, " <= ");
					  break;
	  case TBL_NOP:
					  strcat(command, cp->Value.Value.String);
					  break;
	  case TBL_NULL:
					  strcat(command, " IS NULL ");
					  break;
	  case TBL_NOT_NULL:
					  strcat(command, " IS NOT NULL ");
					  break;
  }

  if ((cp->Operator != TBL_NULL) && (cp->Operator != TBL_NOT_NULL) && (cp->Operator != TBL_NOP)) {
	  char foo[100];

	  switch (cp->Value.Type) {
			case TBL_SIGNED2:
							sprintf(foo, " %d ", *(cp->Value.Value.Signed2));
							strcat(command, foo);
							break;
			case TBL_UNSIGNED2:
							sprintf(foo, " %d ", *(cp->Value.Value.Unsigned2));
							strcat(command, foo);
							break;
			case TBL_SIGNED4:
							sprintf(foo, " %d ", *(cp->Value.Value.Signed4));
							strcat(command, foo);
							break;
			case TBL_UNSIGNED4:
							sprintf(foo, " %d ", *(cp->Value.Value.Unsigned4));
							strcat(command, foo);
							break;
			case TBL_FLOAT4:
							sprintf(foo, " %f ", *(cp->Value.Value.Float4));
							strcat(command, foo);
							break;
			case TBL_FLOAT8:
							sprintf(foo, " %f ", *(cp->Value.Value.Float8));
							strcat(command, foo);
							break;
			case TBL_STRING:
			case TBL_MBSTRING:
			case TBL_TEXT:
							strcpy(quote, "'");
							pQuote = quote;
							pField = cp->Value.Value.String;
							pData = command + strlen(command);
							*pData++ = *pQuote;

							while (*pField != '\0') {
								if (*pField == *pQuote) {
									*pData++ = *pQuote;
									*pData++ = *pField++;
								} else {
									*pData++ = *pField++;
								}
							}
							*pData++ = *pQuote;
							*pData = '\0';
							break;
			default:
							break;
	  }
  }
  return;
}

static void
addCriteria(const TBL_CRITERIA* cp, char* command)
{
  addOneCriteria(cp, command);
  cp++;
  while(cp->FieldName != NULL) {
	  strcat(command, " AND ");
	  addOneCriteria(cp, command);
	  cp++;
  }
}

static void
addOneUpdateValue(const TBL_UPDATE* up, char* command)
{
  char 	*c;
  c = command + strlen(command);

  if (up->Function == TBL_SET) {
    switch (up->Value.Type) {
		case TBL_SIGNED2:
							sprintf(c, "%d", *(up->Value.Value.Signed2));
							break;
		case TBL_UNSIGNED2:
							sprintf(c, "%d", *(up->Value.Value.Unsigned2));
							break;
		case TBL_SIGNED4:
							sprintf(c, "%d", *(up->Value.Value.Signed4));
							break;
		case TBL_UNSIGNED4:
							sprintf(c, "%d", *(up->Value.Value.Unsigned4));
							break;
		case TBL_FLOAT4:
							sprintf(c, "%f", *(up->Value.Value.Float4));
							break;
		case TBL_FLOAT8:
							sprintf(c, "%f", *(up->Value.Value.Float8));
							break;
		case TBL_MBSTRING:
		case TBL_STRING:
							sprintf(c, "\'%s\'", up->Value.Value.String);
							break;
		case TBL_TEXT:
							sprintf(c, "\"FILLER-WILL BE REPLACED\"");
							break;
		case TBL_BINARYDATA:
							sprintf(c, "0xFFFFFFFF");
							break;
		case TBL_OTHER:
							break;
    }
  }else if (up->Function == TBL_ZERO){
	  sprintf(c, " 0 ");
  }else if (up->Function == TBL_INCREMENT){
	  sprintf(c, " %s + 1 ", up->FieldName);
  }else if (up->Function == TBL_DECREMENT){
	  sprintf(c, " %s - 1 ", up->FieldName);
  }else if (up->Function == TBL_ADD) {
	  switch (up->Value.Type) {
		  case TBL_SIGNED2:
								 sprintf(c, " %s + %d ", up->FieldName, *(up->Value.Value.Signed2));
								 break;
		  case TBL_SIGNED4:
								 sprintf(c, " %s + %d ", up->FieldName, *(up->Value.Value.Signed4));
								 break;
		  case TBL_UNSIGNED2:
								 sprintf(c, " %s + %d ", up->FieldName, *(up->Value.Value.Unsigned2));
								 break;
		  case TBL_UNSIGNED4:
								 sprintf(c, " %s + %d ", up->FieldName, *(up->Value.Value.Unsigned4));
								 break;
		  case TBL_FLOAT4:
								 sprintf(c, " %s + %.6f ", up->FieldName, *(up->Value.Value.Float4));
								 break;
		  case TBL_FLOAT8:
								 sprintf(c, " %s + %.6f ", up->FieldName, *(up->Value.Value.Float8));
								 break;
		  default:
			  	  	  	  	  	 break;
	  }
  }else if (up->Function == TBL_SUBTRACT){
	  switch (up->Value.Type) {
		  case TBL_SIGNED2:
								 sprintf(c, " %s - %d ", up->FieldName, *(up->Value.Value.Signed2));
								 break;
		  case TBL_SIGNED4:
								 sprintf(c, " %s - %d ", up->FieldName, *(up->Value.Value.Signed4));
								 break;
		  case TBL_UNSIGNED2:
								 sprintf(c, " %s - %d ", up->FieldName, *(up->Value.Value.Unsigned2));
								 break;
		  case TBL_UNSIGNED4:
								 sprintf(c, " %s - %d ", up->FieldName, *(up->Value.Value.Unsigned4));
								 break;
		  case TBL_FLOAT4:
								 sprintf(c, " %s - %.6f ", up->FieldName, *(up->Value.Value.Float4));
								 break;
		  case TBL_FLOAT8:
								 sprintf(c, " %s - %.6f ", up->FieldName, *(up->Value.Value.Float8));
								 break;
		  default:
			  	  	  	  	  	 break;
	  }
  }
  return;
}

static void
addUpateValues(const TBL_UPDATE* up, char *command)
{
  int 		first = 1;

  while (up->FieldName != NULL) {
	  if (!first) strcat(command, " , ");
	  strcat(command, "\"");
	  strcat(command, up->FieldName);
	  strcat(command, "\"");
	  strcat(command, " = ");
	  addOneUpdateValue(up, command);

	  first = 0;
	  up++;
  }
}

static void
extractOneFieldResult(PGresult* res, int tuple, int fieldNum, TBL_FIELD* fp)
{
  char 		*c;
  int 		len;

  fp->Value.IsNull = 0;

  switch (fp->Value.Type) {
	  case TBL_SIGNED2:
						  fp->Value.Size = 2;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Signed2) = BIG_2;
						  }else{
							  *(fp->Value.Value.Signed2) = atoi(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Signed2) == BIG_2) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_UNSIGNED2:
						  fp->Value.Size = 2;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Unsigned2) = BIG_2;
						  }else{
							  *(fp->Value.Value.Unsigned2) = atoi(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Unsigned2) == BIG_2) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_SIGNED4:
						  fp->Value.Size = 4;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Signed4) = BIG_4;
						  }else{
							  *(fp->Value.Value.Signed4) = atoi(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Signed4) == BIG_4) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_UNSIGNED4:
						  fp->Value.Size = 4;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Unsigned4) = BIG_4;
						  }else{
							  *(fp->Value.Value.Unsigned4) = atoi(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Unsigned4) == BIG_4) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_FLOAT4:
						  fp->Value.Size = 4;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Float4) = BIG_4;
						  }else{
							  *(fp->Value.Value.Float4) = atof(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Float4) == BIG_4) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_FLOAT8:
						  fp->Value.Size = 8;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  *(fp->Value.Value.Float8) = BIG_4;
						  }else{
							  *(fp->Value.Value.Float8) = atof(PQgetvalue(res, tuple, fieldNum));
						  }
						  if (*(fp->Value.Value.Float8) == BIG_4) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  case TBL_TEXT:
	  case TBL_MBSTRING:
	  case TBL_STRING:
						  fp->Value.Size = fp->Value.AllocatedSize;
						  if (PQgetisnull(res, tuple, fieldNum)){
							  fp->Value.Value.String[0] = '\0';
						  }else{
							  strncpy(fp->Value.Value.String, PQgetvalue(res, tuple, fieldNum), fp->Value.AllocatedSize - 1);
							  fp->Value.Value.String[fp->Value.AllocatedSize-1] = '\0';
						  }
						  len = strlen(fp->Value.Value.String);
						  c = fp->Value.Value.String + len;

						  while((len > 0) && (*(--c) == ' ')) {
							  *c = '\0';
							  len--;
						  }
						  if (strcmp(fp->Value.Value.String, "") == 0) {
							  fp->Value.IsNull = 1;
							  fp->Value.Size = 0;
						  }
						  break;
	  default:
		  	  	  	  	  break;
  }
}


static void
extractFieldResults(PGresult* res, int tuple, TBL_FIELD* fp)
{
  int fieldNum;

  for (fieldNum = 0; fp->FieldName != NULL; fp++, fieldNum++) {
	  extractOneFieldResult(res, tuple, fieldNum, fp);
  }
}

CONDITION
TBL_ConnectDB(void)
{

	PGconn 	*DB_conn;

	DB_conn = PQsetdbLogin( "server", "5432", NULL, NULL, "DICOM_DB_NEW", "postgres", NULL);

	if (PQstatus(DB_conn) == CONNECTION_BAD){
		#ifdef CTN_USE_THREADS
			THR_ReleaseMutex(FAC_TBL);
		#endif
		if(G_Verbose) fprintf(stdout, "Can't open DB (%i)\n",getpid());
		G_DbConn = (PGconn*) NULL;
		return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), "TBL_ConnectDB");
	}else{
		G_DbConn = DB_conn;
		if(G_Verbose) fprintf(stdout, "Open DB (%i)\n",getpid());
		return TBL_NORMAL;
	}
}

CONDITION
TBL_CloseDB(void)
{
	if (PQstatus(G_DbConn) != CONNECTION_BAD){
		PQfinish(G_DbConn);
		if(G_Verbose) fprintf(stdout, "Close DB (%i)\n",getpid());
		return TBL_NORMAL;
	}else{
		if(G_Verbose) fprintf(stdout, "ERROR: Can't close DB (%i)\n",getpid());
		return COND_PushCondition(TBL_ERROR(TBL_CLOSERROR), "TBL_CloseDB");
	}
}

CONDITION
TBL_SetOption(const char *string)
{
    return TBL_NORMAL;
}

int
TBL_HasViews(void)
{
    return TBL_NORMAL;
}

int
TBL_HasUpdateIncrement(void)
{
    return 0;
}

/* TBL_Debug
**
** Purpose:
**	Simple function to switch on/off Msql debug messages.
**
** Parameter Dictionary:
**	CTNBOOLEAN flag: the flag that controls the messages
**
** Return Values:
**	TBL_NORMAL: normal termination.
**
** Notes:
**	The initial state of the debugging messages is off (FALSE).
**
** Algorithm:
**	If flag evaluates to true, the global variable G_Verbose is
*	set (to TRUE) otherwise it is reset (to FALSE);
*/
CONDITION
TBL_Debug(CTNBOOLEAN flag)
{
	G_Verbose = flag;
    return TBL_NORMAL;
}

/* TBL_Open
**
** Purpose:
**	This function "opens" the specified table in the specified
**	database.  It creates a new handle for this particular table
**	and passes that identifier back to the user.
**
** Parameter Dictionary:
**	char *databaseName: The name of the database to open.
**	char *tableName: The name of the table to open in the
**		aforementioned database.
**	TBL_HANDLE **handle: The pointer for the new identifier
**		created for this database/table pair is returned
**		through handle.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_DBINITFAILED: The initial database open failed.
**	TBL_ALREADYOPENED: The table/database pair has been opened
**		previously and may not be opened again without
**		first closing it.
**	TBL_DBNOEXIST: The database specified as a calling parameter
**		does not exist.
**	TBL_TBLNOEXIST: The table specified as a calling parameter
**		does not exist within the specified database.
**	TBL_NOMEMORY: There is no memory available from the system.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The first time TBL_Open is invoked, special routines
**	are called to allocate the communication structures needed
**	for subsequent operations.  A check is made to ensure that
**	this table/database pair has not already been opened.  A
**	unique handle(address) is then created for this pair and
**	returned to the user for subsequent table operations.
*/

CONDITION
TBL_Open(const char *databaseName, const char *tableName, TBL_HANDLE ** handle)
{
  TBL_CONTEXT* 		tc;
  char 				*tdb;
  char 				*ttb;
  PGconn 			*conn;

  (*handle) = (void *) NULL;

#ifdef CTN_USE_THREADS
  THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  while (tc != (TBL_CONTEXT *) NULL){
	  if ((strcmp(tc->databaseName, databaseName) == 0) && (strcmp(tc->tableName, tableName) == 0)){
		  tc->refCount++;
		  (*handle) = (void *) tc;
		  G_OpenFlag++;
#ifdef CTN_USE_THREADS
		  THR_ReleaseMutex(FAC_TBL);
#endif
		  return TBL_NORMAL;
	  }
	  tc = tc->next;
  }

  conn = G_DbConn;

  if (PQstatus(conn) == CONNECTION_BAD){
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), tableName);
  }
  /* We have to assume at this point that everything will be ok... */
  if ((tc = (TBL_CONTEXT *) malloc(sizeof(TBL_CONTEXT))) == (TBL_CONTEXT *) NULL){
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }
  
  if ((tdb = (char *) malloc(strlen(databaseName) + 1)) == (char *) NULL){
	  free(tc);
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }
  if ((ttb = (char *) malloc(strlen(tableName) + 1)) == (char *) NULL){
	  free(tc);
	  free(tdb);
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }
  strcpy(tdb, databaseName);
  strcpy(ttb, tableName);
  tc->databaseName = tdb;
  tc->tableName = ttb;
  tc->refCount = 1;
  tc->dbSpecific = conn;
  tc->next = G_ContextHead;
  G_ContextHead = tc;

  (*handle) = (void *) G_ContextHead;

  G_OpenFlag++;

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif

  return TBL_NORMAL;
}

CONDITION
TBL_OpenDB(const char *databaseName, TBL_HANDLE ** handle)
{
  TBL_CONTEXT* 	tc;
  char 			*tdb;
  char 			*ttb;
  PGconn 		*conn;
  char* 		tableName = "none";

  (*handle) = (void *) NULL;

#ifdef CTN_USE_THREADS
  THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  while (tc != (TBL_CONTEXT *) NULL) {
    if ((strcmp(tc->databaseName, databaseName) == 0) && (strcmp(tc->tableName, tableName) == 0)) {
    	tc->refCount++;
    	(*handle) = (void *) tc;
    	G_OpenFlag++;
#ifdef CTN_USE_THREADS
    	THR_ReleaseMutex(FAC_TBL);
#endif
    	return TBL_NORMAL;
    }
    tc = tc->next;
  }

  conn = G_DbConn;

  if (PQstatus(conn) == CONNECTION_BAD) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_OPENFAILED), tableName);
  }

  /* We have to assume at this point that everything will be ok...   */
  if ((tc = (TBL_CONTEXT *) malloc(sizeof(TBL_CONTEXT))) == (TBL_CONTEXT *) NULL) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }

  if ((tdb = (char *) malloc(strlen(databaseName) + 1)) == (char *) NULL) {
	  free(tc);
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }

  if ((ttb = (char *) malloc(strlen(tableName) + 1)) == (char *) NULL) {
	  free(tc);
	  free(tdb);
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_NOMEMORY), "TBL_Open");
  }
  strcpy(tdb, databaseName);
  strcpy(ttb, tableName);
  tc->databaseName = tdb;
  tc->tableName = ttb;
  tc->refCount = 1;
  tc->dbSpecific = conn;
  tc->next = G_ContextHead;
  G_ContextHead = tc;

  (*handle) = (void *) G_ContextHead;

  G_OpenFlag++;

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif

  return TBL_NORMAL;
}

/* TBL_Close
**
** Purpose:
**	This function "closes" the specified table in the specified
**	database.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be closed
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_CLOSERROR: The handle to be closed could not be located
**		in the internal list or no database/table pairs
**		had been opened up to this point.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	Locates the handle specified in the call and removes that
**	entry from the internal list maintained by this	facility.
*/
CONDITION
TBL_Close(TBL_HANDLE ** handle)
{
	TBL_CONTEXT* 	prevtc;
	TBL_CONTEXT* 	tc;
	TBL_CONTEXT* 	hc;

	THR_ObtainMutex(FAC_TBL);

    G_OpenFlag--;
    hc = (TBL_CONTEXT *) (*handle);
    prevtc = tc = G_ContextHead;

    while (tc != (TBL_HANDLE *) NULL) {
    	if (hc == tc) {
//    		PQfinish(tc->dbSpecific);

    		tc->refCount--;
		    if (tc->refCount > 0) {
		    	THR_ReleaseMutex(FAC_TBL);
		    	return TBL_NORMAL;
		    }
		    free(tc->databaseName);
		    free(tc->tableName);

		    if (tc == G_ContextHead){
		    	G_ContextHead = tc->next;
		    }else{
		    	prevtc->next = tc->next;
		    }

		    free(tc);
		    (*handle) = (TBL_HANDLE *) NULL;
		    THR_ReleaseMutex(FAC_TBL);

		    return TBL_NORMAL;
	   }
    	prevtc = tc;
    	tc = tc->next;
    }
    THR_ReleaseMutex(FAC_TBL);
    return COND_PushCondition(TBL_ERROR(TBL_CLOSERROR), "TBL_Close");
}
/* TBL_Select
**
** Purpose:
**	This function selects some number of records (possibly zero),
**	that match the criteria specifications given in the input
**	parameter criteriaList.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed.  This table must be open.
**	TBL_CRITERIA *criteriaList: Contains a list of the criteria
**		to use when selecting records from the specified table.
**		A null list implies that all records will be selected.
**	TBL_FIELD *fieldList: Contains a list of the fields to be
**		retreived from each record that matches the criteria
**		specification.  It is an error to specify a null
**		fieldList.
**	int *count: Contains a number that represents the total number
**		of records retreived by this particular select.  If this
**		parameter is null, then an internal counter is used and
**		the final count will not be returned when the select
**		finishes.
**	CONDITION (*callback)(): The callback function invoked whenever
**		a new record is retreived from the database.  It is
**		invoked with parameters as described below.
**	void *ctx: Ancillary data passed through to the callback function
**		and untouched by this routine.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_NOFIELDLIST: A null field list pointer (fieldList *) was
**		specified.
**	TBL_SELECTFAILED: The select operation failed most probably from
**		a bad specification in the fieldList or criteriaList.  This
**		return is not the same as a valid query returning no records.
**		This error return could result from a misspelled keyword, etc.
**	TBL_EARLYEXIT: The callback routine returned something other than
**		TBL_NORMAL which caused this routine to cancel the remainder
**		of the database operation and return early.
**
** Algorithm:
**	As each record is retreived from the
**	database, the fields requested by the user (contained in
**	fieldList), are filled with the informatiton retreived from
**	the database and a pointer to the list is passed to the
**	callback routine designated by the input parameter callback.
**	The callback routine is invoked as follows:
**
**		callback(fieldList *fieldList, long count, void *ctx)
**
**	The count contains the number of records retreived to this point.
**	ctx contains any additional information the user originally passed
**	to this select function.  If callback returns any value other
**	than TBL_NORMAL, it is assumed that this function should terminate
**	(i.e. cancel the current db operation), and return an abnormal
**	termination message (TBL_EARLYEXIT) to the routine which
**	originally invoked the select.
*/
CONDITION
TBL_Select(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_FIELD * fieldList, long *count, CONDITION(*callback) (), void *ctx)
{
  TBL_CONTEXT* 	tc;
  char* 		dbName;
  char 			*tableName;
  int 			i;
  int 			foundit;
  char 			selectCommand[2048];
  long 			realcount;
  long 			* lp;
  PGresult* 	res;
  PGconn* 		conn;
  int 			nTuples;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;

  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  tableName = tc->tableName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Select");
  }

  strcpy(selectCommand, "SELECT ");
  addFieldNames(fieldList, selectCommand);
  strcat(selectCommand, " FROM " );
  strcat(selectCommand, "srv_app.\"");  
  strcat(selectCommand, tableName);
  strcat(selectCommand, "\"");
  
  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(selectCommand, " WHERE ");
	  addCriteria(criteriaList, selectCommand);
  }
  strcat(selectCommand, ";" );

  if (count != (long *) NULL){
	  lp = count;
  }else{
	  lp = &realcount;
  }
  *lp = 0;

  if(G_Verbose) printf("SQL: %s\n",selectCommand);
  res = PQexec(conn, selectCommand);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_SELECTFAILED), selectCommand, "TBL_Select");
  }
  nTuples = PQntuples(res);

  for (i = 0; i < nTuples; i++) {
	  (*lp)++;
	  extractFieldResults(res, i, fieldList);

	  if (callback != NULL) {
		  if (callback(fieldList, *lp, ctx) != TBL_NORMAL) {
			  PQclear(res);
#ifdef CTN_USE_THREADS
			  THR_ReleaseMutex(FAC_TBL);
#endif
			  return COND_PushCondition(TBL_ERROR(TBL_EARLYEXIT), "TBL_Select");
		  }
	  }
  }
  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

CONDITION
TBL_SelectTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_FIELD * fieldList, long *count, CONDITION(*callback) (), void *ctx, const char* tableName)
{
  TBL_CONTEXT* 	tc;
  char* 		dbName;
  int 			i;
  int 			foundit;
  char 			selectCommand[2048];
  long 			realcount;
  long 			* lp;
  PGresult* 	res;
  PGconn* 		conn;
  int 			nTuples;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
    tc = tc->next;
  }

  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Select");
  }

  strcpy(selectCommand, "SELECT ");
  addFieldNames(fieldList, selectCommand);
  strcat(selectCommand, " FROM " );
  strcat(selectCommand, "srv_app.\"");  
  strcat(selectCommand, tableName);
  strcat(selectCommand, "\"");
  
  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(selectCommand, " WHERE ");
	  addCriteria(criteriaList, selectCommand);
  }

  strcat(selectCommand, ";" );

  if (count != (long *) NULL){
	  lp = count;
  }else{
	  lp = &realcount;
  }
  *lp = 0;

  if(G_Verbose) printf("SQL: %s\n",selectCommand);
  res = PQexec(conn, selectCommand);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_SELECTFAILED), selectCommand, "TBL_SelectTable");
  }
  nTuples = PQntuples(res);

  for (i = 0; i < nTuples; i++) {
	  (*lp)++;
	  extractFieldResults(res, i, fieldList);

	  if (callback != NULL) {
		  if (callback(fieldList, *lp, ctx) != TBL_NORMAL) {
			  PQclear(res);
#ifdef CTN_USE_THREADS
			  THR_ReleaseMutex(FAC_TBL);
#endif
			  return COND_PushCondition(TBL_ERROR(TBL_EARLYEXIT), "TBL_Select");
		  }
	  }
  }
  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

/* TBL_Update
**
** Purpose:
**	This updates existing records in the named table.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed for modification.  This table must be open.
**	TBL_CRITERIA *criteriaList: Contains the list of criteria to
**		select those records that should be updated.
**	TBL_FIELD *fieldList: Contains a list of the keyword/value
**		pairs to be used to modify the selected records.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_NOFIELDLIST: No keyword/value pairs were specified for update.
**	TBL_UPDATEFAILED: The insert operation failed most probably from
**		a bad specification in the fieldList.  This error
**		return could result from a misspelled keyword, etc.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The records which match the (ANDED) criteria in criteriaList
**	are retreived and updated with the information contained in
**	fieldList.  Only the fields contained in fieldList will be
**	updated by this call.
*/
CONDITION
TBL_Update(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  char 				*tableName;
  int 				foundit;
  char 				updateCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  tableName = tc->tableName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Update");
  }

  strcpy(updateCommand, "UPDATE ");
  strcat(updateCommand, "srv_app.\"");  
  strcat(updateCommand, tableName);
  strcat(updateCommand, "\"");
  strcat(updateCommand, " SET ");
  addUpateValues(updateList, updateCommand);

  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(updateCommand, " WHERE ");
	  addCriteria(criteriaList, updateCommand);
  }
  strcat(updateCommand, ";" );

  if(G_Verbose) printf("SQL: %s\n", updateCommand);
  res = PQexec(conn, updateCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_UPDATEFAILED), updateCommand, "TBL_Update");
  }

  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

CONDITION
TBL_UpdateTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList, const char* tableName)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  int 				foundit;
  char 				updateCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Update");
  }

  strcpy(updateCommand, "UPDATE ");
  strcat(updateCommand, "srv_app.\"");
  strcat(updateCommand, tableName);
  strcat(updateCommand, "\"");
  strcat(updateCommand, " SET ");
  addUpateValues(updateList, updateCommand);

  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(updateCommand, " WHERE ");
	  addCriteria(criteriaList, updateCommand);
  }
  strcat(updateCommand, ";" );

  if(G_Verbose) printf("SQL: %s\n",updateCommand);
  res = PQexec(conn, updateCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_UPDATEFAILED), updateCommand, "TBL_UpdateTable");
  }

  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

/* TBL_NextUnique
**
** Purpose:
**	This routine retrieves the next unique number from a predefined table.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed for modification.  This table must be open.
**	char *name : The name of the unique variable to access.
**	int *unique: Contains the next unique number in the sequence upon
**			return.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_SELECTFAILED: The unique number name could not be found.
**	TBL_UPDATEFAILED: The unique number could not be incremented.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The unique number associated with  "name" in the opened table is
**	retrieved and passed back to the user in "unique".  This number is
**	then incremented (by one) and placed back in the table in preparation
**	for the next call to this routine.
*/
CONDITION
TBL_NextUnique(TBL_HANDLE ** handle, char *name, int *unique)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  char 				*tableName;
  int 				foundit;
  char 				selectCommand[2048];
  char 				updateCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;
  int 				nTuples;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  tableName = tc->tableName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_NextUnique");
  }
  res = PQexec(conn, "BEGIN");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  exit(1);
  }
  PQclear(res);

  sprintf(selectCommand, "SELECT UniqueNumber FROM \"%s\" WHERE NumberName=\'%s\'", tableName, name);
  res = PQexec(conn, selectCommand);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  fprintf(stderr, "%s\n", selectCommand);
	  exit(1);
  }

  *unique = -1;
  nTuples = PQntuples(res);
  if (nTuples != 1) {
	  fprintf(stderr, "Expected 1 tuples in TBL_NextUnique, got %d\n", nTuples);
	  exit(1);
  }
  *unique=atoi(PQgetvalue(res, 0, 0));
  PQclear(res);

  sprintf(updateCommand, "UPDATE \"%s\" SET UniqueNumber=UniqueNumber+1 WHERE NumberName=\'%s\'", tableName, name);
  res = PQexec(conn, updateCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  fprintf(stderr, "%s\n", updateCommand);
	  exit(1);
  }
  PQclear(res);

  res = PQexec(conn, "COMMIT");
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  exit(1);
  }
  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

/* TBL_Insert
**
** Purpose:
**	This inserts records into the named table.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed for deletion.  This table must be open.
**	TBL_FIELD *fieldList: Contains a list of the keyword/value
**		pairs to be inserted into the specified table.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_NOFIELDLIST: No keyword/value pairs were specified to
**		insert.
**	TBL_INSERTFAILED: The insert operation failed most probably from
**		a bad specification in the fieldList.  This error
**		return could result from a misspelled keyword, etc.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The table values contained in fieldList are added to the
**	database and table specified by handle.  Each call inserts
**	exactly one record.  It is the users responsibility to ensure
**	that the correct number of values are supplied for the
**	particular table, and that any values which need to be
**	unique (i.e.for the unique key field in a table) are
**	in-fact unique.
*/
CONDITION
TBL_Insert(TBL_HANDLE ** handle, TBL_FIELD * fieldList)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  char 				*tableName;
  int 				foundit;
  char 				insertCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  tableName = tc->tableName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Insert");

  strcpy(insertCommand, "INSERT INTO ");
  strcat(insertCommand, "srv_app.\"");
  strcat(insertCommand, tableName);
  strcat(insertCommand, "\"");
  strcat(insertCommand, "(");
  addFieldNames(fieldList, insertCommand);
  strcat(insertCommand, ") VALUES (" );
  addFieldValues(fieldList, insertCommand);
  strcat(insertCommand, ");" );

  if(G_Verbose) printf("SQL: %s\n",insertCommand);
  res = PQexec(conn, insertCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_INSERTFAILED), insertCommand, "TBL_Insert");
  }
  PQclear(res);
  return TBL_NORMAL;
}
/* TBL_InsertTable
**
** Purpose:
**	This inserts records into the named table.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed for deletion.  This table must be open.
**	TBL_FIELD *fieldList: Contains a list of the keyword/value
**		pairs to be inserted into the specified table.
**	tableName: The name of the table in the database which
**		is the target for the insert.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_NOFIELDLIST: No keyword/value pairs were specified to
**		insert.
**	TBL_INSERTFAILED: The insert operation failed most probably from
**		a bad specification in the fieldList.  This error
**		return could result from a misspelled keyword, etc.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The table values contained in fieldList are added to the
**	database and table specified by handle.  Each call inserts
**	exactly one record.  It is the users responsibility to ensure
**	that the correct number of values are supplied for the
**	particular table, and that any values which need to be
**	unique (i.e.for the unique key field in a table) are
**	in-fact unique.
*/
CONDITION
TBL_InsertTable(TBL_HANDLE ** handle, TBL_FIELD * fieldList, const char* tableName)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  int 				foundit;
  char 				insertCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Insert");

  strcpy(insertCommand, "INSERT INTO ");
  strcat(insertCommand, "srv_app.\"");
  strcat(insertCommand, tableName);
  strcat(insertCommand, "\"");
  strcat(insertCommand, "(");
  addFieldNames(fieldList, insertCommand);
  strcat(insertCommand, ") VALUES (" );
  addFieldValues(fieldList, insertCommand);
  strcat(insertCommand, ");" );

  if(G_Verbose) printf("SQL: %s\n",insertCommand);
  res = PQexec(conn, insertCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  fprintf(stderr, "\n<%s>\n", insertCommand);
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_INSERTFAILED), insertCommand, "TBL_InsertTable");
  }
  PQclear(res);
  return TBL_NORMAL;
}
/*
** INTERNAL USE FUNCTIONS **
*/
void
TBL_BeginInsertTransaction(void)
{
    return;
}

void
TBL_CommitInsertTransaction(void)
{
    return;
}

void
TBL_RollbackInsertTransaction(void)
{
    return;
}



/* TBL_Delete
**
** Purpose:
**	This deletes the records specified from the indicated table.
**
** Parameter Dictionary:
**	TBL_HANDLE **handle: The pointer for the database/table pair
**		to be accessed for deletion.  This table must be open.
**	TBL_CRITERIA *criteriaList: Contains a list of the criteria
**		to use when deleting records from the specified table.
**		A null list implies that all records will be deleted.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_BADHANDLE: The handle passed to the routine was invalid.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_DELETEFAILED: The delete operation failed most probably from
**		a bad specification in the criteriaList.  This error
**		return could result from a misspelled keyword, etc.
**
** Notes:
**	Nothing unusual.
**
** Algorithm:
**	The records selected by criteriaList are removed from the
**	database/table indicated by handle.
*/
CONDITION
TBL_Delete(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  char 				*tableName;
  int 				foundit;
  char 				deleteCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  tableName = tc->tableName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Delete");
  }

  strcpy(deleteCommand, "DELETE FROM ");
  strcat(deleteCommand, "srv_app.\"");
  strcat(deleteCommand, tableName);
  strcat(deleteCommand, "\"");
  
  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(deleteCommand, " WHERE ");
	  addCriteria(criteriaList, deleteCommand);
  }
  strcat(deleteCommand, ";" );

  if(G_Verbose) printf("SQL: %s\n",deleteCommand);
  res = PQexec(conn, deleteCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  fprintf(stderr, "<%s>\n", deleteCommand);
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_DELETEFAILED), deleteCommand, "TBL_Delete");
  }

  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}

CONDITION
TBL_DeleteTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, const char* tableName)
{
  TBL_CONTEXT* 		tc;
  char* 			dbName;
  int 				foundit;
  char 				deleteCommand[2048];
  PGresult* 		res;
  PGconn* 			conn;

#ifdef CTN_USE_THREADS
    THR_ObtainMutex(FAC_TBL);
#endif

  tc = G_ContextHead;
  foundit = 0;
  while (tc != (TBL_CONTEXT *) NULL) {
	  if (tc == (TBL_CONTEXT *) (*handle)) {
		  dbName = tc->databaseName;
		  conn = (PGconn*)tc->dbSpecific;
		  foundit = 1;
		  break;
	  }
	  tc = tc->next;
  }
  if (!foundit) {
#ifdef CTN_USE_THREADS
	  THR_ReleaseMutex(FAC_TBL);
#endif
	  return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_Delete");
  }

  strcpy(deleteCommand, "DELETE FROM ");
  strcat(deleteCommand, "srv_app.\"");
  strcat(deleteCommand, tableName);
  strcat(deleteCommand, "\"");

  if ((criteriaList != (TBL_CRITERIA *) NULL) && (criteriaList->FieldName != 0)) {
	  strcat(deleteCommand, " WHERE ");
	  addCriteria(criteriaList, deleteCommand);
  }
  strcat(deleteCommand, ";" );

  if(G_Verbose) printf("SQL: %s\n",deleteCommand);
  res = PQexec(conn, deleteCommand);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  fprintf(stderr, "<%s>\n", deleteCommand);
	  PQclear(res);
	  return COND_PushCondition(TBL_ERROR(TBL_DELETEFAILED), deleteCommand, "TBL_DeleteTable");
  }

  PQclear(res);

#ifdef CTN_USE_THREADS
  THR_ReleaseMutex(FAC_TBL);
#endif
  return TBL_NORMAL;
}



/* TBL_Layout
**
** Purpose:
**	This function returns the columns and types of a particular
**	table specified by handle.
**
** Parameter Dictionary:
**	char *databaseName: The name of the database to use.
**	char *tableName: The name of the table to access.
**	CONDITION (*callback)(): The callback function invoked whenever
**		a new record is retreived from the database.  It is
**		invoked with parameters as described below.
**	void *ctx: Ancillary data passed through to the callback function
**		and untouched by this routine.
**
** Return Values:
**	TBL_NORMAL: normal termination.
**	TBL_NOCALLBACK: No callback function was specified.
**	TBL_DBNOEXIST: The database specified does not exist.
**	TBL_TBLNOEXIST: The table specified did not exist in the correct
**		internal database table...this may indicate some sort
**		of consistency problem withing the database.
**	TBL_NOCOLUMNS: The table specified contains no columnns.
**	TBL_EARLYEXIT: The callback routine returned something other than
**		TBL_NORMAL which caused this routine to cancel the remainder
**		of the database operation and return early.
**
** Notes:
**	It is an error to specify a null callback function.
**
** Algorithm:
**	As each column is retrieved from the specified table, the
**	callback function is invoked as follows:
**
**		callback(fieldList *fieldList, void *ctx)
**
**	fieldList contains the field name and the type of the column from
**	the table specified.
**	ctx contains any additional information the user originally passed
**	to this layout function.  If callback returns any value other
**	than TBL_NORMAL, it is assumed that this function should terminate
**	(i.e. cancel the current db operation), and return an abnormal
**	termination message (TBL_EARLYEXIT) to the routine which
**	originally invoked TBL_Layout.
*/
CONDITION
TBL_Layout(char *databaseName, char *tableName, CONDITION(*callback) (), void *ctx) {
  TBL_FIELD 	field;
  int 			i;
  char 			descbuf[512];
  TBL_HANDLE* 	handle;
  TBL_CONTEXT* 	tc;
  CONDITION 	cond;
  PGresult 		*res;
  PGconn* 		conn;
  int 			nTuples;
  char 			lcTable[512];

  if (callback == NULL) return COND_PushCondition(TBL_ERROR(TBL_NOCALLBACK), "TBL_Layout");

  remapLower(tableName, lcTable);

  cond = TBL_Open(databaseName, tableName, &handle);
  if (cond != TBL_NORMAL) return cond;

  tc = (TBL_CONTEXT*)handle;
  conn = (PGconn*)tc->dbSpecific;

  strcpy(descbuf, "SELECT a.attname, t.typname, a.attlen ");
  strcat(descbuf, "FROM pg_class c, pg_attribute a, pg_type t ");
  strcat(descbuf, "WHERE c.relname = '");
  strcat(descbuf, lcTable);
  strcat(descbuf, "'");
  strcat(descbuf, "   and a.attnum > 0 ");
  strcat(descbuf, "   and a.attrelid = c.oid ");
  strcat(descbuf, "   and a.atttypid = t.oid ");

  if(G_Verbose) printf("SQL: %s\n",descbuf);
  res = PQexec(conn, descbuf);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	  fprintf(stderr, "PQresultError: %s\n", PQresultErrorMessage(res));
	  exit(1);
  }
  nTuples = PQntuples(res);

  for (i = 0; i < nTuples; i++) {
	  char *c;
	  field.FieldName = PQgetvalue(res, i, 0);
	  field.Value.AllocatedSize = atoi(PQgetvalue(res, i, 2));
	  field.Value.Type = TBL_SIGNED4;
	  c = PQgetvalue(res, i, 1);

	  if (strcmp(c, "bpchar") == 0){
		  field.Value.Type = TBL_STRING;
	  }else if (strcmp(c, "int4") == 0){
		  field.Value.Type = TBL_SIGNED4;
	  }else if (strcmp(c, "float8") == 0){
		  field.Value.Type = TBL_FLOAT8;
	  }else{
		  printf ("%s\n", c);
	  }

	  if (callback != NULL) {
		  if (callback(&field, ctx) != TBL_NORMAL) {
			  PQclear(res);
			  return COND_PushCondition(TBL_ERROR(TBL_EARLYEXIT), "TBL_Layout");
		  }
	  }
    PQclear(res);
  }
  return TBL_NORMAL;
}

CONDITION
TBL_SetEncoding(TBL_HANDLE ** handle, char * CharEncoding)
{
	TBL_CONTEXT* 		tc;
	char*				encoding;
	int 				foundit;
	PGconn* 			conn;
	CONDITION 			cond = 0;

	tc = G_ContextHead;
	foundit = 0;
	while (tc != (TBL_CONTEXT *) NULL) {
		if (tc == (TBL_CONTEXT *) (*handle)) {
			conn = (PGconn*)tc->dbSpecific;
			foundit = 1;
			break;
		}
		tc = tc->next;
	}
	if (!foundit) return COND_PushCondition(TBL_ERROR(TBL_BADHANDLE), "TBL_SetEncoding");

	if (strcmp(CharEncoding,"ISO_IR 100") == 0){
			encoding = "ISO88591";
	}else if(strcmp(CharEncoding,"ISO_IR 101") == 0){
			encoding = "ISO88592";
	}else if(strcmp(CharEncoding,"ISO_IR 109") == 0){
			encoding = "ISO88593";
	}else if(strcmp(CharEncoding,"ISO_IR 110") == 0){
			encoding = "ISO88594";
	}else if(strcmp(CharEncoding,"ISO_IR 144") == 0){
			encoding = "ISO88595";
	}else if(strcmp(CharEncoding,"ISO_IR 127") == 0){
			encoding = "ISO88596";
	}else if(strcmp(CharEncoding,"ISO_IR 126") == 0){
			encoding = "ISO88597";
	}else if(strcmp(CharEncoding,"ISO_IR 138") == 0){
			encoding = "ISO88598";
	}else if(strcmp(CharEncoding,"ISO_IR 148") == 0){
			encoding = "ISO88599";
	}else if(strcmp(CharEncoding,"ISO_IR 166") == 0){
			encoding = "ISO885911";
	}else if(strcmp(CharEncoding,"ISO_IR 192") == 0){
			encoding = "UTF8";
	}else{
			encoding = "UTF8";
	}

    if(G_Verbose) printf("SQL: %s\n", encoding);
	cond = PQsetClientEncoding(conn, encoding);
	if (cond != 0) return COND_PushCondition(TBL_ERROR(TBL_CHARSETFAILED), encoding, "TBL_SetEncoding");

	return TBL_NORMAL;
}


#else				/* If PSQL is not defined...just return the */

CONDITION
TBL_Open(const char *databaseName, const char *tableName, TBL_HANDLE ** handle)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Open");
}

CONDITION
TBL_Close(TBL_HANDLE ** handle)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Close");
}

CONDITION
TBL_Select(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_FIELD * fieldList, long *count, CONDITION(*callback) (), void *ctx)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Select");
}

CONDITION
TBL_Update(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Update");
}

CONDITION
TBL_Insert(TBL_HANDLE ** handle, TBL_FIELD * fieldList)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Insert");
}

CONDITION
TBL_Delete(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Delete");
}

CONDITION
TBL_Layout(char *databaseName, char *tableName, CONDITION(*callback) (), void *ctx) {
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_Layout");
}

CONDITION
TBL_NextUnique(TBL_HANDLE ** handle, char *name, int *unique)
{
    return COND_PushCondition(TBL_ERROR(TBL_UNIMPLEMENTED), "TBL_NextUnique");
}

void
TBL_BeginInsertTransaction(void)
{
    return;
}

void
TBL_RollbackInsertTransaction(void)
{
    return;
}

void
TBL_CommitInsertTransaction(void)
{
    return;
}

CONDITION
TBL_Debug(CTNBOOLEAN flag)
{
    return TBL_UNIMPLEMENTED;
}

CONDITION
TBL_SetOption(const char *string)
{
    return TBL_UNIMPLEMENTED;
}

int
TBL_HasViews(void)
{
    return TBL_UNIMPLEMENTED;
}

#endif
