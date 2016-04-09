#!/usr/bin/python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import parse_flvcd_page

hosts = ('www.acfun.tv',)

def parse(url, options):
    origin_url = url
    url = 'http://www.flvcd.com/parse.php?go=1&kw=' + origin_url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    moonplayer.get_url(url, parse_cb, (options, origin_url))
    
def parse_cb(page, data):
    options, url = data
    result = parse_flvcd_page(page, None)
    moonplayer.get_url(url, parse_danmaku_cb, (options, result, url))
    
        
cid_re = re.compile(r'''data-vid=['"](\d+)['"]''')
name_re = re.compile(r'''data-title=['"](.+?)['"]''')
def parse_danmaku_cb(page, data):
    options, result, url = data
    # Cannot get the video name from flvcd.com page correctly
    match = name_re.search(page)
    if match:
        name = match.group(1) + ".flv"
    else:
        name = result[0]
        
    match = cid_re.search(page)
    if match:
        danmaku = 'http://danmu.aixifan.com/V2/' + match.group(1)
        if len(result) == 0:
            moonplayer.use_fallback_parser(url, options & moonplayer.OPT_DOWNLOAD, danmaku)
        elif options & moonplayer.OPT_DOWNLOAD:
            if len(result) == 2:
                result[0] = name
                moonplayer.download_with_danmaku(result, danmaku)
            else:
                moonplayer.download(result, name)
        else:
            moonplayer.play(result, danmaku)
    else:
        moonplayer.warn('无法获取弹幕！')
        if len(result) == 0:
            moonplayer.use_fallback_parser(url, options & moonplayer.OPT_DOWNLOAD)
        if options & moonplayer.OPT_DOWNLOAD:
            if len(result) == 2:
                result[0] = name
                moonplayer.download(result)
            else:
                moonplayer.download(result, name)
        else:
            moonplayer.play(result)
