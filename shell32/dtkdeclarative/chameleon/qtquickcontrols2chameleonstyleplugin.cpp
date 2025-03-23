// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qtquickcontrols2chameleonstyleplugin.h"
//#include "dquickpaletteprovider_p.h"

#include <qqml.h>

//DQUICK_USE_NAMESPACE

QtQuickControls2ChameleonStylePlugin::~QtQuickControls2ChameleonStylePlugin()
{
    // 恢复到使用QQuickPaletteProvider
//    DQuickPaletteProvider::cleanup();
}

#ifdef USE_QQuickStylePlugin
QString QtQuickControls2ChameleonStylePlugin::name() const
{
    return QStringLiteral("Chameleon");
}
#endif

void QtQuickControls2ChameleonStylePlugin::registerTypes(const char *uri)
{
    // 为 DPalette/QPalette 类型注册QQmlValueTypeProvider
//    DQuickPaletteProvider::init(); // Use the QPalette in the dtk style, don't do custom of palette

    qmlRegisterModule(uri, 2, 0);
}
