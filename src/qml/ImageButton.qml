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

Button {
    id: button
    property string image: ""
    property string hoverImage: ""

    hoverEnabled: true

    background: Image { 
        mipmap: true
        source: button.hovered ? hoverImage : image
        sourceSize.width: parent.width
        sourceSize.height: parent.height
    }
    focusPolicy: Qt.NoFocus

    // topInset and bottomInset is available after Qt5.12
    Component.onCompleted: {
        if (button.topInset !== undefined)
        {
            button.topInset = 0;
            button.bottomInset = 0;
        }
    }
}
