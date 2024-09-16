// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crecttool.h"
#include "cdrawscene.h"
#include "cgraphicsrectitem.h"
#include "frame/cgraphicsview.h"
#include "csidewidthwidget.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "application.h"
#include "cspinbox.h"
#include "seperatorline.h"

#include <DToolButton>
using namespace DrawAttribution;

#include <QtMath>

CRectTool::CRectTool()
    : IDrawTool(rectangle)
{
}

CRectTool::~CRectTool()
{

}

DrawAttribution::SAttrisList CRectTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBrushColor)
           << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth)
           << defaultAttriVar(DrawAttribution::ERectRadius);
    return result;
}

QCursor CRectTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/rectangle_mouse.svg");
    return QCursor(s_cur);
}

QAbstractButton *CRectTool::initToolButton()
{
    DToolButton *m_rectBtn = new DToolButton;
    m_rectBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_R)));
    setWgtAccesibleName(m_rectBtn, "Rectangle tool button");
    m_rectBtn->setToolTip(tr("Rectangle (R)"));
    m_rectBtn->setIconSize(QSize(48, 48));
    m_rectBtn->setFixedSize(QSize(37, 37));
    m_rectBtn->setCheckable(true);
    connect(m_rectBtn, &DToolButton::toggled, m_rectBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_rectangle tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_rectangle tool_active");
        m_rectBtn->setIcon(b ? activeIcon : icon);
    });
    m_rectBtn->setIcon(QIcon::fromTheme("ddc_rectangle tool_normal"));
    return m_rectBtn;
}

void CRectTool::registerAttributionWidgets()
{
    //1.注册画笔的颜色设置控件
    auto penColor = new CColorSettingButton(QObject::tr("Stroke"));
    setWgtAccesibleName(penColor, "stroken color button");
    penColor->setAttribution(EPenColor);
    penColor->setColorFill(CColorSettingButton::EFillBorder);
    drawBoard()->attributionWidget()->installComAttributeWgt(EPenColor, penColor, QColor(0, 0, 0));

    //2.注册画笔宽度设置控件
    auto penWidth = new CSideWidthWidget;
    penWidth->setMinimumWidth(90);

    // task:1708 画笔粗细属性设置项去掉“0px”
    setWgtAccesibleName(penWidth->menuComboBox(), "Pen width combox");
    penWidth->menuComboBox()->removeItem(0);

    QObject::connect(penWidth, &CSideWidthWidget::widthChanged, penWidth, [ = ](int width, bool preview = false) {
        Q_UNUSED(preview)
        drawBoard()->setDrawAttribution(EPenWidth, width);
    });
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, penWidth,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == penWidth) {
            QSignalBlocker bloker(penWidth);
            int width = var.isValid() ? var.toInt() : -1;
            penWidth->setWidth(width);
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EPenWidth, penWidth, 2);
    penWidth->installEventFilter(this);
    widthAttriWgt[0] = penWidth;

    //3.注册边线宽度设置控件
    auto borderWidth = new CSideWidthWidget;
    borderWidth->setMinimumWidth(90);
    setWgtAccesibleName(borderWidth->menuComboBox(), "Line width combox");
    QObject::connect(borderWidth, &CSideWidthWidget::widthChanged, borderWidth, [ = ](int width, bool preview = false) {
        Q_UNUSED(preview)
        drawBoard()->setDrawAttribution(EBorderWidth, width);
    });
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, borderWidth,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == borderWidth) {
            QSignalBlocker bloker(borderWidth);
            int width = var.isValid() ? var.toInt() : -1;
            borderWidth->setWidth(width);
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EBorderWidth, borderWidth, 2);
    borderWidth->installEventFilter(this);
    widthAttriWgt[1] = borderWidth;


    //4.注册填充色设置控件
    auto fillColor = new CColorSettingButton(tr("Fill"));
    fillColor->setAttribution(EBrushColor);
    setWgtAccesibleName(fillColor, "fill color button");
    drawBoard()->attributionWidget()->installComAttributeWgt(EBrushColor, fillColor, QColor(0, 0, 0, 0));

    //5.注册矩形圆角设置控件
    auto rectRadius = new CSpinBoxSettingWgt(tr("Corner Radius"));
    rectRadius->setAttribution(ERectRadius);
    rectRadius->spinBox()->setSpinRange(0, 1000);
    setWgtAccesibleName(rectRadius->spinBox(), "Rect Radio spinbox");
    drawBoard()->attributionWidget()->installComAttributeWgt(ERectRadius, rectRadius, 5);

    //6.注册分隔符
    auto sp = new SeperatorLine(nullptr);
    drawBoard()->attributionWidget()->installComAttributeWgt(ESeparator, sp);
}

void CRectTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsRectItem *pRectItem = dynamic_cast<CGraphicsRectItem *>(pInfo->businessItem);
        if (nullptr != pRectItem) {
            QPointF pointMouse = event->pos();
            bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
            bool altKeyPress = event->keyboardModifiers() & Qt::AltModifier;
            QRectF resultRect;

            if (shiftKeyPress && !altKeyPress) {
                QPointF resultPoint = pointMouse;
                qreal w = resultPoint.x() - pInfo->_startPos.x();
                qreal h = resultPoint.y() - pInfo->_startPos.y();
                qreal abslength = abs(w) - abs(h);
                if (abslength >= 0.1) {
                    if (h >= 0) {
                        resultPoint.setY(pInfo->_startPos.y() + abs(w));
                    } else {
                        resultPoint.setY(pInfo->_startPos.y() - abs(w));
                    }

                } else {
                    if (w >= 0) {
                        resultPoint.setX(pInfo->_startPos.x() + abs(h));
                    } else {
                        resultPoint.setX(pInfo->_startPos.x() - abs(h));
                    }
                }
                QRectF rectF(pInfo->_startPos, resultPoint);
                resultRect = rectF.normalized();

            }
            //按下ALT键
            else if (!shiftKeyPress && altKeyPress) {

                QPointF point1 = pointMouse;
                QPointF centerPoint = pInfo->_startPos;
                QPointF point2 = 2 * centerPoint - point1;
                QRectF rectF(point1, point2);
                resultRect = rectF.normalized();
            }
            //ALT SHIFT都按下
            else if (shiftKeyPress && altKeyPress) {
                QPointF resultPoint = pointMouse;
                qreal w = resultPoint.x() - pInfo->_startPos.x();
                qreal h = resultPoint.y() - pInfo->_startPos.y();
                qreal abslength = abs(w) - abs(h);
                if (abslength >= 0.1) {
                    if (h >= 0) {
                        resultPoint.setY(pInfo->_startPos.y() + abs(w));
                    } else {
                        resultPoint.setY(pInfo->_startPos.y() - abs(w));
                    }

                } else {
                    if (w >= 0) {
                        resultPoint.setX(pInfo->_startPos.x() + abs(h));
                    } else {
                        resultPoint.setX(pInfo->_startPos.x() - abs(h));
                    }
                }
                QPointF point1 = resultPoint;
                QPointF centerPoint = pInfo->_startPos;
                QPointF point2 = 2 * centerPoint - point1;
                QRectF rectF(point1, point2);
                resultRect = rectF.normalized();
            } else {
                QPointF resultPoint = pointMouse;
                QRectF rectF(pInfo->_startPos, resultPoint);
                resultRect = rectF.normalized();
            }

            pRectItem->setRect(resultRect);
            event->setAccepted(true);
        }
    }
}

void CRectTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsRectItem *pRectItem = dynamic_cast<CGraphicsRectItem *>(pInfo->businessItem);
        if (nullptr != pRectItem) {
            if (!pInfo->hasMoved()) {
                event->scene()->removeCItem(pRectItem, true);
                pInfo->businessItem = nullptr;
            } else {
                if (pRectItem->scene() == nullptr) {
                    pRectItem->drawScene()->addCItem(pRectItem);
                }
                pRectItem->setSelected(true);
            }
        }
    }

    IDrawTool::toolCreatItemFinish(event, pInfo);
}

CGraphicsItem *CRectTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsRectItem *m_pRectItem =  new CGraphicsRectItem(event->pos().x(), event->pos().y(), 0, 0);
        event->scene()->addCItem(m_pRectItem);
        return m_pRectItem;
    }
    return nullptr;
}

bool CRectTool::eventFilter(QObject *o, QEvent *e)
{
    if (o == widthAttriWgt[0] || o == widthAttriWgt[1]) {
        auto widthWgt = qobject_cast<CSideWidthWidget *>(o);
        if (widthWgt != nullptr) {
            if (e->type() == QEvent::ParentChange) {
                if (widthWgt->parentWidget() != nullptr) {
                    if (widthWgt->parentWidget()->isWindow()) {
                        widthWgt->setText(tr("Width"));
                        widthWgt->setSpace(15);
                    } else {
                        widthWgt->setText("");
                        widthWgt->setSpace(0);
                    }
                }
            }
        }
    }
    return IDrawTool::eventFilter(o, e);
}

