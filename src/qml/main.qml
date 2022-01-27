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
import QtQuick.Window 2.2
import MoonPlayer 1.0
import CustomWidgets 1.0

CustomWindow
{
    id: window
    visible: true
    minimumWidth: 800
    minimumHeight: 450
    autoHideBars: mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING
    title: "MoonPlayer"

    // Background color
    color: SkinColor.windowBackground
    
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
        onStateChanged: {
            if (mpv.state === MpvObject.VIDEO_PLAYING || mpv.state === MpvObject.TV_PLAYING)
                explorer.visible = false
        }
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
    }

    // Selection dialog
    SelectionDialog {
        id: selectionDialog

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
    }
    
    // Select subtitles
    SelectionDialog {
        id: subtitleSelectionDialog

        title: qsTr("Select subtitles")
        items: mpv.subtitles
        onAccepted: mpv.setProperty("sid", currentIndex)
    }
    
    // Add subtitles
    FileOpenDialog {
        id: addSubtitleDialog
        title: qsTr("Please choose a file")
        onAccepted: mpv.addSubtitle(addSubtitleDialog.fileUrl)
    }

    // Select audio tracks
    SelectionDialog {
        id: audioTrackSelectionDialog

        title: qsTr("Select audio tracks")
        items: mpv.audioTracks
        onAccepted: mpv.setProperty("aid", currentIndex)
    }
    
    // Video options
    VideoOptionsDialog {
        id: videoOptionsDialog
        mpvObject: mpv
    }

    // Danmaku options
    DanmakuOptionsDialog {
        id: danmakuOptionsDialog
        mpvObject: mpv
    }
    
    
    // Playlist
    Playlist {
        id: playlist
        x: playlistX
        y: playlistY
        onOpenFileRequested: fileDialog.open()
        onOpenUrlRequested: openUrlDialog.visible = true
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
    
    // Settings
    Settings {
        id: settings
    }
    
    // Explorer
    Explorer {
        id: explorer
    }
    
    // Downloader
    Downloader {
        id: downloader
    }

    // Open url by Dialog
    OpenUrlDialog {
        id: openUrlDialog
    
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

    // Open file by drag
    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: PlaylistModel.addLocalFiles(drop.urls)
    }
    
    // Menu
    contextMenu: Menu {
        width: 150
        padding: 5
        Action { text: qsTr("Open files"); onTriggered: fileDialog.open() }
        Action { text: qsTr("Open URL"); onTriggered: openUrlDialog.visible = true }
        Action { text: qsTr("Explorer"); onTriggered: explorer.visible = true }
        MenuSeparator { padding: 0 }
        Menu {
            title: qsTr("Video")
            width: 150
            Action { text: qsTr("Options"); onTriggered: videoOptionsDialog.visible = true }
            MenuSeparator { padding: 0 }
            Action { text: qsTr("Default"); onTriggered: mpv.setProperty("video-aspect", 0) }
            Action { text: qsTr("4:3"); onTriggered: mpv.setProperty("video-aspect", 4 / 3) }
            Action { text: qsTr("16:9"); onTriggered: mpv.setProperty("video-aspect", 16 / 9) }
            Action { text: qsTr("16:10"); onTriggered: mpv.setProperty("video-aspect", 16 / 10) }
            Action { text: qsTr("1.85:1"); onTriggered: mpv.setProperty("video-aspect", 1.85) }
            Action { text: qsTr("2.35:1"); onTriggered: mpv.setProperty("video-aspect", 2.35) }
            delegate: MenuItem { height: 25 }
        }
        Menu {
            title: qsTr("Audio")
            width: 150
            Action { text: qsTr("Select"); onTriggered: audioTrackSelectionDialog.visible = true }
            delegate: MenuItem { height: 25 }
        }
        Menu {
            title: qsTr("Subtitle")
            width: 150
            Action { text: qsTr("Visible"); onTriggered: mpv.subVisible = !mpv.subVisible }
            Action { text: qsTr("Add"); onTriggered: addSubtitleDialog.open() }
            Action { text: qsTr("Select"); onTriggered: subtitleSelectionDialog.visible = true }
            delegate: MenuItem { height: 25 }
        }
        Menu {
            title: qsTr("Speed")
            width: 150
            Action { text: qsTr("Up"); onTriggered: if (mpv.speed < 2) mpv.speed += 0.25 }
            Action { text: qsTr("Down"); onTriggered: if (mpv.speed > 0.5) mpv.speed -= 0.25 }
            Action { text: qsTr("Reset"); onTriggered: mpv.speed = 1 }
            delegate: MenuItem { height: 25 }
        }
        Action { text: qsTr("Danmaku"); onTriggered: danmakuOptionsDialog.visible = true }
        Action { text: qsTr("Screenshot"); onTriggered: mpv.screenshot() }
        MenuSeparator { padding: 0 }
        Action { text: qsTr("Downloader"); onTriggered: downloader.visible = true }
        Action { text: qsTr("Settings"); onTriggered: settings.visible = true }
        Action { text: qsTr("Update plugins"); onTriggered: Utils.updateParser() }
        Action { text: qsTr("Browser Ext."); onTriggered: Qt.openUrlExternally("https://coslyk.github.io/moonplayer.html#browser_extension") }
        Action { text: qsTr("Homepage"); onTriggered: Qt.openUrlExternally("https://coslyk.github.io/moonplayer.html") }
        
        delegate: MenuItem { height: 25 }
    }

    // ControlBar
    ControlBar {
        id: controlBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 100
        isPlaying: mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING
        time: mpv.time
        duration: mpv.duration
        onPlayPauseButtonClicked: mpv.state == MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
        onStopButtonClicked: mpv.stop()
        onSettingsButtonClicked: settings.visible = true
        onPlaylistButtonClicked: playlist.visible = true
        onExplorerButtonClicked: explorer.visible = true
        onSeekRequested: mpv.seek(time);
        onVolumeButtonClicked: {
            volumePopup.x = mpv.mapFromItem(volumeButton, 0, 0).x;
            volumePopup.y = mpv.mapFromItem(volumeButton, 0, 0).y - volumePopup.height;
            volumePopup.visible = true;
        }
    }
    controlbar: controlBar
    
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
        onActivated: danmakuOptionsDialog.visible = true
    }

    Shortcut {
        sequence: "H"
        onActivated: mpv.subVisible = !mpv.subVisible
    }
    
    Shortcut {
        sequence: "L"
        onActivated: playlist.visible = true
    }

    Shortcut {
        sequence: "R"
        onActivated: mpv.speed = 1
    }

    Shortcut {
        sequence: "U"
        onActivated: openUrlDialog.visible = true
    }

    Shortcut {
        sequence: "W"
        onActivated: explorer.visible = true
    }

    Shortcut {
        sequence: "Ctrl+Left"
        onActivated: if (mpv.speed > 0.5) mpv.speed -= 0.25;
    }

    Shortcut {
        sequence: "Ctrl+Right"
        onActivated: if (mpv.speed < 2) mpv.speed += 0.25;
    }
    
    Shortcut {
        sequence: "Ctrl+O"
        onActivated: fileDialog.open()
    }
    
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: settings.visible = true
    }
}
