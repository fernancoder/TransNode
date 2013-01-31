/*
 * PathsManager.h
 *
 *  Created on: Jan 3, 2013
 *      Author: fernando
 */

#ifndef PATHSMANAGER_H_
#define PATHSMANAGER_H_

#include "include.h"

const int PM_WORKING_FILENAME_LENGTH = 40;

const int PM_NO_ERROR = 0;
const int PM_GENERATION_ERROR = 1;

class PathsManager
{
	private:
		bool isCalculated;
		int errorStatus;

		string orgFilePath;

		string orgDirectory;
		string uframeDirectory;
		string torrentDirectory;
		string workingDirectory;

		string fileHash;

		string temporalFilePath;
		string uframeFilePath;
		string torrentFilePath;

		long pieceSize;

	private:
		void calculate(bool force=true);

	public:
		PathsManager(string orgFilePath);
		virtual ~PathsManager();

	public:
		void setUframeDirectory(string directoryName);
		void setTorrentDirectory(string directoryName);
		void setPieceSize(long piceSize);
		void setWorkingDirectory(string directoryName);

		string getOrgFilePath();
		string getUframeFilePath();
		string getTorrentFilePath();
		string getTemporalFile();
		string getFileHash();
		long getPieceSize();

		int getErrorStatus();
};

#endif /* PATHSMANAGER_H_ */
