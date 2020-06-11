import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0

Rectangle {
    id: toolBar
    color: Color.toolbar
    height: 32
    
    signal playPauseButtonClicked()
    signal stopButtonClicked()
    signal settingsButtonClicked()
    signal volumeButtonClicked()
    signal playlistButtonClicked()
    signal explorerButtonClicked()
    signal seekRequested(int time)

    property bool isPlaying: false
    property int time: 0
    property int duration: 0
    property alias volumeButton: volumeButton

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

    RowLayout {
        CustomImageButton {
            id: playPauseButton
            image: isPlaying ?
                   (Color.theme === "Light" ? "qrc:/images/pause_grey.png" : "qrc:/images/pause_lightgrey.png") :
                   (Color.theme === "Light" ? "qrc:/images/play_grey.png" : "qrc:/images/play_lightgrey.png")
            width: 16
            height: 16
            onClicked: playPauseButtonClicked()
        }

        CustomImageButton {
            id: stopButton
            image: (Color.theme === "Light" ? "qrc:/images/stop_grey.png" : "qrc:/images/stop_lightgrey.png")
            width: 16
            height: 16
            onClicked: stopButtonClicked()
        }

        CustomImageButton {
            id: settingsButton
            image: (Color.theme === "Light" ? "qrc:/images/settings_grey.png" : "qrc:/images/settings_lightgrey.png")
            width: 16
            height: 16
            onClicked: settingsButtonClicked()
        }

        CustomImageButton {
            id: volumeButton
            image: (Color.theme === "Light" ? "qrc:/images/volume_grey.png" : "qrc:/images/volume_lightgrey.png")
            width: 16
            height: 16
            onClicked: volumeButtonClicked()
        }
           
        Label {
            id: timeText
            text: toHHMMSS(time)
            color: Color.toolbarText
        }
        
        Slider {
            id: timeSlider
            from: 0
            to: duration
            focusPolicy: Qt.NoFocus
            onPressedChanged: {
                if (!pressed)  // released
                    seekRequested(value);
            }
        }
        
        Label {
            id: durationText
            text: toHHMMSS(duration)
            color: Color.toolbarText
        }

        CustomImageButton {
            id: playlistButton
            image: (Color.theme === "Light" ? "qrc:/images/playlist_grey.png" : "qrc:/images/playlist_lightgrey.png")
            width: 16
            height: 16
            onClicked: playlistButtonClicked()
        }

        CustomImageButton {
            id: explorerButton
            image: (Color.theme === "Light" ? "qrc:/images/net_grey.png" : "qrc:/images/net_lightgrey.png")
            width: 16
            height: 16
            onClicked: explorerButtonClicked()
        }
    }

    onTimeChanged: {
        if (!timeSlider.pressed)
            timeSlider.value = time;
    }
}
