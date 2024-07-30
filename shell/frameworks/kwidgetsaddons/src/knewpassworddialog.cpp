// vi: ts=8 sts=4 sw=4
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Pietro Iglio <iglio@fub.it>
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2004, 2005 Andrew Coles <andrew_coles@yahoo.co.uk>
    SPDX-FileCopyrightText: 2007 Michaël Larouche <larouche@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "knewpassworddialog.h"

#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QStyleOption>

#include <ktitlewidget.h>

#include "ui_knewpassworddialog.h"

class KNewPasswordDialogPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KNewPasswordDialog)

public:
    KNewPasswordDialogPrivate(KNewPasswordDialog *parent)
        : q(parent)
    {
    }

    void init();
    void passwordStatusChanged();
    void showMessageWidget(const QString &message, KMessageWidget::MessageType type);

    KNewPasswordDialog *const q;

    QString pass;
    QIcon icon;

    Ui::KNewPasswordDialog ui;
};

void KNewPasswordDialogPrivate::init()
{
    ui.setupUi(q);
    q->setIcon(QIcon::fromTheme(QStringLiteral("dialog-password")));

    ui.statusMsgWidget->hide();

    QObject::connect(ui.pwdWidget, &KNewPasswordWidget::passwordStatusChanged, q, [this]() {
        passwordStatusChanged();
    });
}

void KNewPasswordDialogPrivate::passwordStatusChanged()
{
    switch (ui.pwdWidget->passwordStatus()) {
    case KNewPasswordWidget::PasswordTooShort:
        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        //~ singular Password must be at least %n character long
        //~ plural Password must be at least %n characters long
        showMessageWidget(tr("Password must be at least %n character(s) long.", "@info", ui.pwdWidget->minimumPasswordLength()), KMessageWidget::Error);
        break;
    case KNewPasswordWidget::EmptyPasswordNotAllowed:
        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        showMessageWidget(tr("Password is empty.", "@info"), KMessageWidget::Warning);
        break;
    case KNewPasswordWidget::PasswordNotVerified:
        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        showMessageWidget(tr("Passwords do not match.", "@info"), KMessageWidget::Warning);
        break;
    case KNewPasswordWidget::WeakPassword:
    case KNewPasswordWidget::StrongPassword:
        ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        showMessageWidget(tr("Passwords match.", "@info"), KMessageWidget::Positive);
        break;
    }
}

void KNewPasswordDialogPrivate::showMessageWidget(const QString &message, KMessageWidget::MessageType type)
{
    ui.statusMsgWidget->setText(message);
    ui.statusMsgWidget->setMessageType(type);
    ui.statusMsgWidget->animatedShow();
}

/*
 * Password dialog.
 */

KNewPasswordDialog::KNewPasswordDialog(QWidget *parent)
    : QDialog(parent)
    , d(new KNewPasswordDialogPrivate(this))
{
    d->init();
}

KNewPasswordDialog::~KNewPasswordDialog() = default;

void KNewPasswordDialog::setPrompt(const QString &prompt)
{
    d->ui.labelPrompt->setText(prompt);
    d->ui.labelPrompt->setMinimumSize(d->ui.labelPrompt->sizeHint());
}

QString KNewPasswordDialog::prompt() const
{
    return d->ui.labelPrompt->text();
}

void KNewPasswordDialog::setIcon(const QIcon &icon)
{
    d->icon = icon;
    QStyleOption option;
    option.initFrom(this);
    const int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, this);
    d->ui.labelIcon->setPixmap(icon.pixmap(iconSize, iconSize));
    d->ui.labelIcon->setFixedSize(d->ui.labelIcon->sizeHint());
}

QIcon KNewPasswordDialog::icon() const
{
    return d->icon;
}

bool KNewPasswordDialog::checkAndGetPassword(QString *pwd)
{
    pwd->clear();

    if (d->ui.pwdWidget->passwordStatus() == KNewPasswordWidget::WeakPassword) {
        QMessageBox::StandardButton selectedButton = QMessageBox::warning(this,
                                                                          tr("Low Password Strength", "@title:window"),
                                                                          tr("The password you have entered has a low strength. "
                                                                             "To improve the strength of "
                                                                             "the password, try:\n"
                                                                             " - using a longer password;\n"
                                                                             " - using a mixture of upper- and lower-case letters;\n"
                                                                             " - using numbers or symbols as well as letters.\n"
                                                                             "\n"
                                                                             "Would you like to use this password anyway?"),
                                                                          QMessageBox::Yes | QMessageBox::No);

        if (selectedButton == QMessageBox::No) {
            return false;
        }
    }
    if (!checkPassword(d->ui.pwdWidget->password())) {
        return false;
    }

    *pwd = d->ui.pwdWidget->password();
    return true;
}

void KNewPasswordDialog::accept()
{
    QString pwd;
    if (!checkAndGetPassword(&pwd)) {
        return;
    }
    d->pass = pwd;
    Q_EMIT newPassword(d->pass);
    QDialog::accept();
}

void KNewPasswordDialog::setAllowEmptyPasswords(bool allowed)
{
    d->ui.pwdWidget->setAllowEmptyPasswords(allowed);
}

bool KNewPasswordDialog::allowEmptyPasswords() const
{
    return d->ui.pwdWidget->allowEmptyPasswords();
}

void KNewPasswordDialog::setMinimumPasswordLength(int minLength)
{
    d->ui.pwdWidget->setMinimumPasswordLength(minLength);
}

int KNewPasswordDialog::minimumPasswordLength() const
{
    return d->ui.pwdWidget->minimumPasswordLength();
}

void KNewPasswordDialog::setMaximumPasswordLength(int maxLength)
{
    d->ui.pwdWidget->setMaximumPasswordLength(maxLength);
}

int KNewPasswordDialog::maximumPasswordLength() const
{
    return d->ui.pwdWidget->maximumPasswordLength();
}

// reasonable password length code contributed by Steffen Mthing

void KNewPasswordDialog::setReasonablePasswordLength(int reasonableLength)
{
    d->ui.pwdWidget->setReasonablePasswordLength(reasonableLength);
}

int KNewPasswordDialog::reasonablePasswordLength() const
{
    return d->ui.pwdWidget->reasonablePasswordLength();
}

void KNewPasswordDialog::setPasswordStrengthWarningLevel(int warningLevel)
{
    d->ui.pwdWidget->setPasswordStrengthWarningLevel(warningLevel);
}

int KNewPasswordDialog::passwordStrengthWarningLevel() const
{
    return d->ui.pwdWidget->passwordStrengthWarningLevel();
}

void KNewPasswordDialog::setBackgroundWarningColor(const QColor &color)
{
    d->ui.pwdWidget->setBackgroundWarningColor(color);
}

QColor KNewPasswordDialog::backgroundWarningColor() const
{
    return d->ui.pwdWidget->backgroundWarningColor();
}

QString KNewPasswordDialog::password() const
{
    return d->pass;
}

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void KNewPasswordDialog::setRevealPasswordAvailable(bool reveal)
{
    d->ui.pwdWidget->setRevealPasswordAvailable(reveal);
}

bool KNewPasswordDialog::isRevealPasswordAvailable() const
{
    return d->ui.pwdWidget->isRevealPasswordAvailable();
}
#pragma GCC diagnostic pop
#endif

KPassword::RevealMode KNewPasswordDialog::revealPasswordMode() const
{
    return d->ui.pwdWidget->revealPasswordMode();
}

void KNewPasswordDialog::setRevealPasswordMode(KPassword::RevealMode revealPasswordMode)
{
    d->ui.pwdWidget->setRevealPasswordMode(revealPasswordMode);
}

bool KNewPasswordDialog::checkPassword(const QString &)
{
    return true;
}

#include "moc_knewpassworddialog.cpp"
