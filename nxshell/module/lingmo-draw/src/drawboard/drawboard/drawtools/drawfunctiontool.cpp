// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawfunctiontool.h"
#include <QElapsedTimer>

class DrawFunctionTool::DrawFunctionTool_private
{
public:
    explicit DrawFunctionTool_private(DrawFunctionTool *q):
        _q(q)
    {

    }


    struct S_FUNCEVENTIDINFO {
        QElapsedTimer *getElapsedTimer()
        {
            if (elapsedTimer.isNull()) {
                elapsedTimer = QSharedPointer<QElapsedTimer>(new QElapsedTimer);
            }
            return elapsedTimer.get();
        }
        int elapsedFromBeginToDecide()
        {
            if (getElapsedTimer() != nullptr)
                return getElapsedTimer()->elapsed();
            return 0;
        }
        QSharedPointer<QElapsedTimer> elapsedTimer;
        int decided = 0;
        int elapsedTimeMs = 0;
        bool haveDecided = false;
    };
    QMap<int, S_FUNCEVENTIDINFO> idFuncEvents;

    DrawFunctionTool *_q;
};

DrawFunctionTool::DrawFunctionTool(QObject *parent):
    DrawTool(parent), DrawFunctionTool_d(new DrawFunctionTool_private(this))
{
    setMaxTouchPoint(1);
}

void DrawFunctionTool::pressOnScene(ToolSceneEvent *event)
{
    if (event->isFirstOne()) {
        setMaxTouchPoint(1);
    }
    DrawFunctionTool_private::S_FUNCEVENTIDINFO funcIdEvent;
    funcIdEvent.getElapsedTimer()->restart();
    d_DrawFunctionTool()->idFuncEvents.insert(event->uuid(), funcIdEvent);
    funcStart(event);
}

void DrawFunctionTool::moveOnScene(ToolSceneEvent *event)
{
    if (event->isPressed()) {
        DrawFunctionTool_private::S_FUNCEVENTIDINFO &funcIdEvent = d_DrawFunctionTool()->idFuncEvents[event->uuid()];
        funcIdEvent.elapsedTimeMs = funcIdEvent.elapsedFromBeginToDecide();
        if (!funcIdEvent.haveDecided) {
            funcIdEvent.decided = funcDecide(event);
            funcIdEvent.haveDecided = true;
        }
        funcUpdate(event, funcIdEvent.decided);
    } else {
        funHover(event);
    }
}

void DrawFunctionTool::releaseOnScene(ToolSceneEvent *event)
{
    auto find = d_DrawFunctionTool()->idFuncEvents.find(event->uuid());
    if (find != d_DrawFunctionTool()->idFuncEvents.end())
        funcFinished(event, find.value().decided);
}

void DrawFunctionTool::clearPointRecording()
{
    d_DrawFunctionTool()->idFuncEvents.clear();
    DrawTool::clearPointRecording();
}

int DrawFunctionTool::elapsedFromBeginToDecide(int eventId) const
{
    auto find = d_DrawFunctionTool()->idFuncEvents.find(eventId);
    if (find != d_DrawFunctionTool()->idFuncEvents.end()) {
        return find.value().elapsedTimeMs;
    }
    return 0;
}

int DrawFunctionTool::decideValue(int eventId) const
{
    auto find = d_DrawFunctionTool()->idFuncEvents.find(eventId);
    if (find != d_DrawFunctionTool()->idFuncEvents.end()) {
        return find.value().decided;
    }
    return 0;
}

int DrawFunctionTool::funcDecide(ToolSceneEvent *event)
{
    return 0;
}

void DrawFunctionTool::funHover(ToolSceneEvent *event)
{
    Q_UNUSED(event);
}


