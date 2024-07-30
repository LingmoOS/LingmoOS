// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QTest>

#include <kcountryflagemojiiconengine.h>

using namespace Qt::Literals::StringLiterals;

class KCountryFlagEmojiIconEngineTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIsNull()
    {
        QVERIFY(KCountryFlagEmojiIconEngine(QString()).isNull());
    }

    void testAustria()
    {
        QIcon icon(new KCountryFlagEmojiIconEngine("AT"_L1));
        auto pixmap = icon.pixmap(16, 16);
        QVERIFY(!pixmap.isNull());
    }

    void testScotland()
    {
        QIcon icon(new KCountryFlagEmojiIconEngine("GB-SCT"_L1));
        auto pixmap = icon.pixmap(16, 16);
        QVERIFY(!pixmap.isNull());
    }
};

QTEST_MAIN(KCountryFlagEmojiIconEngineTest)

#include "kcountryflagemojiiconenginetest.moc"
