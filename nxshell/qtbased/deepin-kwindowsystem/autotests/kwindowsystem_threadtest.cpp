/*
 *   Copyright 2014 Aaron Seigo <aseigo@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include "netwm.h"
#include "nettesthelper.h"

#include <QRunnable>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QThreadPool>

class KWindowSystemThreadTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testWindowAdded();
    void testAccessFromThread();

private:
    QWidget *m_widget;
};

class KWindowSystemCreator : public QRunnable
{
public:
    void run() Q_DECL_OVERRIDE
    {
        (void)KWindowSystem::self();
    }
};

class WindowInfoLister : public QThread
{
public:
    void run() Q_DECL_OVERRIDE
    {
        // simulate some activity in another thread gathering window information
        const QList<WId> windows = KWindowSystem::stackingOrder();
        foreach (auto wid, windows) {
            KWindowInfo info(wid, NET::WMVisibleName);
            if (info.valid()) {
                m_names << info.visibleName();
            }
        }
    }

    QStringList m_names;
};

void KWindowSystemThreadTest::initTestCase()
{
    m_widget = nullptr;
    QRunnable *creator = new KWindowSystemCreator;
    creator->setAutoDelete(true);
    QThreadPool::globalInstance()->start(creator);
    QVERIFY(QThreadPool::globalInstance()->waitForDone(5000));
}

void KWindowSystemThreadTest::testWindowAdded()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowAdded(WId)));
    m_widget = new QWidget;
    m_widget->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_widget));
    QVERIFY(spy.count() > 0);
    bool hasWId = false;
    for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        QCOMPARE((*it).count(), 1);
        hasWId = (*it).at(0).toULongLong() == m_widget->winId();
        if (hasWId) {
            break;
        }
    }
    QVERIFY(hasWId);
    QVERIFY(KWindowSystem::hasWId(m_widget->winId()));
}

void KWindowSystemThreadTest::testAccessFromThread()
{
    WindowInfoLister listerThread;
    listerThread.start();
    QVERIFY(listerThread.wait(5000));
    QVERIFY(!listerThread.m_names.isEmpty());
}

QTEST_MAIN(KWindowSystemThreadTest)

#include <kwindowsystem_threadtest.moc>
