/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ResourceLocatorUrlTest_H
#define ResourceLocatorUrlTest_H

#include <QObject>

class ResourceLocatorUrlTest : public QObject
{
    Q_OBJECT
public:
    explicit ResourceLocatorUrlTest(QObject *parent = nullptr);
    ~ResourceLocatorUrlTest() override;

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualResourceLocatorUrl();
    void shouldParseResourceLocatorUrl();
    void shouldGenerateVCard4();
    void shouldGenerateVCardWithParameter();
    void shouldGenerateVCard3();
};

#endif // ResourceLocatorUrlTest_H
