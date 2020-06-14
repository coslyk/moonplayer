import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomDialog {
    id: selectionDialog
    
    property var items: []
    property alias currentIndex: listView.currentIndex
    
    width: 400
    height: 300
    title: qsTr("Selection")
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    ScrollView {
        anchors.fill: parent
        anchors.margins: 15
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
                color: index == listView.currentIndex ? Color.listItemSelected : hovered ? Color.listItemHovered : "transparent"
                
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
}

