/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPSPRINTER_H
#define KCUPSPRINTER_H

#include <KCupsConnection.h>
#include <kcupslib_export.h>

class KCUPSLIB_EXPORT KCupsPrinter
{
public:
    enum Status { Idle = 3, Printing, Stopped };

    KCupsPrinter();
    explicit KCupsPrinter(const QString &printer, bool isClass = false);

    QString name() const;
    bool isClass() const;
    bool isDefault() const;
    bool isShared() const;
    bool isAcceptingJobs() const;
    cups_ptype_e type() const;
    QString location() const;
    QString info() const;
    QString makeAndModel() const;
    QStringList commands() const;
    QStringList memberNames() const;
    QString deviceUri() const;
    QStringList errorPolicy() const;
    QStringList errorPolicySupported() const;
    QStringList opPolicy() const;
    QStringList opPolicySupported() const;
    QStringList jobSheetsDefault() const;
    QStringList jobSheetsSupported() const;
    QStringList requestingUserNameAllowed() const;
    QStringList requestingUserNameDenied() const;
    QStringList authInfoRequired() const;
    QString uriSupported() const;

    Status state() const;
    QString stateMsg() const;
    int markerChangeTime() const;
    QVariant argument(const QString &name) const;

    /**
     * Requires enum PrinterType to work properly
     *
     */
    QIcon icon() const;
    static QIcon icon(cups_ptype_e type);
    QString iconName() const;
    static QString iconName(cups_ptype_e type);

protected:
    KCupsPrinter(const QVariantMap &arguments);

private:
    friend class KCupsRequest;

    QString m_printer;
    bool m_isClass;
    QVariantMap m_arguments;
};

typedef QList<KCupsPrinter> KCupsPrinters;
Q_DECLARE_METATYPE(KCupsPrinters)
Q_DECLARE_METATYPE(KCupsPrinter)

#endif // KCUPSPRINTER_H
