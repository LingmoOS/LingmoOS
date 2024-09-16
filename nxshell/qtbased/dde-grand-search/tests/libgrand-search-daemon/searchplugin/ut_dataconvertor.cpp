// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/convertors/dataconvertor.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(DataConvertor, ut_initConvetor)
{
    DataConvertor dc;
    ASSERT_TRUE(dc.m_convertors.isEmpty());

    dc.m_inited = true;
    dc.initConvetor();
    ASSERT_TRUE(dc.m_convertors.isEmpty());

    dc.m_inited = false;
    dc.initConvetor();
    EXPECT_FALSE(dc.m_convertors.isEmpty());
    EXPECT_TRUE(dc.m_inited);
}

TEST(DataConvertor, ut_isSupported)
{
    DataConvertor dc;
    dc.m_convertors.insert("test", nullptr);

    EXPECT_FALSE(dc.isSupported("1.0"));
    EXPECT_TRUE(dc.isSupported("test"));
}

static AbstractConvertor* createConvertor()
{
    return nullptr;
}

TEST(DataConvertor, ut_regist)
{
    DataConvertor dc;
    dc.m_convertors.insert("test", createConvertor);
    EXPECT_FALSE(dc.regist("test", (CreateConvertor)0x1));
    EXPECT_EQ(dc.m_convertors.size(), 1);
    EXPECT_EQ(dc.m_convertors.value("test"), createConvertor);

    EXPECT_TRUE(dc.regist("test2", (CreateConvertor)0x1));
    EXPECT_EQ(dc.m_convertors.size(), 2);
}

TEST(DataConvertor, ut_unRegist)
{
    DataConvertor dc;
    dc.m_convertors.insert("test", createConvertor);
    ASSERT_EQ(dc.m_convertors.size(), 1);

    dc.unRegist(createConvertor);
    EXPECT_TRUE(dc.m_convertors.isEmpty());
}

TEST(DataConvertor, ut_convert)
{
    DataConvertor dc;
    QString in;
    QString out;
    EXPECT_EQ(dc.convert("1" , "", nullptr, nullptr), -1);
    EXPECT_EQ(dc.convert("1" , "test", nullptr, nullptr), -1);
    EXPECT_EQ(dc.convert("1" , "test", &in, nullptr), -1);
    EXPECT_EQ(dc.convert("1" , "test", &in, &out), 1);
}

class ConvertorTest : public AbstractConvertor
{
public:
    explicit ConvertorTest(){}
    static AbstractConvertor *create(){
        return new ConvertorTest;
    }
    QString version() const Q_DECL_OVERRIDE {
        return "ver";
    }
    QHash<QString, ConvertInterface> interfaces() Q_DECL_OVERRIDE{
        QHash<QString, ConvertInterface> funcs;
        funcs.insert("test", &test);
        return funcs;
    }
    static int test(void *in, void *out) {
        QString *inStr = (QString *)in;
        QString *outStr = (QString *)out;
        if (inStr->compare("test") != 0)
            return 2;
        *outStr = QString("test2");
        return 0;
    }
};

TEST(DataConvertor, ut_convert_1)
{
    DataConvertor dc;
    QString in;
    QString out;

    ConvertorTest test;
    dc.regist(test.version(), &ConvertorTest::create);

    EXPECT_EQ(dc.convert(test.version(), "fake", &in , &out), 1);
    EXPECT_TRUE(in.isEmpty());
    EXPECT_TRUE(out.isEmpty());

    in.clear();
    out.clear();

    EXPECT_EQ(dc.convert(test.version(), "test", &in , &out), 2);
    EXPECT_TRUE(in.isEmpty());
    EXPECT_TRUE(out.isEmpty());

    in = "test";
    out.clear();

    EXPECT_EQ(dc.convert(test.version(), "test", &in , &out), 0);
    EXPECT_EQ(out, QString("test2"));
}
