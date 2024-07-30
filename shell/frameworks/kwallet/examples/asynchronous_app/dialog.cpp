/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010-2012 Martin Sandsmark <martin.sandsmark@kde.org>
    SPDX-FileCopyrightText: 2018 Olivier Churlaud <olivier@churlaud.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include "dialog.h"

#include <KWallet>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    // Create the object
    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), winId(), KWallet::Wallet::Asynchronous);

    QLabel *explanation =
        new QLabel(QStringLiteral("<b>HELLO!</b><br/>"
                                  "Please type in something to save in the wallet!<br/>"
                                  "It will be saved in the form data folder, under <br/>"
                                  "the entry <i>http://test.com/#form</i>."));
    m_statusLabel = new QLabel(QStringLiteral("Opening wallet..."), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_keyInput = new QLineEdit(this);
    m_valueInput = new QLineEdit(this);
    m_launchButton = new QPushButton(QStringLiteral("Save!"), this);
    m_launchButton->setDisabled(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(explanation);
    layout->addStretch();
    layout->addWidget(m_statusLabel);
    layout->addWidget(new QLabel(QStringLiteral("Key:"), this));
    layout->addWidget(m_keyInput);
    layout->addWidget(new QLabel(QStringLiteral("Value:"), this));
    layout->addWidget(m_valueInput);
    layout->addWidget(m_launchButton);

    connect(m_launchButton, &QPushButton::clicked, this, &Dialog::doSave);

    // As we work asynchronously we need to use a signal/slot architecture
    connect(m_wallet, &KWallet::Wallet::walletOpened, this, &Dialog::walletOpened);
    setMinimumSize(500, 200);
}

void Dialog::walletOpened(bool ok)
{
    if (ok && (m_wallet->hasFolder(KWallet::Wallet::FormDataFolder()) || m_wallet->createFolder(KWallet::Wallet::FormDataFolder()))
        && m_wallet->setFolder(KWallet::Wallet::FormDataFolder())) {
        m_launchButton->setDisabled(false);
        m_statusLabel->setText(QStringLiteral("Idle."));
    } else {
        m_statusLabel->setText(QStringLiteral("Error opening wallet!"));
    }
}

void Dialog::doSave()
{
    if (m_keyInput->text().isEmpty() || m_valueInput->text().isEmpty()) {
        m_statusLabel->setText(QStringLiteral("Empty field!"));
        return;
    }

    m_launchButton->setDisabled(true);

    m_statusLabel->setText(QStringLiteral("Saving ..."));

    QMap<QString, QString> map;
    map[m_keyInput->text()] = m_valueInput->text();

    // Write in the map "http://test.com/#form" key/value contained in map
    if (m_wallet->writeMap(QStringLiteral("http://test.com/#form"), map)) {
        m_statusLabel->setText(QStringLiteral("Something went wrong!"));
    } else {
        m_statusLabel->setText(QStringLiteral("Saved!"));
        m_keyInput->clear();
        m_valueInput->clear();
    }
    m_launchButton->setDisabled(false);
}

#include "moc_dialog.cpp"
