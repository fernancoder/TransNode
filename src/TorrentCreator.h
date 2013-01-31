/*
 * TorrentCreator.h
 *
 *  Created on: Dec 28, 2012
 *      Author: fernando
 */

#ifndef TORRENTCREATOR_H_
#define TORRENTCREATOR_H_

#include "include.h"
#include "inttypes.h"
#include "sys/stat.h"

class TorrentCreator {
		protected:
			string filePath;
			string torrentFilePath;
			int pieceSize;

		public:
			TorrentCreator(string filePath, string torrentFilePath, int piceSize);
			virtual ~TorrentCreator();

		public:
			int execute();

		private:
			int verifyFileExtension(string filePath);
			int verifyFileBeing(string filePath);
			int generateTorrentFile();
};

#endif /* TORRENTCREATOR_H_ */
