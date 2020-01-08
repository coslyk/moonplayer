import QtQuick 2.12
import QtQuick.Controls.Material 2.12


Rectangle {
    signal accepted()
    property alias text: textInput.text
    property alias textColor: textInput.color
    property alias validator: textInput.validator
    color: Material.theme == Material.Dark ? "#505050" : "#e6e6e6"
    TextInput {
        id: textInput
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        onAccepted: parent.accepted()
    }
}
