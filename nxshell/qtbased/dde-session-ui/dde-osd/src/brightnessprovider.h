// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BRIGHTNESSPROVIDER_H
#define BRIGHTNESSPROVIDER_H

#include "abstractosdprovider.h"

#include "org_deepin_dde_display1.h"

#include "common.h"
/*!
 * \~chinese \class BrightnessProvider
 * \~chinese \brief 调整屏幕亮度的通知类
 */
class BrightnessProvider : public AbstractOSDProvider
{
    Q_OBJECT
public:
    explicit BrightnessProvider(QObject *parent = nullptr);

    // model
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    // delegate
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString pixmapName() const;

private:
    org::deepin::dde::Display1 *m_displayInter;
};

#endif // BRIGHTNESSPROVIDER_H
