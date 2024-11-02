/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: ding jing <dingjing@kylinos.cn>
 *
 */

#ifndef RECENTVFSMANAGER_H
#define RECENTVFSMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <explor-core_global.h>

namespace Peony {
class PEONYCORESHARED_EXPORT RecentVFSManager : public QObject
{
    Q_OBJECT
public:
    static RecentVFSManager* getInstance ();
    void clearAll ();
    void insert (QString uri, QString mimetype, QString name, QString exec);

private:
    explicit RecentVFSManager(QObject *parent = nullptr);
    bool read ();
    bool write ();
    bool exists (QString uri, QString mimetype, QString name, QString exec);
    bool createNode (QString uri, QString mimetype, QString name, QString exec);

private:
    static RecentVFSManager* m_instance;
    QString m_recent_path;
    QDomDocument m_dom_document;
};
};
#endif // RECENTVFSMANAGER_H
