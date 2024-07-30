/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FALLBACKPACKAGETEST_H
#define FALLBACKPACKAGETEST_H

#include <QTest>

#include "kpackage/package.h"

class FallbackPackageTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void beforeFallback();
    void afterFallback();
    void cycle();

private:
    KPackage::Package m_pkg;
    KPackage::Package m_fallbackPkg;
    QString m_packagePath;
    QString m_fallPackagePath;
};

#endif
