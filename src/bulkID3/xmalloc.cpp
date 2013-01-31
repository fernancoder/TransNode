/*
 * xmalloc.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



/**
 * The functions below are memory allocation wrappers.  If memory allocation fails, an error is printed and the program exits.
 */
void* xcalloc( size_t count, size_t eltsize )
{

	void* value = calloc( count, eltsize );

	if( value == NULL )
	{
		fprintf( stderr, "calloc() failed.  %s", strerror( errno ) );
		exit( EXIT_FAILURE );
	}

	return value;
}



void* xrealloc( void *ptr, size_t newsize )
{

	void* value = realloc( ptr, newsize );

	if( value == NULL )
	{
		fprintf( stderr, "realloc() failed.  %s", strerror( errno ) );
		exit( EXIT_FAILURE );
	}

	return value;
}
