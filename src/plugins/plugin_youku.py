#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import json
import time
import base64

hosts = ('v.youku.com',)

def parse(url, options):
    vid = url.split('id_')[1].split('.html')[0]
    url = 'http://v.youku.com/player/getPlayList/VideoIDS/' + vid + '/Pf/4/ctype/12/ev/1'
    moonplayer.get_url(url, parse_cb, options)
    
def parse_cb(page, options):
    # Check errors
    try:
        data = json.loads(page)['data'][0]
    except:
        moonplayer.warn('Video not found!')
        return
    if 'error' in data:
        moonplayer.warn('Error: ' + data['error'])
        return
    
    # Get title, ep, ip, vid
    title = data['title'].encode('utf-8')
    ep = data['ep']
    ip = data['ip']
    vid = data['vidEncoded']
    # Select video quality
    try:
        if options & moonplayer.OPT_QL_SUPER and 'hd2' in data['streamsizes']:
            streamtype = 'hd2'
            streamsize = data['streamsizes']['hd2']
        elif options & (moonplayer.OPT_QL_SUPER|moonplayer.OPT_QL_HIGH) and \
        'mp4' in data['streamsizes']:
            streamtype = 'mp4'
            streamsize = data['streamsizes']['mp4']
        else:
            streamtype = 'flv'
            streamsize = data['streamsizes']['flv']
    except:
        if options & moonplayer.OPT_QL_SUPER and 'hd2' in data['streamtypes_o']:
            streamtype = 'hd2'
        elif options & (moonplayer.OPT_QL_SUPER|moonplayer.OPT_QL_HIGH) and \
        'mp4' in data['streamtypes_o']:
            streamtype = 'mp4'
        else:
            streamtype = 'flv'
        streamsize = 0
    new_ep, sid, token = generate_ep(vid, ep)
    m3u8_url = 'http://pl.youku.com/playlist/m3u8?ctype=12\
    &ep=%s\
    &ev=1\
    &keyframe=1\
    &oip=%i\
    &sid=%s\
    &token=%s\
    &ts=%i\
    &type=%s\
    &vid=%s' % (new_ep, ip, sid, token, int(time.time()), streamtype, vid)
    m3u8_url = m3u8_url.replace(' ', '')
    moonplayer.get_url(m3u8_url, parse_m3u8_cb, (options, title))
    
def parse_m3u8_cb(page, data):
    options, title = data
    i = 0
    prev_url = ''
    result = []
    for line in page.split('\n'):
        if line.startswith('http://'):
            url = line.split('.ts?')[0]
            if url != prev_url:
                suffix = url.split('.')[-1]
                result.append('%s_%i.%s' % (title, i, suffix))
                result.append(url)
                prev_url = url
                i += 1
    if options & moonplayer.OPT_DOWNLOAD:
        moonplayer.download(result, title)
    else:
        moonplayer.play(result)
    
    
def generate_ep(vid, ep):
    f_code_1 = 'becaf9be'
    f_code_2 = 'bf7e5f01'

    def trans_e(a, c):
        f = h = 0
        b = list(range(256))
        result = ''
        while h < 256:
            f = (f + b[h] + ord(a[h % len(a)])) % 256
            b[h], b[f] = b[f], b[h]
            h += 1
        q = f = h = 0
        while q < len(c):
            h = (h + 1) % 256
            f = (f + b[h]) % 256
            b[h], b[f] = b[f], b[h]
            if isinstance(c[q], int):
                result += chr(c[q] ^ b[(b[h] + b[f]) % 256])
            else:
                result += chr(ord(c[q]) ^ b[(b[h] + b[f]) % 256])
            q += 1
        return result
    e_code = trans_e(f_code_1, base64.b64decode(ep))
    sid, token = e_code.split('_')
    new_ep = trans_e(f_code_2, '%s_%s_%s' % (sid, vid, token))
    return base64.b64encode(new_ep), sid, token
