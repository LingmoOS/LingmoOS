// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pkutils.h"

// PackageKit-Qt
#include <Daemon>

#include <QCoroSignal>
#include <qcoroasyncgenerator.h>
#include <stdexcept>
#include <transaction.h>
#include <tuple>

QCoro::Task<PKUtils::PkPackages> PKUtils::searchNames(const QString & search, PackageKit::Transaction::Filters filters)
{
    PackageKit::Transaction * tx = PackageKit::Daemon::searchNames(search, filters);

    PkPackages results;
    QObject::connect(tx, &PackageKit::Transaction::package, [&results](PackageKit::Transaction::Info info, const QString &packageID, const QString &summary){
        results.append(std::make_tuple(info, packageID, summary));
    });

    const auto [status, runtime] = co_await qCoro(tx, &PackageKit::Transaction::finished);
    qDebug() << "searchNames Coro" << status << runtime;

    co_return results;
}

QCoro::Task<PKUtils::PkPackages> PKUtils::resolve(const QString & packageName, PackageKit::Transaction::Filters filters)
{
    PackageKit::Transaction * tx = PackageKit::Daemon::resolve(packageName);

    PkPackages results;
    QObject::connect(tx, &PackageKit::Transaction::package, [&results](PackageKit::Transaction::Info info, const QString &packageID, const QString &summary){
        results.append(std::make_tuple(info, packageID, summary));
        qDebug() << packageID;
    });

    const auto [status, runtime] = co_await qCoro(tx, &PackageKit::Transaction::finished);
    qDebug() << "resolve Coro" << status << runtime;

    co_return results;
}

QCoro::Task<void> PKUtils::installPackage(const PkPackage & package)
{
    QString pkgId;
    std::tie(std::ignore, pkgId, std::ignore) = package;

    co_await installPackage(pkgId);

    co_return;
}

QCoro::Task<void> PKUtils::installPackage(const QString & packageId)
{
    PackageKit::Transaction * tx = PackageKit::Daemon::installPackage(packageId);

    PackageKit::Transaction::Error _error;
    QString _details;
    QObject::connect(tx, &PackageKit::Transaction::errorCode, [&_error, &_details](PackageKit::Transaction::Error error, const QString &details){
        _error = error;
        _details = details;
    });

    const auto [status, runtime] = co_await qCoro(tx, &PackageKit::Transaction::finished);
    qDebug() << "installPackage Coro" << status << runtime;

    if (status != PackageKit::Transaction::ExitSuccess) {
        throw PkError(_error, _details);
    }

    co_return;
}
