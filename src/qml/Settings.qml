import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2 as SystemDialog
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0 as QSettings

Dialog {
    property alias player: playerSettings
    property alias video: videoSettings
    property alias audio: audioSettings
    property alias danmaku: danmakuSettings
    property alias network: networkSettings
    property alias downloader: downloaderSettings
    
    id: window
    width: 600
    height: 400
    
    // Player settings
    QSettings.Settings {
        id: playerSettings
        category: "player"
        property alias url_open_mode: openUrlComboBox.currentIndex
    }
     
    // Video settings
    QSettings.Settings {
        id: videoSettings
        category: "video"
        property alias hwdec: hwdecComboBox.currentIndex
        property alias hwdec_copy_mode: copyModeCheckBox.checked
    }
    
    // Audio settings
    QSettings.Settings {
        id: audioSettings
        category: "audio"
        property alias output: audioOutputComboBox.currentIndex
    }
    
    // Danmaku settings
    QSettings.Settings {
        id: danmakuSettings
        category: "danmaku"
        property alias alpha: alphaSpinBox.value
        property alias dm: dmSpinBox.value
        property alias ds: dsSpinBox.value
        property alias font: fontDialog.font
        property alias font_size: fontSizeSpinBox.value
    }
    
    // Network settings
    QSettings.Settings {
        id: networkSettings
        category: "network"
        property alias proxy_type: proxyModeComboBox.currentIndex
        property alias proxy: proxyInput.text
        property alias proxy_only_for_parsing: proxyParsingOnlyCheckBox.checked
        onProxyChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
        onProxy_typeChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
        onProxy_only_for_parsingChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
    }
    
    // Downloader settings
    QSettings.Settings {
        id: downloaderSettings
        category: "downloader"
        property alias save_to: fileDialog.folder
        property alias max_threads: maxThreadsSpinBox.value
    }
    
    // UI
    GridLayout {
        anchors.fill: parent
        columns: 3
        rowSpacing: 20
        
        // Title
        Text {
            text: qsTr("Settings")
            font.bold: true
            font.pixelSize: 16
            Layout.columnSpan: 3
        }
        
        // Side panel
        ListView {
            id: listView
            width: 120
            Layout.fillHeight: true

            model: ListModel {
                ListElement { tabLabel: qsTr("Player") }
                ListElement { tabLabel: qsTr("Video") }
                ListElement { tabLabel: qsTr("Audio") }
                ListElement { tabLabel: qsTr("Danmaku") }
                ListElement { tabLabel: qsTr("Network") }
                ListElement { tabLabel: qsTr("Downloader") }
                ListElement { tabLabel: qsTr("Shortcuts") }
            }

            delegate: ItemDelegate {
                text: tabLabel
                width: parent.width
                highlighted: ListView.isCurrentItem
                onClicked: listView.currentIndex = index
            }
        }
        
        ToolSeparator {
            Layout.fillHeight: true
        }
        
        StackLayout {
            currentIndex: listView.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            // Player
            GridLayout {
                columns: 2
                columnSpacing: 40
                Text { text: qsTr("When opening an URL:") }
                ComboBox {
                    id: openUrlComboBox
                    model: [ qsTr("Question"), qsTr("Play"), qsTr("Download") ]
                }
            }
            
            // Video
            GridLayout {
                columns: 2
                columnSpacing: 40
                Text { text: qsTr("Hardware decoding:") }
                ComboBox {
                    id: hwdecComboBox
                    model: [ "auto", "vaapi", "vdpau" ]
                }
                StackLayout {
                    Layout.columnSpan: 2
                    Layout.fillHeight: false
                    currentIndex: hwdecComboBox.currentIndex
                    Text { text: qsTr("Choose hardware decoder automatically.") }
                    Text { text: qsTr("Intel hardware decoding on Linux.") }
                    Text { text: qsTr("Nvidia hardware decoding on Linux.") }
                }
                
                MenuSeparator { Layout.columnSpan: 2; Layout.fillWidth: true }
                CheckBox { id: copyModeCheckBox; text: qsTr("Copy mode"); Layout.columnSpan: 2 }
                Text {
                    text: qsTr("This option will make all video filters work under hardware decoding, but it will comsume more hardware resources.")
                    wrapMode: Text.WordWrap
                    Layout.maximumWidth: 400
                    Layout.columnSpan: 2
                }
            }
            
            // Audio
            GridLayout {
                columns: 2
                columnSpacing: 40
                Text { text: qsTr("Audio output:") }
                ComboBox {
                    id: audioOutputComboBox
                    model: [ "auto" ]
                }
            }
            
            // Danmaku
            GridLayout {
                columns: 2
                columnSpacing: 20
                
                Text { text: qsTr("Font:") }
                Button {
                    text: fontDialog.font.family
                    onClicked: fontDialog.open()
                }
                SystemDialog.FontDialog {
                    id: fontDialog
                    title: qsTr("Please choose a font for Danmaku")
                }
                
                Text { text: qsTr("Font size:") + " (*)" }
                SpinBox { id: fontSizeSpinBox; from: 0; to: 100; value: 0 }
                
                Text { text: qsTr("Alpha (%):") }
                SpinBox { id: alphaSpinBox; from: 0; to: 100; value: 100 }
                
                Text { text: qsTr("Duration of scrolling comment display:") + " (*)" }
                SpinBox { id: dmSpinBox; from: 0; to: 100; value: 0 }
                
                Text { text: qsTr("Duration of still comment display:") }
                SpinBox { id: dsSpinBox; from: 0; to: 100; value: 6 }
                
                MenuSeparator { Layout.columnSpan: 2; Layout.fillWidth: true }
                Text { text: "(*): " + qsTr("Set to 0 to let MoonPlayer choose automatically."); Layout.columnSpan: 2 }
            }
            
            // Network
            GridLayout {
                columns: 2
                columnSpacing: 20
                Text { text: qsTr("Proxy mode:") }
                ComboBox {
                    id: proxyModeComboBox
                    model: [ "no", "http", "socks5" ]
                }
                
                Text { text: qsTr("Proxy:") }
                CustomTextInput {
                    id: proxyInput
                    height: proxyModeComboBox.height
                    width: 300
                    textColor: text.match(/^.+:\d+$/) ? "black" : "red"
                    validator: RegExpValidator { regExp: /.+:\d+/ }
                }
                
                Text {
                    text: qsTr("Note: Due to the limitation of mpv, socks5 proxy is not supported by online playing.")
                    wrapMode: Text.WordWrap
                    Layout.maximumWidth: 400
                    Layout.columnSpan: 2
                }
                
                CheckBox {
                    id: proxyParsingOnlyCheckBox
                    text: qsTr("Use proxy only for parsing videos")
                    Layout.columnSpan: 2
                }
            }
            
            // Downloader
            GridLayout {
                columns: 2
                columnSpacing: 40
                
                Text { text: qsTr("Maximun number of threads:") }
                SpinBox { id: maxThreadsSpinBox; from: 0; to: 100; value: 5 }
                
                Text { text: qsTr("Save to:") }
                Button {
                    text: fileDialog.folder.toString().replace("file://", "")
                    onClicked: fileDialog.open()
                }
                SystemDialog.FileDialog {
                    id: fileDialog
                    title: "Please choose a folder"
                    folder: shortcuts.movies
                    selectFolder: true
                }
            }
            
            // Shortcuts
            GridLayout {
                columns: 2
                rowSpacing: 0
                columnSpacing: 40
                Text { text: "Left/Right" }
                Text { text: qsTr("Navigation") }
                Text { text: "Up/Down" }
                Text { text: qsTr("Set volume") }
                Text { text: "Space" }
                Text { text: qsTr("Pause/continue") }
                Text { text: "Return" }
                Text { text: qsTr("Enter/exit fullscreen") }
                Text { text: "Esc" }
                Text { text: qsTr("Exit fullscreen") }
                Text { text: "S" }
                Text { text: qsTr("Screenshot") }
                Text { text: "D" }
                Text { text: qsTr("Switch on/off danmaku") }
                Text { text: "L" }
                Text { text: qsTr("Show playlist") }
                Text { text: "U" }
                Text { text: qsTr("Open URL") }
                Text { text: "W" }
                Text { text: qsTr("Open Explorer") }
                Text { text: "R" }
                Text { text: qsTr("Set speed to default") }
                Text { text: "Ctrl+Left" }
                Text { text: qsTr("Speed down") }
                Text { text: "Ctrl+Right" }
                Text { text: qsTr("Speed up") }
                Text { text: "Ctrl+O" }
                Text { text: qsTr("Open files") }
                Text { text: "Ctrl+," }
                Text { text: qsTr("Open settings") }
            }
        }
    }
}
