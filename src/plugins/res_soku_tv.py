#!/usr/bin/env python
# -*- coding: utf-8 -*-

from moonplayer_utils import list_links
from HTMLParser import HTMLParser
import moonplayer
import re
import json

res_name = '电视剧 - 全网'

tags_table = {'全部': 0,    '古装': 1007, '警匪': 1009, '搞笑': 1010,
              '悬疑': 1011, '神话': 1012, '偶像': 1001, '历史': 1005,
              '言情': 1002, '家庭': 1013, '科幻': 1014, '其他': 1111}
tags = ['全部', '古装', '警匪', '搞笑', '悬疑', '神话', '偶像', '历史',
        '言情', '家庭', '科幻', '其他']

countries_table = {'全部': 0,    '大陆': 1001, '韩国': 1003, '香港': 1006,
                   '泰国': 1007, '台湾': 1008, '美国': 1009, '其他': 1111}
countries = ['全部', '大陆', '韩国', '香港', '泰国', '台湾', '美国', '其他']

### Search ### 
def search(key, page):
    url = 'http://www.soku.com/v?keyword=' + key
    moonplayer.download_page(url, search_cb, None)

def search_cb(content, data):
    parser = SearchResultParser()
    parser.feed(content.decode('UTF-8'))
    moonplayer.res_show(parser.result)
    

class SearchResultParser(HTMLParser):
    def __init__(self):
        HTMLParser.__init__(self)
        self.s_poster = False
        self.result = []
        self.div_etage = 0
        
    def handle_starttag(self, tag, attrs):
        attrs = {k:v for (k, v) in attrs}
        # Enter an item
        if tag == 'div' and 'class' in attrs and attrs['class'] == 's_poster':
            self.s_poster = True
            self.result.append({'name': '', 'url': '', 'pic_url': ''})
            self.div_etage += 1
            
        elif self.s_poster and tag == 'div':
            self.div_etage += 1
            
        # Link
        elif self.s_poster and tag == 'a' and self.result[-1]['url'] == '':
            url = attrs['href']
            if 'youku.com' in url or url.startswith('/detail/show/'):
                self.result[-1]['url'] = attrs['href']
                if '_log_title' in attrs:
                    self.result[-1]['name'] = attrs['_log_title']
                else:
                    self.result[-1]['name'] = attrs['title']
                    
        # Preview image
        elif self.s_poster and tag == 'img' and 'ykimg.com' in attrs['src']:
            if self.result[-1]['pic_url'] == '':
                self.result[-1]['pic_url'] = attrs['src']
                
        # Replace original url with detail page's
        elif tag == 'a' and attrs['href'].startswith('http://www.youku.com/show_page/'):
            if '_log_title' in attrs:
                name = attrs['_log_title']
            else:
                name = attrs['title']
            if name == self.result[-1]['name']:
                self.result[-1]['url'] = attrs['href']
            
    def handle_endtag(self, tag):
        if self.s_poster and tag == 'div':
            self.div_etage -= 1
            if self.div_etage == 0:
                self.s_poster = False
                if self.result[-1]['pic_url'] == '':
                    del self.result[-1]
            

### Explore ###
def explore(tag, country, page):
    tag_id = tags_table[tag]
    country_id = countries_table[country]
    url = 'http://www.soku.com/channel/teleplaylist_0_%i_%i_1_%i.html' % \
           (tag_id, country_id, page)
    moonplayer.download_page(url, explore_cb, None)
    
    
pic_re = re.compile(r'<img src="(.+?)" alt="(.+)">')
pic2_re = re.compile(r'<img original="(http://g\d.ykimg.com/.+?)"\s[^>]*alt="(.+?)"')
def explore_cb(page, data):
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
    

### Load item ###
def load_item(url):
    if url.startswith('http://'):
        moonplayer.download_page(url, load_youku_item_cb, None)
    else:
        url = 'http://www.soku.com' + url
        moonplayer.download_page(url, load_item_cb, url)
    
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
    
    
# Parse Youku's detail page
yk_showid_re = re.compile(r'''showid:['"](\d+)['"]''')
yk_img_name_re = re.compile(r'''<img src=['"](.+?ykimg.+?)['"] alt=['"](.+?)['"]''')
yk_summary_re = re.compile(r'''<span class=['"]intro-more hide['"]>([^>]+)</span>''')
yk_rating_re = re.compile(r'''<span class=['"]star-num['"]>(.+?)</span>''')
def load_youku_item_cb(page, data):
    result = {}
    match = yk_img_name_re.search(page)
    if match:
        image, name = match.group(1, 2)
        result['image'] = image
        result['name'] = name
    match = yk_summary_re.search(page)
    if match:
        result['summary'] = match.group(1)
    match = yk_rating_re.search(page)
    if match:
        result['rating'] = float(match.group(1))
    match = yk_showid_re.search(page)
    if match:
        iid = match.group(1)
        result['iid'] = iid
        list_url = 'http://list.youku.com/show/module?tab=showInfo&id=' + iid
        moonplayer.download_page(list_url, load_youku_item_cb2, result)
    else:
        moonplayer.warn('[Youku-Detail] Cannot get the show_id!')
        
yk_li_re = re.compile(r'''<li \s*?data-id=['"](reload_.+?)['"].*?>(.+?)</li>''')
yk_item_re = re.compile(r'''<a [^>]*?href=['"](//v\.youku\.com.+?)['"][^>]*?>([^<]+?)</a>''')
def load_youku_item_cb2(page, result):
    srcs = []
    page = json.loads(page)['html']
    match = yk_item_re.search(page)
    while match:
        srcs.append(match.group(2))
        srcs.append('http:' + match.group(1))
        match = yk_item_re.search(page, match.end(0))
    # Child lists
    match = yk_li_re.search(page)
    while match:
        srcs.append(match.group(2))
        srcs.append('http://list.youku.com/show/episode?id=%s&stage=%s' % (result['iid'], match.group(1)))
        match = yk_li_re.search(page, match.end(0))
    result['source'] = srcs
    moonplayer.show_detail(result)
        
