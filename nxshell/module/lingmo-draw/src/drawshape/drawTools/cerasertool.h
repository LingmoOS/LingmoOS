// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CERASERTOOL_H
#define CERASERTOOL_H
#include "idrawtool.h"
#include "cpentool.h"
#include "cgraphicslayer.h"
#include "globaldefine.h"
#include "cattributeitemwidget.h"
#include "application.h"
#include "widgets/toolbutton.h"
#include "drawshape/cdrawparamsigleton.h"
#include "drawshape/globaldefine.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "widgets/cspinbox.h"
#include "cattributemanagerwgt.h"
#include "cgraphicslayer.h"
#include "cdrawscene.h"

#include <QUndoCommand>
#include <QPicture>
#include <QPointer>
#include <DToolButton>
#include <DLabel>
#include <QSvgRenderer>

class CEraserTool : public IDrawTool
{
    Q_OBJECT
public:
    CEraserTool();

    virtual ~CEraserTool() override;

    DrawAttribution::SAttrisList attributions() override;

    QCursor cursor() const override;

protected:
    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;

    void  registerAttributionWidgets() override;

    CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolStart　工具开始事件
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo) override;


    /**
     * @brief toolStart　判断工具活跃类型
     * @param event      当次事件信息
     * @param pInfo      记录信息
     */
    int decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

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
     * @brief returnToSelectTool 工具执行的结束
     * @param event 事件
     * @param pInfo 额外信息
     */
    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    bool isEnable(PageView *pView) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    void mouseHoverEvent(CDrawToolEvent *event) override;

    void enterEvent(CDrawToolEvent *event) override;

    void leaveEvent(CDrawToolEvent *event) override;

    void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene) override;

    bool eventFilter(QObject *o, QEvent *e) override;

protected:
    QPicture paintTempErasePen(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo);

private:
    /**
     * @brief saveZ 保存当前场景下图元的z值,必须与restoreZ成对出现
     */
    void saveZ(PageScene *scene);

    /**
     * @brief saveItemZValue 缓存图元的z值
     */
    void saveItemZValue(CGraphicsItem *pItem);

    /**
     * @brief restoreZ 还原当前场景下图元的z值,必须与saveZ成对出现
     */
    void restoreZ();

private:
    QMap<PageScene *, JDynamicLayer *> _layers;

    QMap<int, JActivedPaintInfo> _activePictures;

    QMap<CGraphicsItem *, qreal> _tempZs;

    QList<QLineF>               m_tempLines;

    bool m_leaved = false;
};

#endif // CERASERTOOL_H
