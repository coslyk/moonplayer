import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Popup {
    id: playlistPopup

    signal openFileRequested()
    signal openUrlRequested()

    width: 180
    height: 300
    
    GridLayout {
        anchors.fill: parent
        columns: 3
        
        Label {
            text: qsTr("Playlist")
            font.pixelSize: 16
            font.bold: true
            Layout.columnSpan: 3
        }
        
        ListView {
            id: listView
            Layout.columnSpan: 3
            Layout.fillHeight: true
            
            model: playlistModel
            delegate: Rectangle {
                width: 150
                height: 20
                radius: 4
                color: index == listView.currentIndex ? "lightsteelblue" : (index == playlistModel.playingIndex ? "lightgrey" : "transparent")
                Label {
                    text: title.length > 10 ? title.slice(0, 10) + "..." : title
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: listView.currentIndex = index
                    onDoubleClicked: playlistModel.playItem(index)
                }
            }
        }
        
        Button {
            id: addButton
            text: qsTr("Add")
            implicitHeight: 30
            implicitWidth: 50
            onClicked: {
                addMenu.x = addButton.x
                addMenu.y = addButton.y - addMenu.height
                addMenu.open()
            }
        }
        
        Button {
            id: delButton
            text: qsTr("Del")
            implicitHeight: 30
            implicitWidth: 50
            onClicked: playlistModel.removeItem(listView.currentIndex)
        }
        
        Button {
            id: clearButton
            text: qsTr("Clear")
            implicitHeight: 30
            implicitWidth: 50
            onClicked: playlistModel.clear()
        }
    }
    
    Menu {
        id: addMenu
        width: 100
        MenuItem {
            text: qsTr("File...")
            onTriggered: openFileRequested()
        }
        MenuItem {
            text: qsTr("Url...")
            onTriggered: openUrlRequested()
        }
    }
}
