// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dfiledragcommon_p.h"
#include "dfiledrag.h"
#include "dfiledragserver.h"
#include "dfiledragclient.h"

#include <QMimeData>
#include <QtDBus>
#include <QSignalSpy>
#include <QTest>

#include <QScopedPointer>

DGUI_USE_NAMESPACE

TEST(ut_DFileDrag, filedrag)
{
    qRegisterMetaType<DFileDragState>("DFileDragState");
    QObject source;
    QScopedPointer<DFileDragServer> s(new DFileDragServer());
    QMimeData *m = new QMimeData();
    // QMimeData will delete on ~DFileDrag~QDrag
    QScopedPointer<DFileDrag> drag(new DFileDrag(&source, s.data()));
    drag->setMimeData(m);

    ASSERT_TRUE(DFileDragClient::checkMimeData(m));
    {
        QSignalSpy spy(s.data(), &DFileDragServer::targetDataChanged);
        DFileDragClient::setTargetData(m, "Key0", "Value0");
        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
        ASSERT_EQ(s->targetData("Key0"), QVariant("Value0"));
    }
    {
        QUrl url = QUrl::fromLocalFile("/tmp/xxx");
        QSignalSpy spy(drag.data(), &DFileDrag::targetUrlChanged);
        DFileDragClient::setTargetUrl(m, url);
        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
        ASSERT_EQ(drag->targetUrl(), url);
    }

    // client will delete on serverDestroyed
    DFileDragClient *c = new DFileDragClient(m);
    {
        ASSERT_EQ(c->progress(), 0);
        QSignalSpy spy(c, &DFileDragClient::progressChanged);
        s->setProgress(30);
        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
        ASSERT_EQ(c->progress(), 30);
    }
    {
        ASSERT_EQ(c->state(), 0);
        QSignalSpy spy(c, &DFileDragClient::stateChanged);
        s->setState(DFileDragState::Running);
        ASSERT_TRUE(QTest::qWaitFor([&spy](){
            return spy.count() > 0;
        }, 1000));
        ASSERT_EQ(c->state(), DFileDragState::Running);
    }
    {
        QSignalSpy spy(c, &DFileDragClient::serverDestroyed);
        // destroy server to emit serverDestroyed
        s.reset();
        waitforSpy(spy, 1000);
        ASSERT_TRUE(spy.count() > 0);
    }
}
