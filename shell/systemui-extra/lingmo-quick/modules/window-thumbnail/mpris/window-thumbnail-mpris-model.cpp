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

#include "window-thumbnail-mpris-model.h"
#include <windowmanager/windowmanager.h>
#include <QDebug>
#include "player-items-model.h"
#include "properties.h"
class WindowThumbnailMprisModelPrivate
{
public:
    QString m_winID;
    uint m_pid;
    QString m_desktopEntry;
    PlayerItemsModel *m_sourceModel = nullptr;
};

WindowThumbnailMprisModel::WindowThumbnailMprisModel(QObject *parent) : QSortFilterProxyModel(parent), d(new WindowThumbnailMprisModelPrivate)
{
    d->m_sourceModel = PlayerItemsModel::self();
    QSortFilterProxyModel::setSourceModel(d->m_sourceModel);
    connect(this, &WindowThumbnailMprisModel::rowsInserted, this, &WindowThumbnailMprisModel::countChanged);
    connect(this, &WindowThumbnailMprisModel::rowsRemoved, this, &WindowThumbnailMprisModel::countChanged);
    connect(this, &WindowThumbnailMprisModel::modelReset, this, &WindowThumbnailMprisModel::countChanged);
    connect(this, &WindowThumbnailMprisModel::dataChanged, this, &WindowThumbnailMprisModel::onDataChanged);
}

WindowThumbnailMprisModel::~WindowThumbnailMprisModel()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

bool WindowThumbnailMprisModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceIndex.data(MprisProperties::Pid).toUInt() == d->m_pid
           && d->m_pid != 0
           && sourceIndex.data(MprisProperties::Valid).toBool()
           && sourceIndex.data(MprisProperties::CanControl).toBool()
           && (sourceIndex.data(MprisProperties::CanGoNext).toBool() || sourceIndex.data(MprisProperties::CanGoPrevious).toBool()
               || sourceIndex.data(MprisProperties::CanPlay).toBool() || sourceIndex.data(MprisProperties::CanPause).toBool());
}

QString WindowThumbnailMprisModel::winID()
{
    return d->m_winID;
}

void WindowThumbnailMprisModel::setWinID(const QString &wid)
{
    d->m_winID = wid;
    d->m_pid = kdk::WindowManager::getPid(wid);
    invalidate();
    Q_EMIT countChanged();
}

QString WindowThumbnailMprisModel::desktopEntry()
{
    return d->m_desktopEntry;
}

void WindowThumbnailMprisModel::setDesktopEntry(const QString &desktopEntry)
{
    d->m_desktopEntry = desktopEntry;
}

void WindowThumbnailMprisModel::operation(const QModelIndex &index, MprisProperties::Operations operation,
                                          const QVariantList &args)
{
    d->m_sourceModel->operation(QSortFilterProxyModel::mapToSource(index), operation, args);
}

int WindowThumbnailMprisModel::count() const
{
    return QSortFilterProxyModel::rowCount({});
}

uint WindowThumbnailMprisModel::pid()
{
    return d->m_pid;
}

void WindowThumbnailMprisModel::setPid(const uint& pid)
{
    d->m_pid = pid;
    invalidate();
    Q_EMIT countChanged();
}

bool WindowThumbnailMprisModel::isCurrentMediaVideo()
{
    return data(index(0, 0, QModelIndex()), MprisProperties::IsCurrentMediaVideo).toBool();
}

bool WindowThumbnailMprisModel::isCurrentMediaAudio()
{
    return data(index(0, 0, QModelIndex()), MprisProperties::IsCurrentMediaAudio).toBool();
}

void WindowThumbnailMprisModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                              const QVector<int> &roles)
{
    if (roles.contains(MprisProperties::IsCurrentMediaVideo)) {
        Q_EMIT isCurrentMediaVideoChanged();
    }
    if (roles.contains(MprisProperties::IsCurrentMediaAudio)) {
        Q_EMIT isCurrentMediaAudioChanged();
    }
}
