// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDFPROCCESSOR_H
#define DDFPROCCESSOR_H
#include "sitemdata.h"
#include "ddfhander.h"

#include <QObject>
#include <QDataStream>

class PageContext;
class DRAWLIB_EXPORT DdfUnitProccessor: public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual int version() = 0;

    bool     save(PageContext *context, const QString &file, DdfHander *hander);
    PageContextData load(const QString &file, DdfHander *hander);

    virtual void initForHander(DdfHander *hander);
    virtual void clearForHander(DdfHander *hander);

    static int  getDdfVersion(const QString &file);
    static bool isVolumeSpaceAvailabel(const QString &desFile, const int needBytesSize);

    virtual PageContextData pageContextToData(PageContext *context) = 0;
    virtual void pageContextLoadData(PageContext *pContext, const PageContextData &data) = 0;

protected:
    virtual bool saveTo(const PageContextData &data,
                        const QString &file,
                        DdfHander *hander) = 0;

    virtual PageContextData loadFrom(const QString &file,
                                     DdfHander *hander) = 0;
};

using UnitHead = UnitHead;
//using UnitData = UnitData;
using UnitTail = UnitTail;

class DdfUnitProccessor_chaos: public DdfUnitProccessor
{
    Q_OBJECT
public:
    Q_INVOKABLE DdfUnitProccessor_chaos(QObject *parent = nullptr);
    int  version() override;

protected:
    //第一级别的重载 会调用到第二级别的重载函数
    bool saveTo(const PageContextData &data,
                const QString &file,
                DdfHander *hander) override;
    PageContextData loadFrom(const QString &file,
                             DdfHander *hander) override;

    PageContextData pageContextToData(PageContext *context) override;
    void pageContextLoadData(PageContext *pContext, const PageContextData &data) override;

protected:
    //第二级别的重载
    virtual void serializationDdfHead(QDataStream &out, const PageContextHead &ddfHead, DdfHander *hander);
    virtual PageContextHead deserializationDdfHead(QDataStream &in, DdfHander *hander);

    virtual void serializationUnitTree(QDataStream &out, const UnitTree_Comp &tree, int count, DdfHander *hander);
    virtual UnitTree_Comp deserializationUnitTree(QDataStream &in, int count, bool &stop, QString &stopReason, DdfHander *hander);

    virtual qint64 calTotalBytes(const PageContextData &outObj, DdfHander *hander);
    bool    isSupportedSaveType(int type);
protected:
    //第三级别的重载 process unit
    virtual void serializationUnit(QDataStream &out, const Unit &unit, DdfHander *hander);
    virtual Unit deserializationUnit(QDataStream &in, DdfHander *hander);
protected:
    //第四级别的重载 process unit中的 head、data、tail
    virtual void serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander);
    virtual void serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander);
    virtual void serializationUnitTail(QDataStream &out, const UnitTail &tail, DdfHander *hander);

    virtual UnitHead deserializationUnitHead(QDataStream &in, DdfHander *hander);
    virtual UnitData deserializationUnitData(QDataStream &in, int type, DdfHander *hander);
    virtual UnitTail deserializationUnitTail(QDataStream &in, DdfHander *hander);

    QString  spaceNotEnoughMessage();

protected:
    QSet<int> supTypes;
};
using DdfUnitProccessor_5_8_0 = DdfUnitProccessor_chaos;

//DdfUnitProccessor_5_8_0_Compatibel是为了保证兼容老版本ddf文件所实现的中间类
//DdfUnitProccessor_5_8_0及之后版本的ddf文件被最新画板程序加载时，需要进行一些兼容性的处理：
//1.解析UnitTree_Comp的过程中(deserializationUnitTree函数)如果遇到旧的模糊图元数据或者旧的画笔数据时需要用户进行交互决定是否加载他们
//2.最新的pContext加载PageContextData数据的方式发生了变化，所以需要重载pageContextLoadData进行处理
class DdfUnitProccessor_5_8_0_Compatibel: public DdfUnitProccessor_5_8_0
{
    Q_OBJECT
public:
    using DdfUnitProccessor_5_8_0::DdfUnitProccessor_5_8_0;

    void pageContextLoadData(PageContext *pContext, const PageContextData &data) override;
    void initForHander(DdfHander *hander) override;
    void clearForHander(DdfHander *hander) override;

    UnitTree_Comp deserializationUnitTree(QDataStream &in, int count, bool &stop, QString &stopReason, DdfHander *hander) override;
    Unit deserializationUnit(QDataStream &in, DdfHander *hander) override;
protected:
    int waitForHowToHanderBlurUnit(DdfHander *hander);
    int waitForHowToHanderOldPenUnit(DdfHander *hander);
protected:
    QSet<DdfHander *> _handerMeetBlur;
    QSet<DdfHander *> _handerMeetPen;
};

#endif // DDFPROCCESSOR_H
