/*
 * linked_list.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_



#include <stdbool.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

struct Node
{
	struct Node* next;
	const void* data;
};

typedef struct Node Node;


typedef struct
{
	Node* head;

} List;



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

bool  		 list_add( List* const list, void* const data );
bool  		 list_remove( List* const list, const Node* node );
const void*  list_find( const List* const list, const void* const data, int (*compare)( const void*, const void* ) );
unsigned int list_size( const List* const list );



#endif /* LINKED_LIST_H_ */
