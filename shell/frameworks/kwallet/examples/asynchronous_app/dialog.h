/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010-2012 Martin Sandsmark <martin.sandsmark@kde.org>
    SPDX-FileCopyrightText: 2018 Olivier Churlaud <olivier@churlaud.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

namespace KWallet
{
class Wallet;
}

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog(QWidget *parent = nullptr);

private Q_SLOTS:
    void doSave();
    void walletOpened(bool ok);

private:
    KWallet::Wallet *m_wallet;
    QLineEdit *m_keyInput;
    QLineEdit *m_valueInput;
    QLabel *m_statusLabel;
    QPushButton *m_launchButton;
};

#endif // DIALOG_H
