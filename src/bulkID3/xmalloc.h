/*
 * xmalloc.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef XMALLOC_H_
#define XMALLOC_H_



#include <stddef.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/


/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

void* xcalloc( size_t count, size_t eltsize );
void* xrealloc( void *ptr, size_t newsize );



#endif /* XMALLOC_H_ */
