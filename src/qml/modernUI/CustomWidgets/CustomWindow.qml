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
        anchors.fill: parent
        property real lastMouseX: 0
        property real lastMouseY: 0
        property bool topBorderActived: false
        property bool bottomBorderActived: false
        property bool leftBorderActived: false
        property bool rightBorderActived: false
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
        
        onPressed: {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            leftBorderActived = (mouseX < 8);
            rightBorderActived = (mouseX > window.width - 8);
            topBorderActived = (mouseY < 8);
            bottomBorderActived = (mouseY > window.height - 8);
            moveable = (!topBorderActived && !bottomBorderActived && !leftBorderActived && !rightBorderActived);
        }
        onReleased: {
            leftBorderActived = rightBorderActived = topBorderActived = bottomBorderActived = false;
            moveable = false;
        }
        onMouseXChanged: {
            window.mouseMoved();
            if (window.visibility == Window.Maximized || window.visibility == Window.FullScreen || !pressed)
                return;
            if (leftBorderActived) {
                window.width -= (mouseX - lastMouseX);
                window.x += (mouseX - lastMouseX);
            }
            else if (rightBorderActived) {
                window.width += (mouseX - lastMouseX);
                lastMouseX = mouseX;
            }
            else if (moveable) {
                window.x += (mouseX - lastMouseX);
            }
        }
        onMouseYChanged: {
            window.mouseMoved();
            if (window.visibility == Window.Maximized || window.visibility == Window.FullScreen || !pressed)
                return;
            if (topBorderActived) {
                window.height -= (mouseY - lastMouseY);
                window.y += (mouseY - lastMouseY);
            }
            else if (bottomBorderActived) {
                window.height += (mouseY - lastMouseY);
                lastMouseY = mouseY;
            }
            else if (moveable) {
                window.y += (mouseY - lastMouseY);
            }
        }
        onDoubleClicked: {
            if (window.visibility == Window.FullScreen)
                window.showNormal();
            else
                window.showFullScreen();
            // macOS fix
            moveable = false;
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
