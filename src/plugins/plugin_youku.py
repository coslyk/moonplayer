#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import json
import time
import base64
from urllib import urlencode, unquote
import urllib2
import flvcd_parser

hosts = ('v.youku.com',)

QL_1080P  = 3
QL_SUPER  = 2
QL_HIGH   = 1
QL_NORMAL = 0

def parse(url, options):
    moonplayer.get_url(url, parse_1, (url, options)) # get essential cookies

def parse_1(content, data):
    url, options = data
    vid = url.split('id_')[1].split('.html')[0]
    url = 'http://play.youku.com/play/get.json?vid=%s&ct=12' % vid
    moonplayer.get_url(url, parse_cb, (options, None, None), 'http://static.youku.com')
    
def parse_cb(page, data):
    options, fallback_sid, fallback_token = data
    # Check errors
    try:
        data = json.loads(page)['data']
    except:
        moonplayer.warn('[Youku] Video not found!')
        return
    if 'error' in data:
        moonplayer.warn('[Youku] Error: ' + data['error']['note'].encode('utf-8'))
        return
    
    # Get title, ep, ip, vid
    title = data['video']['title'].encode('utf-8')
    vid = data['video']['encodeid']
    ep = data['security']['encrypt_string']
    ip = data['security']['ip']
    
    # Sort streams' info
    streams_table = {}
    for stream in data['stream']:
        lang = stream['audio_lang']
        if not lang in streams_table:
            streams_table[lang] = [None] * 4
        if stream['stream_type'] in ('flv', 'flvhd'):
            streams_table[lang][QL_NORMAL] = stream
        elif stream['stream_type'] in ('mp4', 'mp4hd'):
            streams_table[lang][QL_HIGH] = stream
        elif stream['stream_type'] in ('hd2', 'mp4hd2'):
            streams_table[lang][QL_SUPER] = stream
        elif stream['stream_type'] in ('hd3', 'mp4hd3'):
            streams_table[lang][QL_1080P] = stream
            
    # Select video's language
    try:
        for audiolang in data['dvd']['audiolang']:
            if audiolang['vid'] != vid:
                if moonplayer.question('是否切换至：' + audiolang['lang'].encode('utf-8')):
                    url = 'http://play.youku.com/play/get.json?vid=%s&ct=12' % audiolang['vid']
                    moonplayer.get_url(url, parse_cb, (options, None, None), 'http://static.youku.com')
                    return
            else:
                lang = audiolang['langcode']
    except KeyError:
        lang = streams_table.keys()[0]
    streams = streams_table[lang]
        
    # Select video quality
    if options & moonplayer.OPT_QL_1080P and streams[QL_1080P]:
        stream = streams[QL_1080P]
        st = 'flv'
    if options & moonplayer.OPT_QL_SUPER and streams[QL_SUPER]:
        stream = streams[QL_SUPER]
        st = 'flv'
    elif options & moonplayer.OPT_QL_HIGH and streams[QL_HIGH]:
        stream = streams[QL_HIGH]
        st = 'mp4'
    else:
        stream = streams[QL_NORMAL]
        st = 'flv'
            
    # Parse
    if fallback_sid:
        sid = fallback_sid
        token = fallback_token
    else:
        sid, token = trans_e('becaf9be', base64.b64decode(ep)).split('_')
    segs = stream['segs']
    result = []
    for i in xrange(len(segs)):
        name = '%s_%d.%s' % (title, i, st)
        hex_id = hex(i)[2:].upper().zfill(2)
        fileid = stream['stream_fileid'][0:8] + hex_id + stream['stream_fileid'][10:]
        url = 'http://k.youku.com/player/getFlvPath/sid/%s_%s/st/%s/fileid/%s?' % \
            (sid, hex_id, st, fileid)
        param = {
            'K': segs[i]['key'],
            'ctype': 12,
            'ev': 1,
            'oip': ip,
            'token': token,
            'ep': unquote(generate_ep(sid, fileid, token)),
            'ymovie': 1,
            'xfsize': segs[i]['size']}
        url += urlencode(param)
        result.append(name)
        result.append(url)
        
    # Check whether the parsed url is suit for your location.
    try:
        url = result[1].replace('ymovie=', 'yxon=')
        req = urllib2.Request(url)
        req.add_header('User-Agnet', 'moonplayer')
        response = urllib2.urlopen(req, timeout=5)
    except urllib2.HTTPError:
        # Fails first time, use fallback meta
        if fallback_sid != None:
            url = 'http://play.youku.com/play/get.json?vid=%s&ct=10' % vid
            moonplayer.get_url(url, parse_cb, (options, sid, token), 'http://static.youku.com')
        # Fails second time, try flvcd.com parser
        else:
            flvcd_parser.parse('http://v.youku.com/v_show/%s.html' % vid, options)
        return
    
    if options & moonplayer.OPT_DOWNLOAD:
        if len(result) == 2:
            moonplayer.download(result)
        else:
            moonplayer.download(result, title + '.' + st)
    else:
        moonplayer.play(result)
    

def compat_ord(c):
    if type(c) is int:
        return c
    else:
        return ord(c)

def trans_e(s1, s2):
    ls = list(range(256))
    t = 0
    for i in xrange(256):
        t = (t + ls[i] + compat_ord(s1[i % len(s1)])) % 256
        ls[i], ls[t] = ls[t], ls[i]
    s = bytearray()
    x, y = 0, 0
    for i in xrange(len(s2)):
        y = (y + 1) % 256
        x = (x + ls[y]) % 256
        ls[x], ls[y] = ls[y], ls[x]
        s.append(compat_ord(s2[i]) ^ ls[(ls[x] + ls[y]) % 256])
    return bytes(s)

def generate_ep(sid, fileid, token):
    new_ep = trans_e('bf7e5f01', '%s_%s_%s' % (sid, fileid, token))
    return base64.b64encode(new_ep)
