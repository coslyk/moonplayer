#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import re
import moonplayer

tags = ['热门', '最新', '经典', '可播放', '豆瓣高分','冷门佳片', 
        '华语', '欧美', '韩国', '日本', '动作', '冒险', '古装',
        '喜剧', '爱情', '科幻', '悬疑', '恐怖', '动画', '奇幻',
        '儿童', '犯罪', '剧情', '家庭']
        
countries = ['全部']

# Search movies
def search(args):
    start = (args['page'] - 1) * 20
    if 'key' in args:
        key = args['key']
        url = 'http://api.douban.com/v2/movie/search?q=%s&start=%i&count=20' % (key, start)
    else:
        tag = args['tag']
        url = 'http://api.douban.com/v2/movie/search?tag=%s&start=%i&count=20' % (tag, start)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    page = json.loads(content)
    result = []
    for item in page[u'subjects']:
        title = item['title'].encode('utf-8')
        pic_url = item['images']['medium'].encode('utf-8')
        mv_id = item['id'].encode('utf-8')
        result.append({
            'name': title,
            'pic_url': pic_url,
            'flag': mv_id})
    moonplayer.res_show(result)
    
## Load movies' detail
def load_item(mv_id):
    url = 'http://api.douban.com/v2/movie/subject/' + mv_id
    moonplayer.get_url(url, load_item_cb, None)
    
def load_item_cb(page, data):
    data = json.loads(page)
    director = [item[u'name'] for item in data[u'directors']]
    player = [item[u'name'] for item in data[u'casts']]
    
    result = {'name': data[u'title'],
              'flag': data[u'id'],
              'alternate_name': data[u'aka'],
              'image': data[u'images'][u'large'],
              'director': director,
              'player': player,
              'type': data[u'genres'],
              'nation': data[u'countries'],
              'rating': data[u'rating'][u'average'],
              'summary': data[u'summary']}
    url = 'http://movie.douban.com/subject/' + str(data[u'id'])
    moonplayer.get_url(url, load_item_cb2, result)
    
misc_re = re.compile(r'http://.+?/misc/mixed_static/\w+?\.js')
date_re = re.compile(r'<span property="v:initialReleaseDate" content="(.+?)">')
lang_re = re.compile(r'<span class="pl">语言:</span>\s*(.+?)<br/>')
length_re = re.compile(r'<span property="v:runtime" content="(.+?)">')
def load_item_cb2(page, result):
    dates = []
    match = date_re.search(page)
    while match:
        dates.append(match.group(1))
        match = date_re.search(page, match.end(0))
    if len(dates):
        result['date'] = dates
        
    match = lang_re.search(page)
    if match:
        result['language'] = [match.group(1)]
        
    match = length_re.search(page)
    if match:
        result['length'] = match.group(1) + ' min'
    
    match = misc_re.search(page)
    if match:
        url = match.group(0)
        moonplayer.get_url(url, load_item_cb3, result)
    else:
        moonplayer.show_detail(result)
        
msg_re = re.compile(r'videos = (.+?),\n')
def load_item_cb3(page, result):
    match = msg_re.search(page)
    srcs = []
    if match:
        msg = match.group(1)
        data = json.loads(msg)[u'data']
        for item in data:
            if not item[u'need_pay']:
                srcs.append('%s (%s)' % (item[u'title'], item[u'source'][u'name']))
                srcs.append(item[u'sample_link'])
        result['source'] = srcs
    moonplayer.show_detail(result)

