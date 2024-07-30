/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TESTMAINWINDOW_H
#define TESTMAINWINDOW_H

#include <kparts/mainwindow.h>
#include <kparts/readwritepart.h>

namespace KParts
{
class PartManager;
}
class QAction;
class QWidget;

class TestMainWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    TestMainWindow();
    ~TestMainWindow() override;

protected Q_SLOTS:
    void slotFileOpen();
    void slotFileOpenRemote();
    void slotFileEdit();
    void slotFileCloseEditor();

protected:
    void embedEditor();

private:
    QAction *m_paEditFile;
    QAction *m_paCloseEditor;

    KParts::ReadOnlyPart *m_part1;
    KParts::Part *m_part2;
    KParts::ReadWritePart *m_editorpart;
    KParts::PartManager *m_manager;
    QWidget *m_splitter;
};

#endif
