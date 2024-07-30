/*
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parttest.h"
#include <qtest_widgets.h>

#include <KSharedConfig>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>
#include <kparts/guiactivateevent.h>
#include <kparts/openurlarguments.h>
#include <kparts/readonlypart.h>

QTEST_MAIN(PartTest)

class TestPart : public KParts::ReadOnlyPart
{
public:
    TestPart(QObject *parent, QWidget *parentWidget)
        : KParts::ReadOnlyPart(parent)
        , m_openFileCalled(false)
    {
        setWidget(new QWidget(parentWidget));
        connect(this, &KParts::ReadOnlyPart::urlChanged, this, &TestPart::logUrlChanged);
    }

    bool openFileCalled() const
    {
        return m_openFileCalled;
    }

    void logUrlChanged(const QUrl &url)
    {
        qDebug() << "url changed: " << url;
    }

    bool m_guiActivationEventTriggered = false;

protected:
    bool openFile() override
    {
        m_openFileCalled = true;
        return true;
    }
    void guiActivateEvent(KParts::GUIActivateEvent * /*event*/) override
    {
        m_guiActivationEventTriggered = true;
    }

private:
    bool m_openFileCalled;
};

void PartTest::testAutoDeletePart()
{
    KParts::Part *part = new TestPart(nullptr, nullptr);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(partPointer.isNull());
}

void PartTest::testAutoDeleteWidget()
{
    KParts::Part *part = new TestPart(nullptr, nullptr);
    QPointer<KParts::Part> partPointer(part);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(widgetPointer.isNull());
}

void PartTest::testNoAutoDeletePart()
{
    KParts::Part *part = new TestPart(nullptr, nullptr);
    part->setAutoDeletePart(false);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QVERIFY(part->widget() == nullptr);
    QCOMPARE(static_cast<KParts::Part *>(partPointer), part);
    delete part;
}

void PartTest::testNoAutoDeleteWidget()
{
    KParts::Part *part = new TestPart(nullptr, nullptr);
    part->setAutoDeleteWidget(false);
    QWidget *widget = part->widget();
    QVERIFY(widget);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QCOMPARE(static_cast<QWidget *>(widgetPointer), widget);
    delete widget;
}

// There is no operator== in OpenUrlArguments because it's only useful in unit tests
static bool compareArgs(const KParts::OpenUrlArguments &arg1, const KParts::OpenUrlArguments &arg2)
{
    return arg1.mimeType() == arg2.mimeType() && //
        arg1.xOffset() == arg2.xOffset() && //
        arg1.yOffset() == arg2.yOffset() && //
        arg1.reload() == arg2.reload();
}

void PartTest::testOpenUrlArguments()
{
    TestPart *part = new TestPart(nullptr, nullptr);
    QVERIFY(part->closeUrl()); // nothing to do, no error
    QVERIFY(part->arguments().mimeType().isEmpty());
    KParts::OpenUrlArguments args;
    args.setMimeType(QStringLiteral("application/xml"));
    args.setXOffset(50);
    args.setYOffset(10);
    args.setReload(true);
    part->setArguments(args);
    QVERIFY(compareArgs(args, part->arguments()));
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("parttest.cpp")));
    QVERIFY(part->openFileCalled());
    QVERIFY(compareArgs(args, part->arguments()));

    // Explicit call to closeUrl: arguments are cleared
    part->closeUrl();
    QVERIFY(part->arguments().mimeType().isEmpty());

    // Calling openUrl with local file: mimetype is determined
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("parttest.cpp")));
    QCOMPARE(part->arguments().mimeType(), QStringLiteral("text/x-c++src"));
    // (for a remote url it would be determined during downloading)

    delete part;
}

void PartTest::testAutomaticMimeType()
{
    TestPart *part = new TestPart(nullptr, nullptr);
    QVERIFY(part->closeUrl()); // nothing to do, no error
    QVERIFY(part->arguments().mimeType().isEmpty());
    // open a file, and test the detected mimetype
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("notepad.desktop")));
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    // manually closing, no mimetype should be stored now
    part->closeUrl();
    QVERIFY(part->arguments().mimeType().isEmpty());

    // open a new file, and test again its (autdetected) mimetype
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("parttest.cpp")));
    QCOMPARE(part->arguments().mimeType(), QStringLiteral("text/x-c++src"));

    // open a new file, but without explicitly close the first
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("notepad.desktop")));
    // test again its (autdetected) mimetype
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    // open a new file, but manually forcing a mimetype
    KParts::OpenUrlArguments args;
    args.setMimeType(QStringLiteral("application/xml"));
    part->setArguments(args);
    QVERIFY(compareArgs(args, part->arguments()));
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("parttest.cpp")));
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/xml"));

    // clear the args and open a new file, reactivating the automatic mimetype detection again
    part->setArguments(KParts::OpenUrlArguments());
    part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("notepad.desktop")));
    // test again its (autdetected) mimetype
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    delete part;
}

void PartTest::testEmptyUrlAfterCloseUrl()
{
    TestPart *part = new TestPart(nullptr, nullptr);

    QVERIFY(part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("notepad.desktop"))));
    QSignalSpy spy(part, &KParts::ReadOnlyPart::urlChanged);
    QVERIFY(part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("parttest.cpp"))));
    QVERIFY(!part->url().isEmpty());
    QCOMPARE(spy.count(), 1);
    spy.clear();
    QVERIFY(part->closeUrl());
    QVERIFY(part->url().isEmpty());
    QCOMPARE(spy.count(), 1);

    delete part;
}

#include <KConfigGroup>
#include <KToggleToolBarAction>
#include <KToolBar>
#include <kparts/mainwindow.h>
class MyMainWindow : public KParts::MainWindow
{
public:
    MyMainWindow()
        : KParts::MainWindow()
    {
        tb = new KToolBar(this);
        tb->setObjectName(QStringLiteral("testtbvisibility"));
    }

    // createGUI and saveAutoSaveSettings are protected, so the whole test is here:
    void testToolbarVisibility()
    {
        QVERIFY(tb->isVisible());

        TestPart *part = new TestPart(nullptr, nullptr);
        // TODO define xml with a toolbar for the part
        // and put some saved settings into qttestrc in order to test
        // r347935+r348051, i.e. the fact that KParts::MainWindow::createGUI
        // will apply the toolbar settings (and that they won't have been
        // erased by the previous call to saveMainWindowSettings...)
        this->createGUI(part);

        QVERIFY(tb->isVisible());
        this->saveAutoSaveSettings();

        // Hide the toolbar using the action (so that setSettingsDirty is called, too)
        KToggleToolBarAction action(tb, QString(), nullptr);
        action.trigger();
        QVERIFY(!tb->isVisible());

        // Switch the active part, and check that
        // the toolbar doesn't magically reappear,
        // as it did when createGUI was calling applyMainWindowSettings
        this->createGUI(nullptr);
        QVERIFY(!tb->isVisible());
        this->createGUI(part);
        QVERIFY(!tb->isVisible());

        // All ok, show it again so that test can be run again :)
        action.trigger();
        QVERIFY(tb->isVisible());
        close();
    }

private:
    KToolBar *tb;
};

// A KParts::MainWindow unit test
void PartTest::testToolbarVisibility()
{
    // The bug was: hide a toolbar in konqueror,
    // then switch tabs -> the toolbar shows again
    // (unless you waited for the autosave timer to kick in)
    MyMainWindow window;
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("kxmlgui_unittest"));
    window.setAutoSaveSettings(cg.name());
    window.show();
    window.testToolbarVisibility();
}

void PartTest::testShouldNotCrashAfterDelete()
{
    TestPart *part = new TestPart(nullptr, nullptr);
    QVERIFY(part->openUrl(QUrl::fromLocalFile(QFINDTESTDATA("notepad.desktop"))));
    QVERIFY(part->openFileCalled());
    delete part;
}

void PartTest::testActivationEvent()
{
    TestPart *part = new TestPart(nullptr, nullptr);
    QVERIFY(!part->m_guiActivationEventTriggered);
    part->event(new QEvent(QEvent::MouseButtonPress));
    QVERIFY(!part->m_guiActivationEventTriggered);
    part->event(new KParts::GUIActivateEvent(true));
    QVERIFY(part->m_guiActivationEventTriggered);
    delete part;
}

#include "moc_parttest.cpp"
