/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "testmainwindow.h"
#include "parts.h"

#include <KParts/PartLoader>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KXMLGUIFactory>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QSplitter>
#include <QStandardPaths>
#include <QTest>
#include <kparts/partmanager.h>

TestMainWindow::TestMainWindow()
    : KParts::MainWindow()
{
    setXMLFile(QFINDTESTDATA("kpartstest_shell.rc"));

    m_manager = new KParts::PartManager(this);

    // When the manager says the active part changes, the builder updates (recreates) the GUI
    connect(m_manager, &KParts::PartManager::activePartChanged, this, &TestMainWindow::createGUI);

    // We can do this "switch active part" because we have a splitter with
    // two items in it.
    // I wonder what kdevelop uses/will use to embed kedit, BTW.
    m_splitter = new QSplitter(this);

    m_part1 = new Part1(this, m_splitter);
    m_part2 = new Part2(this, m_splitter);

    KActionCollection *coll = actionCollection();

    QAction *paOpen = new QAction(QStringLiteral("&View local file"), this);
    coll->addAction(QStringLiteral("open_local_file"), paOpen);
    connect(paOpen, &QAction::triggered, this, &TestMainWindow::slotFileOpen);
    QAction *paOpenRemote = new QAction(QStringLiteral("&View remote file"), this);
    coll->addAction(QStringLiteral("open_remote_file"), paOpenRemote);
    connect(paOpenRemote, &QAction::triggered, this, &TestMainWindow::slotFileOpenRemote);

    m_paEditFile = new QAction(QStringLiteral("&Edit file"), this);
    coll->addAction(QStringLiteral("edit_file"), m_paEditFile);
    connect(m_paEditFile, &QAction::triggered, this, &TestMainWindow::slotFileEdit);
    m_paCloseEditor = new QAction(QStringLiteral("&Close file editor"), this);
    coll->addAction(QStringLiteral("close_editor"), m_paCloseEditor);
    connect(m_paCloseEditor, &QAction::triggered, this, &TestMainWindow::slotFileCloseEditor);
    m_paCloseEditor->setEnabled(false);
    QAction *paQuit = new QAction(QStringLiteral("&Quit"), this);
    coll->addAction(QStringLiteral("shell_quit"), paQuit);
    connect(paQuit, &QAction::triggered, this, &TestMainWindow::close);
    paQuit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));

    //  (void)new QAction( "Yet another menu item", coll, "shell_yami" );
    //  (void)new QAction( "Yet another submenu item", coll, "shell_yasmi" );

    KStandardAction::configureToolbars(this, &KParts::MainWindow::configureToolbars, actionCollection());

    KStandardAction::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());

    setCentralWidget(m_splitter);
    m_splitter->setMinimumSize(400, 300);

    m_splitter->show();

    m_manager->addPart(m_part1, true); // sets part 1 as the active part
    m_manager->addPart(m_part2, false);
    m_editorpart = nullptr;
}

TestMainWindow::~TestMainWindow()
{
    disconnect(m_manager, nullptr, this, nullptr);
}

void TestMainWindow::slotFileOpen()
{
    const QString file =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QCoreApplication::instance()->applicationName() + QStringLiteral("/kpartstest_shell.rc"));
    if (!m_part1->openUrl(QUrl::fromLocalFile(file))) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file!"));
    }
}

void TestMainWindow::slotFileOpenRemote()
{
    QUrl u(QStringLiteral("http://www.kde.org/index.html"));
    if (!m_part1->openUrl(u)) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file!"));
    }
}

void TestMainWindow::embedEditor()
{
    if (m_manager->activePart() == m_part2) {
        createGUI(nullptr);
    }

    // replace part2 with the editor part
    delete m_part2;
    m_part2 = nullptr;
    if (auto res = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadWritePart>(QStringLiteral("text/plain"), m_splitter, this)) {
        m_editorpart = res.plugin;
        m_editorpart->setReadWrite(); // read-write mode
        m_manager->addPart(m_editorpart);
        m_paEditFile->setEnabled(false);
        m_paCloseEditor->setEnabled(true);
    } else {
        qWarning() << res.errorString;
    }
}

void TestMainWindow::slotFileCloseEditor()
{
    // It is very important to close the url of a read-write part
    // before destroying it. This allows to save the document (if modified)
    // and also to cancel.
    if (!m_editorpart->closeUrl()) {
        return;
    }

    // Is this necessary ? (David)
    if (m_manager->activePart() == m_editorpart) {
        createGUI(nullptr);
    }

    delete m_editorpart;
    m_editorpart = nullptr;
    m_part2 = new Part2(this, m_splitter);
    m_manager->addPart(m_part2);
    m_paEditFile->setEnabled(true);
    m_paCloseEditor->setEnabled(false);
}

void TestMainWindow::slotFileEdit()
{
    if (!m_editorpart) {
        embedEditor();
    }
    // TODO use KFileDialog to allow testing remote files
    const QString file(QDir::current().absolutePath() + QStringLiteral("/kpartstest_shell.rc"));
    if (!m_editorpart->openUrl(QUrl::fromLocalFile(file))) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file!"));
    }
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kpartstest"));
    QApplication app(argc, argv);

    TestMainWindow *shell = new TestMainWindow;
    shell->show();

    return app.exec();
}

#include "moc_testmainwindow.cpp"
