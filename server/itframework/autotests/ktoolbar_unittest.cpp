/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "testguiclient.h"
#include "testxmlguiwindow.h"

#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#endif
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KIconLoader>
#include <KSharedConfig>
#include <kmainwindow.h>
#include <ktoolbar.h>

// We use the data types below in a QVariant, so Q_DECLARE_METATYPE is needed for them.
Q_DECLARE_METATYPE(Qt::MouseButton)
Q_DECLARE_METATYPE(Qt::MouseButtons)
Q_DECLARE_METATYPE(Qt::KeyboardModifiers)

// Ensure everything uses test paths, including stuff run before main, such as the KdePlatformThemePlugin
void enableTestMode()
{
    QStandardPaths::setTestModeEnabled(true);
}
Q_CONSTRUCTOR_FUNCTION(enableTestMode)

class tst_KToolBar : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void ktoolbar();

    void testIconSizeNoXmlGui_data();
    void testIconSizeNoXmlGui();
    void testIconSizeXmlGui_data();
    void testIconSizeXmlGui();
    void testToolButtonStyleNoXmlGui_data();
    void testToolButtonStyleNoXmlGui();
    void testToolButtonStyleXmlGui_data();
    void testToolButtonStyleXmlGui();
    void testToolBarPosition();
    void testXmlGuiSwitching();
    void testKAuthorizedDisableToggleAction();

Q_SIGNALS:
    void signalAppearanceChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void changeGlobalIconSizeSetting(int, int);
    void deleteGlobalIconSizeSetting();
    void changeGlobalToolButtonStyleSetting(const QString &, const QString &);
    void deleteGlobalToolButtonStyleSetting();
    QByteArray m_xml;
    bool m_showWasCalled;
};

QTEST_MAIN(tst_KToolBar)

static void copy_dir(const QString &from, const QDir &to)
{
    QDir src = QDir(from);
    QDir dest = QDir(to.filePath(src.dirName()));
    to.mkpath(src.dirName());
    const auto fileInfos = src.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileInfos) {
        if (fileInfo.isDir()) {
            copy_dir(fileInfo.filePath(), dest);
        } else {
            QFile::copy(fileInfo.filePath(), dest.filePath(fileInfo.fileName()));
        }
    }
}

// This will be called before the first test function is executed.
// It is only called once.
void tst_KToolBar::initTestCase()
{
    // start with a clean place to put data
    QDir testDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    QVERIFY(testDataDir.absolutePath().contains(QLatin1String("qttest")));
    testDataDir.removeRecursively();
    testDataDir.mkpath(QStringLiteral("."));

    // setup action restriction so we can test whether this actually disables some functionality
    KConfigGroup actionRestrictions(KSharedConfig::openConfig(), "KDE Action Restrictions");
    actionRestrictions.writeEntry("action/options_show_toolbar", false);

    // copy a minimal icon theme to where KIconTheme will find it, in case oxygen-icons is not
    // installed
    copy_dir(QFINDTESTDATA("icons"), testDataDir);

    m_xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"otherToolBar\" position=\"bottom\" iconText=\"TextUnderIcon\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"cleanToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"hiddenToolBar\" hidden=\"true\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"secondHiddenToolBar\" hidden=\"true\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar iconSize=\"32\" name=\"bigUnchangedToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";

    qRegisterMetaType<Qt::MouseButtons>("Qt::MouseButtons");
    qRegisterMetaType<Qt::KeyboardModifiers>("Qt::KeyboardModifiers");
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_KToolBar::cleanupTestCase()
{
    QDir testDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    QDir testIconsDir = QDir(testDataDir.absoluteFilePath(QStringLiteral("icons")));
    QVERIFY(testIconsDir.absolutePath().contains(QLatin1String("qttest")));
    testIconsDir.removeRecursively();
}

// This will be called before each test function is executed.
void tst_KToolBar::init()
{
}

// This will be called after every test function.
void tst_KToolBar::cleanup()
{
    QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/tst_KToolBar"));
    deleteGlobalIconSizeSetting();
    deleteGlobalToolButtonStyleSetting();
}

void tst_KToolBar::ktoolbar()
{
    KMainWindow kmw;
    // Creating a KToolBar directly
    KToolBar bar(&kmw);
    QCOMPARE(bar.mainWindow(), &kmw);
    // Asking KMainWindow for a KToolBar (more common)
    KToolBar *mainToolBar = kmw.toolBar(QStringLiteral("mainToolBar"));
    QCOMPARE(mainToolBar->mainWindow(), &kmw);
}

Q_DECLARE_METATYPE(KConfigGroup)

void tst_KToolBar::testIconSizeNoXmlGui_data()
{
    QTest::addColumn<int>("iconSize");
    QTest::newRow("16") << 16;
    QTest::newRow("22") << 22;
    QTest::newRow("32") << 32;
    QTest::newRow("64") << 64;
}

void tst_KToolBar::testIconSizeNoXmlGui()
{
    QFETCH(int, iconSize);
    KConfig config(QStringLiteral("tst_KToolBar"));
    KConfigGroup group(&config, "group");
    {
        KMainWindow kmw;
        KToolBar *mainToolBar = kmw.toolBar(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBar(QStringLiteral("otherToolBar"));

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        // check the actual values - update this if kicontheme's defaults are changed
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::MainToolbar), 22);
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::Toolbar), 22);

        // Changing settings for a given toolbar, as user
        mainToolBar->setIconDimensions(iconSize);
        otherToolBar->setIconDimensions(iconSize);

        // Save settings
        kmw.saveMainWindowSettings(group);
        // was it the default value?
        if (iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar)) {
            QCOMPARE(group.groupList().count(), 0); // nothing to save
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("IconSize"));
        } else {
            QCOMPARE(group.groupList().count(), 2); // two subgroups (one for each toolbar)
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("IconSize"));
        }
    }

    {
        // Recreate, load, compare.
        KMainWindow kmw;
        KToolBar *mainToolBar = kmw.toolBar(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBar(QStringLiteral("otherToolBar"));
        KToolBar *cleanToolBar = kmw.toolBar(QStringLiteral("cleanToolBar"));
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        kmw.applyMainWindowSettings(group);
        QCOMPARE(mainToolBar->iconSize().width(), iconSize);
        QCOMPARE(otherToolBar->iconSize().width(), iconSize);
        QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar)); // unchanged
        const bool mainToolBarWasUsingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        const bool otherToolBarWasUsingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::Toolbar);

#ifndef Q_OS_WIN // the change notification uses DBus
        // Now emulate a change of the kde-global setting (#168480#c12)
        changeGlobalIconSizeSetting(32, 33);

        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::MainToolbar), 32);
        QCOMPARE(KIconLoader::global()->currentSize(KIconLoader::Toolbar), 33);

        if (mainToolBarWasUsingDefaultSize) {
            QCOMPARE(mainToolBar->iconSize().width(), 32);
        } else { // the user chose a specific size for the toolbar, so the new global size isn't used
            QCOMPARE(mainToolBar->iconSize().width(), iconSize);
        }
        if (otherToolBarWasUsingDefaultSize) {
            QCOMPARE(otherToolBar->iconSize().width(), 33);
        } else { // the user chose a specific size for the toolbar, so the new global size isn't used
            QCOMPARE(otherToolBar->iconSize().width(), iconSize);
        }
        QCOMPARE(cleanToolBar->iconSize().width(), 33);
#endif
    }
}

void tst_KToolBar::testIconSizeXmlGui_data()
{
    QTest::addColumn<int>("iconSize"); // set by user and saved to KConfig
    QTest::addColumn<int>("expectedSizeMainToolbar"); // ... after kde-global is changed to 25
    QTest::addColumn<int>("expectedSizeOtherToolbar"); // ... after kde-global is changed to 16
    QTest::addColumn<int>("expectedSizeCleanToolbar"); // ... after kde-global is changed to 16
    QTest::addColumn<int>("expectedSizeBigToolbar"); // ... after kde-global is changed to 16
    // When the user chose a specific size for the toolbar (!= its default size), the new kde-global size isn't applied to that toolbar.
    // So, only in case the toolbar was at iconSize already, and there was no setting in xml, we end up with kdeGlobal being used:
    const int kdeGlobalMain = 25;
    const int kdeGlobalOther = 16;
    QTest::newRow("16") << 16 << 16 << 16 << kdeGlobalOther << 16;
    QTest::newRow("22") << 22 << kdeGlobalMain << kdeGlobalOther << kdeGlobalOther << 22;
    QTest::newRow("32") << 32 << 32 << 32 << kdeGlobalOther << 32;
    QTest::newRow("64") << 64 << 64 << 64 << kdeGlobalOther << 64;
}

void tst_KToolBar::testIconSizeXmlGui()
{
    QFETCH(int, iconSize);
    QFETCH(int, expectedSizeMainToolbar);
    QFETCH(int, expectedSizeOtherToolbar);
    QFETCH(int, expectedSizeCleanToolbar);
    QFETCH(int, expectedSizeBigToolbar);
    KConfig config(QStringLiteral("tst_KToolBar"));
    KConfigGroup group(&config, "group");
    {
        TestXmlGuiWindow kmw(m_xml, "tst_ktoolbar.rc");
        kmw.createActions(QStringList() << QStringLiteral("go_up"));
        kmw.createGUI();
        KToolBar *mainToolBar = kmw.toolBarByName(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBarByName(QStringLiteral("otherToolBar"));
        KToolBar *cleanToolBar = kmw.toolBarByName(QStringLiteral("cleanToolBar"));
        KToolBar *bigToolBar = kmw.toolBarByName(QStringLiteral("bigToolBar"));
        KToolBar *bigUnchangedToolBar = kmw.toolBarByName(QStringLiteral("bigUnchangedToolBar"));

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
        QCOMPARE(otherToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
        // check the actual values - update this if kicontheme's defaults are changed
        QCOMPARE(mainToolBar->iconSize().width(), 22);
        QCOMPARE(otherToolBar->iconSize().width(), 22);
        QCOMPARE(cleanToolBar->iconSize().width(), 22);
        QCOMPARE(bigToolBar->iconSize().width(), 32);
        QCOMPARE(bigUnchangedToolBar->iconSize().width(), 32);

        // Changing settings for a given toolbar, as user (to test the initial report in #168480)
        mainToolBar->setIconDimensions(iconSize);
        otherToolBar->setIconDimensions(iconSize);
        bigToolBar->setIconDimensions(iconSize);

        // Save settings
        kmw.saveMainWindowSettings(group);
        // was it the default size? (for the main toolbar, we only check that one)
        const bool usingDefaultSize = iconSize == KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
        if (usingDefaultSize) {
            QVERIFY(!group.groupList().contains(QLatin1String("Toolbar mainToolBar")));
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("IconSize"));
        } else {
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("IconSize"));
        }

#ifndef Q_OS_WIN // the change notification uses DBus
        // Now emulate a change of the kde-global setting (#168480#c12)
        changeGlobalIconSizeSetting(25, 16);

        QCOMPARE(mainToolBar->iconSize().width(), expectedSizeMainToolbar);
        QCOMPARE(otherToolBar->iconSize().width(), expectedSizeOtherToolbar);
        QCOMPARE(cleanToolBar->iconSize().width(), expectedSizeCleanToolbar);
        QCOMPARE(bigToolBar->iconSize().width(), expectedSizeBigToolbar);
#endif

        // The big unchanged toolbar should be, well, unchanged; AppXml has priority over KDE_Default.
        QCOMPARE(bigUnchangedToolBar->iconSize().width(), 32);
    }
}

void tst_KToolBar::changeGlobalIconSizeSetting(int mainToolbarIconSize, int iconSize)
{
    // We could use KConfig::Normal|KConfig::Global here, to write to kdeglobals like kcmstyle does,
    // but we don't need to. Writing to the app's config file works too.
    KConfigGroup mglobals(KSharedConfig::openConfig(), "MainToolbarIcons");
    mglobals.writeEntry("Size", mainToolbarIconSize);
    KConfigGroup globals(KSharedConfig::openConfig(), "ToolbarIcons");
    // globals.writeEntry("Size", iconSize, KConfig::Normal|KConfig::Global);
    globals.writeEntry("Size", iconSize);
    KSharedConfig::openConfig()->sync();

    QSignalSpy spy(KIconLoader::global(), &KIconLoader::iconChanged);
    KIconLoader::global()->emitChange(KIconLoader::Desktop);
    spy.wait(200);
}

void tst_KToolBar::deleteGlobalIconSizeSetting()
{
    KConfigGroup mglobals(KSharedConfig::openConfig(), "MainToolbarIcons");
    mglobals.deleteEntry("Size");
    KConfigGroup globals(KSharedConfig::openConfig(), "ToolbarIcons");
    globals.deleteEntry("Size");
    KSharedConfig::openConfig()->sync();

    QSignalSpy spy(KIconLoader::global(), &KIconLoader::iconChanged);
    KIconLoader::global()->emitChange(KIconLoader::Desktop);
    spy.wait(200);
}

Q_DECLARE_METATYPE(Qt::ToolButtonStyle)

void tst_KToolBar::testToolButtonStyleNoXmlGui_data()
{
    QTest::addColumn<Qt::ToolButtonStyle>("toolButtonStyle");
    QTest::newRow("Qt::ToolButtonIconOnly") << Qt::ToolButtonIconOnly;
    QTest::newRow("Qt::ToolButtonTextOnly") << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextBesideIcon") << Qt::ToolButtonTextBesideIcon;
    QTest::newRow("Qt::ToolButtonTextUnderIcon") << Qt::ToolButtonTextUnderIcon;
}

void tst_KToolBar::testToolButtonStyleNoXmlGui()
{
    QFETCH(Qt::ToolButtonStyle, toolButtonStyle);
    const Qt::ToolButtonStyle mainToolBarDefaultStyle = Qt::ToolButtonTextBesideIcon; // was TextUnderIcon before KDE-4.4.0
    const bool selectedDefaultForMainToolbar = toolButtonStyle == mainToolBarDefaultStyle;
    const bool selectedDefaultForOtherToolbar = toolButtonStyle == Qt::ToolButtonTextBesideIcon;

    KConfig config(QStringLiteral("tst_KToolBar"));
    KConfigGroup group(&config, "group");
    {
        KMainWindow kmw;
        KToolBar *mainToolBar = kmw.toolBar(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBar(QStringLiteral("otherToolBar"));

        // Default settings (applied by applyAppearanceSettings)
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon); // see r883541
        QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::TopToolBarArea);

        // Changing settings for a given toolbar, as user
        mainToolBar->setToolButtonStyle(toolButtonStyle);
        otherToolBar->setToolButtonStyle(toolButtonStyle);

        // Save settings
        kmw.saveMainWindowSettings(group);
        if (selectedDefaultForMainToolbar) {
            QCOMPARE(group.groupList().count(), 0); // nothing to save
            QVERIFY(!group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
        } else {
            QCOMPARE(group.groupList().count(), 2); // two subgroups (one for each toolbar)
            QVERIFY(group.group("Toolbar mainToolBar").hasKey("ToolButtonStyle"));
        }
    }

    {
        // Recreate, load, compare.
        KMainWindow kmw;
        KToolBar *mainToolBar = kmw.toolBar(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBar(QStringLiteral("otherToolBar"));
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        kmw.applyMainWindowSettings(group);
        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)toolButtonStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)toolButtonStyle);

#ifndef Q_OS_WIN // the change notification uses DBus
        // Now change KDE-global setting
        changeGlobalToolButtonStyleSetting(QStringLiteral("IconOnly"), QStringLiteral("TextOnly"));

        if (selectedDefaultForMainToolbar) {
            QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonIconOnly);
        } else {
            QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)toolButtonStyle);
        }

        if (selectedDefaultForOtherToolbar) {
            QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextOnly);
        } else {
            QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)toolButtonStyle);
        }
#endif
    }
}

void tst_KToolBar::testToolButtonStyleXmlGui_data()
{
    QTest::addColumn<Qt::ToolButtonStyle>("toolButtonStyle");
    // Expected style after KDE-global is changed to main=IconOnly/other=TextOnly
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleMainToolbar");
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleOtherToolbar"); // xml says text-under-icons, user-selected should always win
    QTest::addColumn<Qt::ToolButtonStyle>("expectedStyleCleanToolbar"); // should always follow kde-global -> always textonly.

    QTest::newRow("Qt::ToolButtonTextUnderIcon") << Qt::ToolButtonTextUnderIcon << Qt::ToolButtonTextUnderIcon << Qt::ToolButtonTextUnderIcon
                                                 << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextBesideIcon") << Qt::ToolButtonTextBesideIcon
                                                  << Qt::ToolButtonIconOnly /* was default -> using kde global */ << Qt::ToolButtonTextBesideIcon
                                                  << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonIconOnly") << Qt::ToolButtonIconOnly << Qt::ToolButtonIconOnly << Qt::ToolButtonIconOnly << Qt::ToolButtonTextOnly;
    QTest::newRow("Qt::ToolButtonTextOnly") << Qt::ToolButtonTextOnly << Qt::ToolButtonTextOnly << Qt::ToolButtonTextOnly << Qt::ToolButtonTextOnly;
}

void tst_KToolBar::testToolButtonStyleXmlGui()
{
    QFETCH(Qt::ToolButtonStyle, toolButtonStyle);
    QFETCH(Qt::ToolButtonStyle, expectedStyleMainToolbar);
    QFETCH(Qt::ToolButtonStyle, expectedStyleOtherToolbar);
    QFETCH(Qt::ToolButtonStyle, expectedStyleCleanToolbar);
    const Qt::ToolButtonStyle mainToolBarDefaultStyle = Qt::ToolButtonTextBesideIcon; // was TextUnderIcon before KDE-4.4.0
    KConfig config(QStringLiteral("tst_KToolBar"));
    KConfigGroup group(&config, "group");
    {
        TestXmlGuiWindow kmw(m_xml, "tst_ktoolbar.rc");
        kmw.createActions(QStringList() << QStringLiteral("go_up"));
        kmw.createGUI();
        KToolBar *mainToolBar = kmw.toolBarByName(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = kmw.toolBarByName(QStringLiteral("otherToolBar"));
        KToolBar *cleanToolBar = kmw.toolBarByName(QStringLiteral("cleanToolBar"));

        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)mainToolBarDefaultStyle);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextUnderIcon); // from xml
        QCOMPARE((int)cleanToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon);

        // Changing settings for a given toolbar, as user
        mainToolBar->setToolButtonStyle(toolButtonStyle);
        otherToolBar->setToolButtonStyle(toolButtonStyle);

        // Save settings
        kmw.saveMainWindowSettings(group);

#ifndef Q_OS_WIN // the change notification uses DBus
        // Now change KDE-global setting
        changeGlobalToolButtonStyleSetting(QStringLiteral("IconOnly"), QStringLiteral("TextOnly"));

        QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)expectedStyleMainToolbar);
        QCOMPARE((int)otherToolBar->toolButtonStyle(), (int)expectedStyleOtherToolbar);
        QCOMPARE((int)cleanToolBar->toolButtonStyle(), (int)expectedStyleCleanToolbar);
#endif
    }
}

void tst_KToolBar::changeGlobalToolButtonStyleSetting(const QString &mainToolBar, const QString &otherToolBars)
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    group.writeEntry("ToolButtonStyle", mainToolBar);
    group.writeEntry("ToolButtonStyleOtherToolbars", otherToolBars);
    group.sync();

    // Same dbus connect as the one in KToolBar. We want our spy to be notified of receiving it.
#ifdef QT_DBUS_LIB
    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/KToolBar"),
                                          QStringLiteral("org.kde.KToolBar"),
                                          QStringLiteral("styleChanged"),
                                          this,
                                          SIGNAL(signalAppearanceChanged()));
    QSignalSpy spy(this, &tst_KToolBar::signalAppearanceChanged);

    KToolBar::emitToolbarStyleChanged();
    spy.wait(2000);
#endif
}

void tst_KToolBar::deleteGlobalToolButtonStyleSetting()
{
    KConfigGroup group(KSharedConfig::openConfig(), "Toolbar style");
    group.deleteEntry("ToolButtonStyle");
    group.deleteEntry("ToolButtonStyleOtherToolbars");
    KSharedConfig::openConfig()->sync();
}

void tst_KToolBar::testToolBarPosition()
{
    TestXmlGuiWindow kmw(m_xml, "tst_ktoolbar.rc");
    kmw.createActions(QStringList() << QStringLiteral("go_up"));
    kmw.createGUI();
    KToolBar *mainToolBar = kmw.toolBarByName(QStringLiteral("mainToolBar"));
    KToolBar *otherToolBar = kmw.toolBarByName(QStringLiteral("otherToolBar"));
    QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::TopToolBarArea);
    QCOMPARE(kmw.toolBarArea(otherToolBar), Qt::BottomToolBarArea);
}

void tst_KToolBar::testXmlGuiSwitching()
{
    const QByteArray windowXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "</gui>\n";
    TestXmlGuiWindow kmw(windowXml, "tst_ktoolbar.rc");
    kmw.createActions(QStringList() << QStringLiteral("go_up"));
    kmw.createGUI();
    TestGuiClient firstClient(m_xml);
    kmw.guiFactory()->addClient(&firstClient);

    {
        // qDebug() << "Added gui client";
        KToolBar *mainToolBar = firstClient.toolBarByName(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = firstClient.toolBarByName(QStringLiteral("otherToolBar"));
        KToolBar *bigToolBar = firstClient.toolBarByName(QStringLiteral("bigToolBar"));
        KToolBar *hiddenToolBar = firstClient.toolBarByName(QStringLiteral("hiddenToolBar"));
        KToolBar *secondHiddenToolBar = firstClient.toolBarByName(QStringLiteral("secondHiddenToolBar"));
        QCOMPARE(hiddenToolBar->isHidden(), true);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);
        // Make (unsaved) changes as user
        QMetaObject::invokeMethod(mainToolBar, "slotContextTextRight"); // mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        QMetaObject::invokeMethod(mainToolBar, "slotContextRight"); // kmw.addToolBar(Qt::RightToolBarArea, mainToolBar);
        otherToolBar->setIconDimensions(35);
        bigToolBar->setIconDimensions(35);
        bigToolBar->hide();
        hiddenToolBar->show();
    }
    kmw.guiFactory()->removeClient(&firstClient);
    // qDebug() << "Removed gui client";
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("mainToolBar"), &kmw));
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("otherToolBar"), &kmw));
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("bigToolBar"), &kmw));
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("mainToolBar"), &firstClient));
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("otherToolBar"), &firstClient));
    QVERIFY(!kmw.guiFactory()->container(QStringLiteral("bigToolBar"), &firstClient));

    kmw.guiFactory()->addClient(&firstClient);
    // qDebug() << "Re-added gui client";
    KToolBar *mainToolBar = firstClient.toolBarByName(QStringLiteral("mainToolBar"));
    KToolBar *otherToolBar = firstClient.toolBarByName(QStringLiteral("otherToolBar"));
    KToolBar *bigToolBar = firstClient.toolBarByName(QStringLiteral("bigToolBar"));
    KToolBar *cleanToolBar = firstClient.toolBarByName(QStringLiteral("cleanToolBar"));
    KToolBar *hiddenToolBar = firstClient.toolBarByName(QStringLiteral("hiddenToolBar"));
    KToolBar *secondHiddenToolBar = firstClient.toolBarByName(QStringLiteral("secondHiddenToolBar"));
    QCOMPARE((int)mainToolBar->toolButtonStyle(), (int)Qt::ToolButtonTextBesideIcon);
    QCOMPARE(mainToolBar->isHidden(), false);
    QCOMPARE(kmw.toolBarArea(mainToolBar), Qt::RightToolBarArea);
    QCOMPARE(mainToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::MainToolbar));
    QCOMPARE(otherToolBar->iconSize().width(), 35);
    QCOMPARE(bigToolBar->iconSize().width(), 35);
    QCOMPARE(cleanToolBar->iconSize().width(), KIconLoader::global()->currentSize(KIconLoader::Toolbar));
    QCOMPARE(bigToolBar->isHidden(), true);
    QCOMPARE(hiddenToolBar->isHidden(), false);
    QCOMPARE(secondHiddenToolBar->isHidden(), true);

    // Now change KDE-global setting, what happens to unsaved changes?
    changeGlobalIconSizeSetting(32, 33);
    QCOMPARE(bigToolBar->iconSize().width(), 35); // fine now, saved or unsaved makes no difference
    QCOMPARE(otherToolBar->iconSize().width(), 35);

    // Now save, and check what we saved
    KConfig config(QStringLiteral("tst_KToolBar"));
    KConfigGroup group(&config, "group");
    kmw.saveMainWindowSettings(group);
    QCOMPARE(group.group("Toolbar bigToolBar").readEntry("IconSize", 0), 35);
    QCOMPARE(group.group("Toolbar otherToolBar").readEntry("IconSize", 0), 35);
    QVERIFY(!group.group("Toolbar cleanToolBar").hasKey("IconSize"));
    // QCOMPARE(group.group("Toolbar bigToolBar").readEntry("Hidden", false), true);
    // QVERIFY(!group.group("Toolbar cleanToolBar").hasKey("Hidden"));
    // QVERIFY(!group.group("Toolbar hiddenToolBar").hasKey("Hidden"));

    // Recreate window and apply config; is hidden toolbar shown as expected?
    {
        TestXmlGuiWindow kmw2(windowXml, "tst_ktoolbar.rc");
        kmw2.createActions(QStringList() << QStringLiteral("go_up"));
        kmw2.createGUI();
        TestGuiClient firstClient(m_xml);
        kmw2.guiFactory()->addClient(&firstClient);

        KToolBar *mainToolBar = firstClient.toolBarByName(QStringLiteral("mainToolBar"));
        KToolBar *otherToolBar = firstClient.toolBarByName(QStringLiteral("otherToolBar"));
        KToolBar *bigToolBar = firstClient.toolBarByName(QStringLiteral("bigToolBar"));
        KToolBar *hiddenToolBar = firstClient.toolBarByName(QStringLiteral("hiddenToolBar"));
        KToolBar *secondHiddenToolBar = firstClient.toolBarByName(QStringLiteral("secondHiddenToolBar"));
        QCOMPARE(bigToolBar->isHidden(), false);
        QCOMPARE(hiddenToolBar->isHidden(), true);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);

        kmw2.show();

        // Check that secondHiddenToolBar is not shown+hidden immediately?
        m_showWasCalled = false;
        secondHiddenToolBar->installEventFilter(this);
        kmw2.applyMainWindowSettings(group);

        QCOMPARE(mainToolBar->isHidden(), false);
        QCOMPARE(kmw2.toolBarArea(mainToolBar), Qt::RightToolBarArea);
        QCOMPARE(otherToolBar->iconSize().width(), 35);
        QCOMPARE(bigToolBar->iconSize().width(), 35);
        QCOMPARE(bigToolBar->isHidden(), true);
        QCOMPARE(hiddenToolBar->isHidden(), false);
        QCOMPARE(secondHiddenToolBar->isHidden(), true);
        QVERIFY(!m_showWasCalled);
    }
}

void tst_KToolBar::testKAuthorizedDisableToggleAction()
{
    TestXmlGuiWindow kmw(m_xml, "tst_ktoolbar.rc");
    kmw.createGUI();

    const auto toolbars = kmw.toolBars();
    for (KToolBar *toolbar : toolbars) {
        QVERIFY(!toolbar->toggleViewAction()->isEnabled());
    }
}

bool tst_KToolBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (event->type() == QEvent::Show) {
        m_showWasCalled = true;
        return true;
    }
    return false;
}

#include "ktoolbar_unittest.moc"
