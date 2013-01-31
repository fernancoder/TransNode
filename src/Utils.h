/*
 * Utils.h
 *
 *  Created on: Jan 4, 2013
 *      Author: fernando
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "include.h"
#include <openssl/sha.h>

const int UTIL_WORKING_FILENAME_LENGTH = 40;
const long UTIL_SHA1_CHUNK_SIZE = 32768;	//32K
const int UTIL_SHA1_SIZE = 20;

const int UTIL_NO_ERROR = 0;
const int UTIL_SOURCE_FILE_NOT_FOUND = 1;
const int UTIL_CANT_OPEN_DESTINATION_FILE = 2;

const int UTIL_MAX_JSON_KEY_LENGTH = 256;
const int UTIL_MAX_JSON_VALUE_LENGTH = 256;

namespace tNode
{
class Utils {
public:
	Utils();
	virtual ~Utils();

	static int copyFile(string orgFilePath, string desFilePath);
	static int deleteFile(string desFilePath);
	static string generateRandomString(int strLen);
	static string extractDirectoryFromFilePath(string filePath);
	static string formatDirectory(string path);
	static bool fileExists(string fileName1);
	static string bufferToHexString(const unsigned char *input, int inputSize);
	static string sha1FromFile(string fileName);
	static bool isCurlURL(string fileName);
	static char *stringToCharPtr(string buffer);
	static map<string,string> simpleJsonToMap(string buffer);
	static string RemoveQuotes(string buffer);
	static string folatToPercent(float value);
	static string intToString(int value);
};

} //END NAMESPACE

#endif /* UTILS_H_ */
