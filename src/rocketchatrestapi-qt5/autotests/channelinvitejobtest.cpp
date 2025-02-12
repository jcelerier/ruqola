/*
   Copyright (c) 2018-2019 Montel Laurent <montel@kde.org>

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

#include "channelinvitejobtest.h"
#include "channels/channelinvitejob.h"
#include "ruqola_restapi_helper.h"
#include <QJsonDocument>
#include <QTest>
QTEST_GUILESS_MAIN(ChannelInviteJobTest)
using namespace RocketChatRestApi;
ChannelInviteJobTest::ChannelInviteJobTest(QObject *parent)
    : QObject(parent)
{
}

void ChannelInviteJobTest::shouldHaveDefaultValue()
{
    ChannelInviteJob job;
    verifyDefaultValue(&job);
    QVERIFY(job.inviteUserId().isEmpty());
    QVERIFY(job.inviteUserName().isEmpty());
    QVERIFY(job.roomId().isEmpty());
    QVERIFY(!job.hasQueryParameterSupport());
}

void ChannelInviteJobTest::shouldGenerateRequest()
{
    ChannelInviteJob job;
    QNetworkRequest request = QNetworkRequest(QUrl());
    verifyAuthentication(&job, request);
    QCOMPARE(request.url(), QUrl(QStringLiteral("http://www.kde.org/api/v1/channels.invite")));
    QCOMPARE(request.header(QNetworkRequest::ContentTypeHeader).toString(), QStringLiteral("application/json"));
}

void ChannelInviteJobTest::shouldGenerateUserIdJson()
{
    ChannelInviteJob job;
    const QString roomId = QStringLiteral("foo1");
    const QString userId = QStringLiteral("topic1");
    job.setRoomId(roomId);
    job.setInviteUserId(userId);
    QCOMPARE(job.json().toJson(QJsonDocument::Compact), QStringLiteral("{\"roomId\":\"%2\",\"userId\":\"%1\"}").arg(userId, roomId).toLatin1());
}

void ChannelInviteJobTest::shouldGenerateUserNameJson()
{
    ChannelInviteJob job;
    const QString roomId = QStringLiteral("foo1");
    const QString userName = QStringLiteral("topic1");
    job.setRoomId(roomId);
    job.setInviteUserName(userName);
    QCOMPARE(job.json().toJson(QJsonDocument::Compact), QStringLiteral("{\"roomId\":\"%2\",\"userName\":\"%1\"}").arg(userName, roomId).toLatin1());
}
