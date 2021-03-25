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
import CustomWidgets 1.0
import MoonPlayer 1.0

CustomDialog {
    id: selectionDialog
    
    property var items: []
    property alias currentIndex: listView.currentIndex
    
    width: 400
    height: 300
    title: qsTr("Selection")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: suggestedMargins
        
        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
    
            ListView {
                id: listView
                property int mouseOverIndex: -1
                anchors.fill: parent
                model: items
            
                delegate: Rectangle {
                    property bool hovered: false
                    height: 25
                    width: parent.width
                    color: index == listView.currentIndex ? SkinColor.listItemSelected : hovered ? SkinColor.listItemHovered : "transparent"
                
                    Label {
                        text: modelData
                        anchors.fill: parent
                        verticalAlignment: Label.AlignVCenter
                    }
                
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: listView.currentIndex = index
                        onDoubleClicked: selectionDialog.accept()
                        onEntered: parent.hovered = true
                        onExited: parent.hovered = false
                    }
                }
            }
        }

        DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            onAccepted: selectionDialog.accept()
            onRejected: selectionDialog.reject()
        }
    }
}

