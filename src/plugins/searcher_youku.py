#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
from moonplayer_utils import list_links

searcher_name = '优酷'

def search(keyword, page):
    keyword = keyword.replace(' ', '+')
    url = 'http://www.soku.com/search_video_ajax/q_%s_orderby_1_limitdate_0?site=14&page=%i' % (keyword, page)
    moonplayer.download_page(url, search_cb, None)
    
def search_cb(content, data):
    result = list_links(content, '//v.youku.com/v_show')
    result = ['http:' + item if item.startswith('//') else item for item in result]
    moonplayer.show_list(result)
