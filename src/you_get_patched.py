#!/usr/bin/env python3


# This is a runtime patcher
# Patch you-get at runtime to make it provide enouth message for MoonPlayer


# Init environment and import module
import os, sys, json, platform, io
if platform.system() == 'Darwin':
    _srcdir = '%s/Library/Application Support/MoonPlayer/you-get/src/' % os.getenv('HOME')
else:
    _srcdir = '%s/.moonplayer/you-get/src/' % os.getenv('HOME')
_filepath = os.path.dirname(sys.argv[0])
sys.path.insert(1, os.path.join(_filepath, _srcdir))
import you_get


# Fix: print unusable info under json mode
stdout_bak = sys.stdout
strio = io.StringIO()
sys.stdout = strio

# Patch json_output.output
def output(video_extractor, pretty_print=True):
    ve = video_extractor
    out = {}
    out['url'] = ve.url
    out['title'] = ve.title
    out['site'] = ve.name
    out['streams'] = ve.streams
    if getattr(ve, 'audiolang', None):
        out['audiolang'] = ve.audiolang
    if getattr(ve, 'ua', None):
        out['user-agent'] = ve.ua
    if getattr(ve, 'referer', None):
        out['referer'] = ve.referer
    if getattr(ve, 'danmuku', None) and ve.danmuku.startswith('http'):
        out['danmaku_url'] = ve.danmuku
    else:
        try:
            out['referer'] = ve.streams['__default__']['refer']
        except KeyError:
            pass

    sys.stdout = stdout_bak
    if pretty_print:
        print(json.dumps(out, indent=4, sort_keys=True, ensure_ascii=False))
    else:
        print(json.dumps(out))
    sys.stdout = strio
import you_get.json_output
you_get.json_output.output = output


# Patch bilibili
def get_danmuku_xml(cid):
    return 'http://comment.bilibili.com/{}.xml'.format(cid)
from you_get.extractors import bilibili
bilibili.get_danmuku_xml = get_danmuku_xml


# Run you-get
if __name__ == '__main__':
    you_get.main(repo_path=_filepath)
