/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <AppStreamQt/pool.h>
#include <PackageKit/Daemon>
#include <QCoreApplication>
#include <QDebug>

using namespace AppStream;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Q_ASSERT(app.arguments().count() == 2);

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("appstream"));

    const QString componentName = url.host();
    if (componentName.isEmpty()) {
        qWarning() << "wrongly formatted URI" << url;
        return 1;
    }

    Pool pool;
    auto b = pool.load();
    Q_ASSERT(b);
    const auto components = pool.componentsById(componentName).toList();
    if (components.isEmpty()) {
        qWarning() << "couldn't find" << componentName;
        return 1;
    }

    QStringList packages;
    for (const auto &component : components) {
        packages += component.packageNames();
    }
    packages.removeDuplicates();

    if (packages.isEmpty()) {
        qWarning() << "no packages to install";
        return 1;
    }

    auto resolveTransaction = PackageKit::Daemon::global()->resolve(packages, PackageKit::Transaction::FilterArch);
    Q_ASSERT(resolveTransaction);

    QHash<QString, QString> pkgs;

    QObject::connect(resolveTransaction,
                     &PackageKit::Transaction::package,
                     resolveTransaction,
                     [&pkgs](PackageKit::Transaction::Info info, const QString &packageID, const QString & /*summary*/) {
                         if (info == PackageKit::Transaction::InfoAvailable)
                             pkgs[PackageKit::Daemon::packageName(packageID)] = packageID;
                         qDebug() << "resolved package" << info << packageID;
                     });
    QObject::connect(resolveTransaction, &PackageKit::Transaction::finished, resolveTransaction, [&app, &pkgs](PackageKit::Transaction::Exit status) {
        if (status != PackageKit::Transaction::ExitSuccess) {
            qWarning() << "resolve failed" << status;
            QCoreApplication::exit(1);
            return;
        }
        QStringList pkgids = pkgs.values();

        if (pkgids.isEmpty()) {
            qDebug() << "Nothing to install";
            QCoreApplication::exit(0);
        } else {
            qDebug() << "installing..." << pkgids;
            pkgids.removeDuplicates();
            auto installTransaction = PackageKit::Daemon::global()->installPackages(pkgids);
            QObject::connect(installTransaction, &PackageKit::Transaction::finished, &app, [](PackageKit::Transaction::Exit status) {
                qDebug() << "install finished" << status;
                QCoreApplication::exit(status == PackageKit::Transaction::ExitSuccess ? 0 : 1);
            });
        }
    });
    return app.exec();
}
