/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ORGTEST_H
#define ORGTEST_H

#include <QObject>

class OrgTest : public QObject
{
    Q_OBJECT
public:
    explicit OrgTest(QObject *parent = nullptr);
    ~OrgTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualOrg();
    void shouldParseOrg();
    void shouldParseWithoutOrg();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoOrg();
    void shouldCreateVCardWithParameters();
    void shouldGenerateOrgForVCard3();
    void shouldParseVcardWithTwoOrg();
    void shouldCreateVCardWithSemiColon();
};

#endif // ORGTEST_H
