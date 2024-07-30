/*
    SPDX-FileCopyrightText: 2003, 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QFontMetrics>

#include <QTest>

#include "kwordwrap.h"
#include <QObject>

class KWordWrap_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // Qt5 TODO: how to set the dpi?
        // Only found readonly QScreen::logicalDotsPerInch...
#if 0
        QX11Info::setAppDpiX(0, 96);
        QX11Info::setAppDpiY(0, 96);
#endif
    }

    void oldTruncationTest()
    {
        QFont font(QStringLiteral("helvetica"), 12); // let's hope we all have the same...
        QFontMetrics fm(font);
        QRect r(0, 0, 100, -1);
        QString str = QStringLiteral("test wadabada [/foo/bar/waba] and some more text here");
        KWordWrap ww = KWordWrap::formatText(fm, r, 0, str);
        // qDebug() << str << " => " << ww.truncatedString();
        QVERIFY(ww.truncatedString().endsWith("..."));

        str = QStringLiteral("</p></p></p></p>");
        for (; r.width() > 0; r.setWidth(r.width() - 10)) {
            ww = KWordWrap::formatText(fm, r, 0, str);
            // qDebug() << str << " => " << ww.truncatedString();
            QVERIFY(ww.truncatedString().endsWith("..."));
        }
    }

    void testWithExistingNewlines() // when the input string has \n already
    {
        QRect r(0, 0, 1000, -1); // very wide
        QFont font(QStringLiteral("helvetica"), 12); // let's hope we all have the same...
        QFontMetrics fm(font);
        QString inputString = QStringLiteral("The title here\nFoo (bar)\nFoo2 (bar2)");
        KWordWrap ww = KWordWrap::formatText(fm, r, 0, inputString);
        QString str = ww.wrappedString();
        QCOMPARE(str, inputString);
    }
};

QTEST_MAIN(KWordWrap_UnitTest)

#include "kwordwraptest.moc"
