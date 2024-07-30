/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GENDERTEST_H
#define GENDERTEST_H

#include <QObject>

class GenderTest : public QObject
{
    Q_OBJECT
public:
    explicit GenderTest(QObject *parent = nullptr);
    ~GenderTest() override;

private:
    QByteArray createCard(const QByteArray &gender);

private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualGender();
    void shouldParseGender();
    void shouldParseGender_data();
    void shouldExportOnlyGenderWithoutCommentGender();
    void shouldExportEmptyGender();
    void shouldExportOnlyGenderWithCommentGender();
    void shouldExportOnlyGenderWithoutTypeCommentGender();
    void shouldNotExportInVcard3();
};

#endif // GENDERTEST_H
