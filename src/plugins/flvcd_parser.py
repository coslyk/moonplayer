#!/usr/bin/python
# -*- coding: utf-8 -*-

import moonplayer
from moonplayer_utils import list_links, parse_flvcd_page

def parse(url, options):
    origin_url = url
    url = 'http://www.flvcd.com/parse.php?go=1&kw=' + origin_url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    print url
    moonplayer.get_url(url, parse_cb, (options, origin_url))
    
## Parse videos
def parse_cb(page, data):
    options = data[0]
    url = data[1]
    result = parse_flvcd_page(page, None)
    if len(result) == 0:
        moonplayer.warn('Cannot parse this video:\n' + url)
    elif options & moonplayer.OPT_DOWNLOAD:
        moonplayer.download(result, result[0])
    else:
        moonplayer.play(result)
