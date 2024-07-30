// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include "firewalldlogmodel.h"

FirewalldLogModel::FirewalldLogModel(QObject *parent)
    : LogListModel(parent)
{
}

void FirewalldLogModel::addRawLogs(const QStringList &rawLogsList)
{
    Q_UNUSED(rawLogsList);
    // TODO: Implement-me. look at UfwLogModel for inspiration.
}
