/*
   Copyright (c) 2019 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "discussionsmodel.h"
#include "discussions.h"

DiscussionsModel::DiscussionsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    mDiscussions = new Discussions;
}

DiscussionsModel::~DiscussionsModel()
{
    delete mDiscussions;
}

void DiscussionsModel::checkFullList()
{
    setHasFullList(mDiscussions->discussions().count() == mDiscussions->total());
}

void DiscussionsModel::initialize()
{
    mRoomId.clear();
}

QString DiscussionsModel::roomId() const
{
    return mRoomId;
}

void DiscussionsModel::setRoomId(const QString &roomId)
{
    mRoomId = roomId;
}

int DiscussionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mDiscussions->count();
}

void DiscussionsModel::addMoreDiscussions(const QJsonObject &discussionsObj)
{
    const int numberOfElement = mDiscussions->discussions().count();
    mDiscussions->parseMoreDiscussions(discussionsObj);
    beginInsertRows(QModelIndex(), numberOfElement, mDiscussions->discussions().count() - 1);
    endInsertRows();
    checkFullList();
}

Discussions *DiscussionsModel::discussions() const
{
    return mDiscussions;
}

int DiscussionsModel::total() const
{
    if (mDiscussions) {
        return mDiscussions->total();
    }
    return -1;
}

void DiscussionsModel::parseDiscussions(const QJsonObject &discussionsObj, const QString &roomId)
{
    mRoomId = roomId;
    if (rowCount() != 0) {
        beginRemoveRows(QModelIndex(), 0, mDiscussions->discussions().count() - 1);
        mDiscussions->clear();
        endRemoveRows();
    }
    mDiscussions->parseDiscussions(discussionsObj);
    if (!mDiscussions->isEmpty()) {
        beginInsertRows(QModelIndex(), 0, mDiscussions->discussions().count() - 1);
        endInsertRows();
    }
    checkFullList();
}

QVariant DiscussionsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= mDiscussions->count()) {
        return {};
    }
    const Discussion discussion = mDiscussions->at(index.row());
    switch (role) {
    case ParentId:
        return discussion.parentRoomId();
    case Description:
        return discussion.description().isEmpty() ? discussion.fname() : discussion.description();
    case NumberOfMessages:
        return discussion.numberMessages();
    case LastMessage:
        return discussion.lastMessageDisplay();
    case DiscussionRoomId:
        return discussion.discussionRoomId();
    case TimeStamp:
        return discussion.timeStampDisplay();
    case SortByTimeStamp:
        return discussion.timeStamp();
    }
    return {};
}

QHash<int, QByteArray> DiscussionsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ParentId] = QByteArrayLiteral("parentid");
    roles[Description] = QByteArrayLiteral("description");
    roles[NumberOfMessages] = QByteArrayLiteral("numberofmessages");
    roles[LastMessage] = QByteArrayLiteral("lastmessage");
    roles[DiscussionRoomId] = QByteArrayLiteral("discussionid");
    roles[TimeStamp] = QByteArrayLiteral("timestamp");
    return roles;
}

void DiscussionsModel::setDiscussions(const Discussions &discussions)
{
    if (rowCount() != 0) {
        beginRemoveRows(QModelIndex(), 0, mDiscussions->count() - 1);
        mDiscussions->clear();
        endRemoveRows();
    }
    if (!discussions.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, discussions.count() - 1);
        mDiscussions->setDiscussions(discussions.discussions());
        endInsertRows();
    }
    checkFullList();
}

void DiscussionsModel::setHasFullList(bool state)
{
    if (mHasFullList != state) {
        mHasFullList = state;
        Q_EMIT hasFullListChanged();
    }
}

bool DiscussionsModel::hasFullList() const
{
    return mHasFullList;
}
