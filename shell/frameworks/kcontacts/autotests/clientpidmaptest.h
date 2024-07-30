/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLIENTPIDMAPTEST_H
#define CLIENTPIDMAPTEST_H

#include <QObject>

class ClientPidMapTest : public QObject
{
    Q_OBJECT
public:
    explicit ClientPidMapTest(QObject *parent = nullptr);
    ~ClientPidMapTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualClientPidMap();
    void shouldParseClientPidMap();
    void shouldParseWithoutClientPidMap();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoClientPidMap();
    void shouldCreateVCardWithParameters();
    void shouldGenerateClientPidMapForVCard3();
};

#endif // CLIENTPIDMAPTEST_H
