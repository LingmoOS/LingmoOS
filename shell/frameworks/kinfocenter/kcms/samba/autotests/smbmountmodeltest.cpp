/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QAbstractItemModelTester>
#include <QTest>

#include "../smbmountmodel.h"

class SmbMountModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel()
    {
        QAbstractItemModelTester tester(new SmbMountModel);
    }
};

QTEST_GUILESS_MAIN(SmbMountModelTest)

#include "smbmountmodeltest.moc"
