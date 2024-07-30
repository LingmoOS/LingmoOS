/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef __normalktm_h__
#define __normalktm_h__

#include <KXmlGuiWindow>
#include <kparts/readwritepart.h>

class QAction;
class QWidget;

class TestMainWindow : public KXmlGuiWindow
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
    QWidget *m_splitter;
};

#endif
