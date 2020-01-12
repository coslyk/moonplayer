import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Window
{
    id: window
    
    property alias mouseArea: mouseArea
    property alias titlebar: titlebar
    property var contextMenu
    property bool useSystemFrame: false
    
    flags: Qt.Window | (useSystemFrame ? 0 : Qt.FramelessWindowHint)
    
    signal mouseMoved()
    
    // Set cursor shape
    onMouseMoved: {
        if (useSystemFrame)
            mouseArea.cursorShape = Qt.ArrowCursor;
        else if ((mouseArea.mouseX < 8 && mouseArea.mouseY < 8) || (mouseArea.mouseX > width - 8 && mouseArea.mouseY > height - 8))
            mouseArea.cursorShape = Qt.SizeFDiagCursor;
        else if ((mouseArea.mouseX < 8 && mouseArea.mouseY > height - 8) || (mouseArea.mouseX > width - 8 && mouseArea.mouseY < 8))
            mouseArea.cursorShape = Qt.SizeBDiagCursor;
        else if (mouseArea.mouseX < 8 || mouseArea.mouseX > width - 8)
            mouseArea.cursorShape = Qt.SizeHorCursor;
        else if (mouseArea.mouseY < 8 || mouseArea.mouseY > height - 8)
            mouseArea.cursorShape = Qt.SizeVerCursor;
        else
            mouseArea.cursorShape = Qt.ArrowCursor;
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
            if (useSystemFrame)
                return;
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
        color: "#E6404040"
        width: parent.width
        height: 24
        visible: !useSystemFrame
        z: 100
        anchors.top: parent.top
        anchors.left: parent.left
        Button {
            id: closeButton
            width: 12
            height: 12
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
            background: Rectangle { color: "red"; radius: 6; anchors.fill: parent }
            onClicked: window.close()
        }
        Button {
            id: maxButton
            width: 12
            height: 12
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: closeButton.left
            anchors.rightMargin: 5
            background: Rectangle { color: "green"; radius: 6; anchors.fill: parent }
            onClicked: window.visibility == Window.Maximized ? window.showNormal() : window.showMaximized()
        }
        Button {
            id: minButton
            width: 12
            height: 12
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: maxButton.left
            anchors.rightMargin: 5
            background: Rectangle { color: "yellow"; radius: 6; anchors.fill: parent }
            onClicked: window.showMinimized()
        }
    }
}
