/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PACKAGESTRUCTURETEST_H
#define PACKAGESTRUCTURETEST_H

#include <QTest>

#include "kpackage/package.h"

class PackageStructureTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void validStructures();
    void validPackages();
    void wallpaperPackage();
    void mutateAfterCopy();
    void emptyContentsPrefix();
    void directories();
    void requiredDirectories();
    void files();
    void requiredFiles();
    void path();
    void required();
    void mimeTypes();
    void customContent();

private:
    KPackage::Package ps;
    QString m_packagePath;
};

#endif
