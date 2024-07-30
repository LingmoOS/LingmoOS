/*
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kplotpoint.h>

#include <QTest>

class KPlotPointTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testConstructor()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QStringLiteral("label"), 5.0);
        KPlotPoint *p2 = new KPlotPoint(QPointF(2.0, 3.0), QStringLiteral("label"), 5.0);

        QCOMPARE(p1->x(), 2.0);
        QCOMPARE(p2->x(), 2.0);

        QCOMPARE(p1->y(), 3.0);
        QCOMPARE(p2->y(), 3.0);

        QCOMPARE(p1->barWidth(), 5.0);
        QCOMPARE(p2->barWidth(), 5.0);

        QCOMPARE(p1->label(), QString::fromLatin1("label"));
        QCOMPARE(p2->label(), QString::fromLatin1("label"));

        delete p1;
        delete p2;
    }

    void testPosition()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QStringLiteral("label"), 5.0);

        p1->setX(4.0);
        QCOMPARE(p1->x(), 4.0);

        p1->setY(6.0);
        QCOMPARE(p1->y(), 6.0);

        QCOMPARE(p1->position(), QPointF(4.0, 6.0));

        p1->setPosition(QPointF(1.0, 7.0));
        QCOMPARE(p1->position(), QPointF(1.0, 7.0));
        QCOMPARE(p1->x(), 1.0);
        QCOMPARE(p1->y(), 7.0);

        delete p1;
    }

    void testLabel()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QStringLiteral("label"), 5.0);

        p1->setLabel(QStringLiteral("newLabel"));
        QCOMPARE(p1->label(), QString::fromLatin1("newLabel"));

        delete p1;
    }

    void testBarWidth()
    {
        KPlotPoint *p1 = new KPlotPoint(2.0, 3.0, QStringLiteral("label"), 5.0);

        p1->setBarWidth(5.0);
        QCOMPARE(p1->barWidth(), 5.0);

        delete p1;
    }
};

QTEST_MAIN(KPlotPointTest)

#include "kplotpointtest.moc"
