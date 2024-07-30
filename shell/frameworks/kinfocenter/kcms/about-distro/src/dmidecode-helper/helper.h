// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#pragma once

#include <KAuth/ActionReply>

class DMIDecodeHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply systeminformation(const QVariantMap &args);
};
