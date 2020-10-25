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
import CustomWidgets 1.0

Dialog {
    id: selectionDialog
    
    property MpvObject mpvObject: null
    property var blockWords: []
    
    width: 450
    height: 350
    title: qsTr("Danmaku options")

    GridLayout {
        anchors.fill: parent
        flow: GridLayout.TopToBottom
        rows: 6
        rowSpacing: 0
        columnSpacing: 20

        Label {
            text: qsTr("Shown comment type")
        }

        CheckBox {
            id: topCheckBox
            text: qsTr("Top")
            checked: true
            onClicked: reloadDanmaku()
        }

        CheckBox {
            id: bottomCheckBox
            text: qsTr("Bottom")
            checked: true
            onClicked: reloadDanmaku()
        }

        CheckBox {
            id: scrollingCheckBox
            text: qsTr("Scrolling")
            checked: true
            onClicked: reloadDanmaku()
        }

        Label {
            text: qsTr("Bottom reserved area")
        }

        ComboBox {
            id: reserveSpinBox
            model: ["0 %", "10 %", "20 %", "30 %"]
            onActivated: reloadDanmaku()
        }

        Label {
            text: qsTr("Blocked words")
            Layout.columnSpan: 2
        }

        TextField {
            id: blockWordInput
            selectByMouse: true
            Layout.fillWidth: true
            onAccepted: {
                if (blockWordInput.text.length !== 0) {
                    blockWords.push(blockWordInput.text);
                    blockWordInput.text = "";
                    blockWordsComboBox.model = blockWords;
                    reloadDanmaku();
                }
            }
        }

        ComboBox {
            id: blockWordsComboBox
            model: blockWords
            Layout.rowSpan: 4
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
        }

        Button {
            id: addBlockWordButton
            text: qsTr("Add")
            Layout.fillWidth: true
            onClicked: {
                if (blockWordInput.text.length !== 0) {
                    blockWords.push(blockWordInput.text);
                    blockWordInput.text = "";
                    blockWordsComboBox.model = blockWords;
                    reloadDanmaku();
                }
            }
        }

        Button {
            id: removeBlockWordButton
            text: qsTr("Remove")
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            onClicked: {
                if (blockWords.length !== 0) {
                    blockWords.splice(blockWordsComboBox.currentIndex, 1);
                    blockWordsComboBox.model = blockWords;
                    reloadDanmaku();
                }
            }
        }
    }

    function reloadDanmaku() {
        mpvObject.reloadDanmaku(
            topCheckBox.checked,
            bottomCheckBox.checked,
            scrollingCheckBox.checked,
            reserveSpinBox.currentIndex / 10.0,
            blockWords
        );
    }
}

