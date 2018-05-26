#!/usr/bin/env python
# -*- coding: utf-8 -*-

from moonplayer_utils import list_links
from HTMLParser import HTMLParser
import moonplayer
import re
import json

res_name = '电视剧 - 优酷'

tags = ['', '古装', '武侠', '警匪', '军事', '神话', '科幻', '悬疑', '历史', '儿童',
        '农村', '都市', '家庭', '搞笑', '偶像', '言情', '时装', '优酷出品']
countries = ['', '大陆', '香港', '台湾', '韩国', '日本', '美国', '英国', '泰国', '新加坡']

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
    url = 'http://list.youku.com/category/show/c_97_g_%s_a_%s_s_1_d_2_p_%i.html' % (tag, country, page)
    moonplayer.download_page(url, explore_cb, None)
    
    
pic_re = re.compile(r'''<img[^>]*? src="(.+?ykimg.+?)" alt="(.+?)"''')
def explore_cb(page, data):
    page = page.split('大家都在看')[0]
    name2pic = {}
    result = []
    # Read all pic urls
    match = pic_re.search(page)
    while match:
        url, name = match.group(1, 2)
        name2pic[name] = url
        match = pic_re.search(page, match.end(0))
    # Read links, bind them with relative pic urls
    links = list_links(page, '//v.youku.com/v_show')
    for i in xrange(0, len(links), 2):
        name = links[i]
        url = 'http:' + links[i+1]
        try:
            result.append({'name': name,
                           'url': url,
                           'pic_url': name2pic[name]})
        except KeyError:
            pass
    moonplayer.res_show(result)
    

### Load item ###
def load_item(url):
    if url.startswith('http://v.youku.com/v_show/'):
        moonplayer.download_page(url, load_youku_item_url_cb, None)
    elif url.startswith('http://'):
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
yk_item_url_re = re.compile(r'''<a [^>]*?href=\\?"([^"]+?list\.youku\.com/show/id_.+?\.html\\?)"''')
def load_youku_item_url_cb(page, data):
    match = yk_item_url_re.search(page)
    if match:
        url = match.group(1)
        if url.startswith('//'):
            url = 'http:' + url
        moonplayer.download_page(url, load_youku_item_cb, None)
    else:
        moonplayer.warn('Parse failed!')


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
        list_url = 'http://list.youku.com/show/module?tab=showInfo&callback=excited&id=' + iid
        moonplayer.download_page(list_url, load_youku_item_cb2, result)
    else:
        moonplayer.warn('[Youku-Detail] Cannot get the show_id!')
        
yk_li_re = re.compile(r'''<li \s*?data-id=['"](reload_.+?)['"].*?>(.+?)</li>''')
yk_item_re = re.compile(r'''<a [^>]*?href=['"](//v\.youku\.com.+?)['"][^>]*?>([^<]+?)</a>''')
def load_youku_item_cb2(page, result):
    srcs = []
    page = page.split('(', 1)[-1][0:-2] # Remove callback function
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
        url = 'http://list.youku.com/show/episode?callback=excited&id=%s&stage=%s' % (result['iid'], match.group(1))
        srcs.append('python:res_youku_tv.parse_childlist("%s")' % url)
        match = yk_li_re.search(page, match.end(0))
    result['source'] = srcs
    moonplayer.show_detail(result)
        
def parse_childlist(url):
    moonplayer.download_page(url, load_youku_item_cb2, {})
