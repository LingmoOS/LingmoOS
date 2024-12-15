// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPENTOOL_H
#define CPENTOOL_H
#include "drawitemtool.h"
#include "rasteritem.h"

#include <QUndoCommand>
#include <QPicture>
#include <QPointer>

class DRAWLIB_EXPORT PenTool : public DrawItemTool
{
    Q_OBJECT
public:
    enum EPenToolType {ENormalPen = 1, ECalligraphyPen, ECrayonPen};

    PenTool(QObject *parent = nullptr);

    int toolType() const override;

    SAttrisList attributions() override;

protected:
    PageItem *drawItemStart(ToolSceneEvent *event) override;

    void drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;

    void drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;

    int minMoveUpdateDistance() override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    PRIVATECLASS(PenTool)
};


#endif // CPENTOOL_H
