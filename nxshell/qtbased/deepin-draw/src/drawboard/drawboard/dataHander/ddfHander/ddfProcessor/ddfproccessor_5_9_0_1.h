// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_9_0_1_H
#define DDFPROCCESSOR_5_9_0_1_H
#include "ddfproccessor_5_8_0_84.h"

//5.9.0.1之后添加了图元的transform保存
class DRAWLIB_EXPORT DdfUnitProccessor_5_9_0_1: public DdfUnitProccessor_5_8_0_84
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_9_0_1(QObject *parent = nullptr);
    int version() override;

    void serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander) override;
    UnitHead deserializationUnitHead(QDataStream &in, DdfHander *hander) override;
};
#endif // DDFPROCCESSOR_5_9_0_1_H
