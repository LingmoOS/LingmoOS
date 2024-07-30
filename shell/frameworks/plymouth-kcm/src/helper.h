/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef PLYMOUTH_HELPER_H
#define PLYMOUTH_HELPER_H

#include <KAuth/ActionReply>

using namespace KAuth;

class PlymouthHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    ActionReply save(const QVariantMap &map);
    ActionReply install(const QVariantMap &args);
    ActionReply uninstall(const QVariantMap &args);
};

#endif
