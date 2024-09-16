// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWITEMTOOL_H
#define DRAWITEMTOOL_H
#include "drawtool.h"
#include "pageitem.h"

class DRAWLIB_EXPORT DrawItemTool: public DrawTool
{
    Q_OBJECT
public:
    DrawItemTool(QObject *parent = nullptr);

    void pressOnScene(ToolSceneEvent *event) final;
    void moveOnScene(ToolSceneEvent *event) final;
    void releaseOnScene(ToolSceneEvent *event) final;

    bool isAutoPushToUndoStack() const;
    void setAutoPushToundoStack(bool b);

    bool isAutoCheckItemDeleted() const;
    void setAutoCheckItemDeleted(bool b);

    bool isDrawItemOnTop() const;
    void setDrawItemOnTop(bool b);

protected:
    virtual PageItem *drawItemStart(ToolSceneEvent *event) = 0;
    virtual void      drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) {};
    virtual void      drawItemFinish(ToolSceneEvent *event, PageItem *pItem);
    virtual void      drawItemHover(ToolSceneEvent *event);

    void clearPointRecording() override;
    //调整图标大小
    virtual void cursorScale(QPixmap &s_cur);


    PRIVATECLASS(DrawItemTool)
};


class DRAWLIB_EXPORT DrawComItemTool: public DrawItemTool
{
    Q_OBJECT
public:
    DrawComItemTool(QObject *parent = nullptr);

    int  toolType() const override;

    SAttrisList attributions() override;
    void setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack) override;

    void setCurrent(int itemType);
    int  current() const;

protected:
    PageItem *drawItemStart(ToolSceneEvent *event) override;
    void      drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;
    void      drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;

    int currentTp = RectType;
};

#endif // DRAWITEMTOOL_H
