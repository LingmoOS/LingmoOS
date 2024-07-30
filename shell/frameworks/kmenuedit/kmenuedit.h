/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef kmenuedit_h
#define kmenuedit_h

#include <KXmlGuiWindow>

class QSplitter;
class QAction;
class BasicTab;
class TreeView;
class KTreeWidgetSearchLine;

class KMenuEdit : public KXmlGuiWindow
{
    Q_OBJECT

public:
    KMenuEdit();
    ~KMenuEdit() override;

    void selectMenu(const QString &menu);
    void selectMenuEntry(const QString &menuEntry);

    // dbus method
    void restoreSystemMenu();

protected:
    void setupView();
    void setupActions();
    bool queryClose() override;

protected Q_SLOTS:
    void slotSave();
    void slotChangeView();
    void slotRestoreMenu();
    void slotConfigure();

protected:
    TreeView *m_tree = nullptr;
    BasicTab *m_basicTab = nullptr;
    QSplitter *m_splitter = nullptr;
    KTreeWidgetSearchLine *m_searchLine = nullptr;

    QAction *m_actionDelete = nullptr;
    bool m_showHidden = false;
};

#endif
