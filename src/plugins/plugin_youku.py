#!/usr/bin/python
# -*- encoding: utf-8 -*-

from moonplayer_utils import list_links, parse_flvcd_page
import re
import json
import thread
import moonplayer

#hosts
hosts = ('v.youku.com',)

    
## Parse videos or albums
id_re = re.compile(r'http://v.youku.com/v_show/id_(.+)\.html.*')
def parse(url, options):
    #single video
    match = id_re.match(url)
    if not match:
        moonplayer.warn('Please input a valid youku url.')
        return
    url = 'http://www.flvcd.com/parse.php?go=1&kw=' + url
    if options & moonplayer.OPT_QL_SUPER:
        url += '&format=super'
    elif options & moonplayer.OPT_QL_HIGH:
        url += '&format=high'
    print url
    moonplayer.get_url(url, parse_cb, options)
    
## Parse videos
def parse_cb(page, options):
    result = parse_flvcd_page(page, None)
    if options & moonplayer.OPT_DOWNLOAD:
        moonplayer.download(result, result[0])
    else:
        moonplayer.play(result)

