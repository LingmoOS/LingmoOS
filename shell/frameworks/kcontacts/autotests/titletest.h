/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TITLETEST_H
#define TITLETEST_H

#include <QObject>

class TitleTest : public QObject
{
    Q_OBJECT
public:
    explicit TitleTest(QObject *parent = nullptr);
    ~TitleTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualTitle();
    void shouldParseTwoTitles();
    void shouldParseTitle();
    void shouldParseWithoutTitle();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoTitle();
    void shouldCreateVCardWithParameters();
    void shouldGenerateTitleForVCard3();
};

#endif // TITLETEST_H
