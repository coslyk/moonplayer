#!/usr/bin/python

import re

re1 = re.compile(r'<a\s[^>]*title="([^"]+)"[^>]*href="([^"]+)"')
re2 = re.compile(r'<a\s[^>]*href="([^"]+)"[^>]*title="([^"]+)"')

def list_links(page, start, keyword = ''):
    links = []
    urls = {}
    match = re1.search(page)
    while match:
        (title, url) = match.group(1, 2)
        if type(start) is str:
            ok = url.startswith(start)
        else:
            ok = start.match(url)
        if ok:
            if keyword in title and not url in urls:
                urls[url] = None
                links.append(title)
                links.append(url)
        match = re1.search(page, match.end(0))
        
    match = re2.search(page)
    while match:
        (title, url) = match.group(2, 1)
        if type(start) is str:
            ok = url.startswith(start)
        else:
            ok = start.match(url)
        if ok:
            if keyword in title and not url in urls:
                urls[url] = None
                links.append(title)
                links.append(url)
        match = re2.search(page, match.end(0))
    return links
    
    
url_re = re.compile(r'<a href="(http://.+?)".+?onclick="_alert.+?>\s*http://')
name_re = re.compile(r'document.title\s*=\s*"([^"]+)"')
def parse_flvcd_page(content, suffix):
    page = content.decode('GBK').encode('UTF-8')
    ret = []
    #get name
    match = name_re.search(page)
    if not match:
        return
    name = match.group(1)
    #get urls
    count = 0
    match = url_re.search(page)
    while match:
        url = match.group(1)
        if not suffix:
            if url.find('mp4') != -1:
                suffix = '.mp4'
            else:
                suffix = '.flv'
        ret.append(name + '_' + str(count) + suffix) #filename
        ret.append(url)  #url
        count += 1
        match = url_re.search(page, match.end(0))
    return ret


charset_re = re.compile(r'charset=[\'"](.+?)[\'"]')
def convert_to_utf8(page):
    match = charset_re.search(page)
    if match:
        charset = match.group(1)
        if charset != 'utf-8':
            return page.decode(charset).encode('utf-8')
    return page