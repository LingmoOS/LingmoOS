/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <QString>
#include <QTemporaryDir>
#include <QDebug>
#include <QFile>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
#include "dthemesettings.h"

class TestDThemeSettings : public testing::Test
{
public:
    static void SetUpTestSuite() {
        userConfigPath = new QTemporaryDir;
        userConfigPath->setAutoRemove(true);
        QDir path(userConfigPath->path());
        path.mkdir("deepin");
        QFile::copy(":/qt-theme.ini", userConfigPath->filePath("deepin/qt-theme.ini"));
        prevEnv = qgetenv("D_QT_THEME_CONFIG_PATH");
        qputenv("D_QT_THEME_CONFIG_PATH", userConfigPath->path().toLocal8Bit());
        themeSettings = new DThemeSettings(false);
    }

    static void TearDownTestSuite() {
        qputenv("D_Qt_THEME_CONFIG_PATH", prevEnv);
        delete themeSettings;
        delete userConfigPath;
    }

    static DThemeSettings *themeSettings;
    static QTemporaryDir  *userConfigPath;
    static QByteArray     prevEnv;
};

DThemeSettings *TestDThemeSettings::themeSettings = nullptr;
QTemporaryDir *TestDThemeSettings::userConfigPath = nullptr;
QByteArray TestDThemeSettings::prevEnv = {};

// Test user settings load
TEST_F(TestDThemeSettings, makeSettingsUserSpecified)
{
    QSettings *settings = DThemeSettings::makeSettings();
    ASSERT_NE(nullptr, settings);
    ASSERT_EQ(settings->status(), QSettings::NoError);
    QString name = settings->fileName();
    EXPECT_EQ(name, userConfigPath->filePath("deepin/qt-theme.ini"));
    QStringList keys = settings->allKeys();
    EXPECT_TRUE(keys.contains("Font"));
    EXPECT_TRUE(keys.contains("FontSize"));
    EXPECT_TRUE(keys.contains("IconThemeName"));
    EXPECT_TRUE(keys.contains("MonoFont"));
    EXPECT_TRUE(keys.contains("ScaleLogicalDpi"));
    EXPECT_TRUE(keys.contains("ScreenScaleFactors"));
}

// Make sure that ini file uses UTF-8 codec
TEST_F(TestDThemeSettings, makeSettingsCodec)
{
    QSettings *settings = DThemeSettings::makeSettings();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec *iniCodec = settings->iniCodec();
    EXPECT_EQ(codec->name(), iniCodec->name());
#else
    Q_UNUSED(settings)
#endif
}

// Test properties using fake data
TEST_F(TestDThemeSettings, properties)
{
    EXPECT_EQ(themeSettings->isSetIconThemeName(), true);
    EXPECT_EQ(themeSettings->iconThemeName(), "organic-glass");
    EXPECT_EQ(themeSettings->isSetFallbackIconThemeName(), true);
    EXPECT_EQ(themeSettings->fallbackIconThemeName(), "bloom");
    EXPECT_EQ(themeSettings->isSetStyleNames(), true);
    EXPECT_EQ(themeSettings->styleNames(), QStringList{"chameleon"});
    EXPECT_EQ(themeSettings->isSetSystemFont(), true);
    EXPECT_EQ(themeSettings->systemFont(), "Noto Sans");
    EXPECT_EQ(themeSettings->isSetSystemFixedFont(), true);
    EXPECT_EQ(themeSettings->systemFixedFont(), "Noto Mono");
    EXPECT_EQ(themeSettings->screenScaleFactors(), "1.25");
    DDpi dpi = themeSettings->scaleLogicalDpi();
    EXPECT_DOUBLE_EQ(dpi.first, 96.0);
    EXPECT_DOUBLE_EQ(dpi.second, 96.0);
}
