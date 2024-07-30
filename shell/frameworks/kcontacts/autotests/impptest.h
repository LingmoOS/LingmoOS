/*
    This file is part of libkabc.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPPTEST_H
#define IMPPTEST_H

#include <QObject>

class ImppTest : public QObject
{
    Q_OBJECT
public:
    explicit ImppTest(QObject *parent = nullptr);
    ~ImppTest() override;

private Q_SLOTS:
    void shouldParseImppVcard4();
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldSerialized();
    void shouldEqualImpp();
    void shouldParseWithoutImpp();
    void shouldParseImpp();
    void shouldParseServiceType_data();
    void shouldParseServiceType();
    void shouldExportEmptyType();

    void shouldExportType_data();
    void shouldExportType();

    void shouldExportWithParameters();
    void shouldShouldNotExportTwiceServiceType();

    void testProtocolInformation();
};

#endif // IMPPTEST_H
