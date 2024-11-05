/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef EMBLEMJOB_H
#define EMBLEMJOB_H

#include <QObject>
#include <QRunnable>

namespace Peony {

class EmblemJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    EmblemJob(QStringList &list, QObject *parent);

    void run() override;

private:
    QStringList m_list;
};

}

#endif // EMBLEMJOB_H
