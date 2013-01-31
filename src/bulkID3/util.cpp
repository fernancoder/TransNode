/*
 * util.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "util.h"
#include "xmalloc.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <iconv.h>
#include <dirent.h>
#include <ctype.h>



/**
 * Constants.
 */
const char PATH_SEPARATOR = '/';



/**
 * Prints the specified message.
 */
void print_message( const char* message, const enum MessageType message_type, bool show_sys_error )
{
	char message_buff[strlen( message ) + 15];
	memset( message_buff, 0, strlen( message ) + 15 );

	switch( message_type )
	{
		case INFO:
			strcat( message_buff, "INFO: " );
			break;

		case WARNING:
			strcat( message_buff, "WARNING: " );
			break;

		case ERROR:
			strcat( message_buff, "ERROR: " );
			break;

		default:
			strcat( message_buff, "This is a bug! " );
	}


	strcat( message_buff, message );

	if( show_sys_error )
		printf( "\n\n\t%s  %s\n", message_buff, strerror( errno ) );
	else
		printf( "\n\n\t%s\n", message_buff );
}



/**
 * Returns the size of the file specified by the file descriptor.
 */
ssize_t get_file_size( int fd )
{
	struct stat file_stat;

	if( fstat( fd, &file_stat ) )
		return 0;
	else
		return file_stat.st_size;
}



/**
 * Converts the specified string buffer of size 'str_buffer_size' from the specified source Encoding' to UTF-8.
 * The caller should free the returned string.
 */
char* convert_to_utf_8( enum Encoding encoding, char* string_buffer, size_t str_buffer_size )
{

	// Validate args.
	//
		if( string_buffer == NULL || str_buffer_size < 1 )
			return NULL;


	// Create Encoding string to pass to iconv_open().
	//
		char* src_encoding = NULL;

		switch( encoding )
		{
			case UTF_16LE:
			case UTF_16BE:
				src_encoding = "UTF-16";
				break;

			case UTF_8:
				src_encoding = "UTF-8";
				break;

			case ISO_8859_1:
				src_encoding = "ISO-8859-1";
				break;

			default:
				return NULL;
		}


	// Convert to UTF-8.
	//
		iconv_t conv = iconv_open( "UTF-8", src_encoding );

		if( conv == (iconv_t) -1 )
		{
			print_message( "iconv_open() failed.", ERROR, true );
			return NULL;
		}

		// Create buffer to hold the new UTF-8 string.
		size_t output_buffer_size = str_buffer_size * 2;
		char* output_buffer = (char*)xcalloc( output_buffer_size, sizeof( char ) );
		char* result = output_buffer; // iconv() will change the output_buffer pointer, so we need a copy.

		size_t conversions = iconv( conv, &string_buffer, &str_buffer_size, &output_buffer, &output_buffer_size );

		if( conversions == (size_t)-1 )	// An error occurred.
		{
			print_message( "iconv() failed", ERROR, true );
			free( result );
			result = NULL;
		}

		iconv_close( conv );


	return result;
}



/**
 * Standard split() method.  Returns a string array which must be freed by the caller.
 */
const char** split( char* const string, char delimiter, unsigned int* const token_count )
{

	// Validate args.
	//
		if( string == NULL || strlen( string ) < 1 )
			return NULL;


	// Parse the list of tokens.
	//
		*token_count = 0;
		const char* token = strtok( string, &delimiter );	// Get the first token.

		if( token == NULL )	// No tokens.
			return NULL;

		int num_tokens = 10;	// Initial number of tokens.
		const char** tokens = (const char**)xcalloc( num_tokens, sizeof( char* ) );

		tokens[0] = token;
		(*token_count)++;

		for( int i = 1; (token = strtok( NULL, &delimiter )) != NULL; i++ )
		{
			tokens[i] = token;
			(*token_count)++;

			if( i >= num_tokens )
				//CODIGO ORIGINAL
				//tokens = xrealloc( tokens, (num_tokens *= 2 ) );
				//MI CODIGO
				tokens = (const char**)xrealloc( tokens, (num_tokens *= 2 ) );
		}


	return tokens;
}



/**
 * Trims the whitespace from a string.  The same pointer specified by the caller is returned, for convenience.
 */
char* trim( char* const string )
{

	// Validate args.
	//
		if( string == NULL )
			return string;


	// Find the index of the first non-whitespace character.
	//
		size_t begin = 0;
		size_t length = strlen( string );
		while( begin < length && isspace( string[begin] ) )
			begin++;


	// Find the index of the last non-whitespace character.
	//
		size_t end = strlen( string ) - 1;
		while( end > 0 && isspace( string[end] ) )
			end--;

		string[end+1] = 0;


	// Entire string is whitespace.
	//
		if( begin > end )
		{
			string[0] = 0;
			return string;
		}


	// Copy the actual string to the beginning of the buffer, then terminate it.
	//
		if( begin > 0 )
			for( unsigned int i = 0; i < end - begin; i++ )
				string[i] = begin + i;


	return string;
}



/**
 * Replaces characters in a filename with a replacement character.
 */
char* replace_filename_chars( char* const filename )
{
	const char invalid_chars[] = { '\\', '/', };
	const int SIZE = 2;
	const char REPLACEMENT = '-';
	size_t length = strlen( filename );

	for( unsigned int i = 0; i < length; i++ )
		for( int j = 0; j < SIZE; j++ )
			if( filename[i] == invalid_chars[j] )
				filename[i] = REPLACEMENT;


	return filename;
}
