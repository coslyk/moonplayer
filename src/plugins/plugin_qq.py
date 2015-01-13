#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
import json
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree

hosts = ('v.qq.com',)

def search(keyword, page):
    keyword = keyword.replace(' ', '%20')
    url = 'http://s.video.qq.com/search?comment=0&plat=2&otype=xml&query=%s&cur=%d&num=20&start=0&end=0' % (keyword, page - 1)
    moonplayer.get_url(url, search_cb, None)
    
def search_cb(content, data):
    root = ET.fromstring(content)
    result = []
    for item in root.iter('list'):
        result.append(item.find('title').text)
        result.append(item.find('AW').text)
    moonplayer.show_list(result)
    
## Parse videos 
def parse(url, options):
    parser.parse(url, options)

cover_re = re.compile(r'''COVER_INFO\s*=\s*{[^}]+?title\s*:\s*['"](.+?)['"]''')
name_re = re.compile(r'''VIDEO_INFO\s*=\s*{[^}]+?title\s*:\s*['"](.+?)['"]''')
vid_re = re.compile(r'''vid\s*:\s*['"](.+?)['"]''')
class Parser(object):
    def parse(self, url, options):
        moonplayer.get_url(url, self.parse_cb, options)
        
    def parse_cb(self, page, options):
        match = name_re.search(page)
        if not match:
            moonplayer.warn('Cannot get the video name!')
            return
        self.name = match.group(1)
        if self.name.isdigit():
            match = cover_re.search(page)
            if match:
                self.name = match.group(1) + '-' + self.name
        
        match = vid_re.search(page)
        if match:
            self.vid = match.group(1)
            url = 'http://vv.video.qq.com/getinfo?otype=json&vids=' + self.vid
            moonplayer.get_url(url, self.parse_cb2, options)
        else:
            moonplayer.warn('Cannot find the vid of this video.')
        
    def parse_cb2(self, page, options):
        self.root = root = json.loads(page[13:-1])
        vtypes = {v[u'name']: v[u'id'] for v in root[u'fl'][u'fi']}
        if options & moonplayer.OPT_QL_SUPER and u'shd' in vtypes:
            self.qid = vtypes[u'shd']
        elif options & (moonplayer.OPT_QL_SUPER|moonplayer.OPT_QL_HIGH) and u'hd' in vtypes:
            self.qid = vtypes[u'hd']
        else:
            self.qid = vtypes[u'sd']
        self.count = int(root['vl']['vi'][0]['cl']['fc'])
        self.current = 0
        self.urlpre = root['vl']['vi'][0]['ul']['ui'][-1]['url']
        self.result = []
        self.parse_keys(options)
        
    def parse_keys(self, options):
        self.current += 1
        self.fn = '%s.p%s.%s.mp4' % (self.vid, self.qid%10000, str(self.current))
        url = 'http://vv.video.qq.com/getkey?format=%s&filename=%s&vid=%s&otype=json' % (self.qid, self.fn, self.vid)
        moonplayer.get_url(url, self.parse_keys_cb, options)
        
    def parse_keys_cb(self, page, options):
        skey = json.loads(page.split('=')[1][:-1])['key']
        vurl = '%s%s?vkey=%s' % (self.urlpre, self.fn, skey)
        name = '%s_%i.mp4' % (self.name, self.current)
        self.result.append(name)
        self.result.append(vurl)
        if self.current < self.count:
            self.parse_keys(options)
        elif options & moonplayer.OPT_DOWNLOAD:
            moonplayer.download(self.result, self.name + '.mp4')
        else:
            moonplayer.play(self.result)
            
parser = Parser()
