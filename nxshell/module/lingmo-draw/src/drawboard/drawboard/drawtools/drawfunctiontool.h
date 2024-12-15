// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWFUCTIONTOOL_H
#define DRAWFUCTIONTOOL_H
#include "drawtool.h"

class DRAWLIB_EXPORT DrawFunctionTool: public DrawTool
{
    Q_OBJECT
public:
    DrawFunctionTool(QObject *parent = nullptr);

    void pressOnScene(ToolSceneEvent *event) final;
    void moveOnScene(ToolSceneEvent *event) final;
    void releaseOnScene(ToolSceneEvent *event) final;

    void clearPointRecording() override;

    int elapsedFromBeginToDecide(int eventId)const;
    int decideValue(int eventId) const;

    virtual void funcStart(ToolSceneEvent *event) = 0;
    virtual void funcUpdate(ToolSceneEvent *event, int decided) = 0;
    virtual void funcFinished(ToolSceneEvent *event, int decided) = 0;

    virtual int  funcDecide(ToolSceneEvent *event);
    virtual void funHover(ToolSceneEvent *event);

    PRIVATECLASS(DrawFunctionTool)
};

#endif // DRAWFUCTIONTOOL_H
