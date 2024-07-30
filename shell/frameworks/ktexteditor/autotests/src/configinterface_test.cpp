/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2017 Dominik Haumann <dhaumann@kde.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configinterface_test.h"
#include "moc_configinterface_test.cpp"

#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>
#include <katedocument.h>
#include <kateglobal.h>
#include <katesyntaxmanager.h>
#include <kateview.h>

#include <QFont>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(KateConfigInterfaceTest)

KateConfigInterfaceTest::KateConfigInterfaceTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

KateConfigInterfaceTest::~KateConfigInterfaceTest()
{
}

void KateConfigInterfaceTest::testDocument()
{
    KTextEditor::DocumentPrivate doc(false, false);
    QVERIFY(!doc.configKeys().isEmpty());

    doc.setConfigValue(QStringLiteral("backup-on-save-local"), true);
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-local")).toBool(), true);
    doc.setConfigValue(QStringLiteral("backup-on-save-local"), false);
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-local")).toBool(), false);

    doc.setConfigValue(QStringLiteral("backup-on-save-remote"), true);
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-remote")).toBool(), true);
    doc.setConfigValue(QStringLiteral("backup-on-save-remote"), false);
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-remote")).toBool(), false);

    doc.setConfigValue(QStringLiteral("replace-tabs"), true);
    QCOMPARE(doc.configValue(QLatin1String("replace-tabs")).toBool(), true);
    doc.setConfigValue(QStringLiteral("replace-tabs"), false);
    QCOMPARE(doc.configValue(QLatin1String("replace-tabs")).toBool(), false);

    doc.setConfigValue(QStringLiteral("indent-pasted-text"), true);
    QCOMPARE(doc.configValue(QLatin1String("indent-pasted-text")).toBool(), true);
    doc.setConfigValue(QStringLiteral("indent-pasted-text"), false);
    QCOMPARE(doc.configValue(QLatin1String("indent-pasted-text")).toBool(), false);

    doc.setConfigValue(QStringLiteral("on-the-fly-spellcheck"), true);
    QCOMPARE(doc.configValue(QLatin1String("on-the-fly-spellcheck")).toBool(), true);
    doc.setConfigValue(QStringLiteral("on-the-fly-spellcheck"), false);
    QCOMPARE(doc.configValue(QLatin1String("on-the-fly-spellcheck")).toBool(), false);

    doc.setConfigValue(QStringLiteral("indent-width"), 13);
    QCOMPARE(doc.configValue(QLatin1String("indent-width")).toInt(), 13);
    doc.setConfigValue(QStringLiteral("indent-width"), 4);
    QCOMPARE(doc.configValue(QLatin1String("indent-width")).toInt(), 4);

    doc.setConfigValue(QStringLiteral("tab-width"), 13);
    QCOMPARE(doc.configValue(QLatin1String("tab-width")).toInt(), 13);
    doc.setConfigValue(QStringLiteral("tab-width"), 4);
    QCOMPARE(doc.configValue(QLatin1String("tab-width")).toInt(), 4);

    doc.setConfigValue(QStringLiteral("backup-on-save-suffix"), QLatin1String("_tmp"));
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-suffix")).toString(), QLatin1String("_tmp"));

    doc.setConfigValue(QStringLiteral("backup-on-save-prefix"), QLatin1String("abc_"));
    QCOMPARE(doc.configValue(QLatin1String("backup-on-save-prefix")).toString(), QLatin1String("abc_"));
}

void KateConfigInterfaceTest::testView()
{
    KTextEditor::DocumentPrivate doc(false, false);
    auto view = static_cast<KTextEditor::View *>(doc.createView(nullptr));
    QVERIFY(view);
    QVERIFY(!view->configKeys().isEmpty());

    view->setConfigValue(QStringLiteral("line-numbers"), true);
    QCOMPARE(view->configValue(QLatin1String("line-numbers")).toBool(), true);
    view->setConfigValue(QStringLiteral("line-numbers"), false);
    QCOMPARE(view->configValue(QLatin1String("line-numbers")).toBool(), false);

    view->setConfigValue(QStringLiteral("icon-bar"), true);
    QCOMPARE(view->configValue(QLatin1String("icon-bar")).toBool(), true);
    view->setConfigValue(QStringLiteral("icon-bar"), false);
    QCOMPARE(view->configValue(QLatin1String("icon-bar")).toBool(), false);

    view->setConfigValue(QStringLiteral("folding-bar"), true);
    QCOMPARE(view->configValue(QLatin1String("folding-bar")).toBool(), true);
    view->setConfigValue(QStringLiteral("folding-bar"), false);
    QCOMPARE(view->configValue(QLatin1String("folding-bar")).toBool(), false);

    view->setConfigValue(QStringLiteral("folding-preview"), true);
    QCOMPARE(view->configValue(QLatin1String("folding-preview")).toBool(), true);
    view->setConfigValue(QStringLiteral("folding-preview"), false);
    QCOMPARE(view->configValue(QLatin1String("folding-preview")).toBool(), false);

    view->setConfigValue(QStringLiteral("dynamic-word-wrap"), true);
    QCOMPARE(view->configValue(QLatin1String("dynamic-word-wrap")).toBool(), true);
    view->setConfigValue(QStringLiteral("dynamic-word-wrap"), false);
    QCOMPARE(view->configValue(QLatin1String("dynamic-word-wrap")).toBool(), false);

    view->setConfigValue(QStringLiteral("background-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("background-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("selection-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("selection-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("search-highlight-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("search-highlight-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("replace-highlight-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("replace-highlight-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("default-mark-type"), 6);
    QCOMPARE(view->configValue(QLatin1String("default-mark-type")).toInt(), 6);

    view->setConfigValue(QStringLiteral("allow-mark-menu"), true);
    QCOMPARE(view->configValue(QLatin1String("allow-mark-menu")).toBool(), true);
    view->setConfigValue(QStringLiteral("allow-mark-menu"), false);
    QCOMPARE(view->configValue(QLatin1String("allow-mark-menu")).toBool(), false);

    view->setConfigValue(QStringLiteral("icon-border-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("icon-border-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("folding-marker-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("folding-marker-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("line-number-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("line-number-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("current-line-number-color"), QColor(0, 255, 0));
    QCOMPARE(view->configValue(QLatin1String("current-line-number-color")).value<QColor>(), QColor(0, 255, 0));

    view->setConfigValue(QStringLiteral("modification-markers"), true);
    QCOMPARE(view->configValue(QLatin1String("modification-markers")).toBool(), true);
    view->setConfigValue(QStringLiteral("modification-markers"), false);
    QCOMPARE(view->configValue(QLatin1String("modification-markers")).toBool(), false);

    view->setConfigValue(QStringLiteral("word-count"), true);
    QCOMPARE(view->configValue(QLatin1String("word-count")).toBool(), true);
    view->setConfigValue(QStringLiteral("word-count"), false);
    QCOMPARE(view->configValue(QLatin1String("word-count")).toBool(), false);

    view->setConfigValue(QStringLiteral("line-count"), true);
    QCOMPARE(view->configValue(QLatin1String("line-count")).toBool(), true);
    view->setConfigValue(QStringLiteral("line-count"), false);
    QCOMPARE(view->configValue(QLatin1String("line-count")).toBool(), false);

    view->setConfigValue(QStringLiteral("scrollbar-minimap"), true);
    QCOMPARE(view->configValue(QLatin1String("scrollbar-minimap")).toBool(), true);
    view->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    QCOMPARE(view->configValue(QLatin1String("scrollbar-minimap")).toBool(), false);

    view->setConfigValue(QStringLiteral("scrollbar-preview"), true);
    QCOMPARE(view->configValue(QLatin1String("scrollbar-preview")).toBool(), true);
    view->setConfigValue(QStringLiteral("scrollbar-preview"), false);
    QCOMPARE(view->configValue(QLatin1String("scrollbar-preview")).toBool(), false);

    QFont f(QStringLiteral("Times"), 10, QFont::Bold);
    f.setHintingPreference(QFont::PreferFullHinting);
    view->setConfigValue(QStringLiteral("font"), f);
    QCOMPARE(view->configValue(QLatin1String("font")).value<QFont>(), f);

    {
        KSyntaxHighlighting::Repository &repository = KTextEditor::EditorPrivate::self()->hlManager()->repository();

        auto lightTheme = repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme);
        view->setConfigValue(QStringLiteral("theme"), lightTheme.name());
        QCOMPARE(view->configValue(QLatin1String("theme")).value<QString>(), lightTheme.name());

        auto darkTheme = repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme);
        view->setConfigValue(QStringLiteral("theme"), darkTheme.name());
        QCOMPARE(view->configValue(QLatin1String("theme")).value<QString>(), darkTheme.name());
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
