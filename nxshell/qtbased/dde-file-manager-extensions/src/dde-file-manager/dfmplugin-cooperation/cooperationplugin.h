// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONPLUGIN_H
#define COOPERATIONPLUGIN_H

#include <dfm-framework/dpf.h>

namespace dfmplugin_cooperation {

class CooperationPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "cooperationplugin.json")

    DPF_EVENT_NAMESPACE(dfmplugin_cooperation)

    // Plugin interface
public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void addCooperationSettingItem();

private Q_SLOTS:
    void bindMenuScene();
    void onMenuSceneAdded(const QString &scene);
};

}   // namespace dfmplugin_cooperation

#endif   // COOPERATIONPLUGIN_H
