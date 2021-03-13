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
import QtQuick.Controls.Material 2.3
import MoonPlayer 1.0

Rectangle {
    id: toolBar
    color: SkinColor.toolbar
    width: 450
    height: 70
    radius: 8
    
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
        
    CustomImageButton {
        id: playPauseButton
        image: isPlaying ?
                   (SkinColor.theme === "Light" ? "qrc:/images/pause_grey.png" : "qrc:/images/pause_lightgrey.png") :
                   (SkinColor.theme === "Light" ? "qrc:/images/play_grey.png" : "qrc:/images/play_lightgrey.png")
        width: 16
        height: 16
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: playPauseButtonClicked()
    }
        
    CustomImageButton {
        id: stopButton
        image: (SkinColor.theme === "Light" ? "qrc:/images/stop_grey.png" : "qrc:/images/stop_lightgrey.png")
        width: 16
        height: 16
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: stopButtonClicked()
    }
        
    CustomImageButton {
        id: settingsButton
        image: (SkinColor.theme === "Light" ? "qrc:/images/settings_grey.png" : "qrc:/images/settings_lightgrey.png")
        width: 16
        height: 16
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: settingsButtonClicked()
    }
        
        
    CustomImageButton {
        id: volumeButton
        image: (SkinColor.theme === "Light" ? "qrc:/images/volume_grey.png" : "qrc:/images/volume_lightgrey.png")
        width: 16
        height: 16
        anchors.left: settingsButton.right
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: volumeButtonClicked()
    }
        
    CustomImageButton {
        id: playlistButton
        image: (SkinColor.theme === "Light" ? "qrc:/images/playlist_grey.png" : "qrc:/images/playlist_lightgrey.png")
        width: 16
        height: 16
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: playlistButtonClicked()
    }
        
    CustomImageButton {
        id: explorerButton
        image: (SkinColor.theme === "Light" ? "qrc:/images/net_grey.png" : "qrc:/images/net_lightgrey.png")
        width: 16
        height: 16
        anchors.right: playlistButton.left
        anchors.rightMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 16
        onClicked: explorerButtonClicked()
    }
        
    Label {
        id: timeText
        text: toHHMMSS(time)
        color: SkinColor.toolbarText
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
    }
        
    Label {
        id: durationText
        text: toHHMMSS(duration)
        color: SkinColor.toolbarText
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
    }
        
    Slider {
        id: timeSlider
        from: 0
        to: duration
        focusPolicy: Qt.NoFocus
        anchors.left: timeText.right
        anchors.right: durationText.left
        anchors.verticalCenter: timeText.verticalCenter
        Material.theme: SkinColor.theme === "Light" ? Material.Light : Material.Dark
        onPressedChanged: {
            if (!pressed)  // released
                seekRequested(value);
        }
    }

    onTimeChanged: {
        if (!timeSlider.pressed)
            timeSlider.value = time;
    }
}
