// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWUNDOCOMMAND_H
#define CDRAWUNDOCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QGraphicsItem>
#include "drawshape/sitemdata.h"
#include "cdrawscene.h"

class CGraphicsItem;
//class CDrawScene;

//using funForUndoRedo = void(*)();
/**
 * @brief The CUndoRedoCommand class 撤销还原的基类
 */
class CUndoRedoCommand : public QUndoCommand
{
public:
    enum EDrawUndoCmdType {
        ESceneChangedCmd,
        EItemChangedCmd,
        EDrawGroupCmd,
        EDrawCmdUnknowed,
        EDrawCmdCount
    };
//    enum EVarUndoOrRedo { UndoVar,
//                          RedoVar,
//                          VarTpCount
//                        };

    /**
     * @brief The CUndoRedoCommand 构造函数
     */
    CUndoRedoCommand();

    /**
     * @brief The recordUndoCommand 记录操作
     * @param tp　操作的类型
     * @param tp　操作的额外类型标记 如果没有额外的就填-1
     * @param datas　undo操作相关的数据参数等（不同的子类不同的解释）
     * @param init   是否清除之前的复合操作
     * @param return  返回绑定的undo命令的索引(用于匹对redo)
     */
    static void recordUndoCommand(EDrawUndoCmdType tp,
                                  int expendTp,
                                  const QList<QVariant> &datas,
                                  bool init = false,
                                  bool NoNeedRedoVars = true);

    /**
     * @brief The recordCommand 记录操作
     * @param undoIndex　undo的索引用于将当前的redo指令与其匹配
     * @param tp　操作的类型
     * @param datas　redo操作相关的数据参数等（不同的子类不同的解释）
     */
    static void recordRedoCommand(EDrawUndoCmdType tp, int expendTp, const QList<QVariant> &datas);

    /**
     * @brief The clearCommand 执行记录了的所有操作(执行完成后会调用clearCommand清除)
     */
    static void finishRecord(bool doRedoCmd = false);


    static bool isRecordEmpty();

    /**
     * @brief The clearCommand 清除记录了的所有操作
     */
    static void clearCommand();

    static void pushStack(CUndoRedoCommand *pCmd);

public:
    void undo() Q_DECL_FINAL;

    void redo() Q_DECL_FINAL;

    static void setBlockRedoWhenPushedToStack(bool b);
//    static bool isBlockRedoWhenPushedToStack();

    virtual void real_undo();
    virtual void real_redo();


public:
    /**
     * @brief The setVar 设置数据参数
     * @param vars 参数
     */
    void setVar(const QList<QVariant> &vars, EVarUndoOrRedo varTp);

    /**
     * @brief The var 数据参数
     */
    inline QList<QVariant> var(EVarUndoOrRedo varTp);

protected:
    /**
     * @brief The parsingVars 解析数据参数
     */
    virtual void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp);

public:
    static void setUndoRedoStack(QUndoStack *stack, void *pGraphicsView);
    static QUndoStack *getUndoRedoStack(void *p = nullptr);

protected:
    struct SCommandInfo {
        EDrawUndoCmdType tp = EDrawCmdUnknowed;
        EVarUndoOrRedo urFlag = VarTpCount;
        int expTp = -1;
        QList<QVariant> vars;
        void clear()
        {
            tp = EDrawCmdUnknowed;
            expTp = -1;
            vars.clear();
        }
    };

    struct SCommandInfoCouple {
        SCommandInfo undoInfo;
        SCommandInfo redoInfo;
        bool noneedRedoVars = true;

        bool isVaild() const
        {
            return (undoInfo.tp == redoInfo.tp && undoInfo.tp != EDrawCmdUnknowed);
        }

        EDrawUndoCmdType type() const
        {
            return undoInfo.tp;
        }
        int expendType() const
        {
            return undoInfo.expTp;
        }
    };
    static CUndoRedoCommand *getCmdByCmdInfo(const SCommandInfoCouple &info);

    typedef QList<SCommandInfoCouple> QCommandInfoList;

    static QCommandInfoList s_recordedCmdInfoList;

    struct CKey {
        long long obj;
        EDrawUndoCmdType cmdObjType;
        int expTp;

        CKey(long long ob = 0, EDrawUndoCmdType t1 = EDrawCmdUnknowed, int t2 = -1)
            : obj(ob)
            , cmdObjType(t1)
            , expTp(t2)
        {
        }

        bool operator<(const CKey &other) const
        {
            if (obj < other.obj) {
                return true;
            } else if (obj == other.obj) {
                if (cmdObjType < other.cmdObjType) {
                    return true;
                } else if (cmdObjType == other.cmdObjType) {
                    if (expTp < other.expTp)
                        return true;
                }
            }
            return false;
        }
    };
    static QMap<CKey, int> s_forFindCoupleMap;

    static QMap<void *, QUndoStack *> s_undoStacks;

    QList<QVariant> _vars[VarTpCount];

    static bool s_blockRedoWhenPushToStack;
};

/**
 * @brief The CFunUndoRedo 执行一个函数的undo redo
 */
template<class UndoFun = void (*)(), class RedoFun = void (*)()>
class CFunUndoRedo : public CUndoRedoCommand
{
public:
    CFunUndoRedo(UndoFun uf = nullptr, RedoFun rf = nullptr)
    {
        setFun(uf, rf);
    }

    void setFun(UndoFun uf, RedoFun rf)
    {
        _uf = uf;
        _rf = rf;
    }

    /**
     * @brief The undo 执行undo
     */
    void real_undo() override
    {
        if (_uf != nullptr)
            _uf();
    }

    /**
     * @brief The redo 执行redo
     */
    void real_redo() override
    {
        if (_uf != nullptr)
            _rf();
    }

private:
    UndoFun _uf = nullptr;
    RedoFun _rf = nullptr;
};

/**
 * @brief The CUndoRedoCommandGroup 撤销还原操作组
 */
class CUndoRedoCommandGroup : public CUndoRedoCommand
{
public:
    /**
     * @brief The CUndoRedoCommandGroup 构造函数
     */
    explicit CUndoRedoCommandGroup(bool noticeOnEnd = true);

    ~CUndoRedoCommandGroup();

    /**
     * @brief The addCommand 添加一个操作到组
     * @param pCmd 一个可undo，redo的操作类指针
     */
    void addCommand(CUndoRedoCommand *pCmd);

    int count();

public:
    /**
     * @brief The undo 执行undo
     */
    void real_undo() override;

    /**
     * @brief The redo 执行redo
     */
    void real_redo() override;

private:
    bool addCommand(const SCommandInfoCouple &pCmd);

    void noticeUser(EVarUndoOrRedo tp);

private:
    QList<CUndoRedoCommand *> _allCmds;

    bool _noticeOnfinished = true;

    friend class CUndoRedoCommand;
};

/**
 * @brief The CSceneUndoRedoCommand 场景相关的操作
 */
class CSceneUndoRedoCommand : public CUndoRedoCommand
{
public:
    enum EChangedType { ESizeChanged,
                        EItemAdded,
                        EItemRemoved,
                        EGroupChanged,
                        EAllChanged,
                        EChangedCount
                      };
    explicit CSceneUndoRedoCommand(EChangedType tp = ESizeChanged);

    inline QGraphicsScene *scene();

    inline PageScene *drawScene();

    EChangedType tp();

    /**
     * @brief The parsingVars (默认解析第一个为scene 如需更多参数获取记得子类继承自己解析)
     * @param vars　所有参数数据
     */
    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

    static CSceneUndoRedoCommand *getCmdBySceneCmdInfo(const SCommandInfoCouple &info);

private:
    QGraphicsScene *_scene = nullptr;
    EChangedType _expTp = ESizeChanged;
};

class CSceneItemNumChangedCommand : public CSceneUndoRedoCommand
{
public:
    enum EChangedType { Removed,
                        Added
                      };

    explicit CSceneItemNumChangedCommand(EChangedType tp);
    ~CSceneItemNumChangedCommand();

    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

    QList<CGraphicsItem *> &items();

private:
    QList<CGraphicsItem *> _Items;
    EChangedType _changedTp;
};

class CSceneBoundingChangedCommand : public CSceneUndoRedoCommand
{
public:
    CSceneBoundingChangedCommand();

    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

private:
    QRectF _rect[VarTpCount];
};

class CSceneGroupChangedCommand : public CSceneUndoRedoCommand
{
public:
    CSceneGroupChangedCommand();
    ~CSceneGroupChangedCommand();

    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

    QList<CGraphicsItem *> realChangedItems() {return _Items;}

private:
    PageScene::CGroupBzItemsTree _inf[VarTpCount];

    QList<CGraphicsItem *> _Items;
};

/**
 * @brief The CItemUndoRedoCommand 图元相关的操作
 */
class CItemUndoRedoCommand : public CUndoRedoCommand
{
public:
    /**
     * @brief The CUndoCommandGroup 构造函数
     */
    enum EChangedType { EPosChanged,
                        ESizeChanged,
                        EPropertyChanged,
                        EAllChanged,
                        EChangedCount
                      };

    CItemUndoRedoCommand();

    static CItemUndoRedoCommand *getCmdByItemCmdInfo(const SCommandInfoCouple &info);

    inline  QGraphicsItem *item();
    virtual bool isNoNeedSelected(EVarUndoOrRedo tp) {Q_UNUSED(tp); return false;}

    /**
     * @brief The parsingVars (默认解析第一个为item 如需更多参数获取记得子类继承自己解析)
     * @param vars　所有参数数据
     */
    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

private:
    QGraphicsItem *_pItem = nullptr;
};

/**
 * @brief The CItemMoveCommand 画板 移动图元
 */
class CBzItemAllCommand : public CItemUndoRedoCommand
{
public:
    CBzItemAllCommand();
    ~CBzItemAllCommand();

    inline CGraphicsItem *bzItem();

    bool isNoNeedSelected(EVarUndoOrRedo tp) override;


public:
    /**
     * @brief The undo 执行undo
     */
    void real_undo() override;

    /**
     * @brief The redo 执行redo
     */
    void real_redo() override;

    /**
     * @brief The parsingVars (默认解析第一个为item 如需更多参数获取记得子类继承自己解析)
     * @param vars　所有参数数据
     */
    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

private:
    CGraphicsUnit _itemDate[VarTpCount];
    bool          _noNeedSelected[VarTpCount] = {false, false};
};

/**
 * @brief The CItemMoveCommand 画板 移动图元
 */
class CItemMoveCommand : public CItemUndoRedoCommand
{
public:
    CItemMoveCommand();

public:
    /**
     * @brief The undo 执行undo
     */
    void real_undo() override;

    /**
     * @brief The redo 执行redo
     */
    void real_redo() override;

    /**
     * @brief The parsingVars (默认解析第一个为item 如需更多参数获取记得子类继承自己解析)
     * @param vars　所有参数数据
     */
    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

private:
    QPointF _pos[VarTpCount];
};

/**
 * @brief The CItemSizeCommand  item图元大小变化
 */
class CItemSizeCommand : public CItemUndoRedoCommand
{
public:
    CItemSizeCommand();

public:
    /**
     * @brief The undo 执行undo
     */
    void real_undo() override;

    /**
     * @brief The redo 执行redo
     */
    void real_redo() override;

    /**
     * @brief The parsingVars (默认解析第一个为item 如需更多参数获取记得子类继承自己解析)
     * @param vars　所有参数数据
     */
    void parsingVars(const QList<QVariant> &vars, EVarUndoOrRedo varTp) override;

private:
    QSizeF _sz[VarTpCount];
};


class PageScene;


//快捷使用撤销还原的类
class CCmdBlock
{
public:
    CCmdBlock(PageScene *pScene,
              CSceneUndoRedoCommand::EChangedType EchangedTp,
              QGraphicsItem *pItem, bool doRedo = false);
    CCmdBlock(PageScene *pScene,
              CSceneUndoRedoCommand::EChangedType EchangedTp = CSceneUndoRedoCommand::ESizeChanged,
              const QList<QGraphicsItem *> &list = QList<QGraphicsItem *>(), bool doRedo = false);
    CCmdBlock(CGraphicsItem *pItem, EChangedPhase phase = EChanged, bool doRedo = false);
    ~CCmdBlock();

private:
    CGraphicsItem *_pItem = nullptr;
    EChangedPhase _phase = EChangedUpdate;
    bool _doRedo = false;

    PageScene *_pScene = nullptr;
    CSceneUndoRedoCommand::EChangedType _scenChangedType = CSceneUndoRedoCommand::EChangedCount;
};

#endif // CDRAWUNDOCOMMAND_H
