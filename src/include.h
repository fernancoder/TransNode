/*
 * include.h
 *
 *  Created on: Dec 22, 2012
 *      Author: fernando
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

#define MY_NAME "TransNode"

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

const int NO_ERROR = 0;
const int TAGED_FILE_NOT_FOUND = 1;
const int CANT_CREATE_UNTAGED_FILE = 2;
const int NO_MP3_EXTENSION = 3;
const int CANT_OPEN_UNTAGUED_FILE = 4;
const int BULKID3_GET_MP3_FAULT = 5;
const int TAGED_NO_MP3_EXTENSION = 6;
const int UNTAGED_NO_MP3_EXTENSION = 7;
const int UNTAGED_FILE_NOT_FOUND = 8;
const int UNSOPORTED_TORRENT_ADD = 9;
const int SESSION_ALLREADY_CREATED = 10;
const int TORRENT_NOT_EXISTS = 11;
const int TORRENT_ALLREADY_EXISTS = 12;
const int TORRENT_INVALID_FROMAT = 13;
const int MAGNET_NOT_EXISTS = 14;

const int UNDEFINED_TORRENT_REFERNCE = 0;
const int MAGNET_TORRENT_REFERNCE = 1;
const int LINK_TORRENT_REFERNCE = 2;
const int FILE_TORRENT_REFERNCE = 3;
const int MP3_TORRENT_REFERNCE = 4;

#endif /* INCLUDE_H_ */
