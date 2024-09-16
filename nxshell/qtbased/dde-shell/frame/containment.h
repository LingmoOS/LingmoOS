// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"

Q_MOC_INCLUDE(<appletitemmodel.h>)

#include <DObject>
#include <QVariant>

DS_BEGIN_NAMESPACE

/**
 * @brief 容器插件
 */
class DContainmentPrivate;
class DAppletItemModel;
class DS_SHARE DContainment : public DApplet
{
    Q_OBJECT
    Q_PROPERTY(DAppletItemModel *appletItems READ appletItemModel CONSTANT)
    D_DECLARE_PRIVATE(DContainment)
public:
    explicit DContainment(QObject *parent = nullptr);
    virtual ~DContainment() override;

    DApplet *createApplet(const DAppletData &data);
    void removeApplet(DApplet *applet);

    QList<DApplet *> applets() const;
    QList<QObject *> appletItems();
    DAppletItemModel *appletItemModel() const;

    DApplet *applet(const QString &id) const;

    bool load() override;
    bool init() override;

protected:
    explicit DContainment(DContainmentPrivate &dd, QObject *parent = nullptr);
};

DS_END_NAMESPACE
