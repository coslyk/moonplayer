#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
from moonplayer_utils import parse_flvcd_inf
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree

hosts = ('v.qq.com',)

def search(keyword, page):
    keyword = keyword.replace(' ', '%20')
    url = 'http://s.video.qq.com/search?comment=0&plat=2&otype=xml&query=%s&cur=%d&num=20&start=0&end=0' % (keyword, page - 1)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    root = ET.fromstring(content)
    result = []
    for item in root.iter('list'):
        result.append(item.find('title').text)
        result.append(item.find('AW').text)
    moonplayer.show_list(result)
    
def parse(url, options):
    url = 'http://www.flvcd.com/parse.php?kw=' + url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    moonplayer.get_url(url, parse_cb, options)
    
## Parse videos
def parse_cb(page, options):
    result = parse_flvcd_inf(page)
    if options & moonplayer.OPT_DOWNLOAD:
        moonplayer.download(result, result[0])
    else:
        moonplayer.play(result)
