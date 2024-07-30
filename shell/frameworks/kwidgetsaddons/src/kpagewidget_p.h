/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPAGEWIDGET_P_H
#define KPAGEWIDGET_P_H

#include "kpageview_p.h"
#include "kpagewidget.h"

class KPageWidgetModel;
class KPageWidgetPrivate : public KPageViewPrivate
{
    Q_DECLARE_PUBLIC(KPageWidget)
protected:
    KPageWidgetPrivate(KPageWidget *qq);

    KPageWidgetModel *model() const
    {
        return static_cast<KPageWidgetModel *>(KPageViewPrivate::model);
    }

    void slotCurrentPageChanged(const QModelIndex &, const QModelIndex &);
};

#endif // KPAGEWIDGET_P_H
