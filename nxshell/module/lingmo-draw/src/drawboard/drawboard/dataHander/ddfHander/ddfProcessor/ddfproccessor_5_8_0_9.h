// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_5_8_0_9_H
#define DDFPROCCESSOR_5_8_0_9_H
#include "ddfproccessor.h"

//添加了 [矩形属性,A0B0C0D0,version] 的ddf版本
class DRAWLIB_EXPORT DdfUnitProccessor_5_8_0_9: public DdfUnitProccessor_5_8_0_Compatibel
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_8_0_9(QObject *parent = nullptr);
    int  version() override;
protected:
    void serializationDdfHead(QDataStream &out, const PageContextHead &ddfHead, DdfHander *hander) override;
    PageContextHead deserializationDdfHead(QDataStream &in, DdfHander *hander) override;

    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSOR_5_8_0_9_H
