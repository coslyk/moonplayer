#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
import json

hosts = ('list.youku.com',)

def parse(url, options):
    moonplayer.download_page(url, parse_cb, None)
    
yk_item_re = re.compile(r'''<a [^>]*?href=['"](//v\.youku\.com.+?)['"][^>]*?>([^<]+?)</a>''')
def parse_cb(page, data):
    srcs = []
    page = page.split('(', 1)[-1][0:-2] # Remove callback function
    page = json.loads(page)['html']
    match = yk_item_re.search(page)
    while match:
        srcs.append(match.group(2))
        srcs.append('http:' + match.group(1))
        match = yk_item_re.search(page, match.end(0))
    
    result = {'source': srcs}
    moonplayer.show_detail(result)
