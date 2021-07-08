 /*
 * File Name: scanner.c
 * Compiler: MS Visual Studio 2019
 * Author: Zachary Sabourin
 * Course: CST 8152 - Compilers, Lab Section: 012
 * Assignment: A2
 * Date: November 14, 2020
 * Professor: Paulo Sousa
 * Purpose: The scanner reads a source program from a text file and produces a stream of token
 *			representations. This file contains the processing functions for every token.
 * Function List:
 *		int initScanner(pBuffer psc_buf);
 *		Token processToken(void) ;
 *		static int nextTokenClass(char c);
 *		static int getNextState(int, char);
 *		static int isKeyword(char* kw_lexeme);
 *		Token aStateFuncAVID(char lexeme[]);
 *		Token aStateFuncSVID(char lexeme[]);
 *		Token aStateFuncIL(char lexeme[]);
 *		Token aStateFuncFPL(char lexeme[]);
 *		Token aStateFuncSL(char lexeme[]);
 *		Token aStateFuncErr(char lexeme[]); * 
 */

 /* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
  * to suppress the warnings about using "unsafe" functions like fopen()
  * and standard string library functions defined in string.h.
  * The define does not have any effect in Borland compiler projects.
  */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>   /* standard input / output */
#include <ctype.h>   /* conversion functions */
#include <stdlib.h>  /* standard library functions and constants */
#include <string.h>  /* string functions */
#include <limits.h>  /* integer types constants */
#include <float.h>   /* floating-point types constants */

  /*#define NDEBUG        to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

/* Global objects - variables */
/* This buffer is used as a repository for string literals.
   It is defined in platy_st.c */
extern pBuffer stringLiteralTable;		/* String literal table */
int line;								/* current line number of the source code */
extern int errorNumber;					/* defined in platy_st.c - run-time error number */

static char debugMode = 0;				/* optional for debugging */

/* Local(file) global objects - variables */
static pBuffer lexemeBuffer;			/* pointer to temporary lexeme buffer */
static pBuffer sourceBuffer;			/* pointer to input source buffer */
/* No other global variable declarations/definitiond are allowed */

/* scanner.c static(local) function  prototypes */
static int nextTokenClass(char c);		/* character class function */
static int getNextState(int, char);		/* state machine function */
static int isKeyword(char* kw_lexeme);	/* keywords lookup functuion */


/*
*	Function name: initScanner(pBuffer psc_buf)
*	Purpose: Initializes the scanner.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		bufferIsEmpty(Buffer* const pBE)
*		bufferRewind(Buffer* const pBE)
*		bufferClear(Buffer* const pBE)
*	Parameters:
*		pBuffer psc_buf
*	Return value:
*		EXIT_FAILURE
*		EXIT_SUCCESS
*	Algorithm:
*		Check if the buffer is empty. If it is, return EXIT_FAILURE. If not
*		rewind the buffer, clear the buffer, set the line number, set the source buffer,
*		and return EXIT_SUCCESS.
*/
int initScanner(pBuffer psc_buf) 
{
	if (bufferIsEmpty(psc_buf)) 
		return EXIT_FAILURE;/*1*/
	/* in case the buffer has been read previously  */
	bufferRewind(psc_buf);
	bufferClear(stringLiteralTable);
	line = 1;
	sourceBuffer = psc_buf;
	return EXIT_SUCCESS;/*0*/
/*   scerrnum = 0;  *//*no need - global ANSI C */
}


/*
*	Function name: processToken(void)
*	Purpose: Processes each token read from the buffer to identify them.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		bufferGetChar(Buffer* const pBE);
*		bufferRetract(Buffer* const pBE);
*		getNextState(int state, char c);
*		bufferGetCPosition(Buffer* const pBE);
*		bufferSetMarkPosition(Buffer* const pBE, short mark);
*		bufferCreate(short initCapacity, char incFactor, char opMode);
*		bufferReset(Buffer* const pBE);
*		bufferAddChar(Buffer* const pBE, char symbol);
*		bufferFree(lexemeBuffer);
*		aStateFuncAVID(char lexeme[]);
*		aStateFuncSVID(char lexeme[]);
*		aStateFuncIL(char lexeme[]);
*		aStateFuncFPL(char lexeme[]);
*		aStateFuncSL(char lexeme[]);
*		aStateFuncErr(char lexeme[]);
*	Parameters:
*	Return value:
*		Token currentToken;
*	Algorithm:
*		Get a charecter from the source buffer and identify which token it is.
*		Check for a variety of special case tokens, relational operators, assignment operators,
*		logical operators, end-of-file, AVID, SVIP IL, FPL, SL, and error tokens.
*/
Token processToken(void) 
{
	Token currentToken = { 0 }; /* token to return after pattern recognition. Set all structure members to 0 */
	unsigned char c;	/* input symbol */
	unsigned char newc;	/* new char */

	int state = 0;		/* initial state of the FSM */
	int i;				/* counter */

	short lexStart;		/* start offset of a lexeme in the input char buffer (array) */
	short lexEnd;		/* end offset of a lexeme in the input char buffer (array)*/
	short lexLength;	/* token length */

	/* endless loop broken by token returns it will generate a warning */
	while (1) 
	{ 
		c = bufferGetChar(sourceBuffer);

		switch (c) 
		{
			/******************************************
			*	Special cases token processing
			* *****************************************/
			case ' ':	/* Ignore empty spaces */
				break;

			case '\t':	/* Ignore tabs */
				break;

			case '\n':	/* Ignore newlines but increment the line counter */
				++line;
				break;

			case '#':
				/* Illegal character. Log character into error token */
				currentToken.code = ERR_T;
				currentToken.attribute.err_lex[0] = c;
				return currentToken;

			case '(':	/* Left parenthesis token */
				currentToken.code = LPR_T;
				return currentToken;

			case ')':	/* Right parenthesis token */
				currentToken.code = RPR_T;
				return currentToken;

			case '{':	/* Left bracket token */
				currentToken.code = LBR_T;
				return currentToken;

			case '}':	/* Right bracket token */
				currentToken.code = RBR_T;
				return currentToken;	

			case ',':	/* Comma token */
				currentToken.code = COM_T;
				return currentToken;

			case ';': /* End-of-statment token */
				currentToken.code = EOS_T;
				return currentToken;

			case CHARSEOF0:	/* End-of-file/null character token token */
				currentToken.code = SEOF_T;
				currentToken.attribute.seof = SEOF_0;
				return currentToken;
			
			/******************************************
			*	String Concatenation token processing
			* *****************************************/
			case '$':
				/* If next character is '$', log a string-concatenation token */
				if (bufferGetChar(sourceBuffer) == '$')
				{
					currentToken.code = SCC_OP_T;
					return currentToken;
				}

				/* Retract to previous character, log the error, and return the token */
				if (bufferRetract(sourceBuffer) == RT_FAIL_1)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				currentToken.code = ERR_T;
				currentToken.attribute.err_lex[0] = c;
				return currentToken;

			/******************************************
			*	Relational Operator processing
			* *****************************************/
			case '=':
				/* If next character is '=', log relational operator EQUALS */
				if (bufferGetChar(sourceBuffer) == '=')
				{
					currentToken.code = REL_OP_T;
					currentToken.attribute.rel_op = EQ;
					return currentToken;
				}

				/* Retract buffer. Token is assignment operator */
				if (bufferRetract(sourceBuffer) == RT_FAIL_1)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				currentToken.code = ASS_OP_T;
				return currentToken;

			case '<':
				currentToken.code = REL_OP_T;

				/* If next character is '>', log relational operator NOT EQUALS */
				if (bufferGetChar(sourceBuffer) == '>')
				{
					currentToken.attribute.rel_op = NE;
					return currentToken;
				}

				/* Retract buffer. Token is LESS THAN operator */
				if (bufferRetract(sourceBuffer) == RT_FAIL_1)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				currentToken.attribute.rel_op = LT;
				return currentToken;

			case '>': /* GREATER THAN operator token */
				currentToken.code = REL_OP_T;
				currentToken.attribute.rel_op = GT;
				return currentToken;

			/******************************************
			*	Arithmetic Operator token processing
			* *****************************************/
			case '+':	/* ADDITION operator token */
				currentToken.code = ART_OP_T;
				currentToken.attribute.arr_op = ADD;
				return currentToken;

			case '-':	/* SUBTRACT operator token */
				currentToken.code = ART_OP_T;
				currentToken.attribute.arr_op = SUB;
				return currentToken;

			case '*':	/* MULTIPLICATION operator token */
				currentToken.code = ART_OP_T;
				currentToken.attribute.arr_op = MUL;
				return currentToken;

			case '/':	/* DIVISION operator token */
				currentToken.code = ART_OP_T;
				currentToken.attribute.arr_op = DIV;
				return currentToken;

			/******************************************
			*	Logic Operator token processing
			* *****************************************/
			case '_':;
				
				newc = bufferGetChar(sourceBuffer);
				int retractAmount = 1; /* The amount to retract in case of error */

				/* Check for reserved word "_AND_" */
				if (newc == 'A')
				{
					newc = bufferGetChar(sourceBuffer);
					++retractAmount;

					if (newc == 'N')
					{
						newc = bufferGetChar(sourceBuffer);
						++retractAmount;

						if (newc == 'D')
						{
							newc = bufferGetChar(sourceBuffer);
							++retractAmount;

							if (newc == '_')
							{	
								/* Log token as logical operator AND */
								currentToken.code = LOG_OP_T;
								currentToken.attribute.log_op = AND;
								return currentToken;
							}
						}
					}
				}

				/* Check for reserved word "_OR_" */
				else if (newc == 'O')
				{
					newc = bufferGetChar(sourceBuffer);
					++retractAmount;

					if (newc == 'R')
					{
						newc = bufferGetChar(sourceBuffer);
						++retractAmount;

						if (newc == '_')
						{
							/* Log token as logical operator OR */
							currentToken.code = LOG_OP_T;
							currentToken.attribute.log_op = OR;
							return currentToken;
						}					
					}
				}

				/* Check for reserved word "_NOT_" */
				else if (newc == 'N')
				{
					newc = bufferGetChar(sourceBuffer);
					++retractAmount;

					if (newc == 'O')
					{
						newc = bufferGetChar(sourceBuffer);
						++retractAmount;

						if (newc == 'T')
						{
							newc = bufferGetChar(sourceBuffer);
							++retractAmount;

							if (newc == '_')
							{
								/* Log token as logical operator NOT */
								currentToken.code = LOG_OP_T;
								currentToken.attribute.log_op = NOT;
								return currentToken;
							}
						}
					}
				}

				/* We've encountered an error. Retract to the starting point and log the error token */
				for (i = 0; i < retractAmount; ++i)
				{
					if (bufferRetract(sourceBuffer) == RT_FAIL_1)
					{
						errorNumber = 0;
						return aStateFuncErr("RUN-TIME ERROR");
					}
				}

				currentToken.code = ERR_T;
				currentToken.attribute.err_lex[0] = c;
				currentToken.attribute.err_lex[1] = CHARSEOF0;
				return currentToken;

			/******************************************
			*	Comment token processing
			* *****************************************/
			case '!':
				newc = bufferGetChar(sourceBuffer);

				/* Comment found if next char is '!' */
				if (newc == '!')
				{
					/* Ignore the rest of the line and increment the line counter */
					while (newc != '\n' && newc != CHARSEOF0)
						newc = bufferGetChar(sourceBuffer);
					++line;
					break;			
				}
				
				/* Set the error code, add the characters to the err_lex, and null-terminate the string */
				currentToken.code = ERR_T;
				currentToken.attribute.err_lex[0] = c;
				currentToken.attribute.err_lex[1] = newc;
				currentToken.attribute.err_lex[2] = CHARSEOF0;

				/* Ignore the rest of the line, increment line counter, and return the token */
				while (newc != '\n' && newc != CHARSEOF0)
					newc = bufferGetChar(sourceBuffer);
				++line;
				return currentToken;

			/******************************************
			*	Transition table state processing
			* *****************************************/
			default:
				/* Get the next state from the transition table, log the starting position of the lexeme,
					and set the mark position of the source buffer for future retraction */
				state = getNextState(state, c);
				lexStart = bufferGetCPosition(sourceBuffer);

				if(lexStart == RT_FAIL_1 || !bufferSetMarkPosition(sourceBuffer, lexStart - 1))
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				/* Continue checking characters until we encounter an accepting state */
				while (stateType[state] == NOAS)
				{
					newc = bufferGetChar(sourceBuffer);
					state = getNextState(state, newc);
				}

				/* Accepting state with retract */
				if (stateType[state] == ASWR)
				{
					if (bufferRetract(sourceBuffer) == RT_FAIL_1)
					{
						errorNumber = 0;
						return aStateFuncErr("RUN-TIME ERROR");
					}
				}

				/* Set the end and length of the lexeme */
				lexEnd = bufferGetCPosition(sourceBuffer);
				if (lexEnd == RT_FAIL_1)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				lexLength = lexEnd - lexStart + 1;

				/* Create a lexeme buffer with lexLength+1 size to allow for the '\0' to be added at the end */
				lexemeBuffer = bufferCreate(lexLength + 1, 0, 'f');
				if (!lexemeBuffer)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				/* Retract the source buffer to the mark position and transfer the contents to the lexeme buffer */
				if (bufferReset(sourceBuffer) == RT_FAIL_1)
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				/* Transfer The contents to the lexeme buffer */
				for (i = 0; i < lexLength; ++i)
				{
					if (!bufferAddChar(lexemeBuffer, bufferGetChar(sourceBuffer)))
					{
						errorNumber = 0;
						return aStateFuncErr("RUN-TIME ERROR");
					}
				}

				/* NULL-terminate the string */
				if (!bufferAddChar(lexemeBuffer,CHARSEOF0))
				{
					errorNumber = 0;
					return aStateFuncErr("RUN-TIME ERROR");
				}

				/* Call the appropriate function from the final state table based on the current state */
				currentToken = (*finalStateTable[state])(bufferGetString(lexemeBuffer, 0));
				bufferFree(lexemeBuffer);
				return currentToken;
		}
	}
}

/*
*	Function name: getNextState(int state, char c)
*	Purpose: Identify the next state of the scanner based on the current symbol.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		fprintf()
*		printf()
*		assert(expression)
*		nextTokenClass(char c)
*		exit();
*	Parameters:
*		int state
*		char c
*	Return value:
*		int next
*	Algorithm:
*		Based on the current state and symbol, finds the next state from the transition table.
*		If the next state is an illegal state, the program will exit and identify the area of error. 
*/
int getNextState(int state, char c) 
{
	int col;
	int next;
	if (debugMode)
		fprintf(stderr, "[03] getNextState\n");
	col = nextTokenClass(c);
	next = transitionTable[state][col];
#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n", c, state, col, next);
#endif
	/*
	The assert(int test) macro can be used to add run-time diagnostic to programs
	and to "defend" from producing unexpected results.
	assert() is a macro that expands to an if statement;
	if test evaluates to false (zero) , assert aborts the program
	(by calling abort()) and sends the following message on stderr:

	Assertion failed: test, file filename, line linenum

	The filename and linenum listed in the message are the source file name
	and line number where the assert macro appears.
	If you place the #define NDEBUG directive ("no debugging")
	in the source code before the #include <assert.h> directive,
	the effect is to comment out the assert statement.
	*/
	assert(next != IS);

	/*
	The other way to include diagnostics in a program is to use
	conditional preprocessing as shown bellow. It allows the programmer
	to send more details describing the run-time problem.
	Once the program is tested thoroughly #define DEBUG is commented out
	or #undef DEBUF is used - see the top of the file.
	*/
#ifdef DEBUG
	if (next == IS) 
	{
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n", c, state, col);
		exit(1);
	}
#endif
	return next;
}

/*
*	Function name: nextTokenClass(char c)
*	Purpose: Identifies the transition table position of a character
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		isalpha(int _C)
*		isdigit(int _C)
*	Parameters:
*		char c
*	Return value:
*		int val
*	Algorithm:
*		Return the value of the characters transition table column.
*		Returns 1 to 7 if found, -1 if not found.
*/
int nextTokenClass(char c) 
{
	int val = -1;
	if (debugMode)
		fprintf(stderr, "[04] NextTokenClass\n");

	if (isalpha(c))	/* [A-z](0) */
		val = 0;
	else if (isdigit(c))
	{
		if ((c - '0') == 0)	/* 0(1) */
			val = 1;
		else	/* [1-9](2) */
			val = 2;
	}
	else if (c == '.')	/* .(3) */
		val = 3;
	else if (c == '$')	/* $(4) */
		val = 4;
	else if (c == '"')	/* "(5) */
		val = 5;
	else if (c == CHARSEOF0) /* SEOF(6) */
		val = 6;
	else	/* other(7) */
		val = 7;

	return val;
}

/*
*	Function name: aStateFuncAVID(char lexeme[])
*	Purpose: Stores the input string into the token
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strlen(const char *_Str)
*	Parameters:
*		char lexeme[]
*	Return value:
*		Token currentToken
*	Algorithm:
*		Check if the lexeme is a keyword. If it is, set the token code to KW_T and
*		the token's keyword index to the index of the keyword. If not, transfer
*		the contents of the lexeme into the variable identifier token attribute. 
*		Change the token code to AVID_T. Returns the AVID token or the KW token.
*/
Token aStateFuncAVID(char lexeme[]) 
{
	Token currentToken = { 0 };
	unsigned int i;

	/* If lexeme is a keyword, store the keyword index into the attribute and KW_T the keyword code */
	i = isKeyword(lexeme);
	if (i != -1)
	{
		currentToken.code = KW_T;
		currentToken.attribute.kwt_idx = i;
		return currentToken;
	}
	
	/* Determine the amount of characters to store into the token */
	unsigned int length = (strlen(lexeme) > VID_LEN) ? VID_LEN : strlen(lexeme);

	/* Transfer the contents into the token attribute */
	for (i = 0; i < length; ++i)
		currentToken.attribute.vid_lex[i] = lexeme[i];

	currentToken.attribute.vid_lex[i] = CHARSEOF0; /* Null-terminate the string */
	currentToken.code = AVID_T; /* Set the token code */
	return currentToken;
}

/*
*	Function name: aStateFuncSVID(char lexeme[])
*	Purpose: Stores the input string into the token
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strlen(const char *_Str)
*	Parameters:
*		char lexeme[]
*	Return value:
*		Token currentToken
*	Algorithm:
*		Transfer the contents of the lexeme into the variable identifier
*		token attribute. Change the token code to SVID_T. Returns the SVID token.
*/
Token aStateFuncSVID(char lexeme[]) 
{
	Token currentToken = { 0 };
	unsigned int i;

	/* If the lexeme is longer than VID_LEN, only log VID_LEN - 1 characters 
		and add a '$' at the end */
	if (strlen(lexeme) > VID_LEN)
	{
		for (i = 0; i < VID_LEN - 1; ++i)
			currentToken.attribute.vid_lex[i] = lexeme[i];
		currentToken.attribute.vid_lex[i++] = '$';
	}
	
	/* Log the enitre lexeme */
	else
		for (i = 0; i < strlen(lexeme); ++i)
			currentToken.attribute.vid_lex[i] = lexeme[i];

	/* Terminte the string and set the token code */
	currentToken.attribute.vid_lex[i] = CHARSEOF0;
	currentToken.code = SVID_T;
	return currentToken;
}

/*
*	Function name: aStateFuncIL(char lexeme[])
*	Purpose: Stores the input integer literal into the token
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		atol(const char *_String)
*		aStateFuncErr(char lexeme[])
*	Parameters:
*		char lexeme[]
*	Return value:
*		Exceeded range: aStateFuncErr(char lexeme[])
*		Token currentToken
*	Algorithm:
*		Transfer the converted contents of the lexeme into the integer literal
*		token attribute. Change the token code to INL_T. Returns the INL token.
*		Returns the error token if the value exceeds the range of a 2-byte integer.
*/
Token aStateFuncIL(char lexeme[]) 
{
	Token currentToken = { 0 };
	long decValue = strtol(lexeme, NULL, 10); /* convert the string to a long */

	/* Value cannot exceed the range of a 2-byte integer in c */
	if (decValue > SHRT_MAX || decValue < SHRT_MIN)
		return aStateFuncErr(lexeme);

	/* Assign value and code */
	currentToken.attribute.int_value = decValue;
	currentToken.code = INL_T;
	return currentToken;
}

/*
*	Function name: aStateFuncFPL(char lexeme[])
*	Purpose: Stores the input floating-point literal into the token
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strtod(const char *_String, char **_EndPtr)
*		aStateFuncErr(char lexeme[])
*	Parameters:
*		char lexeme[]
*	Return value:
*		Exceeded range: aStateFuncErr(char lexeme[])
*		Token currentToken
*	Algorithm:
*		Transfer the converted contents of the lexeme into the floating-point literal
*		token attribute. Change the token code to FPL_T. Returns the FPL token.
*		Returns the error token if the value exceeds the range of a 4-byte float.
*/
Token aStateFuncFPL(char lexeme[]) 
{
	Token currentToken = { 0 };
	double fplValue = strtod(lexeme, NULL); /* Convert the string to a double */

	/* Value cannot exceed the range of a 4-byte float in c*/
	if ((fplValue > FLT_MAX || fplValue < FLT_MIN) && fplValue != 0) 
		return aStateFuncErr(lexeme);
	
	/* Assign value and code */
	currentToken.attribute.flt_value = (float)fplValue;
	currentToken.code = FPL_T;
	return currentToken;
}

/*
*	Function name: aStateFuncSL(char lexeme[])
*	Purpose: Stores the input string literal into string literal table.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strlen(const char* _Str)
*		bufferAddChar(const pBuffer pBE, char symbol)
*	Parameters:
*		char lexeme[]
*	Return value:
*		Token currentToken
*	Algorithm:
*		Transfer the contents of the lexeme into the string literal table.
*		Change the token code to STR_T. Returns the SL token.
*/
Token aStateFuncSL(char lexeme[]) 
{
	Token currentToken = { 0 };
	currentToken.attribute.str_offset =	bufferGetAddCPosition(stringLiteralTable);

	unsigned int i;
	for (i = 0; i < strlen(lexeme); ++i) 
	{
		if (lexeme[i] == '\n') /* Increment the line counter */
			++line;
		if (lexeme[i] != '"') /* Add all but the '"' to the string literal table */
		{
			if (!bufferAddChar(stringLiteralTable, lexeme[i]))
			{
				errorNumber = 0;
				return aStateFuncErr("RUN-TIME ERROR");
			}		
		}		
	}

	/* Append the null character to the string, set the token code, and return the token. */
	if (!bufferAddChar(stringLiteralTable, CHARSEOF0))
	{
		errorNumber = 0;
		return aStateFuncErr("RUN-TIME ERROR");
	}

	currentToken.code = STR_T;
	return currentToken;
}

/*
*	Function name: aStateFuncErr(char lexeme[])
*	Purpose: Stores the error into the error token attribute
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strlen(const char* _Str)
*	Parameters:
*		char lexeme[]
*	Return value:
*		Token currentToken
*	Algorithm:
*		Transfer the contents of the lexeme into the error token attribute. 
*		Adds elipses to the end of the string if it's longer than ERR_LEN.
*		Change the token code to ERR_T. Returns the ERR token.
*/
Token aStateFuncErr(char lexeme[]) 
{
	Token currentToken = { 0 };
	unsigned int i;

	/* If the lexeme is longer than ERR_LEN, add "..." to the end of the token */
	if (strlen(lexeme) > ERR_LEN)
	{
		/* Transfer all lexeme contents into the token */
		for (i = 0; i < ERR_LEN; ++i)
		{
			if (lexeme[i] == '\n')
				++line;

			/* Fill the last 3 positions with "..." */
			currentToken.attribute.err_lex[i] = (i < ERR_LEN - 3) ? lexeme[i] : '.';
		}
	}

	/* Add the entire lexeme to the token */
	else
		for (i = 0; i < strlen(lexeme); ++i)
			currentToken.attribute.err_lex[i] = lexeme[i];
		
	/* Null terminate the string, set the error token attribute, and return the token */
	currentToken.attribute.err_lex[i] = CHARSEOF0;
	currentToken.code = ERR_T;
	return currentToken;
}

/*
*	Function name: isKeyword(char *kw_lexeme)
*	Purpose: To check if the input word is in the keyword table
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Nov 14, 2020
*	Called functions:
*		strcmp(const char *_Str1 ,const char *_Str2)
*	Parameters:
*		char kw_lexeme
*	Return value:
*		Not found: -1
*		Found: the position of the keyword
*	Algorithm:
*		Check if the input string is a keyword by seeing if it's in the keyword table.
*		Return the position in the table if found. Return -1 if not found.
*/
int isKeyword(char* kw_lexeme) 
{
	int i;
	for (i = 0; i < KWT_SIZE; ++i)
		if (strcmp(kw_lexeme, keywordTable[i]) == 0) /* Check if the kw_lexeme is in the table */
			return i;

	return -1; /* Keyword not found */
}