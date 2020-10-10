import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import MoonPlayer 1.0

Window
{
    id: window

    property var contextMenu
    property bool autoHideBars: false
    property alias toolbar: toolbarLoader.sourceComponent

    signal mouseMoved()
    
    flags: Qt.Window | Qt.FramelessWindowHint

    // Auto hide mouse cursor, titlebar and toolbar
    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            mouseArea.cursorShape = Qt.BlankCursor;
            if (!toolbarLoader.item.contains(toolbarLoader.item.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                toolbarLoader.item.visible = false;
            }
            if (!titlebar.contains(titlebar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                titlebar.visible = false;
            }
        }
    }
    
    onMouseMoved: {
        // Set cursor shape
        if ((mouseArea.mouseX < 8 && mouseArea.mouseY < 8) || (mouseArea.mouseX > width - 8 && mouseArea.mouseY > height - 8))
            mouseArea.cursorShape = Qt.SizeFDiagCursor;
        else if ((mouseArea.mouseX < 8 && mouseArea.mouseY > height - 8) || (mouseArea.mouseX > width - 8 && mouseArea.mouseY < 8))
            mouseArea.cursorShape = Qt.SizeBDiagCursor;
        else if (mouseArea.mouseX < 8 || mouseArea.mouseX > width - 8)
            mouseArea.cursorShape = Qt.SizeHorCursor;
        else if (mouseArea.mouseY < 8 || mouseArea.mouseY > height - 8)
            mouseArea.cursorShape = Qt.SizeVerCursor;
        else
            mouseArea.cursorShape = Qt.ArrowCursor;

        // Show titlebar and toolbar
        toolbarLoader.item.visible = true;
        titlebar.visible = true;
        if (autoHideBars)
            timer.restart();
    }
    
    // Handle window's resizing and moving
    MouseArea {
        id: mouseArea
        property real lastMouseX: 0
        property real lastMouseY: 0
        property int activeEdges: 0
        property bool moveable: false
        hoverEnabled: true
        anchors.fill: parent
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

        onReleased: {
            activeEdges = 0;
            moveable = false;
        }

        onMouseXChanged: {
            window.mouseMoved();

            // Use system native move & resize on Qt >= 5.15
            if (window.startSystemMove !== undefined && Qt.platform.os !== "osx") {
                return;
            }

            if (window.visibility == Window.Maximized || window.visibility == Window.FullScreen || !pressed) {
                return;
            }

            if (activeEdges & Qt.LeftEdge !== 0) {
                window.width -= (mouseX - lastMouseX);
                window.x += (mouseX - lastMouseX);
            }
            else if (activeEdges & Qt.RightEdge !== 0) {
                window.width += (mouseX - lastMouseX);
                lastMouseX = mouseX;
            }
            else if (moveable) {
                window.x += (mouseX - lastMouseX);
            }
        }
        onMouseYChanged: {
            window.mouseMoved();

            // Use system native move & resize on Qt >= 5.15
            if (window.startSystemMove !== undefined && Qt.platform.os !== "osx") {
                return;
            }

            if (window.visibility == Window.Maximized || window.visibility == Window.FullScreen || !pressed) {
                return;
            }

            if (activeEdges & Qt.TopEdge !== 0) {
                window.height -= (mouseY - lastMouseY);
                window.y += (mouseY - lastMouseY);
            }
            else if (activeEdges & Qt.BottomEdge !== 0) {
                window.height += (mouseY - lastMouseY);
                lastMouseY = mouseY;
            }
            else if (moveable) {
                window.y += (mouseY - lastMouseY);
            }
        }
    }
    
    // Titlebar
    Rectangle {
        id: titlebar
        color: Color.titlebar
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
            background: Rectangle { color: Color.closeButton; radius: 7; anchors.fill: parent }
            onClicked: window.close()
        }
        Button {
            id: maxButton
            width: 14
            height: 14
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: closeButton.left
            anchors.rightMargin: 6
            background: Rectangle { color: Color.maxButton; radius: 7; anchors.fill: parent }
            onClicked: window.visibility == Window.Maximized ? window.showNormal() : window.showMaximized()
        }
        Button {
            id: minButton
            width: 14
            height: 14
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: maxButton.left
            anchors.rightMargin: 6
            background: Rectangle { color: Color.minButton; radius: 7; anchors.fill: parent }
            onClicked: window.showMinimized()
        }
    }

    // Toolbar
    Loader {
        id: toolbarLoader
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        z: 100
    }
}
