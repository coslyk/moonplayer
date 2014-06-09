#!/usr/bin/python

from utils import list_links
import re
import moonplayer

hosts = ('video.sina.com.cn',)
    
## Search videos
def search(kw, page):
    kw = kw.replace(' ', '+')
    url = 'http://video.sina.com.cn/search/index.php?m1=f&k='+ kw + '&page=' + str(page)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    links = list_links(content, 'http://video.sina.com.cn/v/b/')
    moonplayer.show_list(links)
    
## Search albums
def search_album(kw, page):
    kw = kw.replace(' ', '+')
    url = 'http://video.sina.com.cn/search/index.php?m1=f&m2=f2&f4=0&m3=f5&k=' + str(kw) + '&page=' + str(page)
    moonplayer.get_url(url, search_album_cb, None)
    
def search_album_cb(content, data):
    links = list_links(content, 'http://you.video.sina.com.cn/a/')
    moonplayer.show_list(links)
    
## Parse videos and albums
video_re = re.compile(r'http://video.sina.com.cn/v/b/(\d+)-\d+.html')
album_re = re.compile(r'http://you.video.sina.com.cn/a/(\d+)-(\d+).html')
def parse(url, options):
    #video
    if url.isdigit():
        parse_vid(url, options)
        return
    match = video_re.match(url)
    if match:
        parse_vid(match.group(1), options)
        return
    #album
    match = album_re.match(url)
    if match:
        uid = match.group(2)
        tid = match.group(1)
        url = 'http://you.video.sina.com.cn/api/catevideoList.php?page=1&pagesize=500&uid=%s&tid=%s' % (uid, tid)
        moonplayer.get_url(url, parse_album_cb, None)
        return
    moonplayer.warn('Wrong url')
    
## Parse videos
def parse_vid(vid, options):
    url = 'http://v.iask.com/v_play.php?vid=' + vid
    moonplayer.get_url(url, parse_cb, options)
    
name_re = re.compile(r'<vname><!\[CDATA\[(.+?)]')
url_re = re.compile(r'(http://.+?)]')
def parse_cb(content, options):
    name_match = name_re.search(content)
    url_match = url_re.search(content)
    if not name_match and not url_match:
        moonplayer.warn('Fail')
        return
    name = name_match.group(1)
    result = []
    i = 0
    while url_match:
        url = url_match.group(1)
        result.append('%s_%i.hlv' % (name, i))
        result.append(url)
        i += 1
        url_match = url_re.search(content, url_match.end(0))
    if options & moonplayer.OPT_DOWNLOAD:
        moonplayer.download(result, name)
    else:
        moonplayer.play(result)
        
## Parse albums
parse_album_re = re.compile(r'"vid":"(\d+)".+?"name":"([^"]+)"')
def parse_album_cb(content, data):
    albums = []
    match = parse_album_re.search(content)
    while match:
        vid = match.group(1)
        name = match.group(2).decode('unicode-escape').encode('UTF-8')
        albums.append(name)
        albums.append(vid)
        match = parse_album_re.search(content, match.end(0))
    moonplayer.show_album(albums)