/*
    SPDX-FileCopyrightText: 2017 Montel Laurent <montel@kde.org>
    SPDX-FileCopyrightText: 2015 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kpasswordlineedit.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QMenu>

class KPasswordLineEditPrivate
{
public:
    KPasswordLineEditPrivate(KPasswordLineEdit *qq)
        : q(qq)
    {
    }
    void initialize();
    void echoModeToggled();
    void showToggleEchoModeAction(const QString &text);
    void slotCustomContextMenuRequested(const QPoint &pos);

    QIcon passwordIcon;
    QIcon visibleIcon;

    QLineEdit *passwordLineEdit = nullptr;
    QAction *toggleEchoModeAction = nullptr;
    bool isToggleEchoModeAvailable = true;
    KPassword::RevealMode revealPasswordMode = KPassword::RevealMode::OnlyNew;
    KPasswordLineEdit *const q;
};

void KPasswordLineEditPrivate::initialize()
{
    QIcon visibilityIcon = QIcon::fromTheme(QStringLiteral("visibility"), QIcon(QStringLiteral(":/icons/visibility.svg")));
    toggleEchoModeAction = passwordLineEdit->addAction(visibilityIcon, QLineEdit::TrailingPosition);
    toggleEchoModeAction->setObjectName(QStringLiteral("visibilityAction"));
    toggleEchoModeAction->setVisible(false);
    toggleEchoModeAction->setToolTip(QObject::tr("Change the visibility of the password", "@info:tooltip"));
    q->connect(toggleEchoModeAction, &QAction::triggered, q, [this]() {
        echoModeToggled();
    });
    q->connect(passwordLineEdit, &QLineEdit::textChanged, q, [this](const QString &str) {
        showToggleEchoModeAction(str);
    });
}

void KPasswordLineEditPrivate::showToggleEchoModeAction(const QString &text)
{
    if (revealPasswordMode != KPassword::RevealMode::Never) {
        toggleEchoModeAction->setVisible(isToggleEchoModeAvailable && (passwordLineEdit->echoMode() == QLineEdit::Normal || !text.isEmpty()));
    } else {
        toggleEchoModeAction->setVisible(false);
    }
}

void KPasswordLineEditPrivate::echoModeToggled()
{
    if (passwordLineEdit->echoMode() == QLineEdit::Password) {
        passwordLineEdit->setEchoMode(QLineEdit::Normal);
        if (passwordIcon.isNull()) {
            passwordIcon = QIcon::fromTheme(QStringLiteral("hint"), QIcon(QStringLiteral(":/icons/hint.svg")));
        }
        toggleEchoModeAction->setIcon(passwordIcon);
    } else if (passwordLineEdit->echoMode() == QLineEdit::Normal) {
        if (visibleIcon.isNull()) {
            visibleIcon = QIcon::fromTheme(QStringLiteral("visibility"), QIcon(QStringLiteral(":/icons/visibility.svg")));
        }
        passwordLineEdit->setEchoMode(QLineEdit::Password);
        toggleEchoModeAction->setIcon(visibleIcon);
    }
    Q_EMIT q->echoModeChanged(passwordLineEdit->echoMode());
}

void KPasswordLineEditPrivate::slotCustomContextMenuRequested(const QPoint &pos)
{
    auto popup = std::unique_ptr<QMenu>(passwordLineEdit->createStandardContextMenu());
    if (popup) {
        if (toggleEchoModeAction->isVisible()) {
            popup->addSeparator();

            toggleEchoModeAction->setText(passwordLineEdit->echoMode() == QLineEdit::Password ? QObject::tr("Show Password", "@action:inmenu")
                                                                                              : QObject::tr("Hide Password", "@action:inmenu"));
            popup->addAction(toggleEchoModeAction);
        }
        popup->exec(pos);
    }
}

KPasswordLineEdit::KPasswordLineEdit(QWidget *parent)
    : QWidget(parent)
    , d(new KPasswordLineEditPrivate(this))
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setObjectName(QStringLiteral("mainlayout"));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    d->passwordLineEdit = new QLineEdit(this);
    d->passwordLineEdit->setObjectName(QStringLiteral("passwordlineedit"));
    d->passwordLineEdit->setEchoMode(QLineEdit::Password);
    d->passwordLineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->passwordLineEdit, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        d->slotCustomContextMenuRequested(pos);
    });
    connect(d->passwordLineEdit, &QLineEdit::textChanged, this, &KPasswordLineEdit::passwordChanged);
    setFocusProxy(d->passwordLineEdit);
    setFocusPolicy(d->passwordLineEdit->focusPolicy());
    mainLayout->addWidget(d->passwordLineEdit);
    d->initialize();
}

KPasswordLineEdit::~KPasswordLineEdit() = default;

void KPasswordLineEdit::setPassword(const QString &password)
{
    if (d->passwordLineEdit->text() != password) {
        if (d->revealPasswordMode == KPassword::RevealMode::OnlyNew) {
            d->isToggleEchoModeAvailable = password.isEmpty();
        }
        d->passwordLineEdit->setText(password);
    }
}

QString KPasswordLineEdit::password() const
{
    return d->passwordLineEdit->text();
}

void KPasswordLineEdit::clear()
{
    d->passwordLineEdit->clear();
}

void KPasswordLineEdit::setClearButtonEnabled(bool clear)
{
    d->passwordLineEdit->setClearButtonEnabled(clear);
}

bool KPasswordLineEdit::isClearButtonEnabled() const
{
    return d->passwordLineEdit->isClearButtonEnabled();
}

void KPasswordLineEdit::setEchoMode(QLineEdit::EchoMode mode)
{
    d->passwordLineEdit->setEchoMode(mode);
}

QLineEdit::EchoMode KPasswordLineEdit::echoMode() const
{
    return d->passwordLineEdit->echoMode();
}

void KPasswordLineEdit::setReadOnly(bool readOnly)
{
    d->passwordLineEdit->setReadOnly(readOnly);
}

bool KPasswordLineEdit::isReadOnly() const
{
    return d->passwordLineEdit->isReadOnly();
}

QLineEdit *KPasswordLineEdit::lineEdit() const
{
    return d->passwordLineEdit;
}

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
void KPasswordLineEdit::setRevealPasswordAvailable(bool reveal)
{
    d->revealPasswordMode = reveal ? KPassword::RevealMode::OnlyNew : KPassword::RevealMode::Never;
    d->showToggleEchoModeAction(password());
}

bool KPasswordLineEdit::isRevealPasswordAvailable() const
{
    return d->revealPasswordMode != KPassword::RevealMode::Never;
}
#endif

void KPasswordLineEdit::setRevealPasswordMode(KPassword::RevealMode mode)
{
    d->revealPasswordMode = mode;
    d->showToggleEchoModeAction(password());
}

KPassword::RevealMode KPasswordLineEdit::revealPasswordMode() const
{
    return d->revealPasswordMode;
}

QAction *KPasswordLineEdit::toggleEchoModeAction() const
{
    return d->toggleEchoModeAction;
}

#include "moc_kpasswordlineedit.cpp"
