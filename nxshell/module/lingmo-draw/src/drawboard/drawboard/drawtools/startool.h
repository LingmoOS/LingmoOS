// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPOLYGONALSTARTOOL_H
#define CPOLYGONALSTARTOOL_H
#include "drawitemtool.h"

class DRAWLIB_EXPORT StarTool : public DrawItemTool
{
    Q_OBJECT
public:
    StarTool(QObject *parent = nullptr);

    int toolType() const override;

    ~StarTool() override;

    enum {EStartLineSep = EUserAttri + 777};
    SAttrisList attributions() override;

    /**
     * @brief toolUpdate 工具执行的刷新
     * @param event 事件
     * @param scene 场景
     */
    void drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;

    /**
     * @brief toolFinish 创建图元
     * @param event 事件
     * @param scene 场景
     */
    PageItem *drawItemStart(ToolSceneEvent *event) override;
};

#endif // CPOLYGONALSTARTOOL_H
