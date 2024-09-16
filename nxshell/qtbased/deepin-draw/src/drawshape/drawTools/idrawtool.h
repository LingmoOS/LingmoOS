// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWTOOL_H
#define CDRAWTOOL_H

#include "globaldefine.h"
#include "csizehandlerect.h"
#include "cattributeitemwidget.h"
#include "idrawtoolevent.h"
#include "ccentralwidget.h"

#include <QList>
#include <QCursor>
#include <QTouchEvent>
#include <QTime>

class QGraphicsSceneMouseEvent;
class PageScene;
class CGraphicsItem;
class DrawBoard;
using namespace DrawAttribution;

class IDrawTool: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief EStatus 工具的状态
     * EIdle               闲置中
     * EReadyOnly/EActived 表示备点选中了，就绪状态;
     * EWorking            表示正在绘制中;
     * EDisAbled           被禁用了;
     */
    enum EStatus {EIdle, EReady, EActived = EReady, EWorking, EDisAbled};

    /**
     * @brief IDrawTool 构造函数
     * @param mode 工具类型
     */
    explicit IDrawTool(EDrawToolMode mode);

    /**
     * @brief ~IDrawTool 析构函数
     */
    virtual ~IDrawTool();

    EStatus status();

    void setEnable(bool b);

    bool activeTool();

    /**
     * @brief isWorking 是否是正在执行(有触控或者鼠标左键点住中)
     */
    bool isWorking();

    /**
     * @brief interrupt 停止并打断working状态
     */
    void interrupt();

    DrawBoard *drawBoard()const;
    void setDrawBoard(DrawBoard *board);

    Page *currentPage()const;


    void setTouchSensitiveRadius(int sensitiveRadius = 10);
    //int  touchSensitiveRadius()const;

signals:
    void statusChanged(EStatus oldStatus, EStatus newStatus);
    void boardChanged(DrawBoard *old, DrawBoard *cur);

public:
    /**
     * @brief toolButton 工具的激活按钮
     */
    QAbstractButton *toolButton();

    virtual QCursor cursor() const;

    /**
     * @brief attributions 工具的属性
     */
    virtual DrawAttribution::SAttrisList attributions();

    /**
     * @brief setAttributionVar 设置工具的属性
     */
    virtual void  setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack);

    /**
     * @brief toolButton 注册属性界面
     */
    virtual void  registerAttributionWidgets();


    Q_SLOT virtual void onStatusChanged(EStatus oldStatus, EStatus nowStatus);

    /**
     * @brief mousePressEvent 鼠标按下事件
     * @param event 事件
     * @param scene 场景
     */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event, PageScene *scene);

    /**
     * @brief mouseMoveEvent 鼠标移动事件
     * @param event 事件
     * @param scene 场景
     */
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event, PageScene *scene);

    /**
     * @brief mouseReleaseEvent 鼠标放开事件
     * @param event 事件
     * @param scene 场景
     */
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PageScene *scene);

    /**
     * @brief mouseDoubleClickEvent 鼠标双击事件
     * @param event 事件
     * @param scene 场景
     */
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event, PageScene *scene);

    virtual void enterEvent(CDrawToolEvent *event);

    virtual void leaveEvent(CDrawToolEvent *event);

    /**
     * @brief toolDoStart 工具执行的开始
     * @param scene 场景
     */
    void toolDoStart(CDrawToolEvent *event);

    /**
     * @brief toolDoUpdate 工具执行的刷新
     * @param event 事件
     */
    void toolDoUpdate(CDrawToolEvent *event);

    /**
     * @brief toolDoFinish 工具执行的结束
     * @param event 事件
     */
    void toolDoFinish(CDrawToolEvent *event);

    /**
     * @brief isEnable 工具当前是否可用
     */
    virtual bool isEnable(PageView *pView);

    Q_INVOKABLE void refresh();

protected:
    /**
     * @brief toolButton 定义工具的激活按钮
     */
    virtual QAbstractButton *initToolButton();

    DrawAttribution::SAttri defaultAttriVar(int attri) const;

    struct ITERecordInfo;

    /**
     * @brief dueTouchDoubleClickedStart　判定工具开始事件是否是一次双击事件，如果是就会转成双击
     * @param event      当次事件信息
     */
    bool dueTouchDoubleClickedStart(CDrawToolEvent *event);

    /**
     * @brief toolStart　工具开始事件
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    virtual void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolStart　 工具刷新事件
     * @param event       当次事件信息
     * @param pInfo       记录信息
     */
    virtual void toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolStart　工具结束事件
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    virtual void toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolStart　判断工具活跃类型
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    virtual int decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolStart　鼠标hover事件（处理高亮，鼠标样式变化等）
     * @param event      当次事件信息
     */
    virtual void mouseHoverEvent(CDrawToolEvent *event);

    /**
     * @brief toolDoubleClikedEvent　双击事件
     * @param event      当次事件信息
     */
    virtual void toolDoubleClikedEvent(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolCreatItemStart　工具创造业务图元的开始
     * @param event               当次事件信息
     * @param pInfo               记录信息
     */
    virtual void toolCreatItemStart(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolCreatItemUpdate　工具创造业务图元的刷新
     * @param event                当次事件信息
     * @param pInfo                记录信息
     */
    virtual void toolCreatItemUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief toolCreatItemFinish　工具创造业务图元的结束
     * @param event                当次事件信息
     * @param pInfo                记录信息
     */
    virtual void toolCreatItemFinish(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief creatItem　工具创造业务图元指针
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    virtual CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief minMoveUpdateDistance　工具创造业务图元指针
     */

    virtual int minMoveUpdateDistance();

public:
    virtual bool blockPageBeforeOutput(Page *page);
    /**
     * @brief painter　绘制更多的内容（用于绘制框选矩形和高亮路径）
     * @param painter  绘制指针
     * @param rect     矩形大小
     * @param scene    场景指针
     */
    virtual void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene);

    /**
     * @brief clearITE 清理ITE事件记录
     */
    void clearITE();

    /**
     * @brief setViewToolTo 设置当前页的工具
     * @param pView  当前页view
     * @param mode   工具类型
     */
    static void setViewToSelectionTool(PageView *pView = nullptr);

    /**
     * @brief getDrawToolMode 获取当前工具类型
     * @return 工具类型
     */
    EDrawToolMode getDrawToolMode() const;

    /**
     * @brief getCursor 获取鼠标显示的样式
     * @param dir 方向
     * @param bMouseLeftPress true: 鼠标按下 false:鼠标没按下
     * @param toolType 0: 公共 1:selectTool
     * @return
     */
    static QCursor getCursor(CSizeHandleRect::EDirection dir, bool bMouseLeftPress = false, char toolType = 0);

protected:
    /**
     * @brief allowedMaxTouchPointCount 允许的最大触控点数
     */
    virtual int  allowedMaxTouchPointCount();

    /**
     * @brief isPressEventHandledByQt 是否将当前的事件交给qt框架去处理(一般应用于原生的qt控件及代理类，这样才能最大化让qt工作)
     */
    virtual bool isPressEventHandledByQt(CDrawToolEvent *event, ITERecordInfo *pInfo);

    /**
     * @brief returnToSelectTool 是否在一次完整的系列事件（点下->移动（可选）->释放）返回到select工具
     */
    virtual bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo);


    /**
     * @brief autoSupUndoForCreatItem 是否支持在创建生成图元后自动添加到undo栈(默认是true)
     */
    virtual bool autoSupUndoForCreatItem();


    virtual bool rasterItemToLayer(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo);

    /**
     * @brief sendToolEventToItem 将工具事件发送给图元，事件将会根据各工具的类型生成对应的图元事件
     */
    virtual void sendToolEventToItem(CDrawToolEvent *event,
                                     ITERecordInfo *info,
                                     EChangedPhase phase);


    bool isFirstEvent();
    bool isFinalEvent();

private:

    /**
     * @brief getCurVaildActivedPointCount 得到当前有效的激活的触控数(用于和allowedMaxTouchPointCount对比判断是否已经达到允许的最大触控点数)
     */
    int getCurVaildActivedPointCount();

    void changeStatusFlagTo(EStatus status);

protected:
    bool m_bMousePress;
    QPointF m_sPointPress;
    QPointF m_sLastPress;
    QPointF m_sPointRelease;

    bool m_bShiftKeyPress;
    bool m_bAltKeyPress;


    enum EEventLifeTo {EDoNotthing, EDoQtCoversion, ENormal};
    enum EToolOperate { EToolCreatItemMove = -6542,
                        EToolDoNothing = 0
                      };
    struct ITERecordInfo {
        EEventLifeTo eventLife = ENormal;
        QPointF _startPos;
        QPointF _prePos;
        QPointF m_sPointRelease;
        CGraphicsItem *businessItem = nullptr;
        QList<QGraphicsItem *> startPosItems;
        CGraphicsItem *startPosTopBzItem = nullptr;
        bool haveDecidedOperateType = false;
        bool _isvaild = false;
        int _opeTpUpdate = EToolDoNothing;
        int _etcopeTpUpdate = -1;
        int _elapsedToUpdate = -1;
        QList<QGraphicsItem *> etcItems;
        PageScene *_scene = nullptr;
        CDrawToolEvent _startEvent;
        CDrawToolEvent _preEvent;
        CDrawToolEvent _curEvent;
        bool _moved = false;

        bool isVaild() const;
        bool hasMoved() const;
        int elapsedFromStartToUpdate() const { return _elapsedToUpdate; }
        inline QTime *getTimeHandle();

    private:
        QTime _elapsedToUpdateTimeHandle;
    };

    QMap<int, ITERecordInfo> _allITERecordInfo;

public:
    QGraphicsItem *m_noShiftSelectItem;
private:
    EDrawToolMode m_mode;
    QCursor m_RotateCursor;
    QCursor m_LeftTopCursor;
    QCursor m_RightTopCursor;
    QCursor m_LeftRightCursor;
    QCursor m_UpDownCursor;

    QTimer *getTimerForDoubleCliked();
    static QTimer *s_timerForDoubleClike;


    bool _registedWidgets = false;

    QAbstractButton *_pToolButton = nullptr;
    DrawBoard *_drawBoard = nullptr;

    EStatus _status = EIdle;
    bool    _isTouchSensitive = false;
    int     _touchSensitiveRadius = 10;

    friend class CDrawToolFactory;
    friend class DrawToolManager;
};

#endif // CDRAWTOOL_H
