/*
   Copyright (c) 2017-2019 Montel Laurent <montel@kde.org>

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

#include "message.h"
#include "utils.h"
#include "ruqola_debug.h"
#include <KLocalizedString>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

Message::Message(EmojiManager *emojiManager)
    : mEmojiManager(emojiManager)
{
}

void Message::parseMessage(const QJsonObject &o, bool restApi)
{
    const QString roomId = o.value(QLatin1String("rid")).toString();

    //t ? I can't find it.
    const QString type = o.value(QLatin1String("t")).toString();

    mMessageId = o.value(QLatin1String("_id")).toString();
    mRoomId = roomId;
    mText = o.value(QLatin1String("msg")).toString();
    if (restApi) {
        mUpdatedAt = Utils::parseIsoDate(QStringLiteral("_updatedAt"), o);
        mEditedAt = Utils::parseIsoDate(QStringLiteral("editedAt"), o);
        setTimeStamp(Utils::parseIsoDate(QStringLiteral("ts"), o));
        mThreadLastMessage = Utils::parseIsoDate(QStringLiteral("tlm"), o);
        mDiscussionLastMessage = Utils::parseIsoDate(QStringLiteral("dlm"), o);
    } else {
        setTimeStamp(Utils::parseDate(QStringLiteral("ts"), o));
        mUpdatedAt = Utils::parseDate(QStringLiteral("_updatedAt"), o);
        mEditedAt = Utils::parseDate(QStringLiteral("editedAt"), o);
        //Verify if a day we will use not restapi for it.
        mThreadLastMessage = Utils::parseDate(QStringLiteral("tlm"), o);
        //Verify if a day we will use not restapi for it.
        mDiscussionLastMessage = Utils::parseDate(QStringLiteral("dlm"), o);
    }
    mUsername = o.value(QLatin1String("u")).toObject().value(QLatin1String("username")).toString();
    mUserId = o.value(QLatin1String("u")).toObject().value(QLatin1String("_id")).toString();
    mEditedByUsername = o.value(QLatin1String("editedBy")).toObject().value(QLatin1String("username")).toString();
    mEditedByUserId = o.value(QLatin1String("editedBy")).toObject().value(QLatin1String("_id")).toString();
    mAlias = o.value(QLatin1String("alias")).toString();
    mAvatar = o.value(QLatin1String("avatar")).toString();
    mGroupable = o.value(QLatin1String("groupable")).toBool(/*true*/ false); //Laurent, disable for the moment groupable
    mParseUrls = o.value(QLatin1String("parseUrls")).toBool();
    mRole = o.value(QLatin1String("role")).toString();
    mThreadCount = o.value(QLatin1String("tcount")).toInt();
    mDiscussionCount = o.value(QLatin1String("dcount")).toInt();
    mDiscussionRoomId = o.value(QLatin1String("drid")).toString();
    mThreadMessageId = o.value(QLatin1String("tmid")).toString();
    mMessageStarred.parse(o);
    mMessagePinned.parse(o);

    mMessageType = Message::MessageType::NormalText;
    if (!type.isEmpty()) {
        mSystemMessageType = type;
        mMessageType = System;
    }
    parseMentions(o.value(QLatin1String("mentions")).toArray());

    parseAttachment(o.value(QLatin1String("attachments")).toArray());
    parseUrls(o.value(QLatin1String("urls")).toArray());
    parseReactions(o.value(QLatin1String("reactions")).toObject());
    //TODO unread element
}

void Message::parseReactions(const QJsonObject &reacts)
{
    if (!reacts.isEmpty()) {
        mReactions.parseReactions(reacts, mEmojiManager);
    }
}

QString Message::displayTime() const
{
    return mDisplayTime;
}

QString Message::threadMessageId() const
{
    return mThreadMessageId;
}

void Message::setThreadMessageId(const QString &threadMessageId)
{
    mThreadMessageId = threadMessageId;
}

QString Message::discussionRoomId() const
{
    return mDiscussionRoomId;
}

void Message::setDiscussionRoomId(const QString &discussionRoomId)
{
    mDiscussionRoomId = discussionRoomId;
}

int Message::discussionCount() const
{
    return mDiscussionCount;
}

void Message::setDiscussionCount(int discussionCount)
{
    mDiscussionCount = discussionCount;
}

qint64 Message::discussionLastMessage() const
{
    return mDiscussionLastMessage;
}

void Message::setDiscussionLastMessage(const qint64 &discussionLastMessage)
{
    mDiscussionLastMessage = discussionLastMessage;
}

qint64 Message::threadLastMessage() const
{
    return mThreadLastMessage;
}

void Message::setThreadLastMessage(const qint64 &threadLastMessage)
{
    mThreadLastMessage = threadLastMessage;
}

int Message::threadCount() const
{
    return mThreadCount;
}

void Message::setThreadCount(int threadCount)
{
    mThreadCount = threadCount;
}

MessageStarred Message::messageStarred() const
{
    return mMessageStarred;
}

void Message::setMessageStarred(const MessageStarred &messageStarred)
{
    mMessageStarred = messageStarred;
}

MessagePinned Message::messagePinned() const
{
    return mMessagePinned;
}

void Message::setMessagePinned(const MessagePinned &messagePinned)
{
    mMessagePinned = messagePinned;
}

bool Message::unread() const
{
    return mUnread;
}

void Message::setUnread(bool unread)
{
    mUnread = unread;
}

QString Message::role() const
{
    return mRole;
}

void Message::setRole(const QString &role)
{
    mRole = role;
}

void Message::parseMentions(const QJsonArray &mentions)
{
    mMentions.clear();
    for (int i = 0; i < mentions.size(); i++) {
        const QJsonObject mention = mentions.at(i).toObject();
        mMentions.insert(mention.value(QLatin1String("username")).toString(), mention.value(QLatin1String("_id")).toString());
    }
}

void Message::parseUrls(const QJsonArray &urls)
{
    mUrls.clear();
    if (!urls.isEmpty()) {
        qCDebug(RUQOLA_LOG) << " void Message::urls(const QJsonObject &attachements)"<<urls;
        for (int i = 0; i < urls.size(); i++) {
            const QJsonObject url = urls.at(i).toObject();
            MessageUrl messageUrl;
            const QJsonValue urlStr = url.value(QLatin1String("url"));
            if (!urlStr.isUndefined()) {
                messageUrl.setUrl(urlStr.toString());
            }
            const QJsonObject meta = url.value(QLatin1String("meta")).toObject();
            if (!meta.isEmpty()) {
                const QJsonValue pageTitleStr = meta.value(QLatin1String("pageTitle"));
                if (!pageTitleStr.isUndefined()) {
                    messageUrl.setPageTitle(pageTitleStr.toString());
                }
                const QJsonValue descriptionStr = meta.value(QLatin1String("description"));
                if (!descriptionStr.isUndefined()) {
                    messageUrl.setDescription(descriptionStr.toString());
                }
            }
            //TODO add more

            if (!messageUrl.isEmpty()) {
                mUrls.append(messageUrl);
            }
        }
    }
}

Reactions Message::reactions() const
{
    return mReactions;
}

void Message::setReactions(const Reactions &reactions)
{
    mReactions = reactions;
}

bool Message::starred() const
{
    return mMessageStarred.isStarred();
}

void Message::setStarred(bool starred)
{
    mMessageStarred.setIsStarred(starred);
}

QMap<QString, QString> Message::mentions() const
{
    return mMentions;
}

void Message::setMentions(const QMap<QString, QString> &mentions)
{
    mMentions = mentions;
}

void Message::parseAttachment(const QJsonArray &attachments)
{
    mAttachements.clear();
    if (!attachments.isEmpty()) {
        //qDebug() << " void Message::parseAttachment(const QJsonObject &attachements)"<<attachments;
        for (int i = 0; i < attachments.size(); i++) {
            const QJsonObject attachment = attachments.at(i).toObject();
            MessageAttachment messageAttachement;
            const QJsonValue description = attachment.value(QLatin1String("description"));
            if (!description.isUndefined()) {
                messageAttachement.setDescription(description.toString());
            }
            const QJsonValue title = attachment.value(QLatin1String("title"));
            if (!title.isUndefined()) {
                messageAttachement.setTitle(title.toString());
            }

            if (attachment.contains(QLatin1String("audio_url"))) {
                messageAttachement.setLink(attachment.value(QLatin1String("audio_url")).toString());
                mMessageType = Message::MessageType::Audio;
            } else if (attachment.contains(QLatin1String("video_url"))) {
                messageAttachement.setLink(attachment.value(QLatin1String("video_url")).toString());
                mMessageType = Message::MessageType::Video;
            } else if (attachment.contains(QLatin1String("image_url"))) {
                messageAttachement.setLink(attachment.value(QLatin1String("image_url")).toString());
                mMessageType = Message::MessageType::Image;
            } else if (attachment.contains(QLatin1String("title_link"))) { //Last as an image_url can have a title_link
                messageAttachement.setLink(attachment.value(QLatin1String("title_link")).toString());
                mMessageType = Message::MessageType::File;
            }
            //Add image dimension
            if (mMessageType == Message::MessageType::Image) {
                const QJsonValue imageDimensions = attachment.value(QLatin1String("image_dimensions"));
                if (!imageDimensions.isUndefined()) {
                    const QJsonObject imageDimensionsParams = imageDimensions.toObject();

                    messageAttachement.setImageHeight(imageDimensionsParams.value(QLatin1String("height")).toInt());
                    messageAttachement.setImageWidth(imageDimensionsParams.value(QLatin1String("width")).toInt());
                    //TODO validate image size
                } else {
                    //Use default value ?
                    if (attachment.contains(QLatin1String("image_preview"))) {
                        messageAttachement.setImageHeight(120);
                        messageAttachement.setImageWidth(120);
                    } else {
                        //We don't have dimension so we can't load it.
                        //=>convert to normaltext
                        mMessageType = Message::MessageType::NormalText;
                    }
                }
            }

            messageAttachement.setAuthorName(attachment.value(QLatin1String("author_name")).toString());
            //Color
            const QJsonValue color = attachment.value(QLatin1String("color"));
            if (!color.isUndefined()) {
                messageAttachement.setColor(color.toString());
            }
            //MimeType
            messageAttachement.setMimeType(attachment.value(QLatin1String("image_type")).toString());

            if (!messageAttachement.isEmpty()) {
                mAttachements.append(messageAttachement);
            }
        }
    }
}

bool Message::operator==(const Message &other) const
{
    return (mMessageId == other.messageId())
           && (mRoomId == other.roomId())
           && (mText == other.text())
           && (mTimeStamp == other.timeStamp())
           && (mUsername == other.username())
           && (mUserId == other.userId())
           && (mUpdatedAt == other.updatedAt())
           && (mEditedAt == other.editedAt())
           && (mEditedByUsername == other.editedByUsername())
           && (mEditedByUserId == other.editedByUserId())
           && (mAlias == other.alias())
           && (mAvatar == other.avatar())
           && (mSystemMessageType == other.systemMessageType())
           && (mGroupable == other.groupable())
           && (mParseUrls == other.parseUrls())
           && (mUrls == other.urls())
           && (mAttachements == other.attachements())
           && (mMentions == other.mentions())
           && (mRole == other.role())
           && (mReactions == other.reactions())
           && (mUnread == other.unread())
           && (mMessagePinned == other.messagePinned())
           && (mMessageStarred == other.messageStarred())
           && (mThreadCount == other.threadCount())
           && (mThreadLastMessage == other.threadLastMessage())
           && (mDiscussionCount == other.discussionCount())
           && (mDiscussionLastMessage == other.discussionLastMessage())
           && (mDiscussionRoomId == other.discussionRoomId())
           && (mThreadMessageId == other.threadMessageId());
}

Message &Message::operator=(const Message &other)
{
    setMessageId(other.messageId());
    setRoomId(other.roomId());
    setText(other.text());
    setTimeStamp(other.timeStamp());
    setUsername(other.username());
    setUserId(other.userId());
    setUpdatedAt(other.updatedAt());
    setEditedAt(other.editedAt());
    setEditedByUsername(other.editedByUsername());
    setEditedByUserId(other.editedByUserId());
    setAlias(other.alias());
    setAvatar(other.avatar());
    setSystemMessageType(other.systemMessageType());
    setGroupable(other.groupable());
    setParseUrls(other.parseUrls());
    setUrls(other.urls());
    setAttachements(other.attachements());
    setMentions(other.mentions());
    setMessageType(other.messageType());
    setStarred(other.starred());
    setRole(other.role());
    setReactions(other.reactions());
    setUnread(other.unread());
    setMessagePinned(other.messagePinned());
    setMessageStarred(other.messageStarred());
    setThreadCount(other.threadCount());
    setThreadLastMessage(other.threadLastMessage());
    setDiscussionCount(other.discussionCount());
    setDiscussionLastMessage(other.discussionLastMessage());
    setDiscussionRoomId(other.discussionRoomId());
    setThreadMessageId(other.threadMessageId());
    return *this;
}

bool Message::operator<(const Message &other) const
{
    return mTimeStamp < other.mTimeStamp;
}

QString Message::systemMessageType() const
{
    return mSystemMessageType;
}

void Message::setSystemMessageType(const QString &systemMessageType)
{
    mSystemMessageType = systemMessageType;
}

Message::MessageType Message::messageType() const
{
    return mMessageType;
}

QString Message::messageTypeText() const
{
    if (mSystemMessageType == QLatin1String("uj")) {
        return i18n("%1 has joined the channel", mUsername);
    } else if (mSystemMessageType == QLatin1String("ul")) {
        return i18n("%1 has left the channel", mUsername);
    } else if (mSystemMessageType == QLatin1String("room_changed_topic")) {
        if (mText.isEmpty()) {
            return i18n("Topic was cleared by: %1", mUsername);
        } else {
            return i18n("%2 changed topic to: <i>%1</i>", mText, mUsername);
        }
    } else if (mSystemMessageType == QLatin1String("au")) {
        return i18n("%2 added %1 to the conversation", mText, mUsername);
    } else if (mSystemMessageType == QLatin1String("r")) {
        return i18n("%2 changed room name to <a href=\"ruqola:/room/%1\">#%1</a>", mText, mUsername);
    } else if (mSystemMessageType == QLatin1String("ru")) {
        return i18n("%2 removed user %1", mText, mUsername);
    } else if (mSystemMessageType == QLatin1String("room_changed_description")) {
        if (mText.isEmpty()) {
            return i18n("Description was cleared by %1", mUsername);
        } else {
            return i18n("%2 changed room description to %1", mText, mUsername);
        }
    } else if (mSystemMessageType == QLatin1String("room_changed_announcement")) {
        if (mText.isEmpty()) {
            return i18n("Announcement was cleared by %1", mUsername);
        } else {
            return i18n("%2 changed room announcement to %1", mText, mUsername);
        }
    } else if (mSystemMessageType == QLatin1String("room_changed_privacy")) {
        return i18n("%2 changed room privacy to %1", mText, mUsername);
    } else if (mSystemMessageType == QLatin1String("jitsi_call_started")) {
        return i18n("Click to join to video");
    } else if (mSystemMessageType == QLatin1String("rm")) {
        //TODO encrypted message
        return i18n("Message Deleted");
    } else if (mSystemMessageType == QLatin1String("message_pinned")) {
        return i18n("Message Pinned");
    } else if (mSystemMessageType == QLatin1String("otr")) {
        return i18n("Encrypted Message");
    } else if (mSystemMessageType == QLatin1String("user-unmuted")) {
        //TODO improve it
        return i18n("%1 was unmuted", mUsername);
    } else if (mSystemMessageType == QLatin1String("user-muted")) {
        //TODO improve it
        return i18n("%1 was muted", mUsername);
    } else if (mSystemMessageType == QLatin1String("subscription-role-added")) {
        return i18n("Role \'%3\' was added to %1 by %2", mUsername, mText, mRole);
    } else if (mSystemMessageType == QLatin1String("subscription-role-removed")) {
        return i18n("Role \'%3\' was removed to %1 by %2", mUsername, mText, mRole);
    } else if (mSystemMessageType == QLatin1String("e2e")) {
        //TODO need to unencrypt it
        return i18n("Encrypted message: %1", mText);
    } else if (mSystemMessageType == QLatin1String("discussion-created")) {
        return i18n("Discussion created about \"%1\"", mText);
    } else if (mSystemMessageType == QLatin1String("ut")) {
        return i18n("%1 has joined the conversation", mUsername);
    } else if (mSystemMessageType == QLatin1String("room-archived")) {
        return i18n("This room has been archived by %1", mUsername);
    } else if (mSystemMessageType == QLatin1String("room-unarchived")) {
        return i18n("This room has been unarchived by %1", mUsername);
    } else if (mSystemMessageType == QLatin1String("rtc")) {
        qCWarning(RUQOLA_LOG) << "Need to implement : " << mSystemMessageType << " mText " << mText;
        return i18n("Unknown action!");
    } else {
        qCWarning(RUQOLA_LOG) << "Unknown type for message: type: " << mSystemMessageType << " mText " << mText;
        return i18n("Unknown action!");
    }
}

void Message::setMessageType(const MessageType &messageType)
{
    mMessageType = messageType;
}

QVector<MessageAttachment> Message::attachements() const
{
    return mAttachements;
}

void Message::setAttachements(const QVector<MessageAttachment> &attachements)
{
    mAttachements = attachements;
}

QVector<MessageUrl> Message::urls() const
{
    return mUrls;
}

void Message::setUrls(const QVector<MessageUrl> &urls)
{
    mUrls = urls;
}

QString Message::alias() const
{
    return mAlias;
}

void Message::setAlias(const QString &alias)
{
    mAlias = alias;
}

QString Message::editedByUserId() const
{
    return mEditedByUserId;
}

void Message::setEditedByUserId(const QString &editedByUserId)
{
    mEditedByUserId = editedByUserId;
}

QString Message::editedByUsername() const
{
    return mEditedByUsername;
}

void Message::setEditedByUsername(const QString &editedByUsername)
{
    mEditedByUsername = editedByUsername;
}

qint64 Message::editedAt() const
{
    return mEditedAt;
}

void Message::setEditedAt(const qint64 &editedAt)
{
    mEditedAt = editedAt;
}

qint64 Message::updatedAt() const
{
    return mUpdatedAt;
}

void Message::setUpdatedAt(const qint64 &updatedAt)
{
    mUpdatedAt = updatedAt;
}

QString Message::userId() const
{
    return mUserId;
}

void Message::setUserId(const QString &userId)
{
    mUserId = userId;
}

QString Message::username() const
{
    return mUsername;
}

void Message::setUsername(const QString &username)
{
    mUsername = username;
}

qint64 Message::timeStamp() const
{
    return mTimeStamp;
}

void Message::setTimeStamp(const qint64 &timeStamp)
{
    if (mTimeStamp != timeStamp) {
        mTimeStamp = timeStamp;
        mDisplayTime = QDateTime::fromMSecsSinceEpoch(mTimeStamp).time().toString(QStringLiteral("hh:mm"));
    }
}

QString Message::text() const
{
    return mText;
}

void Message::setText(const QString &text)
{
    mText = text;
}

QString Message::messageId() const
{
    return mMessageId;
}

void Message::setMessageId(const QString &messageId)
{
    mMessageId = messageId;
}

QString Message::roomId() const
{
    return mRoomId;
}

void Message::setRoomId(const QString &roomId)
{
    mRoomId = roomId;
}

QString Message::avatar() const
{
    return mAvatar;
}

void Message::setAvatar(const QString &avatar)
{
    mAvatar = avatar;
}

bool Message::parseUrls() const
{
    return mParseUrls;
}

void Message::setParseUrls(bool parseUrls)
{
    mParseUrls = parseUrls;
}

bool Message::groupable() const
{
    return mGroupable;
}

void Message::setGroupable(bool groupable)
{
    mGroupable = groupable;
}

Message Message::fromJSon(const QJsonObject &o)
{
    Message message;
    message.mThreadCount = o[QStringLiteral("tcount")].toString().toInt();
    message.mDiscussionCount = o[QStringLiteral("dcount")].toString().toInt();
    message.mDiscussionRoomId = o[QStringLiteral("drid")].toString();
    message.mThreadMessageId = o[QStringLiteral("tmid")].toString();

    if (o.contains(QLatin1String("tlm"))) {
        message.mThreadLastMessage = static_cast<qint64>(o[QStringLiteral("tlm")].toDouble());
    }
    if (o.contains(QLatin1String("dlm"))) {
        message.mDiscussionLastMessage = static_cast<qint64>(o[QStringLiteral("dlm")].toDouble());
    }

    message.mMessageId = o[QStringLiteral("messageID")].toString();
    message.mRoomId = o[QStringLiteral("roomID")].toString();
    message.mText = o[QStringLiteral("message")].toString();
    message.mTimeStamp = static_cast<qint64>(o[QStringLiteral("timestamp")].toDouble());
    message.mUsername = o[QStringLiteral("username")].toString();
    message.mUserId = o[QStringLiteral("userID")].toString();
    message.mUpdatedAt = static_cast<qint64>(o[QStringLiteral("updatedAt")].toDouble());
    message.mEditedAt = static_cast<qint64>(o[QStringLiteral("editedAt")].toDouble());
    message.mEditedByUsername = o[QStringLiteral("editedByUsername")].toString();
    message.mEditedByUserId = o[QStringLiteral("editedByUserID")].toString();
    message.mAlias = o[QStringLiteral("alias")].toString();
    message.mAvatar = o[QStringLiteral("avatar")].toString();
    message.mGroupable = o[QStringLiteral("groupable")].toBool();
    message.mParseUrls = o[QStringLiteral("parseUrls")].toBool();
    message.mMessageStarred.setIsStarred(o[QStringLiteral("starred")].toBool());
    message.mMessagePinned.setPinned(o[QStringLiteral("pinned")].toBool());
    message.mRole = o[QStringLiteral("role")].toString();
    message.mSystemMessageType = o[QStringLiteral("type")].toString();
    message.mMessageType = o[QStringLiteral("messageType")].toVariant().value<MessageType>();
    const QJsonArray attachmentsArray = o.value(QLatin1String("attachments")).toArray();
    for (int i = 0; i < attachmentsArray.count(); ++i) {
        const QJsonObject attachment = attachmentsArray.at(i).toObject();
        const MessageAttachment att = MessageAttachment::fromJSon(attachment);
        if (!att.isEmpty()) {
            message.mAttachements.append(att);
        }
    }
    const QJsonArray urlsArray = o.value(QLatin1String("urls")).toArray();
    for (int i = 0; i < urlsArray.count(); ++i) {
        const QJsonObject urlObj = urlsArray.at(i).toObject();
        const MessageUrl url = MessageUrl::fromJSon(urlObj);
        if (!url.isEmpty()) {
            message.mUrls.append(url);
        }
    }
    const QJsonObject reactionsArray = o.value(QLatin1String("reactions")).toObject();
    message.setReactions(Reactions::fromJSon(reactionsArray));

    const QJsonArray mentionsArray = o.value(QLatin1String("mentions")).toArray();
    for (int i = 0; i < mentionsArray.count(); ++i) {
        const QJsonObject mention = mentionsArray.at(i).toObject();
        qCDebug(RUQOLA_LOG) << " mention"<<mention;
//        const MessageAttachment att = MessageAttachment::fromJSon(attachment);
//        if (!att.isEmpty()) {
//            message.mAttachements.append(att);
//        }
    }

    return message;
}

QByteArray Message::serialize(const Message &message, bool toBinary)
{
    QJsonDocument d;
    QJsonObject o;

    o[QStringLiteral("messageID")] = message.mMessageId;
    o[QStringLiteral("roomID")] = message.mRoomId;
    o[QStringLiteral("message")] = message.mText;
    o[QStringLiteral("timestamp")] = message.mTimeStamp;
    o[QStringLiteral("username")] = message.mUsername;
    o[QStringLiteral("userID")] = message.mUserId;
    o[QStringLiteral("updatedAt")] = message.mUpdatedAt;
    o[QStringLiteral("editedAt")] = message.mEditedAt;
    if (message.mThreadLastMessage > -1) {
        o[QStringLiteral("tlm")] = message.mThreadLastMessage;
    }
    if (message.mDiscussionLastMessage > -1) {
        o[QStringLiteral("dlm")] = message.mDiscussionLastMessage;
    }

    o[QStringLiteral("editedByUsername")] = message.mEditedByUsername;
    o[QStringLiteral("editedByUserID")] = message.mEditedByUserId;
    o[QStringLiteral("alias")] = message.mAlias;
    o[QStringLiteral("avatar")] = message.mAvatar;
    o[QStringLiteral("groupable")] = message.mGroupable;
    o[QStringLiteral("parseUrls")] = message.mParseUrls;
    o[QStringLiteral("starred")] = message.mMessageStarred.isStarred();
    o[QStringLiteral("pinned")] = message.mMessagePinned.pinned();
    //TODO add pinned
    if (!message.mRole.isEmpty()) {
        o[QStringLiteral("role")] = message.mRole;
    }

    o[QStringLiteral("type")] = message.mSystemMessageType;
    o[QStringLiteral("messageType")] = QJsonValue::fromVariant(QVariant::fromValue<Message::MessageType>(message.mMessageType));
    //TODO add mentions
    //Attachments
    if (!message.mAttachements.isEmpty()) {
        QJsonArray array;
        const int nbAttachment = message.mAttachements.count();
        for (int i = 0; i < nbAttachment; ++i) {
            array.append(MessageAttachment::serialize(message.mAttachements.at(i)));
        }
        o[QStringLiteral("attachments")] = array;
    }
    //FIXME save mentions
    QMapIterator<QString, QString> i(message.mentions());
    while (i.hasNext()) {
        i.next();
        //TODO
    }
    //Urls
    if (!message.mUrls.isEmpty()) {
        QJsonArray array;
        const int nbUrls = message.mUrls.count();
        for (int i = 0; i < nbUrls; ++i) {
            array.append(MessageUrl::serialize(message.mUrls.at(i)));
        }
        o[QStringLiteral("urls")] = array;
    }

    if (!message.reactions().isEmpty()) {
        o[QStringLiteral("reactions")] = Reactions::serialize(message.reactions());
    }

    if (message.mThreadCount > 0) {
        o[QStringLiteral("tcount")] = message.mThreadCount;
        o[QStringLiteral("tlm")] = message.mThreadLastMessage;
    }

    if (message.mDiscussionCount > 0) {
        o[QStringLiteral("dcount")] = message.mDiscussionCount;
        o[QStringLiteral("dlm")] = message.mDiscussionLastMessage;
    }
    if (!message.mDiscussionRoomId.isEmpty()) {
        o[QStringLiteral("drid")] = message.mDiscussionRoomId;
    }

    if (!message.mThreadMessageId.isEmpty()) {
        o[QStringLiteral("tmid")] = message.mThreadMessageId;
    }
    d.setObject(o);
    if (toBinary) {
        return d.toBinaryData();
    }
    return d.toJson(QJsonDocument::Indented);
}

QDebug operator <<(QDebug d, const Message &t)
{
    d << "mMessageId : " << t.messageId();
    d << "mText: " << t.text();
    d << "mTimeStamp: " << t.timeStamp();
    d << "mUsername: " << t.username();
    d << "mUserId: " << t.userId();
    d << "mUpdatedAt: " << t.updatedAt();
    d << "mEditedAt: " << t.editedAt();
    d << "mEditedByUsername: " << t.editedByUsername();
    d << "mEditedByUserId: " << t.editedByUserId();
    d << "mAlias: " << t.alias();
    d << "mSystemMessageType: " << t.systemMessageType();
    d << "mRoomId: " << t.roomId();
    d << "mAvatar: " << t.avatar();
    d << "mGroupable: " << t.groupable();
    d << "mParseUrls: " << t.parseUrls();
    for (int i = 0, total = t.attachements().count(); i < total; ++i) {
        d << "Attachment :" << t.attachements().at(i);
    }
    for (int i = 0, total = t.urls().count(); i < total; ++i) {
        d << "Urls :" << t.urls().at(i);
    }
    d << "Mentions :" << t.mentions();
    d << "mMessageType: " << t.messageType();
    d << "mRole: " << t.role();
    d << "mReaction: " << t.reactions();
    d << "mUnread: " << t.unread();
    d << "starred " << t.messageStarred();
    d << "pinned " << t.messagePinned();
    d << "threadcount " << t.threadCount();
    d << "threadlastmessage " << t.threadLastMessage();
    d << "discussioncount " << t.discussionCount();
    d << "discussionlastmessage " << t.discussionLastMessage();
    d << "discussionRoomId " << t.discussionRoomId();
    d << "threadMessageId " << t.threadMessageId();
    return d;
}
