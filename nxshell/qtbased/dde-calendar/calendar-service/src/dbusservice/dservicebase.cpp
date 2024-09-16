// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicebase.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QFile>
#include <QtDebug>

DServiceBase::DServiceBase(const QString &path, const QString &interface, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_interface(interface)
{
}

QString DServiceBase::getPath() const
{
    return m_path;
}

QString DServiceBase::getInterface() const
{
    return m_interface;
}

QString DServiceBase::getClientName()
{
    uint pid = QDBusConnection::sessionBus().interface()->servicePid(message().service());
    QString name;
    QFile file(QString("/proc/%1/status").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        name = QString(file.readLine()).section(QRegExp("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
        file.close();
    }
    return name;
}

bool DServiceBase::clientWhite(const int index)
{
//    DeepinAIAssista
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    //根据编号,获取不同到白名单
    static QVector<QStringList> whiteList {{"dde-calendar", "DeepinAIAssistant"}, {"dde-calendar"}, {"dde-calendar"}};
    if (whiteList.size() < index) {
        return false;
    }
    for (int i = 0; i < whiteList.at(index).size(); ++i) {
        if (whiteList.at(index).at(i).contains(getClientName())) {
            return true;
        }
    }
    return false;
#else
    Q_UNUSED(index)
    return true;
#endif
}

void DServiceBase::notifyPropertyChanged(const QString &interface, const QString &propertyName)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        getPath(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, property(propertyName.toUtf8()));
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
