/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "normalktm.h"
#include "parts.h"

#include <KParts/PartLoader>

#include <QCheckBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <QAction>
#include <QApplication>
#include <QStandardPaths>

TestMainWindow::TestMainWindow()
    : KXmlGuiWindow()
{
    // We can do this "switch active part" because we have a splitter with
    // two items in it.
    // I wonder what kdevelop uses/will use to embed kedit, BTW.
    m_splitter = new QSplitter(this);

    m_part1 = new Part1(this, m_splitter);
    m_part2 = new Part2(this, m_splitter);

    QMenu *pFile = new QMenu(QStringLiteral("File"), menuBar());
    KActionCollection *coll = actionCollection();
    QAction *paLocal = new QAction(QStringLiteral("&View local file"), this);
    coll->addAction(QStringLiteral("open_local_file"), paLocal);
    connect(paLocal, &QAction::triggered, this, &TestMainWindow::slotFileOpen);
    // No XML: we need to add our actions to the menus ourselves
    pFile->addAction(paLocal);

    QAction *paRemote = new QAction(QStringLiteral("&View remote file"), this);
    coll->addAction(QStringLiteral("open_remote_file"), paRemote);
    connect(paRemote, &QAction::triggered, this, &TestMainWindow::slotFileOpenRemote);
    pFile->addAction(paRemote);

    m_paEditFile = new QAction(QStringLiteral("&Edit file"), this);
    coll->addAction(QStringLiteral("edit_file"), m_paEditFile);
    connect(m_paEditFile, &QAction::triggered, this, &TestMainWindow::slotFileEdit);
    pFile->addAction(m_paEditFile);

    m_paCloseEditor = new QAction(QStringLiteral("&Close file editor"), this);
    coll->addAction(QStringLiteral("close_editor"), m_paCloseEditor);
    connect(m_paCloseEditor, &QAction::triggered, this, &TestMainWindow::slotFileCloseEditor);
    m_paCloseEditor->setEnabled(false);
    pFile->addAction(m_paCloseEditor);

    QAction *paQuit = new QAction(QStringLiteral("&Quit"), this);
    coll->addAction(QStringLiteral("shell_quit"), paQuit);
    connect(paQuit, &QAction::triggered, this, &TestMainWindow::close);
    paQuit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    pFile->addAction(paQuit);

    setCentralWidget(m_splitter);
    m_splitter->setMinimumSize(400, 300);

    m_splitter->show();

    m_editorpart = nullptr;
}

TestMainWindow::~TestMainWindow()
{
}

void TestMainWindow::slotFileOpen()
{
    const QString file =
        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QCoreApplication::instance()->applicationName() + QStringLiteral("/kpartstest_shell.rc"));
    if (!m_part1->openUrl(QUrl::fromLocalFile(file))) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file !"));
    }
}

void TestMainWindow::slotFileOpenRemote()
{
    QUrl u(QStringLiteral("http://www.kde.org/index.html"));
    if (!m_part1->openUrl(u)) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file !"));
    }
}

void TestMainWindow::embedEditor()
{
    // replace part2 with the editor part
    delete m_part2;
    m_part2 = nullptr;
    if (auto res = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadWritePart>(QStringLiteral("text/plain"), m_splitter, this)) {
        m_editorpart = res.plugin;
        m_editorpart->setReadWrite(); // read-write mode
        ////// m_manager->addPart( m_editorpart );
        m_editorpart->widget()->show(); //// we need to do this in a normal KTM....
        m_paEditFile->setEnabled(false);
        m_paCloseEditor->setEnabled(true);
    } else {
        qWarning() << res.errorString;
    }
}

void TestMainWindow::slotFileCloseEditor()
{
    delete m_editorpart;
    m_editorpart = nullptr;
    m_part2 = new Part2(this, m_splitter);
    ////// m_manager->addPart( m_part2 );
    m_part2->widget()->show(); //// we need to do this in a normal KTM....
    m_paEditFile->setEnabled(true);
    m_paCloseEditor->setEnabled(false);
}

void TestMainWindow::slotFileEdit()
{
    if (!m_editorpart) {
        embedEditor();
    }
    // TODO use KFileDialog to allow testing remote files
    if (!m_editorpart->openUrl(QUrl::fromLocalFile(QDir::current().absolutePath() + QStringLiteral("/kpartstest_shell.rc")))) {
        KMessageBox::error(this, QStringLiteral("Couldn't open file!"));
    }
}

int main(int argc, char **argv)
{
    // we cheat and call ourselves kpartstest for TestMainWindow::slotFileOpen()
    QApplication::setApplicationName(QStringLiteral("kpartstest"));
    QApplication app(argc, argv);

    TestMainWindow *shell = new TestMainWindow;
    shell->show();

    app.exec();

    return 0;
}

#include "moc_normalktm.cpp"
