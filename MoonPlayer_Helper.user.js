// ==UserScript==
// @name          MoonPlayer Helper
// @description   Open URLs in MoonPlayer
// @include       http://*.bilibili.com/*
// @include       https://*.bilibili.com/*
// @include       http://*.acfun.cn/*
// @include       https://*.acfun.cn/*
// @include       http://*.baomihua.com/*
// @include       https://*.baomihua.com/*
// @include       http://*.cctv.com/*
// @include       https://*.cctv.com/*
// @include       http://v.ifeng.com/*
// @include       https://v.ifeng.com/*
// @include       http://*.iqiyi.com/*
// @include       https://*.iqiyi.com/*
// @include       http://*.ku6.com/*
// @include       https://*.ku6.com/*
// @include       http://*.le.com/*
// @include       https://*.le.com/*
// @include       http://*.mgtv.com/*
// @include       https://*.mgtv.com/*
// @include       http://*.miaopai.com/*
// @include       https://*.miaopai.com/*
// @include       http://v.163.com/*
// @include       https://v.163.com/*
// @include       http://*.pptv.com/*
// @include       https://*.pptv.com/*
// @include       http://v.qq.com/*
// @include       https://v.qq.com/*
// @include       http://video.sina.com.cn/*
// @include       https://video.sina.com.cn/*
// @include       http://tv.sohu.com/*
// @include       https://tv.sohu.com/*
// @include       http://*.tudou.com/*
// @include       https://*.tudou.com/*
// @include       http://*.youku.com/*
// @include       https://*.youku.com/*
// @include       https://*.youtube.com/*
// ==/UserScript==

var new_url = window.location.href.replace(/^http/, 'moonplayer');

var logo = document.createElement("div");
logo.innerHTML = '<div style="margin: 00 auto 0 auto; ' +
    'border-bottom: 1px solid #ffffff; margin-bottom: 5px; ' +
    'font-size: large; background-color: #ffffff; ' +
    'color: #000000;"><p style="margin: 20px 0 20px 0; text-align:center;"> ' +
    '<a href="' + new_url + '">Open in MoonPlayer</a>' +
    '</p></div>';
document.body.insertBefore(logo, document.body.firstChild);
