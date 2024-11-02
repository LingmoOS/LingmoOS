/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QMetaEnum>
#include <QDebug>
#include <mutex>
#include "player-items-model.h"
#include "player-item.h"
#include "mpris-player-collecter.h"
static PlayerItemsModel *globalInstance = nullptr;
static std::once_flag flag;

PlayerItemsModel *PlayerItemsModel::self()
{
    std::call_once(flag, [&] {
        globalInstance = new PlayerItemsModel();
    });
    return globalInstance;
}

PlayerItemsModel::PlayerItemsModel(QObject *parent) : QAbstractListModel(parent)
{
    m_services = MprisPlayerCollecter::self()->playerServices();
    connect(MprisPlayerCollecter::self(), &MprisPlayerCollecter::playerAdded, this, &PlayerItemsModel::onPlayerAdded);
    connect(MprisPlayerCollecter::self(), &MprisPlayerCollecter::playerRemoved, this, &PlayerItemsModel::onPlayerRemoved);
    connect(MprisPlayerCollecter::self(), &MprisPlayerCollecter::dataChanged, this, &PlayerItemsModel::onDataChanged);
}

QHash<int, QByteArray> PlayerItemsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    QMetaEnum e = QMetaEnum::fromType<MprisProperties::Properties>();

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

QModelIndex PlayerItemsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return {};
    }
    return createIndex(row, column, nullptr);
}

int PlayerItemsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_services.count();
}

QVariant PlayerItemsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_services.count()) {
        return {};
    }
    if(role == MprisProperties::Pid) {
        return MprisPlayerCollecter::self()->pidOfService(m_services.at(index.row()));
    }
    PlayerItem *item = MprisPlayerCollecter::self()->item(m_services.at(index.row()));
    switch (role) {
        case MprisProperties::Valid:
            return item->valid();
        case MprisProperties::CanQuit:
            return item->canQuit();
        case MprisProperties::FullScreen:
            return item->fullScreen();
        case MprisProperties::CanSetFullScreen:
            return item->canSetFullScreen();
        case MprisProperties::CanRaise:
            return item->canRaise();
        case MprisProperties::HasTrackList:
            return item->hasTrackList();
        case MprisProperties::Identity:
            return item->identity();
        case MprisProperties::DesktopEntry:
            return item->desktopEntry();
        case MprisProperties::SupportedUriSchemes:
            return item->supportedUriSchemes();
        case MprisProperties::SupportedMimeTypes:
            return item->supportedMimeTypes();
        case MprisProperties::PlaybackStatus:
            return item->playbackStatus();
        case MprisProperties::LoopStatus:
            return item->loopStatus();
        case MprisProperties::Rate:
            return item->rate();
        case MprisProperties::Shuffle:
            return item->shuffle();
        case MprisProperties::MetaData:
            return item->metaData();
        case MprisProperties::Volume:
            return item->volume();
        case MprisProperties::Position:
            return item->position();
        case MprisProperties::MinimumRate:
            return item->minimumRate();
        case MprisProperties::MaximumRate:
            return item->maximumRate();
        case MprisProperties::CanGoNext:
            return item->canGoNext();
        case MprisProperties::CanGoPrevious:
            return item->canGoPrevious();
        case MprisProperties::CanPlay:
            return item->canPlay();
        case MprisProperties::CanPause:
            return item->canPause();
        case MprisProperties::CanSeek:
            return item->canSeek();
        case MprisProperties::CanControl:
            return item->canControl();
        case MprisProperties::IsCurrentMediaVideo:
            return item->isCurrentMediaVideo();
        case MprisProperties::IsCurrentMediaAudio:
            return item->isCurrentMediaAudio();
        default:
            return {};
    }
    Q_UNREACHABLE();
}

void PlayerItemsModel::onPlayerAdded(const QString &service, uint pid)
{
    Q_UNUSED(pid);
    beginInsertRows(QModelIndex(), m_services.size(), m_services.size());
    m_services.append(service);
    endInsertRows();
}

void PlayerItemsModel::onPlayerRemoved(const QString &service, uint pid)
{
    Q_UNUSED(pid);
    int index = m_services.indexOf(service);
    beginRemoveRows(QModelIndex(), index, index);
    m_services.removeAll(service);
    endRemoveRows();
}

void PlayerItemsModel::operation(const QModelIndex &index, MprisProperties::Operations operation, const QVariantList &args)
{
    if (!index.isValid() || index.row() >= m_services.count()) {
        return;
    }
    switch (operation) {
        case MprisProperties::Raise:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->pause();
        case MprisProperties::Quit:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->quit();
        case MprisProperties::Next:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->next();
        case MprisProperties::Previous:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->previous();
        case MprisProperties::Pause:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->pause();
        case MprisProperties::PlayPause:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->playPause();
        case MprisProperties::Stop:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->stop();
        case MprisProperties::Play:
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->play();
        case MprisProperties::Seek:
            if(args.isEmpty()) {
                qWarning() << "Seek without offset!";
                return;
            }
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->seek(args.at(0).toLongLong());
        case MprisProperties::SetPosition:
            if(args.size() < 2) {
                qWarning() << "setPosition without enough args!";
                return;
            }
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->setPosition(args.at(0).toString(), args.at(1).toLongLong());
        case MprisProperties::OpenUri:
            if(args.isEmpty()) {
                qWarning() << "OpenUri without uri!";
                return;
            }
            return MprisPlayerCollecter::self()->item(m_services.at(index.row()))->openUri(args.at(0).toString());
    }
    Q_UNREACHABLE();
}

void PlayerItemsModel::onDataChanged(const QString &service, const QVector<int> &properties)
{
    int row = m_services.indexOf(service);
    if(row >= 0) {
        QModelIndex changedIndex = index(row ,0, {});
        Q_EMIT dataChanged(changedIndex, changedIndex, properties);
    }
}
