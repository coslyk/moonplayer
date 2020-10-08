import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

Dialog {
    id: window
    
    property alias model: repeater.model
    property alias currentIndex: tabBar.currentIndex
    property alias content: loader.sourceComponent
    
    contentItem: ColumnLayout {
        // Tab bar
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.margins: 5
            Repeater {
                id: repeater
                TabButton {
                    text: tabLabel
                }
            }
        }

        Loader {
            id: loader
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
        }
    }
}
