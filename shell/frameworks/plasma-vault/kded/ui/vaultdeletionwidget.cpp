/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "vaultdeletionwidget.h"

#include "ui_vaultdeletionwidget.h"

#include <KConfigGroup>
#include <KMessageWidget>
#include <KSharedConfig>

#include <QDBusInterface>
#include <QDBusPendingCall>

class VaultDeletionWidget::Private
{
public:
    Ui::VaultDeletionWidget ui;
    QString vaultName;
    QString vaultDevice;
    KSharedConfig::Ptr config;
};

VaultDeletionWidget::VaultDeletionWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    auto messageWidget = new KMessageWidget(d->ui.labelWarning->text(), this);
    messageWidget->setMessageType(KMessageWidget::Warning);
    messageWidget->setCloseButtonVisible(false);
    messageWidget->setIcon(QIcon::fromTheme("dialog-warning"));
    static_cast<QBoxLayout *>(layout())->insertWidget(0, messageWidget);

    d->ui.labelWarning->hide();

    connect(d->ui.textVaultNameConfirmation, &QLineEdit::textEdited, this, [this](const QString &newText) {
        d->ui.buttonDeleteVault->setEnabled(d->vaultName == newText);
    });

    connect(d->ui.buttonDeleteVault, &QPushButton::clicked, this, [this] {
        d->ui.buttonDeleteVault->setEnabled(false);
        Q_EMIT requestCancellation();

        QDBusInterface(QStringLiteral("org.kde.kded6"), QStringLiteral("/modules/plasmavault"), QStringLiteral("org.kde.plasmavault"))
            .asyncCall("deleteVault", d->vaultDevice, d->vaultName);
    });
}

VaultDeletionWidget::~VaultDeletionWidget()
{
}

PlasmaVault::Vault::Payload VaultDeletionWidget::fields() const
{
    return {};
}

void VaultDeletionWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    d->vaultName = payload[KEY_NAME].toString();
    d->vaultDevice = payload[KEY_DEVICE].toString();
    d->ui.buttonDeleteVault->setEnabled(false);
}

#include "moc_vaultdeletionwidget.cpp"
