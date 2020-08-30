import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomDialog {
    id: selectionDialog
    
    property MpvObject mpvObject: null
    
    width: 400
    height: 300
    title: qsTr("Danmaku Options")

    ColumnLayout {
        CheckBox {
            id: topCheckBox
            text: qsTr("Top comments")
            checked: true
            onClicked: reloadDanmaku()
        }

        CheckBox {
            id: bottomCheckBox
            text: qsTr("Bottom comments")
            checked: true
            onClicked: reloadDanmaku()
        }

        CheckBox {
            id: scrollingCheckBox
            text: qsTr("Scrolling comments")
            checked: true
            onClicked: reloadDanmaku()
        }
    }

    function reloadDanmaku() {
        mpvObject.reloadDanmaku(topCheckBox.checked, bottomCheckBox.checked, scrollingCheckBox.checked);
    }
}

