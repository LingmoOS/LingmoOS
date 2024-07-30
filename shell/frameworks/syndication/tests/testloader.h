/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_TESTSYNDICATION_H
#define TEST_TESTSYNDICATION_H

#include "loader.h"

#include <QObject>

class TestLibSyndication : public QObject
{
    Q_OBJECT

public:
    TestLibSyndication(const QString &url);

public Q_SLOTS:

    void slotLoadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode error);
};

#endif // TEST_TESTSYNDICATION_H
