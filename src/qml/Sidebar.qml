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

Control {
    id: sideBar

    // Width
    property var widths: [
        200,  // Playlist
        300,  // Settings
        300   // Explorer
    ]

    // Color settings
    background: Rectangle {
        implicitWidth: widths[layout.currentIndex]
        color: SkinColor.controlbar
    }

    signal openFileRequested()
    signal openUrlRequested()

    function openPlaylist() {
        layout.currentIndex = 0;
    }

    function openSettings() {
        layout.currentIndex = 1;
    }

    function openExplorer() {
        layout.currentIndex = 2;
    }

    StackLayout {
        id: layout
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        
        // Playlist
        Playlist {
            id: playlist
            onOpenFileRequested: openFileRequested()
            onOpenUrlRequested: openUrlRequested()
        }

        // Settings
        Settings {
            id: settings
        }
    
        // Explorer
        Explorer {
            id: explorer
        }
    }
}