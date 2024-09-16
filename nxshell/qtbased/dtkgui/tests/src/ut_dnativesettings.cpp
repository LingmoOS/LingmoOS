// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#define private public
#include "dnativesettings.h"
#undef private

#include <QtDBus>
#include <QtDebug>
#include <QTest>
#include <QSignalSpy>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

class ut_DNativeSettings : public testing::Test
{
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static DNativeSettings *settings;
};

DNativeSettings *ut_DNativeSettings::settings = nullptr;

void ut_DNativeSettings::SetUpTestSuite()
{
    settings = new DNativeSettings(0, "ut_dnativesettings");
}

void ut_DNativeSettings::TearDownTestSuite()
{
    delete settings;
}

TEST_F(ut_DNativeSettings, allKeys)
{
    ASSERT_EQ(settings->allKeys(), QByteArrayList({"Net/ThemeName",
                                            "Qt/ActiveColor",
                                            "Net/IconThemeName",
                                            "DTK/WindowRadius"}));
}

TEST_F(ut_DNativeSettings, valid)
{
    ASSERT_TRUE(settings->isValid());
}

TEST_F(ut_DNativeSettings, get_set)
{
    ASSERT_EQ(settings->getSetting("Net/ThemeName").toString(), "deepin");
    settings->setSetting("Net/ThemeName", "deepin-themeName");
    ASSERT_EQ(settings->getSetting("Net/ThemeName").toString(), "deepin-themeName");
}

TEST_F(ut_DNativeSettings, signal)
{
    {
        QSignalSpy spy(settings, &DNativeSettings::propertyChanged);
        settings->setSetting("Qt/ActiveColor", QColor(Qt::green));

        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
    }

    {
        QSignalSpy spy(settings, &DNativeSettings::allKeysChanged);
        settings->__setAllKeys({"Net/ThemeName", "Qt/ActiveColor"});
        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
    }
}
