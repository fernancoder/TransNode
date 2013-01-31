/*
 * TorrentCreator.cpp
 *
 *  Created on: Dec 28, 2012
 *      Author: fernando
 */

#include "transNode.h"
#include "TorrentCreator.h"

TorrentCreator::TorrentCreator(string filePath, string torrentFilePath, int pieceSize)
{
	this->filePath = filePath;
	this->torrentFilePath = torrentFilePath;
	this->pieceSize = pieceSize;
}

TorrentCreator::~TorrentCreator()
{
	// TODO Auto-generated destructor stub
}

int TorrentCreator::execute()
{
	if ( this->verifyFileExtension(this->filePath) != NO_ERROR )
		return UNTAGED_NO_MP3_EXTENSION;

	if ( this->verifyFileBeing(this->filePath) != NO_ERROR )
		return UNTAGED_FILE_NOT_FOUND;

	return this->generateTorrentFile();
}

int TorrentCreator::verifyFileExtension(string filePath)
{
	const char *ptrFilePath = filePath.c_str();

	int length = strlen( ptrFilePath );
	if( length < 5 || strcasecmp( &ptrFilePath[length - 4], ".mp3" ) != 0 )
		return NO_MP3_EXTENSION;

	return NO_ERROR;
}

int TorrentCreator::verifyFileBeing(string filePath)
{
	ifstream source(this->filePath.c_str(), ios::binary);
	if ( !source )
		return UNTAGED_FILE_NOT_FOUND;

	return NO_ERROR;
}

int TorrentCreator::generateTorrentFile()
{
	tr_metainfo_builder * builder = poemMakeMetaInfo(this->filePath.c_str(), this->torrentFilePath.c_str(), this->pieceSize);

	return NO_ERROR;
}



