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
 
import Qt.labs.folderlistmodel 2.11
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0

// Fallback FileOpenDialog when the system's native dialog is not available
Dialog {
    id: dialog

    property bool selectMultiple: false
    property bool selectFolder: false
    property url fileUrl
    property var fileUrls: [fileUrl]
    
    width: 600
    height: 400
    title: "Open file"
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    standardButtons: Dialog.Ok | Dialog.Cancel

    contentItem: ColumnLayout {

        FolderListModel {
            id: folderModel
            showDirsFirst: true
            showFiles: !selectFolder
            folder: Utils.homeLocation()
        }

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Folder:")
            }
            ComboBox {
                id: folderComboBox
                Layout.fillWidth: true
                model: [
                    folderModel.folder.toString().substring(7),
                    Utils.homeLocation().toString().substring(7),
                    Utils.movieLocation().toString().substring(7),
                    Utils.musicLocation().toString().substring(7),
                    Utils.downloadLocation().toString().substring(7),
                    Utils.desktopLocation().toString().substring(7)
                ]
                onActivated: {
                    if (index != 0) {   // Folder from the list is selected
                        folderModel.folder = Qt.resolvedUrl("file://" + model[index]);
                        if (dialog.selectFolder) {
                            dialog.fileUrl = folderModel.folder; // Select this folder
                        }
                        currentIndex = 0;
                    }
                }
            }
            Button {
                text: qsTr("Go up")
                onClicked: folderModel.folder = folderModel.parentFolder
            }
        }

        ScrollView {
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
    
            ListView {
                id: listView
                anchors.fill: parent
                model: folderModel
            
                delegate: Rectangle {
                    property bool hovered: false
                    height: 32
                    width: parent.width
                    color: index === listView.currentIndex ? SkinColor.listItemSelected : hovered ? SkinColor.listItemHovered : "transparent"

                    // Symbol
                    Rectangle {
                        id: folderSymbol
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        width: 16
                        height: 16
                        radius: 5
                        color: fileIsDir ? SkinColor.folderItem : SkinColor.fileItem
                    }

                    // File / folder name
                    Label {
                        text: fileName
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: folderSymbol.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            listView.currentIndex = index;
                            if (dialog.selectFolder === fileIsDir) {
                                dialog.fileUrl = fileURL;
                            }
                        }
                        onDoubleClicked: {
                            if (fileIsDir) {
                                folderModel.folder = fileURL;
                                if (selectFolder) {
                                    dialog.fileUrl = fileURL;
                                }
                            } else {   // File is double-clicked
                                dialog.fileUrl = fileURL;
                                dialog.accept();
                            }
                        }
                        onEntered: parent.hovered = true
                        onExited: parent.hovered = false
                    }
                }
            }
        }
    }
}

