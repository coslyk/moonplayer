#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links

res_name = '电视剧 - 全网'

tags_table = {'全部': 0,    '古装': 1007, '警匪': 1009, '搞笑': 1010,
              '悬疑': 1011, '神话': 1012, '偶像': 1001, '历史': 1005,
              '言情': 1002, '家庭': 1013, '科幻': 1014, '其他': 1111}
tags = ['全部', '古装', '警匪', '搞笑', '悬疑', '神话', '偶像', '历史',
        '言情', '家庭', '科幻', '其他']

countries_table = {'全部': 0,    '大陆': 1001, '韩国': 1003, '香港': 1006,
                   '泰国': 1007, '台湾': 1008, '美国': 1009, '其他': 1111}
countries = ['全部', '大陆', '韩国', '香港', '泰国', '台湾', '美国', '其他']

def search(args):
    if 'key' in args:
        url = 'http://www.soku.com/v?keyword=' + args['key']
        moonplayer.get_url(url, search_by_key_cb, None)
        return
    tag = args['tag']
    tag_id = tags_table[tag]
    country = args['country']
    country_id = countries_table[country]
    url = 'http://www.soku.com/channel/teleplaylist_0_%i_%i_1_%i.html' % \
           (tag_id, country_id, args['page'])
    moonplayer.get_url(url, search_cb, None)


pic_re = re.compile(r'<img\s[^>]*src="(http://g\d\.ykimg\.com.+?)"')
rest_re = re.compile(r'<a\s[^>]*href="(/detail/show/.+?)"[^>]+title="(.+?)".+?<img\s[^>]*src="(.+?)"')
def search_by_key_cb(content, data):
    result = []
    items = content.split('<div class="detail">')
    for item in items:
        item = item.split('</div><!--detail end-->')[0].replace('\n', '')
        name_url = list_links(item, '/detail/show/')
        if len(name_url):
            name = name_url[0]
            url = name_url[1]
            match = pic_re.search(item)
            if match:
                pic = match.group(1)
                result.append({'name': name, 'url': url, 'pic_url': pic})
    rest = items[-1].split('</div><!--item end-->', 1)[1].replace('\n', '')
    match = rest_re.search(rest)
    while match:
        (url, name, pic) = match.group(1, 2, 3)
        result.append({'name': name, 'url': url, 'pic_url': pic})
        match = rest_re.search(rest, match.end(0))
    moonplayer.res_show(result)

pic2_re = re.compile(r'<img original="(http://g\d.ykimg.com/.+?)"\s[^>]*alt="(.+?)"')
def search_cb(page, data):
    name2pic = {}
    result = []
    # Read all pic urls
    match = pic2_re.search(page)
    while match:
        (url, name) = match.group(1, 2)
        name2pic[name] = url
        match = pic2_re.search(page, match.end(0))
    # Read links, bind them with relative pic urls
    links = list_links(page, '/detail/show/')
    for i in xrange(0, len(links), 2):
        name = links[i]
        url = links[i+1]
        try:
            result.append({'name': name,
                           'url': url,
                           'pic_url': name2pic[name]})
        except KeyError:
            pass
    moonplayer.res_show(result)
    


def load_item(url):
    url = 'http://www.soku.com' + url
    moonplayer.get_url(url, load_item_cb, url)
    
alt_re = re.compile(r'<label>别名:</label><span>(.+?)</span>')
date_re = re.compile(r'<label>上映时间:</label><span>(.+?)</span>')
name_re = re.compile(r'<title>(.+?)_.+?</title>')
summ_re = re.compile(r'<label>剧情简介:</label>\s*(.+?)\s*<')
more_re = re.compile(r'''<span id=['"]show_all_more['"].*?>\s*(.+?)\s*<''')
rating_re = re.compile(r'''<em class=['"]num['"]>(.+?)</em>''')
url_re = re.compile(r'''<a\s[^>]*href=['"]([^>]+?)['"]\s[^>]*site=['"]([^>]+?)['"][^>]*>([^>]+?)</a>''')
url2_re = re.compile(r'''<div\s[^>]*class=['"]linkpanels.+?<a href=['"](.+?)['"]></a>''')
def load_item_cb(page, url):
    page = page.replace('\n', '')
    result = {}
    match = name_re.search(page)
    if match:
        result['name'] = match.group(1)
    match = pic_re.search(page)
    if match:
        result['image'] = match.group(1)
    match = date_re.search(page)
    if match:
        result['dates'] = [match.group(1)]
    match = alt_re.search(page)
    if match:
        result['alt_names'] = [match.group(1)]
        
    match = summ_re.search(page)
    if match:
        result['summary'] = match.group(1)
    match = more_re.search(page, match.end(0))
    if match:
        result['summary'] += match.group(1)
    
    match = rating_re.search(page)
    if match:
        result['rating'] = float(match.group(1))
        
    srcs = []
    match = url_re.search(page)
    while match:
        (url, site, name) = match.group(1, 2, 3)
        srcs.append('(%s) %s' % (site, name))
        srcs.append(url)
        match = url_re.search(page, match.end(0))
    if len(srcs) == 0: # Movies
        match = url2_re.search(page)
        while match:
            url = match.group(1)
            srcs.append('source: http://' + url.split('/')[2])
            srcs.append(url)
            match = url2_re.search(page, match.end(0))
    result['source'] = srcs
    moonplayer.show_detail(result)
