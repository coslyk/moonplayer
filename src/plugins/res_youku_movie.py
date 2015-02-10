#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
import re
from moonplayer_utils import list_links

tags = ['全部', '武侠', '警匪', '犯罪', '科幻', '战争', '恐怖', '惊悚', '纪录片',
        '西部', '戏曲', '歌舞', '奇幻', '冒险', '悬疑', '历史', '动作',
        '传记', '动画', '儿童', '喜剧', '爱情', '剧情', '运动']

countries = ['全部', '大陆', '香港', '台湾', '韩国', '美国', '法国', '英国',
             '德国', '意大利', '加拿大', '印度', '俄罗斯', '泰国', '其他']

def search(args):
    if 'key' in args:
        moonplayer.warn('Not supported.')
        return
    tag = args['tag']
    country = args['country']
    if tag == '全部':
        tag = ''
    if country == '全部':
        country = ''
    url = 'http://www.youku.com/v_olist/c_96_g_%s_a_%s_s_1_d_2_pt_1_p_%i.html' % \
           (tag, country, args['page'])
    moonplayer.get_url(url, search_cb, None)

pic_re = re.compile(r'<img src="(.+?)" alt="(.+)">')
def search_cb(page, data):
    name2pic = {}
    result = []
    # Read all pic urls
    match = pic_re.search(page)
    while match:
        (url, name) = match.group(1, 2)
        name2pic[name] = url
        match = pic_re.search(page, match.end(0))
    # Read links, bind them with relative pic urls
    links = list_links(page, 'http://www.youku.com/show_page/')
    for i in xrange(0, len(links), 2):
        name = links[i]
        url = links[i+1]
        try:
            result.append({'name': name,
                           'flag': url,
                           'pic_url': name2pic[name]})
        except KeyError:
            pass
    moonplayer.res_show(result)
    
from res_youku_tv import load_item
