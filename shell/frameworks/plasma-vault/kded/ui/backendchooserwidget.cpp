/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "backendchooserwidget.h"

#include "ui_backendchooserwidget.h"

#include "engine/backend_p.h"
#include "vault.h"

class BackendChooserWidget::Private
{
public:
    Private(BackendChooserWidget *parent)
        : q(parent)
    {
    }

    Ui::BackendChooserWidget ui;

    bool vaultNameValid = false;
    bool backendValid = false;
    QByteArray bestsBackend;
    int bestBackendPrio = -1;

    void setVaultNameValid(bool valid)
    {
        vaultNameValid = valid;
        q->setIsValid(vaultNameValid && backendValid);
    }

    void setBackendValid(bool valid)
    {
        backendValid = valid;
        q->setIsValid(vaultNameValid && backendValid);
    }

    BackendChooserWidget *const q;
};

BackendChooserWidget::BackendChooserWidget()
    : DialogDsl::DialogModule(false)
    , d(new Private(this))
{
    d->ui.setupUi(this);
    d->ui.textStatus->hide();
    d->ui.page2Layout->setRowStretch(1, 10);

    connect(d->ui.editVaultName, &QLineEdit::textChanged, this, [&](const QString &vaultName) {
        d->setVaultNameValid(!vaultName.isEmpty());
    });

    connect(d->ui.comboBackend, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &BackendChooserWidget::checkCurrentBackend);

    connect(d->ui.pickBackendButton, &QPushButton::clicked, this, &BackendChooserWidget::showBackendSelector);
}

BackendChooserWidget::~BackendChooserWidget()
{
}

void BackendChooserWidget::checkCurrentBackend()
{
    const auto backendId = d->ui.comboBackend->currentData().toString();
    const auto backend = PlasmaVault::Backend::instance(backendId);
    bool backendValid = false;

    d->setBackendValid(false);

    if (!backend) {
        d->ui.textStatus->setHtml(i18n("The specified backend is not available"));
        d->ui.textStatus->show();

    } else {
        d->ui.textStatus->hide();

        const auto result = AsynQt::await(backend->validateBackend());

        if (!result) {
            d->ui.textStatus->setHtml(result.error().message());
            d->ui.textStatus->show();

        } else {
            backendValid = true;
        }
    }

    d->setBackendValid(backendValid);
}

void BackendChooserWidget::showBackendSelector()
{
    d->ui.vaultEncryptionConfig->setCurrentWidget(d->ui.page2);

    checkCurrentBackend();
}

void BackendChooserWidget::addItem(const QByteArray &id, const QString &title, int priority)
{
    d->ui.comboBackend->addItem(title, id);

    if (priority > d->bestBackendPrio) {
        const auto backend = PlasmaVault::Backend::instance(id);
        Q_ASSERT(backend); // backend and UI out of sync. Its an assert since they both are part of the same .so
        if (backend && AsynQt::await(backend->validateBackend())) {
            d->bestBackendPrio = priority;
            d->bestsBackend = id;
            d->ui.backendName->setText(title);
            d->setBackendValid(true);
        }
    }
}

PlasmaVault::Vault::Payload BackendChooserWidget::fields() const
{
    QByteArray backend = d->bestsBackend;
    if (d->ui.vaultEncryptionConfig->currentWidget() == d->ui.page2)
        backend = d->ui.comboBackend->currentData().toByteArray();
    Q_ASSERT(!backend.isEmpty());
    return {{KEY_BACKEND, backend}, {KEY_NAME, d->ui.editVaultName->text()}};
}

void BackendChooserWidget::checkBackendAvailable()
{
    if (d->bestsBackend.isEmpty()) { // in case there are no backends found at all
        showBackendSelector(); // show the more helpful selector
    }
}

#include "moc_backendchooserwidget.cpp"
