// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"

#include <DAppLoader>
#include <DPathBuf>
#include <QSignalSpy>

#include "private/dapploader_p.h"
#include <DQmlAppPreloadInterface>
#include <DQmlAppMainWindowInterface>
#include <QQmlApplicationEngine>

DQUICK_USE_NAMESPACE
DCORE_USE_NAMESPACE

static constexpr char const *AppId = "org.deepin.dtkdeclarative.unit-test";
static const QString envPath(DPathBuf("./env").toString());
static const QStringList ldPaths{DPathBuf("./ld1").toString(), DPathBuf("ld2").toString()};
static const QStringList apiPaths{DPathBuf("./api1").toString(), DPathBuf("./api2").toString()};
static const QString constructPath(DPathBuf("./cpath").toString());

class ut_AppLoader : public ::testing::Test
{
public:
};

TEST_F(ut_AppLoader, instance)
{
    DAppLoader loader(AppId);
    ASSERT_EQ(DAppLoader::instance(), &loader);
}

TEST_F(ut_AppLoader, buildinPluginPath)
{
#ifdef DTK_QML_APP_PLUGIN_PATH
    DAppLoader loader(AppId);
    ASSERT_EQ(loader.pluginPaths().last(), DTK_QML_APP_PLUGIN_PATH);
#endif

    {
        EnvGuard guard("DTK_QML_PLUGIN_PATH", envPath);
        Q_UNUSED(guard)
        DAppLoader loader(AppId);
        ASSERT_EQ(loader.pluginPaths().first(), envPath);
    }
    {
        EnvGuard guard("LD_LIBRARY_PATH", ldPaths.join(':'));
        Q_UNUSED(guard)
        DAppLoader loader(AppId);
        const QString path = loader.pluginPaths().first();
        ASSERT_TRUE(path.startsWith(ldPaths[0]));
    }
    {
        EnvGuard guard("DTK_QML_PLUGIN_PATH", envPath);
        Q_UNUSED(guard)
        EnvGuard guard2("LD_LIBRARY_PATH", ldPaths.join(':'));
        Q_UNUSED(guard2)
        DAppLoader loader(AppId);
        ASSERT_EQ(loader.pluginPaths().first(), envPath);
    }
}

TEST_F(ut_AppLoader, addPluginPath)
{
    EnvGuard guard("DTK_QML_PLUGIN_PATH", envPath);
    Q_UNUSED(guard)
    EnvGuard guard2("LD_LIBRARY_PATH", ldPaths.join(':'));
    Q_UNUSED(guard2)
    {
        DAppLoader loader(AppId, constructPath);
        const QString path = loader.pluginPaths().first();
        ASSERT_TRUE(path.startsWith(constructPath));
    }
    {
        DAppLoader loader(AppId, constructPath);
        loader.addPluginPath(apiPaths[0]);
        loader.addPluginPath(apiPaths[1]);
        const QString path = loader.pluginPaths().first();
        ASSERT_TRUE(path.startsWith(apiPaths[1]));
    }
}

class AppLoaderSimulator : public DAppLoader
{
public:
    AppLoaderSimulator()
        : DAppLoader(AppId, APPLOADER_PLUGINDUMP_OUTPUT_DIR)
    {
        d = DAppLoader::d_func();

        d->engine = new QQmlApplicationEngine();
        updateEngineImportPath(*d->engine);
    }
    ~AppLoaderSimulator()
    {
        // don't release app.
        d->app.take();
    }
    int load()
    {
        d->ensureLoadPreload();
        int argc = 0;
        d->app.reset(d->preloadInstance->creatApplication(argc, nullptr));

        if (!d->app)
            return -1;

        d->preloadInstance->aboutToPreload(d->engine);
        QObject::connect(d->engine, &QQmlApplicationEngine::objectCreated, this, [this](QObject *obj, const QUrl &url) {
            d->_q_onPreloadCreated(obj, url);
        });
        d->engine->load(d->preloadInstance->preloadComponentPath());
        if (d->engine->rootObjects().isEmpty())
            return -1;
        return 0;
    }
    DAppLoaderPrivate *d;
};

TEST_F(ut_AppLoader, exec)
{
    AppLoaderSimulator loader;
    QSignalSpy spy(&loader, SIGNAL(loadFinished()));
    int exitCode = loader.load();
    ASSERT_EQ(exitCode, 0);

    EXPECT_TRUE(QTest::qWaitFor([this, &spy]() {
        return spy.count() >= 1;
    }));
}
