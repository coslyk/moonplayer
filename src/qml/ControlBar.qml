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

Control {
    id: controlBar

    // Color settings
    
    background: Rectangle {
        implicitHeight: 40
        color: SkinColor.controlbar
    }
    
    signal playPauseButtonClicked()
    signal stopButtonClicked()
    signal settingsButtonClicked()
    signal volumeButtonClicked()
    signal sidebarButtonClicked()
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
        spacing: 10
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        ImageButton {
            id: playPauseButton
            image: isPlaying ?
                       (SkinColor.darkMode ? "qrc:/moonplayer_qml/images/pause_lightgrey.png" : "qrc:/moonplayer_qml/images/pause_grey.png") :
                       (SkinColor.darkMode ? "qrc:/moonplayer_qml/images/play_lightgrey.png" : "qrc:/moonplayer_qml/images/play_grey.png")
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: playPauseButtonClicked()
        }

        ImageButton {
            id: stopButton
            image: SkinColor.darkMode ? "qrc:/moonplayer_qml/images/stop_lightgrey.png" : "qrc:/moonplayer_qml/images/stop_grey.png"
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: stopButtonClicked()
        }

        ImageButton {
            id: volumeButton
            image: SkinColor.darkMode ? "qrc:/moonplayer_qml/images/volume_lightgrey.png" : "qrc:/moonplayer_qml/images/volume_grey.png"
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: volumeButtonClicked()
        }
           
        Label {
            id: timeText
            text: toHHMMSS(time)
        }
        
        Slider {
            id: timeSlider
            from: 0
            to: duration
            focusPolicy: Qt.NoFocus
            Layout.fillWidth: true
            Layout.preferredHeight: 16
            onPressedChanged: {
                if (!pressed)  // released
                    seekRequested(value);
            }
        }
        
        Label {
            id: durationText
            text: toHHMMSS(duration)
        }

        ImageButton {
            id: explorerButton
            image: SkinColor.darkMode ? "qrc:/moonplayer_qml/images/net_lightgrey.png" : "qrc:/moonplayer_qml/images/net_grey.png"
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: explorerButtonClicked()
        }

        ImageButton {
            id: settingsButton
            image: SkinColor.darkMode ? "qrc:/moonplayer_qml/images/settings_lightgrey.png" : "qrc:/moonplayer_qml/images/settings_grey.png"
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: settingsButtonClicked()
        }

        ImageButton {
            id: sidebarButton
            image: SkinColor.darkMode ? "qrc:/moonplayer_qml/images/playlist_lightgrey.png" : "qrc:/moonplayer_qml/images/playlist_grey.png"
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            onClicked: sidebarButtonClicked()
        }
    }

    onTimeChanged: {
        if (!timeSlider.pressed)
            timeSlider.value = time;
    }
}
