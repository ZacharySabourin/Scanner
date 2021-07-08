/**
* File Name: table.h
* Compiler: MS Visual Studio 2019
* Author: Zachary Sabourin
* Course: CST 8152 - Compilers, Lab Section: 012
* Assignment: A2
* Date: November 14, 2020
* Professor: Paulo Sousa
* Purpose:
		Holds all of the accept state funcion declarations, state constants, 
		stateType values, and keyword values.
* Function List:
*		Token aStateFuncAVID(char lexeme[]);
*		Token aStateFuncSVID(char lexeme[]);
*		Token aStateFuncIL(char lexeme[]);
*		Token aStateFuncFPL(char lexeme[]);
*		Token aStateFuncSL(char lexeme[]);
*		Token aStateFuncErr(char lexeme[]);
*/

/* Macro section */
#ifndef  TABLE_H_
#define  TABLE_H_ 

#ifndef BUFFER_H_
#include "buffer.h"
#endif

#ifndef TOKEN_H_
#include "token.h"
#endif

#ifndef NULL
#include <_null.h> /* NULL pointer constant is defined there */
#endif

 /*  Source end-of-file (SEOF) sentinel symbol
  *    '\0' or one of 255,0xFF,EOF
  */
#define CHARSEOF0 '\0'
#define CHARSEOF255 255

/*  Special case tokens processed separately one by one
 *  in the token-driven part of the scanner
 *  '=' , ' ' , '(' , ')' , '{' , '}' , == , <> , '>' , '<' , ';',
 *  white space
 *  !!comment , ',' , ';' , '-' , '+' , '*' , '/', ## ,
 *  _AND_, _OR_, _NOT_ , SEOF,
 */

#define ES	11		/* Error state  with no retract */
#define ER  12		/* Error state  with retract */
#define IS  -1		/* Illegal state */

#define MAX_INT_P 32767

 /* State transition table definition */
#define TABLE_COLUMNS 8

/* Transition table - type of states defined in separate table */
int  transitionTable[][TABLE_COLUMNS] = {
	/*				[A-z](0),	0(1),	[1-9](2),	.(3),	$(4),	"(5),	SEOF(6),	other(7)	*/
	/*				L,			Z,		N,			P,		S,		Q,		E,			O			*/
	/* State 00 */  {1,			6,		4,			ES,		ES,		9,		ER,			ES},
	/* State 01 */  {1,			1,		1,			2,		3,		2,		2,			2},
	/* State 02 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 03 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 04 */  {ES,		4,		4,			7,		5,		ES,		5,			5},
	/* State 05 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 06 */  {ES,		6,		ES,			7,		5,		ES,		5,			5},
	/* State 07 */  {8,			7,		7,			8,		8,		8,		8,			8},
	/* State 08 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 09 */  {9,			9,		9,			9,		9,		10,		ER,			9},
	/* State 10 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 11 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS},
	/* State 12 */  {IS,		IS,		IS,			IS,		IS,		IS,		IS,			IS}
};

/* Accepting state table definition */
#define ASWR	2	/* accepting state with retract */
#define ASNR	1	/* accepting state with no retract */
#define NOAS	0	/* not accepting state */

/* List of acceptable states */
int stateType[] = {
	NOAS,	/* State 00 */
	NOAS,	/* State 01 */
	ASWR,	/* State 02 */
	ASNR,	/* State 03 */
	NOAS,	/* State 04 */
	ASWR,	/* State 05 */
	NOAS,	/* State 06 */
	NOAS,	/* State 07 */
	ASWR,	/* State 08 */
	NOAS,	/* State 09 */
	ASNR,	/* State 10 */
	ASNR,	/* State 11 */
	ASWR	/* State 12 */
};

/* Accepting action function declarations */
Token aStateFuncAVID(char lexeme[]);
Token aStateFuncSVID(char lexeme[]);
Token aStateFuncIL(char lexeme[]);
Token aStateFuncFPL(char lexeme[]);
Token aStateFuncSL(char lexeme[]);
Token aStateFuncErr(char lexeme[]);

/* Defining a new type: pointer to function (of one char * argument)
   returning Token
*/
typedef Token(*PTR_AAF)(char* lexeme);

/* Accepting function (action) callback table (array) definition */
/* If you do not want to use the typedef, the equvalent declaration is:
 * Token (*finalStateTable[])(char lexeme[]) = {
 */
PTR_AAF finalStateTable[] = {
	NULL,			/* State 0 */
	NULL,			/* State 1 */
	aStateFuncAVID,	/* State 2 */
	aStateFuncSVID,	/* State 3 */
	NULL,			/* State 4 */
	aStateFuncIL,	/* State 5 */
	NULL,			/* State 6 */
	NULL,			/* State 7 */
	aStateFuncFPL,	/* State 8 */
	NULL,			/* State 9 */
	aStateFuncSL,	/* State 10 */
	aStateFuncErr,	/* State 11 */
	aStateFuncErr	/* State 12 */
};

/* Keyword table size */
#define KWT_SIZE 10

/* Keyword list */
char* keywordTable[] = {
	"PROGRAM",
	"IF",
	"THEN",
	"ELSE",
	"WHILE",
	"DO",
	"INPUT",
	"OUTPUT",
	"TRUE",
	"FALSE"
};

#endif
