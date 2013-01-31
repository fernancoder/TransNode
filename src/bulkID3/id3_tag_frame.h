/*
 * id3_tag_frame.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef TAG_FRAME_H_
#define TAG_FRAME_H_



#include "util.h"
#include "linked_list.h"
#include <stdint.h>
#include <stddef.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

/**
 * Represents a single data item.
 */
typedef struct
{
	uint8_t* 	  datum;
	size_t 		  size;
	size_t 		  storage_size;
	enum Encoding encoding;

} TagFrameDatum;


/**
 * Represents a tag frame.
 */
typedef struct
{
	char	 	  	id[5];
	uint8_t 	  	status_flags;
	uint8_t 	  	format_flags;
	TagFrameDatum**	data;			// Frames may contain multiple data items.
	int 			data_count; 	// Number of TagFrameDatum elements.

} TagFrame;



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

List* 			get_tag_frames( const uint8_t* buffer, const size_t buffer_size, const int tag_version );
void  			free_tag_frame( TagFrame* frame );
const TagFrame* find_tag_frame( const List* const list, const char* const frame_id );
const char** 	validate_frames( const char** const frames, int* const frame_count );
const char** 	get_frame_list( unsigned int* const frame_count );



#endif /* TAG_FRAME_H_ */
