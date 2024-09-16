// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CFILLTOOL_H
#define CFILLTOOL_H
#include "drawfunctiontool.h"
class RasterItem;
class Q_DECL_EXPORT FillTool: public DrawFunctionTool
{
    Q_OBJECT
public:
    FillTool(QObject *parent = nullptr);

    int toolType() const override;

    SAttrisList attributions() override;

    int  minMoveUpdateDistance() override;

    void funcStart(ToolSceneEvent *event)  override;
    void funcUpdate(ToolSceneEvent *event, int decided) override;
    void funcFinished(ToolSceneEvent *event, int decided) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    PageItem *currentLayer(ToolSceneEvent *event);
};

#endif // CFILLTOOL_H
