// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_16_1_H
#define DDFPROCCESSOR_5_8_0_16_1_H
#include "ddfproccessor_5_8_0_10_2.h"

//为了优化图片的保存速度修改了保存图片时由原先的直接保存QImage该成保存QByteArry
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_16_1: public DdfUnitProccessor_5_8_0_10_2
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_16_1(QObject *parent = nullptr);
    int version() override;

protected:
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSOR_5_8_0_16_1_H
