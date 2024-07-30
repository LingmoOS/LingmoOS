// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef FIREWALLDLOGMODEL_H
#define FIREWALLDLOGMODEL_H

#include "loglistmodel.h"

class FirewalldLogModel : public LogListModel
{
    Q_OBJECT
public:
    explicit FirewalldLogModel(QObject *parent);
    void addRawLogs(const QStringList &rawLogsList) override;
};

#endif
