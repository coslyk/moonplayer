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

Window
{
    id: window

    property var contextMenu
    property bool autoHideBars: false
    property var controlbar
    
    signal mouseMoved()
    
    // Auto hide mouse cursor and controlbar
    
    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            mouseArea.cursorShape = Qt.BlankCursor;
            if (!controlbar.contains(controlbar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                controlbar.visible = false;
            }
        }
    }

    onMouseMoved: {
        // Show controlbar
        controlbar.visible = true;
        if (autoHideBars)
            timer.restart();

        // Show cursor
        mouseArea.cursorShape = Qt.ArrowCursor;
    }

    // Handle window's resizing and moving
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        
        // ContextMenu
        onClicked: {
            if (mouse.button === Qt.RightButton)
            {
                contextMenu.x = mouse.x;
                contextMenu.y = mouse.y;
                contextMenu.open();
            }
        }

        onDoubleClicked: {
            if (window.visibility == Window.FullScreen)
                window.showNormal();
            else
                window.showFullScreen();
        }

        onMouseXChanged: window.mouseMoved()
        onMouseYChanged: window.mouseMoved()
    }
}
