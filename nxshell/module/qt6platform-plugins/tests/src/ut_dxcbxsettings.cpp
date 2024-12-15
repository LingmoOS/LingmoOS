// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QDebug>
#include <qxcbconnection.h>

#include "dxcbxsettings.h"
#include "dplatformintegration.h"

DPP_USE_NAMESPACE

static const QByteArray TEST_VALUE("Test/Value");
static bool testPropertyChangedCallback = false;

static void propertyChangedfunc(xcb_connection_t *, const QByteArray &, const QVariant &, void *)
{
    testPropertyChangedCallback = true;
}

class TDXcbXSettings : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    DXcbXSettings *settings = nullptr;
};

void TDXcbXSettings::SetUp()
{
    settings = DPlatformIntegration::instance()->xSettings();
    settings->setSetting(TEST_VALUE, 100);
}

void TDXcbXSettings::TearDown()
{
    settings->setSetting(TEST_VALUE, QVariant());
    testPropertyChangedCallback = false;
}

TEST_F(TDXcbXSettings, getOwner)
{
    xcb_window_t id = DXcbXSettings::getOwner();
    ASSERT_NE(id, 0);
}

TEST_F(TDXcbXSettings, initialized)
{
    bool isInitialized = settings->initialized();
    ASSERT_TRUE(isInitialized);
}

TEST_F(TDXcbXSettings, isEmpty)
{
    bool isEmpty = settings->isEmpty();
    ASSERT_FALSE(isEmpty);
}

TEST_F(TDXcbXSettings, contains)
{
    bool isContained = settings->contains(TEST_VALUE);
    ASSERT_TRUE(isContained);
    isContained = settings->contains("test");
    ASSERT_FALSE(isContained);
}

TEST_F(TDXcbXSettings, setSetting)
{
    settings->setSetting(TEST_VALUE, 200);
    QVariant value = settings->setting(TEST_VALUE);
    bool ok = false;
    int blink = value.toInt(&ok);
    ASSERT_TRUE(ok);
    ASSERT_EQ(blink, 200);
}

TEST_F(TDXcbXSettings, settingKeys)
{
    ASSERT_TRUE(!settings->settingKeys().isEmpty());
}

TEST_F(TDXcbXSettings, registerCallback)
{
    ASSERT_FALSE(testPropertyChangedCallback);
    settings->registerCallback(propertyChangedfunc, (void *)&propertyChangedfunc);
    QVariant oldValue = settings->setting(TEST_VALUE);
    settings->setSetting(TEST_VALUE, oldValue.toInt() + 1);
    ASSERT_TRUE(testPropertyChangedCallback);
    settings->removeCallbackForHandle((void *)&propertyChangedfunc);
}

TEST_F(TDXcbXSettings, registerCallbackForProperty)
{
    ASSERT_FALSE(testPropertyChangedCallback);
    settings->registerCallbackForProperty(TEST_VALUE, propertyChangedfunc, (void *)&propertyChangedfunc);
    QVariant oldValue = settings->setting(TEST_VALUE);
    settings->setSetting(TEST_VALUE, oldValue.toInt() + 1);
    ASSERT_TRUE(testPropertyChangedCallback);
    settings->removeCallbackForHandle(TEST_VALUE, (void *)&propertyChangedfunc);
}
