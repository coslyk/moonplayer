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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import com.github.coslyk.moonplayer 1.0

Item {
    id: playlist

    signal openFileRequested()
    signal openUrlRequested()
    
    GridLayout {
        anchors.fill: parent
        columns: 3
        
        Label {
            text: qsTr("Playlist")
            font.pointSize: 16
            font.bold: true
            Layout.columnSpan: 3
        }
        
        ScrollView {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            ListView {
                id: listView
                
                model: PlaylistModel
                delegate: Rectangle {
                    width: 150
                    height: 20
                    radius: 4
                    clip: true
                    color: index == listView.currentIndex ? SkinColor.listItemSelected : (index == PlaylistModel.playingIndex ? SkinColor.listItemCurrentActive : "transparent")
                    Label { text: title }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: listView.currentIndex = index
                        onDoubleClicked: PlaylistModel.playItem(index)
                    }
                }
            }
        }
        
        Button {
            id: addButton
            text: qsTr("Add")
            implicitWidth: 55
            onClicked: {
                addMenu.x = addButton.x
                addMenu.y = addButton.y - addMenu.height
                addMenu.open()
            }
        }
        
        Button {
            id: delButton
            text: qsTr("Del")
            implicitWidth: 55
            onClicked: PlaylistModel.removeItem(listView.currentIndex)
        }
        
        Button {
            id: clearButton
            text: qsTr("Clear")
            implicitWidth: 55
            onClicked: PlaylistModel.clear()
        }
    }
    
    Menu {
        id: addMenu
        width: 100
        MenuItem {
            text: qsTr("File...")
            onTriggered: openFileRequested()
        }
        MenuItem {
            text: qsTr("Url...")
            onTriggered: openUrlRequested()
        }
    }
}
