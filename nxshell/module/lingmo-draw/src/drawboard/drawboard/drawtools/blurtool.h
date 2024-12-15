// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMASICOTOOL_H
#define CMASICOTOOL_H
#include <drawitemtool.h>

class PageScene;
class RasterItem;
class DRAWLIB_EXPORT BlurTool: public DrawItemTool
{
    Q_OBJECT
public:
    explicit BlurTool(QObject *parent = nullptr);

    int toolType() const override;

    SAttrisList attributions() override;

    int  blurType() const;
    void setBlurType(int tp);

    void setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack) override;

protected:
    PageItem *drawItemStart(ToolSceneEvent *event) override;
    void      drawItemUpdate(ToolSceneEvent *event, PageItem *pItem) override;
    void      drawItemFinish(ToolSceneEvent *event, PageItem *pItem) override;
    void      drawItemHover(ToolSceneEvent *event) override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) override;
    void onCurrentPageChanged(Page *newPage) override;
    int  minMoveUpdateDistance() override;

    void enterSceneEvent(ToolSceneEvent *event) override;
    void leaveSceneEvent(ToolSceneEvent *event) override;

    void drawMore(QPainter *painter, const QRectF &rect, PageScene *scene) override;

    void clearPointRecording() override;

    RasterItem *getRasterItem(ToolSceneEvent *event);
    QPainterPath &getActivedOrgPath(ToolSceneEvent *event, RasterItem *rstItem);
    QPainterPath &getActivedStrokerPath(ToolSceneEvent *event, RasterItem *rstItem);

    void creatBlurSrokerPaths(ToolSceneEvent *event, RasterItem *rstItem);

    Q_SLOT void _onSceneSelectionChanged(const QList<PageItem * > &selectedItems);

    PRIVATECLASS(BlurTool)
};

#endif // CMASICOTOOL_H
