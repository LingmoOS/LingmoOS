/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2017 Dominik Haumann <dhaumann@kde.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_CONFIG_INTERFACE_TEST_H
#define KATE_CONFIG_INTERFACE_TEST_H

#include <QObject>

class KateConfigInterfaceTest : public QObject
{
    Q_OBJECT

public:
    KateConfigInterfaceTest();
    ~KateConfigInterfaceTest() override;

private Q_SLOTS:
    void testDocument();
    void testView();
};

#endif // KATE_CONFIG_INTERFACE_TEST_H
