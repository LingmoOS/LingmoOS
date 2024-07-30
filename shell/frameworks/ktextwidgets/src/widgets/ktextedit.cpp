/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2005 Michael Brade <brade@kde.org>
    SPDX-FileCopyrightText: 2012 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktextedit.h"
#include "ktextedit_p.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QTextCursor>

#include <KCursor>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
#include <KStandardShortcut>
#include <sonnet/backgroundchecker.h>
#include <sonnet/configdialog.h>
#include <sonnet/dialog.h>

class KTextDecorator : public Sonnet::SpellCheckDecorator
{
public:
    explicit KTextDecorator(KTextEdit *textEdit);
    bool isSpellCheckingEnabledForBlock(const QString &textBlock) const override;

private:
    KTextEdit *m_textEdit;
};

void KTextEditPrivate::checkSpelling(bool force)
{
    Q_Q(KTextEdit);

    if (q->document()->isEmpty()) {
        KMessageBox::information(q, i18n("Nothing to spell check."));
        if (force) {
            Q_EMIT q->spellCheckingFinished();
        }
        return;
    }
    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    if (!spellCheckingLanguage.isEmpty()) {
        backgroundSpellCheck->changeLanguage(spellCheckingLanguage);
    }
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(backgroundSpellCheck, force ? q : nullptr);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    spellDialog->activeAutoCorrect(showAutoCorrectionButton);
    QObject::connect(spellDialog, &Sonnet::Dialog::replace, q, [this](const QString &oldWord, int pos, const QString &newWord) {
        spellCheckerCorrected(oldWord, pos, newWord);
    });
    QObject::connect(spellDialog, &Sonnet::Dialog::misspelling, q, [this](const QString &text, int pos) {
        spellCheckerMisspelling(text, pos);
    });
    QObject::connect(spellDialog, &Sonnet::Dialog::autoCorrect, q, &KTextEdit::spellCheckerAutoCorrect);
    QObject::connect(spellDialog, &Sonnet::Dialog::spellCheckDone, q, [this]() {
        spellCheckerFinished();
    });
    QObject::connect(spellDialog, &Sonnet::Dialog::cancel, q, [this]() {
        spellCheckerCanceled();
    });

    // Laurent in sonnet/dialog.cpp we emit done(QString) too => it calls here twice spellCheckerFinished not necessary
    // connect(spellDialog, SIGNAL(stop()), q, SLOT(spellCheckerFinished()));

    QObject::connect(spellDialog, &Sonnet::Dialog::spellCheckStatus, q, &KTextEdit::spellCheckStatus);
    QObject::connect(spellDialog, &Sonnet::Dialog::languageChanged, q, &KTextEdit::languageChanged);
    if (force) {
        QObject::connect(spellDialog, &Sonnet::Dialog::spellCheckDone, q, &KTextEdit::spellCheckingFinished);
        QObject::connect(spellDialog, &Sonnet::Dialog::cancel, q, &KTextEdit::spellCheckingCanceled);
        // Laurent in sonnet/dialog.cpp we emit done(QString) too => it calls here twice spellCheckerFinished not necessary
        // connect(spellDialog, SIGNAL(stop()), q, SIGNAL(spellCheckingFinished()));
    }
    originalDoc = QTextDocumentFragment(q->document());
    spellDialog->setBuffer(q->toPlainText());
    spellDialog->show();
}

void KTextEditPrivate::spellCheckerCanceled()
{
    Q_Q(KTextEdit);

    QTextDocument *doc = q->document();
    doc->clear();
    QTextCursor cursor(doc);
    cursor.insertFragment(originalDoc);
    spellCheckerFinished();
}

void KTextEditPrivate::spellCheckerAutoCorrect(const QString &currentWord, const QString &autoCorrectWord)
{
    Q_Q(KTextEdit);

    Q_EMIT q->spellCheckerAutoCorrect(currentWord, autoCorrectWord);
}

void KTextEditPrivate::spellCheckerMisspelling(const QString &text, int pos)
{
    Q_Q(KTextEdit);

    // qDebug()<<"TextEdit::Private::spellCheckerMisspelling :"<<text<<" pos :"<<pos;
    q->highlightWord(text.length(), pos);
}

void KTextEditPrivate::spellCheckerCorrected(const QString &oldWord, int pos, const QString &newWord)
{
    Q_Q(KTextEdit);

    // qDebug()<<" oldWord :"<<oldWord<<" newWord :"<<newWord<<" pos : "<<pos;
    if (oldWord != newWord) {
        QTextCursor cursor(q->document());
        cursor.setPosition(pos);
        cursor.setPosition(pos + oldWord.length(), QTextCursor::KeepAnchor);
        cursor.insertText(newWord);
    }
}

void KTextEditPrivate::spellCheckerFinished()
{
    Q_Q(KTextEdit);

    QTextCursor cursor(q->document());
    cursor.clearSelection();
    q->setTextCursor(cursor);
    if (q->highlighter()) {
        q->highlighter()->rehighlight();
    }
}

void KTextEditPrivate::toggleAutoSpellCheck()
{
    Q_Q(KTextEdit);

    q->setCheckSpellingEnabled(!q->checkSpellingEnabled());
}

void KTextEditPrivate::undoableClear()
{
    Q_Q(KTextEdit);

    QTextCursor cursor = q->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void KTextEditPrivate::slotAllowTab()
{
    Q_Q(KTextEdit);

    q->setTabChangesFocus(!q->tabChangesFocus());
}

void KTextEditPrivate::menuActivated(QAction *action)
{
    Q_Q(KTextEdit);

    if (action == spellCheckAction) {
        q->checkSpelling();
    } else if (action == autoSpellCheckAction) {
        toggleAutoSpellCheck();
    } else if (action == allowTab) {
        slotAllowTab();
    }
}

void KTextEditPrivate::slotFindHighlight(const QString &text, int matchingIndex, int matchingLength)
{
    Q_Q(KTextEdit);

    Q_UNUSED(text)
    // qDebug() << "Highlight: [" << text << "] mi:" << matchingIndex << " ml:" << matchingLength;
    QTextCursor tc = q->textCursor();
    tc.setPosition(matchingIndex);
    tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, matchingLength);
    q->setTextCursor(tc);
    q->ensureCursorVisible();
}

void KTextEditPrivate::slotReplaceText(const QString &text, int replacementIndex, int replacedLength, int matchedLength)
{
    Q_Q(KTextEdit);

    // qDebug() << "Replace: [" << text << "] ri:" << replacementIndex << " rl:" << replacedLength << " ml:" << matchedLength;
    QTextCursor tc = q->textCursor();
    tc.setPosition(replacementIndex);
    tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, matchedLength);
    tc.removeSelectedText();
    tc.insertText(text.mid(replacementIndex, replacedLength));
    if (replace->options() & KReplaceDialog::PromptOnReplace) {
        q->setTextCursor(tc);
        q->ensureCursorVisible();
    }
    lastReplacedPosition = replacementIndex;
}

void KTextEditPrivate::init()
{
    Q_Q(KTextEdit);

    KCursor::setAutoHideCursor(q, true, false);
    q->connect(q, &KTextEdit::languageChanged, q, &KTextEdit::setSpellCheckingLanguage);
}

KTextDecorator::KTextDecorator(KTextEdit *textEdit)
    : SpellCheckDecorator(textEdit)
    , m_textEdit(textEdit)
{
}

bool KTextDecorator::isSpellCheckingEnabledForBlock(const QString &textBlock) const
{
    return m_textEdit->shouldBlockBeSpellChecked(textBlock);
}

KTextEdit::KTextEdit(const QString &text, QWidget *parent)
    : KTextEdit(*new KTextEditPrivate(this), text, parent)
{
}

KTextEdit::KTextEdit(KTextEditPrivate &dd, const QString &text, QWidget *parent)
    : QTextEdit(text, parent)
    , d_ptr(&dd)
{
    Q_D(KTextEdit);

    d->init();
}

KTextEdit::KTextEdit(QWidget *parent)
    : KTextEdit(*new KTextEditPrivate(this), parent)
{
}

KTextEdit::KTextEdit(KTextEditPrivate &dd, QWidget *parent)
    : QTextEdit(parent)
    , d_ptr(&dd)
{
    Q_D(KTextEdit);

    d->init();
}

KTextEdit::~KTextEdit() = default;

const QString &KTextEdit::spellCheckingLanguage() const
{
    Q_D(const KTextEdit);

    return d->spellCheckingLanguage;
}

void KTextEdit::setSpellCheckingLanguage(const QString &_language)
{
    Q_D(KTextEdit);

    if (highlighter()) {
        highlighter()->setCurrentLanguage(_language);
        highlighter()->rehighlight();
    }

    if (_language != d->spellCheckingLanguage) {
        d->spellCheckingLanguage = _language;
        Q_EMIT languageChanged(_language);
    }
}

void KTextEdit::showSpellConfigDialog(const QString &windowIcon)
{
    Q_D(KTextEdit);

    Sonnet::ConfigDialog dialog(this);
    if (!d->spellCheckingLanguage.isEmpty()) {
        dialog.setLanguage(d->spellCheckingLanguage);
    }
    if (!windowIcon.isEmpty()) {
        dialog.setWindowIcon(QIcon::fromTheme(windowIcon, dialog.windowIcon()));
    }
    if (dialog.exec()) {
        setSpellCheckingLanguage(dialog.language());
    }
}

bool KTextEdit::event(QEvent *ev)
{
    Q_D(KTextEdit);

    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>(ev);
        if (d->overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return QTextEdit::event(ev);
}

bool KTextEditPrivate::handleShortcut(const QKeyEvent *event)
{
    Q_Q(KTextEdit);

    const int key = event->key() | event->modifiers();

    if (KStandardShortcut::copy().contains(key)) {
        q->copy();
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        q->paste();
        return true;
    } else if (KStandardShortcut::cut().contains(key)) {
        q->cut();
        return true;
    } else if (KStandardShortcut::undo().contains(key)) {
        if (!q->isReadOnly()) {
            q->undo();
        }
        return true;
    } else if (KStandardShortcut::redo().contains(key)) {
        if (!q->isReadOnly()) {
            q->redo();
        }
        return true;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        if (!q->isReadOnly()) {
            q->deleteWordBack();
        }
        return true;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        if (!q->isReadOnly()) {
            q->deleteWordForward();
        }
        return true;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        QTextCursor cursor = q->textCursor();
        // We use visual positioning here since keyboard arrows represents visual direction (left, right)
        cursor.movePosition(QTextCursor::WordLeft);
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        QTextCursor cursor = q->textCursor();
        // We use visual positioning here since keyboard arrows represents visual direction (left, right)
        cursor.movePosition(QTextCursor::WordRight);
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::next().contains(key)) {
        QTextCursor cursor = q->textCursor();
        bool moved = false;
        qreal lastY = q->cursorRect(cursor).bottom();
        qreal distance = 0;
        do {
            qreal y = q->cursorRect(cursor).bottom();
            distance += qAbs(y - lastY);
            lastY = y;
            moved = cursor.movePosition(QTextCursor::Down);
        } while (moved && distance < q->viewport()->height());

        if (moved) {
            cursor.movePosition(QTextCursor::Up);
            q->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
        }
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::prior().contains(key)) {
        QTextCursor cursor = q->textCursor();
        bool moved = false;
        qreal lastY = q->cursorRect(cursor).bottom();
        qreal distance = 0;
        do {
            qreal y = q->cursorRect(cursor).bottom();
            distance += qAbs(y - lastY);
            lastY = y;
            moved = cursor.movePosition(QTextCursor::Up);
        } while (moved && distance < q->viewport()->height());

        if (moved) {
            cursor.movePosition(QTextCursor::Down);
            q->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
        }
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::begin().contains(key)) {
        QTextCursor cursor = q->textCursor();
        cursor.movePosition(QTextCursor::Start);
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::end().contains(key)) {
        QTextCursor cursor = q->textCursor();
        cursor.movePosition(QTextCursor::End);
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        QTextCursor cursor = q->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        q->setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        QTextCursor cursor = q->textCursor();
        cursor.movePosition(QTextCursor::EndOfLine);
        q->setTextCursor(cursor);
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::find().contains(key)) {
        q->slotFind();
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::findNext().contains(key)) {
        q->slotFindNext();
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::findPrev().contains(key)) {
        q->slotFindPrevious();
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::replace().contains(key)) {
        if (!q->isReadOnly()) {
            q->slotReplace();
        }
        return true;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        QString text = QApplication::clipboard()->text(QClipboard::Selection);
        if (!text.isEmpty()) {
            q->insertPlainText(text); // TODO: check if this is html? (MiB)
        }
        return true;
    }
    return false;
}

static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition(op, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void KTextEdit::deleteWordBack()
{
    // We use logical positioning here since deleting should always delete the previous word
    // (left in case of LTR text, right in case of RTL text)
    deleteWord(textCursor(), QTextCursor::PreviousWord);
}

void KTextEdit::deleteWordForward()
{
    // We use logical positioning here since deleting should always delete the previous word
    // (left in case of LTR text, right in case of RTL text)
    deleteWord(textCursor(), QTextCursor::NextWord);
}

QMenu *KTextEdit::mousePopupMenu()
{
    Q_D(KTextEdit);

    QMenu *popup = createStandardContextMenu();
    if (!popup) {
        return nullptr;
    }
    connect(popup, &QMenu::triggered, this, [d](QAction *action) {
        d->menuActivated(action);
    });

    const bool emptyDocument = document()->isEmpty();
    if (!isReadOnly()) {
        QList<QAction *> actionList = popup->actions();
        enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
        QAction *separatorAction = nullptr;
        int idx = actionList.indexOf(actionList[SelectAllAct]) + 1;
        if (idx < actionList.count()) {
            separatorAction = actionList.at(idx);
        }

        auto undoableClearSlot = [d]() {
            d->undoableClear();
        };

        if (separatorAction) {
            QAction *clearAllAction = KStandardAction::clear(this, undoableClearSlot, popup);
            if (emptyDocument) {
                clearAllAction->setEnabled(false);
            }
            popup->insertAction(separatorAction, clearAllAction);
        }
    }

    if (!isReadOnly()) {
        popup->addSeparator();
        if (!d->speller) {
            d->speller = new Sonnet::Speller();
        }
        if (!d->speller->availableBackends().isEmpty()) {
            d->spellCheckAction = popup->addAction(QIcon::fromTheme(QStringLiteral("tools-check-spelling")), i18nc("@action:inmenu", "Check Spelling…"));
            if (emptyDocument) {
                d->spellCheckAction->setEnabled(false);
            }
            if (checkSpellingEnabled()) {
                d->languagesMenu = new QMenu(i18n("Spell Checking Language"), popup);
                QActionGroup *languagesGroup = new QActionGroup(d->languagesMenu);
                languagesGroup->setExclusive(true);

                QMapIterator<QString, QString> i(d->speller->availableDictionaries());
                const QString language = spellCheckingLanguage();
                while (i.hasNext()) {
                    i.next();

                    QAction *languageAction = d->languagesMenu->addAction(i.key());
                    languageAction->setCheckable(true);
                    languageAction->setChecked(language == i.value() || (language.isEmpty() && d->speller->defaultLanguage() == i.value()));
                    languageAction->setData(i.value());
                    languageAction->setActionGroup(languagesGroup);
                    connect(languageAction, &QAction::triggered, [this, languageAction]() {
                        setSpellCheckingLanguage(languageAction->data().toString());
                    });
                }
                popup->addMenu(d->languagesMenu);
            }

            d->autoSpellCheckAction = popup->addAction(i18n("Auto Spell Check"));
            d->autoSpellCheckAction->setCheckable(true);
            d->autoSpellCheckAction->setChecked(checkSpellingEnabled());
            popup->addSeparator();
        }
        if (d->showTabAction) {
            d->allowTab = popup->addAction(i18n("Allow Tabulations"));
            d->allowTab->setCheckable(true);
            d->allowTab->setChecked(!tabChangesFocus());
        }
    }

    if (d->findReplaceEnabled) {
        QAction *findAction = KStandardAction::find(this, &KTextEdit::slotFind, popup);
        QAction *findNextAction = KStandardAction::findNext(this, &KTextEdit::slotFindNext, popup);
        QAction *findPrevAction = KStandardAction::findPrev(this, &KTextEdit::slotFindPrevious, popup);
        if (emptyDocument) {
            findAction->setEnabled(false);
            findNextAction->setEnabled(false);
            findPrevAction->setEnabled(false);
        } else {
            findNextAction->setEnabled(d->find != nullptr);
            findPrevAction->setEnabled(d->find != nullptr);
        }
        popup->addSeparator();
        popup->addAction(findAction);
        popup->addAction(findNextAction);
        popup->addAction(findPrevAction);

        if (!isReadOnly()) {
            QAction *replaceAction = KStandardAction::replace(this, &KTextEdit::slotReplace, popup);
            if (emptyDocument) {
                replaceAction->setEnabled(false);
            }
            popup->addAction(replaceAction);
        }
    }
#ifdef HAVE_SPEECH
    popup->addSeparator();
    QAction *speakAction = popup->addAction(i18n("Speak Text"));
    speakAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
    speakAction->setEnabled(!emptyDocument);
    connect(speakAction, &QAction::triggered, this, &KTextEdit::slotSpeakText);
#endif
    return popup;
}

void KTextEdit::slotSpeakText()
{
    Q_D(KTextEdit);

#ifdef HAVE_SPEECH
    QString text;
    if (textCursor().hasSelection()) {
        text = textCursor().selectedText();
    } else {
        text = toPlainText();
    }
    if (!d->textToSpeech) {
        d->textToSpeech = new QTextToSpeech(this);
    }
    d->textToSpeech->say(text);
#endif
}

void KTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = mousePopupMenu();
    if (popup) {
        aboutToShowContextMenu(popup);
        popup->exec(event->globalPos());
        delete popup;
    }
}

void KTextEdit::createHighlighter()
{
    setHighlighter(new Sonnet::Highlighter(this));
}

Sonnet::Highlighter *KTextEdit::highlighter() const
{
    Q_D(const KTextEdit);

    if (d->decorator) {
        return d->decorator->highlighter();
    } else {
        return nullptr;
    }
}

void KTextEdit::clearDecorator()
{
    Q_D(KTextEdit);

    // Set pointer to null before deleting KTextDecorator as dtor will emit signal,
    // which could call this code again and cause double delete/crash
    auto decorator = d->decorator;
    d->decorator = nullptr;
    delete decorator;
}

void KTextEdit::addTextDecorator(Sonnet::SpellCheckDecorator *decorator)
{
    Q_D(KTextEdit);

    d->decorator = decorator;
}

void KTextEdit::setHighlighter(Sonnet::Highlighter *_highLighter)
{
    KTextDecorator *decorator = new KTextDecorator(this);
    // The old default highlighter must be manually deleted.
    delete decorator->highlighter();
    decorator->setHighlighter(_highLighter);

    // KTextEdit used to take ownership of the highlighter, Sonnet::SpellCheckDecorator does not.
    // so we reparent the highlighter so it will be deleted when the decorator is destroyed
    _highLighter->setParent(decorator);
    addTextDecorator(decorator);
}

void KTextEdit::setCheckSpellingEnabled(bool check)
{
    Q_D(KTextEdit);

    Q_EMIT checkSpellingChanged(check);
    if (check == d->spellCheckingEnabled) {
        return;
    }

    // From the above statement we now know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->spellCheckingEnabled = check;
    if (check) {
        if (hasFocus()) {
            createHighlighter();
            if (!spellCheckingLanguage().isEmpty()) {
                setSpellCheckingLanguage(spellCheckingLanguage());
            }
        }
    } else {
        clearDecorator();
    }
}

void KTextEdit::focusInEvent(QFocusEvent *event)
{
    Q_D(KTextEdit);

    if (d->spellCheckingEnabled && !isReadOnly() && !d->decorator) {
        createHighlighter();
    }

    QTextEdit::focusInEvent(event);
}

bool KTextEdit::checkSpellingEnabled() const
{
    Q_D(const KTextEdit);

    return d->spellCheckingEnabled;
}

bool KTextEdit::shouldBlockBeSpellChecked(const QString &) const
{
    return true;
}

void KTextEdit::setReadOnly(bool readOnly)
{
    Q_D(KTextEdit);

    if (!readOnly && hasFocus() && d->spellCheckingEnabled && !d->decorator) {
        createHighlighter();
    }

    if (readOnly == isReadOnly()) {
        return;
    }

    if (readOnly) {
        // Set pointer to null before deleting KTextDecorator as dtor will emit signal,
        // which could call this code again and cause double delete/crash
        auto decorator = d->decorator;
        d->decorator = nullptr;
        delete decorator;

        d->customPalette = testAttribute(Qt::WA_SetPalette);
        QPalette p = palette();
        QColor color = p.color(QPalette::Disabled, QPalette::Window);
        p.setColor(QPalette::Base, color);
        p.setColor(QPalette::Window, color);
        setPalette(p);
    } else {
        if (d->customPalette && testAttribute(Qt::WA_SetPalette)) {
            QPalette p = palette();
            QColor color = p.color(QPalette::Normal, QPalette::Base);
            p.setColor(QPalette::Base, color);
            p.setColor(QPalette::Window, color);
            setPalette(p);
        } else {
            setPalette(QPalette());
        }
    }

    QTextEdit::setReadOnly(readOnly);
}

void KTextEdit::checkSpelling()
{
    Q_D(KTextEdit);

    d->checkSpelling(false);
}

void KTextEdit::forceSpellChecking()
{
    Q_D(KTextEdit);

    d->checkSpelling(true);
}

void KTextEdit::highlightWord(int length, int pos)
{
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(pos + length, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

void KTextEdit::replace()
{
    Q_D(KTextEdit);

    if (document()->isEmpty()) { // saves having to track the text changes
        return;
    }

    if (d->repDlg) {
        d->repDlg->activateWindow();
    } else {
        d->repDlg = new KReplaceDialog(this, 0, QStringList(), QStringList(), false);
        connect(d->repDlg, &KFindDialog::okClicked, this, &KTextEdit::slotDoReplace);
    }
    d->repDlg->show();
}

void KTextEdit::slotDoReplace()
{
    Q_D(KTextEdit);

    if (!d->repDlg) {
        // Should really assert()
        return;
    }

    if (d->repDlg->pattern().isEmpty()) {
        delete d->replace;
        d->replace = nullptr;
        ensureCursorVisible();
        return;
    }

    delete d->replace;
    d->replace = new KReplace(d->repDlg->pattern(), d->repDlg->replacement(), d->repDlg->options(), this);
    d->repIndex = 0;
    if (d->replace->options() & KFind::FromCursor || d->replace->options() & KFind::FindBackwards) {
        d->repIndex = textCursor().anchor();
    }

    // Connect textFound signal to code which handles highlighting of found text.
    connect(d->replace, &KFind::textFound, this, [d](const QString &text, int matchingIndex, int matchedLength) {
        d->slotFindHighlight(text, matchingIndex, matchedLength);
    });
    connect(d->replace, &KFind::findNext, this, &KTextEdit::slotReplaceNext);

    connect(d->replace, &KReplace::textReplaced, this, [d](const QString &text, int replacementIndex, int replacedLength, int matchedLength) {
        d->slotReplaceText(text, replacementIndex, replacedLength, matchedLength);
    });

    d->repDlg->close();
    slotReplaceNext();
}

void KTextEdit::slotReplaceNext()
{
    Q_D(KTextEdit);

    if (!d->replace) {
        return;
    }

    d->lastReplacedPosition = -1;
    if (!(d->replace->options() & KReplaceDialog::PromptOnReplace)) {
        textCursor().beginEditBlock(); // #48541
        viewport()->setUpdatesEnabled(false);
    }

    if (d->replace->needData()) {
        d->replace->setData(toPlainText(), d->repIndex);
    }
    KFind::Result res = d->replace->replace();
    if (!(d->replace->options() & KReplaceDialog::PromptOnReplace)) {
        textCursor().endEditBlock(); // #48541
        if (d->lastReplacedPosition >= 0) {
            QTextCursor tc = textCursor();
            tc.setPosition(d->lastReplacedPosition);
            setTextCursor(tc);
            ensureCursorVisible();
        }

        viewport()->setUpdatesEnabled(true);
        viewport()->update();
    }

    if (res == KFind::NoMatch) {
        d->replace->displayFinalDialog();
        d->replace->disconnect(this);
        d->replace->deleteLater(); // we are in a slot connected to m_replace, don't delete it right away
        d->replace = nullptr;
        ensureCursorVisible();
        // or           if ( m_replace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
    } else {
        // m_replace->closeReplaceNextDialog();
    }
}

void KTextEdit::slotDoFind()
{
    Q_D(KTextEdit);

    if (!d->findDlg) {
        // Should really assert()
        return;
    }
    if (d->findDlg->pattern().isEmpty()) {
        delete d->find;
        d->find = nullptr;
        return;
    }
    delete d->find;
    d->find = new KFind(d->findDlg->pattern(), d->findDlg->options(), this);
    d->findIndex = 0;
    if (d->find->options() & KFind::FromCursor || d->find->options() & KFind::FindBackwards) {
        d->findIndex = textCursor().anchor();
    }

    // Connect textFound() signal to code which handles highlighting of found text
    connect(d->find, &KFind::textFound, this, [d](const QString &text, int matchingIndex, int matchedLength) {
        d->slotFindHighlight(text, matchingIndex, matchedLength);
    });
    connect(d->find, &KFind::findNext, this, &KTextEdit::slotFindNext);

    d->findDlg->close();
    d->find->closeFindNextDialog();
    slotFindNext();
}

void KTextEdit::slotFindNext()
{
    Q_D(KTextEdit);

    if (!d->find) {
        return;
    }
    if (document()->isEmpty()) {
        d->find->disconnect(this);
        d->find->deleteLater(); // we are in a slot connected to m_find, don't delete right away
        d->find = nullptr;
        return;
    }

    if (d->find->needData()) {
        d->find->setData(toPlainText(), d->findIndex);
    }
    KFind::Result res = d->find->find();

    if (res == KFind::NoMatch) {
        d->find->displayFinalDialog();
        d->find->disconnect(this);
        d->find->deleteLater(); // we are in a slot connected to m_find, don't delete right away
        d->find = nullptr;
        // or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
    } else {
        // m_find->closeFindNextDialog();
    }
}

void KTextEdit::slotFindPrevious()
{
    Q_D(KTextEdit);

    if (!d->find) {
        return;
    }
    const long oldOptions = d->find->options();
    d->find->setOptions(oldOptions ^ KFind::FindBackwards);
    slotFindNext();
    if (d->find) {
        d->find->setOptions(oldOptions);
    }
}

void KTextEdit::slotFind()
{
    Q_D(KTextEdit);

    if (document()->isEmpty()) { // saves having to track the text changes
        return;
    }

    if (d->findDlg) {
        d->findDlg->activateWindow();
    } else {
        d->findDlg = new KFindDialog(this);
        connect(d->findDlg, &KFindDialog::okClicked, this, &KTextEdit::slotDoFind);
    }
    d->findDlg->show();
}

void KTextEdit::slotReplace()
{
    Q_D(KTextEdit);

    if (document()->isEmpty()) { // saves having to track the text changes
        return;
    }

    if (d->repDlg) {
        d->repDlg->activateWindow();
    } else {
        d->repDlg = new KReplaceDialog(this, 0, QStringList(), QStringList(), false);
        connect(d->repDlg, &KFindDialog::okClicked, this, &KTextEdit::slotDoReplace);
    }
    d->repDlg->show();
}

void KTextEdit::enableFindReplace(bool enabled)
{
    Q_D(KTextEdit);

    d->findReplaceEnabled = enabled;
}

void KTextEdit::showTabAction(bool show)
{
    Q_D(KTextEdit);

    d->showTabAction = show;
}

bool KTextEditPrivate::overrideShortcut(const QKeyEvent *event)
{
    const int key = event->key() | event->modifiers();

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
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::next().contains(key)) {
        return true;
    } else if (KStandardShortcut::prior().contains(key)) {
        return true;
    } else if (KStandardShortcut::begin().contains(key)) {
        return true;
    } else if (KStandardShortcut::end().contains(key)) {
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        return true;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::find().contains(key)) {
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::findNext().contains(key)) {
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::findPrev().contains(key)) {
        return true;
    } else if (findReplaceEnabled && KStandardShortcut::replace().contains(key)) {
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) { // currently missing in QTextEdit
        return true;
    }
    return false;
}

void KTextEdit::keyPressEvent(QKeyEvent *event)
{
    Q_D(KTextEdit);

    if (d->handleShortcut(event)) {
        event->accept();
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

void KTextEdit::showAutoCorrectButton(bool show)
{
    Q_D(KTextEdit);

    d->showAutoCorrectionButton = show;
}

#include "moc_ktextedit.cpp"
