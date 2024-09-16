// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "csizehandlerect.h"
#include "cdrawparamsigleton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"

#include "cdrawtoolmanagersigleton.h"
#include "cdrawscene.h"
#include "cselecttool.h"
#include "cgraphicsitem.h"
#include "application.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QDebug>
#include <QString>

CSizeHandleRect::CSizeHandleRect(QGraphicsItem *parent, EDirection d)
    : QGraphicsSvgItem(parent)
    , m_dir(d)
    , m_state(SelectionHandleOff)
    , m_bVisible(true)
    , m_lightRenderer(QString(":/theme/light/images/size/node.svg"))
    , m_darkRenderer(QString(":/theme/dark/images/size/node-dark.svg"))
    , m_isRotation(false)
{
    setParentItem(parent);
    setCacheMode(NoCache);
    setSharedRenderer(&m_lightRenderer);
    //hide();
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);


    setCursor(getCursor());
}

CSizeHandleRect::CSizeHandleRect(QGraphicsItem *parent, CSizeHandleRect::EDirection d, const QString &filename)
    : QGraphicsSvgItem(filename, parent)
    , m_dir(d)
    , m_state(SelectionHandleOff)
    , m_bVisible(true)
    , m_isRotation(true)
{
    setParentItem(parent);
    setCacheMode(NoCache);
    //hide();

    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsMovable, false);

    setCursor(getCursor());
}

PageView *CSizeHandleRect::curView() const
{
    PageView *parentView = nullptr;
    if (scene() != nullptr) {
        if (!scene()->views().isEmpty()) {
            parentView = dynamic_cast<PageView *>(scene()->views().first());
        }
    }
    return parentView;
}

void CSizeHandleRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    if (!CGraphicsItem::paintInteractBorderLine)
        return;

    //如果仅存在功能那么什么都不用绘制了
    if (m_onlyLogicAblity)
        return;

    if (isFatherDragging())
        return;

    if (!m_isRotation) {
        if (renderer() != &m_lightRenderer) {
            setSharedRenderer(&m_lightRenderer);
        }
    }

    painter->setClipping(false);
    QRectF rect = this->boundingRect();

    this->renderer()->render(painter, rect);
    painter->setClipping(true);

}

bool CSizeHandleRect::isFatherDragging()
{
    CGraphicsItem *pParentItem = dynamic_cast<CGraphicsItem *>(parentItem());
    if (pParentItem != nullptr && pParentItem->scene() != nullptr) {
        int model = pParentItem->drawScene()->page()->currentTool();
        int operatingTp = pParentItem->operatingType();
        if ((operatingTp == CSelectTool::EDragMove || operatingTp == CSelectTool::ECopyMove) && model == selection) {
            return true;
        }
    }
    return false;
}

void CSizeHandleRect::setState(ESelectionHandleState st)
{
    if (st != m_state) {
        switch (st) {
        case SelectionHandleOff:
            hide();
            break;
        case SelectionHandleInactive:
        case SelectionHandleActive:
            if (m_bVisible) {
                show();
            }
            break;
        }
        m_state = st;
    }
}

bool CSizeHandleRect::hitTest(const QPointF &point)
{
    bool bRet = false;
    if (m_bVisible) {
        QPointF pt = mapFromScene(point);
        bRet = this->boundingRect().contains(pt);
    }
    return bRet;
}

void CSizeHandleRect::move(qreal x, qreal y)
{
    setPos(x, y);
}

QRectF CSizeHandleRect::boundingRect() const
{
    if (curView() == nullptr)
        return QRectF();

    qreal scale = /*curView()->getDrawParam()->getScale()*/curView()->getScale();
    QRectF rect = QGraphicsSvgItem::boundingRect();
    rect.setWidth(rect.width() / scale);
    rect.setHeight(rect.height() / scale);
    return rect;
}

void CSizeHandleRect::setVisible(bool flag)
{
    m_bVisible = flag;
    if (parentItem() != nullptr /* && parentItem()->isSelected()*/) {
        m_bVisible ? show() : hide();
    }
}

void CSizeHandleRect::setJustExitLogicAbility(bool b)
{
    m_onlyLogicAblity = b;
    update();
}

QCursor CSizeHandleRect::getCursor()
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
        srcList << ":/theme/light/images/mouse_style/rotate_mouse.svg" << ":/theme/light/images/mouse_style/icon_drag_leftup.svg"
                << ":/theme/light/images/mouse_style/icon_drag_rightup.svg" << ":/theme/light/images/mouse_style/icon_drag_left.svg"
                << ":/theme/light/images/mouse_style/icon_drag_up.svg";


        QList<QPixmap> memberCursors;

        QSvgRenderer render;
        for (int i = 0; i < srcList.size(); ++i) {
            auto srcPath = srcList.at(i);
            if (render.load(srcPath)) {
                QPixmap pix(QSize(24, 24) * radio);
                //pix.setDevicePixelRatio(radio);
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
    CGraphicsItem *parent = dynamic_cast<CGraphicsItem *>(parentItem());
    qreal rotaAngle = (parent == nullptr ? 0 : parent->drawRotation());
    matrix.rotate(rotaAngle);

    switch (m_dir) {
    case Right:
    case Left:
        cursorResult = QCursor(m_LeftRightCursor.transformed(matrix, Qt::SmoothTransformation));
        break;
    case Top:
    case Bottom:
        cursorResult = QCursor(m_UpDownCursor.transformed(matrix, Qt::SmoothTransformation));
        break;
    case RightTop:
    case LeftBottom:
        cursorResult = QCursor(m_RightTopCursor.transformed(matrix, Qt::SmoothTransformation));
        break;
    case LeftTop:
    case RightBottom:
        cursorResult = QCursor(m_LeftTopCursor.transformed(matrix, Qt::SmoothTransformation));
        break;

    case Rotation: {
        cursorResult = m_RotateCursor;
        //matrix.rotate(this->rotation());
        QPixmap pixmap = cursorResult.pixmap().transformed(matrix, Qt::SmoothTransformation);
        cursorResult = QCursor(pixmap);
        break;
    }
    default:
        break;
    }

    return cursorResult;
}

void CSizeHandleRect::getTransBlockFlag(CSizeHandleRect::EDirection dir, bool &blockX, bool &blockY)
{
    blockX = false;
    blockY = false;
    switch (dir) {
    case Left:
    case Right: {
        blockY = true;
        break;
    }
    case Top:
    case Bottom: {
        blockX = true;
        break;
    }
    case LeftTop:
    case RightTop:
    case RightBottom:
    case LeftBottom:
    case Rotation:
    default:
        blockX = false;
        blockY = false;
        break;
    }
}

void CSizeHandleRect::getTransNegtiveFlag(CSizeHandleRect::EDirection dir, bool &negtiveX, bool &negtiveY)
{
    negtiveX = false;
    negtiveY = false;
    switch (dir) {
    case Left:
    case LeftBottom:
        negtiveX = true;
        break;
    case Top:
    case RightTop:
        negtiveY = true;
        break;
    case LeftTop:
        negtiveX = true;
        negtiveY = true;
        break;
    default:
        negtiveX = false;
        negtiveY = false;
        break;
    }
}

QPointF CSizeHandleRect::transCenter(CSizeHandleRect::EDirection dir, CGraphicsItem *pItem)
{
    return pItem->getCenter(dir);
}
