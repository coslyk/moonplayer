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
 
import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import MoonPlayer 1.0

Dialog {
    id: consoleDialog
    
    width: 600
    height: 450
    title: qsTr("Shortcuts")

    // Center in parent
    x: (parent.width - width) / 2;
    y: (parent.height - height) / 2;

    contentItem: GridLayout {
        columns: 2
        columnSpacing: 10

        Label { text: "Left/Right" }
        Label { text: qsTr("Seek") }
        Label { text: "Up/Down" }
        Label { text: qsTr("Set volume") }
        Label { text: "Space" }
        Label { text: qsTr("Pause/continue") }
        Label { text: "Return" }
        Label { text: qsTr("Enter/exit fullscreen") }
        Label { text: "Esc" }
        Label { text: qsTr("Exit fullscreen") }
        Label { text: "S" }
        Label { text: qsTr("Screenshot") }
        Label { text: "D" }
        Label { text: qsTr("Danmaku options") }
        Label { text: "H" }
        Label { text: qsTr("Switch on/off danmaku") }
        Label { text: "L" }
        Label { text: qsTr("Show playlist") }
        Label { text: "U" }
        Label { text: qsTr("Open URL") }
        Label { text: "V" }
        Label { text: qsTr("Open video options") }
        Label { text: "W" }
        Label { text: qsTr("Open Explorer") }
        Label { text: "Ctrl+O" }
        Label { text: qsTr("Open files") }
        Label { text: "Ctrl+," }
        Label { text: qsTr("Open settings") }
        Label { text: "Ctrl+V" }
        Label { text: qsTr("Paste and open URL") }
    }
}

