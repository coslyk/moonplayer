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
 
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import com.github.coslyk.moonplayer

ColumnLayout {
    
    property MpvObject mpvObject: null
    property var blockWords: []

    // Subtitles
    Label {
        text: qsTr("Subtitles")
        font.bold: true
        font.pixelSize: 16
    }

    CheckBox {
        text: qsTr("Visible")
        checked: true
        onToggled: mpvObject.subVisible = checked
    }

    Label { text: qsTr("Track:") }
    RowLayout {
        ComboBox {
            id: subComboBox
            model: mpvObject.subtitles
        }
        Button {
            text: qsTr("Set")
            onClicked: mpv.setProperty("sid", subComboBox.currentIndex)
        }
        Button {
            text: qsTr("Add")
            onClicked: addSubtitleDialog.open()
        }
    }
        
    FileDialog {
        id: addSubtitleDialog
        title: qsTr("Please choose a file")
        onAccepted: mpv.addSubtitle(addSubtitleDialog.selectedFiles)
    }
    
    // Danmaku options
    Label {
        text: qsTr("Danmaku")
        font.bold: true
        font.pixelSize: 16
        Layout.topMargin: 20
    }

    Label {
        text: qsTr("Shown comment type")
    }

    RowLayout {
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
    }

    Label {
        text: qsTr("Bottom reserved area")
    }

    ComboBox {
        id: reserveSpinBox
        model: ["0 %", "10 %", "20 %", "30 %"]
        onActivated: reloadDanmaku()
    }

    // Blocked words
    Label {
        text: qsTr("Blocked words")
    }

    GridLayout {
        columns: 2

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

        Button {
            id: addBlockWordButton
            text: qsTr("Add")
            onClicked: {
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
            Layout.fillWidth: true
        }

        Button {
            id: removeBlockWordButton
            text: qsTr("Remove")
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

