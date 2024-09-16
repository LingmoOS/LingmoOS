// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "appletitem.h"
#include "containment.h"

DS_BEGIN_NAMESPACE

class DContainment;
class DContainmentItemPrivate;
/**
 * @brief UI插件实例项
 */
class DS_SHARE DContainmentItem : public DAppletItem
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DContainmentItem)
public:
    explicit DContainmentItem(QQuickItem *parent = nullptr);
    virtual ~DContainmentItem() override;

    static DContainment *qmlAttachedProperties(QObject *object);
};

DS_END_NAMESPACE

QML_DECLARE_TYPEINFO(DS_NAMESPACE::DContainmentItem, QML_HAS_ATTACHED_PROPERTIES)
