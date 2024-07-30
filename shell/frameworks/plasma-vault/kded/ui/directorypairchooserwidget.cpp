/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "directorypairchooserwidget.h"
#include "ui_directorypairchooserwidget.h"

#include "backend_p.h"
#include "vault.h"

#include <QStandardPaths>

class DirectoryPairChooserWidget::Private
{
public:
    Ui::DirectoryPairChooserWidget ui;
    const DirectoryPairChooserWidget::Flags flags;

    DirectoryPairChooserWidget *const q;

    class DirectoryValidator
    {
    public:
        bool requireEmptyDirectory;
        bool requireExisting;
        bool valid = false;
        QString defaultPath;
        std::function<void()> update;

        KMessageWidget *widget = nullptr;

        DirectoryValidator(bool requireEmptyDirectory, bool requireExisting, QString defaultPath, std::function<void()> update)
            : requireEmptyDirectory(requireEmptyDirectory)
            , requireExisting(requireExisting)
            , valid(!requireEmptyDirectory && !requireExisting)
            , defaultPath(defaultPath)
            , update(update)
        {
        }

        bool isValid(const QUrl &url) const
        {
            if (url.isEmpty()) {
                widget->hide();
                return false;
            }

            if (url.toLocalFile() == defaultPath) {
                widget->hide();
                return true;
            }

            QDir dir(url.toLocalFile());

            if (!dir.exists()) {
                widget->setText(i18n("The specified path does not exist"));
                widget->show();
                return false;
            }

            if (requireEmptyDirectory && !dir.isEmpty()) {
                widget->setText(i18n("The specified directory is not empty"));
                widget->show();
                return false;
            }

            if (requireExisting && dir.isEmpty()) {
                widget->setText(i18n("The specified directory is empty"));
                widget->show();
                return false;
            }

            widget->hide();
            return true;
        }

        void updateFor(const QUrl &url)
        {
            bool newValid = isValid(url);

            if (valid != newValid) {
                valid = newValid;
                update();
            }
        }
    };

    DirectoryValidator deviceValidator;
    DirectoryValidator mountPointValidator;
    bool allValid;

    void updateValidity()
    {
        bool newAllValid = deviceValidator.valid && mountPointValidator.valid;

        if (allValid != newAllValid) {
            allValid = newAllValid;
            q->setIsValid(allValid);
        }
    }

    Private(DirectoryPairChooserWidget *parent, DirectoryPairChooserWidget::Flags flags)
        : flags(flags)
        , q(parent)
        , deviceValidator(flags & RequireEmptyDevice,
                          flags & RequireExistingDevice,
                          nullptr,
                          [&] {
                              updateValidity();
                          })
        , mountPointValidator(flags & RequireEmptyMountPoint,
                              flags & RequireExistingMountPoint,
                              nullptr,
                              [&] {
                                  updateValidity();
                              })
        , allValid(deviceValidator.valid && mountPointValidator.valid)
    {
    }
};

DirectoryPairChooserWidget::DirectoryPairChooserWidget(DirectoryPairChooserWidget::Flags flags)
    : DialogDsl::DialogModule(false)
    , d(new Private(this, flags))
{
    d->ui.setupUi(this);

    d->deviceValidator.widget = d->ui.messageDevice;
    d->mountPointValidator.widget = d->ui.messageMountPoint;

    if (!(flags & DirectoryPairChooserWidget::ShowDevicePicker)) {
        d->ui.editDevice->setVisible(false);
        d->ui.labelDevice->setVisible(false);
    }

    if (!(flags & DirectoryPairChooserWidget::ShowMountPointPicker)) {
        d->ui.editMountPoint->setVisible(false);
        d->ui.labelMountPoint->setVisible(false);
    }

    d->ui.messageDevice->hide();
    d->ui.messageMountPoint->hide();

    connect(d->ui.editDevice, &KUrlRequester::textEdited, this, [&]() {
        d->deviceValidator.updateFor(d->ui.editDevice->url());
    });

    connect(d->ui.editMountPoint, &KUrlRequester::textEdited, this, [&]() {
        d->mountPointValidator.updateFor(d->ui.editMountPoint->url());
    });
}

DirectoryPairChooserWidget::~DirectoryPairChooserWidget()
{
}

PlasmaVault::Vault::Payload DirectoryPairChooserWidget::fields() const
{
    return {
        {KEY_DEVICE, d->ui.editDevice->url().toLocalFile()},
        {KEY_MOUNT_POINT, d->ui.editMountPoint->url().toLocalFile()},
    };
}

void DirectoryPairChooserWidget::init(const PlasmaVault::Vault::Payload &payload)
{
    if (d->flags & DirectoryPairChooserWidget::AutoFillPaths) {
        const QString basePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma-vault");

        const auto name = payload[KEY_NAME].toString();

        Q_ASSERT(!name.isEmpty());

        QString path = QString("%1/%2.enc").arg(basePath).arg(name);
        int index = 1;
        while (QDir(path).exists()) {
            path = QString("%1/%2_%3.enc").arg(basePath).arg(name).arg(index++);
        }

        d->ui.editDevice->setText(path);
        d->ui.editMountPoint->setText(QDir::homePath() + QStringLiteral("/Vaults/") + name);

        d->deviceValidator.defaultPath = path;
        d->mountPointValidator.defaultPath = QDir::homePath() + QStringLiteral("/Vaults/") + name;
    }

    d->deviceValidator.updateFor(d->ui.editDevice->url());
    d->mountPointValidator.updateFor(d->ui.editMountPoint->url());
    setIsValid(d->allValid);
}

#include "moc_directorypairchooserwidget.cpp"
