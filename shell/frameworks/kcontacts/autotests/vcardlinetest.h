/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VCARDLINETEST_H
#define VCARDLINETEST_H

#include <QObject>

class VCardLineTest : public QObject
{
    Q_OBJECT
public:
    explicit VCardLineTest(QObject *parent = nullptr);
    ~VCardLineTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValues();
    void shouldCopyValue();
    void shouldEqualValue();
};

#endif // VCARDLINETEST_H
