/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1997 Sven Radej <sven.radej@iname.com>
    SPDX-FileCopyrightText: 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>

    Re-designed for KDE 2.x by
    SPDX-FileCopyrightText: 2000, 2001 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klineedit.h"
#include "klineedit_p.h"

#include <KAuthorized>
#include <KConfigGroup>
#include <KCursor>
#include <KLineEditUrlDropEventFilter>
#include <KSharedConfig>
#include <KStandardShortcut>

#include <kcompletionbox.h>

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QTimer>
#include <QToolTip>

KLineEditPrivate::~KLineEditPrivate()
{
    // causes a weird crash in KWord at least, so let Qt delete it for us.
    //        delete completionBox;
}

void KLineEditPrivate::_k_textChanged(const QString &text)
{
    Q_Q(KLineEdit);
    // COMPAT (as documented): emit userTextChanged whenever textChanged is emitted
    if (!completionRunning && (text != userText)) {
        userText = text;
    }
}

// Call this when a completion operation changes the lineedit text
// "as if it had been edited by the user".
void KLineEditPrivate::updateUserText(const QString &text)
{
    Q_Q(KLineEdit);
    if (!completionRunning && (text != userText)) {
        userText = text;
        q->setModified(true);
        Q_EMIT q->textEdited(text);
        Q_EMIT q->textChanged(text);
    }
}

bool KLineEditPrivate::s_backspacePerformsCompletion = false;
bool KLineEditPrivate::s_initialized = false;

void KLineEditPrivate::init()
{
    Q_Q(KLineEdit);
    //---
    completionBox = nullptr;
    handleURLDrops = true;
    trapReturnKeyEvents = false;

    userSelection = true;
    autoSuggest = false;
    disableRestoreSelection = false;
    enableSqueezedText = false;

    completionRunning = false;
    if (!s_initialized) {
        KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("General"));
        s_backspacePerformsCompletion = config.readEntry("Backspace performs completion", false);
        s_initialized = true;
    }

    urlDropEventFilter = new KLineEditUrlDropEventFilter(q);

    // i18n: Placeholder text in line edit widgets is the text appearing
    // before any user input, briefly explaining to the user what to type
    // (e.g. "Enter search pattern").
    // By default the text is set in italic, which may not be appropriate
    // for some languages and scripts (e.g. for CJK ideographs).
    QString metaMsg = KLineEdit::tr("1", "Italic placeholder text in line edits: 0 no, 1 yes");
    italicizePlaceholder = (metaMsg.trimmed() != QLatin1Char('0'));
    //---
    possibleTripleClick = false;
    bgRole = q->backgroundRole();

    // Enable the context menu by default.
    q->QLineEdit::setContextMenuPolicy(Qt::DefaultContextMenu);
    KCursor::setAutoHideCursor(q, true, true);

    KCompletion::CompletionMode mode = q->completionMode();
    autoSuggest = (mode == KCompletion::CompletionMan //
                   || mode == KCompletion::CompletionPopupAuto //
                   || mode == KCompletion::CompletionAuto);
    q->connect(q, &KLineEdit::selectionChanged, q, [this]() {
        _k_restoreSelectionColors();
    });

    if (handleURLDrops) {
        q->installEventFilter(urlDropEventFilter);
    }

    const QPalette p = q->palette();
    if (!previousHighlightedTextColor.isValid()) {
        previousHighlightedTextColor = p.color(QPalette::Normal, QPalette::HighlightedText);
    }
    if (!previousHighlightColor.isValid()) {
        previousHighlightColor = p.color(QPalette::Normal, QPalette::Highlight);
    }

    q->connect(q, &KLineEdit::textChanged, q, [this](const QString &text) {
        _k_textChanged(text);
    });
}

KLineEdit::KLineEdit(const QString &string, QWidget *parent)
    : QLineEdit(string, parent)
    , d_ptr(new KLineEditPrivate(this))
{
    Q_D(KLineEdit);
    d->init();
}

KLineEdit::KLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new KLineEditPrivate(this))
{
    Q_D(KLineEdit);
    d->init();
}

KLineEdit::~KLineEdit()
{
}

QSize KLineEdit::clearButtonUsedSize() const
{
    QSize s;

    if (isClearButtonEnabled()) {
        // from qlineedit_p.cpp

        const int iconSize = height() < 34 ? 16 : 32;
        const int buttonWidth = iconSize + 6;
        const int buttonHeight = iconSize + 2;

        s = QSize(buttonWidth, buttonHeight);
    }

    return s;
}

void KLineEdit::setCompletionMode(KCompletion::CompletionMode mode)
{
    Q_D(KLineEdit);
    KCompletion::CompletionMode oldMode = completionMode();

    if (oldMode != mode //
        && (oldMode == KCompletion::CompletionPopup || oldMode == KCompletion::CompletionPopupAuto) //
        && d->completionBox && d->completionBox->isVisible()) {
        d->completionBox->hide();
    }

    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if (echoMode() != QLineEdit::Normal) {
        mode = KCompletion::CompletionNone; // Override the request.
    }

    if (!KAuthorized::authorize(QStringLiteral("lineedit_text_completion"))) {
        mode = KCompletion::CompletionNone;
    }

    if (mode == KCompletion::CompletionPopupAuto || mode == KCompletion::CompletionAuto || mode == KCompletion::CompletionMan) {
        d->autoSuggest = true;
    } else {
        d->autoSuggest = false;
    }

    KCompletionBase::setCompletionMode(mode);
}

void KLineEdit::setCompletionModeDisabled(KCompletion::CompletionMode mode, bool disable)
{
    Q_D(KLineEdit);
    d->disableCompletionMap[mode] = disable;
}

void KLineEdit::setCompletedText(const QString &t, bool marked)
{
    Q_D(KLineEdit);
    if (!d->autoSuggest) {
        return;
    }

    const QString txt = text();

    if (t != txt) {
        setText(t);
        if (marked) {
            setSelection(t.length(), txt.length() - t.length());
        }
        setUserSelection(false);
    } else {
        setUserSelection(true);
    }
}

void KLineEdit::setCompletedText(const QString &text)
{
    KCompletion::CompletionMode mode = completionMode();
    const bool marked = (mode == KCompletion::CompletionAuto //
                         || mode == KCompletion::CompletionMan //
                         || mode == KCompletion::CompletionPopup //
                         || mode == KCompletion::CompletionPopupAuto);
    setCompletedText(text, marked);
}

void KLineEdit::rotateText(KCompletionBase::KeyBindingType type)
{
    KCompletion *comp = compObj();
    if (comp && //
        (type == KCompletionBase::PrevCompletionMatch //
         || type == KCompletionBase::NextCompletionMatch)) {
        QString input;

        if (type == KCompletionBase::PrevCompletionMatch) {
            input = comp->previousMatch();
        } else {
            input = comp->nextMatch();
        }

        // Skip rotation if previous/next match is null or the same text
        if (input.isEmpty() || input == displayText()) {
            return;
        }
        setCompletedText(input, hasSelectedText());
    }
}

void KLineEdit::makeCompletion(const QString &text)
{
    Q_D(KLineEdit);
    KCompletion *comp = compObj();
    KCompletion::CompletionMode mode = completionMode();

    if (!comp || mode == KCompletion::CompletionNone) {
        return; // No completion object...
    }

    const QString match = comp->makeCompletion(text);

    if (mode == KCompletion::CompletionPopup || mode == KCompletion::CompletionPopupAuto) {
        if (match.isEmpty()) {
            if (d->completionBox) {
                d->completionBox->hide();
                d->completionBox->clear();
            }
        } else {
            setCompletedItems(comp->allMatches(), comp->shouldAutoSuggest());
        }
    } else { // Auto,  ShortAuto (Man) and Shell
        // all other completion modes
        // If no match or the same match, simply return without completing.
        if (match.isEmpty() || match == text) {
            return;
        }

        if (mode != KCompletion::CompletionShell) {
            setUserSelection(false);
        }

        if (d->autoSuggest) {
            setCompletedText(match);
        }
    }
}

void KLineEdit::setReadOnly(bool readOnly)
{
    Q_D(KLineEdit);
    // Do not do anything if nothing changed...
    if (readOnly == isReadOnly()) {
        return;
    }

    QLineEdit::setReadOnly(readOnly);

    if (readOnly) {
        d->bgRole = backgroundRole();
        setBackgroundRole(QPalette::Window);
        if (d->enableSqueezedText && d->squeezedText.isEmpty()) {
            d->squeezedText = text();
            d->setSqueezedText();
        }
    } else {
        if (!d->squeezedText.isEmpty()) {
            setText(d->squeezedText);
            d->squeezedText.clear();
        }

        setBackgroundRole(d->bgRole);
    }
}

void KLineEdit::setSqueezedText(const QString &text)
{
    setSqueezedTextEnabled(true);
    setText(text);
}

void KLineEdit::setSqueezedTextEnabled(bool enable)
{
    Q_D(KLineEdit);
    d->enableSqueezedText = enable;
}

bool KLineEdit::isSqueezedTextEnabled() const
{
    Q_D(const KLineEdit);
    return d->enableSqueezedText;
}

void KLineEdit::setText(const QString &text)
{
    Q_D(KLineEdit);
    if (d->enableSqueezedText && isReadOnly()) {
        d->squeezedText = text;
        d->setSqueezedText();
        return;
    }

    QLineEdit::setText(text);
}

void KLineEditPrivate::setSqueezedText()
{
    Q_Q(KLineEdit);
    squeezedStart = 0;
    squeezedEnd = 0;
    const QString fullText = squeezedText;
    const int fullLength = fullText.length();
    const QFontMetrics fm(q->fontMetrics());
    const int labelWidth = q->size().width() - 2 * q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) - 2;
    const int textWidth = fm.boundingRect(fullText).width();

    // TODO: investigate use of QFontMetrics::elidedText for this
    if (textWidth > labelWidth) {
        const QStringView sview{fullText};
        // TODO: better would be "…" char (0x2026), but for that one would need to ensure it's from the main font,
        // otherwise if resulting in use of a new fallback font this can affect the metrics of the complete text,
        // resulting in shifted characters
        const QString ellipsisText = QStringLiteral("...");
        // start with the dots only
        QString squeezedText = ellipsisText;
        int squeezedWidth = fm.boundingRect(squeezedText).width();

        // estimate how many letters we can add to the dots on both sides
        int letters = fullText.length() * (labelWidth - squeezedWidth) / textWidth / 2;
        squeezedText = sview.left(letters) + ellipsisText + sview.right(letters);
        squeezedWidth = fm.boundingRect(squeezedText).width();

        if (squeezedWidth < labelWidth) {
            // we estimated too short
            // add letters while text < label
            do {
                letters++;
                squeezedText = sview.left(letters) + ellipsisText + sview.right(letters);
                squeezedWidth = fm.boundingRect(squeezedText).width();
            } while (squeezedWidth < labelWidth && letters <= fullLength / 2);
            letters--;
            squeezedText = sview.left(letters) + ellipsisText + sview.right(letters);
        } else if (squeezedWidth > labelWidth) {
            // we estimated too long
            // remove letters while text > label
            do {
                letters--;
                squeezedText = sview.left(letters) + ellipsisText + sview.right(letters);
                squeezedWidth = fm.boundingRect(squeezedText).width();
            } while (squeezedWidth > labelWidth && letters >= 5);
        }

        if (letters < 5) {
            // too few letters added -> we give up squeezing
            q->QLineEdit::setText(fullText);
        } else {
            q->QLineEdit::setText(squeezedText);
            squeezedStart = letters;
            squeezedEnd = fullText.length() - letters;
        }

        q->setToolTip(fullText);

    } else {
        q->QLineEdit::setText(fullText);

        q->setToolTip(QString());
        QToolTip::showText(q->pos(), QString()); // hide
    }

    q->setCursorPosition(0);
}

void KLineEdit::copy() const
{
    Q_D(const KLineEdit);
    if (!d->copySqueezedText(true)) {
        QLineEdit::copy();
    }
}

bool KLineEditPrivate::copySqueezedText(bool copy) const
{
    Q_Q(const KLineEdit);
    if (!squeezedText.isEmpty() && squeezedStart) {
        KLineEdit *that = const_cast<KLineEdit *>(q);
        if (!that->hasSelectedText()) {
            return false;
        }
        int start = q->selectionStart();
        int end = start + q->selectedText().length();
        if (start >= squeezedStart + 3) {
            start = start - 3 - squeezedStart + squeezedEnd;
        } else if (start > squeezedStart) {
            start = squeezedStart;
        }
        if (end >= squeezedStart + 3) {
            end = end - 3 - squeezedStart + squeezedEnd;
        } else if (end > squeezedStart) {
            end = squeezedEnd;
        }
        if (start == end) {
            return false;
        }
        QString t = squeezedText;
        t = t.mid(start, end - start);
        QApplication::clipboard()->setText(t, copy ? QClipboard::Clipboard : QClipboard::Selection);
        return true;
    }
    return false;
}

void KLineEdit::resizeEvent(QResizeEvent *ev)
{
    Q_D(KLineEdit);
    if (!d->squeezedText.isEmpty()) {
        d->setSqueezedText();
    }

    QLineEdit::resizeEvent(ev);
}

void KLineEdit::keyPressEvent(QKeyEvent *e)
{
    Q_D(KLineEdit);
    const int key = e->key() | e->modifiers();

    if (KStandardShortcut::copy().contains(key)) {
        copy();
        return;
    } else if (KStandardShortcut::paste().contains(key)) {
        // TODO:
        // we should restore the original text (not autocompleted), otherwise the paste
        // will get into troubles Bug: 134691
        if (!isReadOnly()) {
            paste();
        }
        return;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        QString text = QApplication::clipboard()->text(QClipboard::Selection);
        insert(text);
        deselect();
        return;
    } else if (KStandardShortcut::cut().contains(key)) {
        if (!isReadOnly()) {
            cut();
        }
        return;
    } else if (KStandardShortcut::undo().contains(key)) {
        if (!isReadOnly()) {
            undo();
        }
        return;
    } else if (KStandardShortcut::redo().contains(key)) {
        if (!isReadOnly()) {
            redo();
        }
        return;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        cursorWordBackward(true);
        if (hasSelectedText() && !isReadOnly()) {
            del();
        }

        e->accept();
        return;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        // Workaround for QT bug where
        cursorWordForward(true);
        if (hasSelectedText() && !isReadOnly()) {
            del();
        }

        e->accept();
        return;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        cursorWordBackward(false);
        e->accept();
        return;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        cursorWordForward(false);
        e->accept();
        return;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        home(false);
        e->accept();
        return;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        end(false);
        e->accept();
        return;
    }

    // Filter key-events if EchoMode is normal and
    // completion mode is not set to CompletionNone
    if (echoMode() == QLineEdit::Normal && completionMode() != KCompletion::CompletionNone) {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            const bool trap = (d->completionBox && d->completionBox->isVisible());
            const bool stopEvent = (trap
                                    || (d->trapReturnKeyEvents //
                                        && (e->modifiers() == Qt::NoButton || //
                                            e->modifiers() == Qt::KeypadModifier)));

            if (stopEvent) {
                Q_EMIT QLineEdit::returnPressed();
                e->accept();
            }
            Q_EMIT returnKeyPressed(displayText());
            if (trap) {
                d->completionBox->hide();
                deselect();
                setCursorPosition(text().length());
            }

            // Eat the event if the user asked for it, or if a completionbox was visible
            if (stopEvent) {
                return;
            }
        }

        const KeyBindingMap keys = keyBindingMap();
        const KCompletion::CompletionMode mode = completionMode();
        const bool noModifier = (e->modifiers() == Qt::NoButton //
                                 || e->modifiers() == Qt::ShiftModifier //
                                 || e->modifiers() == Qt::KeypadModifier);

        if ((mode == KCompletion::CompletionAuto //
             || mode == KCompletion::CompletionPopupAuto //
             || mode == KCompletion::CompletionMan) //
            && noModifier) {
            if (!d->userSelection && hasSelectedText() //
                && (e->key() == Qt::Key_Right || e->key() == Qt::Key_Left) //
                && e->modifiers() == Qt::NoButton) {
                const QString old_txt = text();
                d->disableRestoreSelection = true;
                const int start = selectionStart();

                deselect();
                QLineEdit::keyPressEvent(e);
                const int cPosition = cursorPosition();
                setText(old_txt);

                // keep cursor at cPosition
                setSelection(old_txt.length(), cPosition - old_txt.length());
                if (e->key() == Qt::Key_Right && cPosition > start) {
                    // the user explicitly accepted the autocompletion
                    d->updateUserText(text());
                }

                d->disableRestoreSelection = false;
                return;
            }

            if (e->key() == Qt::Key_Escape) {
                if (hasSelectedText() && !d->userSelection) {
                    del();
                    setUserSelection(true);
                }

                // Don't swallow the Escape press event for the case
                // of dialogs, which have Escape associated to Cancel
                e->ignore();
                return;
            }
        }

        if ((mode == KCompletion::CompletionAuto //
             || mode == KCompletion::CompletionMan)
            && noModifier) {
            const QString keycode = e->text();
            if (!keycode.isEmpty()
                && (keycode.unicode()->isPrint() //
                    || e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)) {
                const bool hasUserSelection = d->userSelection;
                const bool hadSelection = hasSelectedText();

                bool cursorNotAtEnd = false;

                const int start = selectionStart();
                const int cPos = cursorPosition();

                // When moving the cursor, we want to keep the autocompletion as an
                // autocompletion, so we want to process events at the cursor position
                // as if there was no selection. After processing the key event, we
                // can set the new autocompletion again.
                if (hadSelection && !hasUserSelection && start > cPos) {
                    del();
                    setCursorPosition(cPos);
                    cursorNotAtEnd = true;
                }

                d->disableRestoreSelection = true;
                QLineEdit::keyPressEvent(e);
                d->disableRestoreSelection = false;

                QString txt = text();
                int len = txt.length();
                if (!hasSelectedText() && len /*&& cursorPosition() == len */) {
                    if (e->key() == Qt::Key_Backspace) {
                        if (hadSelection && !hasUserSelection && !cursorNotAtEnd) {
                            backspace();
                            txt = text();
                            len = txt.length();
                        }

                        if (!d->s_backspacePerformsCompletion || !len) {
                            d->autoSuggest = false;
                        }
                    }

                    if (e->key() == Qt::Key_Delete) {
                        d->autoSuggest = false;
                    }

                    doCompletion(txt);

                    if ((e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)) {
                        d->autoSuggest = true;
                    }

                    e->accept();
                }

                return;
            }

        } else if ((mode == KCompletion::CompletionPopup || mode == KCompletion::CompletionPopupAuto) //
                   && noModifier && !e->text().isEmpty()) {
            const QString old_txt = text();
            const bool hasUserSelection = d->userSelection;
            const bool hadSelection = hasSelectedText();
            bool cursorNotAtEnd = false;

            const int start = selectionStart();
            const int cPos = cursorPosition();
            const QString keycode = e->text();

            // When moving the cursor, we want to keep the autocompletion as an
            // autocompletion, so we want to process events at the cursor position
            // as if there was no selection. After processing the key event, we
            // can set the new autocompletion again.
            if (hadSelection && !hasUserSelection && start > cPos
                && ((!keycode.isEmpty() && keycode.unicode()->isPrint()) //
                    || e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)) {
                del();
                setCursorPosition(cPos);
                cursorNotAtEnd = true;
            }

            const int selectedLength = selectedText().length();

            d->disableRestoreSelection = true;
            QLineEdit::keyPressEvent(e);
            d->disableRestoreSelection = false;

            if ((selectedLength != selectedText().length()) && !hasUserSelection) {
                d->_k_restoreSelectionColors(); // and set userSelection to true
            }

            QString txt = text();
            int len = txt.length();
            if ((txt != old_txt || txt != e->text()) && len /* && ( cursorPosition() == len || force )*/
                && ((!keycode.isEmpty() && keycode.unicode()->isPrint()) //
                    || e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)) {
                if (e->key() == Qt::Key_Backspace) {
                    if (hadSelection && !hasUserSelection && !cursorNotAtEnd) {
                        backspace();
                        txt = text();
                        len = txt.length();
                    }

                    if (!d->s_backspacePerformsCompletion) {
                        d->autoSuggest = false;
                    }
                }

                if (e->key() == Qt::Key_Delete) {
                    d->autoSuggest = false;
                }

                if (d->completionBox) {
                    d->completionBox->setCancelledText(txt);
                }

                doCompletion(txt);

                if ((e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) //
                    && mode == KCompletion::CompletionPopupAuto) {
                    d->autoSuggest = true;
                }

                e->accept();
            } else if (!len && d->completionBox && d->completionBox->isVisible()) {
                d->completionBox->hide();
            }

            return;
        } else if (mode == KCompletion::CompletionShell) {
            // Handles completion.
            QList<QKeySequence> cut;
            if (keys[TextCompletion].isEmpty()) {
                cut = KStandardShortcut::shortcut(KStandardShortcut::TextCompletion);
            } else {
                cut = keys[TextCompletion];
            }

            if (cut.contains(key)) {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                const QString txt = text();
                const int len = txt.length();
                if (cursorPosition() == len && len != 0) {
                    doCompletion(txt);
                    return;
                }
            } else if (d->completionBox) {
                d->completionBox->hide();
            }
        }

        // handle rotation
        // Handles previous match
        QList<QKeySequence> cut;
        if (keys[PrevCompletionMatch].isEmpty()) {
            cut = KStandardShortcut::shortcut(KStandardShortcut::PrevCompletion);
        } else {
            cut = keys[PrevCompletionMatch];
        }

        if (cut.contains(key)) {
            if (emitSignals()) {
                Q_EMIT textRotation(KCompletionBase::PrevCompletionMatch);
            }
            if (handleSignals()) {
                rotateText(KCompletionBase::PrevCompletionMatch);
            }
            return;
        }

        // Handles next match
        if (keys[NextCompletionMatch].isEmpty()) {
            cut = KStandardShortcut::shortcut(KStandardShortcut::NextCompletion);
        } else {
            cut = keys[NextCompletionMatch];
        }

        if (cut.contains(key)) {
            if (emitSignals()) {
                Q_EMIT textRotation(KCompletionBase::NextCompletionMatch);
            }
            if (handleSignals()) {
                rotateText(KCompletionBase::NextCompletionMatch);
            }
            return;
        }

        // substring completion
        if (compObj()) {
            QList<QKeySequence> cut;
            if (keys[SubstringCompletion].isEmpty()) {
                cut = KStandardShortcut::shortcut(KStandardShortcut::SubstringCompletion);
            } else {
                cut = keys[SubstringCompletion];
            }

            if (cut.contains(key)) {
                if (emitSignals()) {
                    Q_EMIT substringCompletion(text());
                }
                if (handleSignals()) {
                    setCompletedItems(compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }
    const int selectedLength = selectedText().length();

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent(e);

    if (selectedLength != selectedText().length()) {
        d->_k_restoreSelectionColors(); // and set userSelection to true
    }
}

void KLineEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_D(KLineEdit);
    if (e->button() == Qt::LeftButton) {
        d->possibleTripleClick = true;
        QTimer::singleShot(QApplication::doubleClickInterval(), this, [d]() {
            d->_k_tripleClickTimeout();
        });
    }
    QLineEdit::mouseDoubleClickEvent(e);
}

void KLineEdit::mousePressEvent(QMouseEvent *e)
{
    Q_D(KLineEdit);
    if (e->button() == Qt::LeftButton && d->possibleTripleClick) {
        selectAll();
        e->accept();
        return;
    }

    // if middle clicking and if text is present in the clipboard then clear the selection
    // to prepare paste operation
    if (e->button() == Qt::MiddleButton) {
        if (hasSelectedText() && !isReadOnly()) {
            if (QApplication::clipboard()->text(QClipboard::Selection).length() > 0) {
                backspace();
            }
        }
    }

    QLineEdit::mousePressEvent(e);
}

void KLineEdit::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(KLineEdit);
    QLineEdit::mouseReleaseEvent(e);

    if (QApplication::clipboard()->supportsSelection()) {
        if (e->button() == Qt::LeftButton) {
            // Fix copying of squeezed text if needed
            d->copySqueezedText(false);
        }
    }
}

void KLineEditPrivate::_k_tripleClickTimeout()
{
    possibleTripleClick = false;
}

QMenu *KLineEdit::createStandardContextMenu()
{
    Q_D(KLineEdit);
    QMenu *popup = QLineEdit::createStandardContextMenu();

    if (!isReadOnly()) {
        // FIXME: This code depends on Qt's action ordering.
        const QList<QAction *> actionList = popup->actions();
        enum {
            UndoAct,
            RedoAct,
            Separator1,
            CutAct,
            CopyAct,
            PasteAct,
            DeleteAct,
            ClearAct,
            Separator2,
            SelectAllAct,
            NCountActs,
        };
        QAction *separatorAction = nullptr;
        // separator we want is right after Delete right now.
        const int idx = actionList.indexOf(actionList[DeleteAct]) + 1;
        if (idx < actionList.count()) {
            separatorAction = actionList.at(idx);
        }
        if (separatorAction) {
            QAction *clearAllAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), tr("C&lear", "@action:inmenu"), this);
            clearAllAction->setShortcuts(QKeySequence::keyBindings(QKeySequence::DeleteCompleteLine));
            connect(clearAllAction, &QAction::triggered, this, &QLineEdit::clear);
            if (text().isEmpty()) {
                clearAllAction->setEnabled(false);
            }
            popup->insertAction(separatorAction, clearAllAction);
        }
    }

    // If a completion object is present and the input
    // widget is not read-only, show the Text Completion
    // menu item.
    if (compObj() && !isReadOnly() && KAuthorized::authorize(QStringLiteral("lineedit_text_completion"))) {
        QMenu *subMenu = popup->addMenu(QIcon::fromTheme(QStringLiteral("text-completion")), tr("Text Completion", "@title:menu"));
        connect(subMenu, &QMenu::triggered, this, [d](QAction *action) {
            d->_k_completionMenuActivated(action);
        });

        popup->addSeparator();

        QActionGroup *ag = new QActionGroup(this);
        d->noCompletionAction = ag->addAction(tr("None", "@item:inmenu Text Completion"));
        d->shellCompletionAction = ag->addAction(tr("Manual", "@item:inmenu Text Completion"));
        d->autoCompletionAction = ag->addAction(tr("Automatic", "@item:inmenu Text Completion"));
        d->popupCompletionAction = ag->addAction(tr("Dropdown List", "@item:inmenu Text Completion"));
        d->shortAutoCompletionAction = ag->addAction(tr("Short Automatic", "@item:inmenu Text Completion"));
        d->popupAutoCompletionAction = ag->addAction(tr("Dropdown List && Automatic", "@item:inmenu Text Completion"));
        subMenu->addActions(ag->actions());

        // subMenu->setAccel( KStandardShortcut::completion(), ShellCompletion );

        d->shellCompletionAction->setCheckable(true);
        d->noCompletionAction->setCheckable(true);
        d->popupCompletionAction->setCheckable(true);
        d->autoCompletionAction->setCheckable(true);
        d->shortAutoCompletionAction->setCheckable(true);
        d->popupAutoCompletionAction->setCheckable(true);

        d->shellCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionShell]);
        d->noCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionNone]);
        d->popupCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionPopup]);
        d->autoCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionAuto]);
        d->shortAutoCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionMan]);
        d->popupAutoCompletionAction->setEnabled(!d->disableCompletionMap[KCompletion::CompletionPopupAuto]);

        const KCompletion::CompletionMode mode = completionMode();
        d->noCompletionAction->setChecked(mode == KCompletion::CompletionNone);
        d->shellCompletionAction->setChecked(mode == KCompletion::CompletionShell);
        d->popupCompletionAction->setChecked(mode == KCompletion::CompletionPopup);
        d->autoCompletionAction->setChecked(mode == KCompletion::CompletionAuto);
        d->shortAutoCompletionAction->setChecked(mode == KCompletion::CompletionMan);
        d->popupAutoCompletionAction->setChecked(mode == KCompletion::CompletionPopupAuto);

        const KCompletion::CompletionMode defaultMode = KCompletion::CompletionPopup;
        if (mode != defaultMode && !d->disableCompletionMap[defaultMode]) {
            subMenu->addSeparator();
            d->defaultAction = subMenu->addAction(tr("Default", "@item:inmenu Text Completion"));
        }
    }

    return popup;
}

void KLineEdit::contextMenuEvent(QContextMenuEvent *e)
{
    if (QLineEdit::contextMenuPolicy() != Qt::DefaultContextMenu) {
        return;
    }
    QMenu *popup = createStandardContextMenu();

    // ### do we really need this?  Yes, Please do not remove!  This
    // allows applications to extend the popup menu without having to
    // inherit from this class! (DA)
    Q_EMIT aboutToShowContextMenu(popup);

    popup->exec(e->globalPos());
    delete popup;
}

void KLineEditPrivate::_k_completionMenuActivated(QAction *act)
{
    Q_Q(KLineEdit);
    KCompletion::CompletionMode oldMode = q->completionMode();

    if (act == noCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionNone);
    } else if (act == shellCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionShell);
    } else if (act == autoCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionAuto);
    } else if (act == popupCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionPopup);
    } else if (act == shortAutoCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionMan);
    } else if (act == popupAutoCompletionAction) {
        q->setCompletionMode(KCompletion::CompletionPopupAuto);
    } else if (act == defaultAction) {
        q->setCompletionMode(KCompletion::CompletionPopup);
    } else {
        return;
    }

    if (oldMode != q->completionMode()) {
        if ((oldMode == KCompletion::CompletionPopup || oldMode == KCompletion::CompletionPopupAuto) //
            && completionBox //
            && completionBox->isVisible()) {
            completionBox->hide();
        }
        Q_EMIT q->completionModeChanged(q->completionMode());
    }
}

bool KLineEdit::event(QEvent *ev)
{
    Q_D(KLineEdit);
    KCursor::autoHideEventFilter(this, ev);
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>(ev);
        if (d->overrideShortcut(e)) {
            ev->accept();
        }
    } else if (ev->type() == QEvent::ApplicationPaletteChange || ev->type() == QEvent::PaletteChange) {
        // Assume the widget uses the application's palette
        QPalette p = QApplication::palette();
        d->previousHighlightedTextColor = p.color(QPalette::Normal, QPalette::HighlightedText);
        d->previousHighlightColor = p.color(QPalette::Normal, QPalette::Highlight);
        setUserSelection(d->userSelection);
    } else if (ev->type() == QEvent::ChildAdded) {
        QObject *obj = static_cast<QChildEvent *>(ev)->child();
        if (obj) {
            connect(obj, &QObject::objectNameChanged, this, [this, obj] {
                if (obj->objectName() == QLatin1String("_q_qlineeditclearaction")) {
                    QAction *action = qobject_cast<QAction *>(obj);
                    connect(action, &QAction::triggered, this, &KLineEdit::clearButtonClicked);
                }
            });
        }
    }

    return QLineEdit::event(ev);
}

bool KLineEdit::urlDropsEnabled() const
{
    Q_D(const KLineEdit);
    return d->handleURLDrops;
}

void KLineEdit::setTrapReturnKey(bool trap)
{
    Q_D(KLineEdit);
    d->trapReturnKeyEvents = trap;
}

bool KLineEdit::trapReturnKey() const
{
    Q_D(const KLineEdit);
    return d->trapReturnKeyEvents;
}

void KLineEdit::setUrl(const QUrl &url)
{
    setText(url.toDisplayString());
}

void KLineEdit::setCompletionBox(KCompletionBox *box)
{
    Q_D(KLineEdit);
    if (d->completionBox) {
        return;
    }

    d->completionBox = box;
    if (handleSignals()) {
        connect(d->completionBox, &KCompletionBox::currentTextChanged, this, [d](const QString &text) {
            d->_k_completionBoxTextChanged(text);
        });

        connect(d->completionBox, &KCompletionBox::userCancelled, this, &KLineEdit::userCancelled);

        connect(d->completionBox, &KCompletionBox::textActivated, this, &KLineEdit::completionBoxActivated);
        connect(d->completionBox, &KCompletionBox::textActivated, this, &KLineEdit::textEdited);
    }
}

/*
 * Set the line edit text without changing the modified flag. By default
 * calling setText resets the modified flag to false.
 */
static void setEditText(KLineEdit *edit, const QString &text)
{
    if (!edit) {
        return;
    }

    const bool wasModified = edit->isModified();
    edit->setText(text);
    edit->setModified(wasModified);
}

void KLineEdit::userCancelled(const QString &cancelText)
{
    Q_D(KLineEdit);
    if (completionMode() != KCompletion::CompletionPopupAuto) {
        setEditText(this, cancelText);
    } else if (hasSelectedText()) {
        if (d->userSelection) {
            deselect();
        } else {
            d->autoSuggest = false;
            const int start = selectionStart();
            const QString s = text().remove(selectionStart(), selectedText().length());
            setEditText(this, s);
            setCursorPosition(start);
            d->autoSuggest = true;
        }
    }
}

bool KLineEditPrivate::overrideShortcut(const QKeyEvent *e)
{
    Q_Q(KLineEdit);
    QList<QKeySequence> scKey;

    const int key = e->key() | e->modifiers();
    const KLineEdit::KeyBindingMap keys = q->keyBindingMap();

    if (keys[KLineEdit::TextCompletion].isEmpty()) {
        scKey = KStandardShortcut::shortcut(KStandardShortcut::TextCompletion);
    } else {
        scKey = keys[KLineEdit::TextCompletion];
    }

    if (scKey.contains(key)) {
        return true;
    }

    if (keys[KLineEdit::NextCompletionMatch].isEmpty()) {
        scKey = KStandardShortcut::shortcut(KStandardShortcut::NextCompletion);
    } else {
        scKey = keys[KLineEdit::NextCompletionMatch];
    }

    if (scKey.contains(key)) {
        return true;
    }

    if (keys[KLineEdit::PrevCompletionMatch].isEmpty()) {
        scKey = KStandardShortcut::shortcut(KStandardShortcut::PrevCompletion);
    } else {
        scKey = keys[KLineEdit::PrevCompletionMatch];
    }

    if (scKey.contains(key)) {
        return true;
    }

    constexpr int ctrlE = QKeyCombination(Qt::CTRL | Qt::Key_E).toCombined();
    constexpr int ctrlU = QKeyCombination(Qt::CTRL | Qt::Key_U).toCombined();

    // Override all the text manupilation accelerators...
    if (KStandardShortcut::copy().contains(key)) {
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        return true;
    } else if (KStandardShortcut::cut().contains(key)) {
        return true;
    } else if (KStandardShortcut::undo().contains(key)) {
        return true;
    } else if (KStandardShortcut::redo().contains(key)) {
        return true;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        return true;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        return true;
    }

    // Shortcut overrides for shortcuts that QLineEdit handles
    // but doesn't dare force as "stronger than kaction shortcuts"...
    else if (e->matches(QKeySequence::SelectAll)) {
        return true;
    } else if (qApp->platformName() == QLatin1String("xcb") && (key == ctrlE || key == ctrlU)) {
        return true;
    }

    if (completionBox && completionBox->isVisible()) {
        const int key = e->key();
        const Qt::KeyboardModifiers modifiers = e->modifiers();
        if ((key == Qt::Key_Backtab || key == Qt::Key_Tab) //
            && (modifiers == Qt::NoModifier || (modifiers & Qt::ShiftModifier))) {
            return true;
        }
    }

    return false;
}

void KLineEdit::setCompletedItems(const QStringList &items, bool autoSuggest)
{
    Q_D(KLineEdit);
    QString txt;
    if (d->completionBox && d->completionBox->isVisible()) {
        // The popup is visible already - do the matching on the initial string,
        // not on the currently selected one.
        txt = completionBox()->cancelledText();
    } else {
        txt = text();
    }

    if (!items.isEmpty() //
        && !(items.count() == 1 && txt == items.first())) {
        // create completion box if non-existent
        completionBox();

        if (d->completionBox->isVisible()) {
            QListWidgetItem *currentItem = d->completionBox->currentItem();

            QString currentSelection;
            if (currentItem != nullptr) {
                currentSelection = currentItem->text();
            }

            d->completionBox->setItems(items);

            const QList<QListWidgetItem *> matchedItems = d->completionBox->findItems(currentSelection, Qt::MatchExactly);
            QListWidgetItem *matchedItem = matchedItems.isEmpty() ? nullptr : matchedItems.first();

            if (matchedItem) {
                const bool blocked = d->completionBox->blockSignals(true);
                d->completionBox->setCurrentItem(matchedItem);
                d->completionBox->blockSignals(blocked);
            } else {
                d->completionBox->setCurrentRow(-1);
            }
        } else { // completion box not visible yet -> show it
            if (!txt.isEmpty()) {
                d->completionBox->setCancelledText(txt);
            }
            d->completionBox->setItems(items);
            d->completionBox->popup();
        }

        if (d->autoSuggest && autoSuggest) {
            const int index = items.first().indexOf(txt);
            const QString newText = items.first().mid(index);
            setUserSelection(false); // can be removed? setCompletedText sets it anyway
            setCompletedText(newText, true);
        }
    } else {
        if (d->completionBox && d->completionBox->isVisible()) {
            d->completionBox->hide();
        }
    }
}

KCompletionBox *KLineEdit::completionBox(bool create)
{
    Q_D(KLineEdit);
    if (create && !d->completionBox) {
        setCompletionBox(new KCompletionBox(this));
        d->completionBox->setObjectName(QStringLiteral("completion box"));
        d->completionBox->setFont(font());
    }

    return d->completionBox;
}

void KLineEdit::setCompletionObject(KCompletion *comp, bool handle)
{
    Q_D(class KLineEdit);

    KCompletion *oldComp = compObj();
    if (oldComp && handleSignals()) {
        disconnect(d->m_matchesConnection);
    }

    if (comp && handle) {
        d->m_matchesConnection = connect(comp, &KCompletion::matches, this, [this](const QStringList &list) {
            setCompletedItems(list);
        });
    }

    KCompletionBase::setCompletionObject(comp, handle);
}

void KLineEdit::setUserSelection(bool userSelection)
{
    Q_D(KLineEdit);
    // if !d->userSelection && userSelection we are accepting a completion,
    // so trigger an update

    if (!d->userSelection && userSelection) {
        d->updateUserText(text());
    }

    QPalette p = palette();

    if (userSelection) {
        p.setColor(QPalette::Highlight, d->previousHighlightColor);
        p.setColor(QPalette::HighlightedText, d->previousHighlightedTextColor);
    } else {
        QColor color = p.color(QPalette::Disabled, QPalette::Text);
        p.setColor(QPalette::HighlightedText, color);
        color = p.color(QPalette::Active, QPalette::Base);
        p.setColor(QPalette::Highlight, color);
    }

    d->userSelection = userSelection;
    setPalette(p);
}

void KLineEditPrivate::_k_restoreSelectionColors()
{
    Q_Q(KLineEdit);
    if (disableRestoreSelection) {
        return;
    }

    q->setUserSelection(true);
}

void KLineEditPrivate::_k_completionBoxTextChanged(const QString &text)
{
    Q_Q(KLineEdit);
    if (!text.isEmpty()) {
        q->setText(text);
        q->setModified(true);
        q->end(false); // force cursor at end
    }
}

QString KLineEdit::originalText() const
{
    Q_D(const KLineEdit);
    if (d->enableSqueezedText && isReadOnly()) {
        return d->squeezedText;
    }

    return text();
}

QString KLineEdit::userText() const
{
    Q_D(const KLineEdit);
    return d->userText;
}

bool KLineEdit::autoSuggest() const
{
    Q_D(const KLineEdit);
    return d->autoSuggest;
}

void KLineEdit::paintEvent(QPaintEvent *ev)
{
    Q_D(KLineEdit);
    if (echoMode() == Password && d->threeStars) {
        // ### hack alert!
        // QLineEdit has currently no hooks to modify the displayed string.
        // When we call setText(), an update() is triggered and we get
        // into an infinite recursion.
        // Qt offers the setUpdatesEnabled() method, but when we re-enable
        // them, update() is triggered, and we get into the same recursion.
        // To work around this problem, we set/clear the internal Qt flag which
        // marks the updatesDisabled state manually.
        setAttribute(Qt::WA_UpdatesDisabled, true);
        blockSignals(true);
        const QString oldText = text();
        const bool isModifiedState = isModified(); // save modified state because setText resets it
        setText(oldText + oldText + oldText);
        QLineEdit::paintEvent(ev);
        setText(oldText);
        setModified(isModifiedState);
        blockSignals(false);
        setAttribute(Qt::WA_UpdatesDisabled, false);
    } else {
        QLineEdit::paintEvent(ev);
    }
}

void KLineEdit::doCompletion(const QString &text)
{
    Q_D(KLineEdit);
    if (emitSignals()) {
        Q_EMIT completion(text); // emit when requested...
    }
    d->completionRunning = true;
    if (handleSignals()) {
        makeCompletion(text); // handle when requested...
    }
    d->completionRunning = false;
}

#include "moc_klineedit.cpp"
