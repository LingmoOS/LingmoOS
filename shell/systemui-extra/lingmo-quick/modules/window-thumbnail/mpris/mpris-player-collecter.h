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

#ifndef LINGMO_QUICK_MPRIS_PLAYER_COLLECTER_H
#define LINGMO_QUICK_MPRIS_PLAYER_COLLECTER_H

#include <QObject>
#include "player-item.h"
class MprisPlayerCollecterPrivate;
class MprisPlayerCollecter : public QObject
{
    Q_OBJECT
public:
    static MprisPlayerCollecter *self();
    PlayerItem *item(uint pid);
    PlayerItem *item(const QString &service);
    QStringList playerServices();
    uint pidOfService(const QString &service);

Q_SIGNALS:
    void playerAdded(const QString &service, uint pid);
    void playerRemoved(const QString &service, uint pid);
    void dataChanged(const QString &service, const QVector<int> &properties);

private:
    explicit MprisPlayerCollecter(QObject *parent = nullptr);

    MprisPlayerCollecterPrivate *d = nullptr;
};


#endif //LINGMO_QUICK_MPRIS_PLAYER_COLLECTER_H
