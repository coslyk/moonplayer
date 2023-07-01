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
import QtQuick.Window 2.2
import com.github.coslyk.moonplayer 1.0

Window {
    id: dialog
    flags: Qt.Dialog
    
    property string family
    
    width: 400
    height: 330

    signal accepted()
    signal rejected()

    function accept() {
        dialog.visible = false;
        accepted();
    }

    function reject() {
        dialog.visible = false;
        rejected();
    }

    function open() {
        dialog.visible = true;
    }

    function close() {
        dialog.visible = false;
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // Search field
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Search:")
            }
            TextField {
                id: searchField
                Layout.fillWidth: true
                onTextEdited: {
                    if (text !== "") {
                        listView.model = Qt.fontFamilies().filter(function(word) {
                            return word.toUpperCase().includes(text.toUpperCase());
                        });
                    } else {
                        listView.model = Qt.fontFamilies()
                    }
                }
            }
        }

        // Font list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: listView
                property int mouseOverIndex: -1
                anchors.fill: parent
                model: Qt.fontFamilies()
            
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
                        onClicked: {
                            listView.currentIndex = index;
                            dialog.family = modelData;
                        }
                        onDoubleClicked: {
                            listView.currentIndex = index;
                            dialog.family = modelData;
                            dialog.accept();
                        }
                        onEntered: parent.hovered = true
                        onExited: parent.hovered = false
                    }
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }
    
        // Preview
        Label {
            id: previewLabel
            text: qsTr("Hello world!")
            font.family: dialog.family
            font.pixelSize: 16
            Layout.fillWidth: true
        }
    }
}

