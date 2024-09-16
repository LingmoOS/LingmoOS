/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#ifndef FILEDIALOGMANAGERSERVICE_H
#define FILEDIALOGMANAGERSERVICE_H
#include <QDBusContext>
#include <QDBusObjectPath>
#include "filedialogservice.h"
#include <QDBusConnection>

class FileDialogManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.filedialogmanager")
    Q_CLASSINFO("D-Bus Introspection", ""
                                       "  <interface name=\"com.deepin.filemanager.filedialogmanager\">\n"
                                       "    <method name=\"createDialog\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"key\"/>\n"
                                       "      <arg direction=\"out\" type=\"o\" name=\"path\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"destroyDialog\">\n"
                                       "      <arg direction=\"in\" type=\"o\" name=\"path\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"dialogs\">\n"
                                       "      <arg direction=\"out\" type=\"ao\" name=\"dialogObjectList\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"errorString\">\n"
                                       "      <arg direction=\"out\" type=\"s\" name=\"error\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"isUseFileChooserDialog\">\n"
                                       "      <arg direction=\"out\" type=\"b\" name=\"ok\"/>\n"
                                       "    </method>\n"
                                       "  </interface>\n"
                                       "")

public:
    FileDialogManagerService(const QString &service, const QString &path);
    ~FileDialogManagerService();

public Q_SLOTS:
    Q_SCRIPTABLE QDBusObjectPath createDialog(const QString key);
    void destroyDialog(const QDBusObjectPath &path);
    QList<QDBusObjectPath> dialogs();
    QString errorString();
    bool isUseFileChooserDialog();

private:
    bool registerAll();
    bool unregisterAll();
    QMap<QDBusObjectPath, FileDialogService*> m_dialogMap;
    QString m_service;
    QString m_path;
    bool m_useFileChooserDialog;
    QString m_errorMsg;
};

#endif  // FILEDIALOGMANAGERSERVICE_H
