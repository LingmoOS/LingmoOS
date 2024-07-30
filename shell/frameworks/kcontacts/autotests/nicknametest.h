/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NICKNAMETEST_H
#define NICKNAMETEST_H

#include <QObject>

class NickNameTest : public QObject
{
    Q_OBJECT
public:
    explicit NickNameTest(QObject *parent = nullptr);
    ~NickNameTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualNickName();
    void shouldParseNickName();
    void shouldParseWithoutNickName();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoNickName();
    void shouldCreateVCardWithParameters();
    void shouldGenerateNickNameForVCard3();
};

#endif // NICKNAMETEST_H
