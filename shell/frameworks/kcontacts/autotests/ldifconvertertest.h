/*
    This file is part of libkabc.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LDIFCONVERTERTEST_H
#define LDIFCONVERTERTEST_H

#include <QObject>

class LDifConverterTest : public QObject
{
    Q_OBJECT
public:
    explicit LDifConverterTest(QObject *parent = nullptr);
    ~LDifConverterTest() override;

private Q_SLOTS:
    // Import
    void shouldImportEmail();
    void shouldImportMultiEmails();
    void shouldImportStandardBirthday();
    void shouldImportStandardBirthdayWithoutYear();
    void shouldImportTheBatsBirthday();
    void shouldImportTheBatsEmails();
    void shouldImportTitle();
    void shouldImportWorkStreet();
    void shouldImportContactGroup();
    void shouldImportMultiEntries();
    void shouldImportGroupAndAddress();

    // Export
    void shouldExportEmail();
    void shouldExportBirthday();
    void shouldExportBirthdayWithoutYear();
    void shouldExportTitle();
    void shouldExportMultiEntries();
    void shouldExportGroup();
    void shouldExportWorkStreet();
    void shouldExportMultiEmails();
    void shouldExportFullName();

    // Garbage tests
    void testGarbage();
};

#endif // LDIFCONVERTERTEST_H
