// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globals.h"
#include "dsglobal.h"
#include "taskmanagersettings.h"

#include <QJsonObject>
#include <QJsonDocument>

#include <algorithm>
#include <iterator>

#include <string>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

namespace dock {
static inline QString bool2EnableStr(bool enable)
{
    return enable ? QStringLiteral("enabled") : QStringLiteral("disabled");
}

static inline bool enableStr2Bool(QString str)
{
    return str == QStringLiteral("enabled");
}

TaskManagerSettings* TaskManagerSettings::instance()
{
    static TaskManagerSettings* _taskManagerSettings = nullptr;
    if (!_taskManagerSettings) {
        _taskManagerSettings = new TaskManagerSettings();
    }
    return _taskManagerSettings;
}

TaskManagerSettings::TaskManagerSettings(QObject *parent)
    : QObject(parent)
    , m_taskManagerDconfig(DConfig::create(QStringLiteral("org.deepin.dde.shell"), QStringLiteral("org.deepin.ds.dock.taskmanager"), QString(), this))
{
    connect(m_taskManagerDconfig, &DConfig::valueChanged, this, [this](const QString &key){
        if (TASKMANAGER_ALLOWFOCEQUIT_KEY == key) {
            m_allowForceQuit = enableStr2Bool(m_taskManagerDconfig->value(TASKMANAGER_ALLOWFOCEQUIT_KEY).toString());
            Q_EMIT allowedForceQuitChanged();
        } else if (TASKMANAGER_WINDOWSPLIT_KEY == key) {
            m_windowSplit = enableStr2Bool(m_taskManagerDconfig->value(TASKMANAGER_WINDOWSPLIT_KEY).toString());
            Q_EMIT windowSplitChanged();
        } else if (TASKMANAGER_DOCKEDITEMS_KEY == key) {
            loadDockedItems();
            Q_EMIT dockedItemsChanged();
        }
    });

    m_allowForceQuit = enableStr2Bool(m_taskManagerDconfig->value(TASKMANAGER_ALLOWFOCEQUIT_KEY).toString());
    m_windowSplit = enableStr2Bool(m_taskManagerDconfig->value(TASKMANAGER_WINDOWSPLIT_KEY).toString());
    loadDockedItems();
}

bool TaskManagerSettings::isAllowedForceQuit()
{
    return m_allowForceQuit;
}

void TaskManagerSettings::setAllowedForceQuit(bool allowed)
{
    m_allowForceQuit = allowed;
    m_taskManagerDconfig->setValue(TASKMANAGER_ALLOWFOCEQUIT_KEY, bool2EnableStr(m_allowForceQuit));
}

bool TaskManagerSettings::isWindowSplit()
{
    return m_windowSplit;
}

void TaskManagerSettings::setWindowSplit(bool split)
{
    m_windowSplit = split;
    m_taskManagerDconfig->setValue(TASKMANAGER_WINDOWSPLIT_KEY, bool2EnableStr(m_windowSplit));
}

void TaskManagerSettings::dockedItemsPersisted()
{
    QStringList list;

    for (auto dockedDesktopFile : m_dockedItems) {
        if (!dockedDesktopFile.isObject()) continue;
        YAML::Node node;
        auto dockedDesktopFileObj = dockedDesktopFile.toObject();
        for (auto key : dockedDesktopFileObj.keys()) {
            node[key.toStdString()] = dockedDesktopFileObj[key].toString().toStdString();
        }
        auto str = QString::fromStdString(YAML::Dump(node));
        list << str.replace("\n",",");
    }

    m_taskManagerDconfig->setValue(TASKMANAGER_DOCKEDITEMS_KEY, list);
}

void TaskManagerSettings::loadDockedItems()
{
    while (!m_dockedItems.isEmpty()) m_dockedItems.removeLast();

    auto dcokedDesktopFilesStrList = m_taskManagerDconfig->value(TASKMANAGER_DOCKEDITEMS_KEY).toStringList();
    foreach(auto dcokedDesktopFilesStr, dcokedDesktopFilesStrList) {
        YAML::Node node;
        try {
            node = YAML::Load("{" + dcokedDesktopFilesStr.toStdString() + "}");
        } catch (YAML::Exception) {
            qWarning() << "unable to parse docked desktopfiles";
        }

        if (!node.IsMap()) continue;
        auto dockedItem = QJsonObject();
        for (auto it = node.begin(); it != node.end(); ++it) {
            auto key = it->first.as<std::string>();
            auto value = it->second.as<std::string>();
            dockedItem[QString::fromStdString(key)] = QString::fromStdString(value);
        }
        m_dockedItems.append(dockedItem);
    }
}

void TaskManagerSettings::setDockedDesktopFiles(QJsonArray items)
{
    m_dockedItems = items;
    dockedItemsPersisted();
}

void TaskManagerSettings::appnedDockedDesktopfiles(QJsonObject item)
{
    m_dockedItems.append(item);
    dockedItemsPersisted();
}

void TaskManagerSettings::removeDockedDesktopfile(QJsonObject desktopfile)
{
    for (int i = 0; i < m_dockedItems.count(); i++) {
        if (m_dockedItems.at(i) == desktopfile) {
            m_dockedItems.removeAt(i);
            break;
        }
    }
    dockedItemsPersisted();
}

QJsonArray TaskManagerSettings::dockedDesktopFiles()
{
    return m_dockedItems;
}

}
