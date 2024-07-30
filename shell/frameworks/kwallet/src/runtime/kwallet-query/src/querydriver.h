/*
    This file is part of the KDE
    SPDX-FileCopyrightText: 2015 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QApplication>

#include <KWallet>

using namespace KWallet;

class QueryDriver : public QApplication
{
    Q_OBJECT
public:
    enum Mode { List, Read, Write };
    QueryDriver(int &argc, char *argv[]);
    ~QueryDriver() override;

    void setWalletName(const QString &walletName);
    void setMode(Mode mode);
    void setVerbose()
    {
        verbose = true;
    }
    void setEntryName(const QString &entryName)
    {
        this->entryName = entryName;
    }
    void setEntryFolder(const QString &entryFolder)
    {
        this->entryFolder = entryFolder;
    }

private:
    void timerEvent(QTimerEvent *event) override;
    void readEntries();
    void readValue();
    void readMapValue();
    void readPasswordValue();
    void writeValue();

private Q_SLOTS:
    void walletOpened(bool);

public:
    QString walletName;
    Wallet *theWallet = nullptr;
    Mode mode;
    bool verbose = false;
    QString entryName;
    QString entryFolder;
};
