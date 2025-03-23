// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DAppLoader>
#include <QGuiApplication>

DQUICK_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DAppLoader appLoader("org.deepin.%{ProjectName}");
#ifdef PLUGINPATH
    appLoader.addPluginPath(PLUGINPATH);
#endif
#ifdef APP_PLUGIN_PATH
    appLoader.addPluginPath(APP_PLUGIN_PATH);
#endif
    return appLoader.exec(argc, argv);
}
