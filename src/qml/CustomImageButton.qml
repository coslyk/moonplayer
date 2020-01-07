import QtQuick 2.12
import QtQuick.Controls 2.12

Button {
    property string image: ""
    topInset: 0
    bottomInset: 0
    background: Image { 
        mipmap: true
        source: image
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }
    focusPolicy: Qt.NoFocus
}
