// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "containmentitem.h"

#include <QLoggingCategory>
#include <QQmlContext>
#include <QQmlEngine>

DS_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

DContainmentItem::DContainmentItem(QQuickItem *parent)
    : DAppletItem(parent)
{
}

DContainmentItem::~DContainmentItem()
{
}

DContainment *DContainmentItem::qmlAttachedProperties(QObject *object)
{
    auto applet = DAppletItem::qmlAttachedProperties(object);
    while (applet) {
        if (auto containment = qobject_cast<DContainment *>(applet)) {
            return containment;
        }
        applet = applet->parentApplet();
    }
    return nullptr;
}

DS_END_NAMESPACE
