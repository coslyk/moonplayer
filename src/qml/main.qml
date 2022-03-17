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
 
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import MoonPlayer 1.0
import QtQuick.Controls.Material 2.3
import QtQuick.Controls.Universal 2.3

Window
{
    id: window
    visible: true
    minimumWidth: 800
    minimumHeight: 450
    width: 1024
    height: 600
    title: "MoonPlayer"

    // Hide system titlebar when using material UI
    property bool isMaterialUI: Utils.environmentVariable("QT_QUICK_CONTROLS_STYLE").toLowerCase() == "material"
    flags: Qt.Window | (isMaterialUI ? Qt.FramelessWindowHint : 0)

    // Background color
    color: SkinColor.windowBackground
    Material.theme: SkinColor.darkMode ? Material.Dark : Material.Light
    Material.accent: Material.Grey
    Universal.theme: SkinColor.darkMode ? Universal.Dark : Universal.Light

    // Mpv
    MpvObject {
        id: mpv
        anchors.fill: parent
        visible: state !== MpvObject.STOPPED
        volume: volumeSlider.value

        onVideoSizeChanged: {
            if (window.visibility == Window.FullScreen)
                return;
            window.showNormal();
            if (videoSize.width > 0) {
                if (videoSize.width > Screen.desktopAvailableWidth)
                    window.width = Screen.desktopAvailableWidth;
                else
                    window.width = videoSize.width;
            }
            if (videoSize.height > 0) {
                if (videoSize.height > Screen.desktopAvailableHeight)
                    window.height = Screen.desktopAvailableHeight;
                else
                    window.height = videoSize.height;
            }
            window.x = (Screen.width - window.width) / 2;
            window.y = (Screen.height - window.height) / 2;
        }
    }

    // Cover area for mouse event
    CoverArea {
        id: coverArea
        anchors.fill: parent
        autoHideBars: mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING
        contextMenu: contextMenu
        controlbar: controlBar
        sidebar: sidebar
        titlebar: titlebar
        window: window
    }

    // Console dialog
    ConsoleDialog {
        id: consoleDialog

        // Center in parent
        x: (parent.width - width) / 2;
        y: (parent.height - height) / 2;
    }

    // Message dialog
    MessageDialog {
        id: messageDialog

        // Center in parent
        x: (parent.width - width) / 2;
        y: (parent.height - height) / 2;
    }

    // Selection dialog
    SelectionDialog {
        id: selectionDialog

        // Center in parent
        x: (parent.width - width) / 2;
        y: (parent.height - height) / 2;

        Connections {
            target: Dialogs
            onSelectionStarted: {
                selectionDialog.title = title;
                selectionDialog.items = items;
                selectionDialog.visible = true;
            }
        }

        onAccepted: Dialogs.selectionCallback(currentIndex)
    }

    // Text input dialog
    TextInputDialog {
        id: textInputDialog

        // Center in parent
        x: (parent.width - width) / 2;
        y: (parent.height - height) / 2;
    }
    
    // Volume
    Popup {
        id: volumePopup
        width: 40
        height: 120
        Slider {
            id: volumeSlider
            from: 0
            to: 100
            value: 100
            stepSize: 10
            snapMode: Slider.SnapAlways
            anchors.fill: parent
            orientation: Qt.Vertical
        }
    }

    // Open url by Dialog
    OpenUrlDialog {
        id: openUrlDialog

        // Center in parent
        x: (parent.width - width) / 2;
        y: (parent.height - height) / 2;
    
        Connections {
            target: Dialogs
            onOpenUrlStarted: {
                window.raise();
                window.requestActivate();
            }
        }
    }

    // Open file by Dialog
    FileOpenDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        selectMultiple: true
        onAccepted: PlaylistModel.addLocalFiles(fileDialog.fileUrls)
    }

    // Shortcuts dialog
    ShortcutsDialog {
        id: shortcutsDialog
    }

    // Open file by drag
    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: PlaylistModel.addLocalFiles(drop.urls)
    }
    
    // Menu
    Menu {
        id: contextMenu
        width: 160
        padding: 5
        Action { text: qsTr("Open files"); onTriggered: fileDialog.open() }
        Action { text: qsTr("Open URL"); onTriggered: openUrlDialog.visible = true }
        MenuSeparator { padding: 0 }
        Action { text: qsTr("Screenshot"); onTriggered: mpv.screenshot() }
        MenuSeparator { padding: 0 }
        Action { text: qsTr("Explorer"); onTriggered: sidebar.openExplorer() }
        Action { text: qsTr("Downloader"); onTriggered: sidebar.openDownloader() }
        MenuSeparator { padding: 0 }
        Action { text: qsTr("Video options"); onTriggered: sidebar.openVideoOptions() }
        Action { text: qsTr("Subtitle and danmaku"); onTriggered: sidebar.openSubtitles() }
        Action { text: qsTr("Settings"); onTriggered: sidebar.openSettings() }
        MenuSeparator { padding: 0 }
        Action { text: qsTr("Update plugins"); onTriggered: Utils.updateParser() }
        Action { text: qsTr("Shortcuts"); onTriggered: shortcutsDialog.visible = true }
        Action { text: qsTr("Browser Ext."); onTriggered: Qt.openUrlExternally("https://coslyk.github.io/moonplayer.html#browser_extension") }
        Action { text: qsTr("Homepage"); onTriggered: Qt.openUrlExternally("https://coslyk.github.io/moonplayer.html") }
        
        delegate: MenuItem { height: 25 }
    }

    // Titlebar, Controlbar and sidebar
    GridLayout {
        id: mainLayout
        anchors.fill: parent
        rows: 2
        columns: 2
        rowSpacing: 0
        columnSpacing: 0

        // Titlebar
        Rectangle {
            id: titlebar
            color: SkinColor.titlebar
            Layout.fillWidth: true
            Layout.minimumHeight: 28
            Layout.columnSpan: 2
            z: 100
            Button {
                id: closeButton
                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 8
                background: Rectangle { color: SkinColor.closeButton; radius: 7; anchors.fill: parent }
                onClicked: window.close()
            }
            Button {
                id: maxButton
                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: closeButton.left
                anchors.rightMargin: 6
                background: Rectangle { color: SkinColor.maxButton; radius: 7; anchors.fill: parent }
                onClicked: window.visibility == Window.Maximized ? window.showNormal() : window.showMaximized()
            }
            Button {
                id: minButton
                width: 14
                height: 14
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: maxButton.left
                anchors.rightMargin: 6
                background: Rectangle { color: SkinColor.minButton; radius: 7; anchors.fill: parent }
                onClicked: window.showMinimized()
            }
        }

        // Empty item as placeholder
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        // Sidebar
        Sidebar {
            id: sidebar
            Layout.fillHeight: true
            z: 100
            visible: false
            mpv: mpv
            onOpenFileRequested: fileDialog.open()
            onOpenUrlRequested: openUrlDialog.visible = true
        }

        // Controlbar
        ControlBar {
            id: controlBar
            Layout.columnSpan: 2
            Layout.fillWidth: true
            z: 100
            isPlaying: mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING
            time: mpv.time
            duration: mpv.duration
            onPlayPauseButtonClicked: mpv.state == MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
            onStopButtonClicked: mpv.stop()
            onSettingsButtonClicked: sidebar.openSettings()
            onSidebarButtonClicked: sidebar.openPlaylist()
            onExplorerButtonClicked: sidebar.openExplorer()
            onSeekRequested: mpv.seek(time);
            onVolumeButtonClicked: {
                volumePopup.x = mpv.mapFromItem(volumeButton, 0, 0).x;
                volumePopup.y = mpv.mapFromItem(volumeButton, 0, 0).y - volumePopup.height;
                volumePopup.visible = true;
            }
        }
    }
    
    // Handle keyboard event
    Shortcut {
        sequence: "Left"
        onActivated: mpv.seek(mpv.time - 5);
    }
    
    Shortcut {
        sequence: "Right"
        onActivated: mpv.seek(mpv.time + 5);
    }
    
    Shortcut {
        sequence: "Up"
        onActivated: volumeSlider.value += 10;
    }
    
    Shortcut {
        sequence: "Down"
        onActivated: volumeSlider.value -= 10;
    }
    
    Shortcut {
        sequence: "Space"
        onActivated: mpv.state == MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
    }

    Shortcut {
        sequence: "Return"
        onActivated: window.visibility == Window.FullScreen ? window.showNormal() : window.showFullScreen()
    }

    Shortcut {
        sequence: "Esc"
        onActivated: if (window.visibility == Window.FullScreen) showNormal();
    }

    Shortcut {
        sequence: "S"
        onActivated: mpv.screenshot()
    }

    Shortcut {
        sequence: "D"
        onActivated: sidebar.openSubtitles()
    }

    Shortcut {
        sequence: "H"
        onActivated: mpv.subVisible = !mpv.subVisible
    }
    
    Shortcut {
        sequence: "L"
        onActivated: sidebar.openPlaylist()
    }

    Shortcut {
        sequence: "U"
        onActivated: openUrlDialog.visible = true
    }

    Shortcut {
        sequence: "V"
        onActivated: sidebar.openVideoOptions()
    }

    Shortcut {
        sequence: "W"
        onActivated: sidebar.openExplorer()
    }
    
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: fileDialog.open()
    }
    
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: sidebar.openSettings()
    }

    // Hide custom titlebar when not using Material UI
    Component.onCompleted: {
        if (!isMaterialUI)
        {
            titlebar.visible = false;
        }
    }
}
