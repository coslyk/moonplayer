import QtQuick 2.7
import QtQuick.Window 2.2

Window
{
    id: window
    
    property alias mouseArea: mouseArea
    property var contextMenu
    
    signal mouseMoved()
    
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
