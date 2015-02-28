#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links

tags = ['全部', '古装', '武侠', '警匪', '军事', '神话', '科幻',
        '悬疑', '历史', '儿童', '农村', '都市', '家庭', '搞笑',
        '偶像', '言情', '时装', '优酷出品']

countries = ['全部', '大陆', '香港', '台湾', '韩国', '美国',
             '英国', '泰国', '新加坡']

def search(args):
    if 'key' in args:
        moonplayer.warn('Not supported.')
        return
    tag = args['tag']
    country = args['country']
    if tag == '全部':
        tag = ''
    if country == '全部':
        country = ''
    url = 'http://www.youku.com/v_olist/c_97_g_%s_a_%s_s_1_d_2_pt_1_p_%i.html' % \
           (tag, country, args['page'])
    moonplayer.get_url(url, search_cb, None)

pic_re = re.compile(r'<img src="(.+?)" alt="(.+)">')
def search_cb(page, data):
    name2pic = {}
    result = []
    # Read all pic urls
    match = pic_re.search(page)
    while match:
        (url, name) = match.group(1, 2)
        name2pic[name] = url
        match = pic_re.search(page, match.end(0))
    # Read links, bind them with relative pic urls
    links = list_links(page, 'http://www.youku.com/show_page/')
    for i in xrange(0, len(links), 2):
        name = links[i]
        url = links[i+1]
        try:
            result.append({'name': name,
                           'flag': url,
                           'pic_url': name2pic[name]})
        except KeyError:
            pass
    moonplayer.res_show(result)
    
def load_item(url):
    moonplayer.get_url(url, load_item_cb, url)
    
img_re = re.compile(r'''<img src=['"](http://r\d+\.ykimg\.com/.+?)['"] alt=['"].+?['"]>''')
alt_re = re.compile(r'<label>别名:</label>\s*\n*\s*([^<]+?)\s*\n*\s*</li>')
date_re = re.compile(r'<label>上映:</label>(.+?)</span>')
name_re = re.compile(r'''<span class=['"]name['"]>(.+?)</span>''')
summ_re = re.compile(r'''<span .*?style=['"]display:none;?['"].*?>([^}]+?)</span>''')
rating_re = re.compile(r'''<em class=['"]num['"]>(.+?)</em>''')
def load_item_cb(page, url):
    result = {}
    match = name_re.search(page)
    if match:
        result['name'] = match.group(1)
    match = img_re.search(page)
    if match:
        result['image'] = match.group(1)
    match = date_re.search(page)
    if match:
        result['dates'] = [match.group(1)]
    match = alt_re.search(page)
    if match:
        result['alt_names'] = [match.group(1)]
    match = summ_re.search(page.split('Detail', 1)[-1])
    if match:
        result['summary'] = match.group(1)
    match = rating_re.search(page)
    if match:
        result['rating'] = float(match.group(1))
    result['source'] = list_links(page, 'http://v.youku.com/v_show/id_')
    new_url = url.replace('/show_page/', '/show_episode/')
    moonplayer.get_url(new_url, load_item_cb2, result)

def load_item_cb2(page, result):
    srcs = list_links(page, 'http://v.youku.com/v_show/id_')
    if len(srcs):
        result['source'] = srcs
    moonplayer.show_detail(result)
