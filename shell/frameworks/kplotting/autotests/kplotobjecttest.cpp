/*
    SPDX-FileCopyrightText: 2012 Benjamin Port <benjamin.port@ben2367.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kplotobject.h>
#include <kplotpoint.h>

#include <QBrush>
#include <QPen>
#include <QTest>

static const QColor DEFAULT_COLOR = Qt::blue;
static const QColor MODIFIED_COLOR = Qt::red;

static const double DEFAULT_WIDTH = 2;

static const QBrush DEFAULT_BRUSH = QBrush(DEFAULT_COLOR);
static const QBrush MODIFIED_BRUSH = QBrush(MODIFIED_COLOR);

static const QPen DEFAULT_PEN = QPen(DEFAULT_COLOR, 1);
static const QPen MODIFIED_PEN = QPen(MODIFIED_COLOR, 2);

static const KPlotObject::PlotType DEFAULT_PLOT_TYPE = KPlotObject::Bars;

static const KPlotObject::PointStyle DEFAULT_POINT_STYLE = KPlotObject::Asterisk;

class KPlotObjectTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_kPlotObject = new KPlotObject(DEFAULT_COLOR, DEFAULT_PLOT_TYPE, DEFAULT_WIDTH, DEFAULT_POINT_STYLE);
    }

    void cleanupTestCase()
    {
        delete m_kPlotObject;
    }

    void testBrush()
    {
        // verify default brush
        QCOMPARE(m_kPlotObject->brush(), DEFAULT_BRUSH);

        // change brush
        m_kPlotObject->setBrush(MODIFIED_BRUSH);
        QCOMPARE(m_kPlotObject->brush(), MODIFIED_BRUSH);
    }

    void testBarBrush()
    {
        // verify default brush
        QCOMPARE(m_kPlotObject->barBrush(), DEFAULT_BRUSH);

        // change brush
        m_kPlotObject->setBarBrush(MODIFIED_BRUSH);
        QCOMPARE(m_kPlotObject->barBrush(), MODIFIED_BRUSH);
    }

    void testPen()
    {
        // test default pen
        QCOMPARE(m_kPlotObject->pen(), DEFAULT_PEN);

        // change pen
        m_kPlotObject->setPen(MODIFIED_PEN);
        QCOMPARE(m_kPlotObject->pen(), MODIFIED_PEN);
    }

    void testLinePen()
    {
        // test default pen
        QCOMPARE(m_kPlotObject->linePen(), DEFAULT_PEN);

        // change pen
        m_kPlotObject->setLinePen(MODIFIED_PEN);
        QCOMPARE(m_kPlotObject->linePen(), MODIFIED_PEN);
    }

    void testBarPen()
    {
        // test default pen
        QCOMPARE(m_kPlotObject->barPen(), DEFAULT_PEN);

        // change pen
        m_kPlotObject->setBarPen(MODIFIED_PEN);
        QCOMPARE(m_kPlotObject->barPen(), MODIFIED_PEN);
    }

    void testLabelPen()
    {
        // test default pen
        QCOMPARE(m_kPlotObject->labelPen(), DEFAULT_PEN);

        // change pen
        m_kPlotObject->setLabelPen(MODIFIED_PEN);
        QCOMPARE(m_kPlotObject->labelPen(), MODIFIED_PEN);
    }

    void testSize()
    {
        // verify default size
        QCOMPARE(m_kPlotObject->size(), DEFAULT_WIDTH);

        // modify size and test the new one
        m_kPlotObject->setSize(3.0);
        QCOMPARE(m_kPlotObject->size(), 3.0);
    }

    void testDefaultPlotType()
    {
        QCOMPARE(m_kPlotObject->plotTypes(), DEFAULT_PLOT_TYPE);
    }

    void testShowBars()
    {
        m_kPlotObject->setShowBars(false);
        QVERIFY(!(m_kPlotObject->plotTypes() & KPlotObject::Bars));

        m_kPlotObject->setShowBars(true);
        QVERIFY(m_kPlotObject->plotTypes() & KPlotObject::Bars);
    }

    void testShowLines()
    {
        m_kPlotObject->setShowLines(false);
        QVERIFY(!(m_kPlotObject->plotTypes() & KPlotObject::Lines));

        m_kPlotObject->setShowLines(true);
        QVERIFY(m_kPlotObject->plotTypes() & KPlotObject::Lines);
    }

    void testShowPoints()
    {
        m_kPlotObject->setShowPoints(false);
        QVERIFY(!(m_kPlotObject->plotTypes() & KPlotObject::Points));

        m_kPlotObject->setShowPoints(true);
        QVERIFY(m_kPlotObject->plotTypes() & KPlotObject::Points);
    }

    void testPointStyle()
    {
        QCOMPARE(m_kPlotObject->pointStyle(), DEFAULT_POINT_STYLE);

        m_kPlotObject->setPointStyle(KPlotObject::Star);
        QCOMPARE(m_kPlotObject->pointStyle(), KPlotObject::Star);
    }

    void testAddPoint()
    {
        // verify list is empty
        QCOMPARE(m_kPlotObject->points().size(), 0);

        // test: void KPlotObject::addPoint( const QPointF &p, const QString &label, double barWidth )
        m_kPlotObject->addPoint(QPointF(1, 0), QStringLiteral("label1"), 1.0);
        QCOMPARE(m_kPlotObject->points().size(), 1);
        KPlotPoint *p1 = m_kPlotObject->points().at(0);
        QCOMPARE(p1->position(), QPointF(1, 0));
        QCOMPARE(p1->label(), QString::fromLatin1("label1"));
        QCOMPARE(p1->barWidth(), 1.0);

        // test void KPlotObject::addPoint( KPlotPoint *p )
        KPlotPoint *p2 = new KPlotPoint(2, 2, QStringLiteral("label2"), 2.0);
        m_kPlotObject->addPoint(p2);
        QCOMPARE(m_kPlotObject->points().size(), 2);
        KPlotPoint *p2List = m_kPlotObject->points().at(1);
        QCOMPARE(p2, p2List);
        delete p2;

        // test void KPlotObject::addPoint( double x, double y, const QString &label, double barWidth )
        m_kPlotObject->addPoint(3, 3, QStringLiteral("label3"), 3.0);
        QCOMPARE(m_kPlotObject->points().size(), 3);
        KPlotPoint *p3 = m_kPlotObject->points().at(2);
        QCOMPARE(p3->position(), QPointF(3, 3));
        QCOMPARE(p3->label(), QString::fromLatin1("label3"));
        QCOMPARE(p3->barWidth(), 3.0);
    }

    void testRemovePoint()
    {
        // currently 3 points
        QCOMPARE(m_kPlotObject->points().size(), 3);

        KPlotPoint *p1 = m_kPlotObject->points().at(0);
        KPlotPoint *p2 = m_kPlotObject->points().at(1);
        KPlotPoint *p3 = m_kPlotObject->points().at(2);

        QCOMPARE(m_kPlotObject->points().at(0), p1);
        QCOMPARE(m_kPlotObject->points().at(1), p2);
        QCOMPARE(m_kPlotObject->points().at(2), p3);

        m_kPlotObject->removePoint(1);
        QCOMPARE(m_kPlotObject->points().size(), 2);
        QCOMPARE(m_kPlotObject->points().at(0), p1);
        QCOMPARE(m_kPlotObject->points().at(1), p3);
    }

    void testClearPoints()
    {
        // currently 2 points
        QCOMPARE(m_kPlotObject->points().size(), 2);
        m_kPlotObject->clearPoints();
        QCOMPARE(m_kPlotObject->points().size(), 0);
    }

private:
    KPlotObject *m_kPlotObject;
};

QTEST_MAIN(KPlotObjectTest)

#include "kplotobjecttest.moc"
