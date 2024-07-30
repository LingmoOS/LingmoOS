/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kicondialog.h>

#include <QStandardPaths>
#include <QTest>

#include <KConfigGroup>
#include <KSharedConfig>

class KIconDialogTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // set our test theme only present in :/icons
        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
        cg.writeEntry("Theme", "themeinresource");
        cg.sync();
    }

    void testConstructor()
    {
        // At least we test that we don't hit any assert
        KIconDialog iconDialog;
    }
};

QTEST_MAIN(KIconDialogTest)

#include "kicondialog_unittest.moc"
