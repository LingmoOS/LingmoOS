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
#include "remote-file-event-helper.h"
#include "rep_fileeventhandler_replica.h"

#include <QtRemoteObjects>
#include <QUrl>
#include <QDebug>

RemoteFileEventHelper::RemoteFileEventHelper(QObject *parent) : QObject(parent)
{
    setupConnections();
}

RemoteFileEventHelper::~RemoteFileEventHelper()
{
    delete m_fileEventHandlerIface;
    delete m_remoteNode;
}

void RemoteFileEventHelper::setupConnections()
{
    m_remoteNode = new QRemoteObjectNode;
    m_remoteNode->connectToNode(nodeUrl());
    m_fileEventHandlerIface = m_remoteNode->acquire<FileEventHandlerReplica>();
    qDebug() << "connect handle" <<  m_fileEventHandlerIface;
    connect(m_fileEventHandlerIface, &QRemoteObjectReplica::initialized, this, [=]{
        qDebug() << "initilized" << m_fileEventHandlerIface;
    });
    connect(m_fileEventHandlerIface, &QRemoteObjectReplica::stateChanged, this, [=](QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState){
        qDebug() << "state changed" << state << oldState << m_fileEventHandlerIface;
        if (state == QRemoteObjectReplica::Suspect) {
            qDebug() << "try reconnect";
            m_fileEventHandlerIface->deleteLater();
            m_remoteNode->deleteLater();
            setupConnections();
        }
    });
    connect(this, &RemoteFileEventHelper::handleFileEventRequest, m_fileEventHandlerIface, [=](int type, const QString &arg1, const QString &arg2){
        qDebug() << type << arg1 << arg2 << m_fileEventHandlerIface;
        if (m_fileEventHandlerIface->state() == QRemoteObjectReplica::Valid) {
            QUrl url1(arg1);
            QUrl url2(arg2);
            m_fileEventHandlerIface->handleFileEvent(type, url1.toLocalFile(), url2.toLocalFile());
        }
    });
}

QUrl RemoteFileEventHelper::nodeUrl()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    return QUrl(QStringLiteral("local:lingmo-idm-") + QString(qgetenv("USER")) + display);
}
