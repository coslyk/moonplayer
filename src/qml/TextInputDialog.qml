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
    id: dialog
    width: 350
    height: 160
    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        if (textInput.text !== "") {
            Dialogs.textInputCallback(textInput.text);
        }
    }
    
    Connections {
        target: Dialogs
        onTextInputStarted: {
            dialog.title = title;
            textInput.text = defaultValue;
            dialog.open()
        }
    }

    contentItem: ColumnLayout {

        TextField {
            id: textInput
            selectByMouse: true
            Layout.fillWidth: true
            height: 30
            onAccepted: dialog.accept()
        }
    }
}
