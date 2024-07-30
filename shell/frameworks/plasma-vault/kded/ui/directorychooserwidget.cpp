/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "directorychooserwidget.h"
#include "ui_directorychooserwidget.h"

#include "vault.h"

class DirectoryChooserWidget::Private
{
public:
    Ui::DirectoryChooserWidget ui;
    DirectoryChooserWidget::Flags flags;

    bool mountPointValid = false;

    DirectoryChooserWidget *const q;
    Private(DirectoryChooserWidget *parent)
        : q(parent)
    {
    }

    void setMountPointValid(bool valid)
    {
        if (mountPointValid == valid)
            return;

        mountPointValid = valid;

        q->setIsValid(valid);
    }

    bool isDirectoryValid(const QUrl &url) const
    {
        if (url.isEmpty())
            return false;

        QDir directory(url.toString());

        // TODO: Support alternative flags once they are needed
        if (!directory.exists() || directory.entryList().isEmpty())
            return true;

        return false;
    }
};

DirectoryChooserWidget::DirectoryChooserWidget(DirectoryChooserWidget::Flags flags)
    : DialogDsl::DialogModule(false)
    , d(new Private(this))
{
    d->ui.setupUi(this);
    d->flags = flags;

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited, this, [&]() {
        d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
    });
}

DirectoryChooserWidget::~DirectoryChooserWidget()
{
}

PlasmaVault::Vault::Payload DirectoryChooserWidget::fields() const
{
    return {
        {KEY_MOUNT_POINT, d->ui.editMountPoint->url().toLocalFile()},
    };
}

void DirectoryChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    const auto mountPoint = payload[KEY_MOUNT_POINT].toString();

    d->ui.editMountPoint->setText(mountPoint);

    d->setMountPointValid(d->isDirectoryValid(d->ui.editMountPoint->url()));
}

#include "moc_directorychooserwidget.cpp"
