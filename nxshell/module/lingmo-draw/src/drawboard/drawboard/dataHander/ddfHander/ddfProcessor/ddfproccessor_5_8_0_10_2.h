// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_10_2_H
#define DDFPROCCESSOR_5_8_0_10_2_H
#include "ddfproccessor_5_8_0_10_1.h"

//修复了EDdf5_8_0_10_1版本未保存画笔图元path的问题
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_10_2: public DdfUnitProccessor_5_8_0_10_1
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_10_2(QObject *parent = nullptr);
    int version() override;
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSOR_5_8_0_10_2_H
