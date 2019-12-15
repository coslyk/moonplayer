import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MoonPlayer 1.0

Dialog {
    id: explorer
    width: 600
    height: 400
    
    property QtObject currentPlugin: plugins[pluginComboBox.currentIndex]
    
    GridLayout {
        anchors.fill: parent
        columns: 3
        visible: plugins.length !== 0
        
        // Title
        Text {
            text: qsTr("Explorer")
            font.pixelSize: 16
            font.bold: true
            Layout.columnSpan: 3
        }
        
        // Side panel
        ComboBox {
            id: pluginComboBox
            model: plugins
            textRole: "name"
        }
        
        CustomTextInput {
            id: keywordInput
            height: searchButton.height
            Layout.fillWidth: true
            onAccepted: {
                currentPlugin.keyword = keywordInput.text;
                pageSpinBox.value = 1;
            }
        }
        Button {
            id: searchButton
            text: qsTr("Search")
            implicitWidth: pageSpinBox.width
            onClicked: {
                currentPlugin.keyword = keywordInput.text;
                pageSpinBox.value = 1;
            }
        }
        ScrollView {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            ListView {
                model: currentPlugin.resultModel
                delegate: Rectangle {
                    height: 30
                    width: parent.width
                    
                    Text { text: modelData; anchors.fill: parent; verticalAlignment: Text.AlignVCenter }
                    
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.color = "#eeeeee"
                        onExited: parent.color = "white"
                        onDoubleClicked: currentPlugin.openItem(index)
                    }
                }
            }
        }
        Text { text: qsTr("Page: "); horizontalAlignment: Text.AlignRight; Layout.fillWidth: true; Layout.columnSpan: 2 }
        SpinBox {
            id: pageSpinBox
            from: 1
            to: 100
            value: 1
            onValueChanged: currentPlugin.page = value
        }
    }
    
    Text {
        text: qsTr("<p>No plugins found.</p><p><a href=\"moonplayer:plugin\">Download plugins</a></p>")
        visible: plugins.length === 0
        anchors.centerIn: parent
        onLinkActivated: ykdl.updateParser()
    }
}

