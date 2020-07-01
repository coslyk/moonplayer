import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0 as QSettings
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomDialog {
    id: explorer
    width: 600
    height: 400
    title: qsTr("Explorer")
    
    // Remember the last used plugin
    QSettings.Settings {
        category: "explorer"
        property alias last_plugin: pluginComboBox.currentIndex
    }
    
    property QtObject currentPlugin: plugins[pluginComboBox.currentIndex]
    
    contentItem: Item {

        GridLayout {
            columns: 3
            visible: plugins.length !== 0
            anchors.fill: parent
            anchors.margins: 10
        
            // Search input
            ComboBox {
                id: pluginComboBox
                model: plugins
                textRole: "name"
            }
        
            TextField {
                id: keywordInput
                selectByMouse: true
                Layout.fillWidth: true
                onAccepted: {
                    currentPlugin.keyword = keywordInput.text;
                    pageSpinBox.value = 1;
                    resultArea.currentIndex = 1;
                }
            }
            Button {
                id: searchButton
                text: qsTr("Search")
                implicitWidth: pageSpinBox.width
                onClicked: {
                    currentPlugin.keyword = keywordInput.text;
                    pageSpinBox.value = 1;
                    resultArea.currentIndex = 1;
                }
            }

            StackLayout {
                id: resultArea
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Description
                Label {
                    id: descriptionLabel
                    text: currentPlugin.description
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    onLinkActivated: {
                        if (link.startsWith('http://') || link.startsWith('https://')) {
                            Qt.openUrlExternally(link);
                        } else {
                            keywordInput.text = link;
                            currentPlugin.keyword = link;
                            pageSpinBox.value = 1;
                            resultArea.currentIndex = 1;
                        }
                    }
                }

                // Search result
                ScrollView {
                    clip: true
                    ListView {
                        model: currentPlugin.resultModel
                        delegate: Rectangle {
                            height: 30
                            width: parent.width
                            color: "transparent"
                    
                            Label { text: modelData; anchors.fill: parent; verticalAlignment: Text.AlignVCenter }
                    
                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: parent.color = Color.listItemHovered
                                onExited: parent.color = "transparent"
                                onDoubleClicked: currentPlugin.openItem(index)
                            }
                        }
                    }
                }
            }

            Button {
                text: qsTr("Back")
                enabled: resultArea.currentIndex === 1
                onClicked: resultArea.currentIndex = 0
            }
            
            Label { text: qsTr("Page: "); horizontalAlignment: Text.AlignRight; Layout.fillWidth: true }
            SpinBox {
                id: pageSpinBox
                from: 1
                to: 100
                value: 1
                implicitWidth: 120
                onValueChanged: {
                    currentPlugin.page = value;
                    resultArea.currentIndex = 1;
                }
            }
        }
    
        Label {
            text: qsTr("<p>No plugins found.</p><p><a href=\"moonplayer:plugin\">Download plugins</a></p>")
            visible: plugins.length === 0
            anchors.centerIn: parent
            onLinkActivated: utils.updateParser()
        }
    }
}

