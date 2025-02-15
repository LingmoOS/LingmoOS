/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KXMLGUI_UNITTEST_H
#define KXMLGUI_UNITTEST_H

#include <QObject>

class KXmlGui_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testFindVersionNumber_data();
    void testFindVersionNumber();
    void testVersionHandlerSameVersion();
    void testVersionHandlerNewVersionNothingKept();
    void testVersionHandlerNewVersionUserChanges();
    void testPartMerging();
    void testPartMergingSettings();
    void testUiStandardsMerging_data();
    void testUiStandardsMerging();
    void testActionListAndSeparator();
    void testHiddenToolBar();
    void testCustomPlaceToolBar();
    void testDeletedContainers();
    void testAutoSaveSettings();
    void testXMLFileReplacement();
    void testTopLevelSeparator();
    void testMenuNames();
    void testClientDestruction();
    void testMenusNoXmlFile();
    void testShortcuts();
    void testPopupMenuParent();
    void testSpecificApplicationLanguageQLocale();
    void testSingleModifierQKeySequenceEndsWithPlus();
    void testSaveShortcutsAndRefresh();
};

#endif
