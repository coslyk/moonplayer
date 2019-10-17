// ==UserScript==
// @name          MoonPlayer Helper
// @description   Open URLs in MoonPlayer
// @version       1.3.0
// @license       GPLv3
// @author        coslyk
// @email         cos.lyk@gmail.com
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
// @include       http://*.dilidili.name/*
// @include       https://*.dilidili.name/*
// @updateURL     https://github.com/coslyk/moonplayer/raw/master/MoonPlayer_Helper.user.js
// ==/UserScript==

var new_url = window.location.href.replace(/^http/, 'moonplayer');

var float_panel = document.createElement('div');
float_panel.innerHTML = '<div id="float_win" style="position: fixed;' +
          'position: fixed;' +
          'vertical-align: middle;' +
          'width: 100%;' +
          'height: 36px;' +
          'left: 0;' +
          'bottom: 0px;' +
          'z-index: 99999;' +
          'border: 2px solid #ccc;' +
          'background-color: #ddd;' +
          'overflow-y: hidden;' +
          'line-height: 36px;' +
          'font-size: large;' +
          'text-align: center;">' +
          '<a href="' + new_url + '">Open in MoonPlayer</a>' +
          '</div>';
document.body.appendChild(float_panel);
