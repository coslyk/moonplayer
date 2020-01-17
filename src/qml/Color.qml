pragma Singleton

import QtQuick 2.7

QtObject {
    // theme from settings
    property string theme: "Light"

    // Colors for titlebar
    property color titlebar: theme == "Light" ? "#d8e0e0e0" : "#E6404040"
    property color closeButton: "#fa564d"
    property color maxButton: "#ffbf39"
    property color minButton: "#53cb43"

    // Colors for toolbar
    property color toolbar: theme == "Light" ? "#d8e0e0e0" : "#E6303030"
    property color toolbarText: theme == "Light" ? "#505050" : "lightgrey"

    // Colors for window background
    property color windowBackground: theme == "Light" ? "#fafafa" : "black"

    // Colors for listView
    property color listItemHovered: theme == "Dark" ? "#888888" : "#eeeeee"
    property color listItemSelected: theme == "Dark" ? "steelblue" : "lightsteelblue"
    property color listItemCurrentActive: theme == "Dark" ? "grey" : "lightgrey"
}
