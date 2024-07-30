/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KNEWWALLETDIALOG_H
#define KNEWWALLETDIALOG_H

#include <QWizard>

#include "ui_knewwalletdialoggpg.h"
#include "ui_knewwalletdialogintro.h"

namespace GpgME
{
class Key;
}

namespace KWallet
{
class KNewWalletDialogIntro;
class KNewWalletDialogGpg;

class KNewWalletDialog : public QWizard
{
    Q_OBJECT
public:
    KNewWalletDialog(const QString &appName, const QString &walletName, QWidget *parent = nullptr);

    bool isBlowfish() const;
    int gpgId() const
    {
        return _gpgId;
    }
    GpgME::Key gpgKey() const;

private:
    KNewWalletDialogIntro *_intro = nullptr;
    int _introId = 0;
    KNewWalletDialogGpg *_gpg = nullptr;
    int _gpgId = 0;
};

class KNewWalletDialogIntro : public QWizardPage
{
    Q_OBJECT
public:
    KNewWalletDialogIntro(const QString &appName, const QString &walletName, QWidget *parent = nullptr);
    bool isBlowfish() const;
    int nextId() const override;
protected Q_SLOTS:
    void onBlowfishToggled(bool);

private:
    Ui_KNewWalletDialogIntro _ui;
};

class KNewWalletDialogGpg : public QWizardPage
{
    Q_OBJECT
public:
    explicit KNewWalletDialogGpg(QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    virtual bool validateCurrentPage();
protected Q_SLOTS:
    void onItemSelectionChanged();

private:
    bool _alreadyInitialized = false;
    Ui_KNewWalletDialogGpg _ui;
    bool _complete = false;
};

} // namespace

#endif // KNEWWALLETDIALOG_H
