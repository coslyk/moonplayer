import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

Dialog {
    id: window
    
    property alias model: listView.model
    property alias currentIndex: listView.currentIndex
    property alias content: loader.sourceComponent

    // Center in parent
    x: (parent.width - width) / 2;
    y: (parent.height - height) / 2;
    
    RowLayout {
        anchors.fill: parent
        
        // Side panel
        ListView {
            id: listView
            width: 120
            Layout.fillHeight: true

            delegate: ItemDelegate {
                text: tabLabel
                width: parent.width
                highlighted: ListView.isCurrentItem
                onClicked: listView.currentIndex = index
            }
        }
        
        ToolSeparator {
            Layout.fillHeight: true
        }

        Loader {
            id: loader
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
