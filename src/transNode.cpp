/*
 * transNode.cpp
 *
 *  Created on: Dec 20, 2012
 *      Author: fernando
 */

#include <v8.h>
#include <node.h>

using namespace node;
using namespace v8;

#include "transNode.h"
#include "PathsManager.h"
#include "FrameExtractor.h"
#include "TorrentCreator.h"
#include "TransSession.h"

//ESPECIFICO *********
struct Baton {
	int errorStatus;

	string orgFilePath;
	string uframeDirectory;
	string torrentDirectory;
	string uframeFilePath;
	string torrentFilePath;
	string uframeHash;
	int pieceSize;

	string sessionConfigDir;
	string sessionResponse;

	string nodeTorrentLocation;
	int nodeTorrentLinkType;

	TorrentsStat *torrentsStat;
	bool deleteLocalData;

	int torrentId;

	Persistent<Function> callback;

	void (*afterCallBack)(Baton *);
};
//ESPECIFICO *********

static bool isSuscribed = false;
static pthread_t status_change_thread;
static uv_async_t status_change_notifier;
static uv_loop_t *loop;
queue<Baton *> queue_msg = std::queue<Baton *>();
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t suscribe_mutex = PTHREAD_MUTEX_INITIALIZER;

static TransSession *transSession = NULL;

void after(uv_async_t *handle, int status)
{
	pthread_mutex_lock(&queue_mutex);
	Baton *baton = queue_msg.front();
	queue_msg.pop();
	pthread_mutex_unlock(&queue_mutex);

	baton->afterCallBack(baton);
}

void *mp3ToTorrentImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********

	PathsManager *pathsManager = new PathsManager(baton->orgFilePath);
	pathsManager->setWorkingDirectory("/tmp");
	pathsManager->setPieceSize(baton->pieceSize);

	if ( baton->uframeDirectory.compare("") != 0 )
		pathsManager->setUframeDirectory(baton->uframeDirectory);

	if ( baton->torrentDirectory.compare("") != 0 )
		pathsManager->setTorrentDirectory(baton->torrentDirectory);

	FrameExtractor *frameExtractor = new FrameExtractor(pathsManager->getOrgFilePath(), pathsManager->getTemporalFile());
	int result = frameExtractor->execute();
	delete frameExtractor;

	if ( result == NO_ERROR )
	{
		TorrentCreator *torrentCreator = new TorrentCreator(pathsManager->getUframeFilePath(),
															pathsManager->getTorrentFilePath(),
															pathsManager->getPieceSize());
		result = torrentCreator->execute();
		delete torrentCreator;
	}

	baton->uframeHash = pathsManager->getFileHash();
	baton->uframeFilePath = pathsManager->getUframeFilePath();
	baton->torrentFilePath = pathsManager->getTorrentFilePath();

	delete pathsManager;

//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterMp3ToTorrentImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));
	responseInfo->Set(String::NewSymbol("orgFilePath"), String::New(baton->orgFilePath.c_str()));
	responseInfo->Set(String::NewSymbol("uframeHash"), String::New(baton->uframeHash.c_str()));
	responseInfo->Set(String::NewSymbol("uframeFilePath"), String::New(baton->uframeFilePath.c_str()));
	responseInfo->Set(String::NewSymbol("torrentFilePath"), String::New(baton->torrentFilePath.c_str()));
	responseInfo->Set(String::NewSymbol("pieceSize"), Integer::New(baton->pieceSize));

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[argc] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> mp3ToTorrent(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() < 3) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->errorStatus = NO_ERROR;
	String::Utf8Value nodeOrgFilePath(args[0]->ToDetailString());
	baton->orgFilePath = string(*nodeOrgFilePath);
	baton->uframeDirectory = string("");
	baton->torrentDirectory = string("");
	baton->uframeFilePath = string("");
	baton->torrentFilePath = string("");
	baton->uframeHash = string("");
	baton->pieceSize = 0;

	baton->afterCallBack = &afterMp3ToTorrentImpl;

	//ARRAY

	if ( !args[1]->IsObject() ){
	    ThrowException(Exception::TypeError(String::New("Second argument must be an object")));
	    return scope.Close(Undefined());
	}

	Handle<Object> object = Handle<Object>::Cast(args[1]);

	Handle<Value> objUframeDirectory = object->Get(String::New("uframeDirectory"));
	if ( !objUframeDirectory->IsUndefined() )
	{
		String::Utf8Value nodeUframeDirectory(objUframeDirectory->ToDetailString());
		baton->uframeDirectory = string(*nodeUframeDirectory);
	}

	Handle<Value> objTorrentDirectory = object->Get(String::New("torrentDirectory"));
	if ( !objTorrentDirectory->IsUndefined() )
	{
		String::Utf8Value nodeTorrentDirectory(objTorrentDirectory->ToDetailString());
		baton->torrentDirectory = string(*nodeTorrentDirectory);
	}

	Handle<Value> objPieceSize = object->Get(String::New("pieceSize"));
	if ( !objTorrentDirectory->IsUndefined() )
	{
		Local<Integer> nodePieceSize = objPieceSize->ToInteger();
		baton->pieceSize = nodePieceSize->Value();
	}

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[2]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, mp3ToTorrentImpl, (void *)baton);

	return scope.Close(True());
}

void *sessionImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********
	if ( transSession != NULL )
	{
		baton->errorStatus = SESSION_ALLREADY_CREATED;
	}
	else
	{
		transSession = new TransSession(baton->sessionConfigDir);
		baton->sessionResponse = transSession->start();
	}
//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterSessionImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	Local<Object> configurationInfo = Object::New();

	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));
	if ( baton->errorStatus == NO_ERROR )
	{

		map<string,string> mapResult = tNode::Utils::simpleJsonToMap(baton->sessionResponse);

		for ( map<string, string>::iterator p = mapResult.begin(); p != mapResult.end(); p++ )
		{
			if ( !p->second.substr(0,1).compare("\"") )	//Es un string
				configurationInfo->Set(String::NewSymbol(p->first.c_str()), String::New(tNode::Utils::RemoveQuotes(p->second).c_str()));
			else if ( !p->second.compare("true") )	//Booleano a true
				configurationInfo->Set(String::NewSymbol(p->first.c_str()), Boolean::New(true));
			else if ( !p->second.compare("false") )	//Booleano a false
				configurationInfo->Set(String::NewSymbol(p->first.c_str()), Boolean::New(false));
			else	//Es un número
				configurationInfo->Set(String::NewSymbol(p->first.c_str()), Integer::New(atoi(p->second.c_str())));

			responseInfo->Set(String::NewSymbol("configuration"), configurationInfo);
		}
	}

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> session(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() < 1 || args.Length() > 2) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->afterCallBack = &afterSessionImpl;
	baton->sessionConfigDir = string("");
	if (args.Length() == 2 )
	{
		String::Utf8Value nodeSessionConfigDir(args[1]->ToDetailString());
		baton->sessionConfigDir = string(*nodeSessionConfigDir);
	}

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, sessionImpl, (void *)baton);

	return scope.Close(True());
}

void *addImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->nodeTorrentLinkType = UNDEFINED_TORRENT_REFERNCE;
	//MAGNET
	if ( baton->nodeTorrentLocation.size()>8 && !baton->nodeTorrentLocation.compare(0,8,"magnet:?") )
	{
		transSession->addMagnet(baton->nodeTorrentLocation);
		baton->nodeTorrentLinkType = MAGNET_TORRENT_REFERNCE;
	}
	//URL
	else if ( tNode::Utils::isCurlURL(baton->nodeTorrentLocation.c_str()) )
	{
		transSession->addURL(baton->nodeTorrentLocation);
		baton->nodeTorrentLinkType = LINK_TORRENT_REFERNCE;
	}
	//TORRENT
	else if ( baton->nodeTorrentLocation.size()>8 && !baton->nodeTorrentLocation.compare(baton->nodeTorrentLocation.size()-8,8,".torrent") )
	{
		baton->nodeTorrentLinkType = FILE_TORRENT_REFERNCE;
		baton->errorStatus = transSession->addTorrent(baton->nodeTorrentLocation);
	}
	//MP3
	else if ( baton->nodeTorrentLocation.size()>4 && !baton->nodeTorrentLocation.compare(baton->nodeTorrentLocation.size()-4,4,".mp3") )
	{
		PathsManager *pathsManager = new PathsManager(baton->nodeTorrentLocation);
		pathsManager->setWorkingDirectory("/tmp");

		FrameExtractor *frameExtractor = new FrameExtractor(pathsManager->getOrgFilePath(), pathsManager->getTemporalFile());
		int result = frameExtractor->execute();
		delete frameExtractor;

		if ( result == NO_ERROR )
		{
			TorrentCreator *torrentCreator = new TorrentCreator(pathsManager->getUframeFilePath(),
																pathsManager->getTorrentFilePath(),
																pathsManager->getPieceSize());
			result = torrentCreator->execute();
			delete torrentCreator;
		}
		delete pathsManager;

		//LO QUE TENGA QUE HACER PARA COMPARTIR EL FICHERO

		baton->nodeTorrentLinkType = MP3_TORRENT_REFERNCE;
	}
	else
	{
		baton->errorStatus = UNSOPORTED_TORRENT_ADD;
	}
//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterAddImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));
	responseInfo->Set(String::NewSymbol("linkType"), Integer::New(baton->nodeTorrentLinkType));

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> add(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 2) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	String::Utf8Value nodeTorrentLocation(args[0]->ToDetailString());
	baton->nodeTorrentLocation = string(*nodeTorrentLocation);

	baton->afterCallBack = &afterAddImpl;

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, addImpl, (void *)baton);

	return scope.Close(True());
}

void *listImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********

	baton->torrentsStat = transSession->listTorrents();

//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterListImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));

	TorrentsStat *torrentsStat = baton->torrentsStat;
	Handle<Array> arrayToSend = Array::New(torrentsStat->totalTorrents);

	for ( int i=0; i < torrentsStat->totalTorrents; i++)
	{
		Local<Object> torrentInfo = Object::New();
		torrentInfo->Set(String::NewSymbol("id"), Integer::New(torrentsStat->torrentStatList[i].id));

		if ( torrentsStat->torrentStatList[i].error )
			torrentInfo->Set(String::NewSymbol("errorMark"), String::New("*"));
		else
			torrentInfo->Set(String::NewSymbol("errorMark"), String::New(" "));

        if ( torrentsStat->torrentStatList[i].sizeWhenDone )
        {
        	float done =  (torrentsStat->torrentStatList[i].sizeWhenDone - torrentsStat->torrentStatList[i].leftUntilDone) / torrentsStat->torrentStatList[i].sizeWhenDone;
        	torrentInfo->Set(String::NewSymbol("done"), String::New(tNode::Utils::folatToPercent(done).c_str()));
        }
        else
        	torrentInfo->Set(String::NewSymbol("done"), String::New("n/a"));

        torrentInfo->Set(String::NewSymbol("have"), Integer::New(torrentsStat->torrentStatList[i].sizeWhenDone - torrentsStat->torrentStatList[i].leftUntilDone));

        if ( torrentsStat->torrentStatList[i].leftUntilDone || torrentsStat->torrentStatList[i].eta != -1)
        	torrentInfo->Set(String::NewSymbol("eta"), String::New(tNode::Utils::intToString(torrentsStat->torrentStatList[i].eta).c_str()));
        else
        	torrentInfo->Set(String::NewSymbol("eta"), String::New("Done"));

        torrentInfo->Set(String::NewSymbol("up"), Number::New(torrentsStat->torrentStatList[i].rateUpload));
        torrentInfo->Set(String::NewSymbol("down"), Number::New(torrentsStat->torrentStatList[i].rateDownload));
        torrentInfo->Set(String::NewSymbol("ratio"), Number::New(torrentsStat->torrentStatList[i].uploadRatio));
        torrentInfo->Set(String::NewSymbol("status"), String::New(transSession->getTorrentStatusString(torrentsStat->torrentStatList[i]).c_str()));
        torrentInfo->Set(String::NewSymbol("name"), String::New(torrentsStat->torrentStatList[i].name));

		arrayToSend->Set(i,torrentInfo);
	}

	delete torrentsStat->torrentStatList;
	delete [] torrentsStat;

	responseInfo->Set(String::NewSymbol("torrents"), arrayToSend);

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> list(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 1) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->errorStatus = 0;

	baton->afterCallBack = &afterListImpl;

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[0]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, listImpl, (void *)baton);

	return scope.Close(True());
}

void *startImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********
	baton->errorStatus = transSession->startTorrent(baton->torrentId);
//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterStartImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> start(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 2) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->errorStatus = 0;

	baton->afterCallBack = &afterStartImpl;
	Local<Integer> nodeTorrentId = args[0]->ToInteger();
	baton->torrentId = nodeTorrentId->Value();

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, startImpl, (void *)baton);

	return scope.Close(True());
}

void *stopImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********
	baton->errorStatus = transSession->stopTorrent(baton->torrentId);
//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterStopImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> stop(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 2) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->errorStatus = 0;

	baton->afterCallBack = &afterStopImpl;
	Local<Integer> nodeTorrentId = args[0]->ToInteger();
	baton->torrentId = nodeTorrentId->Value();

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, stopImpl, (void *)baton);

	return scope.Close(True());
}

void *removeImpl(void *voidBaton)
{

	Baton *baton = (Baton *)voidBaton;

//ESPECIFICO *********
	baton->errorStatus = transSession->removeTorrent(baton->torrentId, baton->deleteLocalData);
//ESPECIFICO *********

	pthread_mutex_lock(&queue_mutex);
	queue_msg.push(baton);
	pthread_mutex_unlock(&queue_mutex);

	// wake up callback
	uv_async_send(EV_DEFAULT_UC_ &status_change_notifier);

	return NULL;
}

void afterRemoveImpl(Baton *baton)
{

//ESPECIFICO *********
	Local<Object> responseInfo = Object::New();
	responseInfo->Set(String::NewSymbol("errorStatus"), Integer::New(baton->errorStatus));

	const unsigned argc = 1;
	Local<Value> argv[argc];
	argv[0] = responseInfo;
//ESPECIFICO *********

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

	baton->callback.Dispose();
	delete baton;
}

static Handle<Value> remove(const Arguments& args)
{
	HandleScope scope;

	if (args.Length() != 3) {
	    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
	    return scope.Close(Undefined());
	}

	pthread_mutex_lock(&suscribe_mutex);
	if ( !isSuscribed )
	{
		loop = uv_default_loop();
		uv_async_init(loop, &status_change_notifier, after);
		isSuscribed = false;
	}
	pthread_mutex_unlock(&suscribe_mutex);

	Baton *baton = new Baton();
	baton->errorStatus = NO_ERROR;

//ESPECIFICO *********
	baton->errorStatus = 0;

	baton->afterCallBack = &afterRemoveImpl;
	Local<Integer> nodeTorrentId = args[0]->ToInteger();
	baton->torrentId = nodeTorrentId->Value();
	Local<Boolean> nodeDeleteLocalData = args[1]->ToBoolean();
	baton->deleteLocalData = nodeDeleteLocalData->Value();

	//CALLBACK
	baton->callback = Persistent<Function>::New(Local<Function>::Cast(args[2]));
//ESPECIFICO *********

	pthread_create(&status_change_thread, NULL, removeImpl, (void *)baton);

	return scope.Close(True());
}

extern "C" void init(Handle<Object> target) {

	target->Set(String::NewSymbol("mp3ToTorrent"), FunctionTemplate::New(mp3ToTorrent)->GetFunction());
	target->Set(String::NewSymbol("open"), FunctionTemplate::New(session)->GetFunction());
	target->Set(String::NewSymbol("add"), FunctionTemplate::New(add)->GetFunction());
	target->Set(String::NewSymbol("list"), FunctionTemplate::New(list)->GetFunction());
	target->Set(String::NewSymbol("start"), FunctionTemplate::New(start)->GetFunction());
	target->Set(String::NewSymbol("stop"), FunctionTemplate::New(stop)->GetFunction());
	target->Set(String::NewSymbol("remove"), FunctionTemplate::New(remove)->GetFunction());
}

NODE_MODULE(mpd, init)



