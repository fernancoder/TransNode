/*
 * linked_list.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "linked_list.h"
#include <stdbool.h>
#include <stdlib.h>



/**
 * Adds an item to the list.  Returns true if the item was successfully added, false otherwise.
 * Null data is not permitted.
 */
bool list_add( List* const list, void* const data )
{

	// Validate args.
	//
		if( list == NULL || data == NULL )
			return false;


	// Create a new node and add it to the list.
	//
		Node* node = (Node*)calloc( 1, sizeof( Node ) );

		if( node == NULL )
			return false;

		node->data = data;

		if( list->head == NULL )
			list->head = node;
		else
		{
			Node* temp = list->head;
			while( temp->next != NULL )
				temp = temp->next;

			temp->next = node;
		}


	return true;
}



/**
 * Removes an item from the list.  Returns true if the item was removed, false otherwise.
 */
bool list_remove( List* const list, const Node* node )
{

	if( list == NULL || list->head == NULL || node == NULL )
		return false;


	// Special case for head.
	//
		if( node == list->head )
		{
			Node* temp = list->head;

			if( list->head->next != NULL )
				list->head = list->head->next;
			else
				list->head = NULL;

			free( temp );
			return true;
		}


	// Remove the node.
	//
		Node* current = list->head;

		while( current->next != NULL && current->next != node )
			current = current->next;

		if( current->next == node )
		{
			Node* temp = current->next;
			current->next = current->next->next;
			free( temp );
			return true;
		}


	return false;
}



/**
 * Finds an item in the list.  If the item is found, it is returned, otherwise NULL is returned.  The compare
 * function pointer is used to compare data items for equality.
 */
const void* list_find( const List* const list, const void* const data, int (*compare)( const void*, const void* ) )
{

	// Validate args.
	//
		if( list == NULL || data == NULL )
			return NULL;


	// Search the list for item.
	//
		Node* temp = list->head;
		while( temp != NULL )
		{
			if( (*compare)( data, temp->data ) == 0 )
				return temp->data;

			temp = temp->next;
		}


	return NULL;
}



/**
 * Returns the number of items in the list.
 */
unsigned int list_size( const List* const list )
{

	unsigned int size = 0;

	if( list != NULL )
	{
		Node* temp = list->head;
		while( temp != NULL )
		{
			size++;
			temp = temp->next;
		}
	}


	return size;
}
