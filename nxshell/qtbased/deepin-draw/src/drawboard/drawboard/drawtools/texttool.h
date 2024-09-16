// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CTEXTTOOL_H
#define CTEXTTOOL_H
#include "drawitemtool.h"

class DRAWLIB_EXPORT TextTool : public DrawItemTool
{
    Q_OBJECT
public:
    TextTool(QObject *parent = nullptr);

    ~TextTool() override;

    int toolType() const override;

    SAttrisList attributions() override;
    void setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack) override;

protected:
    PageItem *drawItemStart(ToolSceneEvent *event) override;
    void      drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;
    bool isPressEventHandledByQt(ToolSceneEvent *event) override;

    int minMoveUpdateDistance() override;


    PRIVATECLASS(TextTool)
};

#endif // CTEXTTOOL_H
