/*
 * Copyright (C) 2019 Laurent Montel <montel@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.9

import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.12
QQC2.Label {
    id: threadLabel
    signal openThread()

    width: parent.width

    renderType: Text.NativeRendering
    textFormat: Text.RichText

    color: "red" //Convert to kirigami color
    text: i18np("1 reply", "%1 replies", i_tcount)
    wrapMode: QQC2.Label.Wrap
    visible: i_tcount > 0
    MouseArea {
        anchors.fill: parent
        onClicked: {
            threadLabel.openThread();
        }
    }
}
