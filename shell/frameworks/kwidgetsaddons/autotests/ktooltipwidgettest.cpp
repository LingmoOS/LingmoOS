/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktooltipwidgettest.h"

#include <KToolTipWidget>

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QScreen>
#include <QSignalSpy>
#include <QStyle>
#include <QTest>

#include "windowscheck.h"

void KToolTipWidgetTest::initTestCase()
{
    m_screenGeometry = QGuiApplication::primaryScreen()->geometry();
}

void KToolTipWidgetTest::showTooltipShouldShowContent()
{
    KToolTipWidget tooltip;
    QLabel label;

    tooltip.showAt(QPoint(10, 10), &label, nullptr);
    QVERIFY(tooltip.isVisible());
    QVERIFY(label.isVisible());
}

void KToolTipWidgetTest::hideLaterShouldHideAfterDelay()
{
    KToolTipWidget tooltip;
    QLabel label;

    tooltip.showAt(QPoint(10, 10), &label, nullptr);
    QVERIFY(tooltip.isVisible());
    QVERIFY(label.isVisible());

    tooltip.hideLater();
    QVERIFY(tooltip.isVisible());
    QVERIFY(label.isVisible());

    QSignalSpy spy(&tooltip, &KToolTipWidget::hidden);
    connect(&tooltip, &KToolTipWidget::hidden, this, [&]() {
        QVERIFY(tooltip.isHidden());
        QVERIFY(!label.isVisible());
    });

    QEventLoop eventLoop;
    connect(&tooltip, &KToolTipWidget::hidden, &eventLoop, &QEventLoop::quit);
    QCOMPARE(eventLoop.exec(), 0);
    QCOMPARE(spy.count(), 1);
}

void KToolTipWidgetTest::setZeroDelayShouldHideImmediately()
{
    KToolTipWidget tooltip;
    QLabel label;

    tooltip.setHideDelay(0);
    tooltip.showAt(QPoint(10, 10), &label, nullptr);
    QVERIFY(tooltip.isVisible());
    QVERIFY(label.isVisible());

    tooltip.hideLater();
    QVERIFY(tooltip.isHidden());
    QVERIFY(!label.isVisible());
}

void KToolTipWidgetTest::shouldHideImmediatelyIfContentDestroyed()
{
    KToolTipWidget tooltip;
    auto lineEdit = new QLineEdit();

    tooltip.showAt(QPoint(10, 10), lineEdit, nullptr);
    QVERIFY(tooltip.isVisible());

    delete lineEdit;
    QVERIFY(tooltip.isHidden());
}

void KToolTipWidgetTest::shouldNotTakeOwnershipOfContent()
{
    auto parent = new QWidget();
    auto label = new QLabel(parent);
    auto tooltip = new KToolTipWidget();

    tooltip->showAt(QPoint(10, 10), label, nullptr);
    QCOMPARE(label->parent(), tooltip);
    QVERIFY(label->isVisible());

    QSignalSpy spy(label, &QWidget::destroyed);
    delete tooltip;
    QVERIFY(!label->isVisible());
    QCOMPARE(label->parent(), parent);
    QCOMPARE(spy.count(), 0);

    delete parent;
    QCOMPARE(spy.count(), 1);
}

void KToolTipWidgetTest::shouldNotObscureTarget_data()
{
    qDebug() << "Style used" << QApplication::style()->objectName();

    QTest::addColumn<QPoint>("position");
    QTest::addColumn<QString>("content");

    const QMap<QString, QPoint> positions{{QStringLiteral("topleft"), QPoint(m_offset, m_offset)},
                                          {QStringLiteral("topright"), QPoint(-m_offset, m_offset)},
                                          {QStringLiteral("bottomright"), QPoint(-m_offset, -m_offset)},
                                          {QStringLiteral("bottomleft"), QPoint(m_offset, -m_offset)},
                                          {QStringLiteral("centered"), QPoint(m_screenGeometry.width() / 2, m_screenGeometry.height() / 2)}};

    QMapIterator<QString, QPoint> i(positions);
    while (i.hasNext()) {
        i.next();
        QTest::newRow(qPrintable(QStringLiteral("small/%1").arg(i.key()))) << i.value() << QStringLiteral("dummy text");

        QTest::newRow(qPrintable(QStringLiteral("multiline/%1").arg(i.key()))) << i.value() << QStringLiteral("dummy text\nLine 1\nLine 2\nLine 3");

        QTest::newRow(qPrintable(QStringLiteral("one long line/%1").arg(i.key())))
            << i.value()
            << QStringLiteral(
                   "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam nec felis sed elit auctor lobortis non a urna. Quisque non posuere mauris. "
                   "Suspendisse potenti. In diam leo, lobortis at placerat nec, sagittis at tortor. Pellentesque scelerisque enim vel elementum scelerisque. "
                   "Integer eget lectus vitae lorem pulvinar hendrerit. Suspendisse auctor sapien vel semper porta. Vestibulum fringilla aliquet tincidunt. "
                   "Maecenas mollis mauris et erat viverra mollis. Proin suscipit felis nisi, a dapibus est hendrerit euismod. Suspendisse quis faucibus quam. "
                   "Fusce eu cursus magna, et egestas purus. Duis enim sapien, feugiat id facilisis non, rhoncus ut lectus. Aliquam at nisi vel ligula "
                   "interdum ultricies. Donec condimentum ante quam, eu congue lectus pulvinar in. Cras interdum, neque quis fermentum consequat, lectus "
                   "tellus eleifend turpis.");

        if (m_screenGeometry.width() >= 1600 && m_screenGeometry.height() >= 900) {
            QTest::newRow(qPrintable(QStringLiteral("super large/%1").arg(i.key())))
                << i.value()
                << QStringLiteral(
                       "dummy 0 text\nLine 1\nLine 2\nLine 3"
                       "dummy 1 text\nLine 1\nLine 2\nLine 3"
                       "dummy 2 text\nLine 1\nLine 2\nLine 3"
                       "dummy 3 text\nLine 1\nLine 2\nLine 3"
                       "dummy 4 text\nLine 1\nLine 2\nLine 3"

                       "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam nec felis sed elit auctor lobortis non a urna. Quisque non posuere "
                       "mauris. Suspendisse potenti. In diam leo, lobortis at placerat nec, sagittis at tortor. Pellentesque scelerisque enim vel elementum "
                       "scelerisque. Integer eget lectus vitae lorem pulvinar hendrerit. Suspendisse auctor sapien vel semper porta. Vestibulum fringilla "
                       "aliquet tincidunt. Maecenas mollis mauris et erat viverra mollis. Proin suscipit felis nisi, a dapibus est hendrerit euismod. "
                       "Suspendisse quis faucibus quam. Fusce eu cursus magna, et egestas purus. Duis enim sapien, feugiat id facilisis non, rhoncus ut "
                       "lectus. Aliquam at nisi vel ligula interdum ultricies. Donec condimentum ante quam, eu congue lectus pulvinar in. Cras interdum, neque "
                       "quis fermentum consequat, lectus tellus eleifend turpis.");
        }
    }
}

void KToolTipWidgetTest::shouldNotObscureTarget()
{
    QFETCH(QPoint, position);
    QFETCH(QString, content);

    if (isWindowsCI()) {
        QSKIP("GUI Tests on Windows CI are not supported");
    }

    QWidget *containerWidget = new QWidget();
    const int margin = containerWidget->style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth);
    QLabel *targetWidget = new QLabel(QStringLiteral("dummy file"));
    QLayout *layout = new QHBoxLayout(containerWidget);
    layout->addWidget(targetWidget);

    containerWidget->adjustSize();
    containerWidget->move(position.x() >= 0 ? position.x() : m_screenGeometry.right() + position.x() - containerWidget->width(),
                          position.y() >= 0 ? position.y() : m_screenGeometry.bottom() + position.y() - containerWidget->height());
    containerWidget->show();
    QVERIFY(QTest::qWaitForWindowExposed(containerWidget));
    QVERIFY(targetWidget->isVisible());

    QRect targetRect = QRect(targetWidget->frameGeometry());
    targetRect.moveTo(targetWidget->mapToGlobal(QPoint(0, 0)));

    QLabel *contentWidget = new QLabel(content);

    QVERIFY2(containerWidget->windowHandle(), "Container's window handle is invalid");
    KToolTipWidget tooltipWidget;
    tooltipWidget.showBelow(targetRect, contentWidget, containerWidget->windowHandle());
    QVERIFY(QTest::qWaitForWindowExposed(&tooltipWidget));
    const QRect tooltipRect = tooltipWidget.frameGeometry();

    qDebug() << QStringLiteral("tooltip: %1x%2 x=%3, y=%4").arg(tooltipRect.width()).arg(tooltipRect.height()).arg(tooltipRect.left()).arg(tooltipRect.top());
    qDebug() << QStringLiteral("target:  %1x%2 x=%3, y=%4").arg(targetRect.width()).arg(targetRect.height()).arg(targetRect.left()).arg(targetRect.top());

    QVERIFY2(!tooltipRect.intersects(targetRect), "Target obscured");
    QCOMPARE(tooltipRect.intersected(m_screenGeometry), tooltipRect);

    // Check margins
    if (tooltipRect.bottom() < targetRect.top()) {
        QCOMPARE(margin, targetRect.top() - tooltipRect.bottom());
    } else if (tooltipRect.top() > targetRect.bottom()) {
        QCOMPARE(margin, tooltipRect.top() - targetRect.bottom());
    } else if (tooltipRect.right() < targetRect.left()) {
        QCOMPARE(margin, targetRect.left() - tooltipRect.right());
    } else if (tooltipRect.left() > targetRect.right()) {
        QCOMPARE(margin, tooltipRect.left() - targetRect.right());
    }

    tooltipWidget.hide();
    QVERIFY(tooltipWidget.isHidden());

    delete contentWidget;
    delete containerWidget;
}

QTEST_MAIN(KToolTipWidgetTest)

#include "moc_ktooltipwidgettest.cpp"
