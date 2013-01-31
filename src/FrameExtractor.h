/*
 * FrameExtractor.h
 *
 *  Created on: Dec 22, 2012
 *      Author: fernando
 */

#ifndef FRAMEEXTRACTOR_H_
#define FRAMEEXTRACTOR_H_

#include "include.h"
#include "bulkID3/mp3.h"
#include "bulkID3/util.h"

class FrameExtractor {
	protected:
		string orgFilePath;
		string desFilePath;

	public:
		FrameExtractor(string orgFilePath, string desFilePath);
		virtual ~FrameExtractor();

	public:
		int execute();

	private:
		int verifyFileExtension(string filePath);
		int copyFile();
		int processFile();

	//BULKID3
	bool write_file( const Mp3* mp3, int fd );

};

#endif /* FRAMEEXTRACTOR_H_ */
