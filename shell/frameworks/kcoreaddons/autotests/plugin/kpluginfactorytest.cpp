/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <QPluginLoader>
#include <kpluginfactory.h>
#ifndef Q_OS_WIN
#include "plugins.h"
#endif

class KPluginFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
#ifndef Q_OS_WIN
        KPluginMetaData data(QStringLiteral("namespace/jsonplugin_cmake_macro"));
        QVERIFY(data.isValid());
        KPluginFactory::Result<KPluginFactory> factoryResult = KPluginFactory::loadFactory(data);
        auto factory = factoryResult.plugin;
        QVERIFY(factory);

        const QString testProp = QStringLiteral("testme");
        QObject *obj = factory->create<MyPlugin>(this, {testProp});
        QVERIFY(obj);
        QCOMPARE(obj->metaObject()->className(), "SimplePluginClass");
        QCOMPARE(obj->property("arg").toString(), testProp);

        QObject *obj2 = factory->create<MyPlugin2>(this);
        QVERIFY(obj2);
        QCOMPARE(obj2->metaObject()->className(), "SimplePluginClass2");
        QVERIFY(obj != obj2);
        delete obj;
        delete obj2;

        // Try creating a part without keyword/args
        QWidget parentWidget;
        QObject *partTest = factory->create<QObject>(&parentWidget, this);
        QVERIFY(partTest);
        delete partTest;
#endif
    }

    void testPluginWithoutMetaData()
    {
        KPluginFactory::Result<KPluginFactory> factoryResult = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("namespace/pluginwithoutmetadata")));
        QVERIFY(factoryResult);
        auto plugin = factoryResult.plugin->create<QObject>();
        QVERIFY(plugin);
        QCOMPARE(plugin->metaObject()->className(), "PluginWithoutMetaData");
        delete plugin;
    }

    void testCreateUsingUtilityMethods()
    {
        auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("namespace/jsonplugin_cmake_macro")));
        QVERIFY(result.plugin);
        QCOMPARE(result.plugin->metaObject()->className(), "SimplePluginClass");
        QVERIFY(result.errorString.isEmpty());
        QCOMPARE(result.errorReason, KPluginFactory::NO_PLUGIN_ERROR);
        delete result.plugin;
    }

    void testCreateUsingUtilityMethodsErrorHandling()
    {
        {
            auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QFINDTESTDATA("data/jsonplugin.json")));
            QVERIFY(!result.plugin);
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_PLUGIN);
        }
        {
            // it is a valid plugin, but does not contain a KPluginFactory
            QVERIFY(QPluginLoader(QStringLiteral("namespace/qtplugin")).instance());
            auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("namespace/qtplugin")));
            QVERIFY(!result.plugin);
            // But does not contain a valid plugin factory
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_FACTORY);
        }
        {
            // it is a QObject, but not a KPluginFactoryTest instance
            auto result = KPluginFactory::instantiatePlugin<KPluginFactoryTest>(KPluginMetaData(QStringLiteral("namespace/jsonplugin_cmake_macro")));
            QVERIFY(!result.plugin);
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION);
            QVERIFY(result.errorText.contains(QStringLiteral("KPluginFactoryTest")));
        }
    }

    void testStaticPlugins()
    {
        const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("staticnamespace"));
        QCOMPARE(plugins.count(), 1);
        auto factory = KPluginFactory::loadFactory(plugins.first()).plugin;
        QCOMPARE(factory->metaObject()->className(), "static_jsonplugin_cmake_macro_factory");
        auto result = KPluginFactory::instantiatePlugin<QObject>(plugins.first());
        QVERIFY(result);
        delete result.plugin;
    }

    void testNonExistingPlugin()
    {
        KPluginMetaData data(QStringLiteral("does/not/exist"));
        QVERIFY(!data.isValid());
        const auto res = KPluginFactory::instantiatePlugin<QObject>(data);
        QVERIFY(!res);
        QCOMPARE(res.errorReason, KPluginFactory::INVALID_PLUGIN);
        QCOMPARE(res.errorText, QStringLiteral("Could not find plugin does/not/exist"));
    }

    void testInstantiateWidget()
    {
        KPluginMetaData data(QStringLiteral("widgets/widgetsplugin"), KPluginMetaData::AllowEmptyMetaData);
        QVERIFY(data.isValid());
        auto factory = KPluginFactory::loadFactory(data).plugin;
        QVERIFY(factory);

        // This has two constructors, make sure we choose the first one that takes args
        auto obj = factory->create<QWidget>(nullptr, QVariantList{true});
        QVERIFY(obj);
        QVERIFY(obj->property("firstarg").toBool());
        delete obj;
    }
};

QTEST_MAIN(KPluginFactoryTest)

#include "kpluginfactorytest.moc"
