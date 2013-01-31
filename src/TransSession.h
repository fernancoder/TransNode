/*
 * TransSession.h
 *
 *  Created on: Jan 17, 2013
 *      Author: fernando
 */

#ifndef TRANSSESSION_H_
#define TRANSSESSION_H_

#include "include.h"

#include "Utils.h"

#include "libtransmission/transmission.h"
#include "libtransmission/bencode.h"
#include "libtransmission/tr-getopt.h"
#include "libtransmission/utils.h"
#include "libtransmission/version.h"
#include "libtransmission/torrent.h"
#include "libtransmission/session.h"
#include "libtransmission/web.h"
#include "transmissionDaemon/watch.h"


#define PREF_KEY_DIR_WATCH          "watch-dir"
#define PREF_KEY_DIR_WATCH_ENABLED  "watch-dir-enabled"

#define MEM_K 1024
#define MEM_K_STR "KiB"
#define MEM_M_STR "MiB"
#define MEM_G_STR "GiB"
#define MEM_T_STR "TiB"

#define DISK_K 1000
#define DISK_B_STR  "B"
#define DISK_K_STR "kB"
#define DISK_M_STR "MB"
#define DISK_G_STR "GB"
#define DISK_T_STR "TB"

#define SPEED_K 1000
#define SPEED_B_STR  "B/s"
#define SPEED_K_STR "kB/s"
#define SPEED_M_STR "MB/s"
#define SPEED_G_STR "GB/s"
#define SPEED_T_STR "TB/s"

struct TorrentStat
{
	int error;
	char errorString[512];
	int eta;
	int id;
	bool isFinished;
	uint64_t leftUntilDone;
	char name[512];
	int peersGettingFromUs;
	int peersSendingToUs;
	float rateDownload;
	float rateUpload;
	uint64_t sizeWhenDone;
	tr_torrent_activity status;
	float uploadRatio;
};

struct TorrentsStat
{
	int totalTorrents;
	TorrentStat* torrentStatList;
};

void *idleFunction(void *ptrSession);

class TransSession {

	private:
		dtr_watchdir * watchdir;
		tr_session * session;

		string sessionConfigDir;
		pthread_t ideleThtread;

	public:
		TransSession(string sessionConfigDir);
		virtual ~TransSession();
		string getTorrentStatusString (TorrentStat torrentStat);
		string start();
		void idle();
		void stop();

		int addTorrent(string filePath);
		int addMagnet(string filePath);
		int addURL(string filePath);
		struct TorrentsStat *listTorrents();
		int stopTorrent(int torrentId);
		int startTorrent(int torrentId);
		int removeTorrent(int torrentId, bool deleteLocalData);
};

#endif /* TRANSSESSION_H_ */
