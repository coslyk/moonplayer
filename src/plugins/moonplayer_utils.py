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
    
################################
# Parse result page of flvcd.com
################################
url_re = re.compile(r'<a href="(http://.+?)".+?onclick=.+?>\s*http://')
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
                suffix = 'mp4'
            else:
                suffix = 'flv'
        ret.append('%s_%i.%s' % (name, count, suffix)) #filename
        ret.append(url)  #url
        count += 1
        match = url_re.search(page, match.end(0))
    return ret
    
# Parse result page of flvcd.com, method 2
inf_re = re.compile(r'name="inf" value="([^"]+)"')
def parse_flvcd_inf(page, suffix = None):
    page = page.decode('GBK').encode('UTF-8')
    result = []
    #get name
    match = name_re.search(page)
    if not match:
        return
    name = match.group(1)
    #get suffix
    match = inf_re.search(page)
    if not match:
        return
    inf = match.group(1).split('|')
    if suffix == None:
        if 'mp4' in inf[0]:
            suffix = 'mp4'
        else:
            suffix = 'flv'
    #get urls
    i = 0
    for item in inf:
        if len(item):
            result.append('%s_%i.%s' % (name, i, suffix))
            result.append(item.split('<U>')[-1])
            i += 1
    return result

###########################################
## Convert the html page to one with utf-8 encoding
###########################################

charset_re = re.compile(r'charset=[\'"](.+?)[\'"]')
def convert_to_utf8(page):
    match = charset_re.search(page)
    if match:
        charset = match.group(1)
        if charset != 'utf-8':
            return page.decode(charset).encode('utf-8')
    return page

### Process redirection in Python
# If some errors happen when downloading
# with MoonPlayer, try this.

import urllib2
class RedirectHandler(urllib2.HTTPRedirectHandler):
    def http_error_301(self, req, fp, code, msg, headers):
        pass
    def http_error_302(self, req, fp, code, msg, headers):
        pass

cookieprocessor = urllib2.HTTPCookieProcessor()
my_opener = urllib2.build_opener(RedirectHandler, cookieprocessor)
    
def get_redirected_url(src_url):
    req = urllib2.Request(src_url, headers={'User-Agent': 'moonplayer'})
    try:
        response = my_opener.open(req, timeout=5)
        return response.info()['Location']
    except urllib2.HTTPError, e:
        if e.code == 301 or e.code == 302:
            return e.info()['Location']
        raise e
    
def process_redirections(result):
    for i in xrange(1, len(result), 2):
        origin = result[i]
        url = get_redirected_url(origin)
        result[i] = url
    return url
