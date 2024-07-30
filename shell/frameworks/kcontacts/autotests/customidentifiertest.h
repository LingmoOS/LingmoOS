/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CUSTOMIDENTIFIERTEST_H
#define CUSTOMIDENTIFIERTEST_H

#include <QObject>

class CustomIdentifierTest : public QObject
{
    Q_OBJECT
public:
    explicit CustomIdentifierTest(QObject *parent = nullptr);
    ~CustomIdentifierTest() override;

private Q_SLOTS:
    void shouldExportVcard4();
    void shouldHaveEmptyCustoms();
    void shouldExportVcard3();
};

#endif // CUSTOMIDENTIFIERTEST_H
