/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfinddialog.h"
#include "kfinddialog_p.h"

#include "kfind.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>

#include <KGuiItem>
#include <KHistoryComboBox>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KMessageBox>

#include <assert.h>

KFindDialog::KFindDialog(QWidget *parent, long options, const QStringList &findStrings, bool hasSelection, bool replaceDialog)
    : KFindDialog(*new KFindDialogPrivate(this), parent, options, findStrings, hasSelection, replaceDialog)
{
    setWindowTitle(i18n("Find Text"));
}

KFindDialog::KFindDialog(KFindDialogPrivate &dd, QWidget *parent, long options, const QStringList &findStrings, bool hasSelection, bool replaceDialog)
    : QDialog(parent)
    , d_ptr(&dd)
{
    Q_D(KFindDialog);

    d->init(replaceDialog, findStrings, hasSelection);
    setOptions(options);
}

KFindDialog::~KFindDialog() = default;

QWidget *KFindDialog::findExtension() const
{
    Q_D(const KFindDialog);

    if (!d->findExtension) {
        d->findExtension = new QWidget(d->findGrp);
        d->findLayout->addWidget(d->findExtension, 3, 0, 1, 2);
    }

    return d->findExtension;
}

QStringList KFindDialog::findHistory() const
{
    Q_D(const KFindDialog);

    return d->find->historyItems();
}

void KFindDialogPrivate::init(bool forReplace, const QStringList &_findStrings, bool hasSelection)
{
    Q_Q(KFindDialog);

    // Create common parts of dialog.
    QVBoxLayout *topLayout = new QVBoxLayout(q);

    findGrp = new QGroupBox(i18nc("@title:group", "Find"), q);
    findLayout = new QGridLayout(findGrp);

    QLabel *findLabel = new QLabel(i18n("&Text to find:"), findGrp);
    find = new KHistoryComboBox(findGrp);
    find->setMaxCount(10);
    find->setDuplicatesEnabled(false);
    regExp = new QCheckBox(i18n("Regular e&xpression"), findGrp);
    regExpItem = new QPushButton(i18nc("@action:button", "&Edit…"), findGrp);
    regExpItem->setEnabled(false);

    findLayout->addWidget(findLabel, 0, 0);
    findLayout->addWidget(find, 1, 0, 1, 2);
    findLayout->addWidget(regExp, 2, 0);
    findLayout->addWidget(regExpItem, 2, 1);
    topLayout->addWidget(findGrp);

    replaceGrp = new QGroupBox(i18n("Replace With"), q);
    replaceLayout = new QGridLayout(replaceGrp);

    QLabel *replaceLabel = new QLabel(i18n("Replace&ment text:"), replaceGrp);
    replace = new KHistoryComboBox(replaceGrp);
    replace->setMaxCount(10);
    replace->setDuplicatesEnabled(false);
    backRef = new QCheckBox(i18n("Use p&laceholders"), replaceGrp);
    backRefItem = new QPushButton(i18n("Insert Place&holder"), replaceGrp);
    backRefItem->setEnabled(false);

    replaceLayout->addWidget(replaceLabel, 0, 0);
    replaceLayout->addWidget(replace, 1, 0, 1, 2);
    replaceLayout->addWidget(backRef, 2, 0);
    replaceLayout->addWidget(backRefItem, 2, 1);
    topLayout->addWidget(replaceGrp);

    QGroupBox *optionGrp = new QGroupBox(i18n("Options"), q);
    QGridLayout *optionsLayout = new QGridLayout(optionGrp);

    caseSensitive = new QCheckBox(i18n("C&ase sensitive"), optionGrp);
    wholeWordsOnly = new QCheckBox(i18n("&Whole words only"), optionGrp);
    fromCursor = new QCheckBox(i18n("From c&ursor"), optionGrp);
    findBackwards = new QCheckBox(i18n("Find &backwards"), optionGrp);
    selectedText = new QCheckBox(i18n("&Selected text"), optionGrp);
    q->setHasSelection(hasSelection);
    // If we have a selection, we make 'find in selection' default
    // and if we don't, then the option has to be unchecked, obviously.
    selectedText->setChecked(hasSelection);
    slotSelectedTextToggled(hasSelection);

    promptOnReplace = new QCheckBox(i18n("&Prompt on replace"), optionGrp);
    promptOnReplace->setChecked(true);

    optionsLayout->addWidget(caseSensitive, 0, 0);
    optionsLayout->addWidget(wholeWordsOnly, 1, 0);
    optionsLayout->addWidget(fromCursor, 2, 0);
    optionsLayout->addWidget(findBackwards, 0, 1);
    optionsLayout->addWidget(selectedText, 1, 1);
    optionsLayout->addWidget(promptOnReplace, 2, 1);
    topLayout->addWidget(optionGrp);

    buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, [this]() {
        slotOk();
    });
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, [this]() {
        slotReject();
    });
    topLayout->addWidget(buttonBox);

    // We delay creation of these until needed.
    patterns = nullptr;
    placeholders = nullptr;

    // signals and slots connections
    q->connect(selectedText, &QCheckBox::toggled, q, [this](bool checked) {
        slotSelectedTextToggled(checked);
    });
    q->connect(regExp, &QCheckBox::toggled, regExpItem, &QWidget::setEnabled);
    q->connect(backRef, &QCheckBox::toggled, backRefItem, &QWidget::setEnabled);
    q->connect(regExpItem, &QPushButton::clicked, q, [this]() {
        showPatterns();
    });
    q->connect(backRefItem, &QPushButton::clicked, q, [this]() {
        showPlaceholders();
    });

    q->connect(find, &KHistoryComboBox::editTextChanged, q, [this](const QString &text) {
        textSearchChanged(text);
    });

    q->connect(regExp, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(backRef, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(caseSensitive, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(wholeWordsOnly, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(fromCursor, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(findBackwards, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(selectedText, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);
    q->connect(promptOnReplace, &QCheckBox::toggled, q, &KFindDialog::optionsChanged);

    // tab order
    q->setTabOrder(find, regExp);
    q->setTabOrder(regExp, regExpItem);
    q->setTabOrder(regExpItem, replace); // findExtension widgets are inserted in showEvent()
    q->setTabOrder(replace, backRef);
    q->setTabOrder(backRef, backRefItem);
    q->setTabOrder(backRefItem, caseSensitive);
    q->setTabOrder(caseSensitive, wholeWordsOnly);
    q->setTabOrder(wholeWordsOnly, fromCursor);
    q->setTabOrder(fromCursor, findBackwards);
    q->setTabOrder(findBackwards, selectedText);
    q->setTabOrder(selectedText, promptOnReplace);

    // buddies
    findLabel->setBuddy(find);
    replaceLabel->setBuddy(replace);

    if (!forReplace) {
        promptOnReplace->hide();
        replaceGrp->hide();
    }

    findStrings = _findStrings;
    find->setFocus();
    QPushButton *buttonOk = buttonBox->button(QDialogButtonBox::Ok);
    buttonOk->setEnabled(!q->pattern().isEmpty());

    if (forReplace) {
        KGuiItem::assign(buttonOk,
                         KGuiItem(i18n("&Replace"),
                                  QString(),
                                  i18n("Start replace"),
                                  i18n("<qt>If you press the <b>Replace</b> button, the text you entered "
                                       "above is searched for within the document and any occurrence is "
                                       "replaced with the replacement text.</qt>")));
    } else {
        KGuiItem::assign(buttonOk,
                         KGuiItem(i18n("&Find"),
                                  QStringLiteral("edit-find"),
                                  i18n("Start searching"),
                                  i18n("<qt>If you press the <b>Find</b> button, the text you entered "
                                       "above is searched for within the document.</qt>")));
    }

    // QWhatsthis texts
    find->setWhatsThis(i18n("Enter a pattern to search for, or select a previous pattern from the list."));
    regExp->setWhatsThis(i18n("If enabled, search for a regular expression."));
    regExpItem->setWhatsThis(i18n("Click here to edit your regular expression using a graphical editor."));
    replace->setWhatsThis(i18n("Enter a replacement string, or select a previous one from the list."));
    backRef->setWhatsThis(
        i18n("<qt>If enabled, any occurrence of <code><b>\\N</b></code>, where "
             "<code><b>N</b></code> is an integer number, will be replaced with "
             "the corresponding capture (\"parenthesized substring\") from the "
             "pattern.<p>To include (a literal <code><b>\\N</b></code> in your "
             "replacement, put an extra backslash in front of it, like "
             "<code><b>\\\\N</b></code>.</p></qt>"));
    backRefItem->setWhatsThis(i18n("Click for a menu of available captures."));
    wholeWordsOnly->setWhatsThis(i18n("Require word boundaries in both ends of a match to succeed."));
    fromCursor->setWhatsThis(i18n("Start searching at the current cursor location rather than at the top."));
    selectedText->setWhatsThis(i18n("Only search within the current selection."));
    caseSensitive->setWhatsThis(i18n("Perform a case sensitive search: entering the pattern 'Joe' will not match 'joe' or 'JOE', only 'Joe'."));
    findBackwards->setWhatsThis(i18n("Search backwards."));
    promptOnReplace->setWhatsThis(i18n("Ask before replacing each match found."));

    textSearchChanged(find->lineEdit()->text());
}

void KFindDialogPrivate::textSearchChanged(const QString &text)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void KFindDialog::showEvent(QShowEvent *e)
{
    Q_D(KFindDialog);

    if (!d->initialShowDone) {
        d->initialShowDone = true; // only once
        // qDebug() << "showEvent\n";
        if (!d->findStrings.isEmpty()) {
            setFindHistory(d->findStrings);
        }
        d->findStrings = QStringList();
        if (!d->pattern.isEmpty()) {
            d->find->lineEdit()->setText(d->pattern);
            d->find->lineEdit()->selectAll();
            d->pattern.clear();
        }
        // maintain a user-friendly tab order
        if (d->findExtension) {
            QWidget *prev = d->regExpItem;
            const auto children = d->findExtension->findChildren<QWidget *>();
            for (QWidget *child : children) {
                setTabOrder(prev, child);
                prev = child;
            }
            setTabOrder(prev, d->replace);
        }
    }
    d->find->setFocus();
    QDialog::showEvent(e);
}

long KFindDialog::options() const
{
    Q_D(const KFindDialog);

    long options = 0;

    if (d->caseSensitive->isChecked()) {
        options |= KFind::CaseSensitive;
    }
    if (d->wholeWordsOnly->isChecked()) {
        options |= KFind::WholeWordsOnly;
    }
    if (d->fromCursor->isChecked()) {
        options |= KFind::FromCursor;
    }
    if (d->findBackwards->isChecked()) {
        options |= KFind::FindBackwards;
    }
    if (d->selectedText->isChecked()) {
        options |= KFind::SelectedText;
    }
    if (d->regExp->isChecked()) {
        options |= KFind::RegularExpression;
    }
    return options;
}

QString KFindDialog::pattern() const
{
    Q_D(const KFindDialog);

    return d->find->currentText();
}

void KFindDialog::setPattern(const QString &pattern)
{
    Q_D(KFindDialog);

    d->find->lineEdit()->setText(pattern);
    d->find->lineEdit()->selectAll();
    d->pattern = pattern;
    // qDebug() << "setPattern " << pattern;
}

void KFindDialog::setFindHistory(const QStringList &strings)
{
    Q_D(KFindDialog);

    if (!strings.isEmpty()) {
        d->find->setHistoryItems(strings, true);
        d->find->lineEdit()->setText(strings.first());
        d->find->lineEdit()->selectAll();
    } else {
        d->find->clearHistory();
    }
}

void KFindDialog::setHasSelection(bool hasSelection)
{
    Q_D(KFindDialog);

    if (hasSelection) {
        d->enabled |= KFind::SelectedText;
    } else {
        d->enabled &= ~KFind::SelectedText;
    }
    d->selectedText->setEnabled(hasSelection);
    if (!hasSelection) {
        d->selectedText->setChecked(false);
        d->slotSelectedTextToggled(hasSelection);
    }
}

void KFindDialogPrivate::slotSelectedTextToggled(bool selec)
{
    // From cursor doesn't make sense if we have a selection
    fromCursor->setEnabled(!selec && (enabled & KFind::FromCursor));
    if (selec) { // uncheck if disabled
        fromCursor->setChecked(false);
    }
}

void KFindDialog::setHasCursor(bool hasCursor)
{
    Q_D(KFindDialog);

    if (hasCursor) {
        d->enabled |= KFind::FromCursor;
    } else {
        d->enabled &= ~KFind::FromCursor;
    }
    d->fromCursor->setEnabled(hasCursor);
    d->fromCursor->setChecked(hasCursor && (options() & KFind::FromCursor));
}

void KFindDialog::setSupportsBackwardsFind(bool supports)
{
    Q_D(KFindDialog);

    // ########## Shouldn't this hide the checkbox instead?
    if (supports) {
        d->enabled |= KFind::FindBackwards;
    } else {
        d->enabled &= ~KFind::FindBackwards;
    }
    d->findBackwards->setEnabled(supports);
    d->findBackwards->setChecked(supports && (options() & KFind::FindBackwards));
}

void KFindDialog::setSupportsCaseSensitiveFind(bool supports)
{
    Q_D(KFindDialog);

    // ########## This should hide the checkbox instead
    if (supports) {
        d->enabled |= KFind::CaseSensitive;
    } else {
        d->enabled &= ~KFind::CaseSensitive;
    }
    d->caseSensitive->setEnabled(supports);
    d->caseSensitive->setChecked(supports && (options() & KFind::CaseSensitive));
}

void KFindDialog::setSupportsWholeWordsFind(bool supports)
{
    Q_D(KFindDialog);

    // ########## This should hide the checkbox instead
    if (supports) {
        d->enabled |= KFind::WholeWordsOnly;
    } else {
        d->enabled &= ~KFind::WholeWordsOnly;
    }
    d->wholeWordsOnly->setEnabled(supports);
    d->wholeWordsOnly->setChecked(supports && (options() & KFind::WholeWordsOnly));
}

void KFindDialog::setSupportsRegularExpressionFind(bool supports)
{
    Q_D(KFindDialog);

    if (supports) {
        d->enabled |= KFind::RegularExpression;
    } else {
        d->enabled &= ~KFind::RegularExpression;
    }
    d->regExp->setEnabled(supports);
    d->regExp->setChecked(supports && (options() & KFind::RegularExpression));
    if (!supports) {
        d->regExpItem->hide();
        d->regExp->hide();
    } else {
        d->regExpItem->show();
        d->regExp->show();
    }
}

void KFindDialog::setOptions(long options)
{
    Q_D(KFindDialog);

    d->caseSensitive->setChecked((d->enabled & KFind::CaseSensitive) && (options & KFind::CaseSensitive));
    d->wholeWordsOnly->setChecked((d->enabled & KFind::WholeWordsOnly) && (options & KFind::WholeWordsOnly));
    d->fromCursor->setChecked((d->enabled & KFind::FromCursor) && (options & KFind::FromCursor));
    d->findBackwards->setChecked((d->enabled & KFind::FindBackwards) && (options & KFind::FindBackwards));
    d->selectedText->setChecked((d->enabled & KFind::SelectedText) && (options & KFind::SelectedText));
    d->regExp->setChecked((d->enabled & KFind::RegularExpression) && (options & KFind::RegularExpression));
}

// Create a popup menu with a list of regular expression terms, to help the user
// compose a regular expression search pattern.
void KFindDialogPrivate::showPatterns()
{
    Q_Q(KFindDialog);

    typedef struct {
        const KLazyLocalizedString description;
        const char *regExp;
        int cursorAdjustment;
    } Term;
    static const Term items[] = {
        {kli18n("Any Character"), ".", 0},
        {kli18n("Start of Line"), "^", 0},
        {kli18n("End of Line"), "$", 0},
        {kli18n("Set of Characters"), "[]", -1},
        {kli18n("Repeats, Zero or More Times"), "*", 0},
        {kli18n("Repeats, One or More Times"), "+", 0},
        {kli18n("Optional"), "?", 0},
        {kli18n("Escape"), "\\", 0},
        {kli18n("TAB"), "\\t", 0},
        {kli18n("Newline"), "\\n", 0},
        {kli18n("Carriage Return"), "\\r", 0},
        {kli18n("White Space"), "\\s", 0},
        {kli18n("Digit"), "\\d", 0},
    };

    class RegExpAction : public QAction
    {
    public:
        RegExpAction(QObject *parent, const QString &text, const QString &regExp, int cursor)
            : QAction(text, parent)
            , mText(text)
            , mRegExp(regExp)
            , mCursor(cursor)
        {
        }

        QString text() const
        {
            return mText;
        }
        QString regExp() const
        {
            return mRegExp;
        }
        int cursor() const
        {
            return mCursor;
        }

    private:
        QString mText;
        QString mRegExp;
        int mCursor;
    };

    // Populate the popup menu.
    if (!patterns) {
        patterns = new QMenu(q);
        for (const Term &item : items) {
            patterns->addAction(new RegExpAction(patterns, item.description.toString(), QLatin1String(item.regExp), item.cursorAdjustment));
        }
    }

    // Insert the selection into the edit control.
    QAction *action = patterns->exec(regExpItem->mapToGlobal(regExpItem->rect().bottomLeft()));
    if (action) {
        RegExpAction *regExpAction = static_cast<RegExpAction *>(action);
        if (regExpAction) {
            QLineEdit *editor = find->lineEdit();

            editor->insert(regExpAction->regExp());
            editor->setCursorPosition(editor->cursorPosition() + regExpAction->cursor());
        }
    }
}

class PlaceHolderAction : public QAction
{
public:
    PlaceHolderAction(QObject *parent, const QString &text, int id)
        : QAction(text, parent)
        , mText(text)
        , mId(id)
    {
    }

    QString text() const
    {
        return mText;
    }
    int id() const
    {
        return mId;
    }

private:
    QString mText;
    int mId;
};

// Create a popup menu with a list of backreference terms, to help the user
// compose a regular expression replacement pattern.
void KFindDialogPrivate::showPlaceholders()
{
    Q_Q(KFindDialog);

    // Populate the popup menu.
    if (!placeholders) {
        placeholders = new QMenu(q);
        q->connect(placeholders, &QMenu::aboutToShow, q, [this]() {
            slotPlaceholdersAboutToShow();
        });
    }

    // Insert the selection into the edit control.
    QAction *action = placeholders->exec(backRefItem->mapToGlobal(backRefItem->rect().bottomLeft()));
    if (action) {
        PlaceHolderAction *placeHolderAction = static_cast<PlaceHolderAction *>(action);
        if (placeHolderAction) {
            QLineEdit *editor = replace->lineEdit();
            editor->insert(QStringLiteral("\\%1").arg(placeHolderAction->id()));
        }
    }
}

void KFindDialogPrivate::slotPlaceholdersAboutToShow()
{
    Q_Q(KFindDialog);

    placeholders->clear();
    placeholders->addAction(new PlaceHolderAction(placeholders, i18n("Complete Match"), 0));

    const int n = QRegularExpression(q->pattern(), QRegularExpression::UseUnicodePropertiesOption).captureCount();
    for (int i = 1; i <= n; ++i) {
        placeholders->addAction(new PlaceHolderAction(placeholders, i18n("Captured Text (%1)", i), i));
    }
}

void KFindDialogPrivate::slotOk()
{
    Q_Q(KFindDialog);

    // Nothing to find?
    if (q->pattern().isEmpty()) {
        KMessageBox::error(q, i18n("You must enter some text to search for."));
        return;
    }

    if (regExp->isChecked()) {
        // Check for a valid regular expression.
        if (!QRegularExpression(q->pattern(), QRegularExpression::UseUnicodePropertiesOption).isValid()) {
            KMessageBox::error(q, i18n("Invalid PCRE pattern syntax."));
            return;
        }
    }

    find->addToHistory(q->pattern());

    if (q->windowModality() != Qt::NonModal) {
        q->accept();
    }
    Q_EMIT q->okClicked();
}

void KFindDialogPrivate::slotReject()
{
    Q_Q(KFindDialog);

    Q_EMIT q->cancelClicked();
    q->reject();
}

#include "moc_kfinddialog.cpp"
