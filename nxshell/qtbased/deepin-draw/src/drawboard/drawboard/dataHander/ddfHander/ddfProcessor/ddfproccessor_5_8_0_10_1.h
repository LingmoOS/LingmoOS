// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_10_1_H
#define DDFPROCCESSOR_5_8_0_10_1_H
#include "ddfproccessor_5_8_0_9.h"

//添加了直线起点终点的版本（但这个版本在保存时漏掉保存了pen图元的保存 所以这个版本保存的ddf被加载后pen图元显示会问题）
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_10_1: public DdfUnitProccessor_5_8_0_9
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_10_1(QObject *parent = nullptr);

    int version() override;

    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSOR_5_8_0_10_1_H
