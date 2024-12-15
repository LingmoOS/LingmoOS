// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-treeland-output-manager-v1.h"
#include <QtWaylandClient/QWaylandClientExtension>

class TreelandOutputWatcher : public QWaylandClientExtensionTemplate<TreelandOutputWatcher>, public QtWayland::treeland_output_manager_v1
{
    Q_OBJECT
public:
    TreelandOutputWatcher(QObject *parent = nullptr);
    ~TreelandOutputWatcher();

protected:
    void treeland_output_manager_v1_primary_output(const QString &output_name) override;
};
