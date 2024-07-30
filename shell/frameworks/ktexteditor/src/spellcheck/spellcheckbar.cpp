/*
    SPDX-FileCopyrightText: 2003 Zack Rusin <zack@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spellcheckbar.h"
#include "ui_spellcheckbar.h"
#include <KLocalizedString>

#include "sonnet/backgroundchecker.h"
#include "sonnet/speller.h"
/*
#include "sonnet/filter_p.h"
#include "sonnet/settings_p.h"
*/

#include <QProgressDialog>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>
#include <QTimer>

// to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class ReadOnlyStringListModel : public QStringListModel
{
public:
    ReadOnlyStringListModel(QObject *parent)
        : QStringListModel(parent)
    {
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
};

/**
 * Structure abstracts the word and its position in the
 * parent text.
 *
 * @author Zack Rusin <zack@kde.org>
 * @short struct represents word
 */
struct Word {
    Word()
    {
    }

    Word(const QString &w, int st, bool e = false)
        : word(w)
        , start(st)
        , end(e)
    {
    }
    Word(const Word &other)
        : word(other.word)
        , start(other.start)
        , end(other.end)
    {
    }

    Word &operator=(const Word &) = default;

    QString word;
    int start = 0;
    bool end = true;
};

class SpellCheckBar::Private
{
public:
    Ui_SonnetUi ui;
    ReadOnlyStringListModel *suggestionsModel;
    QWidget *wdg;
    QDialogButtonBox *buttonBox;
    QProgressDialog *progressDialog;
    QString originalBuffer;
    Sonnet::BackgroundChecker *checker;

    Word currentWord;
    std::map<QString, QString> replaceAllMap;
    bool restart; // used when text is distributed across several qtextedits, eg in KAider

    QMap<QString, QString> dictsMap;

    int progressDialogTimeout;
    bool showCompletionMessageBox;
    bool spellCheckContinuedAfterReplacement;
    bool canceled;

    void deleteProgressDialog(bool directly)
    {
        if (progressDialog) {
            progressDialog->hide();
            if (directly) {
                delete progressDialog;
            } else {
                progressDialog->deleteLater();
            }
            progressDialog = nullptr;
        }
    }
};

SpellCheckBar::SpellCheckBar(Sonnet::BackgroundChecker *checker, QWidget *parent)
    : KateViewBarWidget(true, parent)
    , d(new Private)
{
    d->checker = checker;

    d->canceled = false;
    d->showCompletionMessageBox = false;
    d->spellCheckContinuedAfterReplacement = true;
    d->progressDialogTimeout = -1;
    d->progressDialog = nullptr;

    initGui();
    initConnections();
}

SpellCheckBar::~SpellCheckBar()
{
    delete d;
}

void SpellCheckBar::closed()
{
    if (viewBar()) {
        viewBar()->removeBarWidget(this);
    }

    // called from hideMe, so don't call it again!
    d->canceled = true;
    d->deleteProgressDialog(false); // this method can be called in response to
    d->replaceAllMap.clear();
    // pressing 'Cancel' on the dialog
    Q_EMIT cancel();
    Q_EMIT spellCheckStatus(i18n("Spell check canceled."));
}

void SpellCheckBar::initConnections()
{
    connect(d->ui.m_addBtn, &QPushButton::clicked, this, &SpellCheckBar::slotAddWord);
    connect(d->ui.m_replaceBtn, &QPushButton::clicked, this, &SpellCheckBar::slotReplaceWord);
    connect(d->ui.m_replaceAllBtn, &QPushButton::clicked, this, &SpellCheckBar::slotReplaceAll);
    connect(d->ui.m_skipBtn, &QPushButton::clicked, this, &SpellCheckBar::slotSkip);
    connect(d->ui.m_skipAllBtn, &QPushButton::clicked, this, &SpellCheckBar::slotSkipAll);
    connect(d->ui.m_suggestBtn, &QPushButton::clicked, this, &SpellCheckBar::slotSuggest);
    connect(d->ui.m_language, &Sonnet::DictionaryComboBox::textActivated, this, &SpellCheckBar::slotChangeLanguage);
    connect(d->checker, &Sonnet::BackgroundChecker::misspelling, this, &SpellCheckBar::slotMisspelling);
    connect(d->checker, &Sonnet::BackgroundChecker::done, this, &SpellCheckBar::slotDone);
    /*
    connect(d->ui.m_suggestions, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(slotReplaceWord()));
            */

    connect(d->ui.cmbReplacement, &KComboBox::returnPressed, this, &SpellCheckBar::slotReplaceWord);
    connect(d->ui.m_autoCorrect, &QPushButton::clicked, this, &SpellCheckBar::slotAutocorrect);
    // button use by kword/kpresenter
    // hide by default
    d->ui.m_autoCorrect->hide();
}

void SpellCheckBar::initGui()
{
    QVBoxLayout *layout = new QVBoxLayout(centralWidget());
    layout->setContentsMargins(0, 0, 0, 0);

    d->wdg = new QWidget(this);
    d->ui.setupUi(d->wdg);
    layout->addWidget(d->wdg);
    setGuiEnabled(false);

    /*
    d->buttonBox = new QDialogButtonBox(this);
    d->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    layout->addWidget(d->buttonBox);
    */

    // d->ui.m_suggestions->setSorting( NONSORTINGCOLUMN );
    fillDictionaryComboBox();
    d->restart = false;

    d->suggestionsModel = new ReadOnlyStringListModel(this);
    d->ui.cmbReplacement->setModel(d->suggestionsModel);
}

void SpellCheckBar::activeAutoCorrect(bool _active)
{
    if (_active) {
        d->ui.m_autoCorrect->show();
    } else {
        d->ui.m_autoCorrect->hide();
    }
}

void SpellCheckBar::showProgressDialog(int timeout)
{
    d->progressDialogTimeout = timeout;
}

void SpellCheckBar::showSpellCheckCompletionMessage(bool b)
{
    d->showCompletionMessageBox = b;
}

void SpellCheckBar::setSpellCheckContinuedAfterReplacement(bool b)
{
    d->spellCheckContinuedAfterReplacement = b;
}

void SpellCheckBar::slotAutocorrect()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    Q_EMIT autoCorrect(d->currentWord.word, d->ui.cmbReplacement->lineEdit()->text());
    slotReplaceWord();
}

void SpellCheckBar::setGuiEnabled(bool b)
{
    d->wdg->setEnabled(b);
}

void SpellCheckBar::setProgressDialogVisible(bool b)
{
    if (!b) {
        d->deleteProgressDialog(true);
    } else if (d->progressDialogTimeout >= 0) {
        if (d->progressDialog) {
            return;
        }
        d->progressDialog = new QProgressDialog(this);
        d->progressDialog->setLabelText(i18nc("@info:progress", "Spell checking in progress…"));
        d->progressDialog->setWindowTitle(i18nc("@title:window", "Check Spelling"));
        d->progressDialog->setModal(true);
        d->progressDialog->setAutoClose(false);
        d->progressDialog->setAutoReset(false);
        // create an 'indefinite' progress box as we currently cannot get progress feedback from
        // the speller
        d->progressDialog->reset();
        d->progressDialog->setRange(0, 0);
        d->progressDialog->setValue(0);
        connect(d->progressDialog, &QProgressDialog::canceled, this, &SpellCheckBar::slotCancel);
        d->progressDialog->setMinimumDuration(d->progressDialogTimeout);
    }
}

void SpellCheckBar::slotCancel()
{
    hideMe();
}

QString SpellCheckBar::originalBuffer() const
{
    return d->originalBuffer;
}

QString SpellCheckBar::buffer() const
{
    return d->checker->text();
}

void SpellCheckBar::setBuffer(const QString &buf)
{
    d->originalBuffer = buf;
    // it is possible to change buffer inside slot connected to done() signal
    d->restart = true;
}

void SpellCheckBar::fillDictionaryComboBox()
{
    // Since m_language is changed to DictionaryComboBox most code here is gone,
    // So fillDictionaryComboBox() could be removed and code moved to initGui()
    // because the call in show() looks obsolete
    Sonnet::Speller speller = d->checker->speller();
    d->dictsMap = speller.availableDictionaries();

    updateDictionaryComboBox();
}

void SpellCheckBar::updateDictionaryComboBox()
{
    Sonnet::Speller speller = d->checker->speller();
    d->ui.m_language->setCurrentByDictionary(speller.language());
}

void SpellCheckBar::updateDialog(const QString &word)
{
    d->ui.m_unknownWord->setText(word);
    // d->ui.m_contextLabel->setText(d->checker->currentContext());
    const QStringList suggs = d->checker->suggest(word);

    if (suggs.isEmpty()) {
        d->ui.cmbReplacement->lineEdit()->clear();
    } else {
        d->ui.cmbReplacement->lineEdit()->setText(suggs.first());
    }
    fillSuggestions(suggs);
}

void SpellCheckBar::show()
{
    d->canceled = false;
    fillDictionaryComboBox();
    updateDictionaryComboBox();
    if (d->originalBuffer.isEmpty()) {
        d->checker->start();
    } else {
        d->checker->setText(d->originalBuffer);
    }
    setProgressDialogVisible(true);
}

void SpellCheckBar::slotAddWord()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    d->checker->addWordToPersonal(d->currentWord.word);
    d->checker->continueChecking();
}

void SpellCheckBar::slotReplaceWord()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    const QString replacementText = d->ui.cmbReplacement->lineEdit()->text();
    Q_EMIT replace(d->currentWord.word, d->currentWord.start, replacementText);

    if (d->spellCheckContinuedAfterReplacement) {
        d->checker->replace(d->currentWord.start, d->currentWord.word, replacementText);
        d->checker->continueChecking();
    } else {
        setProgressDialogVisible(false);
        d->checker->stop();
    }
}

void SpellCheckBar::slotReplaceAll()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    d->replaceAllMap.insert_or_assign(d->currentWord.word, d->ui.cmbReplacement->lineEdit()->text());
    slotReplaceWord();
}

void SpellCheckBar::slotSkip()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    d->checker->continueChecking();
}

void SpellCheckBar::slotSkipAll()
{
    setGuiEnabled(false);
    setProgressDialogVisible(true);
    // ### do we want that or should we have a d->ignoreAll list?
    Sonnet::Speller speller = d->checker->speller();
    speller.addToPersonal(d->currentWord.word);
    d->checker->setSpeller(speller);
    d->checker->continueChecking();
}

void SpellCheckBar::slotSuggest()
{
    QStringList suggs = d->checker->suggest(d->ui.cmbReplacement->lineEdit()->text());
    fillSuggestions(suggs);
}

void SpellCheckBar::slotChangeLanguage(const QString &lang)
{
    Sonnet::Speller speller = d->checker->speller();
    QString languageCode = d->dictsMap[lang];
    if (!languageCode.isEmpty()) {
        d->checker->changeLanguage(languageCode);
        slotSuggest();
        Q_EMIT languageChanged(languageCode);
    }
}

void SpellCheckBar::fillSuggestions(const QStringList &suggs)
{
    d->suggestionsModel->setStringList(suggs);
    if (!suggs.isEmpty()) {
        d->ui.cmbReplacement->setCurrentIndex(0);
    }
}

void SpellCheckBar::slotMisspelling(const QString &word, int start)
{
    setGuiEnabled(true);
    setProgressDialogVisible(false);
    Q_EMIT misspelling(word, start);
    // NOTE this is HACK I had to introduce because BackgroundChecker lacks 'virtual' marks on methods
    // this dramatically reduces spellchecking time in Lokalize
    // as this doesn't fetch suggestions for words that are present in msgid
    if (!updatesEnabled()) {
        return;
    }

    d->currentWord = Word(word, start);
    if (d->replaceAllMap.find(word) != d->replaceAllMap.end()) {
        d->ui.cmbReplacement->lineEdit()->setText(d->replaceAllMap[word]);
        slotReplaceWord();
    } else {
        updateDialog(word);
    }
}

void SpellCheckBar::slotDone()
{
    d->restart = false;
    Q_EMIT done(d->checker->text());
    if (d->restart) {
        updateDictionaryComboBox();
        d->checker->setText(d->originalBuffer);
        d->restart = false;
    } else {
        setProgressDialogVisible(false);
        Q_EMIT spellCheckStatus(i18n("Spell check complete."));
        hideMe();
        if (!d->canceled && d->showCompletionMessageBox) {
            QMessageBox::information(this, i18n("Spell check complete."), i18nc("@title:window", "Check Spelling"));
        }
    }
}

#include "moc_spellcheckbar.cpp"
