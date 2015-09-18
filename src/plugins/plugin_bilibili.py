#!/usr/bin/python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links, parse_flvcd_page

hosts = ('www.bilibili.com',)

def parse(url, options):
    origin_url = url
    url = 'http://www.flvcd.com/parse.php?go=1&kw=' + origin_url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    moonplayer.get_url(url, parse_cb, (options, origin_url))
    
def parse_cb(page, data):
    options = data[0]
    url = data[1]
    result = parse_flvcd_page(page, None)
    if len(result) == 0:
        moonplayer.warn('Cannot parse this video:\n' + url)
        
    elif options & moonplayer.OPT_DOWNLOAD:
        if len(result) == 2: # single clip
            moonplayer.download(result)
        else:
            moonplayer.download(result, result[0])
            
    else:
        moonplayer.get_url(url, parse_danmaku_cb, result)
        
        
cid_re = re.compile(r'cid=(\d+)')
def parse_danmaku_cb(page, result):
    match = cid_re.search(page)
    if match:
        danmaku = 'http://comment.bilibili.com/%s.xml' % match.group(1)
        moonplayer.play(result, danmaku)
    else:
        moonplayer.warn('无法获取弹幕！')
        moonplayer.play(result)
