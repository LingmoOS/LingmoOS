// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWUNDOCOMMAND_H
#define CDRAWUNDOCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QGraphicsItem>
#include "sitemdata.h"
#include "pagescene.h"
#include <pageobject.h>
#include <QDebug>
#include "../utils/globaldefine.h"

class PageItem;

enum EVarUndoOrRedo { UndoVar,
                      RedoVar,
                      VarTpCount
                    };


struct DRAWLIB_EXPORT UndoType {
    int objectType = -1;
    int opeType = -1;
    int extraType = -1;

    UndoType(int obt = -1, int opt = -1, int ext = -1);
    bool isValid()const;
    bool operator<(const UndoType &other) const;
    bool operator==(const UndoType &other) const;
    bool operator!=(const UndoType &other) const;
    friend QDebug operator <<(QDebug debug, const UndoType &c);
};

struct DRAWLIB_EXPORT UndoKey {
    PageObject *object = nullptr;
    UndoType type;

    UndoKey(PageObject *o = nullptr, UndoType tp = UndoType());
    UndoKey(PageObject *o, int obType, int opeTp = -1);
    bool isValid()const;
    bool operator<(const UndoKey &other) const;
    bool operator==(const UndoKey &other) const;
    bool operator!=(const UndoKey &other) const;
    friend QDebug operator<<(QDebug debug, const UndoKey &c);
};


class UndoCommand;
typedef UndoCommand *(*CREATCMD)() ;
typedef QMap<UndoType, CREATCMD>  CREATCMDMAP;
class DRAWLIB_EXPORT UndoCommandFactory: public QObject
{
    Q_OBJECT
public:
    UndoCommand *creatCmd(const UndoType &type);

    bool canCreatCommand(const UndoType &tp) const;

    void registerCommand(const UndoType &type, CREATCMD f);

    static UndoCommandFactory *instance();

private:
    UndoCommandFactory(QObject *parent = nullptr);
private:
    CREATCMDMAP cmdCreatFunsMap;

    static UndoCommandFactory *staticFactory;
};

class DRAWLIB_EXPORT UndoStack: public QUndoStack
{
    Q_OBJECT
public:
    UndoStack(QObject *parent = nullptr);
    ~UndoStack();

    int  userValidStepCount() const;

    static void recordUndo(UndoKey key, const PageVariantList &datas, bool joinPre = true);
    static void recordRedo(UndoKey key, const PageVariantList &datas);
    static void finishRecord(UndoStack *stack, bool doRedoCmd = false);
    static bool isRecordEmpty();
    static void clearRecordings();

    static void recordUndo(LayerItem *layer, int exptype, const QList<PageItem *> &items = QList<PageItem *>());
    static void recordRedo(LayerItem *layer, int exptype, const QList<PageItem *> &items = QList<PageItem *>());

    static void recordUndo(PageItem *pItem, bool joinPre = true);
    static void recordRedo(PageItem *pItem);

    static void recordUndo(const QList<PageItem *> &items, bool joinPre = true);
    static void recordRedo(const QList<PageItem *> &items);

public slots:
    void push(QUndoCommand *cmd);
    void undo();
    void redo();

    PRIVATECLASS(UndoStack)
};

/**
 * @brief The CUndoRedoCommand class 撤销还原的基类
 */
class DRAWLIB_EXPORT UndoCommand : public QUndoCommand
{
public:
    UndoCommand();

    virtual UndoType commandType()const = 0;

    virtual void real_undo();
    virtual void real_redo();

    void blockRedo(bool b);

    PageObject *undoObject()const;

protected:
    void undo() Q_DECL_FINAL;
    void redo() Q_DECL_FINAL;

protected:
    /**
     * @brief The parsingVars 解析数据参数
     */
    virtual void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) = 0;

protected:
    PRIVATECLASS(UndoCommand)

    friend class UndoStack;
};

/**
 * @brief The CUndoRedoCommandGroup 撤销还原操作组
 */
class DRAWLIB_EXPORT GroupUndoCommand : public UndoCommand
{
public:
    /**
     * @brief The CUndoRedoCommandGroup 构造函数
     */
    explicit GroupUndoCommand();

    ~GroupUndoCommand();

    /**
     * @brief The addCommand 添加一个操作到组
     * @param pCmd 一个可undo，redo的操作类指针
     */
    void addCommand(UndoCommand *pCmd);

    int count();

    UndoType commandType()const override;
    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

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
    QList<UndoCommand *> _allCmds;

    friend class UndoCommand;
};

/**
 * @brief The SceneUndoCommand 场景相关的操作
 */
class DRAWLIB_EXPORT LayerUndoCommand : public UndoCommand
{
public:
    enum ChangedType { RectChanged,
                       ChildItemAdded,
                       ChildItemRemoved,
                       ChildGroupAdded,
                       ChildGroupRemoved,
                       UnitChanged,
                       SenceRectChanged,
                       ChangedCount
                     };
    explicit LayerUndoCommand(ChangedType tp = RectChanged);

    UndoType commandType()const override;

    PageScene *pageScene() const;

    LayerItem *layer()const;

    ChangedType tp()const;

private:
    ChangedType _expTp = RectChanged;
};

class DRAWLIB_EXPORT LayerItemAdded : public LayerUndoCommand
{
public:
    enum ChangedType { Removed,
                       Added
                     };

    explicit LayerItemAdded(ChangedType tp);
    ~LayerItemAdded();

    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

    QList<PageItem *> &items();

    static UndoCommand *creatLayerItemAddedCommand();
    static UndoCommand *creatLayerItemRemovedCommand();

private:
    QList<PageItem *> _Items;
    ChangedType _changedTp;
};

class DRAWLIB_EXPORT LayerGroupAdded: public LayerUndoCommand
{
public:
    enum Type {ADD, REMOVE};
    LayerGroupAdded(Type tp = ADD);
    ~LayerGroupAdded();

    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

    static UndoCommand *creatAddInstance();
    static UndoCommand *creatRemoveInstance();

    void real_undo() override;
    void real_redo() override;

private:
    GroupItem *addedGroup = nullptr;
    QList<PageItem *> items;

    const Type _tp;
};

class DRAWLIB_EXPORT LayerRectUndoCommand : public LayerUndoCommand
{
public:
    LayerRectUndoCommand();

    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

    static UndoCommand *creatInstance() {return new LayerRectUndoCommand;}

private:
    QRectF _rect[VarTpCount];
};

class DRAWLIB_EXPORT SenceRectUndoCommand : public LayerUndoCommand
{
public:
    SenceRectUndoCommand();

    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

    void real_undo() override;

    void real_redo() override;

    static UndoCommand *creatInstance() {return new SenceRectUndoCommand;}

private:
    QRectF _rect[VarTpCount];
};

/**
 * @brief The CItemUndoRedoCommand 图元相关的操作
 */
class DRAWLIB_EXPORT ItemUndoCommand : public UndoCommand
{
public:
    /**
     * @brief The CUndoCommandGroup 构造函数
     */
    enum EChangedType { EPosChanged,
                        EZValueChanged,
                        EUnitChanged,
                        EChangedCount
                      };

    ItemUndoCommand(EChangedType tp);

    UndoType commandType()const override;

    PageItem *pageItem() const;

    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

private:
    const EChangedType _tp;
};

/**
 * @brief The CItemMoveCommand 画板 移动图元
 */
class DRAWLIB_EXPORT ItemUnitUndoCommand : public ItemUndoCommand
{
public:
    ItemUnitUndoCommand();
    ~ItemUnitUndoCommand();

    static UndoCommand *creatInstance();

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
    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

private:
    Unit _itemDate[VarTpCount];
};

/**
 * @brief The CItemMoveCommand 画板 移动图元
 */
class DRAWLIB_EXPORT ItemPosUndoCommand : public ItemUndoCommand
{
public:
    ItemPosUndoCommand();

    static UndoCommand *creatInstance();
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
    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

private:
    QPointF _pos[VarTpCount];
};

/**
 * @brief The CItemSizeCommand  item图元大小变化
 */
class DRAWLIB_EXPORT ItemZUndoCommand : public ItemUndoCommand
{
public:
    ItemZUndoCommand();

    UndoType commandType()const override;

    static UndoCommand *creatInstance();

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
    void parsingVars(const PageVariantList &vars, EVarUndoOrRedo varTp) override;

private:
    qreal _z[VarTpCount] = {0, 0};
};


class PageScene;


//快捷使用撤销还原的类
class DRAWLIB_EXPORT UndoRecorder
{
public:
    UndoRecorder(LayerItem *layer,
                 LayerUndoCommand::ChangedType EchangedTp,
                 PageItem *pItem, bool doRedo = false);
    UndoRecorder(LayerItem *layer,
                 LayerUndoCommand::ChangedType EchangedTp,
                 const QList<PageItem *> &list = QList<PageItem *>(), bool doRedo = false);
    explicit UndoRecorder(PageItem *pItem, EChangedPhase phase = EChanged, bool doRedo = false);
    ~UndoRecorder();

private:
    PageItem *_pItem = nullptr;
    EChangedPhase _phase = EChangedUpdate;
    bool _doRedo = false;

    LayerItem *_pLayer = nullptr;
    LayerUndoCommand::ChangedType _scenChangedType = LayerUndoCommand::ChangedCount;

};

#endif // CDRAWUNDOCOMMAND_H
