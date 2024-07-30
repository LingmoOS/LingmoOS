/*
    SPDX-FileCopyrightText: 2014 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include "kx11extras.h"
#include "nettesthelper.h"
#include "netwm.h"

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
    void run() override
    {
        (void)KWindowSystem::self();
    }
};

class WindowInfoLister : public QThread
{
public:
    void run() override
    {
        // simulate some activity in another thread gathering window information
        const QList<WId> windows = KX11Extras::stackingOrder();
        for (auto wid : windows) {
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
    QSignalSpy spy(KX11Extras::self(), &KX11Extras::windowAdded);
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
    QVERIFY(KX11Extras::hasWId(m_widget->winId()));
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
