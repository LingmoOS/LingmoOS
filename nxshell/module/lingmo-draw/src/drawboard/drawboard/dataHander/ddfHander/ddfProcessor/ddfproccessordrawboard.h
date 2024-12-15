// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSORDARWBORAD_H
#define DDFPROCCESSORDARWBORAD_H
#include "ddfProccessor_5_9_9_0.h"

//1.不再使用UnitTree进行ddf保存，作为替代，使用UnitList及Unit中的chilren来使用
//(这带来一个问题，因为以前的ddf文件其图元都是顶层图元，关于组合图元的坐标大小信息将会丢失，
//需要通过其子图元进行重新计算出来，参见DdfUnitProccessor_5_8_0_Compatibel::pageContextLoadData进行了兼容实现)

//2.添加了图层图元layer
class DRAWLIB_EXPORT DdfProccessorDrawBoard: public DdfUnitProccessor_5_10_0
{
public:
    explicit DdfProccessorDrawBoard(QObject *parent = nullptr);

    int  version() override;

    bool saveTo(const PageContextData &data,
                const QString &file,
                DdfHander *hander) override;
    PageContextData loadFrom(const QString &file,
                             DdfHander *hander) override;

    PageContextData pageContextToData(PageContext *context) override;
    void pageContextLoadData(PageContext *pContext, const PageContextData &data) override;

protected:
    qint64 calTotalBytes(const PageContextData &data, DdfHander *hander) override;
    virtual void serializationUnits(QDataStream &out, const QList<Unit> &units, DdfHander *hander) ;
    virtual QList<Unit> deserializationUnits(QDataStream &in, int count, bool &stop, QString &stopReason, DdfHander *hander);
    Unit deserializationUnit(QDataStream &in, DdfHander *hander) override;

protected:
    void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

#endif // DDFPROCCESSORDARWBORAD_H
