#!/usr/bin/python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links, parse_flvcd_page

hosts = ('www.bilibili.com', 'bangumi.bilibili.com')

# Avoid speed limitation
moonplayer.bind_referer('ws.acgvideo.com', 'http://www.bilibili.com')

def parse(url, options):
    origin_url = url
    url = 'http://www.flvcd.com/parse.php?go=1&kw=' + origin_url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    moonplayer.download_page(url, parse_cb, (options, origin_url))
    
def parse_cb(page, data):
    (options, url) = data
    result = parse_flvcd_page(page, None)
    if 'play#' in url:
        post_data = 'episode_id=' + url.split('#')[-1]
        api_url = 'http://bangumi.bilibili.com/web_api/get_source'
        moonplayer.post_content(api_url, post_data, parse_danmaku_cb, (options, result, url))
    else:
        moonplayer.download_page(url, parse_danmaku_cb, (options, result, url))
        
        
cid_re = re.compile(r'cid=(\d+)')
cid_re2 = re.compile(r'"cid":(\d+)')
def parse_danmaku_cb(page, data):
    options, result, url = data
    match = cid_re.search(page)
    if match == None:
        match = cid_re2.search(page)
    if match:
        danmaku = 'http://comment.bilibili.com/%s.xml' % match.group(1)
        if len(result) == 0:
            moonplayer.use_fallback_parser(url, options & moonplayer.OPT_DOWNLOAD, danmaku)
        elif options & moonplayer.OPT_DOWNLOAD:
            if len(result) == 2:
                moonplayer.download_with_danmaku(result, danmaku)
            else:
                moonplayer.download_with_danmaku(result, danmaku, result[0])
        else:
            moonplayer.play(result, danmaku)
    else:
        moonplayer.warn('无法获取弹幕！')
        if len(result) == 0:
            moonplayer.use_fallback_parser(url, options & moonplayer.OPT_DOWNLOAD)
        elif options & moonplayer.OPT_DOWNLOAD:
            if len(result) == 2:
                moonplayer.download(result)
            else:
                moonplayer.download(result, result[0])
        else:
            moonplayer.play(result)
