/*
 * PathsManager.cpp
 *
 *  Created on: Jan 3, 2013
 *      Author: fernando
 */

#include "PathsManager.h"
#include "Utils.h"

PathsManager::PathsManager(string orgFilePath) {

	this->orgFilePath = orgFilePath;

	this->uframeDirectory = string("");
	this->torrentDirectory = string("");
	this->workingDirectory = string("/tmp/");
	this->fileHash = string("");
	this->temporalFilePath = string("");
	this->uframeFilePath = string("");
	this->torrentFilePath = string("");

	this->isCalculated = false;
	this->errorStatus = PM_NO_ERROR;
}

PathsManager::~PathsManager() {
	// TODO Auto-generated destructor stub
}

void PathsManager::calculate(bool force)
{
	if ( this->errorStatus != PM_NO_ERROR)
		return;

	if ( this->isCalculated && !force )
		return;

	//Directorio mp3 origen
	if ( this->orgDirectory.compare("") == 0 )
		this->orgDirectory = tNode::Utils::formatDirectory(tNode::Utils::extractDirectoryFromFilePath(this->orgFilePath));

	//Directorio mp3 untaged
	if ( this->uframeDirectory.compare("") == 0 )
		this->uframeDirectory = tNode::Utils::formatDirectory(this->orgDirectory);

	//Directorio torrent
	if ( this->torrentDirectory.compare("") == 0 )
		this->torrentDirectory = tNode::Utils::formatDirectory(this->orgDirectory);

	//Fichero temporal
	if ( this->temporalFilePath.compare("") == 0 )
	{
		this->temporalFilePath = this->workingDirectory + tNode::Utils::generateRandomString(PM_WORKING_FILENAME_LENGTH) + ".mp3";
	}

	if ( tNode::Utils::fileExists(this->temporalFilePath) ) //Existe el fichero temporal
	{
		if ( this->uframeFilePath.compare("") == 0 || this->torrentFilePath.compare("") == 0 ) //No estan definidas las rutas al mp3 o torrent
		{
			//Calcula el sha1 del fichero temporalFilePath
			this->fileHash = tNode::Utils::sha1FromFile(this->temporalFilePath);

			//Asigna vaslor a uframeFilePath
			this->uframeFilePath = this->uframeDirectory + this->fileHash + ".mp3";

			//Asigna vaslor a torrentFilePath
			this->torrentFilePath = this->torrentDirectory + this->fileHash + ".torrent";

			//Copia el fichero a la ruta establecida
			tNode::Utils::copyFile(this->temporalFilePath, this->uframeFilePath);

			//Borra el fichero
			tNode::Utils::deleteFile(this->temporalFilePath);
		}
	}

	this->isCalculated = true;
}

int PathsManager::getErrorStatus()
{
	this->calculate();
	return this->errorStatus;
}

void PathsManager::setUframeDirectory(string directoryName)
{
	this->isCalculated = false;
	this->uframeDirectory = tNode::Utils::formatDirectory(directoryName);
}

void PathsManager::setTorrentDirectory(string directoryName)
{
	this->isCalculated = false;
	this->torrentDirectory = tNode::Utils::formatDirectory(directoryName);
}

void PathsManager::setWorkingDirectory(string directoryName)
{
	this->isCalculated = false;
	this->workingDirectory = tNode::Utils::formatDirectory(directoryName);
}

void PathsManager::setPieceSize(long piceSize)
{
	this->pieceSize = piceSize;
}

string PathsManager::getOrgFilePath()
{
	return this->orgFilePath;
}

string PathsManager::getTemporalFile()
{
	this->calculate();
	return this->temporalFilePath;
}

string PathsManager::getUframeFilePath()
{
	this->calculate(true);
	if ( this->uframeFilePath.compare("") == 0 )
		this->errorStatus = PM_GENERATION_ERROR;
	return this->uframeFilePath;
}

string PathsManager::getTorrentFilePath()
{
	this->calculate(true);
	if ( this->torrentFilePath.compare("") == 0 )
		this->errorStatus = PM_GENERATION_ERROR;
	return this->torrentFilePath;
}

string PathsManager::getFileHash()
{
	return this->fileHash;
}

long PathsManager::getPieceSize()
{
	return this->pieceSize;
}

