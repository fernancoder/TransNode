/*
 * id3_tag.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef ID3_TAG_H_
#define ID3_TAG_H_



#include "linked_list.h"
#include <stdint.h>
#include <stdbool.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

enum TagHeaderFlags
{
	UNSYNCHRONISATION 	= 0x08,
	EXTENDED_HEADER		= 0x04,
	EXPERIMENTAL		= 0x02,
	FOOTER				= 0x01
};



/**
 * Represents an ID3 version 1 tag.
 */
typedef struct
{
	char 	id[3];		// Set to TAG
	char 	title[30];
	char 	artist[30];
	char 	album[30];
	char 	year[4];
	char 	comment[30];
	uint8_t genre;
	uint8_t	size;		// Always 128.

} V1Tag;


/**
 * Represents an ID3 version 2 tag.
 */
typedef struct
{
    char 	id[3];			// Set to ID3.
    uint8_t major_version;
    uint8_t revision;

    /* Flags */
    bool unsynchronisation;
    bool extended_header;
    bool experimental;
    bool footer;

    List*	frame_list;

} V2Tag;



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

V1Tag* 	 v1_get_tag( int fd );
void 	 v1_free_tag( V1Tag* tag );
void 	 v1_print_tag( const V1Tag* const tag );

V2Tag* 	 v2_get_tag( int fd );
void 	 v2_free_tag( V2Tag* tag );
void 	 v2_print_tag( const V2Tag* const tag, bool verbose );
void 	 v2_sanitize_tag( V2Tag* const tag, const char** const retained_frames, int frame_count );
uint8_t* v2_get_tag_as_buffer( const V2Tag* const tag );
int 	 v2_get_tag_storage_size( const V2Tag* const tag );



#endif /* ID3_TAG_H_ */
