// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_84_H
#define DDFPROCCESSOR_5_8_0_84_H

#include "ddfproccessor_5_8_0_48.h"

//1040 sprint-1引入模糊,仅模糊图片,需要记录图元的模糊信息(做成了一种属性,而非模糊图元了)(实际发布班为5.9.0.1)
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_84: public DdfUnitProccessor_5_8_0_48
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_84(QObject *parent = nullptr);
    int version() override;

protected:

    void serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander) override;
    UnitHead deserializationUnitHead(QDataStream &in, DdfHander *hander) override;

    //bool isSupportedSaveType(int type, DdfHander *hander) override;

};
#endif // DDFPROCCESSOR_5_8_0_84_H
