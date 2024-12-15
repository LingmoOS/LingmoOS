// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWTOOL_H
#define CDRAWTOOL_H

#include "../utils/globaldefine.h"
#include "csizehandlerect.h"
#include "attributewidget.h"
#include "toolsceneevent.h"
#include "drawboard.h"
#include "drawboardtoolmgr.h"

#include <QList>
#include <QCursor>
#include <QTouchEvent>
#include <QTime>
#include <QAbstractButton>
#include <QKeySequence>

class QGraphicsSceneMouseEvent;
class PageScene;
class PageItem;
class DrawBoard;
#define TOOL_ICON_RECT QSize(20, 20)
#define TOOL_BUTTON_RECT QSize(37, 37)

#define TOOLINTERFACE(class) \
    extern "C" {\
        \
        Q_DECL_EXPORT DrawTool *creatTool(){return new class;}\
        \
    }

class DRAWLIB_EXPORT DrawTool: public QObject
{
    Q_OBJECT
public:
    DrawTool(QObject *parent = nullptr);
    ~DrawTool();

    virtual int  toolType() const = 0;
    virtual void pressOnScene(ToolSceneEvent *event) = 0;
    virtual void moveOnScene(ToolSceneEvent *event) = 0;
    virtual void releaseOnScene(ToolSceneEvent *event) = 0;
    virtual void doubleClickOnScene(ToolSceneEvent *event);

    void toolPressOnScene(ToolSceneEvent *event);
    void toolMoveOnScene(ToolSceneEvent *event);
    void toolReleaseOnScene(ToolSceneEvent *event);

    /**
     * @brief EStatus 工具的状态
     * EIdle               闲置中
     * EReadyOnly/EActived 表示备点选中了，就绪状态;
     * EWorking            表示正在绘制中;
     * EDisAbled           被禁用了;
     */
    enum EStatus {EIdle, EReady, EActived = EReady, EWorking, EDisAbled};
    EStatus status() const;
    bool isEnable() const;

public slots:
    void setEnable(bool b);
    bool activeTool();
    bool isWorking();
    void interrupt();

public:
    void setTouchSensitiveRadius(int sensitiveRadius = 10);
    int  touchSensitiveRadius()const;

    void setClearSelectionOnActived(bool b);
    bool isSelectionClearOnActived() const;

    void setAttributionShowOnActived(bool b);
    bool isAttributionsShowOnActived() const;

    DrawBoardToolMgr *toolManager() const;
    void setToolManager(DrawBoardToolMgr *pManager);

    DrawBoard *drawBoard()const;
    Page *currentPage()const;

    QCursor cursor() const;
    void    setCursor(const QCursor &curosr);

    int     maxTouchPoint() const;
    void    setMaxTouchPoint(int num);

    QList<int> currentActivedPointIds()const;

    bool    isContinued() const;
    void    setContinued(bool b);

    QAbstractButton *toolButton() const;
    virtual bool isPressEventHandledByQt(ToolSceneEvent *event);

public:
    virtual void enterSceneEvent(ToolSceneEvent *event) {Q_UNUSED(event)}
    virtual void leaveSceneEvent(ToolSceneEvent *event) {Q_UNUSED(event) clearPointRecording();}
    virtual void contextMenuEvent(ToolSceneEvent *event);

    virtual int  minMoveUpdateDistance();
    virtual void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene);
    virtual SAttrisList attributions();
    virtual void setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack);
    virtual bool blockPageBeforeOutput(Page *page);


protected slots:
    virtual void onStatusChanged(EStatus oldStatus, EStatus nowStatus);
    virtual void onToolManagerChanged(DrawBoardToolMgr *old, DrawBoardToolMgr *now);
    virtual void onCurrentPageChanged(Page *newPage);
    virtual void clearPointRecording();


protected:
    SAttri defaultAttriVar(int attri) const;

private:
    bool tryConvertToDoubleClicked(ToolSceneEvent *event);

signals:
    void statusChanged(EStatus oldStatus, EStatus newStatus);
    void toolManagerChanged(DrawBoardToolMgr *old, DrawBoardToolMgr *now);

protected:
    void changeStatusFlagTo(EStatus status);

    PRIVATECLASS(DrawToolBase)

    friend class PageScene;
    friend class DrawBoardToolMgr;
};

#endif // CDRAWTOOL_H
