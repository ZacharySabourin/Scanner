/*
* File Name: buffer.c
* Compiler: MS Visual Studio 2019
* Author: Zachary Sabourin
* Course: CST 8152 - Compilers, Lab Section: 012
* Assignment: A2
* Date: November 14, 2020
* Professor: Paulo Sousa
* Purpose: To create a buffer that holds the contents of a file.
*		To be able to load the contents of a file into the buffer, print the buffer contents, add characters to the buffer,
*		reset various positions, check and set flags, and add chars to the buffer.
* Function List:
*		pBuffer bufferCreate(short initCapacity, char incFactor, char opMode);
*		pBuffer bufferAddChar(pBuffer const pBE, char symbol);
*		int bufferClear(Buffer* const pBE);
*		void bufferFree(Buffer* const pBE);
*		int bufferIsFull(Buffer* const pBE);
*		short bufferGetAddCPosition(Buffer* const pBE);
*		short bufferGetCapacity(Buffer* const pBE);
*		int bufferGetOpMode(Buffer* const pBE);
*		size_t bufferGetIncrement(Buffer* const pBE);
*		int bufferLoad(FILE* const fi, Buffer* const pBE);
*		int bufferIsEmpty(Buffer* const pBE);
*		char bufferGetChar(Buffer* const pBE);
*		int bufferGetEobFlag(Buffer* const pBE);
*		int bufferPrint(Buffer* const pBE, char newLine);
*		Buffer* bufferSetEnd(Buffer* const pBE, char symbol);
*		char bufferGetRFlag(Buffer* const pBE);
*		short bufferRetract(Buffer* const pBE);
*		short bufferReset(Buffer* const pBE);
*		short bufferGetCPosition(Buffer* const pBE);
*		int bufferRewind(Buffer* const pBE);
*		char* bufferGetString(Buffer* const pBE, short charPosition);
*		pBuffer bufferSetMarkPosition(pBuffer const pBE, short mark);
*		unsigned short bufferGetFlags(pBuffer const pBE);
*/

#define _CRT_SECURE_NO_WARNINGS

#include "buffer.h"

/*
*	Function name: bufferCreate(short initCapacity, char incFactor, char opMode)
*	Purpose: To create the buffer entity.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		calloc(size_t _Count, size_t _Size)
*		malloc(size_t _Size)
*		strcopy(char *_Destination, const char *_Source)
*		free(void *_Block)
*	Parameters:
*		short initCapacity:	>= 0 | <= MAX_BUFFER_CAPACITY
*		char incFactor:
*		char opMode: f || m || a
*	Return value:
*		NULL: on error
*		pBuffer: pointer to buffer entity
*	Algorithm:
*		Allocate memory for the buffer struct, determine and instantiate all the initial buffer values.
*/
pBuffer bufferCreate(short initCapacity, char incFactor, char opMode)
{
	/* Checking validity of parameters */
	if ((initCapacity < 0) || (initCapacity > MAX_BUFFER_CAPACITY))
		return NULL;
	if (opMode != 'f' && opMode != 'm' && opMode != 'a')
		return NULL;

	/* Allocate memory for the buffer */
	pBuffer const myBuffer = calloc(1, sizeof(struct BufferEntity));
	if (!myBuffer)
	{
		free(myBuffer);
		return NULL;
	}

	/* Temporary variables */
	short tempCapacity = 0;
	char tempIncFactor = 0;
	char tempOpMode = 0;

	if (initCapacity == 0)
	{
		/* Set the values to their default */
		tempCapacity = DEFAULT_INIT_CAPACITY;

		if (opMode == 'f')
		{
			tempIncFactor = FIXED_INC_FACTOR;
			tempOpMode = 0;
		}
		else if (opMode == 'm')
		{
			tempIncFactor = DEFAULT_INC_FACTOR;
			tempOpMode = -1;
		}
		else
		{
			tempIncFactor = DEFAULT_INC_FACTOR;
			tempOpMode = 1;
		}
	}

	else
	{
		tempCapacity = initCapacity;

		if (incFactor == 0 || opMode == 'f')
		{
			tempIncFactor = FIXED_INC_FACTOR;
			tempOpMode = 0;
		}

		/* Use the function parameters */
		else
		{
			tempIncFactor = incFactor;

			if (opMode == 'm' && ((incFactor >= 1) || (incFactor <= 100)))
				tempOpMode = -1;
			else if (opMode == 'a' && ((incFactor >= 1) || (incFactor <= 255)))
				tempOpMode = 1;
			else
				return NULL;
		}
	}

	myBuffer->string = malloc(tempCapacity);
	if (!myBuffer->string)
	{
		free(myBuffer);
		return NULL;
	}

	/* Setting all initial values */
	myBuffer->string[0] = '\0';
	myBuffer->capacity = tempCapacity;
	myBuffer->addCPosition = 0;
	myBuffer->getCPosition = 0;
	myBuffer->markCPosition = 0;
	myBuffer->increment = tempIncFactor;
	myBuffer->opMode = tempOpMode;
	myBuffer->flags = DEFAULT_FLAGS;

	return myBuffer;
}

/*
*	Function name: bufferAddChar(pBuffer const pBE, char symbol)
*	Purpose: To add a character to the buffer.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferIsFull(Buffer* const pBE)
*		bufferGetCapacity(Buffer* const pBE)
*		bufferGetIncrement(Buffer* const pBE)
*		realloc(void *_Block, size_t _Size)
*		free(void *_Block)
*	Parameters:
*		pBuffer const pBE: != NULL
*		char symbol
*	Return value:
*		NULL: error
*		pBuffer: pointer to buffer entity
*	Algorithm:
*		Adds a character to the buffer string. Resizes the buffer if needed.
*/
pBuffer bufferAddChar(pBuffer const pBE, char symbol)
{
	if (!pBE)
		return NULL;

	/* Reset the R_FLAG value */
	pBE->flags &= RESET_R_FLAG;

	if (bufferIsFull(pBE))
	{
		/* Don't do anything if in fixed mode or if buffer at max capacity */
		if (pBE->opMode == 0 || bufferGetAddCPosition(pBE) == MAX_BUFFER_CAPACITY)
			return NULL;

		short newCapacity;

		/* Additive mode */
		if (pBE->opMode == 1)
			newCapacity = pBE->capacity + pBE->increment;

		/* Multiplicative mode */
		else
		{
			short availableSpace = MAX_BUFFER_CAPACITY - pBE->capacity;
			int newIncrement = availableSpace * pBE->increment / 100;
			newCapacity = pBE->capacity + (short)newIncrement;
		}

		/* Error on negative value */
		if (newCapacity < 1)
			return NULL;

		/* Give capacity the maximum if maximum is exceeded */
		if (newCapacity >= (MAX_BUFFER_CAPACITY - 1))
			newCapacity = MAX_BUFFER_CAPACITY;

		/* Reallocate the string with a new capacity */
		char* tempString = realloc(pBE->string, newCapacity);
		if (!tempString)
		{
			free(tempString);
			return NULL;
		}

		/* Point the string to the new memory location and set the R_FLAG and new capacity */
		pBE->string = tempString;
		pBE->flags |= SET_R_FLAG;
		pBE->capacity = newCapacity;
	}

	/* Add char to buffer and increment position if not at max capacity */
	pBE->string[pBE->addCPosition] = symbol;
	if (bufferGetAddCPosition(pBE) < bufferGetCapacity(pBE))
		++pBE->addCPosition;

	return pBE;
}

/*
*	Function name: bufferClear(Buffer* const pBE)
*	Purpose: To clear some of the buffer values
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		1 on success
*	   -1 on error
*	Algorithm:
*		Reset all CPosition markers to initial values of 0.
*/
int bufferClear(Buffer* const pBE)
{
	if (!pBE)
		return RT_FAIL_1;

	/* Reset all values to 0 */
	pBE->addCPosition = 0;
	pBE->getCPosition = 0;
	pBE->markCPosition = 0;
	pBE->flags &= DEFAULT_FLAGS;

	return 1;
}

/*
*	Function name: bufferFree(Buffer* const pBE)
*	Purpose: To free the buffer
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		free(void *_Block)
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value: N/A
*	Algorithm:
*/
void bufferFree(Buffer* const pBE)
{
	if (pBE)
		free(pBE);
}

/*
*	Function name: bufferIsFull(Buffer* const pBE)
*	Purpose: To determine if the buffer is full
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE
*	Return value:
*		1 if full
*		0 if not full
*	Algorithm:
*/
int bufferIsFull(Buffer* const pBE)
{
	return (pBE->addCPosition == pBE->capacity) ? 1 : 0;
}

/*
*	Function name: bufferGetAddCPosition(Buffer* const pBE)
*	Purpose: Retrieve the addCPosition of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		short addCposition
*	Algorithm:
*/
short bufferGetAddCPosition(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->addCPosition;
}

/*
*	Function name: bufferGetCapacity(Buffer* const pBE)
*	Purpose: Retrieve the current capacity of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		short capacity
*	Algorithm:
*/
short bufferGetCapacity(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->capacity;
}

/*
*	Function name: bufferGetOpMode(Buffer* const pBE)
*	Purpose: Retrieve the current operational mode of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		int opMode
*	Algorithm:
*/
int bufferGetOpMode(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->opMode;
}

/*
*	Function name: bufferGetIncrement(Buffer* const pBE)
*	Purpose: Retrieve the current increment factor of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		size_t increment
*	Algorithm:
*/
size_t bufferGetIncrement(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->increment;
}

/*
*	Function name: bufferLoad(FILE* const fi, Buffer* const pBE)
*	Purpose: To load a file's contents into the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		feof(FILE *_Stream)
*		fgetc(FILE *_Stream)
*		bufferAddChar(pBuffer const pBE, char symbol)
*		ungetc(int _Character, FILE *_Stream)
*		bufferGetAddCPosition(Buffer* const pBE)
*	Parameters:
*		FILE* const fi: != NULL
*		Buffer* const pBE: != NULL
*	Return value:
*		LOAD_FAIL: NULL file, NULL buffer, or error adding character
*		int addCPosition
*	Algorithm:
*		Load an opened file into the buffer until it can't anymore
*/
int bufferLoad(FILE* const fi, Buffer* const pBE)
{
	if (!pBE || !fi)
		return LOAD_FAIL;

	/* Loop until EOF or issue with adding character */
	while (!feof(fi))
	{
		char c = (char)fgetc(fi);

		/* Exit the loop */
		if (!feof(fi))
		{
			/* Give the char back to the stream and return error */
			if (!bufferAddChar(pBE, c))
			{
				ungetc(c, fi);
				return LOAD_FAIL;
			}
		}	
	}

	/* Return the number of characters added to the buffer */
	return bufferGetAddCPosition(pBE);
}

/*
*	Function name: bufferIsEmpty(Buffer* const pBE)
*	Purpose: Determine whether or not the buffer is empty
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferGetAddCPosition(Buffer* const pBE)
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		0 if not empty
*		1 if empty
*	Algorithm:
*/
int bufferIsEmpty(Buffer* const pBE)
{
	if (!pBE)
		return RT_FAIL_1;
	return bufferGetAddCPosition(pBE) ? 0 : 1;
}

/*
*	Function name: bufferGetChar(Buffer* const pBE)
*	Purpose: Retrieve the current character from the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_2 on NULL pointer
*		0 if at EOB
*		char at current position
*	Algorithm:
*		Retrieve one character from the buffer unless at end of buffer
*/
char bufferGetChar(Buffer* const pBE)
{
	if (!pBE)
		return RT_FAIL_2;

	/* If EOB, set EOB_FLAG and return zero char */
	if (pBE->getCPosition == pBE->addCPosition)
	{
		pBE->flags |= SET_EOB;
		return 0;
	}

	else
		pBE->flags &= RESET_EOB;

	/* Retrieve the current char and increment the position */
	return pBE->string[pBE->getCPosition++];
}

/*
*	Function name: bufferGetEobFlag(Buffer* const pBE)
*	Purpose: Retrieve the value of the EOB_FLAG
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on NULL pointer
*		int value of EOB flag
*	Algorithm:
*/
int bufferGetEobFlag(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : (pBE->flags & CHECK_EOB);
}

/*
*	Function name: bufferPrint(Buffer* const pBE, char newLine)
*	Purpose: Print each character loaded into the buffer
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferGetEobFlag(Buffer* const pBE)
*		bufferGetChar(Buffer* const pBE)
*		printf(const char *const _Format, ...)
*		bufferGetAddCPosition(Buffer* const pBE)
*	Parameters:
*		Buffer* const pBE: != NULL
*		char newLine
*	Return value:
*		LOAD_FAIL on NULL pointer
*		RT_FAIL_1 on error getting char
*		int getCPosition: number of characters printed
*	Algorithm:
*		Prints each character in the buffer string until end of buffer or error.
*/
int bufferPrint(Buffer* const pBE, char newLine)
{
	if (!pBE)
		return LOAD_FAIL;

	/* Loop while the EOB isn't set to 1 */
	while (bufferGetEobFlag(pBE) != 1)
	{
		char c = bufferGetChar(pBE);

		if (c == RT_FAIL_2) /* Exit if error occured */
			return RT_FAIL_1;

		/* Ensures that EOB character is not printed */
		if (bufferGetEobFlag(pBE) != 1)
			printf("%c", c);
	}

	if (newLine != 0)
		printf("\n");

	/* Return number of chars in the buffer */
	return bufferGetCPosition(pBE);
}

/*
*	Function name: bufferSetEnd(Buffer* const pBE, char symbol)
*	Purpose: Resize the buffer and add a character at the end.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferGetCPosition(Buffer* const pBE)
*		realloc(void *_Block, size_t _Size)
*		free(void *_Block)
*		bufferAddChar(pBuffer const pBE, char symbol)
*	Parameters:
*		Buffer* const pBE: != NULL
*		char symbol
*	Return value:
*		NULL: on null pointer or error adding symbol
*		pBuffer: pointer to buffer entity
*	Algorithm:
*		Reallocate the buffer to a new capacity if possible.
*		Set the new capacity and add the symbol to the end of the buffer.
*/
Buffer* bufferSetEnd(Buffer* const pBE, char symbol)
{
	/* Can't change the end point if buffer is at max capacity */
	if (pBE->getCPosition == MAX_BUFFER_CAPACITY)
		return NULL;

	short newCapacity = (pBE->addCPosition + 1);
	if (newCapacity > MAX_BUFFER_CAPACITY)
		return NULL;

	/* Reallocate the buffer string */
	char* tempString = realloc(pBE->string, newCapacity);
	if (!tempString)
	{
		free(tempString);
		return NULL;
	}

	/*
		Point the string to the new memory location, set the R_FLAG and new capacity
		and add the symbol
	*/
	pBE->string = tempString;
	pBE->flags |= SET_R_FLAG;
	pBE->capacity = (short)newCapacity;
	if (!bufferAddChar(pBE, symbol))
		return NULL;

	return pBE;
}

/*
*	Function name: bufferGetRFlag(Buffer* const pBE)
*	Purpose: Determine whether or not the R_FLAG has been set
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		char R_FLAG sset or not
*	Algorithm:
*/
char bufferGetRFlag(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : (pBE->flags & CHECK_R_FLAG);
}

/*
*	Function name: bufferRetract(Buffer* const pBE)
*	Purpose: Decrement the getCPosition value of the buffer entity.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		short getCPosition
*	Algorithm:
*/
short bufferRetract(Buffer* const pBE)
{
	if (!pBE || pBE->getCPosition == 0)
		return RT_FAIL_1;

	return --pBE->getCPosition;
}

/*
*	Function name: bufferReset(Buffer* const pBE)
*	Purpose: Resets the getcPosition of the buffer entity.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferGetCPosition(Buffer* const pBE)
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		short getCPosition
*	Algorithm:
*		Resets the getcPosition of the buffer entity to the value of the markCPosition.
*/
short bufferReset(Buffer* const pBE)
{
	if (!pBE)
		return RT_FAIL_1;

	pBE->getCPosition = pBE->markCPosition;
	return bufferGetCPosition(pBE);
}

/*
*	Function name: bufferGetCPosition(Buffer* const pBE)
*	Purpose: Retrieve the getCPosition of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		short getCPosition
*	Algorithm:
*/
short bufferGetCPosition(Buffer* const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->getCPosition;
}

/*
*	Function name: bufferRewind(Buffer* const pBE)
*	Purpose: Reset the positions of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		0 on success
*	Algorithm:
*		Reset the getCPosition, markCPosition, and reset the EOB_FLAG of the buffer entity
*/
int bufferRewind(Buffer* const pBE)
{
	if (!pBE)
		return RT_FAIL_1;

	/* Reset the positions and reset the EOB_FLAG */
	pBE->getCPosition = 0;
	pBE->markCPosition = 0;
	pBE->flags &= RESET_EOB;
	return 0;
}

/*
*	Function name: bufferGetString(Buffer* const pBE, short charPosition)
*	Purpose: Retrieve the pointer to a character at a given position.
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
		short charPosition
*	Return value:
*		NULL on null pointer
*		char* at the given position
*	Algorithm:
*/
char* bufferGetString(Buffer* const pBE, short charPosition)
{
	return (!pBE) ? NULL : &pBE->string[charPosition];
}

/*
*	Function name: bufferSetMarkPosition(pBuffer const pBE, short mark)
*	Purpose: Set the buffer enitity's markCPosition to a user determined value
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions:
*		bufferGetAddCPosition(Buffer* const pBE)
*	Parameters:
*		Buffer* const pBE: != NULL
*		short mark: >= 0 || <= addCPosition
*	Return value:
*		NULL on null pointer or invalid mark values
*		pBuffer: pointer to buffer entity
*	Algorithm:
*/
pBuffer bufferSetMarkPosition(pBuffer const pBE, short mark)
{
	/* Can't set if null pointer or invalid mark values */
	if (!pBE || mark < 0 || mark > bufferGetAddCPosition(pBE))
		return NULL;

	pBE->markCPosition = mark;
	return pBE;
}

/*
*	Function name: bufferGetFlags(pBuffer const pBE)
*	Purpose: Retrieve the current flags value of the buffer entity
*	Author: Zachary Sabourin
*	History/Version: Ver 1.0, Oct 1, 2020
*	Called functions: N/A
*	Parameters:
*		Buffer* const pBE: != NULL
*	Return value:
*		RT_FAIL_1 on null pointer
*		unsigned short flags: on success
*	Algorithm:
*/
unsigned short bufferGetFlags(pBuffer const pBE)
{
	return (!pBE) ? RT_FAIL_1 : pBE->flags;
}