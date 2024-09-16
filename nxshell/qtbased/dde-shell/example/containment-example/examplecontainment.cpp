// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "examplecontainment.h"

#include "pluginfactory.h"
#include "pluginloader.h"
#include <DConfig>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

ExampleContainment::ExampleContainment(QObject *parent)
    : DContainment(parent)
{

}

ExampleContainment::~ExampleContainment()
{
    DCORE_USE_NAMESPACE;
    auto targetApplet = targetPlugin();
    QJsonArray res;
    for (auto item : applets()) {
        if (targetApplet == item->pluginMetaData()) {
            QJsonObject data;
            data["PluginId"] = item->pluginId();
            data["Id"] = item->id();
            res << data;
        }
    }
    if (!res.isEmpty()) {
        const QJsonDocument doc(res);
        std::unique_ptr<DConfig> config(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.example"));
        config->setValue("appletExamples", doc.toVariant());
    }
}

bool ExampleContainment::load()
{
    DCORE_USE_NAMESPACE;
    std::unique_ptr<DConfig> config(DConfig::create("org.deepin.dde.shell", "org.deepin.ds.example"));

    QList<DAppletData> groups;
    auto children = DPluginLoader::instance()->childrenPlugin(pluginId());
    const auto appletExamples = config->value("appletExamples");
    const auto doc = QJsonDocument::fromVariant(appletExamples);
    const auto applets = doc.array();
    if (!applets.isEmpty()) {
        for (auto item : applets) {
            auto object = item.toObject();
            QVariantMap group;
            group["PluginId"] = object["PluginId"].toString();
            group["Id"] = object["Id"].toString();
            groups << DAppletData(group);
        }
        children.removeAll(targetPlugin());
    }
    for (auto item : children) {
        groups << DAppletData::fromPluginMetaData(item);
    }
    auto data = appletData();
    data.setGroupList(groups);
    setAppletData(data);

    return DApplet::load();
}

DPluginMetaData ExampleContainment::targetPlugin() const
{
    auto children = DPluginLoader::instance()->childrenPlugin(pluginId());
    Q_ASSERT(children.size() >= 1);
    return children.takeFirst();
}

D_APPLET_CLASS(ExampleContainment)

#include "examplecontainment.moc"
