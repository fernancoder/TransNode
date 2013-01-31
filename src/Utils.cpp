/*
 * Utils.cpp
 *
 *  Created on: Jan 4, 2013
 *      Author: fernando
 */

#include "Utils.h"

using namespace tNode;

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

int Utils::copyFile(string orgFilePath, string desFilePath)
{
	ifstream source(orgFilePath.c_str(), ios::binary);
	if ( !source )
		return UTIL_SOURCE_FILE_NOT_FOUND;

	ofstream dest(desFilePath.c_str(), ios::binary);
	if ( !dest )
		return UTIL_CANT_OPEN_DESTINATION_FILE;

	dest << source.rdbuf();

	return UTIL_NO_ERROR;
}

int Utils::deleteFile(string filePath)
{
	remove(filePath.c_str());
	return UTIL_NO_ERROR;
}

string Utils::generateRandomString(int strLen)
{
    static string charset = "0123456789ABCDEF";
    string result;
    result.resize(strLen);

    srand(time(NULL));
    for (int i = 0; i < strLen; i++)
        result[i] = charset[rand() % charset.length()];

    return result;
}

string Utils::extractDirectoryFromFilePath(string filePath)
{
	string directory;
	const size_t last_slash_idx = filePath.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
	    directory = filePath.substr(0, last_slash_idx);
	}

	return directory;
}

string Utils::formatDirectory(string path)
{
	if ( *path.rbegin() != '/' )
		path.append("/");

	return path;
}

bool Utils::fileExists(string fileName)
{
	ifstream ifile(fileName.c_str());
	if (ifile)
	{
		ifile.close();
		return true;
	}
	return false;
}


string Utils::bufferToHexString(const unsigned char *input, int inputSize)
{
    static const char* const lut = "0123456789ABCDEF";

    std::string output;
    output.reserve(2 * inputSize);
    for (size_t i = 0; i < inputSize; ++i)
    {
        const unsigned char c = input[i];

        int x = c >> 4;
        int y = c & 15;

        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

string Utils::sha1FromFile(string fileName)
{
	char buffer[UTIL_SHA1_CHUNK_SIZE];
	unsigned char result[UTIL_SHA1_SIZE];
	SHA_CTX shaContext;

	ifstream ifFh(fileName.c_str(), ios::in | ios::binary);
	if ( ifFh.fail() )
		return string("");

	SHA1_Init(&shaContext);

	while ( ifFh.read(buffer,UTIL_SHA1_CHUNK_SIZE).gcount() != 0 )
	{
		SHA1_Update(&shaContext, buffer, ifFh.gcount());
	}

	SHA1_Final(result, &shaContext);

	ifFh.close();

	return Utils::bufferToHexString(result, UTIL_SHA1_SIZE);
}

bool Utils::isCurlURL(string fileName)
{
    return (fileName.size() >= 8 &&
    		(!fileName.compare(0,6,"ftp://") ||
    		!fileName.compare(0,7,"http://") ||
    		!fileName.compare(0,8,"https://")) );
}

char *Utils::stringToCharPtr(string buffer)
{
	char *response = new char [buffer.length()+1];
	strcpy(response, buffer.c_str());
	return response;
}

map<string,string> Utils::simpleJsonToMap(string buffer)
{
	map<string,string> mapResponse;

	char keyBuff[UTIL_MAX_JSON_KEY_LENGTH];
	char valueBuff[UTIL_MAX_JSON_VALUE_LENGTH];
	char *ptrKeyBuff;
	char *ptrValueBuff;

	const char *charBuffer = buffer.c_str();
	const char *curPos = charBuffer;
	int status = 0;
	bool isEnd = false;

	while ( *curPos && !isEnd)
	{
		switch (status)
		{
			case 0: //Busca primer '{'
				if ( *curPos == '{' )
					status = 1;
			break;
			case 1: //Busca etiqueta
				if ( *curPos == '}' ) //Detecta el final
				{
					isEnd = true;
				}
				else if ( *curPos == '\"' )
				{
					ptrKeyBuff = keyBuff;
					status = 2;
				}
			break;
			case 2: //Lee etiqueta
				if ( *curPos == '\"')
				{
					*ptrKeyBuff = '\0';
					status = 3;
				}
				else
					*(ptrKeyBuff++) = *curPos;
			break;
			case 3: //Busca valor
				if ( *curPos != ':' && *curPos != ' ')
				{
					ptrValueBuff = valueBuff;
					*(ptrValueBuff++) = *curPos;
					status = 4;
				}
			break;
			case 4: //Lee valor
				if ( *curPos == ',' || *curPos == '\n')
				{
					*ptrValueBuff = '\0';
					mapResponse[keyBuff] = valueBuff;
					status = 1;
				}
				else
					*(ptrValueBuff++) = *curPos;
			break;
		}

		curPos++;
	}

	return mapResponse;
}

string Utils::RemoveQuotes(string buffer)
{
	string response = buffer.substr(1,string::npos);
	return response.substr(0,response.length()-1);
}

string Utils::folatToPercent(float value)
{
	ostringstream ss;
	int roundValue = value * 100;
	ss << roundValue;
	string response = ss.str();
	response += "%";
	return response;
}

string Utils::intToString(int value)
{
	ostringstream ss;
	ss << value;
	string response = ss.str();
	return response;
}

