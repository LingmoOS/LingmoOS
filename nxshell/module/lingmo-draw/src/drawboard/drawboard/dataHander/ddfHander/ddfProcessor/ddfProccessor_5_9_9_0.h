// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "ddfProccessor_5_9_0_3.h"

class DRAWLIB_EXPORT DdfUnitProccessor_5_9_9_0: public DdfUnitProccessor_5_9_0_3_Compatibel
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_9_9_0(QObject *parent = nullptr);
    int  version() override;

protected:
    Unit deserializationUnit(QDataStream &in, DdfHander *hander) override;
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

class DRAWLIB_EXPORT DdfUnitProccessor_5_10_0: public DdfUnitProccessor_5_9_9_0
{
public:
    Q_OBJECT
public:
    DdfUnitProccessor_5_10_0(QObject *parent = nullptr);
    int version() override;
protected:
    Unit deserializationUnit(QDataStream &in, DdfHander *hander) override;
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};
