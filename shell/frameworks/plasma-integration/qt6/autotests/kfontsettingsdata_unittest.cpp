/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "../src/platformtheme/kfontsettingsdata.h"
#include "kdeplatformtheme_config.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTest>

#include <QDBusConnection>
#include <QDBusMessage>

static void prepareEnvironment()
{
    qputenv("KDEHOME", QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme")));
    qputenv("XDG_DATA_HOME", QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme/xdg/local")));
    QByteArray configPath = QFile::encodeName(QDir::homePath() + QStringLiteral("/.kde5-unit-test-platformtheme/xdg/config"));
    qputenv("XDG_CONFIG_HOME", configPath);
    qputenv("KDE_SKIP_KDERC", "1");
    qunsetenv("KDE_COLOR_DEBUG");

    QDir().mkpath(configPath);
    configPath.append("/kdeglobals");
    QFile::remove(configPath);
    QFile::copy(CONFIGFILE, configPath);
}

// Run prepareEnvironment before qApp is created! slightly incorrect, QFile::encodeName can be wrong then.
// But we can't use Q_COREAPP_STARTUP_FUNCTION because the platform theme ends up being created
// first, with the wrong environment.
Q_CONSTRUCTOR_FUNCTION(prepareEnvironment)

class KFontSettingsData_UnitTest : public QWidget
{
    Q_OBJECT
public:
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::ApplicationFontChange) {
            m_appChangedFont = true;
        }
        return QWidget::event(e);
    }

private:
    bool m_appChangedFont;
    KFontSettingsData *m_fonts;
private Q_SLOTS:
    void initTestCase()
    {
        m_appChangedFont = false;
        m_fonts = new KFontSettingsData;
        qApp->processEvents(); // give time to delayed dbus connect
    }

    void testFonts()
    {
        QCOMPARE(m_fonts->font(KFontSettingsData::GeneralFont)->family(), QStringLiteral("OxyFontTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::FixedFont)->family(), QStringLiteral("OxyFixedTest Mono"));
        QCOMPARE(m_fonts->font(KFontSettingsData::ToolbarFont)->family(), QStringLiteral("OxyToolbarTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::MenuFont)->family(), QStringLiteral("OxyMenuTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::WindowTitleFont)->family(), QStringLiteral("OxyActiveTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::TaskbarFont)->family(), QStringLiteral("OxyTaskbarTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::SmallestReadableFont)->family(), QStringLiteral("OxySmallestReadableTest"));
    }

    void testFontsChanged()
    {
        QByteArray configPath = qgetenv("XDG_CONFIG_HOME");
        configPath.append("/kdeglobals");
        QFile::remove(configPath);
        QVERIFY(QFile::copy(CHANGED_CONFIGFILE, configPath));

        QEventLoop loop;
        QDBusConnection::sessionBus().connect(QString(),
                                              QStringLiteral("/KDEPlatformTheme"),
                                              QStringLiteral("org.kde.KDEPlatformTheme"),
                                              QStringLiteral("refreshFonts"),
                                              &loop,
                                              SLOT(quit()));

        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/KDEPlatformTheme"), QStringLiteral("org.kde.KDEPlatformTheme"), QStringLiteral("refreshFonts"));
        QDBusConnection::sessionBus().send(message);
        loop.exec();

        QVERIFY(m_appChangedFont);
        QCOMPARE(m_fonts->font(KFontSettingsData::GeneralFont)->family(), QStringLiteral("ChangedFontTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::FixedFont)->family(), QStringLiteral("ChangedFixedTest Mono"));
        QCOMPARE(m_fonts->font(KFontSettingsData::ToolbarFont)->family(), QStringLiteral("ChangedToolbarTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::MenuFont)->family(), QStringLiteral("ChangedMenuTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::WindowTitleFont)->family(), QStringLiteral("ChangedActiveTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::TaskbarFont)->family(), QStringLiteral("ChangedTaskbarTest"));
        QCOMPARE(m_fonts->font(KFontSettingsData::SmallestReadableFont)->family(), QStringLiteral("ChangedSmallestReadableTest"));
    }
};

QTEST_MAIN(KFontSettingsData_UnitTest)

#include "kfontsettingsdata_unittest.moc"
