pragma Singleton

import QtQuick 2.0
import QtQuick.Controls.Material 2.12

QtObject {
    // theme from settings
    property string theme: "Light"

    // Colors for titlebar
    property color titlebar: "#E6404040"
    property color closeButton: "#fa564d"
    property color maxButton: "#ffbf39"
    property color minButton: "#53cb43"

    // Colors for toolbar
    property color toolbar: "#E6303030"
    property color toolbarText: "lightgrey"

    // Colors for listView
    property color listItemHovered: theme == "Dark" ? "#888888" : "#eeeeee"
    property color listItemSelected: theme == "Dark" ? "steelblue" : "lightsteelblue"
    property color listItemCurrentActive: theme == "Dark" ? "grey" : "lightgrey"
}
