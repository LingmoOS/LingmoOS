// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef HELPER_H
#define HELPER_H
#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#include <QObject>

using namespace KAuth;

class SMARTHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    ActionReply smartctl(const QVariantMap &args);
};

#endif // HELPER_H
