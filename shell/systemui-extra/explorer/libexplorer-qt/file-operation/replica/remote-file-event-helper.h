/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef REMOTEFILEEVENTHELPER_H
#define REMOTEFILEEVENTHELPER_H

#include <QObject>
#include <QUrl>

class QRemoteObjectNode;
class FileEventHandlerReplica;

class RemoteFileEventHelper : public QObject
{
    Q_OBJECT
public:
    explicit RemoteFileEventHelper(QObject *parent = nullptr);
    ~RemoteFileEventHelper();

Q_SIGNALS:
    void handleFileEventRequest(int type, const QString &arg1, const QString &arg2);

private:
    void setupConnections();
    static QUrl nodeUrl();

    QRemoteObjectNode *m_remoteNode = nullptr;
    FileEventHandlerReplica *m_fileEventHandlerIface = nullptr;
};

#endif // REMOTEFILEEVENTHELPER_H
