// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cundoredocommand.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QMetaObject>
#include "cdrawscene.h"
#include "cgraphicsitemselectedmgr.h"
#include "cviewmanagement.h"
#include "cgraphicsview.h"
#include "cdrawparamsigleton.h"
#include "application.h"

CUndoRedoCommand::QCommandInfoList CUndoRedoCommand::s_recordedCmdInfoList = CUndoRedoCommand::QCommandInfoList();
QMap<CUndoRedoCommand::CKey, int> CUndoRedoCommand::s_forFindCoupleMap = QMap<CUndoRedoCommand::CKey, int>();
QMap<void *, QUndoStack *> CUndoRedoCommand::s_undoStacks = QMap<void *, QUndoStack *>();
bool CUndoRedoCommand::s_blockRedoWhenPushToStack = false;
CUndoRedoCommand::CUndoRedoCommand()
{
}

void CUndoRedoCommand::clearCommand()
{
    for (int i = 0; i < s_recordedCmdInfoList.size(); ++i) {
        SCommandInfoCouple cmdInfo = s_recordedCmdInfoList[i];

        CUndoRedoCommand *Cmd = getCmdByCmdInfo(cmdInfo);

        if (Cmd != nullptr) {
            if (!cmdInfo.undoInfo.vars.isEmpty())
                Cmd->setVar(cmdInfo.undoInfo.vars, UndoVar);
            if (!cmdInfo.redoInfo.vars.isEmpty())
                Cmd->setVar(cmdInfo.redoInfo.vars, RedoVar);
            delete Cmd;
        }
    }
    s_recordedCmdInfoList.clear();
    s_forFindCoupleMap.clear();
}

void CUndoRedoCommand::pushStack(CUndoRedoCommand *pCmd)
{
    CManageViewSigleton::GetInstance()->getCurView()->pushActionCount();
    getUndoRedoStack()->push(pCmd);
}

void CUndoRedoCommand::finishRecord(bool doRedoCmd)
{
    //所有的CmdInfo组成了这样的一个单元操作
    if (!s_recordedCmdInfoList.isEmpty()) {
        CUndoRedoCommandGroup *pGropCmd = new CUndoRedoCommandGroup;
        for (int i = 0; i < s_recordedCmdInfoList.size();) {
            SCommandInfoCouple cmdInfo = s_recordedCmdInfoList[i];

            bool success = pGropCmd->addCommand(cmdInfo);

            if (success) {
                ++i;
            } else {
                s_recordedCmdInfoList.removeAt(i);
            }
        }

        if (pGropCmd->count() == 0) {
            delete pGropCmd;
            pGropCmd = nullptr;

            qWarning() << "no couple undo redo command!!!!!!!!!!";
            return;
        }

        setBlockRedoWhenPushedToStack(!doRedoCmd);
        pushStack(pGropCmd);
        setBlockRedoWhenPushedToStack(false);

        //执行完后清理操作
        //clearCommand();
        s_recordedCmdInfoList.clear();
        s_forFindCoupleMap.clear();
    }
}

bool CUndoRedoCommand::isRecordEmpty()
{
    return s_recordedCmdInfoList.isEmpty();
}

void CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::EDrawUndoCmdType tp,
                                         int expendTp,
                                         const QList<QVariant> &datas,
                                         bool init,
                                         bool NoNeedRedoVars)
{
    if (datas.isEmpty())
        return;

    if (tp >= EDrawGroupCmd)
        return;

    if (init) {
        clearCommand();
    }
    SCommandInfo undoCmd;
    undoCmd.tp = tp;
    undoCmd.expTp = expendTp;
    undoCmd.urFlag = UndoVar;
    undoCmd.vars = datas;

    SCommandInfoCouple cp;
    cp.undoInfo = undoCmd;
    cp.noneedRedoVars = NoNeedRedoVars;

    s_recordedCmdInfoList.append(cp);

    int index = s_recordedCmdInfoList.size() - 1;

    s_forFindCoupleMap.insert(CKey(reinterpret_cast<long long>(datas.first().toLongLong()), tp, expendTp), index);
}

void CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::EDrawUndoCmdType tp, int expendTp, const QList<QVariant> &datas)
{
    if (datas.isEmpty())
        return;

    if (tp >= EDrawGroupCmd)
        return;

    CKey key(reinterpret_cast<long long>(datas.first().toLongLong()), tp, expendTp);
    auto itF = s_forFindCoupleMap.find(key);
    if (itF != s_forFindCoupleMap.end()) {
        SCommandInfoCouple &cp = s_recordedCmdInfoList[itF.value()];

        //类型必须一致
        if (cp.undoInfo.tp == tp && cp.undoInfo.urFlag == UndoVar) {
            SCommandInfo reDocmd;
            reDocmd.tp = tp;
            reDocmd.expTp = expendTp;
            reDocmd.urFlag = RedoVar;
            reDocmd.vars = datas;
            cp.redoInfo = reDocmd;
            return;
        }
    } else {
        SCommandInfo undoCmd;
        undoCmd.tp = tp;
        undoCmd.expTp = expendTp;
        undoCmd.urFlag = UndoVar;
        undoCmd.vars = datas;

        SCommandInfoCouple cp;
        cp.undoInfo = undoCmd;
        auto cmd = getCmdByCmdInfo(cp);
        if (cmd != nullptr) {
            cmd->parsingVars(undoCmd.vars, UndoVar);
            delete cmd;
        }
    }
    qWarning() << "undo redo command not match to a pair !!!!";
}

void CUndoRedoCommand::undo()
{
    qDebug() << "do Undo----------------------";
    real_undo();
}

void CUndoRedoCommand::redo()
{
    if (!s_blockRedoWhenPushToStack) {
        qDebug() << "do  Redo----------------------";
        real_redo();
    }
}

void CUndoRedoCommand::setBlockRedoWhenPushedToStack(bool b)
{
    s_blockRedoWhenPushToStack = b;
}

//bool CUndoRedoCommand::isBlockRedoWhenPushedToStack()
//{
//    return s_blockRedoWhenPushToStack;
//}

void CUndoRedoCommand::real_undo()
{
}

void CUndoRedoCommand::real_redo()
{
}

void CUndoRedoCommand::setVar(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    _vars[varTp] = vars;
    parsingVars(_vars[varTp], varTp);
}

QList<QVariant> CUndoRedoCommand::var(EVarUndoOrRedo varTp)
{
    return _vars[varTp];
}

void CUndoRedoCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    Q_UNUSED(vars)
    Q_UNUSED(varTp)
}


void CUndoRedoCommand::setUndoRedoStack(QUndoStack *stack, void *pGraphicsView)
{
    s_undoStacks.insert(pGraphicsView, stack);
}

QUndoStack *CUndoRedoCommand::getUndoRedoStack(void *p)
{
    if (p == nullptr) {
        p = CManageViewSigleton::GetInstance()->getCurView();
    }
    return s_undoStacks[p];
}

CUndoRedoCommand *CUndoRedoCommand::getCmdByCmdInfo(const CUndoRedoCommand::SCommandInfoCouple &info)
{
    CUndoRedoCommand *pCmd = nullptr;
    switch (info.type()) {
    case ESceneChangedCmd: {
        pCmd = CSceneUndoRedoCommand::getCmdBySceneCmdInfo(info);
        break;
    }
    case EItemChangedCmd: {
        pCmd = CItemUndoRedoCommand::getCmdByItemCmdInfo(info);
        break;
    }
    default:
        break;
    }
    return pCmd;
}

CUndoRedoCommandGroup::CUndoRedoCommandGroup(bool noticeOnEnd)
    : _noticeOnfinished(noticeOnEnd)
{
}

CUndoRedoCommandGroup::~CUndoRedoCommandGroup()
{
    foreach (auto p, _allCmds) {
        delete p;
    }
    _allCmds.clear();
}

void CUndoRedoCommandGroup::addCommand(CUndoRedoCommand *pCmd)
{
    _allCmds.append(pCmd);
}

int CUndoRedoCommandGroup::count()
{
    return _allCmds.count();
}

bool CUndoRedoCommandGroup::addCommand(const SCommandInfoCouple &pCmd)
{
    bool ret = false;

    CUndoRedoCommand *Cmd = getCmdByCmdInfo(pCmd);
    if (Cmd != nullptr) {
        Cmd->setVar(pCmd.undoInfo.vars, UndoVar);

        if (pCmd.isVaild()) {
            Cmd->setVar(pCmd.redoInfo.vars, RedoVar);
            ret = true;
        } else {
            if (pCmd.noneedRedoVars) {
                Cmd->setVar(pCmd.undoInfo.vars, RedoVar);
                ret = true;
            }
        }
    }
    if (ret)
        addCommand(Cmd);
    else {
        delete Cmd;
    }

    return ret;
}

void CUndoRedoCommandGroup::noticeUser(EVarUndoOrRedo tp)
{
    if (_noticeOnfinished) {
        QList<CGraphicsItem *> bzItems;
        PageScene *pScene = nullptr;
        for (CUndoRedoCommand *pCmd : _allCmds) {
            CItemUndoRedoCommand *pItemCmd = dynamic_cast<CItemUndoRedoCommand *>(pCmd);
            if (pItemCmd != nullptr) {
                if (!pItemCmd->isNoNeedSelected(tp)) {
                    QGraphicsItem *pItem = pItemCmd->item();
                    if (pScene == nullptr) {
                        pScene = qobject_cast<PageScene *>(pItem->scene());
                    }

                    CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(pItem);
                    if (pBzItem != nullptr) {
                        bzItems.append(pBzItem);
                    }
                }
            } else {
                CSceneUndoRedoCommand *pSceneCmd = dynamic_cast<CSceneUndoRedoCommand *>(pCmd);
                if (pScene == nullptr) {
                    pScene = qobject_cast<PageScene *>(pSceneCmd->scene());
                }

                bool finished = false;
                switch (pSceneCmd->tp()) {
                case CSceneUndoRedoCommand::EItemAdded:
                case CSceneUndoRedoCommand::EItemRemoved: {
                    CSceneItemNumChangedCommand *pNumCmd = dynamic_cast<CSceneItemNumChangedCommand *>(pCmd);
                    if (pNumCmd != nullptr) {
                        QList<CGraphicsItem *> &items = pNumCmd->items();
                        for (CGraphicsItem *pItem : items) {
                            CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(pItem);
                            if (pBzItem != nullptr && pBzItem->scene() != nullptr) {
                                bzItems.append(pBzItem);
                            }
                        }
                    }
                    break;
                }
                case CSceneUndoRedoCommand::EGroupChanged: {
                    bzItems.clear();
                    auto pSnGpCMd = static_cast<CSceneGroupChangedCommand *>(pSceneCmd);
                    qDebug() << "pSnGpCMd->realChangedItems() count = " << pSnGpCMd->realChangedItems().count();
                    bzItems.append(pSnGpCMd->realChangedItems());
                    finished = true;
                    break;
                }
                default:
                    break;
                }
                if (finished)
                    break;
            }
        }

        if (pScene != nullptr) {
            pScene->clearSelectGroup();
            for (CGraphicsItem *pBzItem : bzItems) {
                pScene->selectItem(pBzItem->thisBzProxyItem(true));
            }
        }
    }
}

void CUndoRedoCommandGroup::real_undo()
{
    //撤销要反向执行
    for (int i = _allCmds.size() - 1; i >= 0; --i) {
        CUndoRedoCommand *pCmd = _allCmds[i];
        pCmd->real_undo();
    }

    noticeUser(UndoVar);
}

void CUndoRedoCommandGroup::real_redo()
{
    //还原要挨个执行
    for (int i = 0; i < _allCmds.size(); ++i) {
        CUndoRedoCommand *pCmd = _allCmds[i];
        pCmd->real_redo();
    }
    noticeUser(RedoVar);
}

CItemUndoRedoCommand::CItemUndoRedoCommand()
{
}

CItemUndoRedoCommand *CItemUndoRedoCommand::getCmdByItemCmdInfo(const CUndoRedoCommand::SCommandInfoCouple &info)
{
    CItemUndoRedoCommand *pCmd = nullptr;
    EChangedType tp = EChangedType(info.expendType());
    switch (tp) {
    case EPosChanged: {
        pCmd = new CItemMoveCommand();
        break;
    }
    case ESizeChanged: {
        break;
    }
    case EPropertyChanged: {
        pCmd = new CItemMoveCommand();
        break;
    }
    case EAllChanged: {
        pCmd = new CBzItemAllCommand();
        break;
    }
    default:
        break;
    }
    return pCmd;
}

QGraphicsItem *CItemUndoRedoCommand::item()
{
    return _pItem;
}

void CItemUndoRedoCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    Q_UNUSED(varTp);

    if (!vars.isEmpty())
        _pItem = reinterpret_cast<QGraphicsItem *>(vars.first().toLongLong());
}

CItemMoveCommand::CItemMoveCommand()
    : CItemUndoRedoCommand()
{
}

void CItemMoveCommand::real_undo()
{
    if (item() != nullptr) {
        item()->setPos(_pos[UndoVar]);
    }
}

void CItemMoveCommand::real_redo()
{
    if (item() != nullptr) {
        item()->setPos(_pos[RedoVar]);
    }
}

void CItemMoveCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    CItemUndoRedoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "do not set pos in CMoveItemCommand!!!! varTp = " << varTp;
        return;
    }
    //解析
    //QPointF pos = vars[1].toPointF();
    _pos[varTp] = vars[1].toPointF();
}

CItemSizeCommand::CItemSizeCommand()
{
}

void CItemSizeCommand::real_undo()
{
}

void CItemSizeCommand::real_redo()
{
}

void CItemSizeCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    CItemUndoRedoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "do not set pos in CMoveItemCommand!!!!";
        return;
    }
    //解析
    _sz[varTp] = vars[1].toSizeF();
}

CSceneUndoRedoCommand::CSceneUndoRedoCommand(EChangedType tp)
    : CUndoRedoCommand()
    , _expTp(tp)
{
}

QGraphicsScene *CSceneUndoRedoCommand::scene()
{
    return _scene;
}

PageScene *CSceneUndoRedoCommand::drawScene()
{
    return qobject_cast<PageScene *>(_scene);
}

CSceneUndoRedoCommand::EChangedType CSceneUndoRedoCommand::tp()
{
    return _expTp;
}

void CSceneUndoRedoCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    Q_UNUSED(varTp);

    if (!vars.isEmpty())
        _scene = reinterpret_cast<QGraphicsScene *>(vars.first().toLongLong());
}

CSceneUndoRedoCommand *CSceneUndoRedoCommand::getCmdBySceneCmdInfo(const CUndoRedoCommand::SCommandInfoCouple &info)
{
    CSceneUndoRedoCommand *pCmd = nullptr;
    EChangedType expTp = EChangedType(info.expendType());
    switch (expTp) {
    case ESizeChanged: {
        pCmd = new CSceneBoundingChangedCommand;
        break;
    }
    case EItemAdded:
    case EItemRemoved: {
        pCmd = new CSceneItemNumChangedCommand(expTp == EItemAdded ? CSceneItemNumChangedCommand::Added : CSceneItemNumChangedCommand::Removed);
        break;
    }
    case EGroupChanged: {
        pCmd = new CSceneGroupChangedCommand;
        break;
    }
    default:
        break;
    }
    return pCmd;
}

CSceneItemNumChangedCommand::CSceneItemNumChangedCommand(EChangedType tp)
    : CSceneUndoRedoCommand(EItemAdded)
    , _changedTp(tp)
{
}

CSceneItemNumChangedCommand::~CSceneItemNumChangedCommand()
{
}

void CSceneItemNumChangedCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    CSceneUndoRedoCommand::parsingVars(vars, varTp);

    _Items.clear();
    for (int i = 1; i < vars.size(); ++i) {
        CGraphicsItem *pItem = reinterpret_cast<CGraphicsItem *>(vars[i].toLongLong());
        if (pItem != nullptr)
            _Items.append(pItem);
    }
}

void CSceneItemNumChangedCommand::real_undo()
{
    //操作是删除那么undo就是添加回去
    if (_changedTp == Removed) {
        if (drawScene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                drawScene()->addCItem(_Items[i], false);
            }
        }
    } else {
        if (drawScene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                drawScene()->removeCItem(_Items[i]);
            }
        }
    }
}

void CSceneItemNumChangedCommand::real_redo()
{
    //操作是删除那么redo就是删除
    if (_changedTp == Removed) {
        if (scene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                drawScene()->removeCItem(_Items[i]);
            }
        }
    } else {
        if (scene() != nullptr) {
            for (int i = 0; i < _Items.size(); ++i) {
                drawScene()->addCItem(_Items[i], false);
            }
        }
    }
}

QList<CGraphicsItem *> &CSceneItemNumChangedCommand::items()
{
    return _Items;
}

CBzItemAllCommand::CBzItemAllCommand()
{
}

CBzItemAllCommand::~CBzItemAllCommand()
{
    _itemDate[UndoVar].release();
    _itemDate[RedoVar].release();
}

CGraphicsItem *CBzItemAllCommand::bzItem()
{
    return dynamic_cast<CGraphicsItem *>(item());
}

bool CBzItemAllCommand::isNoNeedSelected(EVarUndoOrRedo tp)
{
    return _noNeedSelected[tp];
}

void CBzItemAllCommand::real_undo()
{
    if (bzItem() != nullptr) {
        bzItem()->loadGraphicsUnit(_itemDate[UndoVar]);
    }
}

void CBzItemAllCommand::real_redo()
{
    if (bzItem() != nullptr) {
        bzItem()->loadGraphicsUnit(_itemDate[RedoVar]);
    }
}

void CBzItemAllCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的item
    CItemUndoRedoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "do not set pos in CBzItemAllCommand!!!!";
        return;
    }
    _itemDate[varTp] = vars[1].value<CGraphicsUnit>();

    if (vars.count() > 2) {
        _noNeedSelected[varTp] = vars[2].toBool();
    }
}

CSceneBoundingChangedCommand::CSceneBoundingChangedCommand():
    CSceneUndoRedoCommand(ESizeChanged)
{
}

void CSceneBoundingChangedCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的scene
    CSceneUndoRedoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "do not set pos in CSceneBoundingChangedCommand!!!!";
        return;
    }
    _rect[varTp] = vars[1].toRectF();
}

void CSceneBoundingChangedCommand::real_undo()
{
    if (scene() != nullptr) {
        scene()->setSceneRect(_rect[UndoVar]);
    }
}

void CSceneBoundingChangedCommand::real_redo()
{
    if (scene() != nullptr) {
        scene()->setSceneRect(_rect[RedoVar]);
    }
}

CCmdBlock::CCmdBlock(PageScene *pScene, CSceneUndoRedoCommand::EChangedType EchangedTp, QGraphicsItem *pItem, bool doRedo):
    CCmdBlock(pScene, EchangedTp, QList<QGraphicsItem *>() << pItem, doRedo)
{

}

CCmdBlock::CCmdBlock(PageScene *pScene, CSceneUndoRedoCommand::EChangedType EchangedTp,
                     const QList<QGraphicsItem *> &list, bool doRedo)
    : _doRedo(doRedo), _pScene(pScene), _scenChangedType(EchangedTp)
{
    if (_pScene == nullptr)
        return;

    //是否需要还原信息(删除图元其实不需要)
    bool needRedoInfo = true;
    //记录undo
    QList<QVariant> vars;
    vars << reinterpret_cast<long long>(pScene);
    if (EchangedTp == CSceneUndoRedoCommand::ESizeChanged) {
        vars << pScene->sceneRect();
    } else if (EchangedTp == CSceneUndoRedoCommand::EItemAdded || EchangedTp == CSceneUndoRedoCommand::EItemRemoved) {
        for (QGraphicsItem *pItem : list) {
            vars << reinterpret_cast<long long>(pItem);
        }
        needRedoInfo = false;
    } else if (EchangedTp == CSceneUndoRedoCommand::EGroupChanged) {
        QVariant var;
        var.setValue(_pScene->getGroupTree());
        vars << var;

        //真正的变动图元,可以用于撤销还原后的选中操作
        for (QGraphicsItem *pItem : list) {
            vars << reinterpret_cast<long long>(pItem);
        }
    }
    CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                        EchangedTp, vars, needRedoInfo);
}

CCmdBlock::CCmdBlock(CGraphicsItem *pItem, EChangedPhase phase, bool doRedo)
    : _pItem(pItem)
    , _phase(phase)
    , _doRedo(doRedo)
{
    if (_pItem == nullptr)
        return;

    if (_phase == EChangedUpdate || _phase == EChangedFinished || _phase == EChangedAbandon)
        return;

    if (_pItem->type() == CutType) {
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pItem->drawScene());
        vars << _pItem->drawScene()->sceneRect();
        CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::ESizeChanged, vars, true);
    }

    QList<CGraphicsItem *> items;
    if (_pItem->type() == MgrType) {
        items = dynamic_cast<CGraphicsItemGroup *>(_pItem)->items(true);

    } else {
        items.append(pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        CGraphicsItem *pItem = items[i];

        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(pItem);
        QVariant varInfo;
        varInfo.setValue(pItem->getGraphicsUnit(EUndoRedo));
        vars << varInfo;

        if (_phase == EChangedBegin || _phase == EChanged) {
            CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::EItemChangedCmd,
                                                CItemUndoRedoCommand::EAllChanged, vars, i == 0);
        }
    }
}

CCmdBlock::~CCmdBlock()
{
    if (_pScene != nullptr) {
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pScene);
        //记录undo
        if (_scenChangedType == CSceneUndoRedoCommand::ESizeChanged) {
            vars << _pScene->sceneRect();

        } else if (_scenChangedType == CSceneUndoRedoCommand::EGroupChanged) {
            QVariant var;
            var.setValue(_pScene->getGroupTree());
            vars << var;
        }

        //添加删除动作不需要还原信息(因为根据添加/删除的对象即可确定还原时应该怎么做,及执行反向的操作就可以了)
        if (_scenChangedType != CSceneUndoRedoCommand::EItemAdded && _scenChangedType != CSceneUndoRedoCommand::EItemRemoved) {
            CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                                _scenChangedType, vars);
        }

        CUndoRedoCommand::finishRecord(_doRedo);
        return;
    }

    if (_pItem == nullptr)
        return;

    if (_phase != EChangedFinished && _phase != EChanged)
        return;

    if (_phase == EChangedAbandon) {
        CUndoRedoCommand::clearCommand();
        return;
    }

    if (_pItem->type() == CutType) {
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(_pItem->drawScene());
        vars << _pItem->drawScene()->sceneRect();
        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd,
                                            CSceneUndoRedoCommand::ESizeChanged, vars);
    }

    QList<CGraphicsItem *> items;
    if (_pItem->type() == MgrType) {
        items = dynamic_cast<CGraphicsItemGroup *>(_pItem)->items(true);

    } else {
        items.append(_pItem);
    }

    for (int i = 0; i < items.size(); ++i) {
        CGraphicsItem *pItem = items[i];

        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(pItem);
        QVariant varInfo;
        varInfo.setValue(pItem->getGraphicsUnit(EUndoRedo));
        vars << varInfo;

        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::EItemChangedCmd,
                                            CItemUndoRedoCommand::EAllChanged, vars);
    }

    if (_pItem->drawScene() != nullptr) {
        _pItem->drawScene()->finishRecord(_doRedo);
    }
}

CSceneGroupChangedCommand::CSceneGroupChangedCommand():
    CSceneUndoRedoCommand(EGroupChanged)
{
}

CSceneGroupChangedCommand::~CSceneGroupChangedCommand()
{
}

void CSceneGroupChangedCommand::parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp)
{
    //保证能获取到操作的scene
    CSceneUndoRedoCommand::parsingVars(vars, varTp);

    //是否能解析的判断
    if (vars.count() < 2) {
        qWarning() << "not get undo info for group changed!";
        return;
    }

    //获取到当前组合的情况
    _inf[varTp] = vars[1].value<PageScene::CGroupBzItemsTree>();


    //是否能解析的判断
    if (vars.count() < 3) {
        //qWarning() << "not get group changed selecte items !";
        return;
    }

    _Items.clear();
    for (int i = 1; i < vars.size(); ++i) {
        CGraphicsItem *pItem = reinterpret_cast<CGraphicsItem *>(vars[i].toLongLong());
        if (pItem != nullptr)
            _Items.append(pItem);
    }

    qDebug() << "_inf[varTp] = " << varTp << " " << _inf[varTp].bzItems.count() << " " << _inf[varTp].childGroups.count();

}

void CSceneGroupChangedCommand::real_undo()
{
    CGraphicsItemGroup *pGroup = drawScene()->loadGroupTree(_inf[UndoVar]);
    if (pGroup != nullptr && pGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
        drawScene()->cancelGroup(pGroup);
        delete pGroup;
    }
    qDebug() << "scene group info = " << drawScene()->bzGroups().count();
}

void CSceneGroupChangedCommand::real_redo()
{
    CGraphicsItemGroup *pGroup = drawScene()->loadGroupTree(_inf[RedoVar]);
    if (pGroup != nullptr && pGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
        drawScene()->cancelGroup(pGroup);
        delete pGroup;
    }
    qDebug() << "scene group info = " << drawScene()->bzGroups().count();
}
