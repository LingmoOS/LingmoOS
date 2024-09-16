// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "csizehandlerect.h"
#include "pagecontext.h"
#include "pageview.h"


#include "pagescene.h"
#include "selecttool.h"
#include "pageitem.h"
#include "cgraphicsitemevent.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QDebug>
#include <QString>
#include <QApplication>

const int ROTATIONICONDSTANCE = 8;
class HandleNode::HandleNode_private
{
public:
    explicit HandleNode_private(HandleNode *qq, const int d): q(qq), m_dir(d)
    {

    }
    bool isFatherDragging()
    {
        PageItem *pParentItem = q->parentPageItem();
        if (pParentItem != nullptr && pParentItem->scene() != nullptr) {
            int operatingTp = pParentItem->operatingType();
            if (operatingTp == PageItemEvent::EMove) {
                return true;
            }
        }
        return false;
    }

    void initInnerCursor()
    {
        q->setCursor(getCursor(m_dir));
    }
    QCursor getCursor(int innerTp)
    {
        static bool init = false;
        static QPixmap m_RotateCursor;
        static QPixmap m_LeftTopCursor;
        static QPixmap m_RightTopCursor;
        static QPixmap m_LeftRightCursor;
        static QPixmap m_UpDownCursor;
        if (!init) {
            qreal radio = qApp->devicePixelRatio();

            QStringList srcList;
            srcList << ":/theme/light/images/mouse_style/rotate_mouse.svg"
                    << ":/theme/light/images/mouse_style/icon_drag_leftup.svg"
                    << ":/theme/light/images/mouse_style/icon_drag_rightup.svg"
                    << ":/theme/light/images/mouse_style/icon_drag_left.svg"
                    << ":/theme/light/images/mouse_style/icon_drag_up.svg";


            QList<QPixmap> memberCursors;

            QSvgRenderer render;
            for (int i = 0; i < srcList.size(); ++i) {
                auto srcPath = srcList.at(i);
                if (render.load(srcPath)) {
                    QPixmap pix(QSize(24, 24) * radio);
                    pix.fill(QColor(0, 0, 0, 0));
                    QPainter painter(&pix);
                    render.render(&painter, QRect(QPoint(0, 0), pix.size()));
                    memberCursors << pix;
                }
            }

            // 判断数据边界
            if (memberCursors.size() >= 5) {
                m_RotateCursor  = memberCursors.at(0);
                m_LeftTopCursor = memberCursors.at(1);
                m_RightTopCursor = memberCursors.at(2);
                m_LeftRightCursor = memberCursors.at(3);
                m_UpDownCursor = memberCursors.at(4);
            }

            init = true;
        }

        QCursor cursorResult(Qt::ArrowCursor);
        QMatrix matrix;
        PageItem *parent = q->parentPageItem();
        qreal rotaAngle = (parent == nullptr ? 0 : parent->drawRotation());
        matrix.rotate(rotaAngle);

        switch (innerTp) {
        case Resize_R:
        case Resize_L:
            cursorResult = QCursor(m_LeftRightCursor.transformed(matrix, Qt::SmoothTransformation));
            break;
        case Resize_T:
        case Resize_B:
            cursorResult = QCursor(m_UpDownCursor.transformed(matrix, Qt::SmoothTransformation));
            break;
        case Resize_RT:
        case Resize_LB:
            cursorResult = QCursor(m_RightTopCursor.transformed(matrix, Qt::SmoothTransformation));
            break;
        case Resize_LT:
        case Resize_RB:
            cursorResult = QCursor(m_LeftTopCursor.transformed(matrix, Qt::SmoothTransformation));
            break;

        case Rotation: {
            cursorResult = m_RotateCursor;
            QPixmap pixmap = cursorResult.pixmap().transformed(matrix, Qt::SmoothTransformation);
            cursorResult = QCursor(pixmap);
            break;
        }
        default:
            break;
        }

        return cursorResult;
    }

    static QString innerIcon(int d)
    {
        if (d == Rotation) {
            return ":/theme/light/images/mouse_style/icon_rotate.svg";
        }
        return ":/theme/light/images/size/node.svg";
    }

    QRectF updateRect()
    {
        return q->validRect().adjusted(-1, -1, 1, 1);
    }
    void deliverResizeBegin(EInnerType tp, ToolSceneEvent *event)
    {
        if (tp >= Resize_LT && tp <= Resize_L) {
            PageItemScalEvent scal(PageItemEvent::EScal);
            scal.setEventPhase(EChangedBegin);
            scal.setPressedDirection(tp);
            scal._scenePos = event->pos();
            scal._oldScenePos = event->lastEvent()->pos();
            scal._sceneBeginPos = event->firstEvent()->pos();

            bool xBlock = false;
            bool yBlock = false;
            HandleNode::EInnerType dir = HandleNode::EInnerType(tp);
            HandleNode::getTransBlockFlag(dir, xBlock, yBlock);
            scal.setXTransBlocked(xBlock);
            scal.setYTransBlocked(yBlock);

            bool xNegitiveOffset = false;
            bool yNegitiveOffset = false;
            HandleNode::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
            scal.setXNegtiveOffset(xNegitiveOffset);
            scal.setYNegtiveOffset(yNegitiveOffset);
            scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

            //分发事件
            if (PageScene::isPageItem(q->parentPageItem())) {
                PageItem *pBzItem = q->parentPageItem();

                scal.setPos(pBzItem->mapFromScene(event->pos()));
                scal.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
                scal.setOrgSize(pBzItem->orgRect().size());
                scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->orgRect().center() :
                                  HandleNode::transCenter(dir, pBzItem));
                scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());
                pBzItem->operatingBegin(&scal);
            }
            event->view()->viewport()->update();
        }
    }

    void deliverResize(EInnerType tp, ToolSceneEvent *event)
    {
        if (tp >= Resize_LT && tp <= Resize_L) {
            PageItemScalEvent scal(PageItemEvent::EScal);
            scal.setEventPhase(EChangedUpdate);
            //scal.setToolEventType();
            scal.setPressedDirection(tp);
            scal._scenePos = event->pos();
            scal._oldScenePos = event->lastEvent()->pos();
            scal._sceneBeginPos = event->firstEvent()->pos();

            bool xBlock = false;
            bool yBlock = false;
            HandleNode::EInnerType dir = HandleNode::EInnerType(tp);
            HandleNode::getTransBlockFlag(dir, xBlock, yBlock);
            scal.setXTransBlocked(xBlock);
            scal.setYTransBlocked(yBlock);

            bool xNegitiveOffset = false;
            bool yNegitiveOffset = false;
            HandleNode::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
            scal.setXNegtiveOffset(xNegitiveOffset);
            scal.setYNegtiveOffset(yNegitiveOffset);
            scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

            //分发事件
            if (PageScene::isPageItem(q->parentPageItem())) {
                PageItem *pBzItem = q->parentPageItem();

                scal.setPos(pBzItem->mapFromScene(event->pos()));
                scal.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
                scal.setOrgSize(pBzItem->orgRect().size());
                scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->orgRect().center() :
                                  HandleNode::transCenter(dir, pBzItem));
                scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());

                pBzItem->operating(&scal);
            }
            event->view()->viewport()->update();
        }
    }

    void deliverResizeEnd(EInnerType tp, ToolSceneEvent *event)
    {
        if (tp >= Resize_LT && tp <= Resize_L) {
            PageItemScalEvent scal(PageItemEvent::EScal);
            scal.setEventPhase(EChangedUpdate);
            //scal.setToolEventType();
            scal.setPressedDirection(tp);
            scal._scenePos = event->pos();
            scal._oldScenePos = event->lastEvent()->pos();
            scal._sceneBeginPos = event->firstEvent()->pos();

            bool xBlock = false;
            bool yBlock = false;
            HandleNode::EInnerType dir = HandleNode::EInnerType(tp);
            HandleNode::getTransBlockFlag(dir, xBlock, yBlock);
            scal.setXTransBlocked(xBlock);
            scal.setYTransBlocked(yBlock);

            bool xNegitiveOffset = false;
            bool yNegitiveOffset = false;
            HandleNode::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
            scal.setXNegtiveOffset(xNegitiveOffset);
            scal.setYNegtiveOffset(yNegitiveOffset);
            scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

            //分发事件
            if (PageScene::isPageItem(q->parentPageItem())) {
                PageItem *pBzItem = q->parentPageItem();

                scal.setPos(pBzItem->mapFromScene(event->pos()));
                scal.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
                scal.setOrgSize(pBzItem->orgRect().size());
                scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->orgRect().center() :
                                  HandleNode::transCenter(dir, pBzItem));
                scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());
                pBzItem->operatingEnd(&scal);
            }
            event->view()->viewport()->update();
        }
    }

    void deliverRotaBegin(ToolSceneEvent *event)
    {
        PageItemRotEvent rot(PageItemEvent::ERot);
        rot.setEventPhase(EChangedBegin);
        //rot.setToolEventType(decideValue(event->uuid()));
        rot.setPressedDirection(q->nodeType());
        rot._scenePos = event->pos();
        rot._oldScenePos = event->lastEvent()->pos();
        rot._sceneBeginPos = event->firstEvent()->pos();

        //分发事件
        if (PageScene::isPageItem(q->parentPageItem())) {
            PageItem *pBzItem = q->parentPageItem();
            rot.setPos(pBzItem->mapFromScene(event->pos()));
            rot.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
            rot.setOrgSize(pBzItem->orgRect().size());
            rot.setCenterPos(pBzItem->orgRect().center());
            rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());
            pBzItem->operatingBegin(&rot);
        }
        event->view()->viewport()->update();
    }

    void deliverRota(ToolSceneEvent *event)
    {
        PageItemRotEvent rot(PageItemEvent::ERot);
        rot.setEventPhase(EChangedUpdate);
        //rot.setToolEventType(decideValue(event->uuid()));
        rot.setPressedDirection(q->nodeType());
        rot._scenePos = event->pos();
        rot._oldScenePos = event->lastEvent()->pos();
        rot._sceneBeginPos = event->firstEvent()->pos();

        //分发事件
        if (PageScene::isPageItem(q->parentPageItem())) {
            PageItem *pBzItem = q->parentPageItem();
            rot.setPos(pBzItem->mapFromScene(event->pos()));
            rot.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
            rot.setOrgSize(pBzItem->orgRect().size());
            rot.setCenterPos(pBzItem->orgRect().center());
            rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());

            pBzItem->operating(&rot);
        }
        event->view()->viewport()->update();
    }

    void deliverRotaEnd(ToolSceneEvent *event)
    {
        PageItemRotEvent rot(PageItemEvent::ERot);
        rot.setEventPhase(EChangedFinished);
        //rot.setToolEventType(decideValue(event->uuid()));
        rot.setPressedDirection(q->nodeType());
        rot._scenePos = event->pos();
        rot._oldScenePos = event->lastEvent()->pos();
        rot._sceneBeginPos = event->firstEvent()->pos();

        //分发事件
        if (PageScene::isPageItem(q->parentPageItem()) /*|| parentPageItem() == event->scene()->selectGroup()*/) {
            PageItem *pBzItem = q->parentPageItem();
            rot.setPos(pBzItem->mapFromScene(event->pos()));
            rot.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
            rot.setOrgSize(pBzItem->orgRect().size());
            rot.setCenterPos(pBzItem->orgRect().center());
            rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());
            pBzItem->operatingEnd(&rot);
        }
        event->view()->viewport()->update();
    }


    HandleNode *q;
    const int m_dir;
    bool ignoreScale = true;

//    , m_lightRenderer(QString(":/theme/light/images/size/node.svg"))
//    , m_darkRenderer(QString(":/theme/dark/images/size/node-dark.svg"))
    bool showIcon = true;
    QSvgRenderer m_lightRenderer;
    QSvgRenderer m_darkRenderer;
    bool m_isRotation = true;

    bool lastIsVisble = true;

    QGraphicsItem *currentParent = nullptr;

    qreal lastScale = 1.0;
};
HandleNode::HandleNode(int d, PageItem *parent)
    : HandleNode(d, HandleNode_private::innerIcon(d), parent)
{

}

HandleNode::HandleNode(int d, const QString &iconPath, PageItem *parent)
    : QGraphicsSvgItem(iconPath, parent), HandleNode_d(new HandleNode_private(this, d))
{
    setParentItem(parent);
    setCacheMode(NoCache);

    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);
    setFlag(ItemSendsScenePositionChanges, true);
    //setFlag(ItemIgnoresTransformations, true);

    d_HandleNode()->initInnerCursor();
}

int HandleNode::nodeType() const
{
    return d_HandleNode()->m_dir;
}

void HandleNode::setIgnoreScal(bool b)
{
    d_HandleNode()->ignoreScale = b;
}

bool HandleNode::isIgnoreScal() const
{
    return d_HandleNode()->ignoreScale;
}

PageView *HandleNode::pageView() const
{
    PageView *parentView = nullptr;
    if (scene() != nullptr) {
        if (!scene()->views().isEmpty()) {
            parentView = dynamic_cast<PageView *>(scene()->views().first());
        }
    }
    return parentView;
}

PageItem *HandleNode::parentPageItem() const
{
    if (parentItem() == nullptr) {
        return nullptr;
    }
    return dynamic_cast<PageItem *>(parentItem());
}

void HandleNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!PageItem::isPaintSelectingBorder())
        return;

    if (parentPageItem() == nullptr)
        return;

    if (!parentPageItem()->itemRect().isValid())
        return;

    //如果仅存在功能那么什么都不用绘制了
    if (!d_HandleNode()->showIcon)
        return;

    if (d_HandleNode()->isFatherDragging())
        return;

    qreal v = option->levelOfDetailFromTransform(painter->worldTransform());
    if (v != d_HandleNode()->lastScale) {
        d_HandleNode()->lastScale = v;
        setNodePos();
    }
    //需要显示才设置着色器
    if (isVisible()) {
        painter->setClipping(false);
        QRectF rect = this->validRect();

        this->renderer()->render(painter, rect);
        painter->setClipping(true);
    }
}

QVariant HandleNode::itemChange(GraphicsItemChange doChange, const QVariant &value)
{
    if (doChange == ItemScenePositionHasChanged || doChange == ItemPositionChange) {
        this->update();
    }
    return QGraphicsSvgItem::itemChange(doChange, value);
}

void HandleNode::parentItemChanged(int doChange, const QVariant &value)
{
    if (parentPageItem() != nullptr) {
        setVisible(isVisbleCondition());
        setNodePos();
        this->update();
    } else {
        setVisible(false);
    }
}

void HandleNode::setNodePos()
{
    auto rect = parentPageItem()->orgRect();

    QPointF pos;
    bool isValid = true;
    QSizeF size = boundingRect().size();
    qreal offset = size.height() / 2;
    switch (nodeType()) {
    case Resize_LT: {
        pos = QPointF(rect.topLeft().x(), rect.topLeft().y() - offset);
        break;
    }
    case Resize_T: {
        pos = QPointF(rect.center().x(), rect.top() - offset);
        break;
    }
    case Resize_RT: {
        pos = QPointF(rect.topRight().x(), rect.topRight().y() - offset);
        break;
    }
    case Resize_R: {
        pos = QPointF(rect.right(), rect.center().y() - offset);
        break;
    }
    case Resize_RB: {
        pos = QPointF(rect.bottomRight().x(), rect.bottomRight().y() - offset);
        break;
    }
    case Resize_B: {
        pos = QPointF(rect.center().x(), rect.bottom() - offset);
        break;
    }
    case Resize_LB: {
        pos = QPointF(rect.bottomLeft().x(), rect.bottomLeft().y() - offset);
        break;
    }
    case Resize_L: {
        pos = QPointF(rect.left(), rect.center().y() - offset);
        break;
    }
    case Rotation: {
        pos = QPointF(rect.center().x(), rect.top() - validRect().height() - validRect().height() / 2 + ROTATIONICONDSTANCE);
        break;
    }
    default:
        isValid = false;
        break;
    }
    if (isValid)
        moveCenterTo(pos);
}

bool HandleNode::isVisbleCondition() const
{
    return (parentPageItem() != nullptr && parentPageItem()->isSingleSelected() && parentPageItem()->pageGroup() == nullptr);
}

QCursor HandleNode::innerCursor(int tp) const
{
    return d_HandleNode()->getCursor(tp);
}

QCursor HandleNode::cursor() const
{
    return d_HandleNode()->getCursor(nodeType());
}

void HandleNode::setIconVisible(bool flag)
{
    d_HandleNode()->showIcon = flag;
    update();
}

bool HandleNode::isIconVisible() const
{
    return d_HandleNode()->showIcon;
}


void HandleNode::getTransBlockFlag(HandleNode::EInnerType dir, bool &blockX, bool &blockY)
{
    blockX = false;
    blockY = false;
    switch (dir) {
    case Resize_L:
    case Resize_R: {
        blockY = true;
        break;
    }
    case Resize_T:
    case Resize_B: {
        blockX = true;
        break;
    }
    case Resize_LT:
    case Resize_RT:
    case Resize_RB:
    case Resize_LB:
    case Rotation:
    default:
        blockX = false;
        blockY = false;
        break;
    }
}

void HandleNode::getTransNegtiveFlag(HandleNode::EInnerType dir, bool &negtiveX, bool &negtiveY)
{
    negtiveX = false;
    negtiveY = false;
    switch (dir) {
    case Resize_L:
    case Resize_LB:
        negtiveX = true;
        break;
    case Resize_T:
    case Resize_RT:
        negtiveY = true;
        break;
    case Resize_LT:
        negtiveX = true;
        negtiveY = true;
        break;
    default:
        negtiveX = false;
        negtiveY = false;
        break;
    }
}

QPointF HandleNode::transCenter(HandleNode::EInnerType dir, PageItem *pItem)
{
    QPointF center;
    auto rect = pItem->orgRect();
    switch (dir) {
    case HandleNode::Resize_LT: {
        center = rect.bottomRight();
        break;
    }
    case HandleNode::Resize_T:
        center = QPointF(rect.center().x(), rect.bottom());
        break;
    case HandleNode::Resize_RT:
        center = rect.bottomLeft();
        break;
    case HandleNode::Resize_R:
        center = QPointF(rect.left(), rect.center().y());
        break;
    case HandleNode::Resize_RB:
        center = rect.topLeft();
        break;
    case HandleNode::Resize_B:
        center = QPointF(rect.center().x(), rect.top());
        break;
    case HandleNode::Resize_LB:
        center = rect.topRight();
        break;
    case HandleNode::Resize_L:
        center = QPointF(rect.right(), rect.center().y());
        break;
    case HandleNode::Rotation:
        center = rect.center();
        break;
    default:
        center = rect.center();
        break;
    }
    return center;
}

QRectF HandleNode::validRect() const
{
    if (!d_HandleNode()->ignoreScale) {
        return boundingRect();
    }
    if (pageView() == nullptr) {
        return boundingRect();
    }

    qreal scale = pageView()->getScale();

    QRectF rect = boundingRect();

    auto center = rect.center();
    QTransform trans = QTransform::fromTranslate(center.x(), center.y());
    trans.scale(1.0 / scale, 1.0 / scale);
    trans.translate(-center.x(), -center.y());

    return trans.mapRect(rect);
}

void HandleNode::moveCenterTo(const QPointF &pos)
{
    auto size = boundingRect().size();

    auto posR = pos + QPointF(-size.width() / 2, 0);

    setPos(posR);
}

void HandleNode::pressBegin(int tool, ToolSceneEvent *event)
{
    if (nodeType() >= Resize_LT && nodeType() <= Resize_L) {
        d_HandleNode()->deliverResizeBegin(EInnerType(nodeType()), event);
    } else if (nodeType() == Rotation) {
        d_HandleNode()->deliverRotaBegin(event);
    }
}

void HandleNode::pressMove(int tool, ToolSceneEvent *event)
{
    if (nodeType() >= Resize_LT && nodeType() <= Resize_L) {
        d_HandleNode()->deliverResize(EInnerType(nodeType()), event);
    } else if (nodeType() == Rotation) {
        d_HandleNode()->deliverRota(event);
        event->scene()->setCursor(cursor());
    }
}

void HandleNode::pressRelease(int tool, ToolSceneEvent *event)
{
    if (nodeType() >= Resize_LT && nodeType() <= Resize_L) {
        d_HandleNode()->deliverResizeEnd(EInnerType(nodeType()), event);
    } else if (nodeType() == Rotation) {
        d_HandleNode()->deliverRota(event);
    }

//    auto nodes = parentPageItem()->handleNodes();
//    foreach (auto node, nodes) {
//        node->setCursor(node->d_HandleNode()->getCursor(node->nodeType()));
//    }
}

void HandleNode::update()
{
    if (scene() != nullptr)
        scene()->update(this->mapRectToScene(d_HandleNode()->updateRect()));
    this->QGraphicsSvgItem::update();
}

bool HandleNode::contains(const QPointF &point) const
{
    return validRect().contains(point);
}
