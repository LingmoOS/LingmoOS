// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/convertors/convertorv1_0.h"
#include "utils/searchpluginprotocol.h"
#include "global/matcheditem.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QJsonArray>

GRANDSEARCH_USE_NAMESPACE

TEST(ConvertorV1_0, ut_create)
{
    auto ins = ConvertorV1_0::create();
    EXPECT_NE(ins, nullptr);
    auto ins2 = ConvertorV1_0::create();
    EXPECT_NE(ins, nullptr);
    EXPECT_NE(ins, ins2);

    ConvertorV1_0 *c1 = dynamic_cast<ConvertorV1_0 *>(ins);
    EXPECT_NE(c1, nullptr);

    EXPECT_NO_FATAL_FAILURE(delete c1);
    EXPECT_NO_FATAL_FAILURE(delete ins2);
}

TEST(ConvertorV1_0, ut_version)
{
    auto ins = ConvertorV1_0::create();
    ASSERT_NE(ins, nullptr);

    EXPECT_EQ(ins->version(), QString("1.0"));
    EXPECT_NO_FATAL_FAILURE(delete ins);
}

TEST(ConvertorV1_0, ut_interfaces)
{
    auto ins = ConvertorV1_0::create();
    ASSERT_NE(ins, nullptr);

    auto ifs = ins->interfaces();
    EXPECT_EQ(ifs.value(PLUGININTERFACE_TYPE_SEARCH), &ConvertorV1_0::search);
    EXPECT_EQ(ifs.value(PLUGININTERFACE_TYPE_RESULT), &ConvertorV1_0::result);
    EXPECT_EQ(ifs.value(PLUGININTERFACE_TYPE_STOP), &ConvertorV1_0::stop);
    EXPECT_EQ(ifs.value(PLUGININTERFACE_TYPE_ACTION), &ConvertorV1_0::action);
    EXPECT_NO_FATAL_FAILURE(delete ins);
}

TEST(ConvertorV1_0, ut_search)
{
    QJsonObject out;
    QStringList in;
    EXPECT_EQ(ConvertorV1_0::search(&in, &out), -1);
    in.append("0.1");
    in.append("21111111");
    in.append("test");

    EXPECT_EQ(ConvertorV1_0::search(&in, &out), -1);

    in.clear();
    in.append("1.0");
    in.append("");
    in.append("");
    EXPECT_EQ(ConvertorV1_0::search(&in, &out), 1);

    in.clear();
    in.append("1.0");
    in.append("id");
    in.append("");
    EXPECT_EQ(ConvertorV1_0::search(&in, &out), 1);

    in.clear();
    in.append("1.0");
    in.append("id");
    in.append("test");
    EXPECT_EQ(ConvertorV1_0::search(&in, &out), 0);

    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_VERSION).toString(), QString("1.0"));
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_MISSIONID).toString(), QString("id"));
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_CONTENT).toString(), QString("test"));
}

TEST(ConvertorV1_0, ut_stop)
{
    QJsonObject out;
    QStringList in;

    EXPECT_EQ(ConvertorV1_0::stop(&in, &out), -1);

    in.append("0.1");
    in.append("21111111");
    EXPECT_EQ(ConvertorV1_0::stop(&in, &out), -1);

    in.clear();
    in.append("1.0");
    in.append("");
    EXPECT_EQ(ConvertorV1_0::stop(&in, &out), 1);

    in.clear();
    in.append("1.0");
    in.append("id");
    EXPECT_EQ(ConvertorV1_0::stop(&in, &out), 0);
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_VERSION).toString(), QString("1.0"));
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_MISSIONID).toString(), QString("id"));
}

TEST(ConvertorV1_0, ut_aciton)
{
    QJsonObject out;
    QStringList in;
    EXPECT_EQ(ConvertorV1_0::action(&in, &out), -1);

    in.append("0.1");
    in.append("open");
    in.append("item");
    EXPECT_EQ(ConvertorV1_0::action(&in, &out), -1);

    in.clear();
    in.append("1.0");
    in.append("");
    in.append("");
    EXPECT_EQ(ConvertorV1_0::action(&in, &out), 1);

    in.clear();
    in.append("1.0");
    in.append("open");
    in.append("");
    EXPECT_EQ(ConvertorV1_0::action(&in, &out), 1);

    in.clear();
    in.append("1.0");
    in.append("open");
    in.append("item");
    EXPECT_EQ(ConvertorV1_0::action(&in, &out), 0);

    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_VERSION).toString(), QString("1.0"));
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_ACTION).toString(), QString("open"));
    EXPECT_EQ(out.value(PLUGININTERFACE_PROTOCOL_ITEM).toString(), QString("item"));
}


TEST(ConvertorV1_0, ut_result)
{
    QVariantList in;
    QVariantList out;
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), -1);

    QJsonObject json;
    in.append(QVariant::fromValue(QString()));;
    in.append(QVariant::fromValue((void *)&json));
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), 1);

    in.clear();
    in.append(QVariant::fromValue(QString("plugin")));;
    in.append(QVariant::fromValue((void *)&json));
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), -1);

    json.insert(PLUGININTERFACE_PROTOCOL_VERSION, QJsonValue(QString("0.1")));
    json.insert(PLUGININTERFACE_PROTOCOL_MISSIONID, QJsonValue(QString("id")));
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), -1);

    json.insert(PLUGININTERFACE_PROTOCOL_VERSION, QJsonValue(QString("1.0")));
    json.insert(PLUGININTERFACE_PROTOCOL_MISSIONID, QJsonValue(QString("")));
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), 1);

    json.insert(PLUGININTERFACE_PROTOCOL_MISSIONID, QJsonValue(QString("id")));

    QJsonArray contents;
    {
        QJsonObject group1;
        group1.insert(PLUGININTERFACE_PROTOCOL_GROUP, QJsonValue(QString("g1")));

        QJsonArray items;
        QJsonObject item;
        item.insert(PLUGININTERFACE_PROTOCOL_ITEM, QJsonValue(QString("item1")));
        item.insert(PLUGININTERFACE_PROTOCOL_NAME, QJsonValue(QString("name1")));
        item.insert(PLUGININTERFACE_PROTOCOL_TYPE, QJsonValue(QString("type1")));
        item.insert(PLUGININTERFACE_PROTOCOL_ICON, QJsonValue(QString("icon1")));
        items.append(item);

        item.insert(PLUGININTERFACE_PROTOCOL_ITEM, QJsonValue(QString("item2")));
        item.insert(PLUGININTERFACE_PROTOCOL_NAME, QJsonValue(QString("name2")));
        items.append(item);

        group1.insert(PLUGININTERFACE_PROTOCOL_ITEMS, items);
        contents.append(group1);
    }

    {
        QJsonObject group2;
        group2.insert(PLUGININTERFACE_PROTOCOL_GROUP, QJsonValue(QString("g2")));

        QJsonArray items;
        QJsonObject item;
        item.insert(PLUGININTERFACE_PROTOCOL_ITEM, QJsonValue(QString("item3")));
        item.insert(PLUGININTERFACE_PROTOCOL_NAME, QJsonValue(QString("name3")));
        item.insert(PLUGININTERFACE_PROTOCOL_TYPE, QJsonValue(QString("type3")));
        item.insert(PLUGININTERFACE_PROTOCOL_ICON, QJsonValue(QString("icon3")));
        items.append(item);

        item.insert(PLUGININTERFACE_PROTOCOL_ITEM, QJsonValue(QString("item4")));
        item.insert(PLUGININTERFACE_PROTOCOL_NAME, QJsonValue(QString("name4")));
        items.append(item);

        group2.insert(PLUGININTERFACE_PROTOCOL_ITEMS, items);
        contents.append(group2);
    }
    json.insert(PLUGININTERFACE_PROTOCOL_CONTENT, contents);
    out.clear();
    EXPECT_EQ(ConvertorV1_0::result(&in, &out), 0);
    ASSERT_EQ(out.size(), 2);

    EXPECT_EQ(out.at(0).toString(), QString("id"));
    auto results = out.at(1).value<GrandSearch::MatchedItemMap>();
    ASSERT_EQ(results.size(), 2);

    {
        ASSERT_TRUE(results.contains("g1"));
        auto items = results.value("g1");
        ASSERT_EQ(items.size(), 2);

        {
            auto item = items.at(0);
            EXPECT_EQ(item.item, QString("item1"));
            EXPECT_EQ(item.name, QString("name1"));
            EXPECT_EQ(item.type, QString("type1"));
            EXPECT_EQ(item.icon, QString("icon1"));
            EXPECT_EQ(item.searcher, QString("plugin"));
        }

        {
            auto item = items.at(1);
            EXPECT_EQ(item.item, QString("item2"));
            EXPECT_EQ(item.name, QString("name2"));
            EXPECT_EQ(item.type, QString("type1"));
            EXPECT_EQ(item.icon, QString("icon1"));
            EXPECT_EQ(item.searcher, QString("plugin"));
        }
    }

    {
        ASSERT_TRUE(results.contains("g2"));
        auto items = results.value("g2");
        ASSERT_EQ(items.size(), 2);

        {
            auto item = items.at(0);
            EXPECT_EQ(item.item, QString("item3"));
            EXPECT_EQ(item.name, QString("name3"));
            EXPECT_EQ(item.type, QString("type3"));
            EXPECT_EQ(item.icon, QString("icon3"));
            EXPECT_EQ(item.searcher, QString("plugin"));
        }
        {
            auto item = items.at(1);
            EXPECT_EQ(item.item, QString("item4"));
            EXPECT_EQ(item.name, QString("name4"));
            EXPECT_EQ(item.type, QString("type3"));
            EXPECT_EQ(item.icon, QString("icon3"));
            EXPECT_EQ(item.searcher, QString("plugin"));
        }
    }
}
