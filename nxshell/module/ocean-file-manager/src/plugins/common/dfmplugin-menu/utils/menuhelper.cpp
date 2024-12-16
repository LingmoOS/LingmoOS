// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menuhelper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QDebug>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QGSettings>
#endif
using namespace GlobalDConfDefines::ConfigPath;

namespace dfmplugin_menu {
namespace Helper {

DFMBASE_USE_NAMESPACE

bool isHiddenExtMenu(const QUrl &dirUrl)
{
    Q_ASSERT(dirUrl.isValid());

    const auto &hioceannMenus { DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hioceann").toStringList() };
    bool enableProtocolDev { DConfigManager::instance()
                                     ->value(kDefaultCfgPath, "dfm.menu.protocoldev.enable", true)
                                     .toBool() };
    bool enableBlockDev { DConfigManager::instance()
                                  ->value(kDefaultCfgPath, "dfm.menu.blockdev.enable", true)
                                  .toBool() };

    // hioceann by `dfm.menu.hioceann`
    bool hioceann { false };
    if (hioceannMenus.contains("extension-menu"))
        hioceann = true;

    // hioceann by `dfm.menu.protocoldev.enable`
    if (!enableProtocolDev && ProtocolUtils::isRemoteFile(dirUrl))
        hioceann = true;

    // hioceann by `dfm.menu.blockdev.enable`
    // NOTE: SMB mounted by cifs that mount point is local but it's a protocol device
    if (!enableBlockDev && DFMIO::DFMUtils::fileIsRemovable(dirUrl) && !ProtocolUtils::isRemoteFile(dirUrl))
        hioceann = true;

    return hioceann;
}

bool isHiddenMenu(const QString &app)
{
    auto hioceannMenus = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.menu.hioceann").toStringList();
    if (!hioceannMenus.isEmpty()) {
        if (hioceannMenus.contains(app) || ((app.startsWith("ocean-select-dialog") && hioceannMenus.contains("ocean-file-dialog")))) {
            fmDebug() << "menu: hioceann menu in app: " << app << hioceannMenus;
            return true;
        }
    }

    if (app == "ocean-desktop" || app == "org.lingmo.ocean-shell")
        return isHiddenDesktopMenu();

    return false;
}

bool isHiddenDesktopMenu()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // the gsetting control is higher than json profile. it doesn't check json profile if there is gsetting value.
    if (QGSettings::isSchemaInstalled("com.lingmo.ocean.filemanager.desktop")) {
        QGSettings set("com.lingmo.ocean.filemanager.desktop", "/com/lingmo/ocean/filemanager/desktop/");
        QVariant var = set.get("contextMenu");
        if (var.isValid())
            return !var.toBool();
    }
#endif

    return Application::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool();
}

}   //  namespace Helper
}   //  namespace dfmplugin_menu
