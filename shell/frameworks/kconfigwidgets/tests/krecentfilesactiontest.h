/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Gregor Mi <codeminister@publicstatic.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KCONFIGWIDGETS_TESTS_KRECENTFILESACTIONTEST_H
#define KCONFIGWIDGETS_TESTS_KRECENTFILESACTIONTEST_H

#include <QMainWindow>

class KRecentFilesActionTestPrivate;

class KRecentFilesActionTest : public QMainWindow
{
    Q_OBJECT

public:
    KRecentFilesActionTest();

    ~KRecentFilesActionTest() override;

public Q_SLOTS:
    void addUrl();
    void loadEntries();
    void saveEntries();
    void urlSelected(const QUrl &url);

private:
    KRecentFilesActionTestPrivate *const d;
};

#endif
