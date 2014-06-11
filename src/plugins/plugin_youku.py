#!/usr/bin/python
# -*- encoding: utf-8 -*-

from utils import list_links, re2
import re
import json
import moonplayer

#hosts
hosts = ('v.youku.com',)
    
#search videos
def search(keyword, page):
    url = 'http://www.soku.com/search_video/q_' + keyword + '_orderby_1_page_' + str(page)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    #movies, TV series and details
    movies = list_links(content, 'http://www.youku.com/show_page/id_')
    details = list_links(content, '/detail/show/')
    n = (len(movies) + len(details)) >> 1
    #videos
    links = movies + details + list_links(content, 'http://v.youku.com/v_show/')
    moonplayer.show_list(links)
    for i in xrange(n):
        moonplayer.set_list_item_color(i, '#0000ff')
    
#search albums
def search_album(keyword, page):
    url = 'http://www.soku.com/search_playlist/q_' + keyword + '_orderby_1_page_' + str(page)
    moonplayer.get_url(url, search_album_cb, None)
    
def search_album_cb(content, data):
    links = list_links(content, 'http://www.youku.com/playlist_show/')
    moonplayer.show_list(links)
    
## Parse videos or albums
id_re = re.compile(r'http://v.youku.com/v_show/id_(.+)\.html')
def parse(url, options):
    #albums
    if url.startswith('http://www.youku.com/playlist_show/'):
        prefix = url[0:-5]
        moonplayer.get_url(url, parse_album_cb, (prefix, 1, []))
        return
    #details
    elif url.startswith('/detail/show/'):
        url = 'http://www.soku.com' + url
        moonplayer.get_url(url, parse_details_cb, None)
        return
    #movies or tv series
    elif url.startswith('http://www.youku.com/show_page/id_'):
        url2 = url.replace('/show_page/', '/show_episode/')
        moonplayer.get_url(url2, parse_series_cb, url)
        return
    #single video
    match = id_re.match(url)
    if not match:
        moonplayer.warn('Please input a valid youku url.')
        return
    url = 'http://v.youku.com/player/getPlayList/VideoIDS/' + match.group(1)
    moonplayer.get_url(url, parse_cb, options)
    
## Parse videos
def parse_cb(page, options):
    data = json.loads(page)[u'data'][0]
    vid = data[u'vidEncoded']
    #alternative language
    try:
        langvid = data[u'dvd'][u'audiolang'][1][u'vid']
        if langvid != vid:
            lang = data[u'dvd'][u'audiolang'][1][u'lang'].encode('UTF-8')
            if moonplayer.question('是否解析为：' + lang):
                url = 'http://v.youku.com/player/getPlayList/VideoIDS/' + str(langvid)
                moonplayer.get_url(url, parse_cb, options)
                return
    except KeyError:
        pass
    except IndexError:
        pass
    #check error
    if u'error' in data:
        moonplayer.warn('Error: ' + data[u'error'].encode('UTF-8'))
        return
    # Set quality
    segs = data[u'segs']
    name = data[u'title'].encode('UTF-8')
    if options & moonplayer.OPT_QL_SUPER and u'hd2' in segs:
        fmt = 'hd2'
    elif options & (moonplayer.OPT_QL_HIGH | moonplayer.OPT_QL_SUPER) and u'mp4' in segs:
        fmt = 'mp4'
    else:
        fmt = 'flv'
    # Get video's urls
    url = 'http://v.youku.com/player/getM3U8/vid/%s/type/%s/ts/v.m3u8' % (vid, fmt)
    moonplayer.get_url(url, parse_cb2, (options, name))
    
cb2_re = re.compile(r'(http://.+?)\.ts')
def parse_cb2(page, data):
    prev_url = None
    i = 0
    result = []
    options = data[0]
    name = data[1]
    match = cb2_re.search(page)
    while match:
        url = match.group(1)
        if url != prev_url:
            prev_url = url
            fmt = url.split('.')[-1]
            result.append('%s_%i.%s' % (name, i, fmt))
            result.append(url)
            i += 1
        match = cb2_re.search(page, match.end(0))
    if options & moonplayer.OPT_DOWNLOAD:
        if i == 1: # Only one video
            moonplayer.download(result)
        else:
            moonplayer.download(result, '%s.%s' % (name, fmt))
    else:
        moonplayer.play(result)
        
## Parse details
detail_re = re.compile(r'<a href="(http://v.youku.com/.+?)".+?>(\d+)</a>')
def parse_details_cb(content, data):
    links = []
    match = detail_re.search(content)
    while match:
        (url, name) = match.group(1, 2)
        links.append(name)
        links.append(url)
        match = detail_re.search(content, match.end(0))
    moonplayer.show_album(links)
        
## Parse TV series
def parse_series_cb(content, mov_url):
    links = []
    match = re2.search(content)
    if not match:
        moonplayer.get_url(mov_url, parse_movie_cb, None) #movie
        return
    while match:
        (name, url) = match.group(2, 1)
        links.append(name)
        links.append(url)
        match = re2.search(content, match.end(0))
    moonplayer.show_album(links)
    
## Parse movies
def parse_movie_cb(content, data):
    links = list_links(content, 'http://v.youku.com/v_show/')
    moonplayer.show_album(links)

## Parse albums
def parse_album_cb(content, data):
    prefix = data[0]
    now_page = data[1]
    items = data[2]
    new_items = list_links(content, 'http://v.youku.com/v_show/id_')
    items += new_items
    if len(new_items) > 0:
        url = prefix + '_ascending_1_mode_pic_page_' + str(now_page+1) + '.html'
        moonplayer.get_url(url, parse_album_cb, (prefix, now_page+1, items))
    else:
        moonplayer.show_album(items)
    
## Resource library:
tv_types = ['古装', '武侠', '警匪', '军事', '神话', '科幻', '悬疑', '历史',
            '儿童', '农村', '都市', '家庭', '搞笑', '偶像', '言情', '时装', '优酷出品']
movie_types = ['武侠', '警匪', '犯罪', '科幻', '战争', '恐怖', '惊悚', '纪录片',
               '西部', '戏曲', '歌舞', '奇幻', '冒险', '悬疑', '历史', '动作',
               '传记', '动画', '儿童', '喜剧', '爱情', '剧情', '运动', '短片', '优酷出品']
def library(is_movie, tp, page):
    if is_movie:
        url = 'http://www.youku.com/v_olist/c_96_g_' + tp + '_fe_1_o_6_p_' + str(page) +'.html'
    else:
        url = 'http://www.youku.com/v_olist/c_97_g_' + tp + '_o_6_p_' + str(page) +'.html'
    moonplayer.get_url(url, library_cb, None)
    
def library_cb(content, data):
    links = list_links(content, 'http://www.youku.com/show_page/id_')
    moonplayer.show_list(links)
 
