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
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import MoonPlayer 1.0
import QtQuick.Controls.Material 2.3

Window
{
    id: window

    property var titlebar: titlebar
    
    Material.theme: SkinColor.darkMode ? Material.Dark : Material.Light
    Material.accent: Material.Grey
    
    flags: Qt.Window | Qt.FramelessWindowHint
    
    // Titlebar
    Rectangle {
        id: titlebar
        color: SkinColor.titlebar
        width: parent.width
        height: 28
        z: 100
        anchors.top: parent.top
        anchors.left: parent.left
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
}
