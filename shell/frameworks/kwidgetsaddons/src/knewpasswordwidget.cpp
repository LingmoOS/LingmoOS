// vi: ts=8 sts=4 sw=4
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Pietro Iglio <iglio@fub.it>
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2004, 2005 Andrew Coles <andrew_coles@yahoo.co.uk>
    SPDX-FileCopyrightText: 2007 Michaël Larouche <larouche@kde.org>
    SPDX-FileCopyrightText: 2009 Christoph Feck <cfeck@kde.org>
    SPDX-FileCopyrightText: 2015 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knewpasswordwidget.h"
#include "ui_knewpasswordwidget.h"

class KNewPasswordWidgetPrivate
{
    Q_DECLARE_TR_FUNCTIONS(KNewPasswordWidget)

public:
    KNewPasswordWidgetPrivate(KNewPasswordWidget *parent)
        : q(parent)
    {
    }

    void init();
    void passwordChanged();
    void toggleEchoMode();
    int effectivePasswordLength(QStringView password);
    void updatePasswordStatus(KNewPasswordWidget::PasswordStatus status);

    KNewPasswordWidget *const q;

    KNewPasswordWidget::PasswordStatus passwordStatus = KNewPasswordWidget::WeakPassword;
    int minimumPasswordLength = 0;
    int passwordStrengthWarningLevel = 1;
    int reasonablePasswordLength = 8;

    QAction *toggleEchoModeAction = nullptr;
    QColor backgroundWarningColor;
    QColor defaultBackgroundColor;

    Ui::KNewPasswordWidget ui;
};

void KNewPasswordWidgetPrivate::init()
{
    ui.setupUi(q);

    const QString strengthBarWhatsThis(
        tr("The password strength meter gives an indication of the security "
           "of the password you have entered. To improve the strength of the password, try:"
           "<ul><li>using a longer password;</li>"
           "<li>using a mixture of upper- and lower-case letters;</li>"
           "<li>using numbers or symbols, such as #, as well as letters.</li></ul>",
           "@info:whatsthis"));
    ui.labelStrengthMeter->setWhatsThis(strengthBarWhatsThis);
    ui.strengthBar->setWhatsThis(strengthBarWhatsThis);

    QObject::connect(ui.linePassword, &KPasswordLineEdit::echoModeChanged, q, [this]() {
        toggleEchoMode();
    });

    QObject::connect(ui.linePassword, &KPasswordLineEdit::passwordChanged, q, [this]() {
        passwordChanged();
    });
    QObject::connect(ui.lineVerifyPassword, &QLineEdit::textChanged, q, [this]() {
        passwordChanged();
    });

    defaultBackgroundColor = q->palette().color(QPalette::Active, QPalette::Base);
    backgroundWarningColor = defaultBackgroundColor;

    passwordChanged();
}

void KNewPasswordWidgetPrivate::passwordChanged()
{
    const QString password = ui.linePassword->password();
    const QString verification = ui.lineVerifyPassword->text();
    const bool match = (password == verification);
    const bool partialMatch = password.startsWith(verification);
    const int minPasswordLength = q->minimumPasswordLength();

    QPalette palette = q->palette();
    palette.setColor(QPalette::Active, QPalette::Base, (match || partialMatch) ? defaultBackgroundColor : backgroundWarningColor);
    ui.lineVerifyPassword->setPalette(palette);

    // Password strength calculator
    int pwstrength =
        (20 * ui.linePassword->password().length() + 80 * effectivePasswordLength(ui.linePassword->password())) / qMax(reasonablePasswordLength, 2);
    ui.strengthBar->setValue(qBound(0, pwstrength, 100));

    // update the current password status
    if (match || ui.lineVerifyPassword->isHidden()) {
        if (!q->allowEmptyPasswords() && ui.linePassword->password().isEmpty()) {
            updatePasswordStatus(KNewPasswordWidget::EmptyPasswordNotAllowed);
        } else if (ui.linePassword->password().length() < minPasswordLength) {
            updatePasswordStatus(KNewPasswordWidget::PasswordTooShort);
        } else if (ui.strengthBar && ui.strengthBar->value() < passwordStrengthWarningLevel) {
            updatePasswordStatus(KNewPasswordWidget::WeakPassword);
        } else {
            updatePasswordStatus(KNewPasswordWidget::StrongPassword);
        }
    } else {
        updatePasswordStatus(KNewPasswordWidget::PasswordNotVerified);
    }
}

void KNewPasswordWidgetPrivate::toggleEchoMode()
{
    if (ui.linePassword->lineEdit()->echoMode() == QLineEdit::Normal) {
        ui.lineVerifyPassword->hide();
        ui.labelVerifyPassword->hide();
    } else if (ui.linePassword->lineEdit()->echoMode() == QLineEdit::Password) {
        ui.lineVerifyPassword->show();
        ui.labelVerifyPassword->show();
    }
    passwordChanged();
}

int KNewPasswordWidgetPrivate::effectivePasswordLength(QStringView password)
{
    enum Category {
        Digit,
        Upper,
        Vowel,
        Consonant,
        Special,
    };

    Category previousCategory = Vowel;
    static const QLatin1String vowels("aeiou");
    int count = 0;

    const int len = password.length();
    for (int i = 0; i < len; ++i) {
        const QChar currentChar = password.at(i);
        if (!password.left(i).contains(currentChar)) {
            Category currentCategory;
            switch (currentChar.category()) {
            case QChar::Letter_Uppercase:
                currentCategory = Upper;
                break;
            case QChar::Letter_Lowercase:
                if (vowels.contains(currentChar)) {
                    currentCategory = Vowel;
                } else {
                    currentCategory = Consonant;
                }
                break;
            case QChar::Number_DecimalDigit:
                currentCategory = Digit;
                break;
            default:
                currentCategory = Special;
                break;
            }
            switch (currentCategory) {
            case Vowel:
                if (previousCategory != Consonant) {
                    ++count;
                }
                break;
            case Consonant:
                if (previousCategory != Vowel) {
                    ++count;
                }
                break;
            default:
                if (previousCategory != currentCategory) {
                    ++count;
                }
                break;
            }
            previousCategory = currentCategory;
        }
    }
    return count;
}

void KNewPasswordWidgetPrivate::updatePasswordStatus(KNewPasswordWidget::PasswordStatus status)
{
    if (passwordStatus == status) {
        return;
    }

    passwordStatus = status;
    Q_EMIT q->passwordStatusChanged();
}

KNewPasswordWidget::KNewPasswordWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KNewPasswordWidgetPrivate(this))
{
    d->init();
}

KNewPasswordWidget::~KNewPasswordWidget() = default;

KNewPasswordWidget::PasswordStatus KNewPasswordWidget::passwordStatus() const
{
    return d->passwordStatus;
}

bool KNewPasswordWidget::allowEmptyPasswords() const
{
    return d->minimumPasswordLength == 0;
}

int KNewPasswordWidget::minimumPasswordLength() const
{
    return d->minimumPasswordLength;
}

int KNewPasswordWidget::maximumPasswordLength() const
{
    return d->ui.linePassword->lineEdit()->maxLength();
}

int KNewPasswordWidget::reasonablePasswordLength() const
{
    return d->reasonablePasswordLength;
}

int KNewPasswordWidget::passwordStrengthWarningLevel() const
{
    return d->passwordStrengthWarningLevel;
}

QColor KNewPasswordWidget::backgroundWarningColor() const
{
    return d->backgroundWarningColor;
}

bool KNewPasswordWidget::isPasswordStrengthMeterVisible() const
{
    return d->ui.labelStrengthMeter->isVisible() && d->ui.strengthBar->isVisible();
}

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
bool KNewPasswordWidget::isRevealPasswordAvailable() const
{
    return d->ui.linePassword->isRevealPasswordAvailable();
}
#pragma GCC diagnostic pop
#endif

KPassword::RevealMode KNewPasswordWidget::revealPasswordMode() const
{
    return d->ui.linePassword->revealPasswordMode();
}

QString KNewPasswordWidget::password() const
{
    return d->ui.linePassword->password();
}

void KNewPasswordWidget::setAllowEmptyPasswords(bool allowed)
{
    setMinimumPasswordLength(allowed ? 0 : 1);
    d->passwordChanged();
}

void KNewPasswordWidget::setMinimumPasswordLength(int minLength)
{
    d->minimumPasswordLength = minLength;
    d->passwordChanged();
}

void KNewPasswordWidget::setMaximumPasswordLength(int maxLength)
{
    if (maxLength < minimumPasswordLength()) {
        maxLength = minimumPasswordLength();
    }

    d->ui.linePassword->lineEdit()->setMaxLength(maxLength);
    d->ui.lineVerifyPassword->setMaxLength(maxLength);
}

// reasonable password length code contributed by Steffen Mthing

void KNewPasswordWidget::setReasonablePasswordLength(int reasonableLength)
{
    d->reasonablePasswordLength = qBound(1, reasonableLength, maximumPasswordLength());
}

void KNewPasswordWidget::setPasswordStrengthWarningLevel(int warningLevel)
{
    d->passwordStrengthWarningLevel = qBound(0, warningLevel, 99);
}

void KNewPasswordWidget::setBackgroundWarningColor(const QColor &color)
{
    d->backgroundWarningColor = color;
    update();
}

void KNewPasswordWidget::setPasswordStrengthMeterVisible(bool visible)
{
    d->ui.labelStrengthMeter->setVisible(visible);
    d->ui.strengthBar->setVisible(visible);
}

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void KNewPasswordWidget::setRevealPasswordAvailable(bool reveal)
{
    d->ui.linePassword->setRevealPasswordAvailable(reveal);
}
#pragma GCC diagnostic pop
#endif

void KNewPasswordWidget::setRevealPasswordMode(KPassword::RevealMode revealPasswordMode)
{
    d->ui.linePassword->setRevealPasswordMode(revealPasswordMode);
}

#include "moc_knewpasswordwidget.cpp"
