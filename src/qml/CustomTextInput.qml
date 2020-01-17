import QtQuick 2.7
import QtQuick.Controls.Material 2.3


Rectangle {
    signal accepted()
    property alias text: textInput.text
    property alias textColor: textInput.color
    property alias validator: textInput.validator
    
    color: Material.theme == Material.Dark ? "#505050" : "#e6e6e6"
    height: 34
    clip: true
    
    TextInput {
        id: textInput
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width
        color: Material.theme == Material.Dark ? "white" : "black"
        onAccepted: parent.accepted()
    }
}
