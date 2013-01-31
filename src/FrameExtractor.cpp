/*
 * FrameExtractor.cpp
 *
 *  Created on: Dec 22, 2012
 *      Author: fernando
 */

#include "transNode.h"
#include "Utils.h"
#include "FrameExtractor.h"

//EXIGENCIAS BULKID3
static const char** g_frames   		  = NULL;
static unsigned int	g_frame_count	  = 0;
/////////////////////

FrameExtractor::FrameExtractor(string orgFilePath, string desFilePath)
{
	this->orgFilePath = orgFilePath;
	this->desFilePath = desFilePath;
}

FrameExtractor::~FrameExtractor()
{
	// TODO Auto-generated destructor stub
}

int FrameExtractor::execute()
{
	if ( this->verifyFileExtension(this->orgFilePath) != NO_ERROR )
		return TAGED_NO_MP3_EXTENSION;

	if ( this->verifyFileExtension(this->desFilePath) != NO_ERROR )
		return UNTAGED_NO_MP3_EXTENSION;

	int result = this->copyFile();
	if ( result != NO_ERROR )
		return result;

	return this->processFile();
}

int FrameExtractor::verifyFileExtension(string filePath)
{
	const char *ptrFilePath = filePath.c_str();

	int length = strlen( ptrFilePath );
	if( length < 5 || strcasecmp( &ptrFilePath[length - 4], ".mp3" ) != 0 )
		return NO_MP3_EXTENSION;

	return NO_ERROR;
}

int FrameExtractor::copyFile()
{
	return tNode::Utils::copyFile(this->orgFilePath, this->desFilePath);
}

int FrameExtractor::processFile()
{
	int fd = open( this->desFilePath.c_str(), O_RDWR );
	if( fd < 0 )
		return CANT_OPEN_UNTAGUED_FILE;

	// Create Mp3 instance.
	//
	Mp3* mp3 = get_mp3( this->desFilePath.c_str(), fd, false );

	if( mp3 == NULL )
	{
		close( fd );
		return BULKID3_GET_MP3_FAULT;
	}

	//ME INVENTO UN NOMBRE DE FRAME FALSO
	char* frame_string = strdup( "POEM" );
	g_frames = (const char**)split( frame_string, ',', &g_frame_count );
	free(frame_string);
	//////////////////////////////////////


	// Sanitize the tag.  This should be done after the file rename/move since this
	// operation may strip essential tags, depending on the user's request.
	//
	if( mp3->v2_tag != NULL )
		v2_sanitize_tag( mp3->v2_tag, g_frames, g_frame_count );


	// Write the file and set permission.
	//
	write_file( mp3, fd );
	fchmod( fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );

	// Set metrics.
	//
	/* CREO QUE SOBRA
	static long long tag_size_sum = 0;
	tag_size_sum += v2_get_tag_storage_size( mp3->v2_tag );
	g_mean_tag_size = tag_size_sum / g_files_processed;
	*/

	// Clean up.
	//
	free_mp3( mp3 );
	close( fd );

	return NO_ERROR;
}

bool FrameExtractor::write_file( const Mp3* mp3, int fd )
{
	// Reset to beginning of file.
	lseek( fd, SEEK_SET, 0 );

	size_t tag_size = v2_get_tag_storage_size( mp3->v2_tag );

	// Write the tag to file.
	//
		if( mp3->v2_tag != NULL )
		{
			// If after sanitization, the tag contains no frames, don't write the tag.
			if( list_size( mp3->v2_tag->frame_list ) < 1 )
			{
				//print_message( "After sanitization, tag contains no frames.  Not writing tag.  Please manually create a tag for this file.", WARNING, false );
			}
			else
			{
				uint8_t* buffer = v2_get_tag_as_buffer( mp3->v2_tag );
				if( buffer == NULL || write( fd, buffer, tag_size ) != (ssize_t)tag_size )
				{
					print_message( "Error writing tag to file.", ERROR, true );
					lseek( fd, SEEK_SET, 0 );	// Reset and attempt to write data.
				}

				free( buffer );
			}
		}


	// Write mp3 data.
	//
		if( write( fd, mp3->data_offset, mp3->data_size ) != (ssize_t)mp3->data_size )
			print_message( "Error writing mp3 data to file.", ERROR, true );
		else
		{
			if( ftruncate( fd, tag_size + mp3->data_size ) == -1 )
				print_message( "Failed to truncate the file.", ERROR, true );
		}


	return true;
}
