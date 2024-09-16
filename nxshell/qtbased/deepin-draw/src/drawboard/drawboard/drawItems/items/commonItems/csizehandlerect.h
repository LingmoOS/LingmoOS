// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSIZEHANDLERECT
#define CSIZEHANDLERECT

#include "../../../utils/globaldefine.h"
#include "toolsceneevent.h"

#include <QList>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>

class PageItem;
class PageView;

class DRAWLIB_EXPORT HandleNode : public QGraphicsSvgItem
{
    Q_OBJECT
public:
    enum EInnerType {
        None,
        Resize_LT,
        Resize_T,
        Resize_RT,
        Resize_R,
        Resize_RB,
        Resize_B,
        Resize_LB,
        Resize_L,
        Rotation,
        UserCustom = 1000
    };

    HandleNode(int d, PageItem *parent = nullptr);
    HandleNode(int d, const QString &iconPath, PageItem *parent = nullptr);

    int nodeType() const;

    void setIgnoreScal(bool b);
    bool isIgnoreScal() const;

    PageView *pageView()const;
    PageItem *parentPageItem()const;

    void   setIconVisible(bool flag);
    bool   isIconVisible() const;

    static void    getTransBlockFlag(EInnerType dir, bool &blockX, bool &blockY);
    static void    getTransNegtiveFlag(EInnerType dir, bool &negtiveX, bool &negtiveY);
    static QPointF transCenter(EInnerType dir, PageItem *pItem);

    QRectF validRect() const;

    virtual void pressBegin(int tool, ToolSceneEvent *event);
    virtual void pressMove(int tool, ToolSceneEvent *event);
    virtual void pressRelease(int tool, ToolSceneEvent *event);

    void update();
    QCursor innerCursor(int tp) const;

    virtual QCursor cursor()const;

protected:
    bool contains(const QPointF &point) const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) override;

    virtual void parentItemChanged(int doChange, const QVariant &value);
    virtual void setNodePos();
    virtual void moveCenterTo(const QPointF &pos);
    virtual bool isVisbleCondition()const;


    PRIVATECLASS(HandleNode)

    friend class PageItem;
};

typedef QList<HandleNode *> Handles;




#endif // CSIZEHANDLERECT

