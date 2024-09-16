// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "kpluginloader.h"
#include "compressorapplication.h"

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include <QSet>
#include <QApplication>
#include <iostream>

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

void loadPlugins_stub(void *obj)
{
    PluginManager *mythis = static_cast<PluginManager *>(obj);

    QString strDir = _LIBRARYDIR;
    QCoreApplication::addLibraryPath(strDir);
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("./"));
    QSet<QString> addedPlugins;
    for (const KPluginMetaData &metaData : plugins) {
        const auto pluginId = metaData.pluginId();
        // Filter out duplicate plugins.
        if (addedPlugins.contains(pluginId)) {
            continue;
        }

        Plugin *plugin = new Plugin(mythis, metaData);
        plugin->setEnabled(true);
        addedPlugins << pluginId;
        mythis->m_plugins << plugin;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Starting UnitTest" << std::endl;
    qputenv("QT_QPA_PLATFORM", "offscreen");
    testing::InitGoogleTest(&argc, argv);
    CompressorApplication a(argc, argv);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif

    return RUN_ALL_TESTS();
}
