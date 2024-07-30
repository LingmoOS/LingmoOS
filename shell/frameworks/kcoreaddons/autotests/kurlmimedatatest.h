/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KURLMIMEDATATEST_H
#define KURLMIMEDATATEST_H

#include "config-kdirwatch.h"
#include <QObject>

class KUrlMimeDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testURLList();
    void testOneURL();
    void testFromQUrl();
    void testMostLocalUrlList_data();
    void testMostLocalUrlList();
#if HAVE_QTDBUS
    void testSameProcessCopyPaste();
#endif
    void initTestCase();
};

#endif
