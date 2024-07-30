/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "cgroup_data_model.h"

#include "processcore_export.h"

namespace KSysGuard
{
class PROCESSCORE_EXPORT ApplicationDataModel : public CGroupDataModel
{
    Q_OBJECT
public:
    ApplicationDataModel(QObject *parent = nullptr);
    bool filterAcceptsCGroup(const QString &id) override;
};

}
