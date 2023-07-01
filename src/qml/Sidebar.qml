/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */
 
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import com.github.coslyk.moonplayer 1.0

Control {
    id: sideBar

    signal openFileRequested()
    signal openUrlRequested()

    property MpvObject mpv: null

    function openPlaylist() {
        layout.currentIndex = 0;
        visible = true;
    }

    function openSettings() {
        layout.currentIndex = 1;
        visible = true;
    }

    function openExplorer() {
        layout.currentIndex = 2;
        visible = true;
    }

    function openVideoOptions() {
        layout.currentIndex = 3;
        visible = true;
    }

    function openSubtitles() {
        layout.currentIndex = 4;
        visible = true;
    }

    function openDownloader() {
        layout.currentIndex = 5;
        visible = true;
    }

    // Width
    property var widths: [
        200,  // Playlist
        320,  // Settings
        300,  // Explorer
        300,  // Video options
        320,  // Subtitles and danmaku
        300,  // Downloader
    ]

    // Color settings
    background: Rectangle {
        implicitWidth: widths[layout.currentIndex]
        color: SkinColor.sidebar
    }

    StackLayout {
        id: layout
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 5
        
        // Playlist
        Playlist {
            id: playlist
            onOpenFileRequested: sideBar.openFileRequested()
            onOpenUrlRequested: sideBar.openUrlRequested()
        }

        // Settings
        Settings {
            id: settings
        }
    
        // Explorer
        Explorer {
            id: explorer
        }

        // Video Options
        VideoOptions {
            id: videoOptions
            mpvObject: mpv
        }

        // Subtitles and danmaku
        SubtitlesAndDanmaku {
            id: subAndDanmaku
            mpvObject: mpv
        }

        // Downloader
        Downloader {
            id: downloader
        }
    }
}
