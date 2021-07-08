/**
* File Name: buffer.h
* Compiler: MS Visual Studio 2019
* Author: Zachary Sabourin
* Course: CST 8152 - Compilers, Lab Section: 012
* Assignment: A2
* Date: November 14, 2020
* Professor: Paulo Sousa
* Purpose:
		Holds all of the function declarations, constants, bit-masks, and BufferEntity struct for buffer.c
* Function List:
*		pBuffer bufferCreate(short initCapacity, char incFactor, char opMode);
		pBuffer bufferAddChar(pBuffer const pBE, char symbol);
		int bufferClear(Buffer* const pBE);
		void bufferFree(Buffer* const pBE);
		int bufferIsFull(Buffer* const pBE);
		short bufferGetAddCPosition(Buffer* const pBE);
		short bufferGetCapacity(Buffer* const pBE);
		int bufferGetOpMode(Buffer* const pBE);
		size_t bufferGetIncrement(Buffer* const pBE);
		int bufferLoad(FILE* const fi, Buffer* const pBE);
		int bufferIsEmpty(Buffer* const pBE);
		char bufferGetChar(Buffer* const pBE);
		int bufferGetEobFlag(Buffer* const pBE);
		int bufferPrint(Buffer* const pBE, char newLine);
		Buffer* bufferSetEnd(Buffer* const pBE, char symbol);
		char bufferGetRFlag(Buffer* const pBE);
		short bufferRetract(Buffer* const pBE);
		short bufferReset(Buffer* const pBE);
		short bufferGetCPosition(Buffer* const pBE);
		int bufferRewind(Buffer* const pBE);
		char* bufferGetString(Buffer* const pBE, short charPosition);
		pBuffer bufferSetMarkPosition(pBuffer const pBE, short mark);
		unsigned short bufferGetFlags(pBuffer const pBE);
*/

#ifndef BUFFER_H_
#define BUFFER_H_

#pragma warning(1:4001)		/*to enforce C89 type comments  - to make //comments an warning */
#pragma warning(error:4001)	/* to enforce C89 comments - to make // comments an error */

#include <stdio.h>  /* standard input/output */
#include <malloc.h> /* for dynamic memory allocation*/
#include <limits.h> /* implementation-defined data type ranges and limits */
#include <string.h>	/* string tools */

/* Constant definitions */
#define RT_FAIL_1 (-1)         /* operation failure return value 1 */
#define RT_FAIL_2 (-2)         /* operation failure return value 2 */
#define LOAD_FAIL (-2)         /* load fail return value */

#define DEFAULT_INIT_CAPACITY 200   /* default initial buffer capacity */
#define DEFAULT_INC_FACTOR 15       /* default increment factor */
#define FIXED_INC_FACTOR 0			/* default increment factor in fixed mode */
#define MAX_BUFFER_CAPACITY (SHRT_MAX-1)	/* maximum buffer capacity */

/* Bit-masks constants */
#define DEFAULT_FLAGS	0xFFFC	/* 1111.1111 1111.1100 */
#define SET_EOB			0x0001	/* 0000.0000.0000.0001 */
#define RESET_EOB		0xFFFE	/* 1111.1111.1111.1110 */
#define CHECK_EOB		0x0001	/* 0000.0000.0000.0001 */
#define SET_R_FLAG		0x0002	/* 0000.0000.0000.0010 */
#define RESET_R_FLAG	0xFFFD	/* 1111.1111.1111.1101 */
#define CHECK_R_FLAG	0x0002	/* 0000.0000.0000.0010 */

/* User data type declarations */
typedef struct BufferEntity
{
	char* string;			/* pointer to the beginning of character array (character buffer) */
	short capacity;			/* current dynamic memory size (in bytes) allocated to character buffer */
	short addCPosition;		/* the offset (in chars) to the add-character location */
	short getCPosition;		/* the offset (in chars) to the get-character location */
	short markCPosition;	/* the offset (in chars) to the mark location */
	char  increment;		/* character array (in chars) increment factor */
	char  opMode;			/* operational mode indicator*/
	unsigned short flags;   /* contains character array reallocation flag and end-of-buffer flag */
} Buffer, * pBuffer;

pBuffer bufferCreate(short initCapacity, char incFactor, char opMode);

pBuffer bufferAddChar(pBuffer const pBE, char symbol);

int bufferClear(Buffer* const pBE);

void bufferFree(Buffer* const pBE);

int bufferIsFull(Buffer* const pBE);

short bufferGetAddCPosition(Buffer* const pBE);

short bufferGetCapacity(Buffer* const pBE);

int bufferGetOpMode(Buffer* const pBE);

size_t bufferGetIncrement(Buffer* const pBE);

int bufferLoad(FILE* const fi, Buffer* const pBE);

int bufferIsEmpty(Buffer* const pBE);

char bufferGetChar(Buffer* const pBE);

int bufferGetEobFlag(Buffer* const pBE);

int bufferPrint(Buffer* const pBE, char newLine);

Buffer* bufferSetEnd(Buffer* const pBE, char symbol);

char bufferGetRFlag(Buffer* const pBE);

short bufferRetract(Buffer* const pBE);

short bufferReset(Buffer* const pBE);

short bufferGetCPosition(Buffer* const pBE);

int bufferRewind(Buffer* const pBE);

char* bufferGetString(Buffer* const pBE, short charPosition);

pBuffer bufferSetMarkPosition(pBuffer const pBE, short mark);

unsigned short bufferGetFlags(pBuffer const pBE);

#endif