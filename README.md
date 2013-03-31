TransNode
=========

node.js C++ torrent module based on libstransmission (without RPC)

this library in process of change...
 * not yet compatible with node 0.10.x
 * examples do not yet exist (to show off its power)
 * I want to change over to (err, res) callback format

## install

  sudo apt-get install libcurl4-openssl-dev libevent-dev
  npm install
  node example/test.js

## API

### open
`open(<String> config_dir_path, <Function> callback)`

 * `config_dir_path` is a path to a directory on the local disk in which all of transmission's configuration will be saved (used for reloading sessions and the like)

```
callback(obj)
{
	errorStatus: <Integer>
}
```

### add
`add(<String> path, <Function> callback)`

 * `path` can be a string with a magnet url, a local file, or an http link

```
callback(obj)
{
	errorStatus: <Integer>
	linkType: <Integer>
}
```


### list
`list(<Function> callback)`


### start
`start(<Integer> TorrentId, <Function> callback)`
```
callback(obj)
{
	errorStatus: <Integer>
}
```

### stop
`stop(<Integer> TorrentId, <Function> callback)`
```
callback(obj)
{
	errorStatus: <Integer>
}
```

### remove
`remove(<Integer> TorrentId, <Boolean> delete_local_data, <Function> callback)`
```
callback(obj)
{
	errorStatus: <Integer>
}
```


### mp3ToTorrent
`mp3ToTorrent(<String> mp3_path, <Object> options, <Function> callback)`

this function takes an MP3 file as an argument, with options:
```
options = {
	uframeDirectory: <String>
	torrentDirectory: <String>
	pieceSize: <Integer>
}
```

```
callback(obj)
{
	errorStatus: <Integer>
	orgFilePath: <String>
	uframeHash: <String>
	uframeFilePath: <String>
	torrentFilePath: <String>
	pieceSize: <Integer>
}
```

## License

(The MIT License)

Copyright (c) 2013 Fernando Martín &lt;fernancoder@gmail.com&gt;

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
