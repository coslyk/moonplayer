#!/usr/bin/python

from moonplayer_utils import list_links, convert_to_utf8
import re
import moonplayer
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

#hosts
hosts = ('www.tudou.com',)

#search videos
def search(keyword, page):
    url = 'http://www.soku.com/t/nisearch/' + keyword + \
        '/searchType_item_cid__time__sort_score_display_album_high_0_page_' + str(page)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    links = list_links(content, 'http://www.tudou.com/programs/view/')
    moonplayer.show_list(links)
    
#search albums
def search_album(keyword, page):
    url = 'http://www.soku.com/t/npsearch/' + keyword + \
        '/_cid_0_time__sort_score_display_album_page_' + str(page)
    moonplayer.get_url(url, search_album_cb, keyword)
    
def search_album_cb(page, keyword):
    page = convert_to_utf8(page)
    links = list_links(page, 'http://www.tudou.com/playlist/id/', keyword)
    moonplayer.show_list(links)

#parse videos and albums
def parse(url, options):
    if url.startswith('http://www.tudou.com/playlist/id/'):  #album
        url = 'http://www.tudou.com/plcover/coverPage/getIndexItems.html?page=1&pageSize=512&lid=' + url.split('/')[-2]
        moonplayer.get_url(url, parse_album_cb, None)
        
    elif url.startswith('http://www.tudou.com/listplay/') or url.startswith('http://www.tudou.com/programs/view/'):  #single video
        parser.feed(url, options)
        
    else:  #wrong url
        moonplayer.warn('Please input a valid tudou url.')
    
#parse videos
iid_re = re.compile(r'"pt":(\d+)[^}]+"k":(\d+)')
name_re = re.compile(r'kw:\s*[\'"]([^\'"]+)')

class Parser(object):
    def feed(self, url, options):
        moonplayer.get_url(url, self.parse_cb, options)
        
    def parse_cb(self, page, options):
        #page = content.decode('GBK').encode('UTF-8')
        page = convert_to_utf8(page)
        name_match = name_re.search(page)
        if not name_match:
            moonplayer.warn('Cannot get video name.')
            return
        self.name = name_match.group(1)
        
        if options & moonplayer.OPT_QL_SUPER:
            i = 5
        elif options & moonplayer.OPT_QL_HIGH:
            i = 3
        else:
            i = 2
        vlist = [None] * 6
        iid_match = iid_re.search(page)
        while iid_match:
            (pt, k) = iid_match.group(1, 2)
            pt = int(pt)
            if pt < 6:
                if vlist[pt] == None:
                    vlist[pt] = []
                vlist[pt].append(k)
            iid_match = iid_re.search(page, iid_match.end(0))
        while i >= 0:
            if vlist[i]:
                self.keys = vlist[i]
                self.result = []
                url = 'http://v2.tudou.com/f?id=' + self.keys[0]
                moonplayer.get_url(url, self.parse_keys, options)
                return
            i -= 1
        moonplayer.warn('Fail!')
    
    def parse_keys(self, content, options):
        root = ET.fromstring(content)
        i = len(self.result) / 2
        self.result.append('%s_%i.f4v' % (self.name, i))
        self.result.append(root.text)
        i += 1
        if i < len(self.keys):
            url = 'http://v2.tudou.com/f?id=' + self.keys[i]
            moonplayer.get_url(url, self.parse_keys, options)
        elif options & moonplayer.OPT_DOWNLOAD:
            moonplayer.download(self.result, self.name + '.f4v')
        else:
            moonplayer.play(self.result)
parser = Parser()
            
#parse albums
album_re = re.compile(r'"title":"([^"]+)"[^}]+"code":"([^"]+)"')
def parse_album_cb(content, prefix):
    items = []
    match = album_re.search(content)
    while match:
        items.append(match.group(1)) #name
        items.append('http://www.tudou.com/programs/view/' + match.group(2)) #url
        match = album_re.search(content, match.end(0))
    moonplayer.show_album(items)
