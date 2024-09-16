/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "filedialogmanagerservice.h"
#include <QUuid>
#include <QDBusConnection>
FileDialogManagerService::FileDialogManagerService(const QString &service,
                                                   const QString &path)
  : m_service{service},
    m_path{path},
    m_useFileChooserDialog{true},
    m_errorMsg{}
{
    registerAll();
}

FileDialogManagerService::~FileDialogManagerService()
{
    unregisterAll();
}

bool FileDialogManagerService::registerAll()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService(m_service)) {
        m_errorMsg = QString("Cannot register service %1").arg(m_service);
        return false;
    }
    if (!conn.registerObject(m_path, this, QDBusConnection::ExportAllContents)) {
        m_errorMsg = QString("Cannot register object %1").arg(m_path);
        return false;
    }
    return true;
}

bool FileDialogManagerService::unregisterAll()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.unregisterObject(m_path);
    if (!conn.unregisterService(m_service)) {
        m_errorMsg = QString("Cannot unregister service %1").arg(m_service);
        return false;
    }
    return true;
}

QList<QDBusObjectPath> FileDialogManagerService::dialogs()
{
    return m_dialogMap.keys();
}

QDBusObjectPath FileDialogManagerService::createDialog(const QString key)
{
    QString path{"/com/deepin/filemanager/filedialog/"};
    if (key.isEmpty()) {
        path.append(QUuid::createUuid().toRfc4122().toHex());
    } else {
        path.append(key);
    }
    FileDialogService *dialogService = new FileDialogService(path);
    m_dialogMap.insert(QDBusObjectPath{path}, dialogService);
    return QDBusObjectPath{path};
}

void FileDialogManagerService::destroyDialog(const QDBusObjectPath &path)
{
    auto dialog = m_dialogMap.find(path);
    dialog.value()->deleteLater();
    m_dialogMap.erase(dialog);
}

bool FileDialogManagerService::isUseFileChooserDialog()
{
    return m_useFileChooserDialog;
}

QString FileDialogManagerService::errorString()
{
    return m_errorMsg;
}
