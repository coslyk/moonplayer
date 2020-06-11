import QtQuick 2.7
import QtQuick.Window 2.2

Window
{
    id: window

    property var contextMenu
    property bool autoHideBars: false
    property alias toolbar: toolbarLoader.sourceComponent
    
    signal mouseMoved()
    
    // Auto hide mouse cursor and toolbar
    
    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            mouseArea.cursorShape = Qt.BlankCursor;
            if (!toolbarLoader.item.contains(toolbarLoader.item.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
            {
                toolbarLoader.item.visible = false;
            }
        }
    }

    onMouseMoved: {
        // Show toolbar
        toolbarLoader.item.visible = true;
        if (autoHideBars)
            timer.restart();
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

    // Toolbar
    Loader {
        id: toolbarLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 70
        z: 100
    }
}
