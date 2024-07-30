/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMPORTEXPORTVCARDTEST_H
#define IMPORTEXPORTVCARDTEST_H

#include <QObject>

class ImportExportVCardTest : public QObject
{
    Q_OBJECT
public:
    explicit ImportExportVCardTest(QObject *parent = nullptr);
    ~ImportExportVCardTest() override;
private Q_SLOTS:
    void shouldExportFullTestVcard4();
    void shouldExportMiscElementVcard4();
    void shouldExportMemberElementVcard4();
    void shouldExportMissingNewlineVcard4();
};

#endif // IMPORTEXPORTVCARDTEST_H
