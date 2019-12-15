import QtQuick 2.7
import QtQuick.Controls 2.2

Button {
    property string image: ""
    background: Image { 
        mipmap: true
        source: image
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }
    focusPolicy: Qt.NoFocus
}
