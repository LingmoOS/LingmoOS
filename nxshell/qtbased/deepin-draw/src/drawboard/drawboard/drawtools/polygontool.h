// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPOLYGONTOOL_H
#define CPOLYGONTOOL_H

#include "drawitemtool.h"

class DRAWLIB_EXPORT PolygonTool : public DrawItemTool
{
    Q_OBJECT
public:
    PolygonTool(QObject *parent = nullptr);
    virtual ~PolygonTool() override;

    int toolType() const override;

    enum {EPolygonLineSep = EUserAttri + 776};
    SAttrisList attributions() override;

protected:

    PageItem* drawItemStart(ToolSceneEvent *event) override;
    void      drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;
};

#endif // CPOLYGONTOOL_H
