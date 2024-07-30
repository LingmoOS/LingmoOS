/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_TESTS_TESTPERSONIMPL_H
#define SYNDICATION_TESTS_TESTPERSONIMPL_H

#include <QTest>

class TestPersonImpl : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void fromString();
};

#endif // SYNDICATION_TESTS_TESTPERSONIMPL_H
