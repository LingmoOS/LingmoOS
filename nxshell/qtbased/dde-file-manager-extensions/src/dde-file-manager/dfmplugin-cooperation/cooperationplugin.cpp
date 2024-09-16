// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationplugin.h"
#include "menu/cooperationmenuscene.h"
#include "utils/cooperationhelper.h"
#include "configs/settings/configmanager.h"

#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>

#include <QTranslator>

#include <reportlog/reportlogmanager.h>

#define COOPERATION_SETTING_GROUP "10_advance.03_cooperation"
inline constexpr char kCooperationSettingGroup[] { COOPERATION_SETTING_GROUP };
inline constexpr char kCooperationSettingTransfer[] { "00_file_transfer" };
inline constexpr char kParentScene[] { "ExtendMenu" };

using namespace dfmbase;
using namespace dfmplugin_cooperation;

void CooperationPlugin::initialize()
{
    deepin_cross::ReportLogManager::instance()->init();
    auto translator = new QTranslator(this);
    translator->load(QLocale(), "cooperation-transfer", "_", "/usr/share/dde-file-manager/translations");
    QCoreApplication::installTranslator(translator);

    if (DPF_NAMESPACE::LifeCycle::isAllPluginsStarted())
        bindMenuScene();
    else
        connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &CooperationPlugin::bindMenuScene, Qt::DirectConnection);
}

bool CooperationPlugin::start()
{
    // 加载跨端配置
    auto appName = qApp->applicationName();
    qApp->setApplicationName("dde-cooperation");
    ConfigManager::instance();
    qApp->setApplicationName(appName);

    // 添加文管设置
    if (appName == "dde-file-manager")
        addCooperationSettingItem();

    return true;
}

void CooperationPlugin::addCooperationSettingItem()
{
    SettingJsonGenerator::instance()->addGroup(kCooperationSettingGroup, tr("File transfer"));

    CustomSettingItemRegister::instance()->registCustomSettingItemType("pushbutton", CooperationHelper::createSettingButton);
    QVariantMap config {
        { "key", kCooperationSettingTransfer },
        { "name", QObject::tr("File transfer settings") },
        { "type", "pushbutton" },
        { "default", QObject::tr("Settings", "button") }
    };

    QString key = QString("%1.%2").arg(kCooperationSettingGroup, kCooperationSettingTransfer);
    SettingJsonGenerator::instance()->addConfig(key, config);
}

void CooperationPlugin::bindMenuScene()
{
    dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_RegisterScene", CooperationMenuCreator::name(), new CooperationMenuCreator);

    bool ret = dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Contains", QString(kParentScene)).toBool();
    if (ret) {
        dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Bind", CooperationMenuCreator::name(), QString(kParentScene));
    } else {
        dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &CooperationPlugin::onMenuSceneAdded);
    }
}

void CooperationPlugin::onMenuSceneAdded(const QString &scene)
{
    if (scene == kParentScene) {
        dpfSlotChannel->push("dfmplugin_menu", "slot_MenuScene_Bind", CooperationMenuCreator::name(), QString(kParentScene));
        dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &CooperationPlugin::onMenuSceneAdded);
    }
}
