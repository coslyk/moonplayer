#!/usr/bin/python

import re

###############################
## List all links in a search result
## page and return a result-list
###############################

re1 = re.compile(r'''<a\s[^>]*title=['"]([^'"]+)['"][^>]*href=['"]([^'"]+)['"]''')
re2 = re.compile(r'''<a\s[^>]*href=['"]([^'"]+)['"][^>]*title=['"]([^'"]+)['"]''')

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
            if keyword.lower() in title.lower() and not url in urls:
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
            if keyword.lower() in title.lower() and not url in urls:
                urls[url] = None
                links.append(title)
                links.append(url)
        match = re2.search(page, match.end(0))
    return links

