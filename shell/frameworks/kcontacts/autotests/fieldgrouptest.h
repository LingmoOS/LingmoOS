/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FieldGroupTest_H
#define FieldGroupTest_H

#include <QObject>

class FieldGroupTest : public QObject
{
    Q_OBJECT
public:
    explicit FieldGroupTest(QObject *parent = nullptr);
    ~FieldGroupTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualFieldGroup();
    void shouldParseFieldGroup();
    void shouldParseWithoutFieldGroup();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoLang();
    void shouldCreateVCardWithParameters();
    void shouldNotGenerateFieldGroupForVCard3();
};

#endif // FieldGroupTest_H
