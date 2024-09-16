// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SPRAYGUNTOOL_H
#define SPRAYGUNTOOL_H

#include "drawfunctiontool.h"
class RasterItem;

class Q_DECL_EXPORT SprayGunTool: public DrawFunctionTool
{
    Q_OBJECT
public:
    SprayGunTool(QObject *parent = nullptr);

    int toolType() const override;

    SAttrisList attributions() override;

    int  minMoveUpdateDistance() override;

    void funcStart(ToolSceneEvent *event)  override;
    void funcUpdate(ToolSceneEvent *event, int decided) override;
    void funcFinished(ToolSceneEvent *event, int decided) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;
    virtual void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene);
    PageItem *currentLayer(ToolSceneEvent *event);
private:
    QTimer      *m_timer;
    QPoint      m_pos;
    QImage      *m_img;
    RasterItem  *m_layer;
    QGraphicsScene   *m_scene;
    PRIVATECLASS(SprayGunTool)
};

#endif // SPRAYGUNTOOL_H
