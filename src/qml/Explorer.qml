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
import QtQuick.Layouts
import Qt.labs.settings as QSettings
import com.github.coslyk.moonplayer

Item {
    id: explorer
    
    // Remember the last used plugin
    QSettings.Settings {
        category: "explorer"
        property alias last_plugin: pluginComboBox.currentIndex
    }
    
    property QtObject currentPlugin: plugins[pluginComboBox.currentIndex]

    GridLayout {
        columns: 3
        visible: plugins.length !== 0
        anchors.fill: parent

        Label {
            text: qsTr("Explorer")
            font.bold: true
            Layout.columnSpan: 3
        }
        
        // Search input
        ComboBox {
            id: pluginComboBox
            model: plugins
            textRole: "name"
            implicitWidth: 80
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
                onLinkActivated: (link) => {
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
                            onEntered: parent.color = SkinColor.listItemHovered
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

        SpinBox {
            id: pageSpinBox
            from: 1
            to: 100
            value: 1
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
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
        onLinkActivated: Utils.updateParser()
    }
}

