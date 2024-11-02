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

#ifndef LINGMO_QUICK_WINDOW_THUMBNAIL_MPRIS_MODEL_H
#define LINGMO_QUICK_WINDOW_THUMBNAIL_MPRIS_MODEL_H

#include <QSortFilterProxyModel>
#include "properties.h"

class WindowThumbnailMprisModelPrivate;
class WindowThumbnailMprisModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString winID READ winID WRITE setWinID)
    Q_PROPERTY(uint pid READ pid WRITE setPid)
    Q_PROPERTY(QString desktopEntry READ desktopEntry WRITE setDesktopEntry)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool isCurrentMediaVideo READ isCurrentMediaVideo NOTIFY isCurrentMediaVideoChanged)
    Q_PROPERTY(bool isCurrentMediaAudio READ isCurrentMediaAudio NOTIFY isCurrentMediaAudioChanged)

public:
    explicit WindowThumbnailMprisModel(QObject *parent = nullptr);
    ~WindowThumbnailMprisModel();
    QString winID();
    void setWinID(const QString &wid);
    QString desktopEntry();
    void setDesktopEntry(const QString &desktopEntry);
    bool isCurrentMediaVideo();
    bool isCurrentMediaAudio();
    int count() const;
    uint pid();
    void setPid(const uint &pid);

public Q_SLOTS:
    void operation(const QModelIndex &index, MprisProperties::Operations operation, const QVariantList &args);

Q_SIGNALS:
    void countChanged();
    void isCurrentMediaVideoChanged();
    void isCurrentMediaAudioChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    WindowThumbnailMprisModelPrivate *d = nullptr;
};


#endif //LINGMO_QUICK_WINDOW_THUMBNAIL_MPRIS_MODEL_H
