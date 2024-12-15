// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <DObject>
#include <QQuickItem>

DS_BEGIN_NAMESPACE

class DApplet;
class DAppletItemPrivate;
/**
 * @brief UI插件实例项
 */
class DS_SHARE DAppletItem : public QQuickItem, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DAppletItem)
    Q_PROPERTY(DApplet *applet READ applet CONSTANT)
public:
    explicit DAppletItem(QQuickItem *parent = nullptr);
    virtual ~DAppletItem() override;

    DApplet *applet() const;

    static DApplet *qmlAttachedProperties(QObject *object);
};

DS_END_NAMESPACE

QML_DECLARE_TYPEINFO(DS_NAMESPACE::DAppletItem, QML_HAS_ATTACHED_PROPERTIES)
