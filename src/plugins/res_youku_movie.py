#!/usr/bin/env python
# -*- coding: utf-8 -*-

import moonplayer
from res_youku_tv import search, search_cb, explore_cb, load_item

res_name = '电影 - 优酷'

tags = ['', '武侠', '警匪', '犯罪', '科幻', '战争', '恐怖', '惊悚', '纪录片', '西部', '戏曲',
        '歌舞', '奇幻', '冒险', '悬疑', '历史', '动作', '传记', '动画', '儿童', '喜剧', '爱情',
        '剧情', '运动']
countries = ['', '大陆', '香港', '台湾', '韩国', '日本', '美国', '法国', '英国', '德国',
             '意大利', '加拿大', '印度', '俄罗斯', '泰国', '其他']

def explore(tag, country, page):
    url = 'http://list.youku.com/category/show/c_96_g_%s_a_%s_s_1_d_2_p_%i.html' % (tag, country, page)
    moonplayer.download_page(url, explore_cb, None)
