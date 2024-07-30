/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_PART_P_H
#define _KPARTS_PART_P_H

#include "part.h"
#include "partbase_p.h"
// KF
#include <KPluginMetaData>
// Qt
#include <QPointer>
#include <QWidget>

namespace KParts
{
class PartPrivate : public PartBasePrivate
{
public:
    Q_DECLARE_PUBLIC(Part)

    explicit PartPrivate(Part *qq, const KPluginMetaData &data)
        : PartBasePrivate(qq)
        , m_metaData(data)
        , m_autoDeleteWidget(true)
        , m_autoDeletePart(true)
        , m_manager(nullptr)
    {
    }

    ~PartPrivate() override
    {
    }

    const KPluginMetaData m_metaData;
    bool m_autoDeleteWidget;
    bool m_autoDeletePart;
    PartManager *m_manager;
    QPointer<QWidget> m_widget;
};

} // namespace

#endif
