/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007-2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlgui_unittest.h"
#include "testguiclient.h"
#include "testxmlguiwindow.h"

#include <QAction>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QPushButton>
#include <QShowEvent>
#include <QTest>
#include <QWidget>

#include <KConfigGroup>
#include <KSharedConfig>

#include <kedittoolbar.h>
#include <kswitchlanguagedialog_p.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <kxmlguiversionhandler.cpp> // it's not exported, so we need to include the code here

QTEST_MAIN(KXmlGui_UnitTest)

enum Flags {
    NoFlags = 0,
    AddToolBars = 1,
    AddModifiedToolBars = 2,
    AddActionProperties = 4,
    AddModifiedMenus = 8,
    // next item is 16
};

static void createXmlFile(QFile &file, int version, int flags, const QByteArray &toplevelTag = "gui")
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE " + toplevelTag + " SYSTEM \"kpartgui.dtd\">\n"
        "<" + toplevelTag + " version=\"" + QByteArray::number(version) + "\" name=\"foo\" >\n"
        "<MenuBar>\n";
    file.write(xml);
    if (flags & AddModifiedMenus) {
        file.write(
            "<Menu noMerge=\"1\" name=\"file\" >\n"
            "<text>&amp;File</text>\n"
            "<Action name=\"file_open\" />\n"
            "</Menu>\n");
    }
    file.write("</MenuBar>\n");
    if (flags & AddToolBars) {
        file.write(
            "<ToolBar name=\"mainToolBar\">\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"print\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\">\n"
            "  <text>Bookmark Toolbar</text>\n"
            "</ToolBar>\n"
            "<ToolBar name=\"newToolBar\">\n"
            "  <text>New Toolbar</text>\n"
            "</ToolBar>\n");
    }
    if (flags & AddModifiedToolBars) {
        file.write(
            "<ToolBar name=\"mainToolBar\">\n"
            "  <text>Main Toolbar</text>\n"
            "  <Action name=\"home\" />\n"
            "</ToolBar>\n"
            "<ToolBar name=\"bookmarkToolBar\">\n"
            "  <text>Modified toolbars</text>\n"
            "</ToolBar>\n");
    }
    if (flags & AddActionProperties) {
        file.write(
            "<ActionProperties>\n"
            "  <Action shortcut=\"F9\" name=\"konq_sidebartng\" />\n"
            "</ActionProperties>\n");
    }
    file.write("</" + toplevelTag + ">\n");
}

static void clickApply(KEditToolBar *dialog)
{
    QDialogButtonBox *box = dialog->findChild<QDialogButtonBox *>();
    Q_ASSERT(box != nullptr);
    box->button(QDialogButtonBox::Apply)->setEnabled(true);
    box->button(QDialogButtonBox::Apply)->click();
}

void KXmlGui_UnitTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    // Leftover configuration breaks testAutoSaveSettings
    const QString configFile = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, KSharedConfig::openConfig()->name());
    if (!configFile.isEmpty()) {
        qDebug() << "Removing old config file";
        QFile::remove(configFile);
        KSharedConfig::openConfig()->reparseConfiguration();
    }
}

void KXmlGui_UnitTest::testFindVersionNumber_data()
{
    QTest::addColumn<QString>("xml");
    QTest::addColumn<QString>("version");

    QTest::newRow("simple test") << "<?xml version = '1.0'?>\n"
                                    "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
                                    "<gui version=\"3\" name=\"foo\"/>\n"
                                 << "3";
    QTest::newRow("two digits") << "<?xml version = '1.0'?>\n"
                                   "<gui version=\"42\" name=\"foo\"/>\n"
                                << "42";
    QTest::newRow("with spaces") << // as found in dirfilterplugin.rc for instance
        "<?xml version = '1.0'?>\n"
        "<gui version = \"1\" name=\"foo\"/>\n"
                                 << "1";
    QTest::newRow("with a dot") << // as was found in autorefresh.rc
        "<?xml version = '1.0'?>\n"
        "<gui version = \"0.2\" name=\"foo\"/>\n"
                                << QString() /*error*/;
    QTest::newRow("with a comment") << // as found in kmail.rc
        "<!DOCTYPE gui>\n"
        "<!-- This file should be synchronized with kmail_part.rc to provide\n"
        "the same menu entries at the same place in KMail and Kontact  -->\n"
        "<gui version=\"452\" name=\"kmmainwin\">\n"
                                    << "452";
}

void KXmlGui_UnitTest::testFindVersionNumber()
{
    QFETCH(QString, xml);
    QFETCH(QString, version);
    QCOMPARE(KXMLGUIClient::findVersionNumber(xml), version);
}

void KXmlGui_UnitTest::testVersionHandlerSameVersion()
{
    // This emulates the case where the user has modified stuff locally
    // and the application hasn't changed since, so the version number is unchanged.
    QTemporaryFile userFile;
    QVERIFY(userFile.open());
    createXmlFile(userFile, 2, AddActionProperties | AddModifiedToolBars);
    const QString firstFile = userFile.fileName();

    QTemporaryFile appFile;
    QVERIFY(appFile.open());
    createXmlFile(appFile, 2, AddToolBars);
    const QString secondFile = appFile.fileName();

    QStringList files;
    files << firstFile << secondFile;

    userFile.close();
    appFile.close();

    KXmlGuiVersionHandler versionHandler(files);
    QCOMPARE(versionHandler.finalFile(), firstFile);
    QString finalDoc = versionHandler.finalDocument();
    QVERIFY(finalDoc.startsWith(QLatin1String("<?xml")));
    // Check that the shortcuts defined by the user were kept
    QVERIFY(finalDoc.contains(QLatin1String("<ActionProperties>")));
    QVERIFY(finalDoc.contains(QLatin1String("sidebartng")));
    // Check that the toolbars modified by the user were kept
    QVERIFY(finalDoc.contains(QLatin1String("<Action name=\"home\"")));
    // and that the toolbar that isn't in the local file, isn't there in the GUI
    QVERIFY(!finalDoc.contains(QLatin1String("<ToolBar name=\"newToolBar\"")));

    QVERIFY(userFile.open());
    const QString userFileContents = QString::fromUtf8(userFile.readAll());
    QCOMPARE(finalDoc, userFileContents);
}

void KXmlGui_UnitTest::testVersionHandlerNewVersionNothingKept()
{
    // This emulates the case where the application has been upgraded
    // and the user has a local ui.rc file, but without shortcuts or toolbar changes.
    // Not sure how this can happen - would be a menu-only app and hand editing !?
    // Anyway the point is to test version number comparison :)

    QMap<QString, int> fileToVersionMap; // makes QCOMPARE failures more readable than just temp filenames

    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kxmlgui_unittest");
    QDir().mkpath(dir);
    QFile fileV2(dir + QStringLiteral("/testui.rc"));
    QVERIFY2(fileV2.open(QIODevice::WriteOnly), qPrintable(fileV2.fileName()));
    createXmlFile(fileV2, 2, NoFlags);
    fileToVersionMap.insert(fileV2.fileName(), 2);

    QTemporaryFile fileV5;
    QVERIFY(fileV5.open());
    createXmlFile(fileV5, 5, NoFlags);
    fileToVersionMap.insert(fileV5.fileName(), 5);

    // The highest version is neither the first nor last one in the list,
    // to make sure the code really selects the highest version, not just by chance :)
    // (This is why we add the v1 version at the end of the list)
    QTemporaryFile fileV1;
    QVERIFY(fileV1.open());
    createXmlFile(fileV1, 1, NoFlags);
    fileToVersionMap.insert(fileV1.fileName(), 1);

    QStringList files;
    files << fileV2.fileName() << fileV5.fileName() << fileV1.fileName();

    fileV2.close();
    fileV5.close();
    fileV1.close();

    KXmlGuiVersionHandler versionHandler(files);
    QCOMPARE(fileToVersionMap.value(versionHandler.finalFile()), 5);
    QString finalDoc = versionHandler.finalDocument();
    QVERIFY(finalDoc.startsWith(QLatin1String("<?xml")));
    QVERIFY(finalDoc.contains(QLatin1String("version=\"5\"")));

    QVERIFY(fileV5.open());
    const QString fileV5Contents = QString::fromUtf8(fileV5.readAll());
    QCOMPARE(finalDoc, fileV5Contents);
}

void KXmlGui_UnitTest::testVersionHandlerNewVersionUserChanges()
{
    // This emulates the case where the application has been upgraded
    // after the user has changed shortcuts and toolbars

    QMap<QString, int> fileToVersionMap; // makes QCOMPARE failures more readable than just temp filenames

    // local file
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kxmlgui_unittest");
    QFile fileV2(dir + QLatin1String("/testui.rc"));
    QVERIFY(fileV2.open(QIODevice::WriteOnly));
    createXmlFile(fileV2, 2, AddActionProperties | AddModifiedToolBars);
    fileToVersionMap.insert(fileV2.fileName(), 2);

    // more-global (application) file
    QTemporaryFile fileV5;
    QVERIFY(fileV5.open());
    createXmlFile(fileV5, 5, AddToolBars | AddModifiedMenus, "kpartgui");
    fileToVersionMap.insert(fileV5.fileName(), 5);

    // The highest version is neither the first nor last one in the list,
    // to make sure the code really selects the highest version, not just by chance :)
    // (This is why we add the v1 version at the end of the list)
    QTemporaryFile fileV1;
    QVERIFY(fileV1.open());
    createXmlFile(fileV1, 1, AddToolBars);
    fileToVersionMap.insert(fileV1.fileName(), 1);

    QStringList files;
    files << fileV2.fileName() << fileV5.fileName() << fileV1.fileName();

    fileV2.close();
    fileV5.close();
    fileV1.close();

    KXmlGuiVersionHandler versionHandler(files);
    // We end up with the local file, so in our map it has version 2.
    // But of course by now it says "version=5" in it :)
    QCOMPARE(fileToVersionMap.value(versionHandler.finalFile()), 2);
    const QString finalDoc = versionHandler.finalDocument();
    // qDebug() << finalDoc;
    QVERIFY(finalDoc.startsWith(QLatin1String("<?xml")));
    QVERIFY(finalDoc.contains(QLatin1String("version=\"5\"")));
    // Check that the shortcuts defined by the user were kept
    QVERIFY(finalDoc.contains(QLatin1String("<ActionProperties>")));
    QVERIFY(finalDoc.contains(QLatin1String("sidebartng")));
    // Check that the menus modified by the app are still there
    QVERIFY(finalDoc.contains(QLatin1String("<Action name=\"file_open\"")));
    // Check that the toolbars modified by the user were kept
    QVERIFY(finalDoc.contains(QLatin1String("<Action name=\"home\"")));
    // Check that the toolbars added by the application were kept (https://invent.kde.org/graphics/okular/-/merge_requests/197)
    QVERIFY(finalDoc.contains(QLatin1String("<ToolBar name=\"newToolBar\"")));
}

static QStringList collectMenuNames(KXMLGUIFactory &factory)
{
    const QList<QWidget *> containers = factory.containers(QStringLiteral("Menu"));
    QStringList containerNames;
    for (QWidget *w : containers) {
        containerNames << w->objectName();
    }
    return containerNames;
}

void debugActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        qDebug() << (action->isSeparator() ? QString::fromLatin1("separator") : action->objectName());
    }
}

static void checkActions(const QList<QAction *> &actions, const QStringList &expectedActions)
{
    for (int i = 0; i < expectedActions.count(); ++i) {
        if (i >= actions.count()) {
            break;
        }
        QAction *action = actions.at(i);
        if (action->isSeparator()) {
            QCOMPARE(QStringLiteral("separator"), expectedActions[i]);
        } else {
            QCOMPARE(action->objectName(), expectedActions[i]);
        }
    }
    QCOMPARE(actions.count(), expectedActions.count());
}

void KXmlGui_UnitTest::testPartMerging()
{
    const QByteArray hostXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"go\"><text>&amp;Go</text>\n"
        "  <!-- go_up, go_back, go_forward, go_home, separator: coming from ui_standards.rc -->\n"
        "  <DefineGroup name=\"before_merge\"/>\n"
        "  <Merge/>\n"
        "  <Action name=\"host_after_merge\"/>\n"
        "  <Action name=\"host_after_merge_2\"/>\n"
        "  <Separator/>\n"
        "  <DefineGroup name=\"new_merge\"/>\n"
        "  <Title>Section title</Title>\n"
        "  <Action name=\"last_from_host\"/>\n"
        " </Menu>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <DefineGroup name=\"placed_merge\" append=\"new_merge\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>\n";

    TestGuiClient hostClient;
    hostClient.createActions(QStringList() << QStringLiteral("go_up") << QStringLiteral("go_back") << QStringLiteral("go_forward") << QStringLiteral("go_home")
                                           << QStringLiteral("host_after_merge") << QStringLiteral("host_after_merge_2") << QStringLiteral("last_from_host")
                                           << QStringLiteral("file_new") << QStringLiteral("file_open") << QStringLiteral("file_quit"));
    hostClient.createGUI(hostXml, true /*ui_standards.rc*/);
    KMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&hostClient);

    const QString hostDomDoc = hostClient.domDocument().toString();

    QWidget *goMenuW = factory.container(QStringLiteral("go"), &hostClient);
    QVERIFY(goMenuW);
    QMenu *goMenu = qobject_cast<QMenu *>(goMenuW);
    QVERIFY(goMenu);
    QMenu *fileMenu = qobject_cast<QMenu *>(factory.container(QStringLiteral("file"), &hostClient));

    // debugActions(goMenu->actions());
    // clang-format off
    checkActions(goMenu->actions(), QStringList()
                 << QStringLiteral("go_up")
                 << QStringLiteral("go_back")
                 << QStringLiteral("go_forward")
                 << QStringLiteral("go_home")
                 << QStringLiteral("separator")
                 << QStringLiteral("host_after_merge")
                 << QStringLiteral("host_after_merge_2")
                 << QStringLiteral("separator")
                 << QStringLiteral("separator") // <Title> separator
                 << QStringLiteral("last_from_host"));
    checkActions(fileMenu->actions(), QStringList()
                 << QStringLiteral("file_new")
                 << QStringLiteral("file_open")
                 << QStringLiteral("separator")
                 << QStringLiteral("file_quit"));
    // clang-format on

    qDebug() << "Now merging the part";

    const QByteArray partXml =
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"part\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"go\"><text>&amp;Go</text>\n"
        "  <Action name=\"go_previous\" group=\"before_merge\"/>\n"
        "  <Action name=\"go_next\" group=\"before_merge\"/>\n"
        "  <Separator/>\n"
        "  <Action name=\"first_page\"/>\n"
        "  <Action name=\"last_page\"/>\n"
        "  <Title>Part Section</Title>\n"
        "  <ActionList name=\"action_list\"/>\n"
        "  <Action name=\"action_in_merge_group\" group=\"new_merge\"/>\n"
        "  <Action name=\"undefined_group\" group=\"no_such_merge\"/>\n"
        "  <Action name=\"last_from_part\"/>\n"
        " </Menu>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <Action group=\"placed_merge\" name=\"action_in_placed_merge\"/>\n"
        "  <Action name=\"other_file_action\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>\n";

    TestGuiClient partClient(partXml);
    partClient.createActions(QStringList() << QStringLiteral("go_previous") << QStringLiteral("go_next") << QStringLiteral("first_page")
                                           << QStringLiteral("last_page") << QStringLiteral("last_from_part") << QStringLiteral("action_in_merge_group")
                                           << QStringLiteral("undefined_group") << QStringLiteral("action_in_placed_merge")
                                           << QStringLiteral("other_file_action") << QStringLiteral("action1") << QStringLiteral("action2"));
    const QList<QAction *> actionList = {partClient.actionCollection()->action(QStringLiteral("action1")),
                                         partClient.actionCollection()->action(QStringLiteral("action2"))};

    for (int i = 0; i < 5; ++i) {
        // qDebug() << "addClient, iteration" << i;
        factory.addClient(&partClient);
        partClient.plugActionList(QStringLiteral("action_list"), actionList);

        // clang-format off
        //debugActions(goMenu->actions());
        checkActions(goMenu->actions(), QStringList()
                << QStringLiteral("go_up")
                << QStringLiteral("go_back")
                << QStringLiteral("go_forward")
                << QStringLiteral("go_home")
                << QStringLiteral("separator")
                << QStringLiteral("go_previous")
                << QStringLiteral("go_next")
                // Contents of the <Merge>:
                << QStringLiteral("separator")
                << QStringLiteral("first_page")
                << QStringLiteral("last_page")
                << QStringLiteral("separator") // <title> in the part
                << QStringLiteral("action1")
                << QStringLiteral("action2")
                << QStringLiteral("undefined_group")
                << QStringLiteral("last_from_part")
                // End of <Merge>
                << QStringLiteral("host_after_merge")
                << QStringLiteral("host_after_merge_2")
                << QStringLiteral("separator")
                // Contents of <DefineGroup>
                << QStringLiteral("action_in_merge_group")
                // End of <DefineGroup>
                << QStringLiteral("separator") // <title> is a separator qaction with text
                << QStringLiteral("last_from_host")
                );
        checkActions(fileMenu->actions(), QStringList()
                << QStringLiteral("file_new")
                << QStringLiteral("action_in_placed_merge")
                << QStringLiteral("file_open")
                << QStringLiteral("separator")
                << QStringLiteral("file_quit")
                << QStringLiteral("other_file_action"));
        factory.removeClient(&partClient);
        QCOMPARE(hostClient.domDocument().toString(), hostDomDoc);
        // clang-format on
    }
    factory.removeClient(&hostClient);
}

void KXmlGui_UnitTest::testPartMergingSettings() // #252911
{
    const QByteArray hostXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        // The solution was to remove the duplicated definition
        //        " <Menu name=\"settings\"><text>&amp;Settings</text>\n"
        //        "    <Action name=\"options_configure_keybinding\"/>\n"
        //        "    <Action name=\"options_configure_toolbars\"/>\n"
        //        "    <Merge name=\"configure_merge\"/>\n"
        //        "    <Separator/>\n"
        //        "    <Merge/>\n"
        //        " </Menu>\n"
        "</MenuBar></gui>\n";
    TestGuiClient hostClient;
    hostClient.createActions(QStringList() << QStringLiteral("options_configure_keybinding") << QStringLiteral("options_configure_toolbars"));
    hostClient.createGUI(hostXml, true /*ui_standards.rc*/);
    // qDebug() << hostClient.domDocument().toString();
    KMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&hostClient);
    QWidget *settingsMenu = qobject_cast<QMenu *>(factory.container(QStringLiteral("settings"), &hostClient));
    QVERIFY(settingsMenu);
    // debugActions(settingsMenu->actions());

    const QByteArray partXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"settings\"><text>&amp;Settings</text>\n"
        "    <Action name=\"configure_klinkstatus\"/>\n"
        " </Menu>\n"
        "</MenuBar></gui>\n";
    TestGuiClient partClient(partXml);
    partClient.createActions(QStringList() << QStringLiteral("configure_klinkstatus"));
    factory.addClient(&partClient);
    // debugActions(settingsMenu->actions());
    checkActions(settingsMenu->actions(),
                 QStringList() << QStringLiteral("separator") // that's ok, QMenuPrivate::filterActions won't show it
                               << QStringLiteral("options_configure_keybinding") << QStringLiteral("options_configure_toolbars")
                               << QStringLiteral("configure_klinkstatus"));
    factory.removeClient(&partClient);
    factory.removeClient(&hostClient);
}

void KXmlGui_UnitTest::testUiStandardsMerging_data()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QStringList>("actions");
    QTest::addColumn<QStringList>("expectedMenus");

    const QByteArray xmlBegin =
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n";
    const QByteArray xmlEnd =
        "</MenuBar>\n"
        "</gui>";

    // Merging an empty menu (or a menu with only non-existing actions) would make
    // the empty menu appear at the end after all other menus (fixed for KDE-4.2)
    /* clang-format off */
    QTest::newRow("empty file menu, implicit settings menu")
        << QByteArray(xmlBegin + "<Menu name=\"file\"/>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("settings"));
    QTest::newRow("file menu with non existing action, implicit settings menu")
        << QByteArray(xmlBegin + "<Menu name=\"file\"><Action name=\"foo\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("settings"));
    QTest::newRow("file menu with existing action, implicit settings menu")
        << QByteArray(xmlBegin + "<Menu name=\"file\"><Action name=\"open\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("open") << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("settings"));
    QTest::newRow("implicit file and settings menu")
        << QByteArray(xmlBegin + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("settings")); // we could check that file_open is in the mainToolBar, too

    // Check that unknown non-empty menus are added at the "MergeLocal" position (before settings).
    QTest::newRow("foo menu added at the end")
        << QByteArray(xmlBegin + "<Menu name=\"foo\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("options_configure_toolbars") << QStringLiteral("foo_action"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("foo") << QStringLiteral("settings"));

    QTest::newRow("Bille's testcase: menu patch + menu edit")
        << QByteArray(xmlBegin + "<Menu name=\"patch\"><Action name=\"patch_generate\"/></Menu>\n"
                      + "<Menu name=\"edit\"><Action name=\"edit_foo\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("patch_generate") << QStringLiteral("edit_foo"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("edit") << QStringLiteral("patch"));
    QTest::newRow("Bille's testcase: menu patch + menu edit, lowercase tag")
        << QByteArray(xmlBegin + "<Menu name=\"patch\"><Action name=\"patch_generate\"/></Menu>\n"
                      + "<menu name=\"edit\"><Action name=\"edit_foo\"/></menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("patch_generate") << QStringLiteral("edit_foo"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("edit") << QStringLiteral("patch"));

    // Check that <Menu append="..."> allows to insert menus at specific positions
    QTest::newRow("Menu append")
        << QByteArray(xmlBegin + "<Menu name=\"foo\" append=\"settings_merge\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("options_configure_toolbars")
                          << QStringLiteral("foo_action") << QStringLiteral("help_contents"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("settings") << QStringLiteral("foo") << QStringLiteral("help"));
    QTest::newRow("Custom first menu")
        << QByteArray(xmlBegin + "<Menu name=\"foo\" append=\"first_menu\"><Action name=\"foo_action\"/></Menu>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("edit_undo") << QStringLiteral("foo_action") << QStringLiteral("help_contents"))
        << (QStringList() << QStringLiteral("foo") << QStringLiteral("edit") << QStringLiteral("help"));

    // Tests for noMerge="1"
    QTest::newRow("noMerge empty file menu, implicit settings menu")
        << QByteArray(xmlBegin + "<Menu name=\"file\" noMerge=\"1\"/>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("settings")); // we keep empty menus, see #186382
    QTest::newRow("noMerge empty file menu, file_open moved elsewhere")
        << QByteArray(xmlBegin + "<Menu name=\"file\" noMerge=\"1\"/>\n<Menu name=\"foo\"><Action name=\"file_open\"/></Menu>" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open"))
        << (QStringList() << QStringLiteral("file") << QStringLiteral("foo"));
    QTest::newRow("noMerge file menu with open before new")
        << QByteArray(xmlBegin + "<Menu name=\"file\" noMerge=\"1\"><Action name=\"file_open\"/><Action name=\"file_new\"/></Menu>" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("file_new"))
        << (QStringList() << QStringLiteral("file")); // TODO check the order of the actions in the menu? how?

    // Tests for deleted="true"
    QTest::newRow("deleted file menu, implicit settings menu")
        << QByteArray(xmlBegin + "<Menu name=\"file\" deleted=\"true\"/>\n" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("options_configure_toolbars"))
        << (QStringList() << QStringLiteral("settings"));
    QTest::newRow("deleted file menu, file_open moved elsewhere")
        << QByteArray(xmlBegin + "<Menu name=\"file\" deleted=\"true\"/>\n<Menu name=\"foo\"><Action name=\"file_open\"/></Menu>" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open"))
        << (QStringList() << QStringLiteral("foo"));
    QTest::newRow("deleted file menu with actions (contradiction)")
        << QByteArray(xmlBegin + "<Menu name=\"file\" deleted=\"true\"><Action name=\"file_open\"/><Action name=\"file_new\"/></Menu>" + xmlEnd)
        << (QStringList() << QStringLiteral("file_open") << QStringLiteral("file_new"))
        << (QStringList());
    /* clang-format on */
}

void KXmlGui_UnitTest::testUiStandardsMerging()
{
    QFETCH(QByteArray, xml);
    QFETCH(QStringList, actions);
    QFETCH(QStringList, expectedMenus);

    TestGuiClient client;
    client.createActions(actions);
    client.createGUI(xml, true /*ui_standards.rc*/);

    const QDomDocument domDocument = client.domDocument();
    const QDomElement docElem = domDocument.documentElement();
    QCOMPARE(docElem.attribute(QStringLiteral("name")), QStringLiteral("foo")); // not standard_containers from ui_standards.rc
    QCOMPARE(docElem.attribute(QStringLiteral("version")), QStringLiteral("1")); // not standard_containers from ui_standards.rc

    KMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);

    const QStringList containerNames = collectMenuNames(factory);
    // qDebug() << containerNames;
    QCOMPARE(containerNames, expectedMenus);

    factory.removeClient(&client);
}

void KXmlGui_UnitTest::testActionListAndSeparator()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"groups\"><text>Add to Group</text>\n"
        "  <ActionList name=\"view_groups_list\"/>\n"
        "  <Separator />"
        "  <Action name=\"view_add_to_new_group\" />\n"
        "  <ActionList name=\"second_list\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestGuiClient client(xml);
    client.createActions(QStringList() << QStringLiteral("view_add_to_new_group"));
    KMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);

    QWidget *menuW = factory.container(QStringLiteral("groups"), &client);
    QVERIFY(menuW);
    QMenu *menu = qobject_cast<QMenu *>(menuW);
    QVERIFY(menu);

    // debugActions(menu->actions());
    checkActions(menu->actions(),
                 QStringList() << QStringLiteral("separator") // that's ok, QMenuPrivate::filterActions won't show it
                               << QStringLiteral("view_add_to_new_group"));

    qDebug() << "Now plugging the actionlist";

    QAction *action1 = new QAction(this);
    action1->setObjectName(QStringLiteral("action1"));
    client.actionCollection()->setDefaultShortcut(action1, QKeySequence(QStringLiteral("Ctrl+2")));
    QAction *action2 = new QAction(this);
    action2->setObjectName(QStringLiteral("action2"));
    const QList<QAction *> actionList = {action1, action2};
    client.plugActionList(QStringLiteral("view_groups_list"), actionList);
    QCOMPARE(QKeySequence::listToString(action1->shortcuts()), QStringLiteral("Ctrl+2"));

    const QStringList expectedActionsOneList = {QStringLiteral("action1"),
                                                QStringLiteral("action2"),
                                                QStringLiteral("separator"),
                                                QStringLiteral("view_add_to_new_group")};
    // debugActions(menu->actions());
    checkActions(menu->actions(), expectedActionsOneList);

    QAction *action3 = new QAction(this);
    action3->setObjectName(QStringLiteral("action3"));
    const QList<QAction *> secondActionList = {action3};
    client.plugActionList(QStringLiteral("second_list"), secondActionList);
    QStringList expectedActions = expectedActionsOneList;
    expectedActions << QStringLiteral("action3");
    checkActions(menu->actions(), expectedActions);

    qDebug() << "Now remove+add gui client";

    // While I'm here, what happens with the action list if I remove+add the guiclient,
    // like KXmlGuiWindow::newToolBarConfig does?
    factory.removeClient(&client);
    factory.addClient(&client);
    // We need to get the container widget again, it was re-created.
    menuW = factory.container(QStringLiteral("groups"), &client);
    QVERIFY(menuW);
    menu = qobject_cast<QMenu *>(menuW);
    // debugActions(menu->actions());
    checkActions(menu->actions(),
                 QStringList() << QStringLiteral("separator") // yep, it removed the actionlist thing...
                               << QStringLiteral("view_add_to_new_group"));
    qDebug() << "Now plugging the actionlist again";
    client.plugActionList(QStringLiteral("second_list"), secondActionList);
    client.plugActionList(QStringLiteral("view_groups_list"), actionList);
    checkActions(menu->actions(), expectedActions);
    factory.removeClient(&client);
}

void KXmlGui_UnitTest::testHiddenToolBar()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar hidden=\"true\" name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"visibleToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar hidden=\"true\" name=\"hiddenToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    KConfigGroup cg(KSharedConfig::openConfig(), "testHiddenToolBar");
    TestXmlGuiWindow mainWindow(xml, "kxmlgui_unittest.rc");
    mainWindow.setAutoSaveSettings(cg);
    mainWindow.createActions(QStringList() << QStringLiteral("go_up"));
    mainWindow.createGUI();

    KToolBar *mainToolBar = mainWindow.toolBarByName(QStringLiteral("mainToolBar"));
    QVERIFY(mainToolBar->isHidden());

    KXMLGUIFactory *factory = mainWindow.guiFactory();
    QVERIFY(!factory->container(QStringLiteral("visibleToolBar"), &mainWindow)->isHidden());
    KToolBar *hiddenToolBar = qobject_cast<KToolBar *>(factory->container(QStringLiteral("hiddenToolBar"), &mainWindow));
    qDebug() << hiddenToolBar;
    QVERIFY(hiddenToolBar->isHidden());

    // Now open KEditToolBar (#105525)
    KEditToolBar editToolBar(factory);
    // KEditToolBar loads the stuff in showEvent...
    QShowEvent ev;
    qApp->sendEvent(&editToolBar, &ev);
    clickApply(&editToolBar);
    QVERIFY(qobject_cast<KToolBar *>(factory->container(QStringLiteral("hiddenToolBar"), &mainWindow))->isHidden());

    mainWindow.close();
}

void KXmlGui_UnitTest::testCustomPlaceToolBar()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar name=\"mainToolBar\" noMerge=\"1\">\n"
        "  <text>Main Toolbar</text>\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"customToolBar\" noMerge=\"1\">\n"
        "  <text>Custom Toolbar</text>\n"
        "  <Action name=\"go_up\"/>\n"
        "  <Action name=\"file_open\"/>\n"
        "  <Action name=\"help_about_kde\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    KConfigGroup cg(KSharedConfig::openConfig(), "testHiddenToolBar");
    TestXmlGuiWindow mw(xml, "kxmlgui_unittest.rc");
    mw.setAutoSaveSettings(cg);

    QWidget *centralWidget = new QWidget(&mw);
    centralWidget->setGeometry(0, 0, 100, 100);
    mw.setCentralWidget(centralWidget);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    KToolBar *customToolBar = new KToolBar(QStringLiteral("customToolBar"), centralWidget);
    layout->addWidget(customToolBar);

    mw.createActions(QStringList() << QStringLiteral("go_up"));
    mw.createGUI();

    QVERIFY(centralWidget->findChild<KToolBar *>(QStringLiteral("customToolBar")) != nullptr);

    QVERIFY2(mw.toolBarArea(customToolBar) == Qt::NoToolBarArea, "The custom toolbar should not be in a ToolBarArea of the main window");

    // Now open KEditToolBar, just to check it doesn't crash on apply
    KXMLGUIFactory *factory = mw.guiFactory();
    KEditToolBar editToolBar(factory);
    // KEditToolBar loads the stuff in showEvent...
    QShowEvent ev;
    qApp->sendEvent(&editToolBar, &ev);
    clickApply(&editToolBar);

    QVERIFY2(centralWidget->findChild<KToolBar *>(QStringLiteral("customToolBar")) != nullptr,
             "After a KEditToolBar action the custom toolbar should stay in the widget it was orignally added");
    QVERIFY2(mw.toolBarArea(mw.toolBarByName(QStringLiteral("customToolBar"))) == Qt::NoToolBarArea,
             "After a KEditToolBar action the custom toolbar should not be in a ToolBarArea of the main window");

    mw.close();
}

// taken from KMainWindow_UnitTest::testAutoSaveSettings()
void KXmlGui_UnitTest::testAutoSaveSettings()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "</MenuBar>\n"
        "<ToolBar name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"secondToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    {
        // do not interfere with the "toolbarVisibility" unit test
        KConfigGroup cg(KSharedConfig::openConfig(), "testAutoSaveSettings");
        TestXmlGuiWindow mw(xml, "kxmlgui_unittest.rc");
        mw.show();
        mw.setAutoSaveSettings(cg);

        // Test resizing first (like show() does).
        mw.reallyResize(400, 400);
        QTest::qWait(200);

        mw.createActions(QStringList() << QStringLiteral("go_up"));
        mw.createGUI();

        // Resize again, should be saved
        mw.reallyResize(800, 600);
        QTest::qWait(200);

        KToolBar *mainToolBar = mw.toolBarByName(QStringLiteral("mainToolBar"));
        QCOMPARE(mw.toolBarArea(mainToolBar), Qt::TopToolBarArea);
        KToolBar *secondToolBar = mw.toolBarByName(QStringLiteral("secondToolBar"));
        QCOMPARE((int)mw.toolBarArea(secondToolBar), (int)Qt::TopToolBarArea); // REFERENCE #1 (see below)

        // Move second toolbar to bottom
        const QPoint oldPos = secondToolBar->pos();
        mw.addToolBar(Qt::BottomToolBarArea, secondToolBar);
        const QPoint newPos = secondToolBar->pos();
        QCOMPARE(mw.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
        // Calling to addToolBar is not enough to trigger the event filter for move events
        // in KMainWindow, because there is no layouting happening in hidden mainwindows.
        QMoveEvent moveEvent(newPos, oldPos);
        QApplication::sendEvent(secondToolBar, &moveEvent);

        mw.close();
    }

    {
        KConfigGroup cg(KSharedConfig::openConfig(), "testAutoSaveSettings");
        TestXmlGuiWindow mw2(xml, "kxmlgui_unittest.rc");
        mw2.show();
        mw2.setAutoSaveSettings(cg);
        QTest::qWait(200);
        // Check window size was restored
        QCOMPARE(mw2.size(), QSize(800, 600));

        mw2.createActions(QStringList() << QStringLiteral("go_up"));
        mw2.createGUI();

        // Force window layout to happen
        mw2.reallyResize(800, 600);
        QTest::qWait(200);

        // Check toolbar positions were restored
        KToolBar *mainToolBar = mw2.toolBarByName(QStringLiteral("mainToolBar"));
        QCOMPARE(mw2.toolBarArea(mainToolBar), Qt::TopToolBarArea);
        KToolBar *secondToolBar = mw2.toolBarByName(QStringLiteral("secondToolBar"));
        QCOMPARE(mw2.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
        mw2.applyMainWindowSettings(mw2.autoSaveConfigGroup());
        QCOMPARE(mw2.toolBarArea(secondToolBar), Qt::BottomToolBarArea);
    }
}

void KXmlGui_UnitTest::testDeletedContainers() // deleted="true"
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        "  <Menu deleted=\"true\" name=\"game\"/>\n"
        "</MenuBar>\n"
        "<ToolBar deleted=\"true\" name=\"mainToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar name=\"visibleToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "<ToolBar deleted=\"true\" name=\"deletedToolBar\">\n"
        "  <Action name=\"go_up\"/>\n"
        "</ToolBar>\n"
        "</gui>\n";
    KConfigGroup cg(KSharedConfig::openConfig(), "testDeletedToolBar");
    TestXmlGuiWindow mainWindow(xml, "kxmlgui_unittest.rc");
    mainWindow.setAutoSaveSettings(cg);
    mainWindow.createActions(QStringList() << QStringLiteral("go_up") << QStringLiteral("file_new") << QStringLiteral("game_new"));
    mainWindow.createGUI();
    KXMLGUIFactory *factory = mainWindow.guiFactory();

    // qDebug() << "containers:" << factory->containers("ToolBar");
    QVERIFY(!factory->container(QStringLiteral("mainToolBar"), &mainWindow));
    QVERIFY(!factory->container(QStringLiteral("visibleToolBar"), &mainWindow)->isHidden());
    QVERIFY(!factory->container(QStringLiteral("deletedToolBar"), &mainWindow));
    QVERIFY(factory->container(QStringLiteral("file"), &mainWindow)); // File menu was created
    QVERIFY(!factory->container(QStringLiteral("game"), &mainWindow)); // Game menu was not created

    // Now open KEditToolBar, just to check it doesn't crash.
    KEditToolBar editToolBar(factory);
    // KEditToolBar loads the stuff in showEvent...
    QShowEvent ev;
    qApp->sendEvent(&editToolBar, &ev);
    clickApply(&editToolBar);
    QVERIFY(!factory->container(QStringLiteral("mainToolBar"), &mainWindow));
    QVERIFY(!factory->container(QStringLiteral("visibleToolBar"), &mainWindow)->isHidden());
    QVERIFY(!factory->container(QStringLiteral("deletedToolBar"), &mainWindow));
    QVERIFY(factory->container(QStringLiteral("file"), &mainWindow));
    QVERIFY(!factory->container(QStringLiteral("game"), &mainWindow));

    mainWindow.close();
}

void KXmlGui_UnitTest::testTopLevelSeparator()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"before_separator\"><text>Before Separator</text></Menu>\n"
        " <Separator />\n"
        " <Menu name=\"after_separator\"><text>After Separator</text></Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml, "kxmlgui_unittest.rc");
    mainWindow.createGUI();

    checkActions(mainWindow.menuBar()->actions(),
                 QStringList() << QStringLiteral("before_separator") << QStringLiteral("separator") << QStringLiteral("after_separator")
                               << QStringLiteral("settings") << QStringLiteral("separator") << QStringLiteral("help"));
}

// Check that the objectName() of the menus is set from the name in the XML file
void KXmlGui_UnitTest::testMenuNames()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"filemenu\"><text>File Menu</text></Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml, "kxmlgui_unittest.rc");
    mainWindow.createGUI();

    checkActions(mainWindow.menuBar()->actions(),
                 QStringList() << QStringLiteral("filemenu") << QStringLiteral("settings") << QStringLiteral("separator") << QStringLiteral("help"));
}

// Test what happens when the application's rc file isn't found
// We want a warning to be printed, but we don't want to see all menus from ui_standards.rc
void KXmlGui_UnitTest::testMenusNoXmlFile()
{
    TestXmlGuiWindow mainWindow(QByteArray(), "nolocalfile_either.rc");
    mainWindow.createGUIBad();

    checkActions(mainWindow.menuBar()->actions(), QStringList{QStringLiteral("settings"), QStringLiteral("separator"), QStringLiteral("help")});
}

void KXmlGui_UnitTest::testXMLFileReplacement()
{
    // to differentiate "original" and replacement xml file, one is created with "modified" toolbars
    QTemporaryFile fileOrig;
    QVERIFY(fileOrig.open());
    createXmlFile(fileOrig, 2, AddToolBars);
    const QString filenameOrig = fileOrig.fileName();
    fileOrig.close();

    QTemporaryFile fileReplace;
    QVERIFY(fileReplace.open());
    createXmlFile(fileReplace, 2, AddModifiedToolBars);
    const QString filenameReplace = fileReplace.fileName();
    fileReplace.close();

    // finally, our local xml file has <ActionProperties/>
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kxmlgui_unittest");
    QFile fileLocal(dir + QLatin1String("/testui.rc"));
    QVERIFY2(fileLocal.open(QIODevice::WriteOnly), qPrintable(fileLocal.fileName()));
    createXmlFile(fileLocal, 1, AddActionProperties);
    const QString filenameLocal = fileLocal.fileName();
    fileLocal.close();

    TestGuiClient client;
    // first make sure that the "original" file is loaded, correctly
    client.setXMLFilePublic(filenameOrig);
    QString xml = client.domDocument().toString();
    // qDebug() << xml;
    QVERIFY(xml.contains(QLatin1String("<Action name=\"print\"")));
    QVERIFY(!xml.contains(QLatin1String("<Action name=\"home\"")));
    QVERIFY(!xml.contains(QLatin1String("<ActionProperties>")));

    // now test the replacement (+ local file)
    client.replaceXMLFile(filenameReplace, filenameLocal);
    xml = client.domDocument().toString();
    QVERIFY(!xml.contains(QLatin1String("<Action name=\"print\"")));
    QVERIFY(xml.contains(QLatin1String("<Action name=\"home\"")));
    QVERIFY(xml.contains(QLatin1String("<ActionProperties>")));

    // re-check after a reload
    client.reloadXML();
    QString reloadedXml = client.domDocument().toString();
    QVERIFY(!reloadedXml.contains(QLatin1String("<Action name=\"print\"")));
    QVERIFY(reloadedXml.contains(QLatin1String("<Action name=\"home\"")));
    QVERIFY(reloadedXml.contains(QLatin1String("<ActionProperties>")));

    // Check what happens when the local file doesn't exist
    TestGuiClient client2;
    QFile::remove(filenameLocal);
    client2.replaceXMLFile(filenameReplace, filenameLocal);
    xml = client2.domDocument().toString();
    // qDebug() << xml;
    QVERIFY(!xml.contains(QLatin1String("<Action name=\"print\"")));
    QVERIFY(xml.contains(QLatin1String("<Action name=\"home\""))); // modified toolbars
    QVERIFY(!xml.contains(QLatin1String("<ActionProperties>"))); // but no local xml file
}

void KXmlGui_UnitTest::testClientDestruction() // #170806
{
    const QByteArray hostXml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        " </Menu>\n"
        " <Merge/>\n"
        "</MenuBar>\n"
        "</gui>";
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <Action name=\"file_open\"/>\n"
        "  <Action name=\"file_quit\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(hostXml, "kxmlgui_unittest.rc");
    TestGuiClient *client = new TestGuiClient(xml);
    client->createActions(QStringList() << QStringLiteral("file_open") << QStringLiteral("file_quit"));
    mainWindow.insertChildClient(client);
    mainWindow.createGUI();

    const QStringList menus = {QStringLiteral("file"), QStringLiteral("settings"), QStringLiteral("separator"), QStringLiteral("help")};
    checkActions(mainWindow.menuBar()->actions(), menus);

    QVERIFY(mainWindow.factory()->clients().contains(client));
    delete client;
    QVERIFY(!mainWindow.factory()->clients().contains(client));

    // No change, because deletion is fast, it doesn't do manual unplugging.
    checkActions(mainWindow.menuBar()->actions(), menus);
}

void KXmlGui_UnitTest::testShortcuts()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<MenuBar>\n"
        " <Menu name=\"file\"><text>&amp;File</text>\n"
        "  <Action name=\"file_open\" shortcut=\"Ctrl+O\"/>\n"
        "  <Action name=\"file_quit\" shortcut=\"Ctrl+Q; Ctrl+D\"/>\n"
        " </Menu>\n"
        "</MenuBar>\n"
        "<ActionProperties scheme=\"Default\">\n"
        "  <Action shortcut=\"Ctrl+O\" name=\"file_open\"/>\n"
        "  <Action shortcut=\"Ctrl+Q; Ctrl+D\" name=\"file_quit\"/>\n"
        "</ActionProperties>\n"
        "</gui>";

    TestGuiClient client;
    client.createActions(QStringList() << QStringLiteral("file_open") << QStringLiteral("file_quit"));
    client.createGUI(xml, false /*ui_standards.rc*/);

    KMainWindow mainWindow;
    KXMLGUIBuilder builder(&mainWindow);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);

    QAction *actionOpen = client.action("file_open");
    QAction *actionQuit = client.action("file_quit");
    QVERIFY(actionOpen && actionQuit);
    QCOMPARE(actionOpen->shortcuts(), QList<QKeySequence>() << QKeySequence(QStringLiteral("Ctrl+O")));
    // #345411
    QCOMPARE(actionQuit->shortcuts(), QList<QKeySequence>() << QKeySequence(QStringLiteral("Ctrl+Q")) << QKeySequence(QStringLiteral("Ctrl+D")));

    factory.removeClient(&client);
}

void KXmlGui_UnitTest::testPopupMenuParent()
{
    const QByteArray xml =
        "<?xml version = '1.0'?>\n"
        "<!DOCTYPE gui SYSTEM \"kpartgui.dtd\">\n"
        "<gui version=\"1\" name=\"foo\" >\n"
        "<Menu name=\"foo\"><text>Foo</text></Menu>\n"
        "</gui>";

    TestXmlGuiWindow mainWindow(xml, "kxmlgui_unittest.rc");
    mainWindow.createGUI();

    auto popupMenu = mainWindow.menuByName(QStringLiteral("foo"));
    QVERIFY(popupMenu);
    QCOMPARE(popupMenu->parent(), &mainWindow);
}

void KXmlGui_UnitTest::testSpecificApplicationLanguageQLocale()
{
    qunsetenv("LC_ALL"); // In case the user sets the LC_ALL ENV variable this test could fail
    const QLocale originalSystemLocale = QLocale::system();

    KDEPrivate::setApplicationSpecificLanguage("ru_RU");
    KDEPrivate::initializeLanguages();

#ifdef Q_OS_WIN
    QEXPECT_FAIL("",
                 "KDEPrivate::initializeLanguages assumes that setting $LANGUAGE will have an effect, this isn't the case on Windows. Needs to be fixed.",
                 Abort);
#endif
    QCOMPARE(QLocale::system().language(), QLocale::Russian);

    KDEPrivate::setApplicationSpecificLanguage("wa");
    KDEPrivate::initializeLanguages();

    QCOMPARE(QLocale::system().language(), QLocale::Walloon);

    KDEPrivate::setApplicationSpecificLanguage(QByteArray());
    KDEPrivate::initializeLanguages();

    QCOMPARE(QLocale::system(), originalSystemLocale);
}

void KXmlGui_UnitTest::testSingleModifierQKeySequenceEndsWithPlus()
{
    // Check that native texts of the Meta, Alt, Control, Shift, Keypad modifiers end in "+"
    // we depend on that in KKeySequenceWidgetPrivate::updateShortcutDisplay()
    QVERIFY(QKeySequence(Qt::MetaModifier).toString(QKeySequence::NativeText).endsWith(QLatin1Char('+')));
    QVERIFY(QKeySequence(Qt::AltModifier).toString(QKeySequence::NativeText).endsWith(QLatin1Char('+')));
    QVERIFY(QKeySequence(Qt::ControlModifier).toString(QKeySequence::NativeText).endsWith(QLatin1Char('+')));
    QVERIFY(QKeySequence(Qt::ShiftModifier).toString(QKeySequence::NativeText).endsWith(QLatin1Char('+')));
    QVERIFY(QKeySequence(Qt::KeypadModifier).toString(QKeySequence::NativeText).endsWith(QLatin1Char('+')));
}

void KXmlGui_UnitTest::testSaveShortcutsAndRefresh()
{
    QTemporaryFile xmlFile;
    QVERIFY(xmlFile.open());
    createXmlFile(xmlFile, 2, AddModifiedMenus);
    const QString filename = xmlFile.fileName();
    xmlFile.close();

    QTemporaryFile localXmlFile;
    QVERIFY(localXmlFile.open());
    const QString localFilename = localXmlFile.fileName();
    localXmlFile.close();

    TestGuiClient client;
    client.createActions({QStringLiteral("file_open")});
    client.setLocalXMLFilePublic(localFilename);
    client.setXMLFilePublic(filename);

    QWidget w;
    KXMLGUIBuilder builder(&w);
    KXMLGUIFactory factory(&builder);
    factory.addClient(&client);
    factory.removeClient(&client);
    factory.addClient(&client);

    QAction *a = client.actionCollection()->action(QStringLiteral("file_open"));
    QCOMPARE(a->shortcut(), QKeySequence());
    a->setShortcut(Qt::Key_F22);
    QCOMPARE(a->shortcut(), QKeySequence(Qt::Key_F22));

    client.actionCollection()->writeSettings();

    factory.refreshActionProperties();

    a = client.actionCollection()->action(QStringLiteral("file_open"));
    QCOMPARE(a->shortcut(), QKeySequence(Qt::Key_F22));
}
