#!/usr/bin/env python

from __future__ import print_function
from os.path import expanduser, isfile, join
from io import open
from zipfile import ZipFile
import json, os, platform, re, shutil, sys, tempfile
try:
    from urllib.request import urlopen
except:
    from urllib2 import urlopen

# Read / write local files
def read_from_file(filename):
    with open(filename) as f:
        return f.read()

def write_to_file(filename, data):
    with open(filename, 'w') as f:
        f.write(data)

def extract_from_zipfile(filename, path):
    with ZipFile(filename) as zipfile:
        zipfile.extractall(path=APPDATA)

# network functions
def get_content(url):
    return urlopen(url).read()

def download_as_tmpfile(url):
    data = urlopen(url).read()
    (fd, filename) = tempfile.mkstemp('.zip')
    os.close(fd)
    with open(filename, 'wb') as f:
        f.write(data)
    return filename
    
# Get github repo infos
def get_latest_commit(repo):
    url = 'https://api.github.com/repos/{repo}/branches/master'.format(repo=repo)
    data = get_content(url)
    return json.loads(data)['commit']['sha']

def get_latest_release(repo):
    url = 'https://api.github.com/repos/{repo}/releases/latest'.format(repo=repo)
    data = get_content(url)
    return json.loads(data)['tag_name']

# Platform-specific variables
if platform.system() == 'Darwin':
    APPDATA = expanduser('~/Library/Application Support/MoonPlayer')
elif platform.system() == 'Linux':
    XDG_DATA_HOME = os.getenv('XDG_DATA_HOME', expanduser('~/.local/share'))
    APPDATA = XDG_DATA_HOME + '/moonplayer'
else:
    APPDATA = expanduser(r'~\AppData\Local\MoonPlayer')


### Upgrade ykdl ###
print('=> Upgrading ykdl')
sys.stdout.flush()

try:
    current_ver = read_from_file(join(APPDATA, 'ykdl-version.txt')).strip()
    print('Current version:', current_ver)
    sys.stdout.flush()
except:
    current_ver = ''
    print('Current version: not installed')
    sys.stdout.flush()
latest_ver = get_latest_commit('zhangn1985/ykdl')
print('Latest version:', latest_ver)
if current_ver == latest_ver:
    print('Already up-to-date.')
    sys.stdout.flush()
else:
    print('Downloading https://github.com/zhangn1985/ykdl/archive/master.zip')
    sys.stdout.flush()
    zipfile = download_as_tmpfile('https://github.com/zhangn1985/ykdl/archive/master.zip')
    print('Installing...')
    sys.stdout.flush()
    extract_from_zipfile(zipfile, APPDATA)
    shutil.rmtree(join(APPDATA, 'ykdl'), ignore_errors=True)
    shutil.move(join(APPDATA, 'ykdl-master'), join(APPDATA, 'ykdl'))
    os.remove(zipfile)
    write_to_file(join(APPDATA, 'ykdl-version.txt'), latest_ver)
print()


### Upgrade youtube-dl ###
print('=> Upgrading youtube-dl')
sys.stdout.flush()
try:
    data = read_from_file(join(APPDATA, 'youtube_dl', 'version.py'))
    current_ver = re.search(r"'(.+?)'", data).group(1)
    print('Current version:', current_ver)
    sys.stdout.flush()
except:
    current_ver = ''
    print('Current version: not installed')
    sys.stdout.flush()
latest_ver = get_latest_release('rg3/youtube-dl')
print('Latest version:', latest_ver)
if current_ver == latest_ver:
    print('Already up-to-date.')
    sys.stdout.flush()
else:
    url = 'https://github.com/rg3/youtube-dl/archive/{ver}.zip'.format(ver=latest_ver)
    print('Downloading', url)
    sys.stdout.flush()
    zipfile = download_as_tmpfile(url)
    print('Installing...')
    sys.stdout.flush()
    extract_from_zipfile(zipfile, APPDATA)
    shutil.rmtree(join(APPDATA, 'youtube_dl'), ignore_errors=True)
    shutil.move(join(APPDATA, 'youtube-dl-'+latest_ver, 'youtube_dl'), join(APPDATA, 'youtube_dl'))
    shutil.rmtree(join(APPDATA, 'youtube-dl-'+latest_ver), ignore_errors=True)
    os.remove(zipfile)
print()

### Upgrade plugins ###
print('=> Upgrading plugins')
sys.stdout.flush()
try:
    current_ver = read_from_file(join(APPDATA, 'plugins-version.txt')).strip()
    print('Current version:', current_ver)
    sys.stdout.flush()
except:
    current_ver = ''
    print('Current version: not installed')
    sys.stdout.flush()
latest_ver = get_latest_commit('coslyk/moonplayer-plugins')
print('Latest version:', latest_ver)
sys.stdout.flush()
if current_ver == latest_ver:
    print('Already up-to-date.')
    sys.stdout.flush()
else:
    print('Downloading https://github.com/coslyk/moonplayer-plugins/archive/master.zip')
    sys.stdout.flush()
    zipfile = download_as_tmpfile('https://github.com/coslyk/moonplayer-plugins/archive/master.zip')
    print('Installing...')
    sys.stdout.flush()
    extract_from_zipfile(zipfile, APPDATA)
    path = join(APPDATA, 'moonplayer-plugins-master')
    allfiles = [f for f in os.listdir(path) if f.endswith('.py')]
    for f in allfiles:
        try:
            os.remove(join(APPDATA, 'plugins', f))
        except:
            pass
        shutil.move(join(path, f), join(APPDATA, 'plugins', f))
    shutil.rmtree(path, ignore_errors=True)
    os.remove(zipfile)
    write_to_file(join(APPDATA, 'plugins-version.txt'), latest_ver)
print()
