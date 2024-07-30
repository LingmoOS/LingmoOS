/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ROLETEST_H
#define ROLETEST_H

#include <QObject>

class RoleTest : public QObject
{
    Q_OBJECT
public:
    explicit RoleTest(QObject *parent = nullptr);
    ~RoleTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualRole();
    void shouldParseRole();
    void shouldParseWithoutRole();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoRole();
    void shouldCreateVCardWithParameters();
    void shouldGenerateRoleForVCard3();
};

#endif // ROLETEST_H
