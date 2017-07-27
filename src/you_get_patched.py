#!/usr/bin/env python3


# This is a runtime patcher
# Patch you-get at runtime to make it provide enouth message for MoonPlayer


# Init environment and import module
import os, sys, json, platform
if platform.system() == 'Darwin':
    _srcdir = '%s/Library/Application Support/MoonPlayer/you-get/src/' % os.getenv('HOME')
else:
    _srcdir = '%s/.moonplayer/you-get/src/' % os.getenv('HOME')
_filepath = os.path.dirname(sys.argv[0])
sys.path.insert(1, os.path.join(_filepath, _srcdir))
import you_get

# Patch json_output.output
def output(video_extractor, pretty_print=True):
    ve = video_extractor
    out = {}
    out['url'] = ve.url
    out['title'] = ve.title
    out['site'] = ve.name
    out['streams'] = ve.streams
    if getattr(ve, 'audiolang', None) is not None:
        out['audiolang'] = ve.audiolang
    if getattr(ve, 'ua', None) is not None:
        out['user-agent'] = ve.ua
    if getattr(ve, 'referer', None) is not None:
        out['referer'] = ve.referer
    else:
        try:
            out['referer'] = ve.streams['__default__']['refer']
        except KeyError:
            pass

    if pretty_print:
        print(json.dumps(out, indent=4, sort_keys=True, ensure_ascii=False))
    else:
        print(json.dumps(out))
import you_get.json_output
you_get.json_output.output = output


if __name__ == '__main__':
    you_get.main(repo_path=_filepath)
