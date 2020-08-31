import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomDialog {
    id: selectionDialog
    
    property MpvObject mpvObject: null
    property var blockWords: []
    
    width: 450
    height: 300
    title: qsTr("Danmaku options")

    GridLayout {
        anchors.fill: parent
        anchors.margins: 15
        flow: GridLayout.TopToBottom
        rows: 6
        rowSpacing: 0
        columnSpacing: 20

        Label {
            text: qsTr("Shown comment type")
            font.bold: true
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
            text: qsTr("Bottom reserved height")
            font.bold: true
        }

        ComboBox {
            id: reserveSpinBox
            model: ["0 %", "10 %", "20 %", "30 %"]
        }

        Label {
            text: qsTr("Blocked words")
            font.bold: true
            Layout.columnSpan: 2
        }

        TextField {
            id: blockWordInput
            selectByMouse: true
            Layout.fillWidth: true
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
            blockWords
        );
    }
}

