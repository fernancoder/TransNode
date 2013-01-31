/*
 * id3_util.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "id3_util.h"
#include "xmalloc.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>



/**
 * Searches the specified buffer for one or more UTF-16 strings.  The returned array of char* contains pointers to
 * each discovered strings in the caller's buffer.  Additionally the buffer_size variable contains the number of
 * strings found.  If no strings are found NULL is returned.
 */
static char** id3_split_multi_string_utf_16( const uint8_t* buffer, int* const buffer_size, enum Encoding encoding )
{

	// Set byte order mark.
	//
		uint8_t bom1, bom2;

		if( encoding == UTF_16LE )
		{
			bom1 = 0xFF;
			bom2 = 0xFE;
		}
		else if( encoding == UTF_16BE )
		{
			bom1 = 0xFE;
			bom2 = 0xFF;
		}
		else
		{
			*buffer_size = 0;
			return NULL;
		}


	// Allocate space for some strings.
	//
		const int MAX_STRINGS = 5;
		const uint8_t** strings = (const uint8_t**)xcalloc( MAX_STRINGS, sizeof( uint8_t* ) );


	// Find strings.
	//
		int strings_found = 0;

		for( int i = 0; i < *buffer_size; i++ )
		{
			if( buffer[i] == bom1 && buffer[i+1] == bom2 )
			{
				strings[strings_found++] = &buffer[i++];

				if( strings_found >= MAX_STRINGS )	// Resize the array if necessary.
					//CODIGO ORIGINAL
					//strings = realloc( (void *)strings, sizeof( uint8_t* ) * strings_found * 2 );
					//MI CAMBIO
					strings = (const uint8_t**)xrealloc( strings, sizeof( uint8_t* ) * strings_found * 2 );
			}
		}


	*buffer_size = strings_found;
	return (char**)strings;
}



/**
 * Searches the specified buffer for one or more UTF-8 strings.  The returned array of char* contains pointers to
 * each discovered strings in the caller's buffer.  Additionally the 'buffer_size' variable contains the number of
 * strings found, upon return.  If no strings are found NULL is returned.
 */
static char** id3_split_multi_string_utf_8( const uint8_t* buffer, int* const buffer_size )
{

	// Move to the first non-zero value in the array.  The array could possibly contain all zeros.
	//
		int begin_marker = 0;
		while( begin_marker < *buffer_size && buffer[begin_marker] == 0 )
			begin_marker++;

		if( begin_marker >= *buffer_size )	// No valid strings in the buffer.
		{
			*buffer_size = 0;
			return NULL;
		}


	// Allocate space for some strings.
	//
		const int MAX_STRINGS = 5;
		const uint8_t** strings = (const uint8_t**)xcalloc( MAX_STRINGS, sizeof( uint8_t* ) );


	// Find strings.
	//
		// At this point we know buffer contains at least one string.
		int strings_found = 0;
		strings[strings_found++] = buffer;

		for( int i = begin_marker; i < *buffer_size - 1; i++ )
		{
			if( buffer[i] == 0x00 && buffer[i+1] != 0x00 )
				strings[strings_found++] = &buffer[++i];

			if( strings_found >= MAX_STRINGS )	// Resize the array if necessary.
				strings = (const uint8_t**)realloc( strings, sizeof( uint8_t* ) * strings_found * 2 );
		}


	*buffer_size = strings_found;
	return (char**)strings;
}



/**
 * This method determines if the supplied buffer contains multiple strings.  The returned array contains
 * pointers to the beginning of each string in the caller's buffer.  If no strings were found or an error occurs,
 * NULL is returned.  In addition, the number of strings found is returned in the buffer_size argument.
 */
char** id3_split_multi_string( const uint8_t* buffer, int* buffer_size, enum Encoding encoding )
{

	// Validate args.
	//
		if( buffer == NULL || *buffer_size < 1 )
			return NULL;


	// Determine the Encoding of the string.
	//
		switch( encoding )
		{
			case BINARY:
				break;

			case UTF_16LE:
			case UTF_16BE:
				return (char**)id3_split_multi_string_utf_16( buffer, buffer_size, encoding );

			case UTF_8:
			case ISO_8859_1:
				return (char**)id3_split_multi_string_utf_8( buffer, buffer_size );
		}


	return NULL;
}



/**
 * Returns an Encoding value based on the contents of the supplied buffer.  If an ID3 encoding indicator
 * is found in the buffer, the fact is communicated to the caller via the 'has_Encoding_indicator' argument.
 */
enum Encoding id3_get_encoding( const uint8_t* buffer, bool* has_encoding_indicator )
{

	// The first byte in the data buffer should be an encoding indicator which declares the encoding
	// of the frame.  We set the frame's encoding according to this indicator.
	//
	// According to the ID3 spec:
	//
	//   $00   ISO-8859-1 [ISO-8859-1]. Terminated with $00.
	//   $01   UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All strings in the same frame SHALL have the same byteorder.  Terminated with $00 00.
	//   $02   UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM.  Terminated with $00 00.
	//   $03   UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.
	//
		if( buffer[0] <= 0x04 )
		{
			*has_encoding_indicator = true;	// This indicates an Encoding indicator was found.
			return (enum Encoding)buffer[0];
		}


	*has_encoding_indicator = false;

	// No Encoding indicator provided.  Test for byte-order mark.
	//
		if( buffer[0] == 0xFF && buffer[1] == 0xFE )
			return UTF_16LE;

		if( buffer[0] == 0xFE && buffer[1] == 0xFF )
			return UTF_16BE;


	// No byte-order mark.  Test for ISO-8859-1 range.
	//
		if( buffer[0] >= 0x20 && buffer[1] <= 0x7E )
			return ISO_8859_1;


	// Presume binary.
	//
		return BINARY;
}



/**
 * Converts a sync-safe integer to an integer.
 */
int id3_sync_safe_to_int( const uint8_t* const sync_safe )
{

	uint32_t byte0 = sync_safe[0];
	uint32_t byte1 = sync_safe[1];
	uint32_t byte2 = sync_safe[2];
	uint32_t byte3 = sync_safe[3];

	return byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
}



/**
 *	Converts an integer to a sync-safe integer.  Copies a 4-byte, sync-safe integer into the caller supplied buffer.
 */
bool id3_int_to_sync_safe( uint32_t integer, uint8_t* const sync_safe )
{

	// We have to ensure the incoming value is not too large.  Since all most significant bits are zero'ed, the largest supported
	// value is 0x0FFFFFFF, since that leaves 4 empty bits to work with.
	//
		if( integer > 0x0FFFFFFF || sync_safe == NULL )
			return false;


	integer <<= 1;
	sync_safe[0] = (integer & 0x000000FF) >> 1;

	integer <<= 1;
	sync_safe[1] = (integer & 0x0000FF00) >> 9;

	integer <<= 1;
	sync_safe[2] = (integer & 0x00FF0000) >> 17;

	integer <<= 1;
	sync_safe[3] = (integer & 0xFF000000) >> 25;


	return true;
}
