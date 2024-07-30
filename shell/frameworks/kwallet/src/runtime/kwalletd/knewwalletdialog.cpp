/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "knewwalletdialog.h"
#include "kwalletd_debug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <gpgme++/context.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

Q_DECLARE_METATYPE(GpgME::Key)

namespace KWallet
{
KNewWalletDialog::KNewWalletDialog(const QString &appName, const QString &walletName, QWidget *parent)
    : QWizard(parent)
{
    setOption(HaveFinishButtonOnEarlyPages);
    _intro = new KNewWalletDialogIntro(appName, walletName, this);
    _introId = addPage(_intro);

    _gpg = new KNewWalletDialogGpg(this);
    _gpgId = addPage(_gpg);
}

bool KNewWalletDialog::isBlowfish() const
{
    return _intro->isBlowfish();
}

GpgME::Key KNewWalletDialog::gpgKey() const
{
    QVariant varKey = field(QStringLiteral("key"));
    return varKey.value<GpgME::Key>();
}

KNewWalletDialogIntro::KNewWalletDialogIntro(const QString &appName, const QString &walletName, QWidget *parent)
    : QWizardPage(parent)
{
    _ui.setupUi(this);
    if (appName.isEmpty()) {
        _ui.labelIntro->setText(
            i18n("<qt>KDE has requested to create a new wallet named '<b>%1</b>'. This is used to store sensitive data in a secure fashion. Please choose the "
                 "new wallet's type below or click cancel to deny the application's request.</qt>",
                 walletName.toHtmlEscaped()));
    } else {
        _ui.labelIntro->setText(
            i18n("<qt>The application '<b>%1</b>' has requested to create a new wallet named '<b>%2</b>'. This is used to store sensitive data in a secure "
                 "fashion. Please choose the new wallet's type below or click cancel to deny the application's request.</qt>",
                 appName.toHtmlEscaped(),
                 walletName.toHtmlEscaped()));
    }
}

void KNewWalletDialogIntro::onBlowfishToggled(bool blowfish)
{
    setFinalPage(blowfish);
}

bool KNewWalletDialogIntro::isBlowfish() const
{
    return _ui.radioBlowfish->isChecked();
}

int KNewWalletDialogIntro::nextId() const
{
    if (isBlowfish()) {
        return -1;
    } else {
        return qobject_cast<const KNewWalletDialog *>(wizard())->gpgId();
    }
}

KNewWalletDialogGpg::KNewWalletDialogGpg(QWidget *parent)
    : QWizardPage(parent)
{
    _ui.setupUi(this);
}

struct AddKeyToList {
    QTableWidget *_list;
    int _row;
    AddKeyToList(QTableWidget *list)
        : _list(list)
        , _row(0)
    {
    }
    void operator()(const GpgME::Key &k)
    {
        GpgME::UserID uid = k.userID(0);
        QString name(uid.name());
        if (uid.comment()) {
            name = QStringLiteral("%1 (%2)").arg(name, uid.comment());
        }
        _list->setItem(_row, 0, new QTableWidgetItem(name));
        _list->setItem(_row, 1, new QTableWidgetItem(uid.email()));
        _list->setItem(_row, 2, new QTableWidgetItem(k.shortKeyID()));
        QVariant varKey;
        varKey.setValue(k);
        _list->item(_row, 0)->setData(Qt::UserRole, varKey);
        ++_row;
    }
};

void KNewWalletDialogGpg::initializePage()
{
    if (_alreadyInitialized) {
        return;
    }

    registerField(QStringLiteral("key"), this);

    GpgME::initializeLibrary();
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err) {
        qCDebug(KWALLETD_LOG) << "OpenPGP not supported on your system!";
        KMessageBox::error(this,
                           i18n("The GpgME library failed to initialize for the OpenPGP protocol. Please check your system's configuration then try again."));
        Q_EMIT completeChanged();
        return;
    }
    std::shared_ptr<GpgME::Context> _ctx(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (!_ctx) {
        KMessageBox::error(this,
                           i18n("The GpgME library failed to initialize for the OpenPGP protocol. Please check your system's configuration then try again."));
        Q_EMIT completeChanged();
        return;
    }
    _ctx->setKeyListMode(GpgME::Local);

    std::vector<GpgME::Key> keys;
    err = _ctx->startKeyListing();
    while (!err) {
        GpgME::Key k = _ctx->nextKey(err);
        if (err) {
            break;
        }
        if (!k.isInvalid() && k.canEncrypt() && (k.ownerTrust() == GpgME::Key::Ultimate)) {
            keys.push_back(k);
        }
    }
    _ctx->endKeyListing();

    if (keys.size() == 0) {
        KMessageBox::error(this,
                           i18n("Seems that your system has no keys suitable for encryption. Please set-up at least one encryption key, then try again."));
        Q_EMIT completeChanged();
        return;
    }

    _ui.listCertificates->setRowCount(keys.size());
    std::for_each(keys.begin(), keys.end(), AddKeyToList(_ui.listCertificates));
    _ui.listCertificates->resizeColumnsToContents();
    _ui.listCertificates->setCurrentCell(0, 0);

    _alreadyInitialized = true;
}

void KNewWalletDialogGpg::onItemSelectionChanged()
{
    _complete = _ui.listCertificates->currentRow() >= 0;
    QVariant varKey = _ui.listCertificates->item(_ui.listCertificates->currentRow(), 0)->data(Qt::UserRole);
    setField(QStringLiteral("key"), varKey);
    Q_EMIT completeChanged();
}

bool KNewWalletDialogGpg::isComplete() const
{
    return _complete;
}

bool KNewWalletDialogGpg::validateCurrentPage()
{
    return false;
}

} // namespace
#include "moc_knewwalletdialog.cpp"
