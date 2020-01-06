import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Dialog {
    id: selectionDialog
    
    property var items: []
    property alias currentIndex: listView.currentIndex
    
    width: 400
    height: 300
    title: qsTr("Selection")
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    ListView {
        id: listView
        property int mouseOverIndex: -1
        anchors.fill: parent
        model: items
        
        delegate: Rectangle {
            height: 25
            width: parent.width
            color: index == listView.currentIndex ? "lightsteelblue" : "white"
            
            Text {
                text: modelData
                anchors.fill: parent
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                onClicked: listView.currentIndex = index
                onDoubleClicked: selectionDialog.accept()
            }
        }
    }
}

