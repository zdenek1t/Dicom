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
** Module Name(s):
** Author, Date:	Steve Moore, David E. Beecher, January 1994
** Intent:		Define typedefs and function prototypes for
**			TBL facility (for handling table operations).
** Last Update:		$Author: smm $, $Date: 1999/05/14 03:12:03 $
** Source File:		$RCSfile: tbl.h,v $
** Revision:		$Revision: 1.26 $
** Status:		$State: Exp $
*/


#ifndef _TBL_IS_IN
#define _TBL_IS_IN 1

#ifdef  __cplusplus
extern "C" {
#endif

typedef void TBL_HANDLE;

typedef enum {
    TBL_OTHER,
    TBL_UNSIGNED2, TBL_UNSIGNED4,
    TBL_SIGNED2, TBL_SIGNED4,
    TBL_FLOAT4, TBL_FLOAT8,
    TBL_STRING, TBL_TEXT,
    TBL_BINARYDATA, TBL_MBSTRING
}   TBL_DATATYPE;

typedef struct {
    TBL_DATATYPE 		Type;
    int 				AllocatedSize;
    int 				Size;
    int 				IsNull;
    union {
    	void 				*Other;
    	short 				*Signed2;
    	int 				*Signed4;
    	unsigned short 		*Unsigned2;
    	unsigned int 		*Unsigned4;
    	float 				*Float4;
    	double 				*Float8;
    	char 				*String;
    	char 				*Text;
    	void 				*BinaryData;
    }   				Value;
}   TBL_VALUE;

typedef enum {
    TBL_NULL, TBL_NOT_NULL, TBL_EQUAL, TBL_NOT_EQUAL,
    TBL_GREATER, TBL_GREATER_EQUAL,
    TBL_LESS, TBL_LESS_EQUAL, TBL_LIKE,
    TBL_NOP
}   TBL_OPERATOR;

typedef enum {
    TBL_SET, TBL_INCREMENT, TBL_DECREMENT, TBL_ZERO,
    TBL_ADD, TBL_SUBTRACT
}   TBL_FUNCTION;

typedef struct {
    char 			*FieldName;
    TBL_OPERATOR 	Operator;
    TBL_VALUE 		Value;
}   TBL_CRITERIA;

typedef struct {
    char 		*FieldName;
    TBL_VALUE 	Value;
}   TBL_FIELD;

typedef struct {
    char 			*FieldName;
    TBL_FUNCTION 	Function;
    TBL_VALUE 		Value;
}   TBL_UPDATE;

typedef struct _TBL_CONTEXT {
    int 				  refCount;
    char				  *databaseName,
						  *tableName,
						  *schemaName;
    void 				  *dbSpecific;
    struct _TBL_CONTEXT   *next;
}   TBL_CONTEXT;

#define TBL_LOAD_STRING(v, s)	(v)->Type = TBL_STRING; \
	(v)->Size = strlen((s)); \
	(v)->AllocatedSize = (v)->Size + 1; \
	(v)->IsNull = 0; \
	(v)->Value.String = (s);

#define	TBL_EXISTING_STRING(v, s) (v)->Type = TBL_STRING; \
	(v)->AllocatedSize = sizeof((s)); \
	(v)->Size = 0; \
	(v)->IsNull = 1; \
	(v)->Value.String = (s);

#define	TBL_FIELD_DECLARE_STRING(v, fname, str, siz) (v).FieldName = fname; \
	(v).Value.Type =TBL_STRING; \
	(v).Value.AllocatedSize = (siz); \
	(v).Value.Size = 0; \
	(v).Value.IsNull = 1; \
	(v).Value.Value.String = (str);

#define	TBL_FIELD_DECLARE_NUM(v, fname, type, d, siz) (v).FieldName = fname; \
	(v).Value.Type = (type); \
	(v).Value.AllocatedSize = (siz); \
	(v).Value.Size = 0; \
	(v).Value.IsNull = 1; \
	if (type == TBL_SIGNED4) \
		(v).Value.Value.Signed4 = (int *)&(d); \
	else if (type == TBL_FLOAT4) \
		(v).Value.Value.Float4 = (float *)&(d); \
	else if (type == TBL_FLOAT8) \
		(v).Value.Value.Float8 = (double *)&(d);

#define TBL_CRITERIA_LOAD_BYTE(v,fname,s,type,operator)  	\
        (v).FieldName = fname;			\
        (v).Operator = (TBL_OPERATOR)operator;	\
        (v).Value.Type = (TBL_DATATYPE)type;	\
        (v).Value.IsNull = 0;			\
        if( type == TBL_STRING ){		\
            (v).Value.Size=strlen((s));		\
            (v).Value.AllocatedSize=strlen((s))+1;\
            (v).Value.Value.String=(s);		\
        }

#define TBL_FIELD_LOAD_BYTE(v,fname,s,type)  	\
        (v).FieldName = fname;			\
        (v).Value.Type = (TBL_DATATYPE)type;	\
        (v).Value.IsNull = 0;			\
        if( type == TBL_STRING ){		\
            (v).Value.Size=strlen((s));		\
            (v).Value.AllocatedSize=strlen((s))+1;\
            (v).Value.Value.String=(s);		\
        }else if( type == TBL_TEXT){		\
            (v).Value.Size=strlen((s));		\
            (v).Value.AllocatedSize=strlen((s))+1;\
            (v).Value.Value.Text=(s);		\
        }

#define TBL_UPDATE_LOAD_BYTE(v,fname,s,type,f) 	\
        (v).FieldName = fname;			\
        (v).Function = f;			\
        (v).Value.Type = (TBL_DATATYPE)type;	\
        (v).Value.IsNull = 0;			\
        if( type == TBL_STRING ){		\
            (v).Value.Size=strlen((s));		\
            (v).Value.AllocatedSize=strlen((s))+1;\
            (v).Value.Value.String=(s);		\
        } else if (type == TBL_TEXT) {		\
            (v).Value.Size=strlen((s));		\
            (v).Value.AllocatedSize=strlen((s))+1;\
            (v).Value.Value.Text=(s);		\
        }

#define TBL_CRITERIA_LOAD_NUM(v,fname,s,type,operator)  	\
        (v).FieldName = fname;			\
        (v).Operator = (TBL_OPERATOR) operator;	\
        (v).Value.Type = (TBL_DATATYPE) type;	\
        (v).Value.IsNull = 0;			\
        if( type == TBL_SIGNED4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Signed4=(int *)&(s);\
        }else if( type == TBL_FLOAT4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Float4=(float *)&(s);\
        }else if( type == TBL_FLOAT8 ){		\
            (v).Value.Size=8;			\
            (v).Value.AllocatedSize=8;		\
            (v).Value.Value.Float8=(double *)&(s);\
        }

#define TBL_FIELD_LOAD_NUM(v,fname,s,type)  	\
        (v).FieldName = fname;			\
        (v).Value.Type = type;			\
        (v).Value.IsNull = 0;			\
        if( type == TBL_SIGNED4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Signed4=(int *)&(s);\
        }else if( type == TBL_FLOAT4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Float4=(float *)&(s);\
        }else if( type == TBL_FLOAT8 ){		\
            (v).Value.Size=8;			\
            (v).Value.AllocatedSize=8;		\
            (v).Value.Value.Float8=(double *)&(s);\
        }

#define TBL_UPDATE_LOAD_NUM(v,fname,s,type,f)  	\
        (v).FieldName = fname;			\
        (v).Function = f;			\
        (v).Value.Type = type;			\
        (v).Value.IsNull = 0;			\
        if( type == TBL_SIGNED4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Signed4=(int *)&(s);\
        }else if( type == TBL_FLOAT4 ){		\
            (v).Value.Size=4;			\
            (v).Value.AllocatedSize=4;		\
            (v).Value.Value.Float4=(float *)&(s);\
        }else if( type == TBL_FLOAT8 ){		\
            (v).Value.Size=8;			\
            (v).Value.AllocatedSize=8;		\
            (v).Value.Value.Float8=(double *)&(s);\
        }

#define	TBL_FIELD_STRING(name, string) (name), TBL_STRING, \
sizeof((string)), 0, 1, (void *)(string)

/* Define the function prototypes for this facility. */

CONDITION
TBL_Open(const char *databaseName, const char *tableName, TBL_HANDLE ** handle);
CONDITION
TBL_OpenDB(const char *databaseName, TBL_HANDLE ** handle);
CONDITION
TBL_Close(TBL_HANDLE ** handle);

CONDITION
TBL_Select(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList,  TBL_FIELD * fieldList, long *count, CONDITION(*callback) (TBL_FIELD*, long, void*), void *ctx);
CONDITION
TBL_SelectTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_FIELD * fieldList, long *count, CONDITION(*callback) (TBL_FIELD*, long, void*), void *ctx, const char* tableName);
CONDITION
TBL_Update(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList);
CONDITION
TBL_UpdateTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, TBL_UPDATE * updateList, const char* tableName);
CONDITION
TBL_Insert(TBL_HANDLE ** handle, TBL_FIELD * fieldList);
CONDITION
TBL_InsertTable(TBL_HANDLE ** handle, TBL_FIELD * fieldList, const char* tablename);
CONDITION
TBL_Delete(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList);
CONDITION
TBL_DeleteTable(TBL_HANDLE ** handle, const TBL_CRITERIA * criteriaList, const char* tableName);
CONDITION
TBL_Layout(char *databaseName, char *tableName, CONDITION(*callback) (), void *ctx);
CONDITION
TBL_NextUnique(TBL_HANDLE ** handle, char *name, int *unique);
CONDITION
TBL_Debug(CTNBOOLEAN flag);

int
TBL_HasViews(void);
int
TBL_HasUpdateIncrement(void);

char
*TBL_Message(CONDITION condition);

void
TBL_BeginInsertTransaction(void);
void
TBL_CommitInsertTransaction(void);
void
TBL_RollbackInsertTransaction(void);

CONDITION
TBL_SetOption(const char* string);

CONDITION
TBL_SetEncoding(TBL_HANDLE ** handle, char * CharEncoding);

CONDITION
TBL_ConnectDB(void);
CONDITION
TBL_CloseDB(void);


/* Define condition values */

#define	TBL_NORMAL				FORM_COND(FAC_TBL, SEV_SUCC, 1)
#define	TBL_UNIMPLEMENTED		FORM_COND(FAC_TBL, SEV_ERROR, 2)
#define	TBL_MALLOCFAILURE		FORM_COND(FAC_TBL, SEV_ERROR, 3)
#define	TBL_OPENFAILED			FORM_COND(FAC_TBL, SEV_ERROR, 4)
#define	TBL_FILEOPENFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 5)
#define	TBL_ILLEGALFORMAT		FORM_COND(FAC_TBL, SEV_ERROR, 6)
#define	TBL_LISTCREATEFAILURE	FORM_COND(FAC_TBL, SEV_ERROR, 7)
#define	TBL_LISTFAILURE			FORM_COND(FAC_TBL, SEV_ERROR, 8)
#define	TBL_ALREADYOPENED		FORM_COND(FAC_TBL, SEV_ERROR, 9)
#define	TBL_DBNOEXIST			FORM_COND(FAC_TBL, SEV_ERROR, 10)
#define	TBL_TBLNOEXIST			FORM_COND(FAC_TBL, SEV_ERROR, 11)
#define	TBL_NOMEMORY			FORM_COND(FAC_TBL, SEV_ERROR, 12)
#define	TBL_CLOSERROR			FORM_COND(FAC_TBL, SEV_ERROR, 13)
#define	TBL_BADHANDLE			FORM_COND(FAC_TBL, SEV_ERROR, 14)
#define	TBL_NOFIELDLIST			FORM_COND(FAC_TBL, SEV_ERROR, 15)
#define	TBL_SELECTFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 16)
#define	TBL_EARLYEXIT			FORM_COND(FAC_TBL, SEV_ERROR, 17)
#define	TBL_DELETEFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 18)
#define	TBL_INSERTFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 18)
#define	TBL_UPDATEFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 19)
#define	TBL_DBINITFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 20)
#define	TBL_NOCOLUMNS			FORM_COND(FAC_TBL, SEV_ERROR, 21)
#define	TBL_NOCALLBACK			FORM_COND(FAC_TBL, SEV_ERROR, 22)
#define	TBL_DBSPECIFIC			FORM_COND(FAC_TBL, SEV_ERROR, 23)
#define	TBL_CHARSETFAILED		FORM_COND(FAC_TBL, SEV_ERROR, 24)

#ifdef  __cplusplus
}
#endif

#endif
