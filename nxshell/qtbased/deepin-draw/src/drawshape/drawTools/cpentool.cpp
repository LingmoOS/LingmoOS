// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpentool.h"
#include "cdrawscene.h"
#include "cgraphicspenitem.h"
#include "frame/cgraphicsview.h"
#include "application.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "seperatorline.h"
#include "application.h"
#include "cpictureitem.h"
#include "cviewmanagement.h"
#include "cgraphicslayer.h"
#include "cgraphicsitemevent.h"
#include "cundoredocommand.h"
#include "ccolorpickwidget.h"
#include "pickcolorwidget.h"
#include "cerasertool.h"

#include <QAbstractItemView>
#include <QUndoStack>
#include <QtMath>
#include <QPicture>
#include <DToolButton>

CPenTool::CPenTool()
    : IDrawTool(pen)
    , m_pRenderImage(QImage(":/icons/deepin/builtin/texts/crayon.png"))
{
    //m_pRenderImage = QImage(":/icons/deepin/builtin/texts/crayon.png");
    setTouchSensitiveRadius(0);
}

CPenTool::~CPenTool()
{

}

DrawAttribution::SAttrisList CPenTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EPenStyle)
           << defaultAttriVar(DrawAttribution::EPenWidth);
    return result;
}

QCursor CPenTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/brush_mouse.svg");
    return QCursor(s_cur, 9, 26);
}

QAbstractButton *CPenTool::initToolButton()
{
    DToolButton *m_penBtn = new DToolButton;
    m_penBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_P)));
    setWgtAccesibleName(m_penBtn, "Pencil tool button");
    m_penBtn->setToolTip(tr("Pencil (P)"));
    m_penBtn->setIconSize(QSize(48, 48));
    m_penBtn->setFixedSize(QSize(37, 37));
    m_penBtn->setCheckable(true);

    connect(m_penBtn, &DToolButton::toggled, m_penBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_brush tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_brush tool_active");
        m_penBtn->setIcon(b ? activeIcon : icon);
    });
    m_penBtn->setIcon(QIcon::fromTheme("ddc_brush tool_normal"));
    return m_penBtn;
}

void CPenTool::registerAttributionWidgets()
{
    //8.线条开端样式设置控件
    auto streakBeginStyle = new CComBoxSettingWgt(tr("Start"));
    auto pStreakStartComboBox = new QComboBox;
    setWgtAccesibleName(pStreakStartComboBox, "Line start style combox");
    pStreakStartComboBox->setFixedSize(QSize(90, 36));
    pStreakStartComboBox->setIconSize(QSize(34, 20));
    pStreakStartComboBox->setFocusPolicy(Qt::NoFocus);

    pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
    pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_circle"), "");
    pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_circle"), "");
    pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_arrow"), "");
    pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_arrow"), "");

    streakBeginStyle->setComboBox(pStreakStartComboBox);
    connect(pStreakStartComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), streakBeginStyle, [ = ](int index) {
        drawBoard()->setDrawAttribution(EStreakBeginStyle, index);
    });
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, streakBeginStyle, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == streakBeginStyle) {
            QSignalBlocker bloker(pStreakStartComboBox);
            pStreakStartComboBox->setCurrentIndex(var.toInt());
        }
    });

    streakBeginStyle->setAttribution(EStreakBeginStyle);
    drawBoard()->attributionWidget()->installComAttributeWgt(EStreakBeginStyle, streakBeginStyle, 0);

    //9.线条末端样式设置控件
    auto streakEndStyle = new CComBoxSettingWgt(tr("End"));
    streakEndStyle->setAttribution(EStreakEndStyle);
    auto pStreakEndComboBox = new QComboBox;
    setWgtAccesibleName(pStreakEndComboBox, "Line end style combox");
    pStreakEndComboBox->setFixedSize(QSize(90, 36));
    pStreakEndComboBox->setIconSize(QSize(34, 20));
    pStreakEndComboBox->setFocusPolicy(Qt::NoFocus);

    pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
    pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_circle"), "");
    pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_circle"), "");
    pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_arrow"), "");
    pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_arrow"), "");

    streakEndStyle->setComboBox(pStreakEndComboBox);
    connect(pStreakEndComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), streakEndStyle, [ = ](int index) {
        drawBoard()->setDrawAttribution(EStreakEndStyle, index);
    });
    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, streakEndStyle, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == streakEndStyle) {
            QSignalBlocker bloker(pStreakEndComboBox);
            pStreakEndComboBox->setCurrentIndex(var.toInt());
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EStreakEndStyle, streakEndStyle, 0);

    //注册分隔符
    auto spl = new SeperatorLine();
    spl->setProperty(WidgetShowInVerWindow, false);
    drawBoard()->attributionWidget()->installComAttributeWgt(1775, spl);

    //10.画笔样式设置控件
    auto penStyleWgt = new CComBoxSettingWgt();
    penStyleWgt->setAttribution(EPenStyle);
    m_pPenStyleComboBox = new QComboBox;
    drawApp->setWidgetAccesibleName(m_pPenStyleComboBox, "Pen style combobox");

    m_pPenStyleComboBox->view()->installEventFilter(this);

    m_pPenStyleComboBox->setFixedSize(QSize(230, 36));
    m_pPenStyleComboBox->setIconSize(QSize(24, 20));
    m_pPenStyleComboBox->setFocusPolicy(Qt::NoFocus);

    m_pPenStyleComboBox->addItem(QIcon::fromTheme("icon_marker"), tr("Watercolor"));
    m_pPenStyleComboBox->addItem(QIcon::fromTheme("icon_calligraphy"), tr("Calligraphy pen"));
    m_pPenStyleComboBox->addItem(QIcon::fromTheme("icon_crayon"), tr("Crayon"));

    penStyleWgt->setComboBox(m_pPenStyleComboBox);

    connect(m_pPenStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), penStyleWgt, [ = ](int index) {
        emit drawApp->attributionsWgt()->attributionChanged(EPenStyle, index + 1);
    });
    connect(drawApp->attributionsWgt(), &CAttributeManagerWgt::updateWgt, penStyleWgt, [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == penStyleWgt) {
            QSignalBlocker bloker(m_pPenStyleComboBox);
            m_pPenStyleComboBox->setCurrentIndex(var.toInt() - 1);
        }
    });
    drawBoard()->attributionWidget()->installComAttributeWgt(EPenStyle, penStyleWgt, 1);
    penStyleWgt->installEventFilter(this);
}

QPixmap CPenTool::pictureColorChanged(const QImage &image, const QColor &color)
{
    QImage _image = image.scaled(24, 24).convertToFormat(QImage::Format_ARGB32);
    for (int i = 0; i < _image.width(); ++i) {
        for (int j = 0; j < _image.height(); ++j) {
            int alpha = _image.pixelColor(i, j).alpha();
            if (alpha != 0) {
                auto c = color;
                c.setAlpha(alpha);
                _image.setPixelColor(i, j, c);
            }
        }
    }
    QPixmap pixmap;
    pixmap.convertFromImage(_image);
    return pixmap;
}

CGraphicsItem *CPenTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    Q_UNUSED(event)

    return nullptr;
}

void CPenTool::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    toolDoUpdate(event);
    if (pInfo->_opeTpUpdate == ECrayonPen)
        paintCrayonPen(event, pInfo, 0);
}

int CPenTool::decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    int ret = drawBoard()->currentPage()->defaultAttriVar(EPenStyle).toInt();

    event->view()->setCacheEnable(true);

    if (_layers.find(event->scene()) == _layers.end()) {
        auto scene = event->scene();
        JDynamicLayer *layer = nullptr;
        if (scene->selectGroup()->items().count() == 1) {
            auto pSelected = dynamic_cast<JDynamicLayer *>(scene->selectGroup()->items().first());
            if (pSelected != nullptr && !pSelected->isBlocked()) {
                layer = pSelected;
            }
        }

        if (layer == nullptr) {
            scene->clearSelectGroup();
            layer = new  JDynamicLayer;
            scene->addCItem(layer);
            scene->selectItem(layer, true, false, true);
        }
        _layers.insert(scene, layer);
    }

    if (ret != 0) {
        _activePictures[event->uuid()].beginSubPicture();
        _activePictures[event->uuid()].setPenForPath(getViewDefualtPen(event->view()));
        _activePictures[event->uuid()].setBrushForPath(getViewDefualtBrush(event->view()));
    }
    return ret;
}

void CPenTool::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    QPicture picture;
    switch (pInfo->_opeTpUpdate) {
    case ENormalPen: {
        picture = paintNormalPen(event, pInfo);
        break;
    }
    case ECalligraphyPen: {
        picture = paintCalligraphyPen(event, pInfo);
        break;
    }
    case ECrayonPen : {
        picture = paintCrayonPen(event, pInfo);
        break;
    }
    default:
        break;
    }
    _activePictures[event->uuid()].addSubPicture(picture);
    _activePictures[event->uuid()].addPoint(_layers[event->scene()]->mapFromScene((event->pos())));

    //实时显示,绘制到临时显示的缓冲图上
    paintPictureToView(picture, event->view());
}

void CPenTool::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    auto picture = _activePictures.take(event->uuid());
    picture.endSubPicture();

    if (pInfo->_opeTpUpdate == ENormalPen || pInfo->_opeTpUpdate == ECalligraphyPen
            || pInfo->_opeTpUpdate == ECrayonPen || pInfo->_opeTpUpdate == 0) {
        auto pLayer = dynamic_cast<JDynamicLayer *>(_layers[event->scene()]);
        if (pLayer != nullptr)
            pLayer->addPicture(picture.picture(), true, true);

        auto eraserTool = dynamic_cast<CEraserTool *>(drawBoard()->tool(eraser));
        if (eraserTool != nullptr) {
            drawBoard()->tool(eraser)->setEnable(drawBoard()->tool(eraser)->isEnable(event->view()));
        }
    }

    if (_allITERecordInfo.count() == 1) {
        event->view()->setCacheEnable(false);
        _layers.remove(event->scene());
    }
}

int CPenTool::allowedMaxTouchPointCount()
{
    return 10;
}

bool CPenTool::returnToSelectTool(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)

    //return !pInfo->hasMoved();
    return false;
}

int CPenTool::minMoveUpdateDistance()
{
//    if (drawBoard()->currentPage() != nullptr)
//        return 1;
//    return getViewDefualtPen(drawBoard()->currentPage()->view()).width();
    return 0;
}

void CPenTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    auto scene = currentPage() != nullptr ? currentPage()->scene() : nullptr;

    if (scene == nullptr)
        return;

    if (oldStatus == EIdle && nowStatus == EReady) {
        scene->blockSelectionStyle(true);
    }

    if (oldStatus == EReady && nowStatus == EIdle) {
        scene->blockSelectionStyle(false);

        auto it = _layers.find(scene);
        if (it != _layers.end()) {
            scene->clearSelectGroup();
            scene->selectItem(it.value());
        }
    }
}

QPen CPenTool::getViewDefualtPen(PageView *view) const
{
    PageView *pView = view;
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setColor(pView->page()->defaultAttriVar(EPenColor).value<QColor>());
    pen.setWidthF(pView->page()->defaultAttriVar(EPenWidth).value<qreal>());
    return pen;
}

QBrush CPenTool::getViewDefualtBrush(PageView *view) const
{
    PageView *pView = view;
    QBrush brush;
    brush.setColor(pView->page()->defaultAttriVar(EPenColor).value<QColor>());
    return brush;
}

QPicture CPenTool::paintNormalPen(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    QPicture picture;
    QPainter painter(&picture);

    QPointF  prePos = _layers[event->scene()]->mapFromScene(pInfo->_prePos);
    QPointF  pos = _layers[event->scene()]->mapFromScene((event->pos())) ;

    QLineF l(prePos, pos);

    PageView *pView = event->scene()->drawView();
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setColor(pView->page()->defaultAttriVar(EPenColor).value<QColor>());
    pen.setWidthF(pView->page()->defaultAttriVar(EPenWidth).value<qreal>());
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.drawLine(l);

    painter.end();

    return picture;
}

QPicture CPenTool::paintCalligraphyPen(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    PageView *pView = event->scene()->drawView();

    QPicture picture;
    QPainter painter(&picture);

    QPointF  prePos = _layers[event->scene()]->mapFromScene(pInfo->_prePos);
    QPointF  pos = _layers[event->scene()]->mapFromScene((event->pos())) ;

    QLineF l(prePos, pos);
    const qreal angleDegress = 30;
    const qreal cW = pView->page()->defaultAttriVar(EPenWidth).value<qreal>() + 10;
    qreal offXLen = qCos(qDegreesToRadians(angleDegress)) * (cW / 2.0);
    qreal offYLen = qSin(qDegreesToRadians(angleDegress)) * (cW / 2.0);
    QPointF point1(prePos.x() - offXLen, prePos.y() - offYLen);
    QPointF point2(prePos.x() + offXLen, prePos.y() + offYLen);

    QPointF point3(pos.x() - offXLen, pos.y() - offYLen);
    QPointF point4(pos.x() + offXLen, pos.y() + offYLen);

    QPainterPath path;
    path.moveTo(point1);

    if (l.length() > 20) {
        path.quadTo(QLineF(point1, point2).center(), point2);
        path.quadTo(QLineF(point2, point4).center(), point4);
        path.quadTo(QLineF(point4, point3).center(), point3);
        path.quadTo(QLineF(point3, point1).center(), point1);
    } else {
        path.lineTo(point2);
        path.lineTo(point4);
        path.lineTo(point3);
        path.lineTo(point1);
    }
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(pView->page()->defaultAttriVar(EPenColor).value<QColor>());

    QPen p; p.setColor(pView->page()->defaultAttriVar(EPenColor).value<QColor>());
    painter.setPen(p);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.drawPath(path);

    painter.end();

    return picture;
}

QPicture CPenTool::paintCrayonPen(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo, qreal space)
{
    QColor color = drawBoard()->defaultAttriVar(drawBoard()->currentPage(), EPenColor).value<QColor>();

    int px = -1;
    int penWidth = drawBoard()->defaultAttriVar(drawBoard()->currentPage(), EPenWidth).toInt();
    if (penWidth > 1)
        px = penWidth;

    QImage renderImage = pictureColorChanged(m_pRenderImage, color).toImage().scaled(24 + px, 24 + px).convertToFormat(QImage::Format_ARGB32);

    QPicture picture;
    QPainter painter(&picture);

    QPointF  prePos = _layers[event->scene()]->mapFromScene(pInfo->_prePos);
    QPointF  pos = _layers[event->scene()]->mapFromScene((event->pos())) ;

    QLineF line(prePos, pos);
    qreal length = line.length();
    qreal width = 1.0 - length / 80.0;
    width = qMax(width, 0.25);

    painter.setOpacity(width);

    qreal total = space;
    QSize brushTextureSize = renderImage.size();

    if (!qFuzzyIsNull(total)) {
        QRect rect = QRect(QPointF(line.p2().toPoint() - QPointF(brushTextureSize.width() / 2, brushTextureSize.height() / 2)).toPoint(),
                           brushTextureSize);
        painter.drawImage(rect, renderImage);

        pInfo->_prePos = event->pos();
    }
    event->setPosXAccepted(false);
    event->setPosYAccepted(false);

    while (total < length) {
        line.setLength(total);
        QRect rect = QRect(QPointF(line.p2().toPoint() - QPointF(brushTextureSize.width() / 2, brushTextureSize.height() / 2)).toPoint(),
                           brushTextureSize);

        painter.drawImage(rect, renderImage);
        total += space;
    }

    return picture;
}

void CPenTool::paintPictureToView(const QPicture &picture, PageView *view)
{
    QPainter painter(&view->cachedPixmap());

    auto trans = _layers[view->drawScene()]->sceneTransform() * view->viewportTransform();

    painter.setTransform(trans);

    painter.drawPicture(QPoint(0, 0), picture);

    view->viewport()->update(trans.mapRect(picture.boundingRect()));
}

bool CPenTool::eventFilter(QObject *o, QEvent *e)
{
    if (m_pPenStyleComboBox->parentWidget() == o) {
        if (QEvent::ParentChange == e->type()) {
            CComBoxSettingWgt *comb = qobject_cast<CComBoxSettingWgt *>(m_pPenStyleComboBox->parentWidget());

            if (comb->parentWidget() != nullptr) {
                if (comb->parentWidget()->isWindow()) {
                    comb->setText(tr("Style"));
                } else {
                    comb->setText("");
                }
            }
        }
    }
    return IDrawTool::eventFilter(o, e);
}
