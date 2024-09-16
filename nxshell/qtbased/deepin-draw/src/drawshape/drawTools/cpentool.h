// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPENTOOL_H
#define CPENTOOL_H
#include "idrawtool.h"
#include <QUndoCommand>
#include <QPicture>
#include <QPointer>
#include "cgraphicslayer.h"

class CPenTool : public IDrawTool
{
    Q_OBJECT
public:
    enum EPenToolType {ENormalPen = 1, ECalligraphyPen, ECrayonPen};

    CPenTool();

    virtual ~CPenTool() override;

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
     * @brief allowedMaxTouchPointCount 允许的最大支持实时绘制点数
     */
    int  allowedMaxTouchPointCount() override;

    /**
     * @brief returnToSelectTool 工具执行的结束
     * @param event 事件
     * @param pInfo 额外信息
     */
    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    /**
     * @brief minMoveUpdateDistance　创建图元时最小移动距离
     */
    int minMoveUpdateDistance() override;

    /**
     * @brief pictureColorChanged　改变图片颜色(不含透明区域)
     */
    QPixmap pictureColorChanged(const QImage &image, const QColor &color);

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    bool autoSupUndoForCreatItem() override {return false;}
protected:
    QPen     getViewDefualtPen(PageView *view) const;
    QBrush   getViewDefualtBrush(PageView *view) const;

    QPicture paintNormalPen(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo);
    QPicture paintCalligraphyPen(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo);
    QPicture paintCrayonPen(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo, qreal space = 8);

    void paintPictureToView(const QPicture &picture, PageView *view);

    bool eventFilter(QObject *o, QEvent *e) override;

protected:
    QMap<PageScene *, JDynamicLayer *> _layers;
    //JDynamicLayer *_layer = nullptr;
    bool _isNewLayer = false;
    QMap<int, JActivedPaintInfo> _activePictures;
    QComboBox *m_pPenStyleComboBox = nullptr;
    QImage m_pRenderImage;
    QPointF m_prePos;
};


#endif // CPENTOOL_H
