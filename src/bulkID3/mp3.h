/*
 * mp3.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef MP3_H_
#define MP3_H_



#include "id3_tag.h"
#include <stddef.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

/**
 * Represents an MP3 file.
 */
typedef struct
{
	char*	 old_file_name;
	char*	 new_file_name;
	char*	 file_path;
	V1Tag* 	 v1_tag;
	V2Tag* 	 v2_tag;
	size_t	 data_size;		// Size of MP3 frame buffer.
	uint8_t* data;			// Pointer to actual malloc'ed block.  To be passed to free.
	uint8_t* data_offset;	// Pointer to the actual mp3 frame data.  This is some value > data.

} Mp3;



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

Mp3*  get_mp3( const char* const file, int fd, bool tag_only );
void  free_mp3( Mp3* mp3 );
char* get_artist( const Mp3* const mp3 );
bool  rename_from_tag( Mp3* const mp3 );



#endif /* MP3_H_ */
