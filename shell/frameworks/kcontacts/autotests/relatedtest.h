/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RELATEDTEST_H
#define RELATEDTEST_H

#include <QObject>

class RelatedTest : public QObject
{
    Q_OBJECT
public:
    explicit RelatedTest(QObject *parent = nullptr);
    ~RelatedTest() override;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldAssignValue();
    void shouldAssignExternal();
    void shouldSerialized();
    void shouldEqualRelated();
    void shouldParseRelated();
    void shouldParseWithoutRelated();
    void shouldCreateVCard4();
    void shouldCreateVCardWithTwoRelated();
    void shouldCreateVCardWithParameters();
    void shouldNotExportInVcard3();
    void shouldParseRelatedWithArgument();
};

#endif // RELATEDTEST_H
