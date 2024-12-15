// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ddfproccessor_5_9_0_1.h"
//5.9.0.3之后在默认head中添加了QRecF类型的数据(对所有图元都应该有一个矩形),同时开始支持组合信息的保存
class DRAWLIB_EXPORT DdfUnitProccessor_5_9_0_3: public DdfUnitProccessor_5_9_0_1
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_5_9_0_3(QObject *parent = nullptr);
    int version() override;

protected:
    PageContextData pageContextToData(PageContext *context) override;

    void serializationUnitTree(QDataStream &out, const UnitTree_Comp &tree, int count, DdfHander *hander) override;
    UnitTree_Comp deserializationUnitTree(QDataStream &in, int count, bool &stop, QString &stopReason, DdfHander *hander) override;

    virtual void     serializationUnitTree_Loop(QDataStream &out, const UnitTree_Comp &tree, int count, int &currentProgress, DdfHander *hander) ;
    virtual UnitTree_Comp deserializationUnitTree_Loop(QDataStream &in, int count, int &currentProgress, bool &stop, QString &stopReason, DdfHander *hander);

protected:

    //bool     isSupportedSaveType(int type, DdfHander *hander) override;

    void     serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander) override;
    UnitHead deserializationUnitHead(QDataStream &in, DdfHander *hander) override;

    void     serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander) override;
    UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander) override;
};

//DdfUnitProccessor_5_9_0_3版本的ddf文件被加载到之后版本的程序时，如果遇到旧的画笔图元，需要在解析时进行用户交互抉择是否加载
class DRAWLIB_EXPORT DdfUnitProccessor_5_9_0_3_Compatibel: public DdfUnitProccessor_5_9_0_3
{
    Q_OBJECT
public:
    using DdfUnitProccessor_5_9_0_3::DdfUnitProccessor_5_9_0_3;

    UnitTree_Comp deserializationUnitTree_Loop(QDataStream &in, int count, int &currentProgress, bool &stop,
                                               QString &stopReason, DdfHander *hander) override;
};
