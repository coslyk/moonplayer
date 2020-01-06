import QtQuick 2.7
import QtQuick.Window 2.0
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.0
import QtQml 2.2
import MoonPlayer 1.0

CustomWindow
{
    id: window
    visible: true
    minimumWidth: 800
    minimumHeight: 450
    
    function toHHMMSS(seconds) {
        var hours = Math.floor(seconds / 3600);
        seconds -= hours*3600;
        var minutes = Math.floor(seconds / 60);
        seconds -= minutes*60;

        if (hours   < 10) {hours   = "0"+hours;}
        if (minutes < 10) {minutes = "0"+minutes;}
        if (seconds < 10) {seconds = "0"+seconds;}
        return hours+':'+minutes+':'+seconds;
    }
    
    // Mpv
    MpvObject {
        id: mpv
        anchors.fill: parent
        volume: volumeSlider.value
        onTimeChanged: {
            if (!timeSlider.pressed)
                timeSlider.value = time;
        }
        onStopped: {
            if (!stoppedByUser)
                playlistModel.playNextItem();
        }
        onVideoSizeChanged: {
            if (window.visibility == Window.FullScreen)
                return;
            window.showNormal();
            if (videoSize.width != 0) {
                if (videoSize.width > Screen.desktopAvailableWidth)
                    window.width = Screen.desktopAvailableWidth;
                else
                    window.width = videoSize.width;
            }
            if (videoSize.height != 0) {
                if (videoSize.height > Screen.desktopAvailableHeight)
                    window.height = Screen.desktopAvailableHeight;
                else
                    window.height = videoSize.height;
            }
            window.x = (Screen.width - window.width) / 2;
            window.y = (Screen.height - window.height) / 2;
        }
    }
    
    // Select subtitles
    SelectionDialog {
        id: subtitleSelectionDialog
        title: qsTr("Select subtitles")
        items: mpv.subtitles
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        onAccepted: mpv.sid = currentIndex
    }
    
    // Add subtitles
    FileDialog {
        id: addSubtitleDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        onAccepted: mpv.addSubtitle(addSubtitleDialog.fileUrl)
    }
    
    
    // Playlist
    Playlist {
        id: playlist
        x: window.width / 2 + 200
        y: window.height - 410
        onOpenFileRequested: fileDialog.open()
        onOpenUrlRequested: openUrlDialog.open()
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
            handle.implicitWidth: 20
            handle.implicitHeight: 20
        }
    }
    
    // Settings
    Settings {
        id: settings
        x: (window.width - settings.width) / 2;
        y: (window.height - settings.height) / 2;
    }
    
    // Explorer
    Explorer {
        id: explorer
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    }
    
    // Downloader
    Downloader {
        id: downloader
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    }

    // Open url by Dialog
    OpenUrlDialog {
        id: openUrlDialog
        x: (window.width - width) / 2
        y: (window.height - height) / 2
    
        Connections {
            target: playlistModel
            onUrlDialogRequested: {
                window.raise();
                window.requestActivate();
            }
        }
    }

    // Open file by Dialog
    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home
        selectMultiple: true
        onAccepted: playlistModel.addLocalFiles(fileDialog.fileUrls)
    }

    // Open file by drag
    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: playlistModel.addLocalFiles(drop.urls)
    }
    
    // Menu
    contextMenu: Menu {
        padding: 5
        width: 120
        MenuItem { text: qsTr("Open files"); onTriggered: fileDialog.open(); height: 25 }
        MenuItem { text: qsTr("Open URL"); onTriggered: openUrlDialog.open(); height: 25 }
        MenuItem { text: qsTr("Playlist"); onTriggered: playlist.open(); height: 25 }
        MenuSeparator { padding: 0 }
        MenuItem { text: qsTr("Show danmaku"); onTriggered: mpv.subVisible = !mpv.subVisible; height: 25 }
        MenuItem { text: qsTr("Add subtitle"); onTriggered: addSubtitleDialog.open(); height: 25 }
        MenuItem { text: qsTr("Select subtitles"); onTriggered: subtitleSelectionDialog.open(); height: 25 }
        MenuItem { text: qsTr("Screenshot"); onTriggered: mpv.screenshot(); height: 25 }
        MenuSeparator { padding: 0 }
        MenuItem { text: qsTr("Downloader"); onTriggered: downloader.open(); height: 25 }
        MenuItem { text: qsTr("Settings"); onTriggered: settings.open(); height: 25 }
        MenuItem { text: qsTr("Update plugins"); onTriggered: ykdl.updateParser(); height: 25 }
        MenuItem { text: qsTr("Browser Ext."); onTriggered: Qt.openUrlExternally("https://github.com/coslyk/moonplayer/wiki/BrowserExtension"); height: 25 }
        MenuItem { text: qsTr("Homepage"); onTriggered: Qt.openUrlExternally("https://github.com/coslyk/moonplayer"); height: 25 }
    }
    
    // Toolbar
    Rectangle {
        id: toolBar
        width: 450
        height: 70
        radius: 8
        color: "#E6303030"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        
        CustomImageButton {
            id: playPauseButton
            image: (mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING) ? "qrc:/images/pause.png" : "qrc:/images/play.png"
            width: 16
            height: 16
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: mpv.state == MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
        }
        
        CustomImageButton {
            id: stopButton
            image: "qrc:/images/stop.png"
            width: 16
            height: 16
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: mpv.stop()
        }
        
        CustomImageButton {
            id: settingsButton
            image: "qrc:/images/settings.png"
            width: 16
            height: 16
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: settings.open()
        }
        
        
        CustomImageButton {
            id: volumeButton
            image: "qrc:/images/volume.png"
            width: 16
            height: 16
            anchors.left: settingsButton.right
            anchors.leftMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: {
                volumePopup.x = mpv.mapFromItem(volumeButton, 0, 0).x;
                volumePopup.y = mpv.mapFromItem(volumeButton, 0, 0).y - volumePopup.height;
                volumePopup.open();
            }
        }
        
        CustomImageButton {
            id: playlistButton
            image: "qrc:/images/playlist.png"
            width: 16
            height: 16
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: playlist.open()
        }
        
        CustomImageButton {
            id: downloaderButton
            image: "qrc:/images/net.png"
            width: 16
            height: 16
            anchors.right: playlistButton.left
            anchors.rightMargin: 8
            anchors.top: parent.top
            anchors.topMargin: 16
            onClicked: explorer.open()
        }
        
        Text {
            id: timeText
            text: toHHMMSS(mpv.time)
            color: "lightgrey"
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
        }
        
        Text {
            id: durationText
            text: toHHMMSS(mpv.duration)
            color: "lightgrey"
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
        }
        
        CustomSlider {
            id: timeSlider
            from: 0
            to: mpv.duration
            focusPolicy: Qt.NoFocus
            anchors.left: timeText.right
            anchors.right: durationText.left
            anchors.verticalCenter: timeText.verticalCenter
            onPressedChanged: {
                if (!pressed)  // released
                    mpv.seek(value);
            }
        }
    }
    
    // Auto hide mouse cursor, titlebar and toolbar
    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            mouseArea.cursorShape = Qt.BlankCursor;
            if (!toolBar.contains(toolBar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                toolBar.visible = false;
            }
            if (!titlebar.contains(titlebar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                titlebar.visible = false;
            }
        }
    }
    
    onMouseMoved: {
        toolBar.visible = true;
        titlebar.visible = true;
        if (mpv.state == MpvObject.VIDEO_PLAYING || mpv.state == MpvObject.TV_PLAYING)
        {
            timer.restart();
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
        onActivated: mpv.subVisible = !mpv.subVisible;
    }
    
    Shortcut {
        sequence: "L"
        onActivated: playlist.open()
    }

    Shortcut {
        sequence: "R"
        onActivated: mpv.speed = 1
    }

    Shortcut {
        sequence: "U"
        onActivated: openUrlDialog.open()
    }

    Shortcut {
        sequence: "W"
        onActivated: explorer.open()
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
        onActivated: settings.open()
    }
}
