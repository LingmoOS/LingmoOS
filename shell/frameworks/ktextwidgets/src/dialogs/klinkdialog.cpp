/*
    klinkdialog
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmailcom>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "klinkdialog_p.h"

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KLinkDialogPrivate
{
public:
    QLabel *textLabel = nullptr;
    QLineEdit *textLineEdit = nullptr;
    QLabel *linkUrlLabel = nullptr;
    QLineEdit *linkUrlLineEdit = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
};
//@endcond

KLinkDialog::KLinkDialog(QWidget *parent)
    : QDialog(parent)
    , d(new KLinkDialogPrivate)
{
    setWindowTitle(i18n("Manage Link"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGridLayout *grid = new QGridLayout;

    d->textLabel = new QLabel(i18n("Link Text:"), this);
    d->textLineEdit = new QLineEdit(this);
    d->textLineEdit->setClearButtonEnabled(true);
    d->linkUrlLabel = new QLabel(i18n("Link URL:"), this);
    d->linkUrlLineEdit = new QLineEdit(this);
    d->linkUrlLineEdit->setClearButtonEnabled(true);

    grid->addWidget(d->textLabel, 0, 0);
    grid->addWidget(d->textLineEdit, 0, 1);
    grid->addWidget(d->linkUrlLabel, 1, 0);
    grid->addWidget(d->linkUrlLineEdit, 1, 1);

    layout->addLayout(grid);

    d->buttonBox = new QDialogButtonBox(this);
    d->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(d->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(d->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(d->buttonBox);

    d->textLineEdit->setFocus();
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(d->textLineEdit, &QLineEdit::textChanged, this, &KLinkDialog::slotTextChanged);
}

KLinkDialog::~KLinkDialog()
{
    delete d;
}

void KLinkDialog::slotTextChanged(const QString &text)
{
    d->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.trimmed().isEmpty());
}

void KLinkDialog::setLinkText(const QString &linkText)
{
    d->textLineEdit->setText(linkText);
    if (!linkText.trimmed().isEmpty()) {
        d->linkUrlLineEdit->setFocus();
    }
}

void KLinkDialog::setLinkUrl(const QString &linkUrl)
{
    d->linkUrlLineEdit->setText(linkUrl);
}

QString KLinkDialog::linkText() const
{
    return d->textLineEdit->text().trimmed();
}

QString KLinkDialog::linkUrl() const
{
    return d->linkUrlLineEdit->text();
}

#include "moc_klinkdialog_p.cpp"
