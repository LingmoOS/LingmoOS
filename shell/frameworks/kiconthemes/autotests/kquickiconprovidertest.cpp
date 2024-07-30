/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KQuickIconProvider>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QTest>

class KQuickIconProviderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRegisteringEngine()
    {
        QQmlApplicationEngine engine;
        engine.addImageProvider(QStringLiteral("icon"), new KQuickIconProvider);
    }
};

QTEST_MAIN(KQuickIconProviderTest)

#include "kquickiconprovidertest.moc"
