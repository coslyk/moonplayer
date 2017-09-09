#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import json
import time
from urllib import urlencode

res_name = 'Youtube'

tags = ['None']
countries = ['None']

appkey = 'AIzaSyCIM4EzNqi1in22f4Z3Ru3iYvLaY8tc3bo'

## Explore
bangumi_list = None

def explore(tag, country, page):
    moonplayer.warn('Not supported')


## Search
def search(key, page):
    global pageTokens
    qs = {
        'q': key,
        'maxResults': 25,
        'safeSearch': 'none',
        'part': 'id,snippet',
        'type': 'video',
        'key': appkey
    }
    if page == 1:
        pageTokens = ['', '']
    elif page < len(pageTokens):
        qs['pageToken'] = pageTokens[page]
    else:
        moonplayer.warn("Cannot skip page due to the limitation of Youtube's API.")
        return
    url = 'https://www.googleapis.com/youtube/v3/search?' + urlencode(qs)
    moonplayer.download_page(url, search_cb, page)

def search_cb(content, page):
    global ytb_details
    data = json.loads(content)
    result = []
    ytb_details = {}
    for item in data['items']:
        name = item['snippet']['title']
        url = 'https://www.youtube.com/watch?v=' + item['id']['videoId']
        t = {
            'name': name,
            'url': url,
            'pic_url': item['snippet']['thumbnails']['default']['url']
        }
        result.append(t)
        detail = {
            'name': name,
            'image': item['snippet']['thumbnails']['high']['url'],
            'dates': item['snippet']['publishedAt'].split('T'),
            'summary': item['snippet']['description'],
            'source': [name, url]
        }
        ytb_details[url] = detail
    if page + 1 == len(pageTokens) and 'nextPageToken' in data:
        pageTokens.append(data['nextPageToken'])
    moonplayer.res_show(result)

## Load item
def load_item(url):
    moonplayer.show_detail(ytb_details[url])
