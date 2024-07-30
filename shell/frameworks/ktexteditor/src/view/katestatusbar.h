/*
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_STATUS_BAR_H
#define KATE_STATUS_BAR_H

#include "kateviewhelpers.h"

#include <KLocalizedString>
#include <KSqueezedTextLabel>

#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>

namespace KTextEditor
{
class ViewPrivate;
}

class WordCounter;
class KateStatusBar;
class KateModeMenuList;

class KateStatusBarOpenUpMenu : public QMenu
{
public:
    explicit KateStatusBarOpenUpMenu(QWidget *parent);
    void setVisible(bool) override;
};

/**
 * For convenience an own button class to ensure a unified look&feel.
 * Should someone dislike the QPushButton at all could he change it
 * to a e.g. QLabel subclass
 */
class StatusBarButton : public QPushButton
{
public:
    explicit StatusBarButton(KateStatusBar *parent, const QString &text = QString());
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
};

class KateStatusBar : public KateViewBarWidget
{
    Q_OBJECT
    friend class StatusBarButton;

public:
    explicit KateStatusBar(KTextEditor::ViewPrivate *view);

    KateModeMenuList *modeMenu() const;

public Q_SLOTS:
    void updateStatus();

    void viewModeChanged();

    void cursorPositionChanged();

    void updateDictionary();

    void selectionChanged();

    void documentConfigChanged();

    void modeChanged();

    void wordCountChanged(int, int, int, int);

    void toggleWordCount(bool on);

    void configChanged();

    void changeDictionary(QAction *action);

    void updateEOL();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    KTextEditor::ViewPrivate *const m_view;
    StatusBarButton *m_cursorPosition = nullptr;
    QString m_wordCount;
    StatusBarButton *m_zoomLevel = nullptr;
    StatusBarButton *m_inputMode = nullptr;
    StatusBarButton *m_mode = nullptr;
    StatusBarButton *m_encoding = nullptr;
    StatusBarButton *m_tabsIndent = nullptr;
    StatusBarButton *m_dictionary = nullptr;
    StatusBarButton *m_eol = nullptr;
    QActionGroup *m_dictionaryGroup = nullptr;
    KateStatusBarOpenUpMenu *m_dictionaryMenu = nullptr;
    QMenu *m_indentSettingsMenu;
    KateModeMenuList *m_modeMenuList = nullptr;
    unsigned int m_selectionMode;
    QActionGroup *m_tabGroup;
    QActionGroup *m_indentGroup;
    QAction *m_mixedAction;
    QAction *m_hardAction;
    QAction *m_softAction;
    WordCounter *m_wordCounter;

private:
    void addNumberAction(QActionGroup *group, QMenu *menu, int data);
    void updateGroup(QActionGroup *group, int w);

public Q_SLOTS:
    void slotTabGroup(QAction *);
    void slotIndentGroup(QAction *);
    void slotIndentTabMode(QAction *);
    void toggleShowLines(bool checked);
    void toggleShowWords(bool checked);
};

#endif
