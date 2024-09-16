// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cundoredocommand.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QMetaObject>
#include "pagescene.h"
#include "pageview.h"
#include "pagecontext.h"
#include "layeritem.h"

QDebug operator<<(QDebug debug, const UndoType &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << c.objectType << ", " << c.opeType << ", " << c.extraType << ')';

    return debug;
}
QDebug operator<<(QDebug debug, const UndoKey &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << c.object << ", " << c.type << ')';

    return debug;
}

class UndoCommand::UndoCommand_private
{
public:
    explicit UndoCommand_private(UndoCommand *qq): q(qq) {}

    void setVar(const UndoKey &key, const PageVariantList &vars, EVarUndoOrRedo varTp)
    {
        if (key.type != q->commandType()) {
            return;
        }

        object = key.object;
        _vars[varTp] = vars;
        q->parsingVars(_vars[varTp], varTp);
    }

    PageVariantList var(EVarUndoOrRedo varTp)
    {
        return _vars[varTp];
    }

    UndoCommand *q;

    PageObject *object = nullptr;

    PageVariantList _vars[VarTpCount];

    bool _blockRedo = false;
};
UndoCommand::UndoCommand(): UndoCommand_d(new UndoCommand_private(this))
{
}


void UndoCommand::undo()
{
    qDebug() << "do Undo----------------------";
    real_undo();
}

void UndoCommand::redo()
{
    if (!d_UndoCommand()->_blockRedo) {
        qDebug() << "do  Redo----------------------";
        real_redo();
    }
}

void UndoCommand::blockRedo(bool b)
{
    d_UndoCommand()->_blockRedo = b;
}

PageObject *UndoCommand::undoObject() const
{
    return d_UndoCommand()->object;
}

void UndoCommand::real_undo()
{
}

void UndoCommand::real_redo()
{
}

GroupUndoCommand::GroupUndoCommand()
{
}

GroupUndoCommand::~GroupUndoCommand()
{
    foreach (auto p, _allCmds) {
        delete p;
    }
    _allCmds.clear();
}

void GroupUndoCommand::addCommand(UndoCommand *pCmd)
{
    _allCmds.append(pCmd);
}

int GroupUndoCommand::count()
{
    return _allCmds.count();
}

UndoType GroupUndoCommand::commandType() const
{
    return UndoType(PageLayerObject);
}

void GroupUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    Q_UNUSED(vars)
    Q_UNUSED(varTp)
}

void GroupUndoCommand::real_undo()
{
    //撤销要反向执行
    for (int i = _allCmds.size() - 1; i >= 0; --i) {
        UndoCommand *pCmd = _allCmds[i];
        pCmd->real_undo();
    }

    //noticeUser(UndoVar);
}

void GroupUndoCommand::real_redo()
{
    //还原要挨个执行
    for (int i = 0; i < _allCmds.size(); ++i) {
        UndoCommand *pCmd = _allCmds[i];
        pCmd->real_redo();
    }
    //noticeUser(RedoVar);
}

ItemUndoCommand::ItemUndoCommand(EChangedType tp): _tp(tp)
{
}

UndoType ItemUndoCommand::commandType() const
{
    return UndoType(PageItemObject, _tp);
}

PageItem *ItemUndoCommand::pageItem() const
{
    return dynamic_cast<PageItem *>(undoObject());
}

void ItemUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    Q_UNUSED(varTp);
}

UndoCommand *ItemUnitUndoCommand::creatInstance()
{
    return new ItemUnitUndoCommand;
}

ItemPosUndoCommand::ItemPosUndoCommand()
    : ItemUndoCommand(EPosChanged)
{
}

UndoCommand *ItemPosUndoCommand::creatInstance()
{
    return new ItemPosUndoCommand;
}

void ItemPosUndoCommand::real_undo()
{
    if (pageItem() != nullptr) {
        pageItem()->setPos(_pos[UndoVar]);
    }
}

void ItemPosUndoCommand::real_redo()
{
    if (pageItem() != nullptr) {
        pageItem()->setPos(_pos[RedoVar]);
    }
}

void ItemPosUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    ItemUndoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "do not set pos in CMoveItemCommand!!!! varTp = " << varTp;
        return;
    }
    //解析
    //QPointF pos = vars[1].toPointF();
    _pos[varTp] = vars[0].toPointF();
}

ItemZUndoCommand::ItemZUndoCommand(): ItemUndoCommand(EZValueChanged)
{
}

UndoType ItemZUndoCommand::commandType() const
{
    return UndoType(PageItemObject, EZValueChanged);
}

void ItemZUndoCommand::real_undo()
{
    pageItem()->setPageZValue(_z[UndoVar]);
}

void ItemZUndoCommand::real_redo()
{
    pageItem()->setPageZValue(_z[RedoVar]);
}

void ItemZUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    ItemUndoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.isEmpty()) {
        return;
    }
    //qWarning() << "vars[0].toDouble() = " << vars[0].toDouble() << varTp;
    //解析
    _z[varTp] = vars[0].toDouble();
}

UndoCommand *ItemZUndoCommand::creatInstance()
{
    return new ItemZUndoCommand();
}

LayerUndoCommand::LayerUndoCommand(ChangedType tp)
    : UndoCommand()
    , _expTp(tp)
{
}

UndoType LayerUndoCommand::commandType() const
{
    return UndoType(PageLayerObject, _expTp);
}

PageScene *LayerUndoCommand::pageScene() const
{
    if (layer() == nullptr)
        return nullptr;

    return layer()->pageScene();
}

LayerItem *LayerUndoCommand::layer() const
{
    return dynamic_cast<LayerItem *>(undoObject());
}

LayerUndoCommand::ChangedType LayerUndoCommand::tp() const
{
    return _expTp;
}

//void SceneUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
//{
//    Q_UNUSED(varTp);

//}

LayerItemAdded::LayerItemAdded(ChangedType tp)
    : LayerUndoCommand(tp == Added ? ChildItemAdded : ChildItemRemoved)
    , _changedTp(tp)
{
}

LayerItemAdded::~LayerItemAdded()
{
}

void LayerItemAdded::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    _Items.clear();
    for (int i = 0; i < vars.size(); ++i) {
        PageItem *pItem = vars[i].toPageItem();
        if (pItem != nullptr)
            _Items.append(pItem);
    }
}

void LayerItemAdded::real_undo()
{
    //操作是删除那么undo就是添加回去
    if (_changedTp == Removed) {
        if (pageScene() != nullptr) {
            pageScene()->clearSelections();
            pageScene()->blockAssignZValue(true);
            for (int i = 0; i < _Items.size(); ++i) {
                pageScene()->addPageItem(_Items[i], layer());
            }
            pageScene()->blockAssignZValue(false);
            pageScene()->selectPageItem(_Items);
        }
    } else {
        if (pageScene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                pageScene()->removePageItem(_Items[i]);
            }
        }
    }
}

void LayerItemAdded::real_redo()
{
    //操作是删除那么redo就是删除
    if (_changedTp == Removed) {
        if (pageScene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                pageScene()->removePageItem(_Items[i]);
            }
        }
    } else {
        if (pageScene() != nullptr) {
            pageScene()->clearSelections();
            pageScene()->blockAssignZValue(true);
            for (int i = 0; i < _Items.size(); ++i) {
                pageScene()->addPageItem(_Items[i], layer());
            }
            pageScene()->blockAssignZValue(false);
            pageScene()->selectPageItem(_Items);
        }
    }
}

QList<PageItem *> &LayerItemAdded::items()
{
    return _Items;
}

UndoCommand *LayerItemAdded::creatLayerItemAddedCommand()
{
    return new LayerItemAdded(Added);
}

UndoCommand *LayerItemAdded::creatLayerItemRemovedCommand()
{
    return new LayerItemAdded(Removed);
}

ItemUnitUndoCommand::ItemUnitUndoCommand(): ItemUndoCommand(EUnitChanged)
{
}

ItemUnitUndoCommand::~ItemUnitUndoCommand()
{
}

void ItemUnitUndoCommand::real_undo()
{
    if (pageItem() != nullptr) {
        pageItem()->loadItemUnit(_itemDate[UndoVar]);
    }
}

void ItemUnitUndoCommand::real_redo()
{
    if (pageItem() != nullptr) {
        pageItem()->loadItemUnit(_itemDate[RedoVar]);
    }
}

void ItemUnitUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    ItemUndoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.isEmpty()) {
        qWarning() << "do not set pos in CBzItemAllCommand!!!!";
        return;
    }
    _itemDate[varTp] = vars.first().toUnit();
}

LayerRectUndoCommand::LayerRectUndoCommand():
    LayerUndoCommand(RectChanged)
{
}

void LayerRectUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    //是否能解析的判断
    if (vars.isEmpty()) {
        qWarning() << "not found any info,forgot insert layer rect ??";
        return;
    }
    _rect[varTp] = vars.first().toRectF();
}

void LayerRectUndoCommand::real_undo()
{
    if (layer() != nullptr) {
        extern void Raster_SetRect(RasterItem * item, const QRectF & rct, bool addcmd);
        Raster_SetRect(layer(), _rect[UndoVar], false);
        //layer()->setRasterRect(_rect[UndoVar]);
    }
}

void LayerRectUndoCommand::real_redo()
{
    if (layer() != nullptr) {
        extern void Raster_SetRect(RasterItem * item, const QRectF & rct, bool addcmd);
        Raster_SetRect(layer(), _rect[RedoVar], false);
        //layer()->setRasterRect(_rect[RedoVar]);
    }
}

SenceRectUndoCommand::SenceRectUndoCommand():
    LayerUndoCommand(SenceRectChanged)
{

}

void SenceRectUndoCommand::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    //是否能解析的判断
    if (vars.isEmpty()) {
        qWarning() << "not found any info,forgot insert layer rect ??";
        return;
    }
    _rect[varTp] = vars.first().toRectF();
}

void SenceRectUndoCommand::real_undo()
{
    if (layer() != nullptr) {
        pageScene()->setSceneRect(_rect[UndoVar]);
    }
}
void SenceRectUndoCommand::real_redo()
{
    if (layer() != nullptr) {
        pageScene()->setSceneRect(_rect[UndoVar]);
    }
}

UndoRecorder::UndoRecorder(LayerItem *layer, LayerUndoCommand::ChangedType EchangedTp, PageItem *pItem, bool doRedo):
    UndoRecorder(layer, EchangedTp, QList<PageItem *>() << pItem, doRedo)
{

}

UndoRecorder::UndoRecorder(LayerItem *layer, LayerUndoCommand::ChangedType EchangedTp,
                           const QList<PageItem *> &list, bool doRedo)
    : _doRedo(doRedo), _pLayer(layer), _scenChangedType(EchangedTp)
{
    if (_pLayer == nullptr)
        return;

    //是否需要还原信息(删除图元其实不需要)
    bool needRedoInfo = true;
    //记录undo
    PageVariantList vars;
    if (EchangedTp == LayerUndoCommand::RectChanged) {
        vars << _pLayer->itemRect();
    } else if (EchangedTp == LayerUndoCommand::ChildItemAdded || EchangedTp == LayerUndoCommand::ChildItemRemoved) {
        vars << list;
        needRedoInfo = false;
    } else if (EchangedTp == LayerUndoCommand::ChildGroupAdded || EchangedTp == LayerUndoCommand::ChildGroupRemoved) {
        vars << list;
        needRedoInfo = false;
    } else if (EchangedTp == LayerUndoCommand::SenceRectChanged) {
        vars << layer->pageScene()->sceneRect();
    }
    UndoStack::recordUndo(UndoKey(layer, PageLayerObject,
                                  EchangedTp), vars, needRedoInfo);
}

UndoRecorder::UndoRecorder(PageItem *pItem, EChangedPhase phase, bool doRedo)
    : _pItem(pItem)
    , _phase(phase)
    , _doRedo(doRedo)
{
    if (_pItem == nullptr)
        return;

    if (_phase == EChangedUpdate || _phase == EChangedFinished || _phase == EChangedAbandon)
        return;

    if (_pItem->type() == CutType) {
        UndoStack::recordUndo(UndoKey(_pItem->pageScene(), PageLayerObject,
                                      LayerUndoCommand::RectChanged), PageVariant(_pItem->pageScene()->sceneRect()), true);
    }

    QList<PageItem *> items;
    /*    if (_pItem->type() == SelectionItemType) {
            items = dynamic_cast<SelectionItem *>(_pItem);

        } else */{
        items.append(pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        PageItem *pItem = items[i];
        PageVariantList vars(pItem->getItemUnit(UnitUsage_Undo));
        if (_phase == EChangedBegin || _phase == EChanged) {
            UndoStack::recordUndo(UndoKey(pItem, PageItemObject,
                                          ItemUndoCommand::EUnitChanged), vars, i == 0);
        }
    }
}

UndoRecorder::~UndoRecorder()
{
    if (_pLayer != nullptr) {
        PageVariantList vars;
        //记录undo
        if (_scenChangedType == LayerUndoCommand::RectChanged) {
            vars << _pLayer->itemRect();

        } else if (_scenChangedType == LayerUndoCommand::ChildGroupAdded) {
//            QVariant var;
//            var.setValue(_pScene->getUnitItemTree());
//            vars << var;
        } else if (_scenChangedType == LayerUndoCommand::SenceRectChanged)
            vars << _pLayer->pageScene()->sceneRect();

        //添加删除动作不需要还原信息(因为根据添加/删除的对象即可确定还原时应该怎么做,及执行反向的操作就可以了)
        if (_scenChangedType != LayerUndoCommand::ChildItemAdded && _scenChangedType != LayerUndoCommand::ChildItemRemoved) {
            UndoStack::recordRedo(UndoKey(_pLayer, PageLayerObject,
                                          _scenChangedType), vars);
        }

        UndoStack::finishRecord(_pLayer->page()->view()->stack(), _doRedo);
        return;
    }

    if (_pItem == nullptr)
        return;

    if (_phase != EChangedFinished && _phase != EChanged)
        return;

    if (_phase == EChangedAbandon) {
        UndoStack::clearRecordings();
        return;
    }

    if (_pItem->type() == CutType) {
        PageVariantList vars;
        vars << _pItem->pageScene()->sceneRect();
        UndoStack::recordRedo(UndoKey(_pItem->pageScene(), PageLayerObject,
                                      LayerUndoCommand::RectChanged), vars);
    }

    QList<PageItem *> items;
    /*    if (_pItem->type() == GroupItemType) {
            items = dynamic_cast<GroupItem *>(_pItem)->items(true);

        } else */{
        items.append(_pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        PageItem *pItem = items[i];
        PageVariantList vars;
        vars << pItem->getItemUnit(UnitUsage_Undo);
        UndoStack::recordRedo(UndoKey(pItem, PageItemObject,
                                      ItemUndoCommand::EUnitChanged), vars);
    }

    if (_pItem->pageScene() != nullptr) {
        UndoStack::finishRecord(_pItem->pageView()->stack(), _doRedo);
    }
}

UndoKey::UndoKey(PageObject *o, UndoType tp): object(o), type(tp)
{

}
UndoKey::UndoKey(PageObject *o, int obType, int opeTp): object(o), type(UndoType(obType, opeTp))
{

}
bool UndoKey::isValid() const
{
    return object != nullptr && type.isValid();
}

bool UndoKey::operator<(const UndoKey &other) const
{
    if (object < other.object) {
        return true;
    } else if (object == other.object) {
        if (type < other.type) {
            return true;
        }
    }
    return false;
}

bool UndoKey::operator==(const UndoKey &other) const
{
    return (object == other.object) && (type == type);
}

bool UndoKey::operator!=(const UndoKey &other) const
{
    return !(*this == other);
}

class UndoStack::UndoStack_private
{
public:
    explicit UndoStack_private(UndoStack *qq): q(qq) {}

    UndoStack *q;

    struct SCommandInfo {
        UndoKey key;
        EVarUndoOrRedo urFlag = VarTpCount;
        PageVariantList vars;
        void clear()
        {
            key.object = nullptr;
            key.type.objectType = -1;
            key.type.extraType = -1;
            key.type.opeType = -1;
            vars.clear();
        }
        bool isValid()const
        {
            return urFlag != VarTpCount && key.isValid();
        }
    };

    struct SCommandInfoCouple {
        SCommandInfo undoInfo;
        SCommandInfo redoInfo;
        bool isValid() const
        {
            return (undoInfo.key == redoInfo.key && redoInfo.isValid());
        }

        UndoKey key() const
        {
            if (isValid())
                return undoInfo.key;
            return UndoKey();
        }

        UndoType type()const
        {
            return undoInfo.key.type;
        }
    };


    static UndoCommand *getCmdByCmdInfo(const SCommandInfoCouple &info)
    {
        if (!info.isValid()) {
            qWarning() << "no valid undo/redo couple info....";
            return nullptr;
        }
        UndoCommand *pCmd = factory()->creatCmd(info.type());
        if (pCmd != nullptr) {
            pCmd->d_UndoCommand()->setVar(info.key(), info.undoInfo.vars, UndoVar);
            pCmd->d_UndoCommand()->setVar(info.key(), info.redoInfo.vars, RedoVar);
        }
        return pCmd;
    }

    static UndoCommandFactory *factory()
    {
        return UndoCommandFactory::instance();
    }

    typedef QList<SCommandInfoCouple> QCommandInfoList;

    static QCommandInfoList s_recordedCmdInfoList;
    static QMap<UndoKey, int> s_forFindCoupleMap;

    int countStepNum = 0;
};
UndoStack::UndoStack_private::QCommandInfoList UndoStack::UndoStack_private::s_recordedCmdInfoList =
    UndoStack::UndoStack_private::QCommandInfoList();
QMap<UndoKey, int> UndoStack::UndoStack_private::s_forFindCoupleMap = QMap<UndoKey, int>();

UndoStack::UndoStack(QObject *parent): QUndoStack(parent), UndoStack_d(new UndoStack_private(this))
{

}

UndoStack::~UndoStack()
{
}

void UndoStack::push(QUndoCommand *cmd)
{
    d_UndoStack()->countStepNum += 1;
    QUndoStack::push(cmd);
}

void UndoStack::undo()
{
    d_UndoStack()->countStepNum -= 1;
    QUndoStack::undo();
}

void UndoStack::redo()
{
    d_UndoStack()->countStepNum += 1;
    QUndoStack::redo();
}

void UndoStack::recordUndo(UndoKey key, const PageVariantList &datas, bool joinPre)
{
    if (!UndoCommandFactory::instance()->canCreatCommand(key.type)) {
        return;
    }

    if (datas.isEmpty())
        return;

    if (!joinPre) {
        clearRecordings();
    }
    UndoStack_private::SCommandInfo undoCmd;
    undoCmd.key = key;
    undoCmd.urFlag = UndoVar;
    undoCmd.vars = datas;

    UndoStack_private::SCommandInfoCouple cp;
    cp.undoInfo = undoCmd;
    cp.redoInfo = undoCmd; //先初始化redo信息

    UndoStack_private::s_recordedCmdInfoList.append(cp);
    int index = UndoStack_private::s_recordedCmdInfoList.size() - 1;
    UndoStack_private::s_forFindCoupleMap.insert(key, index);
}

void UndoStack::recordRedo(UndoKey key, const PageVariantList &datas)
{
    if (!UndoCommandFactory::instance()->canCreatCommand(key.type)) {
        return;
    }

    if (datas.isEmpty())
        return;

    auto itF = UndoStack_private::s_forFindCoupleMap.find(key);
    if (itF != UndoStack_private::s_forFindCoupleMap.end()) {
        UndoStack_private::SCommandInfoCouple &cp = UndoStack_private::s_recordedCmdInfoList[itF.value()];

        //类型必须一致
        if (cp.undoInfo.key == key && cp.undoInfo.urFlag == UndoVar) {
            UndoStack_private::SCommandInfo reDocmd;
            reDocmd.key = key;
            reDocmd.urFlag = RedoVar;
            reDocmd.vars = datas;
            cp.redoInfo = reDocmd;
            return;
        }
    } /*else {
        UndoStack_private::SCommandInfo undoCmd;
        undoCmd.key = key;
        undoCmd.urFlag = UndoVar;
        undoCmd.vars = datas;

        UndoStack_private::SCommandInfoCouple cp;
        cp.undoInfo = undoCmd;
        auto cmd = UndoStack_private::getCmdByCmdInfo(cp);
        if (cmd != nullptr) {
            cmd->parsingVars(undoCmd.vars, UndoVar);
            delete cmd;
        }
    }*/
    qWarning() << "undo redo command not match to a pair !!!!";
}

void UndoStack::finishRecord(UndoStack *stack, bool doRedoCmd)
{
    //所有的CmdInfo组成了这样的一个单元操作
    if (!UndoStack_private::s_recordedCmdInfoList.isEmpty()) {
        GroupUndoCommand *pGropCmd = new GroupUndoCommand;
        for (int i = 0; i < UndoStack_private::s_recordedCmdInfoList.size();) {
            UndoStack_private::SCommandInfoCouple cmdInfo = UndoStack_private::s_recordedCmdInfoList[i];

            auto cmd = UndoStack_private::getCmdByCmdInfo(cmdInfo);
            bool success = (cmd != nullptr);

            if (success) {
                ++i;
                pGropCmd->addCommand(cmd);
            } else {
                UndoStack_private::s_recordedCmdInfoList.removeAt(i);
            }
        }

        if (pGropCmd->count() == 0) {
            delete pGropCmd;
            pGropCmd = nullptr;
            qWarning() << "no couple undo redo command!!!!!!!!!!";
            return;
        }

        pGropCmd->blockRedo(!doRedoCmd);
        stack->push(pGropCmd);
        pGropCmd->blockRedo(false);

        //执行完后清理操作
        clearRecordings();
        //UndoStack_private::s_recordedCmdInfoList.clear();
        //UndoStack_private::s_forFindCoupleMap.clear();
    }
}

bool UndoStack::isRecordEmpty()
{
    return UndoStack_private::s_recordedCmdInfoList.isEmpty();
}

void UndoStack::clearRecordings()
{
//    for (int i = 0; i < UndoStack_private::s_recordedCmdInfoList.size(); ++i) {
//        UndoStack_private::SCommandInfoCouple cmdInfo = UndoStack_private::s_recordedCmdInfoList[i];

//        UndoRedoCommand *Cmd = UndoStack_private::getCmdByCmdInfo(cmdInfo);

//        if (Cmd != nullptr) {
//            if (!cmdInfo.undoInfo.vars.isEmpty())
//                Cmd->setVar(cmdInfo.undoInfo.vars, UndoVar);
//            if (!cmdInfo.redoInfo.vars.isEmpty())
//                Cmd->setVar(cmdInfo.redoInfo.vars, RedoVar);
//            delete Cmd;
//        }
//    }
    UndoStack_private::s_recordedCmdInfoList.clear();
    UndoStack_private::s_forFindCoupleMap.clear();
}

void UndoStack::recordUndo(LayerItem *layer, int exptype, const QList<PageItem *> &items)
{
    PageVariantList vars;

    LayerUndoCommand::ChangedType tp = LayerUndoCommand::ChangedType(exptype);

    //当前仅支持EGroupChanged的场景快照还原
    switch (tp) {
    case LayerUndoCommand::ChildGroupAdded: {
        break;
    }
    default:
        vars << items;
        return;
    }

    UndoStack::recordUndo(UndoKey(layer, PageLayerObject, exptype),
                          vars, true);
}

void UndoStack::recordRedo(LayerItem *layer, int exptype, const QList<PageItem *> &items)
{
    PageVariantList vars;

    LayerUndoCommand::ChangedType tp = LayerUndoCommand::ChangedType(exptype);

    //当前仅支持EGroupChanged的场景快照还原
    switch (tp) {
    case LayerUndoCommand::ChildGroupAdded: {
        break;
    }
    default:
        vars << items;
        return;
    }

    UndoStack::recordRedo(UndoKey(layer, PageLayerObject, exptype),
                          vars);
}

void UndoStack::recordUndo(PageItem *pItem, bool joinPre)
{
    PageVariantList vars;
    vars << pItem->getItemUnit(UnitUsage_Undo);
    UndoStack::recordUndo(UndoKey(pItem, PageItemObject,
                                  ItemUndoCommand::EUnitChanged), vars, joinPre);
}

void UndoStack::recordRedo(PageItem *pItem)
{
    PageVariantList vars;
    vars << pItem->getItemUnit(UnitUsage_Undo);
    UndoStack::recordRedo(UndoKey(pItem, PageItemObject,
                                  ItemUndoCommand::EUnitChanged), vars);
}

void UndoStack::recordUndo(const QList<PageItem *> &items, bool joinPre)
{
    if (!joinPre)
        clearRecordings();

    for (int i = 0; i < items.size(); ++i) {
        PageItem *pItem = items[i];
        recordUndo(pItem, true);
    }
}

void UndoStack::recordRedo(const QList<PageItem *> &items)
{
    for (int i = 0; i < items.size(); ++i) {
        PageItem *pItem = items[i];
        recordRedo(pItem);
    }
}


int UndoStack::userValidStepCount() const
{
    return d_UndoStack()->countStepNum;
}

UndoType::UndoType(int obt, int opt, int ext): objectType(obt), opeType(opt), extraType(ext)
{

}


bool UndoType::isValid() const
{
    return objectType != -1;
}

bool UndoType::operator<(const UndoType &other) const
{
    if (objectType < other.objectType) {
        return true;
    } else if (objectType == other.objectType) {
        if (opeType < other.opeType) {
            return true;
        } else if (opeType == other.opeType) {
            if (extraType < other.extraType)
                return true;
        }
    }
    return false;
}

bool UndoType::operator==(const UndoType &other) const
{
    return (objectType == other.objectType) && (opeType == other.opeType) && (extraType == other.extraType);
}

bool UndoType::operator!=(const UndoType &other) const
{
    return !(*this == other);
}

UndoCommandFactory *UndoCommandFactory::staticFactory = nullptr;
UndoCommand *UndoCommandFactory::creatCmd(const UndoType &type)
{
    auto find = cmdCreatFunsMap.find(type);
    if (find != cmdCreatFunsMap.end()) {
        CREATCMD f = find.value();
        return f();
    }
    return nullptr;
}

bool UndoCommandFactory::canCreatCommand(const UndoType &tp) const
{
    bool b = (cmdCreatFunsMap.find(tp) != cmdCreatFunsMap.end());
    if (!b) {
        qWarning() << "don't have this type undo command. (type = " << tp << ")";
    }
    return b;
}

void UndoCommandFactory::registerCommand(const UndoType &type, CREATCMD f)
{
    cmdCreatFunsMap.insert(type, f);
}

UndoCommandFactory *UndoCommandFactory::instance()
{
    if (staticFactory == nullptr) {
        staticFactory = new UndoCommandFactory;

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::RectChanged),
                                       &LayerRectUndoCommand::creatInstance);

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::ChildItemAdded),
                                       &LayerItemAdded::creatLayerItemAddedCommand);

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::ChildItemRemoved),
                                       &LayerItemAdded::creatLayerItemRemovedCommand);

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::ChildGroupAdded),
                                       &LayerGroupAdded::creatAddInstance);

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::ChildGroupRemoved),
                                       &LayerGroupAdded::creatRemoveInstance);


        staticFactory->registerCommand(UndoType(PageItemObject, ItemUndoCommand::EPosChanged),
                                       &ItemPosUndoCommand::creatInstance);
        staticFactory->registerCommand(UndoType(PageItemObject, ItemUndoCommand::EZValueChanged),
                                       &ItemZUndoCommand::creatInstance);
        staticFactory->registerCommand(UndoType(PageItemObject, ItemUndoCommand::EUnitChanged),
                                       &ItemUnitUndoCommand::creatInstance);

        staticFactory->registerCommand(UndoType(PageLayerObject, LayerUndoCommand::SenceRectChanged),
                                       &SenceRectUndoCommand::creatInstance);

    }
    return staticFactory;
}

UndoCommandFactory::UndoCommandFactory(QObject *parent): QObject(parent)
{
    //registerCommand()
}

LayerGroupAdded::LayerGroupAdded(Type tp): LayerUndoCommand(tp == ADD ? ChildGroupAdded : ChildGroupRemoved),
    _tp(tp)
{

}

LayerGroupAdded::~LayerGroupAdded()
{
}

void LayerGroupAdded::parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp)
{
    addedGroup = nullptr;
    items.clear();

    addedGroup = dynamic_cast<GroupItem *>(vars.first().toPageItem());
    for (int i = 1; i < vars.count(); ++i) {
        items.append(vars.at(i).toPageItem());
    }
}

UndoCommand *LayerGroupAdded::creatAddInstance()
{
    return new LayerGroupAdded;
}

UndoCommand *LayerGroupAdded::creatRemoveInstance()
{
    return new LayerGroupAdded(REMOVE);
}

void LayerGroupAdded::real_undo()
{
    if (_tp == ADD) {
        if (pageScene() != nullptr) {
            items = addedGroup->childPageItems();
            pageScene()->cancelGroup(addedGroup);
        }
    } else {
        if (pageScene() != nullptr) {
            addedGroup->addToGroup(items);
            pageScene()->blockAssignZValue(true);
            pageScene()->addPageItem(addedGroup, layer());
            pageScene()->blockAssignZValue(false);

            pageScene()->clearSelections();
            pageScene()->selectPageItem(addedGroup);
        }
    }

}

void LayerGroupAdded::real_redo()
{
    if (_tp == ADD) {
        if (pageScene() != nullptr) {
            addedGroup->addToGroup(items);
            pageScene()->blockAssignZValue(true);
            pageScene()->addPageItem(addedGroup, layer());
            pageScene()->blockAssignZValue(false);

            pageScene()->clearSelections();
            pageScene()->selectPageItem(addedGroup);
        }
    } else {
        if (pageScene() != nullptr) {
            pageScene()->cancelGroup(addedGroup);
        }
    }
}

