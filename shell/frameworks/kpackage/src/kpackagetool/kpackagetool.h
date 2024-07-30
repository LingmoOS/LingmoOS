/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGETOOL_H
#define PACKAGETOOL_H

#include "package.h"
#include "packagejob.h"
#include <QCoreApplication>

class QCommandLineParser;
class KJob;

namespace KPackage
{
class PackageToolPrivate;

class PackageTool : public QCoreApplication
{
    Q_OBJECT

public:
    PackageTool(int &argc, char **argv, QCommandLineParser *parser);
    ~PackageTool() override;

    void listPackages(const QString &kpackageType, const QString &path = QString());
    void showPackageInfo(const QString &pluginName);
    void showAppstreamInfo(const QString &pluginName);
    QString resolvePackageRootWithOptions();

private Q_SLOTS:
    void runMain();
    void packageInstalled(KPackage::PackageJob *job);
    void packageUninstalled(KPackage::PackageJob *job);

private:
    PackageToolPrivate *d;
};

}

#endif
