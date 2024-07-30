/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Lingmo/Containment>

class TaskPanel : public Lingmo::Containment
{
    Q_OBJECT

public:
    TaskPanel(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    Q_INVOKABLE void triggerTaskSwitcher() const;
};
