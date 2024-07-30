/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kreplace.h"

#include "kfind_p.h"
#include "kreplacedialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

//#define DEBUG_REPLACE
#define INDEX_NOMATCH -1

class KReplaceNextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KReplaceNextDialog(QWidget *parent);
    void setLabel(const QString &pattern, const QString &replacement);

    QPushButton *replaceAllButton() const;
    QPushButton *skipButton() const;
    QPushButton *replaceButton() const;

private:
    QLabel *m_mainLabel = nullptr;
    QPushButton *m_allButton = nullptr;
    QPushButton *m_skipButton = nullptr;
    QPushButton *m_replaceButton = nullptr;
};

KReplaceNextDialog::KReplaceNextDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(false);
    setWindowTitle(i18n("Replace"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_mainLabel = new QLabel(this);
    layout->addWidget(m_mainLabel);

    m_allButton = new QPushButton(i18nc("@action:button Replace all occurrences", "&All"));
    m_allButton->setObjectName(QStringLiteral("allButton"));
    m_skipButton = new QPushButton(i18n("&Skip"));
    m_skipButton->setObjectName(QStringLiteral("skipButton"));
    m_replaceButton = new QPushButton(i18n("Replace"));
    m_replaceButton->setObjectName(QStringLiteral("replaceButton"));
    m_replaceButton->setDefault(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_allButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_skipButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_replaceButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void KReplaceNextDialog::setLabel(const QString &pattern, const QString &replacement)
{
    m_mainLabel->setText(i18n("Replace '%1' with '%2'?", pattern, replacement));
}

QPushButton *KReplaceNextDialog::replaceAllButton() const
{
    return m_allButton;
}

QPushButton *KReplaceNextDialog::skipButton() const
{
    return m_skipButton;
}

QPushButton *KReplaceNextDialog::replaceButton() const
{
    return m_replaceButton;
}

////

class KReplacePrivate : public KFindPrivate
{
    Q_DECLARE_PUBLIC(KReplace)

public:
    KReplacePrivate(KReplace *q, const QString &replacement)
        : KFindPrivate(q)
        , m_replacement(replacement)
    {
    }

    KReplaceNextDialog *nextDialog();
    void doReplace();

    void slotSkip();
    void slotReplace();
    void slotReplaceAll();

    QString m_replacement;
    int m_replacements = 0;
    QRegularExpressionMatch m_match;
};

////

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent)
    : KFind(*new KReplacePrivate(this, replacement), pattern, options, parent)
{
}

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent, QWidget *dlg)
    : KFind(*new KReplacePrivate(this, replacement), pattern, options, parent, dlg)
{
}

KReplace::~KReplace() = default;

int KReplace::numReplacements() const
{
    Q_D(const KReplace);

    return d->m_replacements;
}

QDialog *KReplace::replaceNextDialog(bool create)
{
    Q_D(KReplace);

    if (d->dialog || create) {
        return d->nextDialog();
    }
    return nullptr;
}

KReplaceNextDialog *KReplacePrivate::nextDialog()
{
    Q_Q(KReplace);

    if (!dialog) {
        auto *nextDialog = new KReplaceNextDialog(q->parentWidget());
        q->connect(nextDialog->replaceAllButton(), &QPushButton::clicked, q, [this]() {
            slotReplaceAll();
        });
        q->connect(nextDialog->skipButton(), &QPushButton::clicked, q, [this]() {
            slotSkip();
        });
        q->connect(nextDialog->replaceButton(), &QPushButton::clicked, q, [this]() {
            slotReplace();
        });
        q->connect(nextDialog, &QDialog::finished, q, [this]() {
            slotDialogClosed();
        });
        dialog = nextDialog;
    }
    return static_cast<KReplaceNextDialog *>(dialog);
}

void KReplace::displayFinalDialog() const
{
    Q_D(const KReplace);

    if (!d->m_replacements) {
        KMessageBox::information(parentWidget(), i18n("No text was replaced."));
    } else {
        KMessageBox::information(parentWidget(), i18np("1 replacement done.", "%1 replacements done.", d->m_replacements));
    }
}

static int replaceHelper(QString &text, const QString &replacement, int index, long options, const QRegularExpressionMatch *match, int length)
{
    QString rep(replacement);
    if (options & KReplaceDialog::BackReference) {
        // Handle backreferences
        if (options & KFind::RegularExpression) { // regex search
            Q_ASSERT(match);
            const int capNum = match->regularExpression().captureCount();
            for (int i = 0; i <= capNum; ++i) {
                rep.replace(QLatin1String("\\") + QString::number(i), match->captured(i));
            }
        } else { // with non-regex search only \0 is supported, replace it with the
                 // right portion of 'text'
            rep.replace(QLatin1String("\\0"), text.mid(index, length));
        }
    }

    // Then replace rep into the text
    text.replace(index, length, rep);
    return rep.length();
}

KFind::Result KReplace::replace()
{
    Q_D(KReplace);

#ifdef DEBUG_REPLACE
    // qDebug() << "d->index=" << d->index;
#endif
    if (d->index == INDEX_NOMATCH && d->lastResult == Match) {
        d->lastResult = NoMatch;
        return NoMatch;
    }

    do { // this loop is only because validateMatch can fail
#ifdef DEBUG_REPLACE
         // qDebug() << "beginning of loop: d->index=" << d->index;
#endif
         // Find the next match.
        d->index = KFind::find(d->text, d->pattern, d->index, d->options, &d->matchedLength, d->options & KFind::RegularExpression ? &d->m_match : nullptr);

#ifdef DEBUG_REPLACE
        // qDebug() << "KFind::find returned d->index=" << d->index;
#endif
        if (d->index != -1) {
            // Flexibility: the app can add more rules to validate a possible match
            if (validateMatch(d->text, d->index, d->matchedLength)) {
                if (d->options & KReplaceDialog::PromptOnReplace) {
#ifdef DEBUG_REPLACE
                    // qDebug() << "PromptOnReplace";
#endif
                    // Display accurate initial string and replacement string, they can vary
                    QString matchedText(d->text.mid(d->index, d->matchedLength));
                    QString rep(matchedText);
                    replaceHelper(rep, d->m_replacement, 0, d->options, d->options & KFind::RegularExpression ? &d->m_match : nullptr, d->matchedLength);
                    d->nextDialog()->setLabel(matchedText, rep);
                    d->nextDialog()->show(); // TODO kde5: virtual void showReplaceNextDialog(QString,QString), so that kreplacetest can skip the show()

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    Q_EMIT textFound(d->text, d->index, d->matchedLength);

                    d->lastResult = Match;
                    return Match;
                } else {
                    d->doReplace(); // this moves on too
                }
            } else {
                // not validated -> move on
                if (d->options & KFind::FindBackwards) {
                    d->index--;
                } else {
                    d->index++;
                }
            }
        } else {
            d->index = INDEX_NOMATCH; // will exit the loop
        }
    } while (d->index != INDEX_NOMATCH);

    d->lastResult = NoMatch;
    return NoMatch;
}

int KReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;
    QRegularExpressionMatch match;
    index = KFind::find(text, pattern, index, options, &matchedLength, options & KFind::RegularExpression ? &match : nullptr);

    if (index != -1) {
        *replacedLength = replaceHelper(text, replacement, index, options, options & KFind::RegularExpression ? &match : nullptr, matchedLength);
        if (options & KFind::FindBackwards) {
            index--;
        } else {
            index += *replacedLength;
        }
    }
    return index;
}

void KReplacePrivate::slotReplaceAll()
{
    Q_Q(KReplace);

    doReplace();
    options &= ~KReplaceDialog::PromptOnReplace;
    Q_EMIT q->optionsChanged();
    Q_EMIT q->findNext();
}

void KReplacePrivate::slotSkip()
{
    Q_Q(KReplace);

    if (options & KFind::FindBackwards) {
        index--;
    } else {
        index++;
    }
    if (dialogClosed) {
        dialog->deleteLater();
        dialog = nullptr; // hide it again
    } else {
        Q_EMIT q->findNext();
    }
}

void KReplacePrivate::slotReplace()
{
    Q_Q(KReplace);

    doReplace();
    if (dialogClosed) {
        dialog->deleteLater();
        dialog = nullptr; // hide it again
    } else {
        Q_EMIT q->findNext();
    }
}

void KReplacePrivate::doReplace()
{
    Q_Q(KReplace);

    Q_ASSERT(index >= 0);
    const int replacedLength = replaceHelper(text, m_replacement, index, options, &m_match, matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    Q_EMIT q->textReplaced(text, index, replacedLength, matchedLength);

#ifdef DEBUG_REPLACE
    // qDebug() << "after replace() signal: d->index=" << d->index << " replacedLength=" << replacedLength;
#endif
    m_replacements++;
    if (options & KFind::FindBackwards) {
        Q_ASSERT(index >= 0);
        index--;
    } else {
        index += replacedLength;
        // when replacing the empty pattern, move on. See also kjs/regexp.cpp for how this should be done for regexps.
        if (pattern.isEmpty()) {
            ++index;
        }
    }
#ifdef DEBUG_REPLACE
    // qDebug() << "after adjustment: d->index=" << d->index;
#endif
}

void KReplace::resetCounts()
{
    Q_D(KReplace);

    KFind::resetCounts();
    d->m_replacements = 0;
}

bool KReplace::shouldRestart(bool forceAsking, bool showNumMatches) const
{
    Q_D(const KReplace);

    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // ... Or if the prompt-on-replace option was set.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if (!forceAsking && (d->options & KFind::FromCursor) == 0 && (d->options & KReplaceDialog::PromptOnReplace) == 0) {
        displayFinalDialog();
        return false;
    }
    QString message;
    if (showNumMatches) {
        if (!d->m_replacements) {
            message = i18n("No text was replaced.");
        } else {
            message = i18np("1 replacement done.", "%1 replacements done.", d->m_replacements);
        }
    } else {
        if (d->options & KFind::FindBackwards) {
            message = i18n("Beginning of document reached.");
        } else {
            message = i18n("End of document reached.");
        }
    }

    message += QLatin1Char('\n');
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        (d->options & KFind::FindBackwards) ? i18n("Do you want to restart search from the end?") : i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionTwoActions(parentWidget(),
                                              message,
                                              QString(),
                                              KGuiItem(i18nc("@action:button Restart find & replace", "Restart")),
                                              KGuiItem(i18nc("@action:button Stop find & replace", "Stop")));
    return (ret == KMessageBox::PrimaryAction);
}

void KReplace::closeReplaceNextDialog()
{
    closeFindNextDialog();
}

#include "kreplace.moc"
#include "moc_kreplace.cpp"
