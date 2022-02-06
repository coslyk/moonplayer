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

// Area to receive mouse events
MouseArea {

    id: mouseArea

    property bool autoHideBars: false
    property var contextMenu
    property var controlbar
    property var titlebar
    property var window

    property real lastMouseX: 0
    property real lastMouseY: 0
    property int activeEdges: 0
    property bool moveable: false

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

    // Enter/exit fullscreen
    onDoubleClicked: {
        if (window.visibility == Window.FullScreen) {
            window.showNormal();
        } else {
            window.showFullScreen();
        }

        // macOS fix
        moveable = false;
    }

    // Mouse pressed
    onPressed: {
        if (mouse.button !== Qt.LeftButton ||
            window.visibility == Window.Maximized ||
            window.visibility == Window.FullScreen)
        {
            return;
        }

        // Set active edges
        activeEdges = 0;
        if (mouseX < 8) activeEdges |= Qt.LeftEdge;
        if (mouseY < 8) activeEdges |= Qt.TopEdge;
        if (mouseX > window.width - 8) activeEdges |= Qt.RightEdge;
        if (mouseY > window.height - 8) activeEdges |= Qt.BottomEdge;

        // Use system native move & resize on Qt >= 5.15
        if (window.startSystemMove !== undefined && Qt.platform.os !== "osx") {
            if (activeEdges === 0) {
                window.startSystemMove();
            } else {
                window.startSystemResize(activeEdges);
            }
        }
            
        // Use software move & resize on Qt < 5.15
        else {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            moveable = (activeEdges === 0);
        }
    }

    // Mouse released
    onReleased: {
        activeEdges = 0;
        moveable = false;
    }

    // Mouse moved
    onPositionChanged: {

        // Show titlebar and controlbar
        controlbar.visible = true;

        if (titlebar !== undefined) {
            titlebar.visible = true;
        }

        if (autoHideBars) {
            timer.restart();
        }
        
        // Set cursor shape
        if (!mouseArea.pressed) {
            if (window.visibility !== Window.Windowed) {
                mouseArea.cursorShape = Qt.ArrowCursor;
            } else if ((mouse.x < 8 && mouse.y < 8) || (mouse.x > width - 8 && mouse.y > height - 8)) {
                mouseArea.cursorShape = Qt.SizeFDiagCursor;
            } else if ((mouse.x < 8 && mouse.y > height - 8) || (mouse.x > width - 8 && mouse.y < 8)) {
                mouseArea.cursorShape = Qt.SizeBDiagCursor;
            } else if (mouse.x < 8 || mouse.x > width - 8) {
                mouseArea.cursorShape = Qt.SizeHorCursor;
            } else if (mouse.y < 8 || mouse.y > height - 8) {
                mouseArea.cursorShape = Qt.SizeVerCursor;
            } else {
                mouseArea.cursorShape = Qt.ArrowCursor;
            }
        }

        // Use system native move & resize on Qt >= 5.15
        if (window.startSystemMove !== undefined && Qt.platform.os !== "osx") {
            return;
        }

        // Ignore when window is maximized, fullscreen or mouse is not pressed
        if (window.visibility == Window.Maximized || window.visibility == Window.FullScreen || !pressed) {
            return;
        }

        // Resize in width
        if (activeEdges & Qt.LeftEdge) {
            window.width -= (mouse.x - lastMouseX);
            window.x += (mouse.x - lastMouseX);
        }
        else if (activeEdges & Qt.RightEdge) {
            window.width += (mouse.x - lastMouseX);
            lastMouseX = mouse.x;
        }

        // Resize in height
        if (activeEdges & Qt.TopEdge) {
            window.height -= (mouse.y - lastMouseY);
            window.y += (mouse.y - lastMouseY);
        }
        else if (activeEdges & Qt.BottomEdge) {
            window.height += (mouse.y - lastMouseY);
            lastMouseY = mouse.y;
        }
        
        if (moveable) {
            window.x += (mouse.x - lastMouseX);
            window.y += (mouse.y - lastMouseY);
        }
    }

    // Auto hide mouse cursor, titlebar and controlbar
    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            if (mouseArea.pressed === true) {
                return;
            }

            mouseArea.cursorShape = Qt.BlankCursor;

            if (!controlbar.contains(controlbar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                controlbar.visible = false;
            }
            
            if (!titlebar.contains(titlebar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                titlebar.visible = false;
            }
        }
    }
}