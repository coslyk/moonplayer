#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
from moonplayer_utils import list_links

searcher_name = '全网搜索'

def search(keyword, page):
    url = 'http://www.soku.com/v?keyword=%s&type=0&ext=2&noqc=&curpage=%i' % (keyword, page)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    result = list_links(content, 'http://www.soku.com/u?url=')
    for i in xrange(1, len(result), 2):
        result[i] = result[i].replace('http://www.soku.com/u?url=', '')
    moonplayer.show_list(result)
