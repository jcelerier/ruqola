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

#include "notificationdesktopsoundpreferencemodeltest.h"
#include "model/notificationdesktopsoundpreferencemodel.h"
#include <QTest>

QTEST_MAIN(NotificationDesktopSoundPreferenceModelTest)

NotificationDesktopSoundPreferenceModelTest::NotificationDesktopSoundPreferenceModelTest(QObject *parent)
    : QObject(parent)
{
}

void NotificationDesktopSoundPreferenceModelTest::shouldHaveDefaultValue()
{
    NotificationDesktopSoundPreferenceModel w;
    QCOMPARE(w.rowCount(), 8);

    QHash<int, QByteArray> roles;
    roles[NotificationDesktopSoundPreferenceModel::NotificationPreferenceI18n] = QByteArrayLiteral("preferencei18n");
    roles[NotificationDesktopSoundPreferenceModel::NotificationPreference] = QByteArrayLiteral("preference");

    QCOMPARE(w.roleNames(), roles);
    //FIXME QCOMPARE(w.currentPreference(), 0);
}

//TODO add autotest about currentStatus changed !
