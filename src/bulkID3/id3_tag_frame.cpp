/*
 * id3_tag_frame.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "id3_tag_frame.h"
#include "id3_util.h"
#include "linked_list.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>



// Version 2.3 and 2.4 tags, and some non-standard tags too.
static const char* const TAG_FRAME_LIST[] = {
"AENC",
"APIC",
"ASPI",
"ATXT",
"COMM",
"COMR",
"CTOC",
"CHAP",
"ENCR",
"EQUA",
"EQU2",
"ETCO",
"GEOB",
"GRID",
"IPLS",
"LINK",
"MCDI",
"MLLT",
"OWNE",
"PRIV",
"PCNT",
"POPM",
"POSS",
"RBUF",
"RVAD",
"RVA2",
"RVRB",
"SEEK",
"SIGN",
"SYLT",
"SYTC",
"TALB",
"TBPM",
"TCOM",
"TCON",
"TCOP",
"TDAT",
"TDEN",
"TDLY",
"TDOR",
"TDRC",
"TDRL",
"TDTG",
"TENC",
"TEXT",
"TFLT",
"TIME",
"TIPL",
"TIT1",
"TIT2",
"TIT3",
"TKEY",
"TLAN",
"TLEN",
"TMCL",
"TMED",
"TMOO",
"TOAL",
"TOFN",
"TOLY",
"TOPE",
"TORY",
"TOWN",
"TPE1",
"TPE2",
"TPE3",
"TPE4",
"TPOS",
"TPRO",
"TPUB",
"TRCK",
"TRDA",
"TRSN",
"TRSO",
"TSIZ",
"TSOA",
"TSOP",
"TSOT",
"TSRC",
"TSSE",
"TSST",
"TYER",
"TXXX",
"UFID",
"USER",
"USLT",
"WCOM",
"WCOP",
"WOAF",
"WOAR",
"WOAS",
"WORS",
"WPAY",
"WPUB",
"WXXX",
	// Unofficial frames.
"COMM",
"RGAD",
"TCMP",
"TSO2",
"TSOC",
"XRVA"
};



/**
 * The following functions parse specific frames types.  Each function is given a buffer containing a frame, and is expected to
 * create and return an array of TagFrameDatum types.  Upon return 'buffer_size' contains the number of TagFrameDatam
 * elements in the returned array.
 */
static TagFrameDatum** parse_frame_TEXT( const uint8_t* buffer, size_t* buffer_size )
{

	// Determine Encoding.
	//
		bool has_encoding_indicator = false;
		enum Encoding encoding = id3_get_encoding( buffer, &has_encoding_indicator );

		// If an encoding indicator exists, then increment the buffer pointer and decrement the buffer size.
		if( has_encoding_indicator )
		{
			buffer++;
			(*buffer_size)--;
		}


	// The buffer might contain only an Encoding indicator and nothing else.
	//
		if( *buffer_size < 1 )
			return NULL;


	// Determine if buffer contains multiple strings.
	//
		int string_count = *buffer_size;
		char** strings = id3_split_multi_string( buffer, &string_count, encoding );

		if( string_count < 1 )
		{
			*buffer_size = 0;
			return NULL;
		}


	// Get some memory.
	//
		TagFrameDatum** data = (TagFrameDatum**)xcalloc( string_count, sizeof( TagFrameDatum* ) );


	// Create frame data items.
	//
		for( int i = 0; i < string_count; i++ )
		{
			// This line finds the beginning of the next string or the end of the buffer.
			int size = i == string_count - 1 ? buffer + *buffer_size - (uint8_t*)strings[i] : strings[i+1] - strings[i];

			data[i] 			  = (TagFrameDatum*)xcalloc( 1, sizeof( TagFrameDatum ) );
			data[i]->datum 		  = (uint8_t*)convert_to_utf_8( encoding, strings[i], size );
			data[i]->encoding 	  = UTF_8;
			data[i]->size 		  = strlen( (char*)data[i]->datum );
			data[i]->storage_size = strlen( (char*)data[i]->datum ) + 2;  // Account for Encoding indicator and terminator.
		}


	free( strings );
	*buffer_size = string_count; // string_count contains the number of strings found.
	return data;
}



static TagFrameDatum** parse_frame_BINARY( const uint8_t* buffer, size_t* buffer_size )
{

	TagFrameDatum** data = (TagFrameDatum**)xcalloc( 1, sizeof( TagFrameDatum* ) );

	data[0] = (TagFrameDatum*)xcalloc( 1, sizeof( TagFrameDatum ) );
	data[0]->datum = (uint8_t*)xcalloc( *buffer_size, sizeof( uint8_t ) );

	for( unsigned int i = 0; i < *buffer_size; i++ )
		data[0]->datum[i] = buffer[i];

	data[0]->encoding = BINARY;
	data[0]->size = *buffer_size;
	data[0]->storage_size = *buffer_size;


	*buffer_size = 1;
	return data;
}



static TagFrameDatum** parse_frame_COMM( const uint8_t* buffer, size_t* buffer_size )
{

	// Ensure the frame is properly formatted.
	//
		if( *buffer_size < 4 )
		{
			print_message( "COMM frame is invalid.", WARNING, false );
			return parse_frame_TEXT( buffer, buffer_size );
		}


	// Determine Encoding of language string.
	//
		bool has_encoding_indicator = false;
		id3_get_encoding( buffer, &has_encoding_indicator );

		// If an encoding indicator exists, then increment the buffer pointer and decrement the buffer size.
		if( has_encoding_indicator )
		{
			buffer++;
			(*buffer_size)--;
		}


	TagFrameDatum** data = (TagFrameDatum**)xcalloc( 2, sizeof( TagFrameDatum* ) );

	// Set language field.
	//
		data[0] = (TagFrameDatum*)xcalloc( 1, sizeof( TagFrameDatum ) );
		data[0]->datum = (uint8_t*)xcalloc( 3, sizeof( uint8_t ) );
		data[0]->encoding = UTF_8;
		data[0]->size = 3;
		data[0]->storage_size = 5;

		for( int i = 0; i < 3; i++ )
			data[0]->datum[i] = buffer[i];


	// Move the buffer forward.  If there's no data left in the buffer, then return.
	//
		buffer += 3;
		*buffer_size -= 3;

		if( *buffer_size < 2 )
		{
			*buffer_size = 1;
			return data;
		}


	// Determine Encoding of text string.
	//
		has_encoding_indicator = false;
		enum Encoding encoding = id3_get_encoding( buffer, &has_encoding_indicator );

		// If an encoding indicator exists, then increment the buffer pointer and decrement the buffer size.
		if( has_encoding_indicator )
		{
			buffer++;
			(*buffer_size)--;
		}


	// Set text field.
	//
		data[1] = (TagFrameDatum*)xcalloc( 1, sizeof( TagFrameDatum ) );

		if( encoding == BINARY )
		{
			data[1]->datum = (uint8_t*)xcalloc( *buffer_size, sizeof( uint8_t ) );
			data[1]->encoding = UTF_8;
			data[1]->size = *buffer_size;
			data[1]->storage_size = *buffer_size;

			for( unsigned int i = 0; i < *buffer_size; i++ )
				data[1]->datum[i] = buffer[i];
		}
		else
		{
			// Special case:  Some COMM frames contain the sequence 0xFF 0xFE 0x00 0x00 before the beginning of the actual BOM and text.
			if( buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0x00 && buffer[3] == 0x00 )
			{
				buffer += 4;
				*buffer_size -= 4;
			}


			// Copy string to buffer.
			uint8_t buf[*buffer_size];
			memcpy( buf, buffer, *buffer_size );

			data[1]->datum = (uint8_t*)convert_to_utf_8( encoding, (char*)buf, *buffer_size );
			data[1]->encoding = UTF_8;
			data[1]->size = strlen( (char*)data[1]->datum );
			data[1]->storage_size = strlen( (char*)data[1]->datum ) + 1;	// Account for terminator.
		}


	*buffer_size = 2;
	return data;
}



/**
 * Returns a 32 bit unsigned integer given a 4 byte, big endian array.
 */
static uint32_t big_endian_array_to_int( const uint8_t* const num )
{
	uint32_t b0 = num[0];
	uint32_t b1 = num[1];
	uint32_t b2 = num[2];
	uint32_t b3 = num[3];

	return b0 << 24 | b1 << 16 | b2 << 8 | b3;
}



/**
 * Populates a tag frame with data according to the frame's type.
 */
static void get_tag_frame_data( TagFrame* const frame, const uint8_t* const buffer, size_t buffer_size )
{

	// Validate args.
	//
		if( frame == NULL || buffer == NULL || buffer_size < 1 )
			return;


	// TODO: support additional frame types.
	if( 'T' == frame->id[0] )
		frame->data = parse_frame_TEXT( buffer, &buffer_size );
	else if( strcmp( "COMM", frame->id ) == 0 )
		frame->data = parse_frame_COMM( buffer, &buffer_size );
	else
		frame->data = parse_frame_BINARY( buffer, &buffer_size );


	frame->data_count = buffer_size;
}



/**
 * Frees a tag frame.
 */
void free_tag_frame( TagFrame* frame )
{

	if( frame != NULL )
	{
		for( int i = 0; i < frame->data_count; i++ )
		{
			free( frame->data[i]->datum );
			free( frame->data[i] );
		}

		free( frame->data );
		free( frame );
		frame = NULL;
	}
}



/**
 * Returns a List of frames discovered in the supplied buffer.
 */
List* get_tag_frames( const uint8_t* buffer, const size_t buffer_size, const int tag_version )
{

	List* list = (List*)xcalloc( 1, sizeof( List ) );

	for( unsigned int t = 0; t < buffer_size; t++ ) // Start searching for frames after the tag header.
	{
		size_t size = sizeof( TAG_FRAME_LIST ) / sizeof( char* );

		for( unsigned int i = 0; i < size; i++ )	// Search the list of frames to see if we have a match.
		{
			if( strcmp( (char*)&buffer[t], TAG_FRAME_LIST[i] ) != 0 )
				continue;


			// Create frame.
			//
				TagFrame* frame = (TagFrame*) xcalloc( 1, sizeof( TagFrame ) );

				strcpy( frame->id, TAG_FRAME_LIST[i] );
				frame->status_flags = buffer[t+8];
				frame->format_flags = buffer[t+9];

				int data_size = 0;
				if( tag_version == 4 )
					data_size = id3_sync_safe_to_int( &buffer[t+4] );
				else
					data_size = big_endian_array_to_int( &buffer[t+4] );

				get_tag_frame_data( frame, buffer+t+10, data_size );


			// If the frame actually contains data, then add it to the frame list.
			//
				if( frame->data != NULL )
				{
					if( !list_add( list, frame ) )
						print_message( "Could not add frame to list.", ERROR, false );
				}
				else
					free( frame );

			t += data_size + 10 - 1;	// Increment the counter, the size of the frame. +1 to counter-act loop increment.
			break;
		}
	}


	return list;
}



/**
 * Compares two frames based on their IDs.
 */
static int compare_tag_frames( const void* vp1, const void* vp2 )
{

	const TagFrame *frame_1 = (const TagFrame *)vp1;
	const TagFrame *frame_2 = (const TagFrame *)vp2;

	assert( frame_1 != NULL && frame_2 != NULL );

	return strcmp( frame_1->id, frame_2->id );
}



/**
 * Returns a frame, if it is found.  If the frame is not found or an error occurs, NULL is returned.
 */
const TagFrame* find_tag_frame( const List* const list, const char* const frame_id )
{

	// Validate args.
	//
		if( list == NULL || frame_id == NULL )
			return NULL;


	// Find and return the frame.
	//
		TagFrame frame;
		strcpy( frame.id, frame_id );

		return (TagFrame*)list_find( list, (void*)&frame, compare_tag_frames );
}



/**
 * This method compares the list in the frames argument with the list of valid frame IDs, and returns
 * a list of any invalid frame IDs found.  If no invalid frame IDs are found NULL is returned.  Any frame IDs
 * returned in the invalid frame list, are pointers to the offending frame in the caller's list.  The frame_count
 * argument should specify the number of frames in the list to be inspected.  Upon return frame_count contains
 * the number of invalid frames found.
 */
const char** validate_frames( const char** const frames, int* const frame_count )
{

	// Validate args.
	//
		if( frames == NULL || *frame_count < 1 )
			return NULL;


	const char** invalid_frames = NULL;
	int invalid_frame_count = 0;

	for( int i = 0; i < *frame_count; i++ )
	{
		bool found = false;
		size_t size = sizeof( TAG_FRAME_LIST ) / sizeof( char* );

		for( unsigned int j = 0; j < size; j++ )
		{
			if( strcmp( frames[i], TAG_FRAME_LIST[j] ) == 0 )
			{
				found = true;
				break;
			}
		}

		if( !found )
		{
			if( invalid_frames == NULL )
				invalid_frames = (const char**)xcalloc( *frame_count, sizeof( char* ) );

			invalid_frames[invalid_frame_count] = frames[i];
			invalid_frame_count++;
		}
	}


	*frame_count = invalid_frame_count;
	return invalid_frames;
}



/**
 * Returns a copy of the list of available frames (a copy of the pointer array, not the actual data).
 * Upon return 'frame_count' will contain the number of frames in the list.  The caller should free the returned list.
 */
const char** get_frame_list( unsigned int* const frame_count )
{

	*frame_count = sizeof( TAG_FRAME_LIST ) / sizeof( char* );
	const char** frames = (const char**)xcalloc( 1, sizeof( TAG_FRAME_LIST ) );

	for( unsigned int i = 0; i < *frame_count; i++ )
		frames[i] = TAG_FRAME_LIST[i];


	return frames;
}
