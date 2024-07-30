/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QAction>
#include <QApplication>
#include <QStandardPaths>
#include <QTest>
#include <QTextEdit>
#include <QTimer>

#include <KConfigGroup>
#include <KMessageBox>
#include <kactioncollection.h>
#include <kxmlguiwindow.h>

// BUG: if this symbol is defined the problem consists on:
//      - main window is created.
//      - settings are saved (and applied), but in this case no toolbars exist yet, so they don't
//        apply to any toolbar.
//      - after 1 second the GUI is created.
//
//      How to reproduce ?
//          - Move one toolbar to other place (bottom, left, right, or deattach it).
//          - Close the test (so settings are saved).
//          - Reopen the test. The toolbar you moved is not keeping the place you specified.
#define REPRODUCE_TOOLBAR_BUG

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public Q_SLOTS:
    void slotTest();
    void slotCreate();

private:
    void setupActions();
};

void MainWindow::slotTest()
{
    KMessageBox::information(nullptr, QStringLiteral("Test"), QStringLiteral("Test"));
}

void MainWindow::slotCreate()
{
    setupGUI(ToolBar | Keys);
    createGUI(xmlFile());

    if (autoSaveConfigGroup().isValid()) {
        applyMainWindowSettings(autoSaveConfigGroup());
    }
}

void MainWindow::setupActions()
{
    QAction *testAction = new QAction(this);
    testAction->setText(QStringLiteral("Test"));
    testAction->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    actionCollection()->addAction(QStringLiteral("test"), testAction);
    connect(testAction, &QAction::triggered, this, &MainWindow::slotTest);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());

    setAutoSaveSettings();

    // BUG: if the GUI is created after an amount of time (so settings have been saved), then toolbars
    //      are shown misplaced. KMainWindow uses a 500 ms timer to save window settings.
#ifdef REPRODUCE_TOOLBAR_BUG
    QTimer::singleShot(1000, this, &MainWindow::slotCreate); // more than 500 ms so the main window has saved settings.
    // We can think of this case on natural applications when they
    // load plugins and change parts. It can take 1 second perfectly.
#else
    QTimer::singleShot(0, this, &MainWindow::slotCreate);
#endif
}

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setXMLFile(QFINDTESTDATA("kxmlguiwindowtestui.rc"), true);
    // Because we use a full path in setXMLFile, we need to call setLocalXMLFile too.
    // In your apps, just pass a relative filename to setXMLFile instead.
    setLocalXMLFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kxmlguiwindowtest/kxmlguiwindowtestui.rc"));

    setCentralWidget(new QTextEdit(this));
    setupActions();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    return app.exec();
}

#include "kxmlguiwindowtest.moc"
