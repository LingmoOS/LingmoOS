/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "offlineonlywidget.h"

#include "ui_offlineonlywidget.h"

#include <KConfigGroup>
#include <KSharedConfig>

class OfflineOnlyChooserWidget::Private
{
public:
    Ui::OfflineOnlyChooserWidget ui;
    KSharedConfig::Ptr config;
};

OfflineOnlyChooserWidget::OfflineOnlyChooserWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);
}

OfflineOnlyChooserWidget::~OfflineOnlyChooserWidget()
{
}

PlasmaVault::Vault::Payload OfflineOnlyChooserWidget::fields() const
{
    return {
        {KEY_OFFLINEONLY, d->ui.checkShouldBeOffline->isChecked()},
    };
}

void OfflineOnlyChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    d->ui.checkShouldBeOffline->setChecked(payload[KEY_OFFLINEONLY].toBool());
}

#include "moc_offlineonlywidget.cpp"
