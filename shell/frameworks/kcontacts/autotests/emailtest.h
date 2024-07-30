/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EMAILTEST_H
#define EMAILTEST_H

#include <QObject>

class EmailTest : public QObject
{
    Q_OBJECT
public:
    explicit EmailTest(QObject *parent = nullptr);
    ~EmailTest() override;

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualEmail();
    void shouldParseEmailVCard();
    void shouldParseEmailVCardWithoutEmail();
    void shouldParseEmailVCardWithMultiEmails();
    void shouldExportVcard();
};

#endif // EMAILTEST_H
