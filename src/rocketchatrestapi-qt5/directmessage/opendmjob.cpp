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

#include "opendmjob.h"

#include "rocketchatqtrestapi_debug.h"
#include "restapimethod.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
using namespace RocketChatRestApi;
OpenDmJob::OpenDmJob(QObject *parent)
    : RestApiAbstractJob(parent)
{
}

OpenDmJob::~OpenDmJob()
{
}

bool OpenDmJob::start()
{
    if (!canStart()) {
        deleteLater();
        return false;
    }
    const QByteArray baPostData = json().toJson(QJsonDocument::Compact);
    addLoggerInfo("OpenDmJob::start: " + baPostData);
    QNetworkReply *reply = mNetworkAccessManager->post(request(), baPostData);
    connect(reply, &QNetworkReply::finished, this, &OpenDmJob::slotOpenDmFinished);
    return true;
}

void OpenDmJob::slotOpenDmFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply) {
        const QByteArray data = reply->readAll();
        const QJsonDocument replyJson = QJsonDocument::fromJson(data);
        const QJsonObject replyObject = replyJson.object();

        if (replyObject[QStringLiteral("success")].toBool()) {
            addLoggerInfo(QByteArrayLiteral("Create direct message sucess: ") + replyJson.toJson(QJsonDocument::Indented));
            Q_EMIT openDmDone();
        } else {
            emitFailedMessage(replyObject);
            addLoggerWarning(QByteArrayLiteral("Create direct message Problem: ") + replyJson.toJson(QJsonDocument::Indented));
        }
        reply->deleteLater();
    }
    deleteLater();
}

QString OpenDmJob::directUserId() const
{
    return mDirectUserId;
}

void OpenDmJob::setDirectUserId(const QString &userId)
{
    mDirectUserId = userId;
}

bool OpenDmJob::requireHttpAuthentication() const
{
    return true;
}

bool OpenDmJob::canStart() const
{
    if (mDirectUserId.isEmpty()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "OpenDmJob: mDirectUserId is empty";
        return false;
    }
    if (!RestApiAbstractJob::canStart()) {
        qCWarning(ROCKETCHATQTRESTAPI_LOG) << "Impossible to start OpenDmJob job";
        return false;
    }
    return true;
}

QJsonDocument OpenDmJob::json() const
{
    QJsonObject jsonObj;
    jsonObj[QLatin1String("userId")] = mDirectUserId;
    const QJsonDocument postData = QJsonDocument(jsonObj);
    return postData;
}

QNetworkRequest OpenDmJob::request() const
{
    const QUrl url = mRestApiMethod->generateUrl(RestApiUtil::RestApiUrlType::ImOpen);
    QNetworkRequest request(url);
    addAuthRawHeader(request);
    addRequestAttribute(request);
    return request;
}
