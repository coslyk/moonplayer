#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links
from res_soku_tv import search_cb, explore_cb, load_item

res_name = '电影 - 全网'

tags_table = {'全部': 0,    '喜剧': 2001, '爱情': 2003, '恐怖': 2004,
              '动作': 2002, '科幻': 2007, '战争': 2008, '警匪': 2012,
              '犯罪': 2015, '动画': 2009, '奇幻': 2013, '其他': 2111}
tags = ['全部', '喜剧', '爱情', '恐怖', '动作', '科幻', '战争', '警匪',
        '犯罪', '动画', '奇幻', '其他']

countries_table = {'全部': 0,    '香港': 2004, '美国': 2002, '大陆': 2001,
                   '韩国': 2007, '台湾': 2006, '日本': 2008, '其他': 2111}
countries = ['全部', '香港', '美国', '大陆', '韩国', '台湾', '日本', '其他']

def explore(tag, country, page):
    tag_id = tags_table[tag]
    country_id = countries_table[country]
    url = 'http://www.soku.com/channel/movielist_0_%i_%i_1_%i.html' % \
           (tag_id, country_id, page)
    moonplayer.get_url(url, explore_cb, None)

def search(key, page):
    url = 'http://www.soku.com/v?keyword=' + key
    moonplayer.get_url(url, search_cb, None)
