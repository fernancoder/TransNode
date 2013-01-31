/*
 * id3_util.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef ID3_UTIL_H_
#define ID3_UTIL_H_



#include <stdint.h>
#include <stdbool.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

/**
 * The values of the Encodings below correspond to the Encoding marker values in the ID3 specification.
 */
enum Encoding
{
	ISO_8859_1 	= 0x00,
	UTF_16LE	= 0x01,
	UTF_16BE	= 0x02,
	UTF_8		= 0x03,
	BINARY		= 0x04
};



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

char** 			id3_split_multi_string( const uint8_t* buffer, int* buffer_size, enum Encoding enc );
enum Encoding 	id3_get_encoding( const uint8_t* buffer, bool* has_encoding_indicator );
int 			id3_sync_safe_to_int( const uint8_t* const sync_safe );
bool 			id3_int_to_sync_safe( uint32_t integer, uint8_t* const sync_safe );



#endif /* ID3_UTIL_H_ */
