// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CERASERTOOL_H
#define CERASERTOOL_H
#include "drawitemtool.h"
#include "rasteritem.h"
#include "globaldefine.h"

class DRAWLIB_EXPORT EraserTool : public DrawItemTool
{
    Q_OBJECT
public:
    EraserTool(QObject *parent = nullptr);

    ~EraserTool() override;

    int toolType() const override;

    SAttrisList attributions() override;

protected:

    PageItem *drawItemStart(ToolSceneEvent *event) override;
    void      drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;
    void      drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;
    void      drawItemHover(ToolSceneEvent *event) override;

    int  minMoveUpdateDistance() override;

    void onCurrentPageChanged(Page *newPage) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    void enterSceneEvent(ToolSceneEvent *event) override;

    void leaveSceneEvent(ToolSceneEvent *event) override;

    void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene) override;

protected:
    RasterPaint paintTempErasePen(ToolSceneEvent *event, RasterItem *pItem);

    RasterItem *getRasterItem(ToolSceneEvent *event);

    Q_SLOT void _onSceneSelectionChanged(const QList<PageItem * > &selectedItems);

    PRIVATECLASS(EraserTool)
};

#endif // CERASERTOOL_H
