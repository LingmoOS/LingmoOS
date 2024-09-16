// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSELECTTOOL_H
#define CSELECTTOOL_H
#include "idrawtool.h"
#include "globaldefine.h"
#include "csizehandlerect.h"


#include <QCursor>
#include <QTime>
#include <QPainterPath>

class CGraphicsItem;
class CGraphicsRotateAngleItem;
class CGraphicsItemGroup;
class CGraphicsItemHighLight;
class CGraphicsRectItem;
class GraphicsEllipseItem;
class QUndoCommand;

class CSelectTool : public IDrawTool
{
    Q_OBJECT
public:
    CSelectTool();

    ~CSelectTool() override;

    QCursor cursor() const override;

    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;
    DrawAttribution::SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack) override;
    void  registerAttributionWidgets() override;
    enum EOperateType { ENothingDo = 0,
                        ERectSelect,
                        EDragMove,
                        EResizeMove,
                        ERotateMove,
                        ECopyMove,
                        EDragSceneMove,
                        EOperateCount
                      };

    /**
     * @brief toolStart　工具开始事件
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolStart　 工具刷新事件
     * @param event       当次事件信息
     * @param pInfo       记录信息
     */
    void toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolStart　工具结束事件
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    void toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolDoubleClikedEvent　双击事件
     * @param event      当次事件信息
     */
    void toolDoubleClikedEvent(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolStart　判断工具活跃类型
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    int decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolStart　鼠标hover事件（处理高亮，鼠标样式变化等）
     * @param event      当次事件信息
     */
    void mouseHoverEvent(CDrawToolEvent *event) override;

    void leaveEvent(CDrawToolEvent *event) override;

    /**
     * @brief painter　绘制更多的内容（用于绘制框选矩形和高亮路径）
     * @param painter  绘制指针
     * @param rect     矩形大小
     * @param scene    场景指针
     */
    void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene) override;

protected:
    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    /**
     * @brief sendToolEventToItem 将工具事件发送给图元，事件将会根据各工具的类型生成对应的图元事件
     */
    void sendToolEventToItem(CDrawToolEvent *event,
                             ITERecordInfo *info,
                             EChangedPhase phase) override;

    /**
     * @brief processItemsMove 处理图元的移动
     */
    void processItemsMove(CDrawToolEvent *event,
                          ITERecordInfo *info,
                          EChangedPhase phase);

    /**
     * @brief processItemsMove 处理图元的缩放调整大小
     */
    void processItemsScal(CDrawToolEvent *event,
                          ITERecordInfo *info,
                          EChangedPhase phase);

    /**
     * @brief processItemsMove 处理图元的旋转
     */
    void processItemsRot(CDrawToolEvent *event,
                         ITERecordInfo *info,
                         EChangedPhase phase);

    void processHightLight(CDrawToolEvent *event);

    void processCursor(CDrawToolEvent *event);

    bool eventFilter(QObject *o, QEvent *e) override;

private:
    /* 当前有item正在被拖拽移动 */
    bool m_isItemMoving = false;

    /* 高亮路径 */
    QPainterPath _hightLight;

    QLabel *_titleLabe = nullptr;
};

#endif // CSELECTTOOL_H
