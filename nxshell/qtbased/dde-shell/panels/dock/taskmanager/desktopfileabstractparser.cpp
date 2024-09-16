// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanagersettings.h"
#include "desktopfileabstractparser.h"

#include <QPointer>
#include <QJsonObject>
#include <QStringLiteral>
#include <QLoggingCategory>

#include <fstream>

Q_LOGGING_CATEGORY(abstractdesktopfileLog, "dde.shell.dock.abstractdesktopfile")


namespace dock {

DesktopfileAbstractParser::DesktopfileAbstractParser(QString id, QObject* parent)
    : QObject(parent)
    , m_id(id)
{
}

DesktopfileAbstractParser::~DesktopfileAbstractParser()
{
}

void DesktopfileAbstractParser::launch()
{

}

void DesktopfileAbstractParser::launchWithAction(const QString& action)
{

}

void DesktopfileAbstractParser::requestQuit()
{

}


QString DesktopfileAbstractParser::id()
{
    return m_id;
}

QString DesktopfileAbstractParser::name()
{
    return "";
}

QString DesktopfileAbstractParser::desktopIcon()
{
    return "application-default-icon";
}

QString DesktopfileAbstractParser::xDeepinVendor()
{
    return "";
}

QList<QPair<QString, QString>> DesktopfileAbstractParser::actions()
{
    return QList<QPair<QString, QString>>();
}

QString DesktopfileAbstractParser::genericName()
{
    return "";
}

QString DesktopfileAbstractParser::identifyWindow(QPointer<AbstractWindow> window)
{
    QString res = QStringLiteral("asbtractAPP:://");
    do {
        if (window->pid() == 0) break;

        auto filePath = QStringLiteral("/proc/%1/cmdline").arg(QString::number(window->pid()));
        std::ifstream fs(filePath.toStdString());
        if (!fs.is_open()) break;
        std::string tmp;
        while (std::getline(fs, tmp, '\0')) {
            res.append(QString::fromStdString(tmp));
        }
    } while(false);

    return res;
}

QString DesktopfileAbstractParser::identifyType()
{
    return QStringLiteral("asbtractAPP");
}

QString DesktopfileAbstractParser::type()
{
    return identifyType();
}

std::pair<bool, QString> DesktopfileAbstractParser::isValied()
{
    return std::make_pair(false, QStringLiteral("abstractdesktopfile has no desktopfile backend"));
}

void DesktopfileAbstractParser::addAppItem(QPointer<AppItem> item)
{
    if (m_appitems.contains(item)) return;

    connect(item.get(), &QObject::destroyed, this, [this](){
        auto item = qobject_cast<AppItem*>(sender());
        m_appitems.removeAll(item);
    });

    m_appitems.append(item);
}

QPointer<AppItem> DesktopfileAbstractParser::getAppItem()
{
    return m_appitems.size() > 0 ? m_appitems.first() : nullptr;
}

bool DesktopfileAbstractParser::isDocked()
{
    if (!isValied().first) {
        qDebug() << isValied().second;
        return false;
    }

    QJsonObject desktopfile;
    desktopfile["type"] = type();
    desktopfile["id"] = id();
    return TaskManagerSettings::instance()->dockedDesktopFiles().contains(desktopfile);
}

void DesktopfileAbstractParser::setDocked(bool docked)
{
    if (!isValied().first && docked) {
        qDebug() << isValied().second;
        return;
    }

    QJsonObject desktopfile;
    desktopfile["type"] = type();
    desktopfile["id"] = id();

    if (docked) {
        TaskManagerSettings::instance()->appnedDockedDesktopfiles(desktopfile);
    } else {
        TaskManagerSettings::instance()->removeDockedDesktopfile(desktopfile);
    }

    Q_EMIT dockedChanged();
}

}
