// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CELLIPSETOOL_H
#define CELLIPSETOOL_H
#include "drawitemtool.h"

class DRAWLIB_EXPORT EllipseTool : public DrawItemTool
{
    Q_OBJECT
public:
    EllipseTool(QObject *parent = nullptr);

    ~EllipseTool() override;

    int toolType() const override;

    SAttrisList attributions() override;
protected:
    /**
     * @brief toolFinish 创建图元
     * @param event 事件
     * @param scene 场景
     */
    PageItem *drawItemStart(ToolSceneEvent *eventpInfo) override;

    /**
     * @brief toolUpdate 工具执行的刷新
     * @param event 事件
     * @param scene 场景
     */
    void drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;

    /**
     * @brief toolFinish 工具执行的结束
     * @param event 事件
     * @param scene 场景
     */
    void drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;
};

#endif // CELLIPSETOOL_H
