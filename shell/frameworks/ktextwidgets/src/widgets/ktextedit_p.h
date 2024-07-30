/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2005 Michael Brade <brade@kde.org>
    SPDX-FileCopyrightText: 2012 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDIT_P_H
#define KTEXTEDIT_P_H

#include "kfind.h"
#include "kfinddialog.h"
#include "kreplace.h"
#include "kreplacedialog.h"

#include <Sonnet/SpellCheckDecorator>
#include <Sonnet/Speller>

#include <QSettings>
#include <QTextDocumentFragment>
#ifdef HAVE_SPEECH
#include <QTextToSpeech>
#endif

class KTextEditPrivate
{
    Q_DECLARE_PUBLIC(KTextEdit)

public:
    explicit KTextEditPrivate(KTextEdit *qq)
        : q_ptr(qq)
        , customPalette(false)
        , spellCheckingEnabled(false)
        , findReplaceEnabled(true)
        , showTabAction(true)
        , showAutoCorrectionButton(false)
    {
        // Check the default sonnet settings to see if spellchecking should be enabled.
        QSettings settings(QStringLiteral("KDE"), QStringLiteral("Sonnet"));
        spellCheckingEnabled = settings.value(QStringLiteral("checkerEnabledByDefault"), false).toBool();
    }

    virtual ~KTextEditPrivate()
    {
        delete decorator;
        delete findDlg;
        delete find;
        delete replace;
        delete repDlg;
        delete speller;
#ifdef HAVE_SPEECH
        delete textToSpeech;
#endif
    }

    /**
     * Checks whether we should/should not consume a key used as a shortcut.
     * This makes it possible to handle shortcuts in the focused widget before any
     * window-global QAction is triggered.
     */
    bool overrideShortcut(const QKeyEvent *e);
    /**
     * Actually handle a shortcut event.
     */
    bool handleShortcut(const QKeyEvent *e);

    void spellCheckerMisspelling(const QString &text, int pos);
    void spellCheckerCorrected(const QString &, int, const QString &);
    void spellCheckerAutoCorrect(const QString &, const QString &);
    void spellCheckerCanceled();
    void spellCheckerFinished();
    void toggleAutoSpellCheck();

    void slotFindHighlight(const QString &text, int matchingIndex, int matchingLength);
    void slotReplaceText(const QString &text, int replacementIndex, int /*replacedLength*/, int matchedLength);

    /**
     * Similar to QTextEdit::clear(), only that it is possible to undo this
     * action.
     */
    void undoableClear();

    void slotAllowTab();
    void menuActivated(QAction *action);

    void init();

    void checkSpelling(bool force);

    KTextEdit *const q_ptr;
    QAction *autoSpellCheckAction;
    QAction *allowTab;
    QAction *spellCheckAction;
    QMenu *languagesMenu = nullptr;
    bool customPalette : 1;

    bool spellCheckingEnabled : 1;
    bool findReplaceEnabled : 1;
    bool showTabAction : 1;
    bool showAutoCorrectionButton : 1;
    QTextDocumentFragment originalDoc;
    QString spellCheckingLanguage;
    Sonnet::SpellCheckDecorator *decorator = nullptr;
    Sonnet::Speller *speller = nullptr;
    KFindDialog *findDlg = nullptr;
    KFind *find = nullptr;
    KReplaceDialog *repDlg = nullptr;
    KReplace *replace = nullptr;
#ifdef HAVE_SPEECH
    QTextToSpeech *textToSpeech = nullptr;
#endif

    int findIndex = 0;
    int repIndex = 0;
    int lastReplacedPosition = -1;
};

#endif
