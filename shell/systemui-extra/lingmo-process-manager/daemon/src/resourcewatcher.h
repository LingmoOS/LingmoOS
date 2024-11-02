/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RESOURCEWATCHER_H
#define RESOURCEWATCHER_H

#include <QObject>

class ResourceWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ResourceWatcher(QObject *parent = nullptr);

Q_SIGNALS:
    void finished();
    void ResourceThresholdWarning(QString resource, int level);

public Q_SLOTS:
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif // RESOURCEWATCHER_H
