#!/usr/bin/env python


# This is a runtime patcher
# Patch ykdl at runtime to make it provide enouth message for MoonPlayer
# Add support for danmaku and cookies

from __future__ import print_function
import os, sys, json, platform, io
from os.path import expanduser


# Init environment and import module
if platform.system() == 'Darwin':
    _srcdir = '%s/Library/Application Support/MoonPlayer/ykdl/' % os.getenv('HOME')
elif platform.system() == 'Linux' or platform.system() == 'FreeBSD':
    _srcdir = '%s/moonplayer/ykdl/' % os.getenv('XDG_DATA_HOME', os.getenv('HOME') + '/.local/share')
else:
    _srcdir = expanduser(r'~\AppData\Local\MoonPlayer\ykdl')
_filepath = os.path.dirname(sys.argv[0])
sys.path.insert(0, _srcdir)


# Patch bilibase
danmaku_url = ''
from ykdl.extractors.bilibili.bilibase import BiliBase
old_bilibase_prepare = BiliBase.prepare
def bilibase_prepare(self):
    retVal = old_bilibase_prepare(self)
    global danmaku_url
    danmaku_url = 'http://comment.bilibili.com/{}.xml'.format(self.vid)
    return retVal
BiliBase.prepare = bilibase_prepare


# Patch jsonlize
from ykdl.videoinfo import VideoInfo
from ykdl.util.html import fake_headers
old_jsonlize = VideoInfo.jsonlize
def jsonlize(self):
    retVal = old_jsonlize(self)
    retVal['danmaku_url'] = danmaku_url
    if retVal['extra']['ua'] == '':
        retVal['extra']['ua'] = fake_headers['User-Agent']
    return retVal
VideoInfo.jsonlize = jsonlize


# Run ykdl
from ykdl.common import url_to_module
from ykdl.compact import ProxyHandler, build_opener, install_opener
from ykdl.util.html import fake_headers, get_content
from ykdl.util.match import match1
from argparse import ArgumentParser
import socket
try:
    from http.cookiejar import MozillaCookieJar
    from urllib.request import HTTPCookieProcessor
except:
    from cookielib import MozillaCookieJar
    from urllib2 import HTTPCookieProcessor

def arg_parser():
    parser = ArgumentParser(description="Ykdl for MoonPlayer")
    parser.add_argument('--http-proxy', type=str, help="set proxy HOST:PORT for http(s) transfer. default: no proxy")
    parser.add_argument('--socks-proxy', type=str, help="set socks proxy HOST:PORT. default: no proxy")
    parser.add_argument('-t', '--timeout', type=int, default=60, help="set socket timeout seconds, default 60s")
    parser.add_argument('-u', '--user-agent', type=str, help="Custom User-Agent")
    parser.add_argument('video_url', type=str, help="video url")
    return parser.parse_args()

def main():
    args = arg_parser()
    handlers = []

    if args.timeout:
        socket.setdefaulttimeout(args.timeout)

    if args.user_agent:
        fake_headers['User-Agent'] = args.user_agent

    if args.http_proxy:
        proxy_handler = ProxyHandler({
            'http': args.http_proxy,
            'https': args.http_proxy
        })
        handlers.append(proxy_handler)

    elif args.socks_proxy:
        try:
            import socks
            addr, port = args.socks_proxy.split(':')
            socks.set_default_proxy(socks.SOCKS5, addr, int(port))
            socket.socket = socks.socksocket
        except:
            print('Failed to set socks5 proxy. Please install PySocks.', file=sys.stderr)

    opener = build_opener(*handlers)
    install_opener(opener)

    m, u = url_to_module(args.video_url)
    info = m.parser(u)

    # Is a playlist?
    if m.list_only():
        video_list = m.prepare_list()
        result = [ {'title': match1(get_content(url), r'<title>(.+?)</title>'), 'url': url} for url in video_list ]
    else:
        result = info.jsonlize()
    print(json.dumps(result, indent=4, sort_keys=True, ensure_ascii=False))

if __name__ == '__main__':
    main()
