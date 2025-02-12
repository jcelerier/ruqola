/*
   Copyright (c) 2019 Montel Laurent <montel@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.9
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

RowLayout {
    id: searchLabel
    signal loadMoreElements()
    property string labelText
    property int numberOfElements: 0
    property bool hasFullList: false
    QQC2.Label {
        visible: searchLabel.labelText.length > 0
        text: searchLabel.labelText
        Component.onCompleted: {
            font.italic = true
            font.bold = true
        }
    }
    QQC2.Label {
        visible: numberOfElements > 0 && !hasFullList
        text: i18n("(Click here for Loading more...)")
        textFormat: Text.PlainText
        color: Kirigami.Theme.linkColor
        Component.onCompleted: {
            font.italic = true
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                searchLabel.loadMoreElements()
            }
        }
    }
}
