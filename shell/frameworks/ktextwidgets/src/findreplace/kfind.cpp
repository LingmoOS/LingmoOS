/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S .R.Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>
    SPDX-FileCopyrightText: 2004 Arend van Beelen jr. <arend@auton.nl>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfind.h"
#include "kfind_p.h"

#include "kfinddialog.h"

#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>

#include <QDialog>
#include <QDialogButtonBox>
#include <QHash>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

// #define DEBUG_FIND

static const int INDEX_NOMATCH = -1;

class KFindNextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KFindNextDialog(const QString &pattern, QWidget *parent);

    QPushButton *findButton() const;

private:
    QPushButton *m_findButton = nullptr;
};

// Create the dialog.
KFindNextDialog::KFindNextDialog(const QString &pattern, QWidget *parent)
    : QDialog(parent)
{
    setModal(false);
    setWindowTitle(i18n("Find Next"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(i18n("<qt>Find next occurrence of '<b>%1</b>'?</qt>", pattern), this));

    m_findButton = new QPushButton;
    KGuiItem::assign(m_findButton, KStandardGuiItem::find());
    m_findButton->setDefault(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_findButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QPushButton *KFindNextDialog::findButton() const
{
    return m_findButton;
}

////

KFind::KFind(const QString &pattern, long options, QWidget *parent)
    : KFind(*new KFindPrivate(this), pattern, options, parent)
{
}

KFind::KFind(KFindPrivate &dd, const QString &pattern, long options, QWidget *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(KFind);

    d->options = options;
    d->init(pattern);
}

KFind::KFind(const QString &pattern, long options, QWidget *parent, QWidget *findDialog)
    : KFind(*new KFindPrivate(this), pattern, options, parent, findDialog)
{
}

KFind::KFind(KFindPrivate &dd, const QString &pattern, long options, QWidget *parent, QWidget *findDialog)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(KFind);

    d->findDialog = findDialog;
    d->options = options;
    d->init(pattern);
}

void KFindPrivate::init(const QString &_pattern)
{
    Q_Q(KFind);

    matches = 0;
    pattern = _pattern;
    dialog = nullptr;
    dialogClosed = false;
    index = INDEX_NOMATCH;
    lastResult = KFind::NoMatch;

    // TODO: KF6 change this comment once d->regExp is removed
    // set options and create d->regExp with the right options
    q->setOptions(options);
}

KFind::~KFind() = default;

bool KFind::needData() const
{
    Q_D(const KFind);

    // always true when d->text is empty.
    if (d->options & KFind::FindBackwards)
    // d->index==-1 and d->lastResult==Match means we haven't answered nomatch yet
    // This is important in the "replace with a prompt" case.
    {
        return (d->index < 0 && d->lastResult != Match);
    } else
    // "index over length" test removed: we want to get a nomatch before we set data again
    // This is important in the "replace with a prompt" case.
    {
        return d->index == INDEX_NOMATCH;
    }
}

void KFind::setData(const QString &data, int startPos)
{
    setData(-1, data, startPos);
}

void KFind::setData(int id, const QString &data, int startPos)
{
    Q_D(KFind);

    // cache the data for incremental find
    if (d->options & KFind::FindIncremental) {
        if (id != -1) {
            d->customIds = true;
        } else {
            id = d->currentId + 1;
        }

        Q_ASSERT(id <= d->data.size());

        if (id == d->data.size()) {
            d->data.append(KFindPrivate::Data(id, data, true));
        } else {
            d->data.replace(id, KFindPrivate::Data(id, data, true));
        }
        Q_ASSERT(d->data.at(id).text == data);
    }

    if (!(d->options & KFind::FindIncremental) || needData()) {
        d->text = data;

        if (startPos != -1) {
            d->index = startPos;
        } else if (d->options & KFind::FindBackwards) {
            d->index = d->text.length();
        } else {
            d->index = 0;
        }
#ifdef DEBUG_FIND
        // qDebug() << "setData: '" << d->text << "' d->index=" << d->index;
#endif
        Q_ASSERT(d->index != INDEX_NOMATCH);
        d->lastResult = NoMatch;

        d->currentId = id;
    }
}

QDialog *KFind::findNextDialog(bool create)
{
    Q_D(KFind);

    if (!d->dialog && create) {
        KFindNextDialog *dialog = new KFindNextDialog(d->pattern, parentWidget());
        connect(dialog->findButton(), &QPushButton::clicked, this, [d]() {
            d->slotFindNext();
        });
        connect(dialog, &QDialog::finished, this, [d]() {
            d->slotDialogClosed();
        });
        d->dialog = dialog;
    }
    return d->dialog;
}

KFind::Result KFind::find()
{
    Q_D(KFind);

    Q_ASSERT(d->index != INDEX_NOMATCH || d->patternChanged);

    if (d->lastResult == Match && !d->patternChanged) {
        // Move on before looking for the next match, _if_ we just found a match
        if (d->options & KFind::FindBackwards) {
            d->index--;
            if (d->index == -1) { // don't call KFind::find with -1, it has a special meaning
                d->lastResult = NoMatch;
                return NoMatch;
            }
        } else {
            d->index++;
        }
    }
    d->patternChanged = false;

    if (d->options & KFind::FindIncremental) {
        // if the current pattern is shorter than the matchedPattern we can
        // probably look up the match in the incrementalPath
        if (d->pattern.length() < d->matchedPattern.length()) {
            KFindPrivate::Match match;
            if (!d->pattern.isEmpty()) {
                match = d->incrementalPath.value(d->pattern);
            } else if (d->emptyMatch) {
                match = *d->emptyMatch;
            }
            QString previousPattern(d->matchedPattern);
            d->matchedPattern = d->pattern;
            if (!match.isNull()) {
                bool clean = true;

                // find the first result backwards on the path that isn't dirty
                while (d->data.at(match.dataId).dirty == true && !d->pattern.isEmpty()) {
                    d->pattern.truncate(d->pattern.length() - 1);

                    match = d->incrementalPath.value(d->pattern);

                    clean = false;
                }

                // remove all matches that lie after the current match
                while (d->pattern.length() < previousPattern.length()) {
                    d->incrementalPath.remove(previousPattern);
                    previousPattern.truncate(previousPattern.length() - 1);
                }

                // set the current text, index, etc. to the found match
                d->text = d->data.at(match.dataId).text;
                d->index = match.index;
                d->matchedLength = match.matchedLength;
                d->currentId = match.dataId;

                // if the result is clean we can return it now
                if (clean) {
                    if (d->customIds) {
                        Q_EMIT textFoundAtId(d->currentId, d->index, d->matchedLength);
                    } else {
                        Q_EMIT textFound(d->text, d->index, d->matchedLength);
                    }

                    d->lastResult = Match;
                    d->matchedPattern = d->pattern;
                    return Match;
                }
            }
            // if we couldn't look up the match, the new pattern isn't a
            // substring of the matchedPattern, so we start a new search
            else {
                d->startNewIncrementalSearch();
            }
        }
        // if the new pattern is longer than the matchedPattern we might be
        // able to proceed from the last search
        else if (d->pattern.length() > d->matchedPattern.length()) {
            // continue from the previous pattern
            if (d->pattern.startsWith(d->matchedPattern)) {
                // we can't proceed from the previous position if the previous
                // position already failed
                if (d->index == INDEX_NOMATCH) {
                    return NoMatch;
                }

                QString temp(d->pattern);
                d->pattern.truncate(d->matchedPattern.length() + 1);
                d->matchedPattern = temp;
            }
            // start a new search
            else {
                d->startNewIncrementalSearch();
            }
        }
        // if the new pattern is as long as the matchedPattern, we reset if
        // they are not equal
        else if (d->pattern != d->matchedPattern) {
            d->startNewIncrementalSearch();
        }
    }

#ifdef DEBUG_FIND
    // qDebug() << "d->index=" << d->index;
#endif
    do {
        // if we have multiple data blocks in our cache, walk through these
        // blocks till we either searched all blocks or we find a match
        do {
            // Find the next candidate match.
            d->index = KFind::find(d->text, d->pattern, d->index, d->options, &d->matchedLength, nullptr);

            if (d->options & KFind::FindIncremental) {
                d->data[d->currentId].dirty = false;
            }

            if (d->index == -1 && d->currentId < d->data.count() - 1) {
                d->text = d->data.at(++d->currentId).text;

                if (d->options & KFind::FindBackwards) {
                    d->index = d->text.length();
                } else {
                    d->index = 0;
                }
            } else {
                break;
            }
        } while (!(d->options & KFind::RegularExpression));

        if (d->index != -1) {
            // Flexibility: the app can add more rules to validate a possible match
            if (validateMatch(d->text, d->index, d->matchedLength)) {
                bool done = true;

                if (d->options & KFind::FindIncremental) {
                    if (d->pattern.isEmpty()) {
                        delete d->emptyMatch;
                        d->emptyMatch = new KFindPrivate::Match(d->currentId, d->index, d->matchedLength);
                    } else {
                        d->incrementalPath.insert(d->pattern, KFindPrivate::Match(d->currentId, d->index, d->matchedLength));
                    }

                    if (d->pattern.length() < d->matchedPattern.length()) {
                        d->pattern += QStringView(d->matchedPattern).mid(d->pattern.length(), 1);
                        done = false;
                    }
                }

                if (done) {
                    d->matches++;
                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    if (d->customIds) {
                        Q_EMIT textFoundAtId(d->currentId, d->index, d->matchedLength);
                    } else {
                        Q_EMIT textFound(d->text, d->index, d->matchedLength);
                    }

                    if (!d->dialogClosed) {
                        findNextDialog(true)->show();
                    }

#ifdef DEBUG_FIND
                    // qDebug() << "Match. Next d->index=" << d->index;
#endif
                    d->lastResult = Match;
                    return Match;
                }
            } else { // Skip match
                if (d->options & KFind::FindBackwards) {
                    d->index--;
                } else {
                    d->index++;
                }
            }
        } else {
            if (d->options & KFind::FindIncremental) {
                QString temp(d->pattern);
                temp.truncate(temp.length() - 1);
                d->pattern = d->matchedPattern;
                d->matchedPattern = temp;
            }

            d->index = INDEX_NOMATCH;
        }
    } while (d->index != INDEX_NOMATCH);

#ifdef DEBUG_FIND
    // qDebug() << "NoMatch. d->index=" << d->index;
#endif
    d->lastResult = NoMatch;
    return NoMatch;
}

void KFindPrivate::startNewIncrementalSearch()
{
    KFindPrivate::Match *match = emptyMatch;
    if (match == nullptr) {
        text.clear();
        index = 0;
        currentId = 0;
    } else {
        text = data.at(match->dataId).text;
        index = match->index;
        currentId = match->dataId;
    }
    matchedLength = 0;
    incrementalPath.clear();
    delete emptyMatch;
    emptyMatch = nullptr;
    matchedPattern = pattern;
    pattern.clear();
}

static bool isInWord(QChar ch)
{
    return ch.isLetter() || ch.isDigit() || ch == QLatin1Char('_');
}

static bool isWholeWords(const QString &text, int starts, int matchedLength)
{
    if (starts == 0 || !isInWord(text.at(starts - 1))) {
        const int ends = starts + matchedLength;
        if (ends == text.length() || !isInWord(text.at(ends))) {
            return true;
        }
    }
    return false;
}

static bool matchOk(const QString &text, int index, int matchedLength, long options)
{
    if (options & KFind::WholeWordsOnly) {
        // Is the match delimited correctly?
        if (isWholeWords(text, index, matchedLength)) {
            return true;
        }
    } else {
        // Non-whole-word search: this match is good
        return true;
    }
    return false;
}

static int findRegex(const QString &text, const QString &pattern, int index, long options, int *matchedLength, QRegularExpressionMatch *rmatch)
{
    QString _pattern = pattern;

    // Always enable Unicode support in QRegularExpression
    QRegularExpression::PatternOptions opts = QRegularExpression::UseUnicodePropertiesOption;
    // instead of this rudimentary test, add a checkbox to toggle MultilineOption ?
    if (pattern.startsWith(QLatin1Char('^')) || pattern.endsWith(QLatin1Char('$'))) {
        opts |= QRegularExpression::MultilineOption;
    } else if (options & KFind::WholeWordsOnly) { // WholeWordsOnly makes no sense with multiline
        _pattern = QLatin1String("\\b") + pattern + QLatin1String("\\b");
    }

    if (!(options & KFind::CaseSensitive)) {
        opts |= QRegularExpression::CaseInsensitiveOption;
    }

    QRegularExpression re(_pattern, opts);
    QRegularExpressionMatch match;
    if (options & KFind::FindBackwards) {
        // Backward search, until the beginning of the line...
        (void)text.lastIndexOf(re, index, &match);
    } else {
        // Forward search, until the end of the line...
        match = re.match(text, index);
    }

    // index is -1 if no match is found
    index = match.capturedStart(0);
    // matchedLength is 0 if no match is found
    *matchedLength = match.capturedLength(0);

    if (rmatch) {
        *rmatch = match;
    }

    return index;
}

// static
int KFind::find(const QString &text, const QString &pattern, int index, long options, int *matchedLength, QRegularExpressionMatch *rmatch)
{
    // Handle regular expressions in the appropriate way.
    if (options & KFind::RegularExpression) {
        return findRegex(text, pattern, index, options, matchedLength, rmatch);
    }

    // In Qt4 QString("aaaaaa").lastIndexOf("a",6) returns -1; we need
    // to start at text.length() - pattern.length() to give a valid index to QString.
    if (options & KFind::FindBackwards) {
        index = qMin(qMax(0, text.length() - pattern.length()), index);
    }

    Qt::CaseSensitivity caseSensitive = (options & KFind::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;

    if (options & KFind::FindBackwards) {
        // Backward search, until the beginning of the line...
        while (index >= 0) {
            // ...find the next match.
            index = text.lastIndexOf(pattern, index, caseSensitive);
            if (index == -1) {
                break;
            }

            if (matchOk(text, index, pattern.length(), options)) {
                break;
            }
            index--;
            // qDebug() << "decrementing:" << index;
        }
    } else {
        // Forward search, until the end of the line...
        while (index <= text.length()) {
            // ...find the next match.
            index = text.indexOf(pattern, index, caseSensitive);
            if (index == -1) {
                break;
            }

            if (matchOk(text, index, pattern.length(), options)) {
                break;
            }
            index++;
        }
        if (index > text.length()) { // end of line
            // qDebug() << "at" << index << "-> not found";
            index = -1; // not found
        }
    }
    if (index <= -1) {
        *matchedLength = 0;
    } else {
        *matchedLength = pattern.length();
    }
    return index;
}

void KFindPrivate::slotFindNext()
{
    Q_Q(KFind);

    Q_EMIT q->findNext();
}

void KFindPrivate::slotDialogClosed()
{
    Q_Q(KFind);

#ifdef DEBUG_FIND
    // qDebug() << " Begin";
#endif
    Q_EMIT q->dialogClosed();
    dialogClosed = true;
#ifdef DEBUG_FIND
    // qDebug() << " End";
#endif
}

void KFind::displayFinalDialog() const
{
    Q_D(const KFind);

    QString message;
    if (numMatches()) {
        message = i18np("1 match found.", "%1 matches found.", numMatches());
    } else {
        message = i18n("<qt>No matches found for '<b>%1</b>'.</qt>", d->pattern.toHtmlEscaped());
    }
    KMessageBox::information(dialogsParent(), message);
}

bool KFind::shouldRestart(bool forceAsking, bool showNumMatches) const
{
    Q_D(const KFind);

    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if (!forceAsking && (d->options & KFind::FromCursor) == 0) {
        displayFinalDialog();
        return false;
    }
    QString message;
    if (showNumMatches) {
        if (numMatches()) {
            message = i18np("1 match found.", "%1 matches found.", numMatches());
        } else {
            message = i18n("No matches found for '<b>%1</b>'.", d->pattern.toHtmlEscaped());
        }
    } else {
        if (d->options & KFind::FindBackwards) {
            message = i18n("Beginning of document reached.");
        } else {
            message = i18n("End of document reached.");
        }
    }

    message += QLatin1String("<br><br>"); // can't be in the i18n() of the first if() because of the plural form.
    // Hope this word puzzle is ok, it's a different sentence
    message += (d->options & KFind::FindBackwards) ? i18n("Continue from the end?") : i18n("Continue from the beginning?");

    int ret = KMessageBox::questionTwoActions(dialogsParent(),
                                              QStringLiteral("<qt>%1</qt>").arg(message),
                                              QString(),
                                              KStandardGuiItem::cont(),
                                              KStandardGuiItem::stop());
    bool yes = (ret == KMessageBox::PrimaryAction);
    if (yes) {
        const_cast<KFindPrivate *>(d)->options &= ~KFind::FromCursor; // clear FromCursor option
    }
    return yes;
}

long KFind::options() const
{
    Q_D(const KFind);

    return d->options;
}

void KFind::setOptions(long options)
{
    Q_D(KFind);

    d->options = options;
}

void KFind::closeFindNextDialog()
{
    Q_D(KFind);

    if (d->dialog) {
        d->dialog->deleteLater();
        d->dialog = nullptr;
    }
    d->dialogClosed = true;
}

int KFind::index() const
{
    Q_D(const KFind);

    return d->index;
}

QString KFind::pattern() const
{
    Q_D(const KFind);

    return d->pattern;
}

void KFind::setPattern(const QString &pattern)
{
    Q_D(KFind);

    if (d->pattern != pattern) {
        d->patternChanged = true;
        d->matches = 0;
    }

    d->pattern = pattern;

    // TODO: KF6 change this comment once d->regExp is removed
    // set the options and rebuild d->regeExp if necessary
    setOptions(options());
}

int KFind::numMatches() const
{
    Q_D(const KFind);

    return d->matches;
}

void KFind::resetCounts()
{
    Q_D(KFind);

    d->matches = 0;
}

bool KFind::validateMatch(const QString &, int, int)
{
    return true;
}

QWidget *KFind::parentWidget() const
{
    return static_cast<QWidget *>(parent());
}

QWidget *KFind::dialogsParent() const
{
    Q_D(const KFind);

    // If the find dialog is still up, it should get the focus when closing a message box
    // Otherwise, maybe the "find next?" dialog is up
    // Otherwise, the "view" is the parent.
    return d->findDialog ? static_cast<QWidget *>(d->findDialog) : (d->dialog ? d->dialog : parentWidget());
}

#include "kfind.moc"
#include "moc_kfind.cpp"
