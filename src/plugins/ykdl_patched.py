#!/usr/bin/env python


# This is a runtime patcher
# Patch ykdl at runtime to make it provide enouth message for MoonPlayer


# Change default coding
import os, sys, json, platform, io
from os.path import expanduser

# Init environment and import module
if platform.system() == 'Darwin':
    _srcdir = '%s/Library/Application Support/MoonPlayer/ykdl/' % os.getenv('HOME')
elif platform.system() == 'Linux':
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
from cykdl.__main__ import main
if __name__ == '__main__':
    main()
