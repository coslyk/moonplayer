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
    // Palette object
    property var sysPalette: SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    // theme from settings, 0 == Classic, 1 == Material, 2 == Win10
    property int theme: 1

    // Dark mode, classic theme must follow system's settings
    property bool darkModeSet: true
    property bool darkMode: theme === 0 ? (sysPalette.window.hsvValue < 0.3) : darkModeSet

    // Colors for titlebar
    property color titlebar: darkMode ? "#E6404040" : "#d8e0e0e0"
    property color closeButton: "#fa564d"
    property color maxButton: "#ffbf39"
    property color minButton: "#53cb43"

    // Colors for controlbar and sidebar
    property color controlbar: theme === 0 ? sysPalette.window : darkMode ? "#d0303030" : "#d0e0e0e0"
    property color sidebar: theme === 0 ? sysPalette.window : darkMode ? "#f0303030" : "#f0e0e0e0"

    // Colors for window background
    property color windowBackground: darkMode || theme === 0 ? "black" : "#fafafa"

    // Colors for listView
    property color listItemHovered: darkMode ? "#888888" : "#eeeeee"
    property color listItemSelected: darkMode ? "steelblue" : "lightsteelblue"
    property color listItemCurrentActive: darkMode ? "grey" : "lightgrey"

    // Colors for file dialog
    property color fileItem: darkMode ? "grey" : "lightgrey"
    property color folderItem: darkMode ? "darkgoldenrod" : "wheat"
}
