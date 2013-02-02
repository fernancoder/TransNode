{
  "targets": [
    {
      "target_name": "trans-node",
      "sources": [
        "src/transNode.cpp",
        "src/bulkID3/id3_tag_frame.cpp",
        "src/bulkID3/id3_tag.cpp",
        "src/bulkID3/id3_util.cpp",
        "src/bulkID3/linked_list.cpp",
        "src/bulkID3/mp3.cpp",
        "src/bulkID3/util.cpp",
        "src/bulkID3/xmalloc.cpp",
        "src/libtransmission/announcer.c",
        "src/libtransmission/announcer-http.c",
        "src/libtransmission/announcer-udp.c",
        "src/libtransmission/bandwidth.c",
        "src/libtransmission/bencode.c",
        "src/libtransmission/bitfield.c",
        "src/libtransmission/blocklist.c",
        "src/libtransmission/cache.c",
        "src/libtransmission/clients.c",
        "src/libtransmission/completion.c",
        "src/libtransmission/ConvertUTF.c",
        "src/libtransmission/crypto.c",
        "src/libtransmission/fdlimit.c",
        "src/libtransmission/handshake.c",
        "src/libtransmission/history.c",
        "src/libtransmission/inout.c",
        "src/libtransmission/json.c",
        "src/libtransmission/jsonsl.c",
        "src/libtransmission/list.c",
        "src/libtransmission/magnet.c",
        "src/libtransmission/makemeta.c",
        "src/libtransmission/metainfo.c",
        "src/libtransmission/natpmp.c",
        "src/libtransmission/net.c",
        "src/libtransmission/peer-io.c",
        "src/libtransmission/peer-mgr.c",
        "src/libtransmission/peer-msgs.c",
        "src/libtransmission/platform.c",
        "src/libtransmission/port-forwarding.c",
        "src/libtransmission/ptrarray.c",
        "src/libtransmission/resume.c",
        "src/libtransmission/rpcimpl.c",
        "src/libtransmission/rpc-server.c",
        "src/libtransmission/session.c",
        "src/libtransmission/stats.c",
        "src/libtransmission/torrent.c",
        "src/libtransmission/torrent-ctor.c",
        "src/libtransmission/torrent-magnet.c",
        "src/libtransmission/tr-dht.c",
        "src/libtransmission/trevent.c",
        "src/libtransmission/tr-lpd.c",
        "src/libtransmission/tr-udp.c",
        "src/libtransmission/tr-utp.c",
        "src/libtransmission/upnp.c",
        "src/libtransmission/utils.c",
        "src/libtransmission/verify.c",
        "src/libtransmission/web.c",
        "src/libtransmission/webseed.c",
        "src/libtransmission/wildmat.c",
        "src/transmissionDaemon/watch.cpp",
        "src/PathsManager.cpp",
        "src/FrameExtractor.cpp",
        "src/TorrentCreator.cpp",
        "src/TransSession.cpp",
        "src/Utils.cpp",
        "third-party/dht/dht.c",
        "third-party/libutp/utp.cpp",
        "third-party/libutp/utp_utils.cpp",
        "third-party/libnatpmp/natpmp.c",
        "third-party/libnatpmp/wingettimeofday.c",
        "third-party/libnatpmp/getgateway.c",
        "third-party/miniupnp/connecthostport.c",
        "third-party/miniupnp/igd_desc_parse.c",
        "third-party/miniupnp/minisoap.c",
        "third-party/miniupnp/minissdpc.c",
        "third-party/miniupnp/miniupnpc.c",
        "third-party/miniupnp/miniwget.c",
        "third-party/miniupnp/minixml.c",
        "third-party/miniupnp/portlistingparse.c",
        "third-party/miniupnp/receivedata.c",
        "third-party/miniupnp/upnpcommands.c",
        "third-party/miniupnp/upnpreplyparse.c"
      ],
      "defines": [
        "POSIX",
        "WITH_UTP"
      ],
      "link_settings": {
        "libraries": [
          "-lcrypto",
          "-lcurl",
          "-levent"
        ]
      },
      "include_dirs": [
        "/usr/local/include",
        "third-party/libnatpmp",
        "third-party/libutp",
        "third-party/libutp/utp_config_lib",
        "third-party"
      ]
    }
  ]
}
