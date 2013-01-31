/*
 * id3_tag.c
 *
 *      Author: J. Andrew Laughlin
 *
 *      License: See LICENSE.txt
 */



#include "id3_tag.h"
#include "id3_tag_frame.h"
#include "util.h"
#include "linked_list.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>



/**
 * Returns an ID3 version 1 tag if one is found in the specified file.  Otherwise NULL is returned.
 * Caller must call free_v1_tag() to free the tag.
 */
V1Tag* v1_get_tag( int fd )
{

	const int TAG_SIZE = 128;

	size_t size = get_file_size( fd );
	if( size < 1 )
	{
		print_message( "Error reading file size.", ERROR, true );
		return NULL;
	}


	lseek( fd, -TAG_SIZE, SEEK_END );
	V1Tag* tag = (V1Tag*)xcalloc( 1, TAG_SIZE + 1 /* 1 byte to hold size */ );
	ssize_t bytes_read = read( fd, tag, TAG_SIZE );

	if( bytes_read != TAG_SIZE )
	{
		print_message( "Error reading version 1 tag.", ERROR, true );
		free( tag );
		return NULL;
	}


	tag->size = TAG_SIZE;

	if( ((uint8_t*)tag)[0] != 'T' || ((uint8_t*)tag)[1] != 'A' || ((uint8_t*)tag)[2] != 'G' )
	{
		free( tag );
		return NULL;
	}


	// Terminate all the arrays to make valid strings.
	//
		tag->title[29]   = 0;
		tag->artist[29]  = 0;
		tag->album[29]   = 0;
		tag->comment[29] = 0;

		trim( tag->title );
		trim( tag->artist );
		trim( tag->album );
		trim( tag->comment );


	return tag;
}



/**
 * Frees a version 1 tag.
 */
void v1_free_tag( V1Tag* tag )
{

	free( tag );
	tag = NULL;
}



/**
 * Prints a version 1 tag.
 */
void v1_print_tag( const V1Tag* const tag )
{

	// Validate args.
	//
		if( tag == NULL )
			return;


	// Print tag.
	//
		printf( "\n\n\t%-16s %-30i", "ID3 Tag Version:", 1 );
		printf( "\n\t%-16s %-30i", "Size (in bytes):", 128 );
		printf( "\n\t%-16s %-30s", "Artist:", tag->artist );
		printf( "\n\t%-16s %-30s", "Title:", tag->title );
		printf( "\n\t%-16s %-30s", "Album:", tag->album );
		printf( "\n\t%-16s %-30s", "Comment:", tag->comment );
		printf( "\n\t%-16s %-30i", "Genre:", tag->genre );

		uint8_t buffer[5] = {0};
		memcpy( buffer, tag->year, 4 );
		printf( "\n\t%-16s %-30s", "Year:",  buffer );
}



/**
 * Returns an ID3 version 2 tag if one is found in the specified by file descriptor.  Otherwise NULL is returned.
 * Caller must call free_V2_tag() to free the tag.
 */
V2Tag* v2_get_tag( int fd )
{

	// Validate args.
	//
		if( fd == 0 )
			return NULL;


	lseek( fd, 0, SEEK_SET ); // Reset to the beginning of the file.


	// Get the tag size.
	//
		uint8_t header[10];
		ssize_t bytes_read = read( fd, header, 10 );

		if( bytes_read != 10 )
		{
			print_message( "Error reading ID3 version 2 tag.", ERROR, true );
			return NULL;
		}

		// Ensure this file has an ID3 tag header.
		if( header[0] != 'I' && header[1] != 'D' && header[2] != '3' )
		{
			lseek( fd, 0, SEEK_SET ); // Reset to the beginning of the file.
			return NULL;
		}


	// Create the tag.
	//
		V2Tag* tag = (V2Tag*)xcalloc( 1, sizeof( V2Tag ) );

		tag->id[0] 			= 'I';
		tag->id[1]			= 'D';
		tag->id[2] 			= '3';
		tag->major_version 	= header[3];
		tag->revision	 	= header[4];

		/* Flags */
		tag->unsynchronisation = UNSYNCHRONISATION & header[5];
		tag->extended_header = EXTENDED_HEADER & header[5];
		tag->experimental = EXPERIMENTAL & header[5];
		tag->footer = FOOTER & header[5];


	// No support for major versions < 3.
	//
		if( tag->major_version < 3 )
		{
			print_message( "Unsupported version 2.x tag.  Tag versions less than 2.3 are not supported.", INFO, false );
			lseek( fd, 0, SEEK_SET ); // Reset to the beginning of the file.
			free( tag );
			return NULL;
		}


	// Get tag frames.
	//
		int size = id3_sync_safe_to_int( &header[6] );
		uint8_t frames[size];
		bytes_read = read( fd, frames, size );

		if( bytes_read != size )
		{
			print_message( "Error reading tag frames.", ERROR, true );
			v2_free_tag( tag );
			return NULL;
		}

		tag->frame_list = get_tag_frames( frames, size, tag->major_version );


	return tag;
}



/**
 * Frees a version 2 tag.
 */
void v2_free_tag( V2Tag* tag )
{

	if( tag != NULL )
	{
		while( tag->frame_list->head != NULL )
		{
			//CODIGO ORIGINAL
			//free_tag_frame( (void*)tag->frame_list->head->data );
			//MI CODIGO
			free_tag_frame( (TagFrame*)(tag->frame_list->head->data) );
			list_remove( tag->frame_list, tag->frame_list->head );
		}

		free( tag->frame_list );
		free( tag );
		tag = NULL;
	}
}



/**
 * Prints a version 2 tag.
 */
void v2_print_tag( const V2Tag* const tag, bool verbose )
{

	// Validate args.
	//
		if( tag == NULL )
			return;


	printf( "\n\n\t%-23s 2.%i.%i", "ID3 Tag Version:", tag->major_version, tag->revision );

	if( verbose )
	{
		/* Size */
		printf( "\n\t%-23s %i", "Size (in bytes):", v2_get_tag_storage_size( tag ) );

		/* Flags */
		printf( "\n\t%-23s %s", "Unsynchronisation:", tag->unsynchronisation ? "Applied" : "Not applied" );
		printf( "\n\t%-23s %s", "Extended header:",   tag->extended_header   ? "Yes" : "No" );
		printf( "\n\t%-23s %s", "Experimental indicator:", tag->experimental ? "On" : "Off" );
		printf( "\n\t%-23s %s", "Footer:", tag->footer ? "Present" : "Not present" );
	}

	printf( "\n\n\t[Frames - %i]", (int)list_size( tag->frame_list ) );
	printf( "\n\t\t Type \t Size \t Value\n\t\t------------------------------------------------------------" );


	// Iterate through frames.
	//
		Node* node = tag->frame_list->head;

		while( node != NULL )
		{
			TagFrame* frame = (TagFrame*)node->data;

			printf( "\n\t\t %s", frame->id );

			// Print size.
			int size = 0;

			for( int j = 0; j < frame->data_count; j++ )
				size += frame->data[j]->size;

			printf( "\t %i", size );

			// Print value.
			for( int j = 0; j < frame->data_count; j++ )
				printf( "\t %s", frame->data[j]->datum );

			node = node->next;
		}
}



/**
 * Cleans up the version 2 tag.  All tags are updated to version 2.4.
 */
void v2_sanitize_tag( V2Tag* const tag, const char** const retained_frames, int frame_count )
{

	// Validate args.
	//
		if( retained_frames == NULL || tag == NULL || frame_count < 1 )
			return;


	// Update tag header.
	//
		tag->major_version = 0x04;
		tag->revision	   = 0x00;

		/* Turn off all flags. */
		tag->unsynchronisation = false;
		tag->extended_header = false;
		tag->experimental = false;
		tag->footer = false;


	// Search the list of retained frames to see if we should keep this one.
	//
		if( tag->frame_list != NULL )
		{
			Node* node = tag->frame_list->head;

			while( node != NULL )
			{
				TagFrame* frame = (TagFrame*)node->data;
				bool found = false;

				// Search the list of retained frames for the current frame.  If not found, then delete it from the tag.
				for( int i = 0; i < frame_count; i++ )
				{
					if( strcmp( retained_frames[i], frame->id ) == 0 )
					{
						found = true;
						break;
					}
				}

				// Remove the frame.
				if( !found )
				{
					free_tag_frame( frame );

					Node* temp = node->next;	// The current node will get deleted.
					if( !list_remove( tag->frame_list, node ) )
						print_message( "Could not remove frame from frame list.", ERROR, false );

					node = temp;
				}
				else
					node = node->next;
			}
		}
}



/**
 * Returns an ID3 version 2 tag as an array of bytes.
 */
uint8_t* v2_get_tag_as_buffer( const V2Tag* const tag )
{

	// Create a buffer for the tag.
	//
		int tag_size = v2_get_tag_storage_size( tag );
		uint8_t* buffer = (uint8_t*)xcalloc( tag_size, sizeof( uint8_t ) );


	// Write tag header.
	//
		buffer[0] = tag->id[0];	// ID3
		buffer[1] = tag->id[1];
		buffer[2] = tag->id[2];
		buffer[3] = tag->major_version;
		buffer[4] = tag->revision;

		uint8_t flags = 0;
		flags |= tag->unsynchronisation;
		flags |= tag->extended_header;
		flags |= tag->experimental;
		flags |= tag->footer;


		uint8_t sync_safe[4] = {0};

		if( !id3_int_to_sync_safe( tag_size - 10, sync_safe ) )
			return false;

		buffer[6] = sync_safe[3];
		buffer[7] = sync_safe[2];
		buffer[8] = sync_safe[1];
		buffer[9] = sync_safe[0];


	// Write frames.
	//
		int buffer_index = 10;
		Node* node = tag->frame_list->head;

		while( node != NULL )
		{
			TagFrame* frame = (TagFrame*)node->data;

			// Set frame ID.
			//
				buffer[buffer_index++] = frame->id[0];
				buffer[buffer_index++] = frame->id[1];
				buffer[buffer_index++] = frame->id[2];
				buffer[buffer_index++] = frame->id[3];

			// Set frame size.
			//
				int size = 0;
				for( int i = 0; i < frame->data_count; i++ )
					size += frame->data[i]->storage_size;

				if( !id3_int_to_sync_safe( size, sync_safe ) )
				{
					print_message( "Error creating sync-safe integer.", ERROR, false );
					free( buffer );
					return NULL;
				}

				buffer[buffer_index++] = sync_safe[3];
				buffer[buffer_index++] = sync_safe[2];
				buffer[buffer_index++] = sync_safe[1];
				buffer[buffer_index++] = sync_safe[0];

			// Set frame flags.
			//
				buffer[buffer_index++] = frame->status_flags;
				buffer[buffer_index++] = frame->format_flags;

			// Set frame Encoding.
			//
				if( frame->data[0]->encoding != BINARY )
					buffer[buffer_index++] = frame->data[0]->encoding;

			// Set each data item.
			//
				for( int i = 0; i < frame->data_count; i++ )
				{
					// Copy item data.
					for( unsigned int j = 0; j < frame->data[i]->size; j++ )
						buffer[buffer_index++] = frame->data[i]->datum[j];

					if( frame->data[i]->encoding != BINARY )
						buffer[buffer_index++] = 0x00;	// Terminate.
				}

			node = node->next;
		}


	return buffer;
}



/**
 * Calculates the on-disk storage size of a version 2 tag, in bytes.
 */
int v2_get_tag_storage_size( const V2Tag* const tag )
{

	// Validate args.
	//
		if( tag == NULL )
			return 0;


	// Calculate space necessary for this tag.
	//
		int tag_size = 10; // Account for tag header.

		// Iterate the list.
		Node* node = tag->frame_list->head;

		while( node != NULL )
		{
			tag_size += 10; // Account for frame header.

			for( int j = 0; j < ((TagFrame*)node->data)->data_count; j++ )
				tag_size += ((TagFrame*)node->data)->data[j]->storage_size;

			node = node->next;
		}


	return tag_size;
}
