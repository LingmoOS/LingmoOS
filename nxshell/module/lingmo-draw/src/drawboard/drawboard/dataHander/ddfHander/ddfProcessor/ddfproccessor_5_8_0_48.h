// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_48_H
#define DDFPROCCESSOR_5_8_0_48_H

#include "ddfproccessor_5_8_0_20.h"

//1030专业版5_8_0_48版本后将多保存图片图元的翻转信息,以解决翻转信息丢失的问题
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_48: public DdfUnitProccessor_5_8_0_20
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_48(QObject *parent = nullptr);
    int version() override;
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSOR_5_8_0_48_H
