/*
 * mp3.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "mp3.h"
#include "id3_tag_frame.h"
#include "util.h"
#include "xmalloc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>


//CODIGO ORIGINAL
//extern const char PATH_SEPARATOR;



/**
 * Returns a populated ID3_Mp3 type.  If tag_only is true then the data buffer is not populated and will be null.
 */
Mp3* get_mp3( const char* const file, int fd, bool tag_only )
{

	// Validate args.
	//
		if( file == NULL || fd <= 2 /* shouldn't be stdin, stdout or stderr */ )
			return NULL;


	// Parse file and directory components.
	//
		Mp3* mp3 	= (Mp3*)xcalloc( 1, sizeof( Mp3 ) );
		mp3->v1_tag	= v1_get_tag( fd );
		mp3->v2_tag = v2_get_tag( fd );

		// Make a copy of file so it doesn't get modified.
		size_t length = strlen( file ) + 1;
		char temp_file[length];
		strcpy( temp_file, file );

		char* bname = basename( temp_file );
		mp3->old_file_name = (char*)xcalloc( strlen( bname ) + 2, sizeof( char ) );
		strcpy( mp3->old_file_name, basename( temp_file ) );

		mp3->file_path = (char*)xcalloc( length, sizeof( char ) );
		strcpy( mp3->file_path, dirname( temp_file ) );


		// basename() will remove trailing path separators, so we have to add one.
		length = strlen( mp3->file_path );

		/* CODIGO ORIGINAL
		if( mp3->file_path[length-1] != PATH_SEPARATOR )
			mp3->file_path[length] = PATH_SEPARATOR;*/
		//MI CODIGO
		if( mp3->file_path[length-1] != '/' )
			mp3->file_path[length] = '/';


	if( tag_only )
		return mp3;


	// Create a buffer to hold frame data.
	//
	// The memory allocation is assigned to the mp3->data member.  After which, mp3->data is searched to determine
	// the location of the beginning of the actual mp3 frames, and that location is then assigned to mp3->data_offset.
	// There is a good chance the beginning of the frame data does not start immediately after the tag.  In this case
	// there will be garbage after the tag, up until the beginning of the frame data.  This method precludes the need
	// to pack the frame data at the beginning of the allocated buffer.
	//
		ssize_t size = get_file_size( fd );

		if( mp3->v1_tag != NULL )
			size -= mp3->v1_tag->size;

		off_t current_pos = lseek( fd, 0, SEEK_CUR );	// Get the current read position.
		size -= current_pos;

		mp3->data = (uint8_t*)xcalloc( size, sizeof( uint8_t ) );
		ssize_t bytes_read = read( fd, mp3->data, size );

		if( bytes_read != size )
		{
			print_message( "Error reading MP3 frame data", ERROR, true );
			free_mp3( mp3 );
			return NULL;
		}


	// Find the MP3 frame data.  Look for the MP3 frame sync (11 bits) after the ID3 tag.
	//
		const uint16_t FRAME_SYNC = 0xE0;

		for( int i = 0; i < size; i++ )
		{
			if( mp3->data[i] == 0xFF && ((mp3->data[i+1] & FRAME_SYNC) == FRAME_SYNC) )
			{
				mp3->data_offset = mp3->data + i;
				mp3->data_size = size - i;
				break;
			}
		}

		if( mp3->data == NULL )	// No frames found.  Something went wrong.
		{
			print_message( "No valid MP3 frames found.", ERROR, false );
			free_mp3( mp3 );
			return NULL;
		}


	return mp3;
}



/**
 * Frees memory associated with an MP3_file.
 */
void free_mp3( Mp3* mp3 )
{

	if( mp3 != NULL )
	{
		v1_free_tag( mp3->v1_tag );
		v2_free_tag( mp3->v2_tag );
		free( mp3->data );
		free( mp3->file_path );
		free( mp3->new_file_name );
		free( mp3->old_file_name );
		free( mp3 );
		mp3 = NULL;
	}
}


/**
 * Attempts to find the song's artist.  If found, the artist is returned, otherwise NULL is returned.
 * Caller should free the returned string.
 */
char* get_artist( const Mp3* const mp3 )
{

	// Validate args.
	//
		if( mp3 == NULL )
			return false;


	// Find artist.
	//
		if( mp3->v2_tag != NULL )
		{
			const char* artist_tags[4];
			artist_tags[0] = "TPE1";
			artist_tags[1] = "TPE2";
			artist_tags[2] = "TPE3";
			artist_tags[3] = "TCOM";

			const TagFrame* artist_frame = NULL;
			for( int i = 0; i < 4; i++ )
			{
				artist_frame = find_tag_frame( mp3->v2_tag->frame_list, artist_tags[i] );

				if( artist_frame != NULL )
					break;
			}

			if( artist_frame != NULL )
				return strdup( trim( (char*)artist_frame->data[0]->datum ) );
		}


		if( mp3->v1_tag != NULL )
			if( strlen( mp3->v1_tag->artist ) > 0 )
				return strdup( mp3->v1_tag->artist );


		return NULL;
}



/**
 * Attempts to find the song's title.  If found, the title is returned, otherwise NULL is returned.
 * Caller should free the returned string.
 */
static char* get_title( const Mp3* const mp3 )
{

	// Validate args.
	//
		if( mp3 == NULL )
			return false;


	// Find title.
	//
		if( mp3->v2_tag != NULL )
		{
			const char* title_tags[3];
			title_tags[0] = "TIT2";
			title_tags[1] = "TIT1";
			title_tags[2] = "TIT3";


			const TagFrame* title_frame = NULL;
			for( int i = 0; i < 3; i++ )
			{
				title_frame = find_tag_frame( mp3->v2_tag->frame_list, title_tags[i] );

				if( title_frame != NULL )
					break;
			}

			if( title_frame != NULL )
				return strdup( trim( (char*)title_frame->data[0]->datum ) );
		}

		if( mp3->v1_tag != NULL )
			if( strlen( mp3->v1_tag->title ) > 0 )
				return strdup( mp3->v1_tag->title );


		return NULL;
}



/**
 * Returns true if the MP3 was successfully renamed using the ID3 version 2 tag, false otherwise.
 */
bool rename_from_tag( Mp3* const mp3 )
{

	// Validate args.
	//
		if( mp3 == NULL )
			return false;


	// Get artist.
	//
		char* artist = get_artist( mp3 );

		if( artist == NULL )
		{
			print_message( "No suitable artist found for rename.", WARNING, false );
			return false;
		}


	// Get title.
	//
		char* title = get_title( mp3 );

		if( title == NULL )
		{
			print_message( "No suitable song title found for rename.", WARNING, false );
			free( artist );
			return false;
		}


	// Set the new file name.
	//
		const char* seperator = " - ";
		const char* file_ext  = ".mp3";

		int length = strlen( artist )	 +
					 strlen( title )  	 +
					 strlen( seperator ) +
					 strlen( file_ext )  + 4;

		// Get some memory to contain the new path.
		mp3->new_file_name = (char*)xcalloc( length, sizeof( char ) );
		strcat( mp3->new_file_name, replace_filename_chars( artist ) );
		strcat( mp3->new_file_name, seperator );
		strcat( mp3->new_file_name, replace_filename_chars( title ) );
		strcat( mp3->new_file_name, file_ext );


	// Clean up.
	//
		free( artist );
		free( title );


	return true;
}
