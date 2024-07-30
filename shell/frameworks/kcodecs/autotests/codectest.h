/*
    SPDX-FileCopyrightText: 2010 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CODECTEST_H
#define CODECTEST_H

#include <QObject>

class CodecTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCodecs();
    void testCodecs_data();
};

#endif
