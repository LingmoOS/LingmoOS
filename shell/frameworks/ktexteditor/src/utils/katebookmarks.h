/*
    SPDX-FileCopyrightText: 2002, 2003, 2004 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_BOOKMARKS_H
#define KATE_BOOKMARKS_H

#include <QObject>

namespace KTextEditor
{
class ViewPrivate;
}

class KToggleAction;
class KActionCollection;
class QMenu;
class QAction;

class KateBookmarks : public QObject
{
public:
    enum Sorting { Position, Creation };
    explicit KateBookmarks(KTextEditor::ViewPrivate *parent, Sorting sort = Position);
    ~KateBookmarks() override;

    void createActions(KActionCollection *);

    KateBookmarks::Sorting sorting()
    {
        return m_sorting;
    }
    void setSorting(Sorting s)
    {
        m_sorting = s;
    }

protected:
    void insertBookmarks(QMenu &menu);

private:
    void toggleBookmark();
    void clearBookmarks();

    void gotoLine(int line);

    void bookmarkMenuAboutToShow();

    void goNext();
    void goPrevious();

    void marksChanged();

private:
    KTextEditor::ViewPrivate *m_view;
    KToggleAction *m_bookmarkToggle;
    QAction *m_bookmarkClear;
    QAction *m_goNext;
    QAction *m_goPrevious;

    Sorting m_sorting;
    QMenu *m_bookmarksMenu;

    uint _tries;
};

#endif
