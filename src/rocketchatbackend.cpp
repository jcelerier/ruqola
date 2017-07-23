/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright 2016  Riccardo Iaconelli <riccardo@kde.org>
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

#include "rocketchatbackend.h"
#include <QtCore>
#include <QJsonObject>
#include <QDebug>
#include "ruqola.h"
#include "ddpclient.h"

void debug_callback(QJsonDocument doc)
{
    qDebug() << "DEBUG:" << doc;
}

void process_backlog(QJsonDocument messages)
{
    qDebug() << messages.object().value("messages").toArray().size();
    RocketChatBackend::processIncomingMessages(messages.object().value("messages").toArray());
}


void rooms_callback(const QJsonDocument &doc)
{
    RoomModel *model = Ruqola::self()->roomModel();
    
    QJsonArray removed = doc.object().value("remove").toArray();
    QJsonArray updated = doc.object().value("update").toArray();
    
    for (int i = 0; i < updated.size(); i++) {
        QJsonObject room = updated.at(i).toObject();
    
        if (room.value("t").toString() != "d") {

            QString roomID = room.value("_id").toString();
            MessageModel *roomModel = Ruqola::self()->getModelForRoom(roomID);
            
            // let's be extra safe around crashes
            if (Ruqola::self()->loginStatus() == DDPClient::LoggedIn) {
                Room r;
                r.id = roomID;
                r.name = room["name"].toString();
                r.topic = room["topic"].toString();
                                
                qDebug() << "Adding room" << r.name << r.id << r.topic;

                model->addRoom(r);
            }
            
            QJsonArray params;
            params.append(QJsonValue(roomID));
            Ruqola::self()->ddp()->subscribe("stream-room-messages", params);

            // Load history
            params.append(QJsonValue(QJsonValue::Null));
            params.append(QJsonValue(50)); // Max number of messages to load;
            QJsonObject dateObject;
            dateObject["$date"] = QJsonValue(roomModel->lastTimestamp());
            params.append(dateObject);
            Ruqola::self()->ddp()->method("loadHistory", QJsonDocument(params), process_backlog);
        }
    } 
}

void subs_callback(const QJsonDocument &doc)
{
    RoomModel *model = Ruqola::self()->roomModel();

    QJsonArray removed = doc.object().value("remove").toArray();
    QJsonArray updated = doc.object().value("update").toArray();
    
    for (int i = 0; i < updated.size(); i++) {
        QJsonObject room = updated.at(i).toObject();
    
        if (room.value("t").toString() != "d") {

            QString roomID = room.value("rid").toString();
            MessageModel *roomModel = Ruqola::self()->getModelForRoom(roomID);
            
            // let's be extra safe around crashes
            if (Ruqola::self()->loginStatus() == DDPClient::LoggedIn) {
                Room r;
                r.id = roomID;
                r.name = room["name"].toString();
                r.topic = room["topic"].toString();
                                
                qDebug() << "Adding room" << r.name << r.id << r.topic;
                
                model->addRoom(r);
            }
            
            
            QJsonArray params;
            params.append(QJsonValue(roomID));
            Ruqola::self()->ddp()->subscribe("stream-room-messages", params);

            // Load history
            params.append(QJsonValue(QJsonValue::Null));
            params.append(QJsonValue(50)); // Max number of messages to load;
            QJsonObject dateObject;
            dateObject["$date"] = QJsonValue(roomModel->lastTimestamp());
            params.append(dateObject);
            Ruqola::self()->ddp()->method("loadHistory", QJsonDocument(params), process_backlog);
        }
    } 
}

void RocketChatBackend::processIncomingMessages(const QJsonArray &messages)
{
    foreach (const QJsonValue &v, messages) {
        QJsonObject o = v.toObject();
        
        Message m;
        QString roomId = o.value("rid").toString();
        QString type = o.value("t").toString();

        m.messageID = o.value("_id").toString();
        m.roomID = roomId;
        m.message = o.value("msg").toString();
        m.timestamp = (qint64)o.value("ts").toObject().value("$date").toDouble();
        m.username = o.value("u").toObject().value("username").toString();
        m.userID = o.value("u").toObject().value("_id").toString();
        m.updatedAt = o.value("_updatedAt").toObject().value("$date").toDouble();
        m.editedAt = o.value("editedAt").toObject().value("$date").toDouble();
        m.editedByUsername = o.value("editedBy").toObject().value("username").toString();
        m.editedByUserID = o.value("editedBy").toObject().value("userID").toString();
        m.url = o.value("urls").toObject().value("url").toString();
        m.meta = o.value("urls").toObject().value("meta").toString();
        m.headers = o.value("urls").toObject().value("headers").toString();
        m.parsedUrl = o.value("urls").toObject().value("parsedUrl").toString();
        m.imageUrl = o.value("attachments").toObject().value("imageUrl").toString();
        m.color = o.value("attachments").toObject().value("color").toString();
        m.alias = o.value("alias").toString();
        m.avatar = o.value("avatar").toString();
        m.groupable = o.value("groupable").toBool();
        m.parseUrls = o.value("parseUrls").toBool();

        if (!type.isEmpty()) {
            m.systemMessage = true;
            m.systemMessageType = type;
        } else {
            m.systemMessage = false;
        }
        
        Ruqola::self()->getModelForRoom(roomId)->addMessage(m);
    }
}

RocketChatBackend::RocketChatBackend(QObject* parent)
 : QObject(parent)
{
        connect(Ruqola::self(), &Ruqola::loginStatusChanged, this, &RocketChatBackend::onLoginStatusChanged);
        connect(Ruqola::self(), &Ruqola::userIDChanged, this, &RocketChatBackend::onUserIDChanged);
        connect(Ruqola::self()->ddp(), &DDPClient::changed, this, &RocketChatBackend::onChanged);
        connect(Ruqola::self()->ddp(), &DDPClient::added, this, &RocketChatBackend::onAdded);
}

RocketChatBackend::~RocketChatBackend()
{
}

void RocketChatBackend::onLoginStatusChanged()
{
    if (Ruqola::self()->loginStatus() == DDPClient::LoggedIn) {
        qDebug() << "GETTING LIST OF ROOMS";
        QJsonObject params;
        params["$date"] = QJsonValue(0); // get ALL rooms we've ever seen
        Ruqola::self()->ddp()->method("rooms/get", QJsonDocument(params), rooms_callback);
    }
}

void RocketChatBackend::onLoggedIn()
{

}

void RocketChatBackend::onAdded(const QJsonObject &object)
{
    QString collection = object.value("collection").toString();
        
    if (collection == "stream-room-messages") {
        
    } else if (collection == "users") {
        
        QJsonObject fields =  object.value("fields").toObject();
        QString username = fields.value("username").toString();
        if(username == Ruqola::self()->userName()){
            Ruqola::self()->setUserID(object["id"].toString());
            qDebug() << "User id set to " << Ruqola::self()->userID();
        }

         qDebug() << "NEW USER ADDED: " << username;
        
    } else if (collection == "rooms") {

    }
    else if (collection == "stream-notify-user"){

    }
}


void RocketChatBackend::onChanged(const QJsonObject &object)
{
    QString collection = object["collection"].toString();

    if (collection == "stream-room-messages") {
        QJsonObject fields = object.value("fields").toObject();
        QString roomId = fields.value("eventName").toString();
        QJsonArray contents = fields.value("args").toArray();
        processIncomingMessages(contents);        

    } else if (collection == "users") {
        qDebug() << "USER CHANGED";
        
    } else if (collection == "rooms") {

    } else if (collection == "stream-notify-user") {
        QJsonObject fields = object.value("fields").toObject();
        QJsonArray contents = fields.value("args").toArray();
        QString message = contents.at(0).toObject()["text"].toString();
        Ruqola::self()->notification()->showMessage("New message", message, QSystemTrayIcon::Information, 5000 );
        qDebug() << "New notification" << object.value("fields").toObject();
    }
}

void RocketChatBackend::onUserIDChanged()
{
    qDebug() << "subscribing to notification feed";
    QJsonArray params;
    params.append(QJsonValue(QString("%1/%2").arg(Ruqola::self()->userID()).arg(QString("notification"))));
    Ruqola::self()->ddp()->subscribe("stream-notify-user", params);
}

