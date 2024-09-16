// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <exception>
#include <transaction.h>
#include <tuple>

#include <QDebug>
#include <QString>
#include <QCoroTask>

#include <Transaction>

namespace PKUtils {
    typedef std::tuple<PackageKit::Transaction::Info, QString, QString> PkPackage;
    typedef QVector<PkPackage> PkPackages;

    class PkError : public std::exception {
    public:
        PkError(PackageKit::Transaction::Error err, const QString &msg) : m_error(err), m_reason(msg) {}
        inline PackageKit::Transaction::Error error() const { return m_error; }
        inline QString reason() const { return m_reason; }

        inline static const PkError * castFromStdException(const std::exception &e) { return dynamic_cast<const PKUtils::PkError*>(&e); }
        inline static void printException(const std::exception &e) {
            const PkError * pkErr = castFromStdException(e);
            if (pkErr) {
                qDebug() << pkErr->error() << pkErr->reason();
            } else {
                qDebug() << pkErr->what();
            }
        }
    private:
        PackageKit::Transaction::Error m_error;
        QString m_reason;
    };

    // packagekit wrappers. might throw PkError on error.
    QCoro::Task<PkPackages> searchNames(const QString & search, PackageKit::Transaction::Filters filters = PackageKit::Transaction::FilterNone);
    QCoro::Task<PkPackages> resolve(const QString & search, PackageKit::Transaction::Filters filters = PackageKit::Transaction::FilterNone);
    QCoro::Task<void> installPackage(const PkPackage & package);
    QCoro::Task<void> installPackage(const QString & packageId);
}
