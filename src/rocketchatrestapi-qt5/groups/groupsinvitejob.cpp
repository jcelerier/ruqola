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

#include "groupsinvitejob.h"

#include "rocketchatqtrestapi_debug.h"
#include "restapimethod.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
using namespace RocketChatRestApi;
GroupsInviteJob::GroupsInviteJob(QObject *parent)
    : RestApiAbstractJob(parent)
{
}

GroupsInviteJob::~GroupsInviteJob()
{
}

bool GroupsInviteJob::start()
{
    if (!canStart()) {
        deleteLater();
        return false;
    }
    const QByteArray baPostData = json().toJson(QJsonDocument::Compact);
    addLoggerInfo("GroupsInviteJob::start: " + baPostData);
    QNetworkReply *reply = mNetworkAccessManager->post(request(), baPostData);
    connect(reply, &QNetworkReply::finished, this, &GroupsInviteJob::slotInviteGroupsFinished);
    return true;
}

void GroupsInviteJob::slotInviteGroupsFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply) {
        const QByteArray data = reply->readAll();
        const QJsonDocument replyJson = QJsonDocument::fromJson(data);
        const QJsonObject replyObject = replyJson.object();

        if (replyObject[QStringLiteral("success")].toBool()) {
            addLoggerInfo(QByteArrayLiteral("GroupsInviteJob: success: ") + replyJson.toJson(QJsonDocument::Indented));
            Q_EMIT inviteGroupsDone();
        } else {
            emitFailedMessage(replyObject);
            addLoggerWarning(QByteArrayLiteral("GroupsInviteJob: Problem: ") + replyJson.toJson(QJsonDocument::Indented));
        }
        reply->deleteLater();
    }
    deleteLater();
}

QString GroupsInviteJob::inviteUserName() const
{
    return mInviteUserName;
}

void GroupsInviteJob::setInviteUserName(const QString &inviteUserName)
{
    mInviteUserName = inviteUserName;
}

QString GroupsInviteJob::inviteUserId() const
{
    return mInviteUserId;
}

void GroupsInviteJob::setInviteUserId(const QString &userId)
{
    mInviteUserId = userId;
}

bool GroupsInviteJob::requireHttpAuthentication() const
{
    return true;
}

bool GroupsInviteJob::canStart() const
{
    if (mInviteUserId.isEmpty() && mInviteUserName.isEmpty()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "GroupsInviteJob: mInviteUserId is empty or mInviteUserName is empty";
        return false;
    }
    if (mRoomId.isEmpty()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "GroupsInviteJob: RoomId is empty";
        return false;
    }
    if (!RestApiAbstractJob::canStart()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "Impossible to start GroupsInviteJob job";
        return false;
    }
    return true;
}

QJsonDocument GroupsInviteJob::json() const
{
    QJsonObject jsonObj;
    jsonObj[QLatin1String("roomId")] = roomId();
    if (!inviteUserId().isEmpty()) {
        jsonObj[QLatin1String("userId")] = inviteUserId();
    } else if (!inviteUserName().isEmpty()) {
        jsonObj[QLatin1String("userName")] = inviteUserName();
    }
    const QJsonDocument postData = QJsonDocument(jsonObj);
    return postData;
}

QString GroupsInviteJob::roomId() const
{
    return mRoomId;
}

void GroupsInviteJob::setRoomId(const QString &roomId)
{
    mRoomId = roomId;
}

QNetworkRequest GroupsInviteJob::request() const
{
    const QUrl url = mRestApiMethod->generateUrl(RestApiUtil::RestApiUrlType::GroupsInvite);
    QNetworkRequest request(url);
    addAuthRawHeader(request);
    addRequestAttribute(request);
    return request;
}
