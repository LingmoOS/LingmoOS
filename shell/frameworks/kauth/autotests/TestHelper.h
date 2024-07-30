/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <QObject>

#include <actionreply.h>

using namespace KAuth;

class TestHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    ActionReply echoaction(QVariantMap args);
    ActionReply standardaction(QVariantMap args);
    ActionReply longaction(QVariantMap args);
    ActionReply failingaction(QVariantMap args);
};

#endif
