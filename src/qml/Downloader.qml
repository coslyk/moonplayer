import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomDialog {
    id: downloader
    width: 600
    height: 400
    title: qsTr("Downloader")
    
    contentItem: Item {
        ListView {
            id: listView
            anchors.fill: parent
            anchors.margins: 10
            model: downloaderModel

            onCountChanged: {
                if (count !== 0)
                    downloader.open();
            }
        
            delegate: Rectangle {
                height: 30
                width: parent.width
                color: "transparent"
            
                Label {
                    text: name
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    anchors.right: separator.left
                    anchors.verticalCenter: parent.verticalCenter
                }
            
                ToolSeparator {
                    id: separator
                    anchors.right: stateText.left
                    anchors.verticalCenter: parent.verticalCenter
                }
            
                Label {
                    id: stateText
                    text: (
                        modelData.state === DownloaderItem.DOWNLOADING ? progress + "%" :
                        modelData.state === DownloaderItem.WAITING ? qsTr("Waiting") :
                        modelData.state === DownloaderItem.PAUSED ? qsTr("Paused") :
                       modelData.state === DownloaderItem.FINISHED ? qsTr("Finished") :
                        modelData.state === DownloaderItem.ERROR ? qsTr("Error") :
                        qsTr("Canceled")
                        )
                    width: 80
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onEntered: parent.color = Color.listItemHovered
                    onExited: parent.color = "transparent"
                
                    onClicked: {
                        if (mouse.button == Qt.RightButton)
                        {
                            contextMenu.x = mouse.x
                            contextMenu.y = mouse.y
                            contextMenu.open();
                        }
                    }
                
                    onDoubleClicked: {
                        if (modelData.state === DownloaderItem.FINISHED)
                        {
                            playlistModel.addItem(name, filePath, danmakuUrl);
                            downloader.close();
                        }
                    }
                
                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("Play")
                            enabled: modelData.state === DownloaderItem.FINISHED
                            onTriggered: {
                                playlistModel.addItem(name, filePath);
                                downloader.close();
                            }
                        }
                        MenuItem {
                            text: qsTr("Continue")
                            enabled: modelData.state === DownloaderItem.PAUSED
                            onTriggered: modelData.start()
                        }
                        MenuItem {
                            text: qsTr("Pause")
                            enabled: modelData.state === DownloaderItem.DOWNLOADING
                            onTriggered: modelData.pause()
                        }
                        MenuItem {
                            text: qsTr("Stop")
                            onTriggered: modelData.stop()
                        }
                    }
                }
            }
        }
    
        Label {
            textFormat: Text.RichText
            text: qsTr("<p>There is no files being downloaded now -_-</p>
        
            <p>Add some urls to start download!</p>
        
            <p>Try our <a href=\"https://coslyk.github.io/moonplayer.html#browser_extension\">Browser extensions</a></p>")
        
            anchors.centerIn: parent
            visible: downloaderModel.length === 0
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
