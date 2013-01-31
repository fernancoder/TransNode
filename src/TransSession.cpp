/*
 * TransSession.cpp
 *
 *  Created on: Jan 17, 2013
 *      Author: fernando
 */

#include "TransSession.h"

/* DE MOMENTO CUELA PERO NO PUEDE SER ESTA KAKA DE ESTATICOS*/
static bool paused = false;
static bool closing = false;
static FILE *logfile = NULL;
static tr_session * mySession = NULL;

struct add_torrent_idle_data
{
    struct tr_rpc_idle_data * data;
    tr_ctor * ctor;
};

static tr_rpc_callback_status
on_rpc_callback (tr_session            * session UNUSED,
                 tr_rpc_callback_type    type,
                 struct tr_torrent     * tor UNUSED,
                 void                  * user_data UNUSED)
{
    if (type == TR_RPC_SESSION_CLOSE)
        closing = true;
    return TR_RPC_OK;
}

static void
onFileAdded (tr_session * session, const char * dir, const char * file)
{
    char * filename = tr_buildPath (dir, file, NULL);
    tr_ctor * ctor = tr_ctorNew (session);
    int err = tr_ctorSetMetainfoFromFile (ctor, filename);

    if (!err)
    {
        tr_torrentNew (ctor, &err);

        if (err == TR_PARSE_ERR)
            tr_err ("Error parsing .torrent file \"%s\"", file);
        else
        {
            bool trash = false;
            int test = tr_ctorGetDeleteSource (ctor, &trash);

            tr_inf ("Parsing .torrent file successful \"%s\"", file);

            if (!test && trash)
            {
                tr_inf ("Deleting input .torrent file \"%s\"", file);
                if (remove (filename))
                    tr_err ("Error deleting .torrent file: %s", tr_strerror (errno));
            }
            else
            {
                char * new_filename = tr_strdup_printf ("%s.added", filename);
                rename (filename, new_filename);
                tr_free (new_filename);
            }
        }
    }

    tr_ctorFree (ctor);
    tr_free (filename);
}

static void
printMessage (FILE * logfile, int level, const char * name, const char * message, const char * file, int line)
{
    if (logfile != NULL)
    {
        char timestr[64];
        tr_getLogTimeStr (timestr, sizeof (timestr));
        if (name)
            fprintf (logfile, "[%s] %s %s (%s:%d)\n", timestr, name, message, file, line);
        else
            fprintf (logfile, "[%s] %s (%s:%d)\n", timestr, message, file, line);
    }
#ifdef HAVE_SYSLOG
    else /* daemon... write to syslog */
    {
        int priority;

        /* figure out the syslog priority */
        switch (level) {
            case TR_MSG_ERR: priority = LOG_ERR; break;
            case TR_MSG_DBG: priority = LOG_DEBUG; break;
            default: priority = LOG_INFO; break;
        }

        if (name)
            syslog (priority, "%s %s (%s:%d)", name, message, file, line);
        else
            syslog (priority, "%s (%s:%d)", message, file, line);
    }
#endif
}

static void
pumpLogMessages (FILE * logfile)
{
    const tr_msg_list * l;
    tr_msg_list * list = tr_getQueuedMessages ();

    for (l=list; l!=NULL; l=l->next)
        printMessage (logfile, l->level, l->name, l->message, l->file, l->line);

    if (logfile != NULL)
        fflush (logfile);

    tr_freeMessageList (list);
}

static const char*
getConfigDir ()
{
    const char * configDir = NULL;
    configDir = tr_getDefaultConfigDir (MY_NAME);
    return configDir;
}

static void
gotMetadataFromURL (tr_session       * session UNUSED,
                    bool               did_connect UNUSED,
                    bool               did_timeout UNUSED,
                    long               response_code,
                    const void       * response,
                    size_t             response_byte_count,
                    void             * user_data)
{
    struct add_torrent_idle_data * data = (struct add_torrent_idle_data *)user_data;

    if (response_code==200 || response_code==221) /* http or ftp success.. */
    {
        tr_ctorSetMetainfo (data->ctor, (const uint8_t *)response, response_byte_count);

        int err = 0;
        tr_torrent * tor = tr_torrentNew (data->ctor, &err);
        tr_ctorFree (data->ctor);

    }
    else
    {
        char result[1024];
        tr_snprintf (result, sizeof (result), "gotMetadataFromURL: http error %ld: %s",
                     response_code, tr_webGetResponseStr (response_code));
        //????? tr_idle_function_done (data->data, result);
    }

    tr_free (data);
}

/* DE MOMENTO CUELA PERO NO PUEDE SER ESTA KAKA DE ESTATICOS*/

TransSession::TransSession(string sessionConfigDir)
{
	this->sessionConfigDir = sessionConfigDir;
	this->watchdir = NULL;
	this->session = NULL;
}

TransSession::~TransSession()
{

}

string TransSession::getTorrentStatusString (TorrentStat torrentStat)
{
	tr_torrent_activity status = torrentStat.status;
	bool isFinished = torrentStat.isFinished;
	int64_t fromUs = torrentStat.peersGettingFromUs;
	int64_t toUs = torrentStat.peersSendingToUs;
	uint64_t leftUntilDone = torrentStat.leftUntilDone;

    switch (status)
    {
        case TR_STATUS_DOWNLOAD_WAIT:
        case TR_STATUS_SEED_WAIT:
        	return string("Queued");

        case TR_STATUS_STOPPED:
            if (isFinished)
                return string("Finished");
            else
            	return string("Stopped");
            break;

        case TR_STATUS_CHECK_WAIT:
        case TR_STATUS_CHECK: {
        	return string("Verifing");
        }

        case TR_STATUS_DOWNLOAD:
        case TR_STATUS_SEED: {
            if (fromUs && toUs)
            	return string("Up & Down");
            else if (toUs)
            	return string("Downloading");
            else if (fromUs) {
                if (leftUntilDone > 0)
                	return string("Uploading");
                else
                	return string("Seeding");
            } else {
            	return string("Idle");
            }
            break;
        }

        default:
        	return string("Unknown");
            break;
    }

    return string("Unknown");
}

string TransSession::start()
{
    bool boolVal;
	bool loaded;
	tr_benc settings;
	const char * configDir = NULL;

	//ATENCION*********************************
	logfile = stderr;
	//ATENCION*********************************

	/* load settings from defaults + config file */
	tr_bencInitDict (&settings, 0);
	tr_bencDictAddBool (&settings, TR_PREFS_KEY_RPC_ENABLED, true);
	if ( !this->sessionConfigDir.compare("") )
	{
		configDir = tNode::Utils::stringToCharPtr(this->sessionConfigDir);
	}
	else
		configDir = getConfigDir();

	loaded = tr_sessionLoadSettings (&settings, configDir, MY_NAME);
	if ( !loaded )
		return string("{}");

	/* dump settings */
    char * str = tr_bencToStr (&settings, TR_FMT_JSON, NULL);
    string response = string(str);	//Lo que devolverá el método
    tr_free (str);

    /* start the session */
    tr_formatter_mem_init (MEM_K, MEM_K_STR, MEM_M_STR, MEM_G_STR, MEM_T_STR);
    tr_formatter_size_init (DISK_K, DISK_K_STR, DISK_M_STR, DISK_G_STR, DISK_T_STR);
    tr_formatter_speed_init (SPEED_K, SPEED_K_STR, SPEED_M_STR, SPEED_G_STR, SPEED_T_STR);
    this->session = tr_sessionInit ("TransNode", configDir, true, &settings);
    tr_sessionSetRPCCallback (this->session, on_rpc_callback, NULL);
    tr_ninf (NULL, "Using settings from \"%s\"", configDir);
    tr_sessionSaveSettings (this->session, configDir, &settings);

    mySession = this->session;

    /* maybe add a watchdir */
    {
    	const char * dir;

        if (tr_bencDictFindBool (&settings, PREF_KEY_DIR_WATCH_ENABLED, &boolVal)
        	&& boolVal
            && tr_bencDictFindStr (&settings, PREF_KEY_DIR_WATCH, &dir)
            && dir
            && *dir)
        {
        		tr_inf ("Watching \"%s\" for new .torrent files", dir);
                this->watchdir = dtr_watchdir_new (mySession, dir, onFileAdded);
        }
	}

    /* load the torrents */
    {
        tr_torrent ** torrents;
        tr_ctor * ctor = tr_ctorNew (mySession);
        if (paused)
            tr_ctorSetPaused (ctor, TR_FORCE, true);
        torrents = tr_sessionLoadTorrents (mySession, ctor, NULL);
        tr_free (torrents);
        tr_ctorFree (ctor);
    }

    //Tarea para el idle
    pthread_create (&(this->ideleThtread), NULL, idleFunction, (void *)this);

    return response;
}

void TransSession::idle()
{
    while (!closing) {
        tr_wait_msec (1000); /* sleep one second */
        dtr_watchdir_update (this->watchdir);
        pumpLogMessages (logfile);
    }
}

void TransSession::stop()
{
/*
    printf ("Closing transmission session...");
    tr_sessionSaveSettings (mySession, configDir, &settings);
    dtr_watchdir_free (this->watchdir);
    tr_sessionClose (mySession);
    pumpLogMessages (logfile);

    // shutdown
   #if HAVE_SYSLOG
       if (!foreground)
       {
           syslog (LOG_INFO, "%s", "Closing session");
           closelog ();
       }
   #endif

       // cleanup
       if (pidfile_created)
           remove (pid_filename);
       tr_bencFree (&settings);
*/
}

int TransSession::addTorrent(string filePath)
{
    tr_ctor * ctor = tr_ctorNew (this->session);
    if ( tr_ctorSetMetainfoFromFile (ctor, filePath.c_str()) != 0 )
    	return TORRENT_NOT_EXISTS;

    int err = 0;
    tr_torrent * tor = tr_torrentNew (ctor, &err);
    tr_ctorFree (ctor);

    if (err == TR_PARSE_DUPLICATE)
    	return TORRENT_ALLREADY_EXISTS;
    if (err == TR_PARSE_ERR)
        return TORRENT_INVALID_FROMAT;

    return 0;
}

int TransSession::addMagnet(string filePath)
{
    tr_ctor * ctor = tr_ctorNew (this->session);
    if ( tr_ctorSetMetainfoFromMagnetLink (ctor, filePath.c_str()) != 0 )
    	return MAGNET_NOT_EXISTS;

    int err = 0;
    tr_torrent * tor = tr_torrentNew (ctor, &err);
    tr_ctorFree (ctor);

    if (err == TR_PARSE_DUPLICATE)
    	return TORRENT_ALLREADY_EXISTS;
    if (err == TR_PARSE_ERR)
        return TORRENT_INVALID_FROMAT;

    return 0;
}

int TransSession::addURL(string filePath)
{
	const char * cookies = NULL;
	tr_ctor    * ctor = tr_ctorNew(this->session);

    struct add_torrent_idle_data * d = tr_new0 (struct add_torrent_idle_data, 1);
    //d->data = idle_data;
    d->ctor = ctor;
    tr_webRun (session, filePath.c_str(), NULL, cookies, gotMetadataFromURL, d);

    return 0;
}

TorrentsStat *TransSession::listTorrents()
{
        tr_torrent * tor = NULL;
	    int torrentCount = session->torrentCount;
	    TorrentsStat *torrentsStat = new TorrentsStat;
	    TorrentStat *torrentStat = new TorrentStat[torrentCount];
	    int idx = 0;
        while ((tor = tr_torrentNext (this->session, tor)))
        {
        	const tr_stat *st = tr_torrentStat (tor);

        	torrentStat[idx].error = st->error;
        	strcpy(torrentStat[idx].errorString, st->errorString);
        	torrentStat[idx].eta = st->eta;
        	torrentStat[idx].id = st->id;
        	torrentStat[idx].isFinished = st->finished;
        	torrentStat[idx].leftUntilDone = st->leftUntilDone;
        	strcpy(torrentStat[idx].name,tor->info.name);
        	torrentStat[idx].peersGettingFromUs = st->peersGettingFromUs;
        	torrentStat[idx].peersSendingToUs = st->peersSendingToUs;
        	torrentStat[idx].rateDownload = st->pieceDownloadSpeed_KBps;
        	torrentStat[idx].rateUpload = st->pieceUploadSpeed_KBps;
        	torrentStat[idx].sizeWhenDone = st->sizeWhenDone;
        	torrentStat[idx].status = st->activity;
        	torrentStat[idx].uploadRatio = st->ratio;

        	idx++;
        }

        torrentsStat->totalTorrents = torrentCount;
        torrentsStat->torrentStatList = torrentStat;

        return torrentsStat;
}

int TransSession::stopTorrent(int torrentId)
{

    tr_torrent * tor = NULL;
    int torrentCount = session->torrentCount;

    while ((tor = tr_torrentNext (this->session, tor)))
    {
    	if ( (tor->isRunning || tr_torrentIsQueued (tor)) && tor->uniqueId == torrentId )
    	{
    		tor->isStopping = true;
    		break;
    		//notify (session, TR_RPC_TORRENT_STOPPED, tor);
    	}
    }

	return 0;
}

int TransSession::startTorrent(int torrentId)
{

    tr_torrent * tor = NULL;
    int torrentCount = session->torrentCount;

    while ((tor = tr_torrentNext (this->session, tor)))
    {
    	if ( !tor->isRunning && tor->uniqueId == torrentId )
    	{
    		tr_torrentStart (tor);
    		break;
    		//notify (session, TR_RPC_TORRENT_STOPPED, tor);
    	}
    }

	return 0;
}

int TransSession::removeTorrent(int torrentId, bool deleteLocalData)
{
    tr_torrent * tor = NULL;
    int torrentCount = session->torrentCount;
    tr_rpc_callback_type type = deleteLocalData ? TR_RPC_TORRENT_TRASHING
                      	 	 	 	 	 	    : TR_RPC_TORRENT_REMOVING;

    while ((tor = tr_torrentNext (this->session, tor)))
    {
    	if ( tor->uniqueId == torrentId )
    	{
    		tr_torrentRemove (tor, deleteLocalData, NULL);
    		break;
    		//notify (session, TR_RPC_TORRENT_STOPPED, tor);
    	}
    }

	return 0;
}

void *idleFunction(void *ptrSession)
{
	((TransSession *)ptrSession)->idle();
	((TransSession *)ptrSession)->stop();
}
