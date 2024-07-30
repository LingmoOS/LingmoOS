// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef UFWLOGMODEL_H
#define UFWLOGMODEL_H

#include "loglistmodel.h"

class UfwLogModel : public LogListModel
{
    Q_OBJECT
public:
    explicit UfwLogModel(QObject *parent);
    void addRawLogs(const QStringList &rawLogsList) override;
};

#endif
