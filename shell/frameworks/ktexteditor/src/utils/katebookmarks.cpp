/*
    SPDX-FileCopyrightText: 2002, 2003, 2004 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katebookmarks.h"

#include "kateabstractinputmode.h"
#include "katedocument.h"
#include "kateview.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KGuiItem>
#include <KLocalizedString>
#include <KStringHandler>
#include <KToggleAction>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QEvent>
#include <QList>
#include <QMenu>
#include <QRegularExpression>

namespace KTextEditor
{
class Document;
}

KateBookmarks::KateBookmarks(KTextEditor::ViewPrivate *view, Sorting sort)
    : QObject(view)
    , m_view(view)
    , m_bookmarkClear(nullptr)
    , m_sorting(sort)
{
    setObjectName(QStringLiteral("kate bookmarks"));
    connect(view->doc(), &KTextEditor::DocumentPrivate::marksChanged, this, &KateBookmarks::marksChanged);
    _tries = 0;
    m_bookmarksMenu = nullptr;
}

KateBookmarks::~KateBookmarks() = default;

void KateBookmarks::createActions(KActionCollection *ac)
{
    m_bookmarkToggle = new KToggleAction(i18n("Set &Bookmark"), this);
    ac->addAction(QStringLiteral("bookmarks_toggle"), m_bookmarkToggle);
    m_bookmarkToggle->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new")));
    ac->setDefaultShortcut(m_bookmarkToggle, Qt::CTRL | Qt::Key_B);
    m_bookmarkToggle->setWhatsThis(i18n("If a line has no bookmark then add one, otherwise remove it."));
    connect(m_bookmarkToggle, &QAction::triggered, this, &KateBookmarks::toggleBookmark);

    m_bookmarkClear = new QAction(i18n("Clear &All Bookmarks"), this);
    ac->addAction(QStringLiteral("bookmarks_clear"), m_bookmarkClear);
    m_bookmarkClear->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-remove")));
    m_bookmarkClear->setWhatsThis(i18n("Remove all bookmarks of the current document."));
    connect(m_bookmarkClear, &QAction::triggered, this, &KateBookmarks::clearBookmarks);

    m_goNext = new QAction(i18n("Next Bookmark"), this);
    ac->addAction(QStringLiteral("bookmarks_next"), m_goNext);
    m_goNext->setIcon(QIcon::fromTheme(QStringLiteral("go-down-search")));
    ac->setDefaultShortcut(m_goNext, Qt::ALT | Qt::Key_PageDown);
    m_goNext->setWhatsThis(i18n("Go to the next bookmark."));
    connect(m_goNext, &QAction::triggered, this, &KateBookmarks::goNext);

    m_goPrevious = new QAction(i18n("Previous Bookmark"), this);
    ac->addAction(QStringLiteral("bookmarks_previous"), m_goPrevious);
    m_goPrevious->setIcon(QIcon::fromTheme(QStringLiteral("go-up-search")));
    ac->setDefaultShortcut(m_goPrevious, Qt::ALT | Qt::Key_PageUp);
    m_goPrevious->setWhatsThis(i18n("Go to the previous bookmark."));
    connect(m_goPrevious, &QAction::triggered, this, &KateBookmarks::goPrevious);

    KActionMenu *actionMenu = new KActionMenu(i18n("&Bookmarks"), this);
    actionMenu->setPopupMode(QToolButton::InstantPopup);
    actionMenu->setIcon(QIcon::fromTheme(QStringLiteral("bookmarks")));
    ac->addAction(QStringLiteral("bookmarks"), actionMenu);
    m_bookmarksMenu = actionMenu->menu();

    connect(m_bookmarksMenu, &QMenu::aboutToShow, this, &KateBookmarks::bookmarkMenuAboutToShow);
    // Ensure the bookmarks menu is populated with at least the basic actions, otherwise macOS will not show it in the global menu bar.
    bookmarkMenuAboutToShow();

    marksChanged();

    // Always want the actions with shortcuts plugged into something so their shortcuts can work
    m_view->addAction(m_bookmarkToggle);
    m_view->addAction(m_bookmarkClear);
    m_view->addAction(m_goNext);
    m_view->addAction(m_goPrevious);
}

void KateBookmarks::toggleBookmark()
{
    uint mark = m_view->doc()->mark(m_view->cursorPosition().line());
    if (mark & KTextEditor::Document::markType01) {
        m_view->doc()->removeMark(m_view->cursorPosition().line(), KTextEditor::Document::markType01);
    } else {
        m_view->doc()->addMark(m_view->cursorPosition().line(), KTextEditor::Document::markType01);
    }
}

void KateBookmarks::clearBookmarks()
{
    // work on a COPY of the hash, the removing will modify it otherwise!
    const auto hash = m_view->doc()->marks();
    for (auto it = hash.cbegin(); it != hash.cend(); ++it) {
        m_view->doc()->removeMark(it.value()->line, KTextEditor::Document::markType01);
    }
}

void KateBookmarks::insertBookmarks(QMenu &menu)
{
    const int line = m_view->cursorPosition().line();
    static const QRegularExpression re(QStringLiteral("&(?!&)"));
    int next = -1; // -1 means next bookmark doesn't exist
    int prev = -1; // -1 means previous bookmark doesn't exist

    // reference ok, not modified
    const auto &hash = m_view->doc()->marks();
    if (hash.isEmpty()) {
        return;
    }

    std::vector<int> bookmarkLineArray; // Array of line numbers which have bookmarks

    // Find line numbers where bookmarks are set & store those line numbers in bookmarkLineArray
    for (auto it = hash.cbegin(); it != hash.cend(); ++it) {
        if (it.value()->type & KTextEditor::Document::markType01) {
            bookmarkLineArray.push_back(it.value()->line);
        }
    }

    if (m_sorting == Position) {
        std::sort(bookmarkLineArray.begin(), bookmarkLineArray.end());
    }

    QAction *firstNewAction = menu.addSeparator();
    // Consider each line with a bookmark one at a time
    for (size_t i = 0; i < bookmarkLineArray.size(); ++i) {
        const int lineNo = bookmarkLineArray.at(i);
        // Get text in this particular line in a QString
        QFontMetrics fontMetrics(menu.fontMetrics());
        QString bText = fontMetrics.elidedText(m_view->doc()->line(lineNo), Qt::ElideRight, fontMetrics.maxWidth() * 32);
        bText.replace(re, QStringLiteral("&&")); // kill undesired accellerators!
        bText.replace(QLatin1Char('\t'), QLatin1Char(' ')); // kill tabs, as they are interpreted as shortcuts

        QAction *before = nullptr;
        if (m_sorting == Position) {
            // 3 actions already present
            if (size_t(menu.actions().size()) <= i + 3) {
                before = nullptr;
            } else {
                before = menu.actions().at(i + 3);
            }
        }

        const QString actionText(QStringLiteral("%1  %2  - \"%3\"").arg(QString::number(lineNo + 1), m_view->currentInputMode()->bookmarkLabel(lineNo), bText));
        // Adding action for this bookmark in menu
        if (before) {
            QAction *a = new QAction(actionText, &menu);
            menu.insertAction(before, a);
            connect(a, &QAction::triggered, this, [this, lineNo]() {
                gotoLine(lineNo);
            });

            if (!firstNewAction) {
                firstNewAction = a;
            }
        } else {
            menu.addAction(actionText, this, [this, lineNo]() {
                gotoLine(lineNo);
            });
        }

        // Find the line number of previous & next bookmark (if present) in relation to the cursor
        if (lineNo < line) {
            if (prev == -1 || prev < lineNo) {
                prev = lineNo;
            }
        } else if (lineNo > line) {
            if (next == -1 || next > lineNo) {
                next = lineNo;
            }
        }
    }

    if (next != -1) {
        // Insert action for next bookmark
        m_goNext->setText(i18n("&Next: %1 - \"%2\"", next + 1, KStringHandler::rsqueeze(m_view->doc()->line(next), 24)));
        menu.insertAction(firstNewAction, m_goNext);
        firstNewAction = m_goNext;
    }
    if (prev != -1) {
        // Insert action for previous bookmark
        m_goPrevious->setText(i18n("&Previous: %1 - \"%2\"", prev + 1, KStringHandler::rsqueeze(m_view->doc()->line(prev), 24)));
        menu.insertAction(firstNewAction, m_goPrevious);
        firstNewAction = m_goPrevious;
    }

    if (next != -1 || prev != -1) {
        menu.insertSeparator(firstNewAction);
    }
}

void KateBookmarks::gotoLine(int line)
{
    m_view->setCursorPosition(KTextEditor::Cursor(line, 0));
}

void KateBookmarks::bookmarkMenuAboutToShow()
{
    m_bookmarksMenu->clear();
    m_bookmarkToggle->setChecked(m_view->doc()->mark(m_view->cursorPosition().line()) & KTextEditor::Document::markType01);
    m_bookmarksMenu->addAction(m_bookmarkToggle);
    m_bookmarksMenu->addAction(m_bookmarkClear);

    m_goNext->setText(i18n("Next Bookmark"));
    m_goPrevious->setText(i18n("Previous Bookmark"));

    insertBookmarks(*m_bookmarksMenu);
}

void KateBookmarks::goNext()
{
    // reference ok, not modified
    const auto &hash = m_view->doc()->marks();
    if (hash.isEmpty()) {
        return;
    }

    int line = m_view->cursorPosition().line();
    int found = -1;
    int firstBookmarkLine = -1;
    for (auto it = hash.cbegin(); it != hash.cend(); ++it) {
        const int markLine = it.value()->line;
        if (markLine > line && (found == -1 || found > markLine)) {
            found = markLine;
        }
        if (markLine < firstBookmarkLine || firstBookmarkLine == -1) {
            firstBookmarkLine = markLine;
        }
    }

    // either go to next bookmark or the first in the document, bug 472354
    if (found != -1) {
        gotoLine(found);
    } else {
        gotoLine(firstBookmarkLine);
    }
}

void KateBookmarks::goPrevious()
{
    // reference ok, not modified
    const auto &hash = m_view->doc()->marks();
    if (hash.isEmpty()) {
        return;
    }

    int line = m_view->cursorPosition().line();
    int found = -1;
    int lastBookmarkLine = -1;
    for (auto it = hash.cbegin(); it != hash.cend(); ++it) {
        const int markLine = it.value()->line;
        if (markLine < line && (found == -1 || found < markLine)) {
            found = markLine;
        }
        if (markLine > lastBookmarkLine) {
            lastBookmarkLine = markLine;
        }
    }

    // either go to previous bookmark or the last in the document, bug 472354
    if (found != -1) {
        gotoLine(found);
    } else {
        gotoLine(lastBookmarkLine);
    }
}

void KateBookmarks::marksChanged()
{
    const bool bookmarks = !m_view->doc()->marks().isEmpty();
    if (m_bookmarkClear) {
        m_bookmarkClear->setEnabled(bookmarks);
    }
    if (m_goNext) {
        m_goNext->setEnabled(bookmarks);
    }
    if (m_goPrevious) {
        m_goPrevious->setEnabled(bookmarks);
    }
}
