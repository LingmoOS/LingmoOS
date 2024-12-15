// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsdockedhelper.h"

#include <QJsonObject>
#include <QJsonDocument>

#include <qcontainerfwd.h>
#include <yaml-cpp/yaml.h>

namespace apps {
AppsDockedHelper* AppsDockedHelper::instance()
{
    static AppsDockedHelper* _instance = nullptr;

    if (_instance == nullptr) {
        _instance = new AppsDockedHelper;
    }

    return _instance;
}

AppsDockedHelper::AppsDockedHelper(QObject *parent)
    : QObject(parent)
    , m_config(DConfig::create(QStringLiteral("org.lingmo.ocean.shell"), QStringLiteral("org.lingmo.ds.dock.taskmanager"), QString(), this))
{
    // TODO: remove yaml and rewrite this
    auto updateDockedDesktopfiles = [this](){
        m_dockedDesktopIDs.clear();
        auto dcokedDesktopFilesStrList = m_config->value("Docked_Items").toStringList();
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
            if (dockedItem["type"] == "amAPP") {
                m_dockedDesktopIDs.insert(dockedItem["id"].toString());
            }
        }
    };

    connect(m_config, &DConfig::valueChanged, this, [this, updateDockedDesktopfiles](const QString &key){
        if (key != "Docked_Items") return;
        updateDockedDesktopfiles();
    });

    updateDockedDesktopfiles();
}

bool AppsDockedHelper::isDocked(const QString &appItemId) const
{
    return m_dockedDesktopIDs.contains(appItemId.chopped(8));
}

void AppsDockedHelper::setDocked(const QString &appId, bool docked)
{
    // TODO
}
}

