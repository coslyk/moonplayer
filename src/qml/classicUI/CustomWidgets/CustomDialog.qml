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

import QtQuick.Window 2.2

Window {
    id: window
    flags: Qt.Dialog
    modality: Qt.WindowModal

    readonly property int suggestedMargins: 8
    readonly property int reservedHeight: 0

    signal accepted()
    signal rejected()

    function accept() {
        window.visible = false;
        accepted();
    }

    function reject() {
        window.visible = false;
        rejected();
    }

    function open() {
        window.visible = true;
    }

    function close() {
        window.visible = false;
    }
}
