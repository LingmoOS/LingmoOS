/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPATTRIBUTES_H_
#define _KWALLETFREEDESKTOPATTRIBUTES_H_

#include "kwalletfreedesktopservice.h"
#include <QJsonObject>

class KWalletFreedesktopAttributes : public QObject
{
public:
    KWalletFreedesktopAttributes(const QString &walletName);

    void read();
    void write();
    void remove(const EntryLocation &entryLocation);
    void remove(const FdoUniqueLabel &itemUniqLabel);
    void renameLabel(const EntryLocation &oldLocation, const EntryLocation &newLocation);
    void deleteFile();
    void renameWallet(const QString &newName);
    void newItem(const EntryLocation &entryLocation);

    QList<EntryLocation> matchAttributes(const StrStrMap &attributes) const;
    void setAttributes(const EntryLocation &entryLocation, const StrStrMap &attributes);
    StrStrMap getAttributes(const EntryLocation &entryLocation) const;

    void setAttributes(const FdoUniqueLabel &itemUniqLabel, const StrStrMap &attributes);
    StrStrMap getAttributes(const FdoUniqueLabel &itemUniqLabel) const;

    QString getStringParam(const EntryLocation &entryLocation, const QString &paramName, const QString &defaultParam) const;
    qulonglong getULongLongParam(const EntryLocation &entryLocation, const QString &paramName, qulonglong defaultParam) const;

    QString getStringParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, const QString &defaultParam) const;
    qulonglong getULongLongParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, qulonglong defaultParam) const;

    void setParam(const EntryLocation &entryLocation, const QString &paramName, const QString &param);
    void setParam(const EntryLocation &entryLocation, const QString &paramName, qulonglong param);

    void setParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, const QString &param);
    void setParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, qulonglong param);

    qulonglong lastModified() const;
    qulonglong birthTime() const;
    void updateLastModified();

    QList<EntryLocation> listItems() const;

private:
    QString _path;
    QJsonObject _params;
};

#endif
