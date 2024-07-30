/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kreplacedialog.h"
#include "kfinddialog_p.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QRegularExpression>

#include <KHistoryComboBox>
#include <KLocalizedString>
#include <KMessageBox>

/**
 * we need to insert the strings after the dialog is set
 * up, otherwise QComboBox will deliver an awful big sizeHint
 * for long replacement texts.
 */
class KReplaceDialogPrivate : public KFindDialogPrivate
{
    Q_DECLARE_PUBLIC(KReplaceDialog)

public:
    explicit KReplaceDialogPrivate(KReplaceDialog *qq)
        : KFindDialogPrivate(qq)
    {
    }

    void slotOk();

    QStringList replaceStrings;
    mutable QWidget *replaceExtension = nullptr;
    bool initialShowDone = false;
};

KReplaceDialog::KReplaceDialog(QWidget *parent, long options, const QStringList &findStrings, const QStringList &replaceStrings, bool hasSelection)
    : KFindDialog(*new KReplaceDialogPrivate(this), parent, options, findStrings, hasSelection, true /*create replace dialog*/)
{
    Q_D(KReplaceDialog);

    d->replaceStrings = replaceStrings;
}

KReplaceDialog::~KReplaceDialog() = default;

void KReplaceDialog::showEvent(QShowEvent *e)
{
    Q_D(KReplaceDialog);

    if (!d->initialShowDone) {
        d->initialShowDone = true; // only once

        if (!d->replaceStrings.isEmpty()) {
            setReplacementHistory(d->replaceStrings);
            d->replace->lineEdit()->setText(d->replaceStrings[0]);
        }
    }

    KFindDialog::showEvent(e);
}

long KReplaceDialog::options() const
{
    Q_D(const KReplaceDialog);

    long options = 0;

    options = KFindDialog::options();
    if (d->promptOnReplace->isChecked()) {
        options |= PromptOnReplace;
    }
    if (d->backRef->isChecked()) {
        options |= BackReference;
    }
    return options;
}

QWidget *KReplaceDialog::replaceExtension() const
{
    Q_D(const KReplaceDialog);

    if (!d->replaceExtension) {
        d->replaceExtension = new QWidget(d->replaceGrp);
        d->replaceLayout->addWidget(d->replaceExtension, 3, 0, 1, 2);
    }

    return d->replaceExtension;
}

QString KReplaceDialog::replacement() const
{
    Q_D(const KReplaceDialog);

    return d->replace->currentText();
}

QStringList KReplaceDialog::replacementHistory() const
{
    Q_D(const KReplaceDialog);

    QStringList lst = d->replace->historyItems();
    // historyItems() doesn't tell us about the case of replacing with an empty string
    if (d->replace->lineEdit()->text().isEmpty()) {
        lst.prepend(QString());
    }
    return lst;
}

void KReplaceDialog::setOptions(long options)
{
    Q_D(KReplaceDialog);

    KFindDialog::setOptions(options);
    d->promptOnReplace->setChecked(options & PromptOnReplace);
    d->backRef->setChecked(options & BackReference);
}

void KReplaceDialog::setReplacementHistory(const QStringList &strings)
{
    Q_D(KReplaceDialog);

    if (!strings.isEmpty()) {
        d->replace->setHistoryItems(strings, true);
    } else {
        d->replace->clearHistory();
    }
}

void KReplaceDialogPrivate::slotOk()
{
    Q_Q(KReplaceDialog);

    // If regex and backrefs are enabled, do a sanity check.
    if (regExp->isChecked() && backRef->isChecked()) {
        const QRegularExpression re(q->pattern(), QRegularExpression::UseUnicodePropertiesOption);
        const int caps = re.captureCount();

        const QString rep = q->replacement();
        const QRegularExpression check(QStringLiteral("((?:\\\\)+)(\\d+)"));
        auto iter = check.globalMatch(rep);
        while (iter.hasNext()) {
            const QRegularExpressionMatch match = iter.next();
            if ((match.captured(1).size() % 2) && match.captured(2).toInt() > caps) {
                KMessageBox::information(q,
                                         i18n("Your replacement string is referencing a capture greater than '\\%1', ", caps)
                                             + (caps ? i18np("but your pattern only defines 1 capture.", "but your pattern only defines %1 captures.", caps)
                                                     : i18n("but your pattern defines no captures."))
                                             + i18n("\nPlease correct."));
                return; // abort OKing
            }
        }
    }

    slotOk();
    replace->addToHistory(q->replacement());
}

#include "moc_kreplacedialog.cpp"
