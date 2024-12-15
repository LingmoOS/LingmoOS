// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPICTURETOOL_H
#define CPICTURETOOL_H

#include "drawtool.h"

class DRAWLIB_EXPORT ImageLoadTool: public DrawTool
{
    Q_OBJECT
public:
    explicit ImageLoadTool(QObject *parent = nullptr);
    ~ImageLoadTool();

    int toolType() const override;

    void pressOnScene(ToolSceneEvent *event) override;
    void moveOnScene(ToolSceneEvent *event) override;
    void releaseOnScene(ToolSceneEvent *event) override;

    SAttrisList attributions() override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    PRIVATECLASS(ImageLoadTool)
};


#endif // CPICTURETOOL_H
