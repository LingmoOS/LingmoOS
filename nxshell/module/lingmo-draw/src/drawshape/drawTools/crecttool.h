// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CRECTTOOL_H
#define CRECTTOOL_H

#include "idrawtool.h"
class CRectTool : public IDrawTool
{
    Q_OBJECT
public:
    CRectTool();
    virtual ~CRectTool() override;
    DrawAttribution::SAttrisList attributions() override;

    QCursor cursor() const override;


protected:

    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;

    void  registerAttributionWidgets() override;

    /**
     * @brief toolUpdate 工具执行的刷新
     * @param event 事件
     * @param scene 场景
     */
    virtual void toolCreatItemUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolFinish 工具执行的结束
     * @param event 事件
     * @param scene 场景
     */
    virtual void toolCreatItemFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolFinish 创建图元
     * @param event 事件
     * @param scene 场景
     */
    virtual CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo) override;


    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QWidget *widthAttriWgt[2] = {nullptr, nullptr};
};

#endif // CRECTTOOL_H
