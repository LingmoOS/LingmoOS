// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor.h"
#include "pagescene.h"
#include "dialog.h"
#include "layeritem.h"
#include "itemgroup.h"

#include <QStorageInfo>
#include <QStack>
#include <QApplication>
#include <QScreen>
static EDdfVersion s_getVersion(QDataStream &stream)
{
    EDdfVersion     version = EDdfUnknowed;
    if (stream.device() != nullptr) {
        quint32         headCheckFlag;
        qint64 pos = stream.device()->pos();

        stream.device()->seek(0);

        stream >> headCheckFlag;

        if (headCheckFlag == static_cast<quint32>(0xA0B0C0D0)) {
            int verVar;
            stream >> verVar;
            version = EDdfVersion(verVar);

        } else {
            version = EDdf5_8_0_Base;
        }
        //还原
        stream.device()->seek(pos);
    }

    return version;
}
void DdfUnitProccessor::initForHander(DdfHander *hander)
{
    Q_UNUSED(hander)
}

void DdfUnitProccessor::clearForHander(DdfHander *hander)
{
    Q_UNUSED(hander)
}

bool DdfUnitProccessor::save(PageContext *context, const QString &file, DdfHander *hander)
{
    PageContextData data = pageContextToData(context);
    return saveTo(data, file, hander);
}

PageContextData DdfUnitProccessor::load(const QString &file, DdfHander *hander)
{
    auto data = loadFrom(file, hander);
    return data;
}

int DdfUnitProccessor::getDdfVersion(const QString &file)
{
    EDdfVersion ver = EDdfUnknowed;
    QFile f(file);
    if (f.exists()) {
        if (f.open(QFile::ReadOnly)) {
            QDataStream s(&f);
            //获取版本号
            ver = s_getVersion(s);
        }
    }
    return ver;
}

bool DdfUnitProccessor::isVolumeSpaceAvailabel(const QString &desFile, const int needBytesSize)
{
    /* 如果空间不足那么提示 */
    QString dirPath = QFileInfo(desFile).absolutePath();
    QStorageInfo volume(dirPath);
    if (volume.isValid()) {
        qint64 availabelCount = volume.bytesAvailable();
        //qint64 bytesFree      = volume.bytesFree() ;
        if (!volume.isReady() || volume.isReadOnly() || availabelCount < needBytesSize) {
            return false;
        }
    }
    return true;
}


DdfUnitProccessor_chaos::DdfUnitProccessor_chaos(QObject *parent): DdfUnitProccessor(parent)
{
    supTypes.insert(RectType);
    supTypes.insert(EllipseType);
    supTypes.insert(TriangleType);
    supTypes.insert(PolygonalStarType);
    supTypes.insert(PolygonType);
    supTypes.insert(LineType);
    supTypes.insert(PenType);
    supTypes.insert(TextType);
    supTypes.insert(PictureType);
    supTypes.insert(BlurType);
}

int DdfUnitProccessor_chaos::version()
{
    return EDdf5_8_0_Base;
}

bool DdfUnitProccessor_5_8_0::saveTo(const PageContextData &data,
                                     const QString &file,
                                     DdfHander *hander)
{
    //auto data = defineDdfHead(datas, hander);

    int needBytesCount = calTotalBytes(data, hander);

    if (!isVolumeSpaceAvailabel(file, needBytesCount)) {
        hander->setError(DataHander::EInsufficientPartitionSpace, spaceNotEnoughMessage());
    } else {
        QFile writeFile(file);
        if (writeFile.open(QIODevice::WriteOnly)) {

            QDataStream out(&writeFile);

            serializationDdfHead(out, data.head, hander);

            serializationUnitTree(out, data.units_comp, data.head.unitCount, hander);

            writeFile.close();
        } else {
            hander->setError(writeFile.error(), writeFile.errorString());
        }
    }
    return hander->error() == 0;
}
Unit childTreeToGroupUnit(const QList<UnitTree_Comp> &childGroups, const Unit &parentUnit, const QList<Unit> &items)
{
    Unit unit = parentUnit;
    unit.head.dataType = GroupItemType;
    unit.usage = UnitUsage_Save;

    auto parentToSceneTrans = unit.head.trans.inverted() * QTransform::fromTranslate(unit.head.pos.x(), unit.head.pos.y());
    foreach (auto child, items) {
        auto childToSceneTrans = child.head.trans.inverted() * QTransform::fromTranslate(child.head.pos.x(), child.head.pos.y());
        auto childToParent = (childToSceneTrans * parentToSceneTrans.inverted());
//        child.head.pos = childToParent.map(QPointF(0, 0));
//        if (!child.head.pos.isNull())
//            childToParent *= QTransform::fromTranslate(-child.head.pos.x(), -child.head.pos.y());
        child.head.trans = childToParent * child.head.trans;
        unit.chidren.append(child);
    }
    foreach (auto p, childGroups) {
        auto childUt = childTreeToGroupUnit(p.childTrees, p.treeUnit, p.bzItems);
        auto childToSceneTrans = childUt.head.trans * QTransform::fromTranslate(childUt.head.pos.x(), childUt.head.pos.y());
        childUt.head.trans *= (childToSceneTrans * parentToSceneTrans.inverted());
        unit.chidren.append(childUt);
    }
    return unit;
}
Unit treeToLayerUnit(const UnitTree_Comp &tree)
{
    Unit unit(UnitUsage_Save);
    unit.head.dataType = LayerItemType;
    unit.head.pos = QPointF(0, 0);

    LayerUnitData data;
    unit.data.data.setValue<LayerUnitData>(data);
    unit.chidren.append(tree.bzItems);
    if (!tree.childTrees.isEmpty()) {
        foreach (auto gp, tree.childTrees) {
            unit.chidren.append(childTreeToGroupUnit(gp.childTrees, gp.treeUnit, gp.bzItems));
        }
    }

    return unit;
}

PageContextData DdfUnitProccessor_5_8_0::loadFrom(const QString &file, DdfHander *hander)
{
    QString resultInfo;

    PageContextData data;
    QFile readFile(file);
    if (readFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&readFile);
        data.head = deserializationDdfHead(in, hander);
        bool stop = false;
        data.units_comp = deserializationUnitTree(in, data.head.unitCount, stop, resultInfo, hander);
        readFile.close();
    }
    return data;
}

void DdfUnitProccessor_chaos::serializationUnit(QDataStream &out, const Unit &unit, DdfHander *hander)
{
    qApp->processEvents();
    serializationUnitHead(out, unit.head, hander);
    serializationUnitData(out, unit.head.dataType, unit.data, hander);
    serializationUnitTail(out, unit.tail, hander);
}

Unit DdfUnitProccessor_chaos::deserializationUnit(QDataStream &in, DdfHander *hander)
{
    qApp->processEvents();
    Unit unit(UnitUsage_Save);
    unit.head = deserializationUnitHead(in, hander);
    unit.data = deserializationUnitData(in, unit.head.dataType, hander);
    unit.tail = deserializationUnitTail(in, hander);
    return unit;
}

PageContextData DdfUnitProccessor_chaos::pageContextToData(PageContext *context)
{
    PageContext *pContext = context;

    PageContextData data;
    data.units_comp = pContext->scene()->getGroupTreeInfo();

    int count = 0;
    foreach (auto it, data.units_comp.bzItems) {
        if (isSupportedSaveType(it.head.dataType))
            ++count;
    }

    data.head.version = version();
    data.head.unitCount = count;
    data.head.rect = pContext->pageRect();

    return data;
}

void DdfUnitProccessor_chaos::pageContextLoadData(PageContext *pContext, const PageContextData &data)
{
    auto scene = pContext->scene();

    //禁止选中和自动赋予z值的操作(z值可以通过数据加载确定)
    scene->blockAssignZValue(true);
    scene->blockSelect(true);

    foreach (auto ut, data.units) {
        pContext->addItem(ut);
    }

    scene->blockSelect(false);
    scene->blockAssignZValue(false);
}

bool DdfUnitProccessor_chaos::isSupportedSaveType(int type)
{
    return supTypes.contains(type);
}

void DdfUnitProccessor_chaos::serializationDdfHead(QDataStream &out, const PageContextHead &ddfHead, DdfHander *hander)
{
    out << ddfHead.unitCount;
    out << ddfHead.rect;
}

void DdfUnitProccessor_5_8_0::serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander)
{
    out << head.headCheck[0];
    out << head.headCheck[1];
    out << head.headCheck[2];
    out << head.headCheck[3];
    out << head.dataType;
    out << head.dataLength;
    out << head.pen;
    out << head.brush;
    out << head.pos;
    out << head.rotate;
    out << head.zValue;
}

void DdfUnitProccessor_5_8_0::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    switch (type) {
    case RectType: {
        RectUnitData i = item.data.value<RectUnitData>();
        out << i.topLeft;
        out << i.bottomRight;
        i.xRedius = 0;
        i.yRedius = 0;
        break;
    }
    case EllipseType: {
        EllipseUnitData i = item.data.value<EllipseUnitData>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        break;
    }
    case TriangleType: {
        TriangleUnitData i = item.data.value<TriangleUnitData>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        break;
    }
    case PolygonalStarType: {
        StarUnit i = item.data.value<StarUnit>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        out << i.anchorNum;
        out << i.radius;
        break;
    }
    case PolygonType: {
        PolygonUnitData i = item.data.value<PolygonUnitData>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        out << i.pointNum;
        break;
    }
    case LineType: {
        LineUnitData i = item.data.value<LineUnitData>();
        out << i.point1;
        out << i.point2;
        break;
    }
    case PenType: {
        PenUnitData_Comp i = item.data.value<PenUnitData_Comp>();
        qint8 pentype = 0;
        out << pentype;
        out << i.path;
        out << i.arrow;
        break;
    }
    case TextType: {
        TextUnitData i = item.data.value<TextUnitData>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        out << i.font;
        out << i.manResizeFlag;
        out << i.content;
        break;
    }
    case PictureType: {
        ImageUnitData_Comp i = item.data.value<ImageUnitData_Comp>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        out << i.image;
        break;
    }
    case BlurType: {
        BlurUnitData_Comp i = item.data.value<BlurUnitData_Comp>();
        UnitData rectTemp;
        rectTemp.data.setValue<PenUnitData_Comp>(i.data);
        serializationUnitData(out, RectType, rectTemp, hander);
        out << i.effect;
        break;
    }
    }
}

void DdfUnitProccessor_5_8_0::serializationUnitTail(QDataStream &out, const UnitTail &tail, DdfHander *hander)
{
    out << tail.tailCheck[0];
    out << tail.tailCheck[1];
    out << tail.tailCheck[2];
    out << tail.tailCheck[3];
}

PageContextHead DdfUnitProccessor_chaos::deserializationDdfHead(QDataStream &in, DdfHander *hander)
{
    PageContextHead head;
    head.version = EDdfUnknowed;
    in >> head.unitCount;
    in >> head.rect;
    return head;
}

void DdfUnitProccessor_chaos::serializationUnitTree(QDataStream &out, const UnitTree_Comp &tree, int count, DdfHander *hander)
{
    emit hander->progressChanged(0, 1, tr("Saving..."));
    //loop tree.in this version,the tree only one leve,and there is no group.so only loop the bzItems.
    int i = 1;
    foreach (auto unit, tree.bzItems) {
        emit hander->progressChanged(i, count, "");
        if (isSupportedSaveType(unit.head.dataType))
            serializationUnit(out, unit, hander);
    }
}

UnitTree_Comp DdfUnitProccessor_chaos::deserializationUnitTree(QDataStream &in,
                                                               int count,
                                                               bool &stop, QString &stopReason, DdfHander *hander)
{
    UnitTree_Comp tree;
    for (int i = 0; i < count; ++i) {
        emit hander->progressChanged(i, count, "");
        Unit unit = deserializationUnit(in, hander);
        if (isSupportedSaveType(unit.head.dataType))
            tree.bzItems.append(unit);
    }
    return tree;
}

UnitHead DdfUnitProccessor_5_8_0::deserializationUnitHead(QDataStream &in, DdfHander *hander)
{
    UnitHead head;
    in >> head.headCheck[0];
    in >> head.headCheck[1];
    in >> head.headCheck[2];
    in >> head.headCheck[3];
    in >> head.dataType;
    in >> head.dataLength;
    in >> head.pen;
    in >> head.brush;
    in >> head.pos;
    in >> head.rotate;
    in >> head.zValue;
    return head;
}

UnitData DdfUnitProccessor_5_8_0::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    QVariant var;
    switch (type) {
    case RectType: {
        RectUnitData i;
        in >> i.topLeft;
        in >> i.bottomRight;
        i.xRedius = 0;
        i.yRedius = 0;
        var.setValue<RectUnitData>(i);
        break;
    }
    case EllipseType: {
        EllipseUnitData i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        var.setValue<EllipseUnitData>(i);
        break;
    }
    case TriangleType: {
        TriangleUnitData i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        var.setValue<TriangleUnitData>(i);
        break;
    }
    case PolygonalStarType: {
        StarUnit i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        in >> i.anchorNum;
        in >> i.radius;
        var.setValue<StarUnit>(i);
        break;
    }
    case PolygonType: {
        PolygonUnitData i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        in >> i.pointNum;
        var.setValue<PolygonUnitData>(i);
        break;
    }
    case LineType: {
        LineUnitData i;
        in >> i.point1;
        in >> i.point2;
        var.setValue<LineUnitData>(i);
        break;
    }
    case PenType: {
        PenUnitData_Comp i;
        qint8 pentype;
        in >> pentype;
        in >> i.path;
        in >> i.arrow;

        //以当前版本的形式保存之前的版本样式
        if (!i.arrow.isEmpty()) {
            i.start_type = soildArrow;
        }
        var.setValue<PenUnitData_Comp>(i);

        break;
    }
    case TextType: {
        TextUnitData i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        in >> i.font;
        in >> i.manResizeFlag;
        in >> i.content;
        var.setValue<TextUnitData>(i);
        break;
    }
    case PictureType: {
        ImageUnitData_Comp i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.rect = temp.data.value<RectUnitData>();
        in >> i.image;
        var.setValue<ImageUnitData_Comp>(i);
        break;
    }
    case BlurType: {
        BlurUnitData_Comp i;
        auto temp = deserializationUnitData(in, RectType, hander);
        i.data = temp.data.value<PenUnitData_Comp>();
        //in >> i.data;
        in >> i.effect;
        var.setValue<BlurUnitData_Comp>(i);
        break;
    }
    }
    UnitData result;
    result.data = var;

    return result;
}

UnitTail DdfUnitProccessor_5_8_0::deserializationUnitTail(QDataStream &in, DdfHander *hander)
{
    UnitTail tail;
    in >> tail.tailCheck[0];
    in >> tail.tailCheck[1];
    in >> tail.tailCheck[2];
    in >> tail.tailCheck[3];
    return tail;
}

qint64 DdfUnitProccessor_chaos::calTotalBytes(const PageContextData &data, DdfHander *hander)
{
    QSignalBlocker blocker(hander);

    QByteArray  allBytes;

    QDataStream out(&allBytes, QIODevice::WriteOnly);

    serializationDdfHead(out, data.head, hander);

    serializationUnitTree(out, data.units_comp, data.head.unitCount, hander);

    return allBytes.size();
}

QString DdfUnitProccessor_chaos::spaceNotEnoughMessage()
{
    return tr("Unable to save. There is not enough disk space.");
}


GroupItem *loadTree(LayerItem *layer, const UnitTree_Comp &tree, PageScene *scene)
{
    //GroupItem *parentGroup = new GroupItem;
    QList<PageItem *> its;
    foreach (auto p, tree.bzItems) {
        auto it = PageItem::creatItemInstance(p.head.dataType, p);
        layer->addItem(it);
        its.append(it);
    }
    foreach (auto gp, tree.childTrees) {
        its.append(loadTree(layer, gp, scene));
    }
    auto g = scene->creatGroup(its);
    layer->addItem(g);
    return g;
}

void DdfUnitProccessor_5_8_0_Compatibel::pageContextLoadData(PageContext *pContext, const PageContextData &data)
{
    auto scene = pContext->scene();

    //禁止选中和自动赋予z值的操作(z值可以通过数据加载确定)
    scene->blockAssignZValue(true);
    scene->blockSelect(true);

    scene->clearAll();

    Unit unit(UnitUsage_Save);
    unit.head.dataType = LayerItemType;
    unit.head.pos = QPointF(0, 0);
    unit.head.rect = data.head.rect;

    LayerUnitData layerData;
    unit.setValue<LayerUnitData>(layerData);

    LayerItem *layer = static_cast<LayerItem *>(PageItem::creatItemInstance(LayerItemType, unit));
    scene->insertTopLayer(layer);

    auto tree = data.units_comp;
    foreach (auto p, tree.bzItems) {
        layer->addItem(PageItem::creatItemInstance(p.head.dataType, p));
    }

    foreach (auto gp, tree.childTrees) {
        layer->addItem(loadTree(layer, gp, scene));
    }

    scene->blockSelect(false);
    scene->blockAssignZValue(false);
}

void DdfUnitProccessor_5_8_0_Compatibel::initForHander(DdfHander *hander)
{
    _handerMeetBlur.insert(hander);
    _handerMeetPen.insert(hander);
}

void DdfUnitProccessor_5_8_0_Compatibel::clearForHander(DdfHander *hander)
{
    _handerMeetBlur.remove(hander);
    _handerMeetPen.remove(hander);
}

UnitTree_Comp DdfUnitProccessor_5_8_0_Compatibel::deserializationUnitTree(QDataStream &in, int count,
                                                                          bool &stop, QString &stopReason,
                                                                          DdfHander *hander)
{
    UnitTree_Comp tree;
    for (int i = 0; i < count; ++i) {
        emit hander->progressChanged(i, count, "");
        Unit unit = deserializationUnit(in, hander);
        if (isSupportedSaveType(unit.head.dataType)) {
            if (unit.head.dataType == BlurType) {
                if (_handerMeetBlur.contains(hander)) {
                    _handerMeetBlur.remove(hander);
                    int ret = waitForHowToHanderBlurUnit(hander);
                    if (ret != 0) {
                        stop = true;
                        stopReason = "because refuse old blur unit.";
                        hander->setError(DdfHander::EUserCancelLoad_OldBlur, stopReason);
                        break;
                    }
                }
            } else if (unit.head.dataType == PenType) {
                if (_handerMeetPen.contains(hander)) {
                    _handerMeetPen.remove(hander);
                    int ret = waitForHowToHanderOldPenUnit(hander);
                    if (ret != 0) {
                        stop = true;
                        stopReason = "because refuse old pen unit.";
                        hander->setError(DdfHander::EUserCancelLoad_OldPen, stopReason);
                        break;
                    }
                }
            }
            tree.bzItems.append(unit);
        }
    }
    return tree;
}

Unit DdfUnitProccessor_5_8_0_Compatibel::deserializationUnit(QDataStream &in, DdfHander *hander)
{
    Unit unit(UnitUsage_Save);

    unit.head = deserializationUnitHead(in, hander);

    unit.data = deserializationUnitData(in, unit.head.dataType, hander);

    unit.tail = deserializationUnitTail(in, hander);

    //实现兼容型
//    if (unit.head.dataType == LayerItemType) {
//        //生成一个矩形大小及坐标位置
//        LayerUnitData wantedData = unit.value<LayerUnitData>();
//        LayerItem *item = new LayerItem;
//        QList<QSharedPointer<RasterCommandBase>> commands = wantedData.rasterData.commands;
//        foreach (auto p, commands) {
//            p->doCommandFor(item);
//        }
//        unit.head.pos = item->pos();
//        unit.head.rect = item->itemRect();
//        unit.head.trans = item->transform();

//        delete item;
//    } else if (unit.head.dataType == RasterItemType) {
//        //生成一个矩形大小及坐标位置
//        RasterItem *item = new RasterItem;
//        QList<QSharedPointer<RasterCommandBase>> commands = unit.value<RasterUnitData>().commands;
//        foreach (auto p, commands) {
//            p->doCommandFor(item);
//        }
//        unit.head.pos = item->pos();
//        unit.head.rect = item->itemRect();
//        unit.head.trans = item->transform();
//        delete  item;
//    }

    auto itemType = unit.head.dataType;
    //不再支持老的图像及画笔类型数据
    if ((PictureType == itemType || itemType == PenType)) {
        unit.head.dataType = RasterItemType;
        PageItem *item = PageItem::creatItemInstance(itemType, unit);
        if (item != nullptr) {
            if (item != nullptr && unit.isValid())
                item->loadItemUnit(unit);

            QImage img = static_cast<VectorItem *>(item)->rasterSelf();

            RasterUnitData rasterData;
            rasterData.baseImg = img;

            if (itemType == PictureType) {
                //rasterData.blocked = true;
                rasterData.layerType = (RasterItem::EImageType);
            } else {
                rasterData.layerType = (RasterItem::EPenType);
            }
            unit.setValue<RasterUnitData>(rasterData);
            delete item;
        } else {
            unit.setValue<RasterUnitData>(RasterUnitData());
        }
    }
    return unit;
}

int DdfUnitProccessor_5_8_0_Compatibel::waitForHowToHanderBlurUnit(DdfHander *hander)
{
    int returnRet = 1;

    emit hander->message_waitAnswer(SMessage(tr("The blur effect will be lost as the file is in old version. Proceed to open it?"),
                                             ENormalMsg, QStringList() << tr("Open") << tr("Cancel"),
                                             QList<EButtonType>() << EWarningMsgBtn << ENormalMsgBtn), returnRet);
    return returnRet;
}
int DdfUnitProccessor_5_8_0_Compatibel::waitForHowToHanderOldPenUnit(DdfHander *hander)
{
    int returnRet = 0;

    emit hander->message_waitAnswer(SMessage(tr("The pen effect will be lost as the file is in old version. Proceed to open it?"),
                                             ENormalMsg, QStringList() << tr("Open") << tr("Cancel"),
                                             QList<EButtonType>() << EWarningMsgBtn << ENormalMsgBtn),
                                    returnRet);
    return returnRet;
}
