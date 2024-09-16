// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMASICOTOOL_H
#define CMASICOTOOL_H
#include "idrawtool.h"

class JDynamicLayer;
class IBlurTool: public IDrawTool
{
    Q_OBJECT
public:
    explicit IBlurTool(QObject *parent = nullptr);
    DrawAttribution::SAttrisList attributions() override;
    QCursor cursor() const override;

protected:
    QAbstractButton *initToolButton() override;
    void  registerAttributionWidgets() override;

    void toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo) override;
    int  decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;
    void toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo) override;
    void toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;
    bool returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;
    bool isEnable(PageView *pView) override;
    /**
     * @brief minMoveUpdateDistance　创建图元时最小移动距离
     */
    int minMoveUpdateDistance() override;

    int  allowedMaxTouchPointCount() override;
    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;

    bool eventFilter(QObject *o, QEvent *e) override;

private:
    /**
     * @brief isBlurEnable 工具执行的结束
     * @param pItem 当前图元
     */
    bool isBlurEnable(const CGraphicsItem *pItem);

    /**
     * @brief getBlurEnableItems 获取到能进行模糊的图元
     * @param pItem 当前图元 如果pItem是一个基本业务图元,那么直接判断它是否可模糊,如果是一个组合那么判断其组合内是否有可模糊的图元
     */
    QList<CGraphicsItem *> getBlurEnableItems(const CGraphicsItem *pItem);

    /**
     * @brief saveZ 保存当前场景下图元的z值,必须与restoreZ成对出现
     */
    void saveZ(PageScene *scene);

    /**
     * @brief restoreZ 还原当前场景下图元的z值,必须与saveZ成对出现
     */
    void restoreZ();

    /**
     * @brief saveItemZValue 缓存图元的z值
     */
    void saveItemZValue(CGraphicsItem *pItem);


private:
    QMap<PageScene *, JDynamicLayer *> _layers;

    struct LayerSaveInfo {
        LayerSaveInfo(qreal zz = 0, bool b = false): z(zz), blocked(b) {}
        qreal z;
        bool  blocked;
    };
    QMap<JDynamicLayer *, LayerSaveInfo>        _saveZs;

    //该次可进行模糊的图元
    QList<CGraphicsItem *> _blurEnableItems;

    //改次进行了模糊操作的图元
    QSet<CGraphicsItem *> _bluringItemsSet;

    bool _pressedPosBlurEnable = false;

    QMap<CGraphicsItem *, qreal> _tempZs;

    CGraphicsItem *_pLastTopItem = nullptr;
};

#endif // CMASICOTOOL_H
