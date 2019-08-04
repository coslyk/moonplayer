#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''A HLS stream downloader that does not require ffmpeg
    Usage:
        download_hls(m3u8_url, name)

    Exception:
        UnsupportedError: This m3u8 format is currently not supported
        SegmentDownloadError: One of the segment is not able to be downloaded
        Mp4ConvertError: Cannot convert mpeg-ts file to mp4
'''

from __future__ import print_function

class UnsupportedError(Exception):
    pass

class SegmentDownloadError(Exception):
    pass

class Mp4ConvertError(Exception):
    pass

import argparse
import re
import binascii
import os
import socket
import subprocess
import sys
from distutils.spawn import find_executable

try:
    from urllib.request import Request, urlopen, ProxyHandler, install_opener, build_opener
    from urllib.parse import urljoin, urlparse
except:
    from urllib2 import Request, urlopen, ProxyHandler, install_opener, build_opener
    from urlparse import urljoin, urlparse

import struct
try:
    struct.pack('!I', 0)
    compat_struct_pack = struct.pack
    compat_struct_unpack = struct.unpack
except TypeError:
    # In Python 2.6 and 2.7.x < 2.7.7, struct requires a bytes argument
    # See https://bugs.python.org/issue19099
    def compat_struct_pack(spec, *args):
        if isinstance(spec, compat_str):
            spec = spec.encode('ascii')
        return struct.pack(spec, *args)

    def compat_struct_unpack(spec, *args):
        if isinstance(spec, compat_str):
            spec = spec.encode('ascii')
        return struct.unpack(spec, *args)
try:
    from Crypto.Cipher import AES
    can_decrypt_frag = True
except ImportError:
    can_decrypt_frag = False


# Download functions
fake_headers = {
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
    'Accept-Encoding': 'gzip, deflate',
    'Accept-Language': 'zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3',
    'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0 Iceweasel/38.2.1'
}

def get_content(url, headers=fake_headers):
    req = Request(url, headers=headers)
    response = urlopen(req)
    data = response.read()

    # Handle HTTP compression for gzip and deflate (zlib)
    resheader = response.info()
    if 'Content-Encoding' in resheader:
        content_encoding = resheader['Content-Encoding']
    elif hasattr(resheader, 'get_payload'):
        payload = resheader.get_payload()
        if isinstance(payload, str):
            content_encoding =  re.match(r'Content-Encoding:\s*([\w-]+)', payload).group(1)
        else:
            content_encoding = None
    else:
        content_encoding = None
    if content_encoding == 'gzip':
        data = ungzip(data)
    elif content_encoding == 'deflate':
        data = undeflate(data)

    # Decode the response body
    charset = None
    if 'Content-Type' in resheader:
        match = re.match(r'charset=([\w-]+)', resheader['Content-Type'])
        if match:
            charset = match.group(1)
    if charset is None:
        match = re.match(r'charset=\"?([\w-]+)', str(data))
        if match:
            charset = match.group(1)
    if charset is None:
        charset = 'utf-8'
    try:
        data = data.decode(charset, errors='replace')
    except:
        print("wrong charset for {}".format(url), file=sys.stderr)
    return data

def ungzip(data):
    """Decompresses data for Content-Encoding: gzip.
    """
    from io import BytesIO
    import gzip
    buffer = BytesIO(data)
    f = gzip.GzipFile(fileobj=buffer)
    return f.read()

def undeflate(data):
    """Decompresses data for Content-Encoding: deflate.
    (the zlib compression is used.)
    """
    import zlib
    decompressobj = zlib.decompressobj(-zlib.MAX_WBITS)
    return decompressobj.decompress(data)+decompressobj.flush()


# Check functions
def _can_decrypt(manifest):
    is_aes128_enc = '#EXT-X-KEY:METHOD=AES-128' in manifest
    return can_decrypt_frag or not is_aes128_enc

def _can_download(manifest):
    check_results = []
    check_results.append(not re.search(r'#EXT-X-KEY:METHOD=(?!NONE|AES-128)', manifest))
    is_aes128_enc = '#EXT-X-KEY:METHOD=AES-128' in manifest
    check_results.append(can_decrypt_frag or not is_aes128_enc)
    check_results.append(not (is_aes128_enc and r'#EXT-X-BYTERANGE' in manifest))
    return all(check_results)

def _parse_m3u8_attributes(attrib):
    info = {}
    for (key, val) in re.findall(r'(?P<key>[A-Z0-9-]+)=(?P<val>"[^"]+"|[^",]+)(?:,|$)', attrib):
        if val.startswith('"'):
            val = val[1:-1]
        info[key] = val
    return info


# Post processor
def _ffmpeg_available():
    return find_executable('ffmpeg') is not None

def _get_audio_codec(filename):
    cmd = ['ffmpeg', '-i', filename]
    try:
        handle = subprocess.Popen(
            cmd, stderr=subprocess.PIPE,
            stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        stdout_data, stderr_data = handle.communicate()
        if handle.wait() != 1:
            return None
    except (IOError, OSError):
        return None
    output = stderr_data.decode('ascii', 'ignore')
    match = re.search(
        r'Stream\s*#\d+:\d+(?:\[0x[0-9a-f]+\])?(?:\([a-z]{3}\))?:\s*Audio:\s*([0-9a-z]+)',
        output)
    if match:
        return match.group(1)
    return None

def _convert_ts_to_mp4(in_file, out_file):
    cmd = ['ffmpeg', '-y', '-i', in_file, '-c', 'copy', '-f', 'mp4']
    acodec = _get_audio_codec(in_file)
    print('Detected audio codec:', acodec)
    if acodec == 'aac':
        cmd += ['-bsf:a', 'aac_adtstoasc']
    cmd.append(out_file)
    handle = subprocess.Popen(
        cmd, stderr=subprocess.PIPE,
        stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    stdout, stderr = handle.communicate()
    if handle.wait() != 0:
        stderr = stderr.decode('utf-8', 'replace')
        msg = stderr.strip().split('\n')[-1]
        raise Mp4ConvertError(msg)


# Main function
def download_hls(m3u8_url, name):
    # Check if it is supported
    manifest = get_content(m3u8_url)
    if not _can_download(manifest):
        if not _can_decrypt(manifest):
            raise UnsupportedError('Please install PyCryto.')
        else:
            raise UnsupportedError('This HLS file is not supported. Consider using FFMpeg.')
    
    def is_ad_fragment_start(s):
        return (s.startswith('#ANVATO-SEGMENT-INFO') and 'type=ad' in s
            or s.startswith('#UPLYNK-SEGMENT') and s.endswith(',ad'))

    def is_ad_fragment_end(s):
        return (s.startswith('#ANVATO-SEGMENT-INFO') and 'type=master' in s
            or s.startswith('#UPLYNK-SEGMENT') and s.endswith(',segment'))
    
    # Get total number of fragments
    total_frags = 0
    ad_frags = 0
    ad_frag_next = False
    for line in manifest.splitlines():
        line = line.strip()
        if not line:
            continue
        if line.startswith('#'):
            if is_ad_fragment_start(line):
                ad_frag_next = True
            elif is_ad_fragment_end(line):
                ad_frag_next = False
            continue
        if ad_frag_next:
            ad_frags += 1
            continue
        total_frags += 1

    # retry times
    retry_times = 3

    # Set timeout to 10s.
    # If a segment cannot be downloaded within 10s, retry it.
    socket.setdefaulttimeout(10)

    # Scan manifest
    current_seg = 0
    decrypt_info = {'METHOD': 'NONE'}
    byte_range = {}
    ad_frag_next = False
    headers = fake_headers.copy()
    segments_info = []

    for line in manifest.splitlines():
        line = line.strip()

        # Segment's URL
        if line:
            if not line.startswith('#'):
                # Skip ad segment
                if ad_frag_next:
                    continue
                frag_url = (
                    line
                    if re.match(r'^https?://', line)
                    else urljoin(m3u8_url, line))
                segments_info.append({
                    'url': frag_url,
                    'byte_range': byte_range.copy(),
                    'current_seg': current_seg,
                    'decrypt_info': decrypt_info.copy()
                    })
                current_seg += 1

            # Extract key
            elif line.startswith('#EXT-X-KEY'):
                decrypt_url = decrypt_info.get('URI')
                decrypt_info = _parse_m3u8_attributes(line[11:])
                if decrypt_info['METHOD'] == 'AES-128':
                    if 'IV' in decrypt_info:
                        decrypt_info['IV'] = binascii.unhexlify(decrypt_info['IV'][2:].zfill(32))
                    if not re.match(r'^https?://', decrypt_info['URI']):
                        decrypt_info['URI'] = urljoin(
                            m3u8_url, decrypt_info['URI'])
                    if decrypt_url != decrypt_info['URI']:
                        decrypt_info['KEY'] = None
            
            # Correct seq number
            elif line.startswith('#EXT-X-MEDIA-SEQUENCE'):
                current_seg = int(line[22:])

            # Get range
            elif line.startswith('#EXT-X-BYTERANGE'):
                splitted_byte_range = line[17:].split('@')
                sub_range_start = int(splitted_byte_range[1]) if len(splitted_byte_range) == 2 else byte_range['end']
                byte_range = {
                    'start': sub_range_start,
                    'end': sub_range_start + int(splitted_byte_range[0]),
                }

            # Skip ads
            elif is_ad_fragment_start(line):
                ad_frag_next = True
            elif is_ad_fragment_end(line):
                ad_frag_next = False


    # Download segments
    ts_file = name + '.ts'
    with open(ts_file, 'wb') as f:
        i = 0
        total = len(segments_info)
        for segment in segments_info:
            print('Download segments: (%i/%i)' % (i, total))
            sys.stdout.flush()

            # Set header
            headers = fake_headers.copy()
            if segment['byte_range']:
                headers['Range'] = 'bytes=%d-%d' % (segment['byte_range']['start'], segment['byte_range']['end'])

            # Download
            retry_count = 0
            while retry_count < retry_times:
                try:
                    req = Request(segment['url'], headers=headers)
                    frag_content = urlopen(req).read()
                    break
                except:
                    retry_count += 1
                    if retry_count < retry_times:
                        print('Download segment %i fails, retry...' % i)
                    else:
                        raise SegmentDownloadError('Download segment %i fails. Stop downloading.' % i)

            # Decrypt
            if segment['decrypt_info']['METHOD'] == 'AES-128':
                iv = segment['decrypt_info'].get('IV') or compat_struct_pack('>8xq', segment['current_seg'])
                key = segment['decrypt_info'].get('KEY') or get_content(segment['decrypt_info']['URI'])
                frag_content = AES.new(key, AES.MODE_CBC, iv).decrypt(frag_content)

            # Write to file
            f.write(frag_content)
            i += 1

    # Convert ts file to mp4
    if not _ffmpeg_available():
        print('Please install ffmpeg to convert .ts file to mp4.')
        return
    print('Convert file to mp4...')
    mp4_file = name + '.mp4'
    _convert_ts_to_mp4(ts_file, mp4_file)
    os.remove(ts_file)
    print('Finished converting!')


def main():
    parser = argparse.ArgumentParser(description='HLS Downloader')
    parser.add_argument('-x', '--http-proxy', type=str, help='set proxy for http(s) transfer. default: no proxy')
    parser.add_argument('-s', '--socks-proxy', type=str, help='set socks5 proxy. default: no proxy')
    parser.add_argument('-t', '--title', type=str, help='Title of the video')
    parser.add_argument('url', type=str, help='URL of hls stream')
    args = parser.parse_args()

    if args.title:
        title = args.title
    else:
        title = urlparse(args.url).path.split('/')[-1].split('.')[0]

    if args.http_proxy:
        opener = build_opener(ProxyHandler({
            'http': 'http://' + args.http_proxy,
            'https': 'http://' + args.http_proxy
        }))
        install_opener(opener)
        
    elif args.socks_proxy:
        try:
            import socks
            addr, port = args.socks_proxy.split(':')
            socks.set_default_proxy(socks.SOCKS5, addr, int(port))
            socket.socket = socks.socksocket
        except:
            print('Failed to set socks5 proxy. Please install PySocks.', file=sys.stderr)
            
    download_hls(args.url, title)

if __name__ == "__main__":
    main()
