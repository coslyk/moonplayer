/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */
 
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
