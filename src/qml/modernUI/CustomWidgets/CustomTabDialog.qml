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

Dialog {
    id: window
    
    property alias model: listView.model
    property alias currentIndex: listView.currentIndex
    property alias content: loader.sourceComponent

    // Center in parent
    x: (parent.width - width) / 2;
    y: (parent.height - height) / 2;
    
    contentItem: RowLayout {
        // Side panel
        ListView {
            id: listView
            width: 120
            Layout.fillHeight: true

            delegate: ItemDelegate {
                text: tabLabel
                width: parent.width
                highlighted: ListView.isCurrentItem
                onClicked: listView.currentIndex = index
            }
        }
        
        ToolSeparator {
            Layout.fillHeight: true
        }

        Loader {
            id: loader
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
