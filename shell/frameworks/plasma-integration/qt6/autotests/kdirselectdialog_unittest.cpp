/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "../src/platformtheme/kdirselectdialog_p.h"
#include "../src/platformtheme/kfiletreeview_p.h"
#include <QTest>

class KDirSelectDialog_UnitTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSetCurrentUrl_data()
    {
        QTest::addColumn<QUrl>("url");
        QTest::addColumn<QUrl>("expectedUrl");

        QTest::newRow("only_scheme") << QUrl(QStringLiteral("trash:")) << QUrl(QStringLiteral("trash:/"));
        QTest::newRow("with_no_host") << QUrl(QStringLiteral("trash://")) << QUrl(QStringLiteral("trash://"));
        QTest::newRow("with_root_path") << QUrl(QStringLiteral("trash:///")) << QUrl(QStringLiteral("trash:///"));
    }

    void testSetCurrentUrl()
    {
        QFETCH(QUrl, url);
        QFETCH(QUrl, expectedUrl);

        KDirSelectDialog dirDialog;
        dirDialog.setCurrentUrl(url);

        QCOMPARE(dirDialog.rootUrl(), expectedUrl);
    }
};

QTEST_MAIN(KDirSelectDialog_UnitTest)

#include "kdirselectdialog_unittest.moc"
