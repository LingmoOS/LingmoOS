/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LANGTEST_H
#define LANGTEST_H

#include <QObject>

class LangTest : public QObject
{
    Q_OBJECT
public:
    explicit LangTest(QObject *parent = nullptr);
    ~LangTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualLanguage();
    void shouldParseLanguage();
    void shouldParseWithoutLanguage();
    void shouldCreateVCard();
    void shouldCreateVCardWithTwoLang();
    void shouldCreateVCardWithParameters();
    void shouldNotGenerateLangForVCard3();
};

#endif // LANGTEST_H
