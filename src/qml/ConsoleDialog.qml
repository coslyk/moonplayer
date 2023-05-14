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
import MoonPlayer 1.0

Dialog {
    id: consoleDialog
    
    width: 600
    height: 400
    modal: true
    closePolicy: Popup.NoAutoClose

    header: RowLayout {
        Label {
            id: titleLabel
            font.pixelSize: 16
            font.bold: true
            Layout.fillWidth: true
            Layout.topMargin: 16
            Layout.leftMargin: 16
        }
        Button {
            id: closeButton
            implicitWidth: 16
            implicitHeight: 16
            Layout.topMargin: 16
            Layout.rightMargin: 16
            background: Rectangle { color: SkinColor.closeButton; radius: 8; anchors.fill: parent }
            onClicked: consoleDialog.close()
        }
    }

    contentItem: ScrollView {
        clip: true
    
        ListView {
            id: listView
            anchors.fill: parent
            model: Dialogs.consoleOutputs
            
            delegate: Label {
                text: modelData
                height: 25
                width: parent.width
            }
        }
    }

    Connections {
        target: Dialogs
        function onConsoleStarted(title) {
            titleLabel.text = title;
            closeButton.visible = false;
            consoleDialog.open();
        }
        function onConsoleFinished() {
            closeButton.visible = true;
        }
    }
}

