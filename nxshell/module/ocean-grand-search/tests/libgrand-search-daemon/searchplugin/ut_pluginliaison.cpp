// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/pluginliaison_p.h"
#include "searchplugin/pluginliaison.h"
#include "searchplugin/convertors/dataconvertor.h"
#include "grand-search/utils/utils.h"
#include "global/builtinsearch.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

// 因为#include <QTest>, qApp会变成static_cast<QGuiApplication *>(QCoreApplication::instance()
// 而main函数中定义的是QCoreApplication，此转换会导致报内存泄露，这里将qApp重新定义回QCoreApplication::instance()
#define qApp QCoreApplication::instance()

GRANDSEARCH_USE_NAMESPACE

TEST(PluginLiaison, ut_init)
{
    stub_ext::StubExt st;
    bool sup = true;
    st.set_lamda(&DataConvertor::isSupported, [&sup]() {
        return sup;
    });

    PluginLiaison pl;
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("","1","2","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","","2","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","3","","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","3","4",""));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    //版本不支持
    sup = false;
    EXPECT_FALSE(pl.init("1","2","3","4","5"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    sup = true;
    EXPECT_TRUE(pl.init("1","2","3","4","5"));
    EXPECT_NE(pl.d->m_inteface, nullptr);

    // 重复初始化
    EXPECT_FALSE(pl.init("1","2","3","4","5"));
}

TEST(PluginLiaison, ut_isVaild)
{
    stub_ext::StubExt st;
    bool sup = true;
    st.set_lamda(&QDBusAbstractInterface::isValid, [&sup]() {
        return sup;
    });

    PluginLiaison pl;
    EXPECT_FALSE(pl.isVaild());

    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_ver.clear();
    EXPECT_FALSE(pl.isVaild());

    pl.d->m_ver = "1.0";
    EXPECT_TRUE(pl.isVaild());

    sup = false;
    EXPECT_FALSE(pl.isVaild());
}

TEST(PluginLiaison, ut_search)
{
    stub_ext::StubExt st;
    bool search = false;
    st.set_lamda(&SearchPluginInterfaceV1::Search, [&search]() {
        search = true;
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                          "org.freedesktop.DBus.Properties", "Get");
        auto var = QVariant::fromValue(QString("test"));
        auto reply = msg.createReply(var);
        auto ret = QDBusPendingReply<QString>(reply);
        return ret;
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_SEARCH);
        return 0;
    });

    PluginLiaison pl;
    pl.d->m_inteface = new SearchPluginInterfaceV1("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus.Properties",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_inteface ->setTimeout(10);
    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.search("task", "test"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_FALSE(search);

    search = false;
    pl.d->m_ver = "test";
    EXPECT_TRUE(pl.search("task", "test"));
    EXPECT_TRUE(search);
    EXPECT_TRUE(pl.d->m_searching.load());
    ASSERT_NE(pl.d->m_replyWatcher, nullptr);

    EXPECT_EQ(pl.d->m_replyWatcher->thread(), qApp->thread());

    //todo dbus mmleak
    QTest::qWaitFor([&](){
        return pl.d->m_replyWatcher->isFinished();
    }, 50);

    EXPECT_TRUE(pl.d->m_replyWatcher->isError());

    search = false;
    EXPECT_FALSE(pl.search("task2", "test2"));
    EXPECT_FALSE(search);
}

TEST(PluginLiaison, ut_stop)
{
    stub_ext::StubExt st;
    bool stop = false;
    st.set_lamda(&SearchPluginInterfaceV1::Stop, [&stop]() {
        stop = true;
        return QDBusPendingReply<bool>(QDBusMessage::createError(QDBusError::NoReply, ""));
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_STOP);
        return 0;
    });

    PluginLiaison pl;
    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_searching.store(false);

    EXPECT_TRUE(pl.stop("test"));

    pl.d->m_searching.store(true);
    EXPECT_TRUE(pl.stop(""));
    EXPECT_FALSE(pl.d->m_searching.load());

    pl.d->m_searching.store(true);

    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.stop("task"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_FALSE(stop);

    pl.d->m_ver = "test";
    stop = false;
    pl.d->m_searching.store(true);
    EXPECT_TRUE(pl.stop("task"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_TRUE(stop);
}

TEST(PluginLiaison, ut_action)
{
    stub_ext::StubExt st;
    bool action = false;
    st.set_lamda(&SearchPluginInterfaceV1::Action, [&action]() {
        action = true;
        return QDBusPendingReply<bool>(QDBusMessage::createError(QDBusError::NoReply, ""));
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_ACTION);
        return 0;
    });

    PluginLiaison pl;
    EXPECT_FALSE(pl.action("", ""));
    EXPECT_FALSE(pl.action("test", ""));
    EXPECT_FALSE(pl.action("test", "test"));
    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);

    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.action("test", "test"));
    EXPECT_FALSE(action);

    action = false;
    pl.d->m_ver = "test";
    EXPECT_TRUE(pl.action("test", "test"));
    EXPECT_TRUE(action);
}

TEST(PluginLiaison, ut_onServiceStarted)
{
    PluginLiaison pl;
    EXPECT_NO_FATAL_FAILURE(pl.d->onServiceStarted("demo", "", ""));

    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    bool registered = true;
    stub_ext::StubExt st;
    st.set_lamda(&QDBusConnectionInterface::isServiceRegistered, [&registered]()->QDBusReply<bool> {
        QDBusMessage msg;
        return msg.createReply(QVariant::fromValue(registered));
    });

    bool ready = false;
    pl.connect(&pl, &PluginLiaison::ready, &pl, [&ready](){ ready = true;}, Qt::DirectConnection);

    pl.d->onServiceStarted("demo", "", "");
    EXPECT_FALSE(ready);

    ready = false;
    registered = false;
    pl.d->onServiceStarted("service", "", "");
    EXPECT_FALSE(ready);

    ready = false;
    registered = true;
    pl.d->onServiceStarted("service", "", "");
    EXPECT_TRUE(ready);
}

TEST(PluginLiaison, ut_parseResult_0)
{
    PluginLiaison pl;
    QString json;

    // 对fromJson进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&QJsonDocument::fromJson, [](const QByteArray &json, QJsonParseError *error){
        error->error = QJsonParseError::UnterminatedObject;
        QJsonDocument doc;
        return doc;
    });
    
    // 对doc.object()进行打桩
    bool root = false;
    stu.set_lamda(&QJsonDocument::object, [&root](){
        root = true;
        QJsonObject obj;
        return obj;
    });

    bool result = false;
    pl.connect(&pl, &PluginLiaison::searchFinished, &pl, [&result](){result = true;});
    pl.d->parseResult(json, pl.d);
    EXPECT_TRUE(result);
    EXPECT_FALSE(root);
}

TEST(PluginLiaison, ut_parseResult_1)
{
    PluginLiaison pl;
    QString json;

    // 对fromJson进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&QJsonDocument::fromJson, [](const QByteArray &json, QJsonParseError *error){
        error->error = QJsonParseError::NoError;
        QJsonDocument doc;
        return doc;
    });

    // 对loadAcpuire进行打桩
    bool load = true;
    stu.set_lamda(&QAtomicInteger<bool>::loadAcquire, [&load](){
        load = false;
        return load;
    });

    pl.d->parseResult(json, pl.d);
    EXPECT_FALSE(load);
}

TEST(PluginLiaison, ut_parseResult_2)
{
    PluginLiaison pl;
    QString json;

    // 对fromJson进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&QJsonDocument::fromJson, [](const QByteArray &json, QJsonParseError *error){
        error->error = QJsonParseError::NoError;
        QJsonDocument doc;
        return doc;
    });

    // 对loadAcpuire进行打桩
    stu.set_lamda(&QAtomicInteger<bool>::loadAcquire, [](){
        return true;
    });

    // 对doc.object()进行打桩
    stu.set_lamda(&QJsonDocument::object, [](){
        QJsonObject obj;
        return obj;
    });

    // 对DataConvIns->convert进行打桩
    stu.set_lamda(&DataConvertor::convert, [](DataConvertor*, const QString &version, const QString &type, void *in, void *out)->int{
        QVariantList ret;
        QMap<QString, GrandSearch::MatchedItems> map;
        GrandSearch::MatchedItems items;
        GrandSearch::MatchedItem item;
        QVariant extra;
        QVariantHash itemWeight({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 32}});
        extra = QVariant::fromValue(itemWeight);
        item = {"a", "a", "a", GRANDSEARCH_GROUP_FILE, " ", extra};
        items.append(item);
        itemWeight.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 27);
        extra = QVariant::fromValue(itemWeight);
        item = {"a", "ab", "ab", GRANDSEARCH_GROUP_FILE, " ", extra};
        items.append(item);
        map.insert(GRANDSEARCH_GROUP_FILE, items);
        ret.append("test");
        ret.append(QVariant::fromValue(map));
        out = &ret;
        return 1;
    });

    pl.d->m_searching = true;
    bool result = false;
    pl.connect(&pl, &PluginLiaison::searchFinished, &pl, [&result](GrandSearch::MatchedItemMap items){result = true;});
    pl.d->parseResult(json, pl.d);
    EXPECT_TRUE(result);
}
