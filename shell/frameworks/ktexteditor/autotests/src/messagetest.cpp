/*
    This file is part of the Kate project.
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "messagetest.h"

#include <katedocument.h>
#include <katemessagewidget.h>
#include <kateview.h>
#include <ktexteditor/message.h>

#include <QStandardPaths>
#include <QtTestWidgets>

using namespace KTextEditor;

QTEST_MAIN(MessageTest)

void MessageTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void MessageTest::cleanupTestCase()
{
}

void MessageTest::testPostMessage()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    QPointer<Message> message = new Message(QStringLiteral("Message text"), Message::Information);
    message->setPosition(Message::TopInView);

    // posing message should succeed
    QVERIFY(doc.postMessage(message));

    //
    // show message for one second, then delete again
    //
    QTest::qWait(500);
    QVERIFY(view->messageWidget());
    QVERIFY(view->messageWidget()->isVisible());

    QVERIFY(message != nullptr);
    delete message;
    QTest::qWait(1000); // fadeout animation takes 500 ms
    QVERIFY(!view->messageWidget()->isVisible());
}

void MessageTest::testAutoHide()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    //
    // show a message with autoHide. Check, if it's deleted correctly
    // auto hide mode: Message::Immediate
    //
    QPointer<Message> message = new Message(QStringLiteral("Message text"), Message::Information);
    message->setPosition(Message::TopInView);
    message->setAutoHide(1000);
    message->setAutoHideMode(Message::Immediate);

    doc.postMessage(message);

    QTest::qWait(500);
    QVERIFY(view->messageWidget()->isVisible());

    // should be deleted after 1.5 seconds
    QTest::qWait(1000);
    QVERIFY(message.data() == nullptr);

    // message widget should be hidden after 2 seconds
    QTest::qWait(500);
    QVERIFY(!view->messageWidget()->isVisible());
}

void MessageTest::testAutoHideAfterUserInteraction()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    //
    // show a message with autoHide. Check, if it's deleted correctly
    // auto hide mode: Message::AfterUserInteraction
    //
    QPointer<Message> message = new Message(QStringLiteral("Message text"), Message::Information);
    message->setPosition(Message::TopInView);
    message->setAutoHide(2000);
    QVERIFY(message->autoHideMode() == Message::AfterUserInteraction);

    doc.postMessage(message);

    QTest::qWait(1000);
    QVERIFY(view->messageWidget()->isVisible());

    // now trigger user interaction after 1 second
    view->insertText(QStringLiteral("Hello world"));
    view->setCursorPosition(Cursor(0, 5));

    // should still be there after deleted after another 1.9 seconds
    QTest::qWait(1900);
    QVERIFY(message.data() != nullptr);
    QVERIFY(view->messageWidget()->isVisible());

    // another 300ms later: 3.2 seconds are gone, message should be deleted
    // and fade animation should be active
    QTest::qWait(300);
    QVERIFY(message.data() == nullptr);
    QVERIFY(view->messageWidget()->isVisible());

    // after a total of 3.7 seconds, widget should be hidden
    QTest::qWait(500);
    QVERIFY(!view->messageWidget()->isVisible());
}

void MessageTest::testMessageQueue()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    //
    // add two messages, both with autoHide to 1 second, and check that the queue is processed correctly
    // auto hide mode: Message::Immediate
    //
    QPointer<Message> m1 = new Message(QStringLiteral("Info text"), Message::Information);
    m1->setPosition(Message::TopInView);
    m1->setAutoHide(1000);
    m1->setAutoHideMode(Message::Immediate);

    QPointer<Message> m2 = new Message(QStringLiteral("Error text"), Message::Error);
    m2->setPosition(Message::TopInView);
    m2->setAutoHide(1000);
    m2->setAutoHideMode(Message::Immediate);

    // post both messages
    QVERIFY(doc.postMessage(m1));
    QVERIFY(doc.postMessage(m2));

    // after 0.5s, first message should be visible, (timer of m1 triggered)
    QTest::qWait(500);
    QVERIFY(view->messageWidget()->isVisible());
    QVERIFY(m1.data() != nullptr);
    QVERIFY(m2.data() != nullptr);

    // after 1.2s, first message is deleted, and hide animation is active
    QTest::qWait(700);
    QVERIFY(view->messageWidget()->isVisible());
    QVERIFY(m1.data() == nullptr);
    QVERIFY(m2.data() != nullptr);

    // timer of m2 triggered after 1.5s, i.e. after hide animation if finished
    QTest::qWait(500);

    // after 2.1s, second message should be visible
    QTest::qWait(500);
    QVERIFY(view->messageWidget()->isVisible());
    QVERIFY(m2.data() != nullptr);

    // after 2.6s, second message is deleted, and hide animation is active
    QTest::qWait(500);
    QVERIFY(view->messageWidget()->isVisible());
    QVERIFY(m2.data() == nullptr);

    // after a total of 3.1s, animation is finished and widget is hidden
    QTest::qWait(500);
    QVERIFY(!view->messageWidget()->isVisible());
}

void MessageTest::testPriority()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    //
    // add two messages
    // - m1: no auto hide timer, priority 0
    // - m2: auto hide timer of 1 second, priority 1
    // test:
    // - m1 should be hidden in favour of m2
    // - changing text of m1 while m2 is displayed should not change the displayed text
    //
    QPointer<Message> m1 = new Message(QStringLiteral("m1"), Message::Positive);
    m1->setPosition(Message::TopInView);
    QVERIFY(m1->priority() == 0);

    QPointer<Message> m2 = new Message(QStringLiteral("m2"), Message::Error);
    m2->setPosition(Message::TopInView);
    m2->setAutoHide(1000);
    m2->setAutoHideMode(Message::Immediate);
    m2->setPriority(1);
    QVERIFY(m2->priority() == 1);

    // post m1
    QVERIFY(doc.postMessage(m1));

    // after 1s, message should be displayed
    QTest::qWait(1000);
    QVERIFY(view->messageWidget()->isVisible());
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("m1"));
    QVERIFY(m1.data() != nullptr);

    // post m2, m1 should be hidden, and m2 visible
    QVERIFY(doc.postMessage(m2));
    QVERIFY(m2.data() != nullptr);

    // alter text of m1 when m2 is visible, shouldn't influence m2
    QTest::qWait(600);
    m1->setText(QStringLiteral("m1 changed"));

    // after 0.7 seconds, m2 is visible
    QTest::qWait(100);
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("m2"));
    QVERIFY(m2.data() != nullptr);

    // after 1.6 seconds, m2 is hidden again and m1 is visible again
    QTest::qWait(900);
    QVERIFY(view->messageWidget()->isVisible());
    QVERIFY(m1.data() != nullptr);
    QVERIFY(m2.data() == nullptr);

    // finally check m1 again
    QTest::qWait(1000);
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("m1 changed"));
}

void MessageTest::testCreateView()
{
    KTextEditor::DocumentPrivate doc;

    //
    // - first post a message
    // - then create two views
    //
    // test:
    // - verify that both views get the message
    // - verify that, once the message is deleted, both views hide the message
    //
    QPointer<Message> m1 = new Message(QStringLiteral("message"), Message::Positive);
    m1->setPosition(Message::TopInView);
    QVERIFY(m1->priority() == 0);

    // first post message to doc without views
    QVERIFY(doc.postMessage(m1));

    // now create views
    KTextEditor::ViewPrivate *v1 = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    KTextEditor::ViewPrivate *v2 = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    v1->show();
    v2->show();
    v1->resize(400, 300);
    v2->resize(400, 300);

    // make sure both views show the message
    QVERIFY(v1->messageWidget()->isVisible());
    QVERIFY(v2->messageWidget()->isVisible());
    QCOMPARE(v1->messageWidget()->text(), QStringLiteral("message"));
    QCOMPARE(v2->messageWidget()->text(), QStringLiteral("message"));
    QVERIFY(m1.data() != nullptr);

    // delete message, then check after fadeout time 0f 0.5s whether message is gone
    delete m1;
    QTest::qWait(600);
    QVERIFY(!v1->messageWidget()->isVisible());
    QVERIFY(!v2->messageWidget()->isVisible());
}

void MessageTest::testHideView()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    // create message that hides after 2s immediately
    QPointer<Message> message = new Message(QStringLiteral("Message text"), Message::Information);
    message->setAutoHide(2000);
    message->setAutoHideMode(Message::Immediate);
    message->setPosition(Message::TopInView);

    // posting message should succeed
    QVERIFY(doc.postMessage(message));

    //
    // test:
    // - show the message for 1.5s, then hide the view
    // - the auto hide timer will continue, no matter what
    // - showing the view again after the auto hide timer is finished + animation time really hide the widget
    //
    QTest::qWait(1100);
    QVERIFY(view->messageWidget()->isVisible());
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("Message text"));

    // hide view
    view->hide();

    // wait 1s, message should be null (after total of 2200 ms)
    QTest::qWait(1100);
    QVERIFY(message.data() == nullptr);

    // show view again, message contents should be fading for the lasting 300 ms
    view->show();
    QVERIFY(view->messageWidget()->isVisible());
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("Message text"));

    // wait another 0.5s, then message widget should be hidden
    QTest::qWait(500);
    QVERIFY(message.data() == nullptr);
    QVERIFY(!view->messageWidget()->isVisible());
}

void MessageTest::testHideViewAfterUserInteraction()
{
    KTextEditor::DocumentPrivate doc;

    KTextEditor::ViewPrivate *view = static_cast<KTextEditor::ViewPrivate *>(doc.createView(nullptr));
    view->show();
    view->resize(400, 300);

    // create message that hides after 2s immediately
    QPointer<Message> message = new Message(QStringLiteral("Message text"), Message::Information);
    message->setAutoHide(2000);
    QVERIFY(message->autoHideMode() == Message::AfterUserInteraction);
    message->setPosition(Message::TopInView);

    // posting message should succeed
    QVERIFY(doc.postMessage(message));

    //
    // test:
    // - show the message for 1.5s, then hide the view
    // - this should stop the autoHide timer
    // - showing the view again should restart the autoHide timer (again 2s)
    //
    QTest::qWait(1500);
    QVERIFY(view->messageWidget()->isVisible());
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("Message text"));

    // hide view
    view->hide();

    // wait 1s, check that message is still valid
    QTest::qWait(1000);
    QVERIFY(message.data() != nullptr);

    //
    // show view again, and trigger user interaction through resize:
    // should retrigger the autoHide timer
    //
    view->show();
    QTest::qWait(2000);
    view->insertText(QStringLiteral("Hello world"));
    view->setCursorPosition(Cursor(0, 5));

    // wait 1.5s and check that message is still displayed
    QTest::qWait(1500);
    QVERIFY(message.data() != nullptr);
    QVERIFY(view->messageWidget()->isVisible());
    QCOMPARE(view->messageWidget()->text(), QStringLiteral("Message text"));

    // wait another 0.8s, then the message is deleted
    QTest::qWait(800);
    QVERIFY(message.data() == nullptr);
    QVERIFY(view->messageWidget()->isVisible());

    // another 0.5s, and the message widget should be hidden
    QTest::qWait(600);
    QVERIFY(!view->messageWidget()->isVisible());
}

#include "moc_messagetest.cpp"
