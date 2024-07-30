/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LOADERUTILTEST_H
#define LOADERUTILTEST_H

#include <QObject>

class LoaderUtilTest : public QObject
{
    Q_OBJECT
public:
    explicit LoaderUtilTest(QObject *parent = nullptr);
    ~LoaderUtilTest() override = default;
private Q_SLOTS:
    void testParsing_data();
    void testParsing();
};

#endif // LOADERUTILTEST_H
