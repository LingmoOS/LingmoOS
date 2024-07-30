/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLINEEDIT_P_H
#define KLINEEDIT_P_H

#include "klineedit.h"

class KCompletionBox;
class KLineEditUrlDropEventFilter;

class KLineEditPrivate
{
public:
    explicit KLineEditPrivate(KLineEdit *parent)
        : q_ptr(parent)
    {
    }

    ~KLineEditPrivate();

    void _k_textChanged(const QString &text);
    void _k_completionMenuActivated(QAction *act);
    void _k_tripleClickTimeout(); // resets possibleTripleClick
    void _k_restoreSelectionColors();
    void _k_completionBoxTextChanged(const QString &text);

    void updateUserText(const QString &text);

    /**
     * Checks whether we should/should not consume a key used as a shortcut.
     * This makes it possible to handle shortcuts in the focused widget before any
     * window-global QAction is triggered.
     */
    bool overrideShortcut(const QKeyEvent *e);

    void init();

    bool copySqueezedText(bool copy) const;

    /**
     * Properly sets the squeezed text whenever the widget is
     * created or resized.
     */
    void setSqueezedText();

    QMap<KCompletion::CompletionMode, bool> disableCompletionMap;

    QColor previousHighlightColor;
    QColor previousHighlightedTextColor;

    QPalette::ColorRole bgRole;

    QString squeezedText;
    QString userText;
    QString lastStyleClass;

    QMetaObject::Connection m_matchesConnection;
    KCompletionBox *completionBox;

    KLineEditUrlDropEventFilter *urlDropEventFilter;

    QAction *noCompletionAction;
    QAction *shellCompletionAction;
    QAction *autoCompletionAction;
    QAction *popupCompletionAction;
    QAction *shortAutoCompletionAction;
    QAction *popupAutoCompletionAction;
    QAction *defaultAction;

    KLineEdit *const q_ptr;

    int squeezedEnd;
    int squeezedStart;

    static bool s_initialized;
    static bool s_backspacePerformsCompletion; // Configuration option

    bool userSelection : 1;
    bool autoSuggest : 1;
    bool disableRestoreSelection : 1;
    bool handleURLDrops : 1;
    bool trapReturnKeyEvents : 1;
    bool enableSqueezedText : 1;
    bool completionRunning : 1;
    bool italicizePlaceholder : 1;
    bool threeStars : 1;
    bool possibleTripleClick : 1; // set in mousePressEvent, deleted in tripleClickTimeout
    Q_DECLARE_PUBLIC(KLineEdit)
};

#endif
