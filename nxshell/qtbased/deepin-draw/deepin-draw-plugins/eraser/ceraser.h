// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CERASER_H
#define CERASER_H
#include "idrawtool.h"
#include "cgraphicslayer.h"

#ifdef __cplusplus
extern "C" {

#endif
__attribute__((visibility("default"))) IDrawTool *creatTool();

#ifdef __cplusplus
}
#endif

class CEraser: public IDrawTool
{
    Q_OBJECT
public:
    CEraser();

    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;

    CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo) override {return nullptr;}

    int  decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    void toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    int  allowedMaxTouchPointCount() override {return 1;}

    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    JDynamicLayer *currentLayer(CDrawScene *scene);

protected:
    QMap<int, QPainterPath>      _resultImagePaths;

    QMap<int, JActivedPaintInfo> _activePictures;
};


#endif // CCALLIGRAPHYPEN_H
