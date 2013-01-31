/*
 * util.h
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#ifndef UTIL_H_
#define UTIL_H_



#include "id3_util.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>



/*****************************************************************************
 *
 * Types.
 *
 *****************************************************************************/

enum MessageType
{
	INFO,
	WARNING,
	ERROR
};



/*****************************************************************************
 *
 * Functions.
 *
 *****************************************************************************/

void 		 print_message( const char* message, const enum MessageType mess_type, bool show_sys_error );
ssize_t 	 get_file_size( int fd );
char* 		 convert_to_utf_8( enum Encoding enc, char* string_buffer, size_t str_buffer_size );
const char** split( char* const string, char delimiter, unsigned int* const token_count );
char* 		 trim( char* const string );
char*  		 replace_filename_chars( char* const filename );



#endif /* UTIL_H_ */
