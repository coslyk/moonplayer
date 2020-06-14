import QtQuick 2.7
import QtQuick.Controls 2.3

Dialog {
    id: window

    // Center in parent
    x: (parent.width - width) / 2;
    y: (parent.height - height) / 2;

    padding: 0

    header: Label {
        text: title
        padding: 20
        font.bold: true
        font.pixelSize: 16
        height: 20
    }
}