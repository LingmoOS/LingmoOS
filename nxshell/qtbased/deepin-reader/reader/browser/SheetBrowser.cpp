// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SheetBrowser.h"
#include "PDFModel.h"
#include "BrowserPage.h"
#include "BrowserWord.h"
#include "DocSheet.h"
#include "BrowserAnnotation.h"
#include "TipsWidget.h"
#include "BrowserMenu.h"
#include "FileAttrWidget.h"
#include "Application.h"
#include "TextEditWidget.h"
#include "BrowserMagniFier.h"
#include "FindWidget.h"
#include "MsgHeader.h"
#include "DjVuModel.h"
#include "Utils.h"
#include "SheetRenderer.h"
#include "SecurityDialog.h"

#include <DMenu>
#include <DGuiApplicationHelper>

#include <QGraphicsItem>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QBitmap>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QUuid>
#include <QScroller>
#include <QPainterPath>
#include <QDebug>
#include <QTemporaryDir>
#include <QProcess>
#include <QTimerEvent>
#include <QDesktopServices>

DWIDGET_USE_NAMESPACE

#define REPEAT_MOVE_DELAY 500
const qreal deltaManhattanLength = 12.0;
SheetBrowser::SheetBrowser(DocSheet *parent) : DGraphicsView(parent), m_sheet(parent)
{
    setMouseTracking(true);

    setScene(new QGraphicsScene(this));

    setFrameShape(QFrame::NoFrame);

    setContextMenuPolicy(Qt::CustomContextMenu);

    setAttribute(Qt::WA_TranslucentBackground);

    setBackgroundBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().itemBackground().color()));

    setAttribute(Qt::WA_AcceptTouchEvents);

    grabGesture(Qt::PinchGesture);//捏合缩放

    setProperty("pinchgetsturing", false);

    m_scroller = QScroller::scroller(this);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onVerticalScrollBarValueChanged(int)));

    connect(verticalScrollBar(), &QScrollBar::sliderPressed, this, &SheetBrowser::onRemoveDocSlideGesture);

    connect(verticalScrollBar(), &QScrollBar::sliderReleased, this, &SheetBrowser::onSetDocSlideGesture);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onHorizontalScrollBarValueChanged(int)));

    connect(horizontalScrollBar(), &QScrollBar::sliderPressed, this, &SheetBrowser::onRemoveDocSlideGesture);

    connect(horizontalScrollBar(), &QScrollBar::sliderReleased, this, &SheetBrowser::onSetDocSlideGesture);

    m_tipsWidget = new TipsWidget(this);
    m_tipsWidget->setAccessibleName("Tips");
    m_tipsWidget->setAutoChecked(true);

    connect(this, SIGNAL(sigAddHighLightAnnot(BrowserPage *, QString, QColor)), this, SLOT(onAddHighLightAnnot(BrowserPage *, QString, QColor)), Qt::QueuedConnection);

    this->verticalScrollBar()->setProperty("_d_slider_spaceUp", 8);
    this->verticalScrollBar()->setProperty("_d_slider_spaceDown", 8);
    this->verticalScrollBar()->setAccessibleName("verticalScrollBar");
    this->horizontalScrollBar()->setProperty("_d_slider_spaceLeft", 8);
    this->horizontalScrollBar()->setProperty("_d_slider_spaceRight", 8);
    this->horizontalScrollBar()->setAccessibleName("horizontalScrollBar");
}

SheetBrowser::~SheetBrowser()
{
    disconnect(this, SIGNAL(sigAddHighLightAnnot(BrowserPage *, QString, QColor)), this, SLOT(onAddHighLightAnnot(BrowserPage *, QString, QColor)));

    qDeleteAll(m_items);

    if (nullptr != m_noteEditWidget)
        delete m_noteEditWidget;
}

QImage SheetBrowser::firstThumbnail(const QString &filePath)
{
    deepin_reader::Document *document = nullptr;

    int fileType = Dr::fileType(filePath);

    deepin_reader::Document::Error error = deepin_reader::Document ::NoError;

    qDebug() << "SheetBrowser::firstThumbnail";
    document = DocumentFactory::getDocument(fileType, filePath, "", "", nullptr, error);

    if (nullptr == document)
        return QImage();

    if (document->pageCount() <= 0)
        return QImage();

    deepin_reader::Page *page = document->page(0);

    if (nullptr == page) {
        delete document;
        return QImage();
    }

    QImage image = page->render(256, 256);

    delete page;

    delete document;

    return image;
}

void SheetBrowser::init(SheetOperation &operation, const QSet<int> &bookmarks)
{
    int pageCount = m_sheet->pageCount();

    for (int i = 0; i < pageCount; ++i) {
        BrowserPage *item = new BrowserPage(this, i, m_sheet);

        if (bookmarks.contains(i))
            item->setBookmark(true);

        m_items.append(item);

        if (m_sheet->renderer()->getPageSize(i).width() > m_maxWidth)
            m_maxWidth = m_sheet->renderer()->getPageSize(i).width();

        if (m_sheet->renderer()->getPageSize(i).height() > m_maxHeight)
            m_maxHeight = m_sheet->renderer()->getPageSize(i).height();

        scene()->addItem(item);
    }

    setMouseShape(operation.mouseShape);

    deform(operation);

    m_initPage = operation.currentPage;

    m_hasLoaded = true;
}

void SheetBrowser::setMouseShape(const Dr::MouseShape &shape)
{
    closeMagnifier();
    if (Dr::MouseShapeHand == shape) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        foreach (BrowserPage *item, m_items)
            item->setWordSelectable(false);
    } else if (Dr::MouseShapeNormal == shape) {
        setDragMode(QGraphicsView::NoDrag);
        foreach (BrowserPage *item, m_items)
            item->setWordSelectable(true);
    }
    m_bHandAndLink = false;
}

void SheetBrowser::setBookMark(int index, int state)
{
    if (index >= 0 && index < m_items.count())
        m_items.at(index)->setBookmark(state);
}

void SheetBrowser::setAnnotationInserting(bool inserting)
{
    m_annotationInserting = inserting;
}

void SheetBrowser::onVerticalScrollBarValueChanged(int)
{
    beginViewportChange();

    int curScrollPage = currentScrollValueForPage();

    if (m_bNeedNotifyCurPageChanged && curScrollPage != m_currentPage) {
        curpageChanged(curScrollPage);
    }
}

void SheetBrowser::onHorizontalScrollBarValueChanged(int)
{
    beginViewportChange();
}

void SheetBrowser::beginViewportChange()
{
    if (nullptr == m_viewportChangeTimer) {
        m_viewportChangeTimer = new QTimer(this);
        connect(m_viewportChangeTimer, &QTimer::timeout, this, &SheetBrowser::onViewportChanged);
        m_viewportChangeTimer->setSingleShot(true);
    }

    if (m_viewportChangeTimer->isActive())
        m_viewportChangeTimer->stop();

    m_viewportChangeTimer->start(100);
}

void SheetBrowser::hideSubTipsWidget()
{
    if (m_tipsWidget)
        m_tipsWidget->hide();

    setCursor(QCursor(Qt::ArrowCursor));
}

void SheetBrowser::onViewportChanged()
{
    int fromIndex = 0;
    int toIndex = 0;
    currentIndexRange(fromIndex, toIndex);

    foreach (BrowserPage *item, m_items) {
        if (item->itemIndex() < fromIndex - 2 || item->itemIndex() > toIndex + 2) {//上下多2个浮动
            item->clearPixmap();
            item->clearWords();
        }
    }
}

void SheetBrowser::onAddHighLightAnnot(BrowserPage *page, QString text, QColor color)
{
    if (page) {
        Annotation *highLightAnnot = nullptr;

        highLightAnnot = page->addHighlightAnnotation(text, color);

        if (highLightAnnot)
            emit sigOperaAnnotation(MSG_NOTE_ADD, highLightAnnot->page - 1, highLightAnnot);

    }
}

void SheetBrowser::showNoteEditWidget(deepin_reader::Annotation *annotation, const QPoint &point)
{
    // 超链接与高亮区域重合时，手形工具下只响应超链接
    if (annotation == nullptr || m_bHandAndLink)
        return;

    m_tipsWidget->hide();
    if (m_noteEditWidget == nullptr) {
        m_noteEditWidget = new TextEditShadowWidget(this);
        connect(m_noteEditWidget->getTextEditWidget(), &TextEditWidget::sigNeedShowTips, m_sheet, &DocSheet::showTips);
        connect(m_noteEditWidget->getTextEditWidget(), &TextEditWidget::sigRemoveAnnotation, this, &SheetBrowser::onRemoveAnnotation);
        connect(m_noteEditWidget->getTextEditWidget(), &TextEditWidget::sigUpdateAnnotation, this, &SheetBrowser::onUpdateAnnotation);
        connect(m_noteEditWidget->getTextEditWidget(), &TextEditWidget::sigHide, this, [ = ] {
            setIconAnnotSelect(false);
        });
    }

    m_noteEditWidget->getTextEditWidget()->setEditText(annotation->contents());
    m_noteEditWidget->getTextEditWidget()->setAnnotation(annotation);
    m_noteEditWidget->showWidget(point);
}

bool SheetBrowser::calcIconAnnotRect(BrowserPage *page, const QPointF &point, QRectF &iconRect)
{
    if (nullptr == page || nullptr == m_sheet)
        return false;

    const SheetOperation  &operation = m_sheet->operation();

    qreal scaleFactor = operation.scaleFactor;

    QPointF clickPoint = page->mapFromScene(point);

    clickPoint = getAnnotPosInPage(clickPoint, page);

    if (clickPoint.x() < 0 || clickPoint.y() < 0 || qFuzzyIsNull(scaleFactor - 10))
        return false;

    iconRect = QRectF(clickPoint.x() / scaleFactor - 10, clickPoint.y() / scaleFactor - 10, 20, 20);

    return true;
}

QPointF SheetBrowser::translate2Local(QPointF clickPoint)
{
    const SheetOperation  &operation = m_sheet->operation();

    if (qFuzzyIsNull(operation.scaleFactor)) {
        return QPointF();
    }

    return QPointF(clickPoint.x() / operation.scaleFactor, clickPoint.y() / operation.scaleFactor);
}

Annotation *SheetBrowser::getClickAnnot(BrowserPage *page, const QPointF clickPoint, bool drawRect)
{
    if (nullptr == m_sheet || nullptr == page)
        return nullptr;

    QPointF pagePointF = page->mapFromScene(clickPoint);

    pagePointF = translate2Local(pagePointF);

    foreach (Annotation *annot, page->annotations()) {
        foreach (const QRectF &rect, annot->boundary()) {
            if (rect.contains(pagePointF)) {
                if (drawRect)
                    page->setSelectIconRect(true, annot);
                return annot;
            }
        }
    }

    return nullptr;
}

Annotation *SheetBrowser::addHighLightAnnotation(const QString contains, const QColor color, QPoint &showPoint)
{
    Annotation *highLightAnnot = nullptr;

    if (m_selectStartWord == nullptr || m_selectEndWord == nullptr) {
        return nullptr;
    }

    BrowserPage *startPage = qgraphicsitem_cast<BrowserPage *>(m_selectStartWord->parentItem());
    BrowserPage *endPage = qgraphicsitem_cast<BrowserPage *>(m_selectEndWord->parentItem());

    m_selectEndPos = QPointF();
    m_selectStartPos = QPointF();

    if (startPage == nullptr || endPage == nullptr) {
        m_selectStartWord = nullptr;
        m_selectEndWord = nullptr;
        return nullptr;
    }

    //item坐标转成全局坐标,注释编辑框的显示位置
    showPoint =  this->mapToGlobal(this->mapFromScene(m_selectEndWord->mapToScene(m_selectEndWord->boundingRect().topRight())));

    if (startPage == endPage) {
        highLightAnnot = startPage->addHighlightAnnotation(contains, color);
    } else {
        if (endPage->itemIndex() < startPage->itemIndex())
            qSwap(startPage, endPage);

        int startIndex = m_items.indexOf(startPage);
        int endIndex = m_items.indexOf(endPage);

        for (int index = startIndex; index < endIndex; index++) {
            if (m_items.at(index))
                emit sigAddHighLightAnnot(m_items.at(index), QString(), color);
        }

        highLightAnnot = endPage->addHighlightAnnotation(contains, color);
    }

    if (highLightAnnot) {
        emit sigOperaAnnotation(MSG_NOTE_ADD, highLightAnnot->page - 1, highLightAnnot);
    }

    m_selectStartWord = nullptr;
    m_selectEndWord = nullptr;

    return highLightAnnot;
}

void SheetBrowser::jump2PagePos(BrowserPage *jumpPage, const qreal posLeft, const qreal posTop)
{
    if (nullptr == jumpPage)
        return;

    const SheetOperation &operation = m_sheet->operation();
    Dr::Rotation rotation = operation.rotation;

    m_bNeedNotifyCurPageChanged = false;
    if (Dr::RotateBy0 == rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(jumpPage->pos().x() + posLeft * m_lastScaleFactor));
        verticalScrollBar()->setValue(static_cast<int>(jumpPage->pos().y() + posTop * m_lastScaleFactor));
    } else if (Dr::RotateBy90 == rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(jumpPage->pos().x() - posTop * m_lastScaleFactor));
        verticalScrollBar()->setValue(static_cast<int>(jumpPage->pos().y() + posLeft * m_lastScaleFactor));
    } else if (Dr::RotateBy180 == rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(jumpPage->pos().x() - posLeft * m_lastScaleFactor));
        verticalScrollBar()->setValue(static_cast<int>(jumpPage->pos().y() - posTop * m_lastScaleFactor));
    } else if (Dr::RotateBy270 == rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(jumpPage->pos().x() + posTop * m_lastScaleFactor));
        verticalScrollBar()->setValue(static_cast<int>(jumpPage->pos().y() - posLeft * m_lastScaleFactor));
    }
    m_bNeedNotifyCurPageChanged = true;

    curpageChanged(jumpPage->itemIndex() + 1);
}

bool SheetBrowser::moveIconAnnot(BrowserPage *page, const QPointF &clickPoint)
{
    if (nullptr == page)
        return false;

    QRectF iconRect;

    bool isVaild = calcIconAnnotRect(page, clickPoint, iconRect);

    if (isVaild) {
        return page->moveIconAnnotation(iconRect);
    }

    return false;
}

void SheetBrowser::currentIndexRange(int &fromIndex, int &toIndex)
{
    fromIndex = -1;
    toIndex = -1;

    int value = verticalScrollBar()->value();

    for (int i = 0; i < m_items.count(); ++i) {
        int y = 0;

        switch (m_sheet->operation().rotation) {
        case Dr::RotateBy0:
        case Dr::RotateBy90:
            y = static_cast<int>(m_items[i]->y());
            break;
        case Dr::RotateBy180:
            y = static_cast<int>(m_items[i]->y() - m_items[i]->boundingRect().height());
            break;
        case Dr::RotateBy270:
            y = static_cast<int>(m_items[i]->y() - m_items[i]->boundingRect().width());
            break;
        default:
            break;
        }

        if (-1 == fromIndex && y + m_items[i]->rect().height() >= value) {
            fromIndex = i;
        }

        if (-1 == toIndex && y >= (value + this->height())) {
            toIndex = i - 1;
            break;
        }

        if (i == (m_items.count() - 1)) {
            toIndex = i;
        }
    }
}

QString SheetBrowser::selectedWordsText()
{
    QString text;
    foreach (BrowserPage *item, m_items)
        text += item->selectedWords();

    return text;
}

QList<deepin_reader::Annotation *> SheetBrowser::annotations()
{
    QList<deepin_reader::Annotation *> list;

    foreach (BrowserPage *item, m_items) {
        list.append(item->annotations());
    }

    return list;
}

bool SheetBrowser::removeAnnotation(deepin_reader::Annotation *annotation)
{
    bool ret = false;
    int pageIndex = -1;
    foreach (BrowserPage *item, m_items) {
        if (item->hasAnnotation(annotation)) {
            pageIndex = annotation->page - 1;
            ret = item->removeAnnotation(annotation);
            break;
        }
    }

    if (ret)
        emit sigOperaAnnotation(MSG_NOTE_DELETE, pageIndex, annotation);

    return ret;
}

bool SheetBrowser::removeAllAnnotation()
{
    foreach (BrowserPage *item, m_items) {
        if (item && item->removeAllAnnotation()) {
        }
    }

    emit sigOperaAnnotation(MSG_ALL_NOTE_DELETE, -1, nullptr);

    return true;
}

bool SheetBrowser::updateAnnotation(deepin_reader::Annotation *annotation, const QString &text, QColor color)
{
    if (nullptr == annotation)
        return false;

    bool ret = false;

    int pageIndex = annotation->page - 1;

    foreach (BrowserPage *item, m_items) {
        if (item && item->hasAnnotation(annotation)) {
            ret = item->updateAnnotation(annotation, text, color);
        }
    }

    if (ret) {
        if (!text.isEmpty())
            emit sigOperaAnnotation(MSG_NOTE_ADD, pageIndex, annotation);
        else
            emit sigOperaAnnotation(MSG_NOTE_DELETE, pageIndex, annotation);
    }

    return ret;
}

void SheetBrowser::onRemoveAnnotation(deepin_reader::Annotation *annotation, bool tips)
{
    m_sheet->removeAnnotation(annotation, tips);
}

void SheetBrowser::onUpdateAnnotation(deepin_reader::Annotation *annotation, const QString &text)
{
    updateAnnotation(annotation, text);
}

void SheetBrowser::onSetDocSlideGesture()
{
    m_scroller->stop();
}

void SheetBrowser::onRemoveDocSlideGesture()
{
    m_scroller->stop();
}

void SheetBrowser::onRemoveIconAnnotSelect()
{
    clearSelectIconAnnotAfterMenu();
}

void SheetBrowser::onInit()
{
    if (1 != m_initPage) {
        setCurrentPage(m_initPage);
        m_initPage = 1;
    }

    onViewportChanged();
}

void SheetBrowser::jumpToOutline(const qreal &linkLeft, const qreal &linkTop, int index)
{
    int pageIndex = index;

    if (nullptr == m_sheet || pageIndex < 0 || pageIndex >= m_items.count() || linkLeft < 0 || linkTop < 0)
        return;

    const SheetOperation  &operation = m_sheet->operation();
    Dr::Rotation rotation = operation.rotation;

    if (rotation != Dr::NumberOfRotations && rotation != Dr::RotateBy0) {
        setCurrentPage(++pageIndex);
        return;
    }

    BrowserPage *jumpPage = m_items.at(pageIndex);
    if (nullptr == jumpPage)
        return;

    if (linkTop > 0)
        jump2PagePos(jumpPage, linkLeft, jumpPage->boundingRect().height() / operation.scaleFactor - linkTop);
    else
        jump2PagePos(jumpPage, linkLeft, 0);
}

void SheetBrowser::jumpToHighLight(deepin_reader::Annotation *annotation, const int index)
{
    if (nullptr == m_sheet || nullptr == annotation || index < 0 || index >= m_items.count())
        return;

    BrowserPage *jumpPage = m_items.at(index);

    const QList<QRectF> &anootList = annotation->boundary();

    if (nullptr == jumpPage || anootList.count() < 1)
        return;

    QRectF rect = anootList.at(0);

    if (rect.isNull())
        return;

    qreal posLeft = rect.left();

    qreal posTop = rect.top();

    const SheetOperation &operation = m_sheet->operation();
    Dr::Rotation rotation = operation.rotation;

    if (Dr::RotateBy90 == rotation) {
        posLeft = rect.left();
        posTop = rect.bottom();
    } else if (Dr::RotateBy180 == rotation) {
        posLeft = rect.right();
        posTop = rect.bottom();
    } else if (Dr::RotateBy270 == rotation) {
        posLeft = rect.right();
        posTop = rect.top();
    }

    jump2PagePos(jumpPage, posLeft, posTop);
}

void SheetBrowser::wheelEvent(QWheelEvent *event)
{
    // 当注释窗口弹出时，屏蔽鼠标滚动
    if (nullptr != m_noteEditWidget && !m_noteEditWidget->isHidden()) {
        return;
    }

    m_scroller->stop();

    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        if (nullptr == m_sheet)
            return;

        if (event->delta() > 0) {
            m_sheet->zoomin();
        } else {
            m_sheet->zoomout();
        }

        return;
    }

    QGraphicsView::wheelEvent(event);
}

bool SheetBrowser::event(QEvent *event)
{
    if (event && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent && keyEvent->key() == Qt::Key_Tab && !keyEvent->isAutoRepeat()) {
            if (m_lastSelectIconAnnotPage && m_items.contains(m_lastSelectIconAnnotPage)) {
                m_lastSelectIconAnnotPage->setSelectIconRect(false);
            }
        }

        if (keyEvent && keyEvent->key() == Qt::Key_Menu && !keyEvent->isAutoRepeat()) {
            this->showMenu();
        }
        if (keyEvent->key() == Qt::Key_M && (keyEvent->modifiers() & Qt::AltModifier) && !keyEvent->isAutoRepeat()) {
            //搜索框
            if (m_findWidget && m_findWidget->isVisible() && m_findWidget->hasFocus()) {
                return DGraphicsView::event(event);
            }

            this->showMenu();
        }
    }

    if (event->type() == QEvent::Gesture) {
        return gestureEvent(reinterpret_cast<QGestureEvent *>(event));
    }

    return QGraphicsView::event(event);
}

bool SheetBrowser::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(reinterpret_cast<QPinchGesture *>(pinch));

    return true;
}

void SheetBrowser::pinchTriggered(QPinchGesture *gesture)
{
    static bool  canRotate = false;
    static qreal currentStepScaleFactor = 1.0;
    static qreal tempScalefactor = 1.0;

    if (gesture->state() == Qt::GestureStarted) {
        m_startPinch = true;
        canRotate = true;
        tempScalefactor = m_lastScaleFactor;
        this->setProperty("pinchgetsturing", true);
        scene()->setSelectionArea(QPainterPath());
    }

    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        if (canRotate && qAbs(gesture->rotationAngle()) > 35.0) {
            if (gesture->rotationAngle() < 0.0) {
                m_sheet->rotateLeft();
            } else {
                m_sheet->rotateRight();
            }
            canRotate = false;
            return;
        }
    }

    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        currentStepScaleFactor = gesture->totalScaleFactor();
    }

    qreal curscalfactor = currentStepScaleFactor * tempScalefactor;
    if (gesture->state() == Qt::GestureFinished) {
        this->setProperty("pinchgetsturing", false);
        QTimer::singleShot(10, [this]() {
            //稍微延迟下,不然还是会引起mouse事件触发
            m_startPinch = false;
        });
        canRotate = false;
        currentStepScaleFactor = 1.0;
        tempScalefactor = m_lastScaleFactor;
    }

    m_sheet->setScaleFactor(curscalfactor);
}

void SheetBrowser::deform(SheetOperation &operation)
{
    m_lastScaleFactor = operation.scaleFactor;

    //根据缩放模式调整缩放比例
    switch (operation.rotation) {
    default:
    case Dr::RotateBy0:
    case Dr::RotateBy180:
        if (Dr::FitToPageWidthMode == operation.scaleMode)
            operation.scaleFactor = static_cast<double>(this->width() - 25.0) / m_maxWidth / (Dr::TwoPagesMode == operation.layoutMode ? 2 : 1);
        else if (Dr::FitToPageHeightMode == operation.scaleMode)
            operation.scaleFactor = static_cast<double>(this->height()) / m_maxHeight;
        else if (Dr::FitToPageDefaultMode == operation.scaleMode)
            operation.scaleFactor = 1.0 ;
        else if (Dr::FitToPageWorHMode == operation.scaleMode) {
            qreal scaleFactor = static_cast<double>(this->width() - 25.0) / m_maxWidth / (Dr::TwoPagesMode == operation.layoutMode ? 2 : 1);
            if (scaleFactor * m_maxHeight > this->height())
                scaleFactor = static_cast<double>(this->height()) / m_maxHeight;
            operation.scaleFactor = scaleFactor;
        } else
            operation.scaleMode = Dr::ScaleFactorMode;
        break;
    case Dr::RotateBy90:
    case Dr::RotateBy270:
        if (Dr::FitToPageWidthMode == operation.scaleMode)
            operation.scaleFactor = static_cast<double>(this->width() - 25.0) / m_maxHeight / (Dr::TwoPagesMode == operation.layoutMode ? 2 : 1);
        else if (Dr::FitToPageHeightMode == operation.scaleMode)
            operation.scaleFactor = static_cast<double>(this->height()) / m_maxWidth;
        else if (Dr::FitToPageDefaultMode == operation.scaleMode)
            operation.scaleFactor = 1.0 ;
        else if (Dr::FitToPageWorHMode == operation.scaleMode) {
            qreal scaleFactor = static_cast<double>(this->width() - 25.0) / m_maxHeight / (Dr::TwoPagesMode == operation.layoutMode ? 2 : 1);
            if (scaleFactor * m_maxWidth > this->height())
                scaleFactor = static_cast<double>(this->height()) / m_maxWidth;
            operation.scaleFactor = scaleFactor;
        } else
            operation.scaleMode = Dr::ScaleFactorMode;
        break;
    }

    int page = operation.currentPage;
    //进行render 并算出最宽的一行
    double maxWidth = 0;        //最宽的一行
    double maxHeight = 0;       //总高度
    int space = 5;              //页之间间隙

    for (int i = 0; i < m_items.count(); ++i) {
        //m_items[i]->clearWords();     //忘记为什么要删除，暂时注释

        if (i % 2 == 1)
            continue;

        int j = i + 1;

        m_items.at(i)->render(operation.scaleFactor, operation.rotation, true);

        if (j < m_items.count())
            m_items.at(j)->render(operation.scaleFactor, operation.rotation, true);

        if (Dr::SinglePageMode == operation.layoutMode) {
            if (m_items.at(i)->rect().width() > maxWidth)
                maxWidth = static_cast<int>(m_items.at(i)->rect().width());

            if (j < m_items.count()) {
                if (m_items.at(j)->rect().width() > maxWidth)
                    maxWidth = static_cast<int>(m_items.at(j)->rect().width());
            }

        } else if (Dr::TwoPagesMode == operation.layoutMode) {
            if (j < m_items.count()) {
                if (static_cast<int>(m_items.at(i)->rect().width() + m_items.at(i + 1)->rect().width()) > maxWidth)
                    maxWidth = static_cast<int>(m_items.at(i)->rect().width() + m_items.at(i + 1)->rect().width());
            } else {
                if (static_cast<int>(m_items.at(i)->rect().width()) * 2 > maxWidth) {
                    maxWidth = static_cast<int>(m_items.at(i)->rect().width()) * 2;
                }
            }
        }
    }

    if (Dr::SinglePageMode == operation.layoutMode) {
        for (int i = 0; i < m_items.count(); ++i) {
            int x = static_cast<int>(maxWidth - m_items.at(i)->rect().width()) / 2;
            if (x < 0)
                x = 0;

            if (Dr::RotateBy0 == operation.rotation)
                m_items.at(i)->setPos(x, maxHeight);
            else if (Dr::RotateBy90 == operation.rotation)
                m_items.at(i)->setPos(x + m_items.at(i)->boundingRect().height(), maxHeight);
            else if (Dr::RotateBy180 == operation.rotation)
                m_items.at(i)->setPos(x + m_items.at(i)->boundingRect().width(), maxHeight + m_items.at(i)->boundingRect().height());
            else if (Dr::RotateBy270 == operation.rotation)
                m_items.at(i)->setPos(x, maxHeight + m_items.at(i)->boundingRect().width());

            maxHeight += m_items.at(i)->rect().height() + space;
        }
    } else if (Dr::TwoPagesMode == operation.layoutMode) {
        for (int i = 0; i < m_items.count(); ++i) {
            if (i % 2 == 1)
                continue;

            int x = static_cast<int>(maxWidth / 2 - m_items.at(i)->rect().width());

            if (Dr::RotateBy0 == operation.rotation) {
                m_items.at(i)->setPos(x, maxHeight);
                if (m_items.count() > i + 1) {
                    m_items.at(i + 1)->setPos(x + space + m_items.at(i)->rect().width(), maxHeight);
                }
            } else if (Dr::RotateBy90 == operation.rotation) {
                m_items.at(i)->setPos(x + m_items.at(i)->boundingRect().height(), maxHeight);
                if (m_items.count() > i + 1) {
                    m_items.at(i + 1)->setPos(x + space + m_items.at(i)->rect().width() + m_items.at(i + 1)->boundingRect().height(), maxHeight);
                }
            } else if (Dr::RotateBy180 == operation.rotation) {
                m_items.at(i)->setPos(x + m_items.at(i)->boundingRect().width(), maxHeight + m_items.at(i)->boundingRect().height());
                if (m_items.count() > i + 1) {
                    m_items.at(i + 1)->setPos(x + space + m_items.at(i)->rect().width() + m_items.at(i + 1)->boundingRect().width(), maxHeight + m_items.at(i + 1)->boundingRect().height());
                }
            } else if (Dr::RotateBy270 == operation.rotation) {
                m_items.at(i)->setPos(x, maxHeight + m_items.at(i)->boundingRect().width());
                if (m_items.count() > i + 1) {
                    m_items.at(i + 1)->setPos(x + space + m_items.at(i)->rect().width(), maxHeight + m_items.at(i + 1)->boundingRect().width());
                }
            }

            if (m_items.count() > i + 1)
                maxHeight +=  qMax(m_items.at(i)->rect().height(), m_items.at(i + 1)->rect().height()) + space;
            else
                maxHeight += m_items.at(i)->rect().height() + space;

        }
        maxWidth += space;
    }

    setSceneRect(0, 0, maxWidth, maxHeight);

    if (page > 0 && page <= m_items.count()) {
        verticalScrollBar()->setValue(static_cast<int>(m_items[page - 1]->getTopLeftPos().y()));
        horizontalScrollBar()->setValue(static_cast<int>(m_items[page - 1]->getTopLeftPos().x()));
    }

    //update Magnifier Image
    if (magnifierOpened() && operation.rotation != m_lastrotation) {
        m_magnifierLabel->updateImage();
    }
    m_lastrotation = operation.rotation;

    if (m_tipsWidget)
        m_tipsWidget->hide();

    beginViewportChange();
}

bool SheetBrowser::hasLoaded()
{
    return m_hasLoaded;
}

void SheetBrowser::resizeEvent(QResizeEvent *event)
{
    if (hasLoaded() && m_sheet->operation().scaleMode != Dr::ScaleFactorMode) {
        deform(m_sheet->operationRef());
        m_sheet->setOperationChanged();
    }

    if (magnifierOpened()) {
        QTimer::singleShot(1, this, SLOT(openMagnifier()));
    }

    QGraphicsView::resizeEvent(event);
}

void SheetBrowser::mousePressEvent(QMouseEvent *event)
{
    QPointF point = event->pos();

    BrowserPage *page = getBrowserPageForPoint(point);

    m_scroller->stop();

    if (!m_startPinch && (QGraphicsView::NoDrag == dragMode() || QGraphicsView::RubberBandDrag == dragMode())) {
        Qt::MouseButton btn = event->button();
        m_iconAnnot = nullptr;

        if (btn == Qt::LeftButton) {

            //清除上一次选中
            clearSelectIconAnnotAfterMenu();

            m_canTouchScreen = false;
            //点击文字,链接,图标注释,手势滑动时,不滑动文档页面
            if (event->source() == Qt::MouseEventSynthesizedByQt && setDocTapGestrue(event->pos())) {
                m_canTouchScreen = true;
                m_repeatTimer.start(REPEAT_MOVE_DELAY, this);
                m_scroller->handleInput(QScroller::InputPress, event->pos(), static_cast<qint64>(event->timestamp()));
            } else {
                m_selectStartWord = nullptr;
                m_selectEndWord = nullptr;
                scene()->setSelectionArea(QPainterPath());
            }

            m_selectWord = nullptr;
            m_selectEndPos = QPointF();
            m_selectStartPos = m_selectPressedPos = mapToScene(event->pos());

            if (page != nullptr) {
                m_selectIndex = page->itemIndex();
                //add by dxh 2020-8-19  防止书签附近有文字时,操作书签无效
                page->setPageBookMark(page->mapFromScene(m_selectPressedPos));
            }

            deepin_reader::Annotation *clickAnno = getClickAnnot(page, m_selectPressedPos, true);

            if (m_lastSelectIconAnnotPage)
                m_lastSelectIconAnnotPage->setSelectIconRect(false);

            m_lastSelectIconAnnotPage = page;

            if (clickAnno && clickAnno->type() == deepin_reader::Annotation::AText) {
                m_selectIconAnnotation = true;
                m_iconAnnotationMovePos = m_selectPressedPos;
                m_annotationInserting = false;
                m_iconAnnot = clickAnno;
                return DGraphicsView::mousePressEvent(event);
            }
        } else if (btn == Qt::RightButton) {
            closeMagnifier();

            m_selectPressedPos = QPointF();

            BrowserPage *item = nullptr;

            BrowserAnnotation *annotation = nullptr;

            BrowserWord *selectWord = nullptr;

            const QList<QGraphicsItem *>  &list = scene()->items(mapToScene(event->pos()));

            const QString &selectWords = selectedWordsText();

            for (int i = list.size() - 1; i >= 0; i--) {
                QGraphicsItem *baseItem = list.at(i);
                if (nullptr != dynamic_cast<BrowserAnnotation *>(baseItem)) {
                    annotation = dynamic_cast<BrowserAnnotation *>(baseItem);
                }

                if (nullptr != dynamic_cast<BrowserPage *>(baseItem)) {
                    item = dynamic_cast<BrowserPage *>(baseItem);
                }

                if (nullptr != dynamic_cast<BrowserWord *>(baseItem)) {
                    selectWord = dynamic_cast<BrowserWord *>(baseItem);
                }
            }

            m_tipsWidget->hide();

            if (item == nullptr)
                return;

            BrowserMenu menu;
            connect(&menu, &BrowserMenu::signalMenuItemClicked, [ & ](const QString & objectname) {
                const QPointF &clickPos = mapToScene(event->pos());
                if (objectname == "Copy") {
                    Utils::setCurrentFilePath(m_sheet->filePath());
                    //右键菜单->复制
                    Utils::copyText(selectWords);
                } else if (objectname == "CopyAnnoText") {
                    //右键菜单->复制注释文本
                    if (annotation){
                        Utils::setCurrentFilePath(m_sheet->filePath());
                        Utils::copyText(annotation->annotationText());
                    }
                } else if (objectname == "AddTextHighlight") {
                    QPoint pointEnd;
                    addHighLightAnnotation("", Utils::getCurHiglightColor(), pointEnd);
                } else if (objectname == "ChangeAnnotationColor") {
                    if (annotation) {
                        updateAnnotation(annotation->annotation(), annotation->annotationText(), Utils::getCurHiglightColor());
                    }
                } else if (objectname == "RemoveAnnotation") {
                    if (annotation) {
                        m_selectIconAnnotation = false;
                        if (m_lastSelectIconAnnotPage)
                            m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);
                        m_sheet->removeAnnotation(annotation->annotation());
                    }
                } else if (objectname == "AddAnnotationIcon") {
                    if (annotation)  {
                        updateAnnotation(annotation->annotation(), annotation->annotationText(), QColor());
                        showNoteEditWidget(annotation->annotation(), mapToGlobal(event->pos()));
                    } else {
                        Annotation *iconAnnot = addIconAnnotation(page, clickPos, "");
                        if (iconAnnot)
                            showNoteEditWidget(iconAnnot, mapToGlobal(event->pos()));
                    }
                } else if (objectname == "AddBookmark") {
                    m_sheet->setBookMark(item->itemIndex(), true);
                } else if (objectname == "RemoveHighlight") {
                    if (annotation)
                        m_sheet->removeAnnotation(annotation->annotation(), !annotation->annotationText().isEmpty());
                } else if (objectname == "AddAnnotationHighlight") {
                    if (annotation)  {
                        updateAnnotation(annotation->annotation(), annotation->annotationText(), Utils::getCurHiglightColor());
                        showNoteEditWidget(annotation->annotation(), mapToGlobal(event->pos()));
                    } else {
                        QPoint pointEnd = event->pos();
                        deepin_reader::Annotation *addAnnot = nullptr;
                        addAnnot = addHighLightAnnotation("", Utils::getCurHiglightColor(), pointEnd);
                        if (addAnnot)
                            showNoteEditWidget(addAnnot, mapToGlobal(event->pos()));
                    }
                } else if (objectname == "Search") {
                    m_sheet->prepareSearch();
                } else if (objectname == "RemoveBookmark") {
                    m_sheet->setBookMark(item->itemIndex(), false);
                } else if (objectname == "Fullscreen") {
                    m_sheet->openFullScreen();
                } else if (objectname == "ExitFullscreen") {
                    m_sheet->closeFullScreen();
                } else if (objectname == "SlideShow") {
                    m_sheet->openSlide();
                } else if (objectname == "FirstPage") {
                    this->emit sigNeedPageFirst();
                } else if (objectname == "PreviousPage") {
                    this->emit sigNeedPagePrev();
                } else if (objectname == "NextPage") {
                    this->emit sigNeedPageNext();
                } else if (objectname == "LastPage") {
                    this->emit sigNeedPageLast();
                } else if (objectname == "RotateLeft") {
                    m_sheet->rotateLeft();
                } else if (objectname == "RotateRight") {
                    m_sheet->rotateRight();
                } else if (objectname == "Print") {
                    QTimer::singleShot(1, m_sheet, SLOT(onPopPrintDialog()));
                } else if (objectname == "DocumentInfo") {
                    QTimer::singleShot(1, m_sheet, SLOT(onPopInfoDialog()));
                }
            });

            connect(&menu, &BrowserMenu::sigMenuHide, this, &SheetBrowser::onRemoveIconAnnotSelect);

            if (annotation && annotation->annotationType() == deepin_reader::Annotation::AText) {
                if (m_lastSelectIconAnnotPage)
                    m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);
                //文字注释(图标)
                menu.initActions(m_sheet, item->itemIndex(), SheetMenuType_e::DOC_MENU_ANNO_ICON, annotation->annotationText());
            } else if (selectWord && selectWord->isSelected() && !selectWords.isEmpty()) {
                //选择文字
                menu.initActions(m_sheet, item->itemIndex(), SheetMenuType_e::DOC_MENU_SELECT_TEXT);
            } else if (annotation && annotation->annotationType() == deepin_reader::Annotation::AHighlight) {
                //文字高亮注释
                menu.initActions(m_sheet, item->itemIndex(), SheetMenuType_e::DOC_MENU_ANNO_HIGHLIGHT, annotation->annotationText());
            } else if (nullptr != item) {
                //默认
                menu.initActions(m_sheet, item->itemIndex(), SheetMenuType_e::DOC_MENU_DEFAULT);
            }
            menu.exec(mapToGlobal(event->pos()));

            //菜单点击后，需要重新重置下页面，不然不会刷新书签状态
            item->updateBookmarkState();
            //这里return是不希望页面上再次触发GraphicsSceneHoverMove事件
            return;
        }
    }

    DGraphicsView::mousePressEvent(event);
}

void SheetBrowser::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_startPinch && (QGraphicsView::NoDrag == dragMode() || QGraphicsView::RubberBandDrag == dragMode()) && !m_bHandAndLink) {
        QPoint mousePos = event->pos();

        //触摸
        if (m_canTouchScreen && event->source() == Qt::MouseEventSynthesizedByQt) {
            QPointF delta = mapToScene(mousePos) - m_selectPressedPos;
            if (!m_selectPressedPos.isNull() && delta.manhattanLength() > 20) {
                m_repeatTimer.stop();
                m_scroller->handleInput(QScroller::InputMove, event->pos(), static_cast<qint64>(event->timestamp()));
            }

            return QGraphicsView::mouseMoveEvent(event);
        }

        //处理当前拖放图标注释
        if (m_selectIconAnnotation && m_lastSelectIconAnnotPage) {
            QPointF posInPage = m_lastSelectIconAnnotPage->mapFromScene(mapToScene(mousePos));

            posInPage = getAnnotPosInPage(posInPage, m_lastSelectIconAnnotPage);

            QPointF curPointF = m_lastSelectIconAnnotPage->mapToScene(posInPage);

            const QPointF &delta = curPointF - m_iconAnnotationMovePos;

            if (delta.manhattanLength() > deltaManhattanLength) {
                m_iconAnnotationMovePos = QPointF();

                m_lastSelectIconAnnotPage->setDrawMoveIconRect(true);

                m_lastSelectIconAnnotPage->setIconMovePos(m_lastSelectIconAnnotPage->mapFromScene(curPointF));

                setCursor(QCursor(Qt::PointingHandCursor));

                if (m_tipsWidget)
                    m_tipsWidget->hide();

                return QGraphicsView::mouseMoveEvent(event);
            }
        }

        if (m_sheet->isFullScreen() && !m_sheet->operation().sidebarVisible) {
            if (mousePos.x() <= 0) {
                m_sheet->setSidebarVisible(true, false);
            } else {
                m_sheet->setSidebarVisible(false, false);
            }
        }

        if (magnifierOpened()) {
            //放大镜
            showMagnigierImage(mousePos);
        } else {
            QPointF mousposF = event->pos();

            BrowserPage *page = getBrowserPageForPoint(mousposF);

            if (page) { //加载页码内的文字
                page->loadWords();
                if (m_selectIndex >= 0 && !m_selectPressedPos.isNull()) {//将两页之间所有的页面文字都取出来
                    if (page->itemIndex() - m_selectIndex > 1) {
                        for (int i = m_selectIndex + 1; i < page->itemIndex(); ++i) {
                            m_items.at(i)->loadWords();
                        }
                    } else if (m_selectIndex - page->itemIndex() > 1) {
                        for (int i = page->itemIndex() + 1; i < m_selectIndex; ++i) {
                            m_items.at(i)->loadWords();
                        }
                    }
                }
            }

            if (m_selectPressedPos.isNull()) { //鼠标处于非按压状态
                if (page) {
                    const Link &mlink = getLinkAtPoint(mousePos);
                    BrowserAnnotation *browserAnno = page->getBrowserAnnotation(mousposF);
                    //鼠标所在位置存在注释且不为空 当前非平板模式 显示tips
                    if (event->source() != Qt::MouseEventSynthesizedByQt && browserAnno && !browserAnno->annotationText().isEmpty()) {
                        m_tipsWidget->setText(browserAnno->annotationText());
                        QPoint showRealPos(QCursor::pos().x(), QCursor::pos().y() + 20);
                        m_tipsWidget->move(showRealPos);
                        m_tipsWidget->show();
                        setCursor(QCursor(Qt::PointingHandCursor));
                    } else if (mlink.isValid() && nullptr == browserAnno) {
                        //处理移动到超链接区域
                        //未开启放大镜时，如果超链接文本添加了高亮注释，高亮注释优先显示
                        if (!mlink.urlOrFileName.isEmpty()) { // 超链接地址为空时，不显示浮窗
                            QPoint showRealPos(QCursor::pos().x(), QCursor::pos().y() + 20);
                            m_tipsWidget->move(showRealPos);
                            m_tipsWidget->setText(mlink.urlOrFileName);
                            m_tipsWidget->show();
                        }
                        setCursor(QCursor(Qt::PointingHandCursor)); //设为指针光标
                    } else if (page->getBrowserWord(mousposF)) {
                        m_tipsWidget->hide();
                        setCursor(QCursor(Qt::IBeamCursor));
                    } else {
                        m_tipsWidget->hide();
                        setCursor(QCursor(Qt::ArrowCursor));
                    }
                } else {
                    setCursor(QCursor(Qt::ArrowCursor));
                }
            } else {//当前正在被按下
                m_tipsWidget->hide();
                QPointF beginPos = m_selectPressedPos;
                QPointF endPos = mapToScene(event->pos());
                //根据鼠标位置移动滚动条
                moveScrollBar(event->pos());
                //开始的word
                const QList<QGraphicsItem *> &beginItemList = scene()->items(beginPos);
                BrowserWord *beginWord = nullptr;
                foreach (QGraphicsItem *item, beginItemList) {
                    if (!item->isPanel()) {
                        beginWord = qgraphicsitem_cast<BrowserWord *>(item);
                        break;
                    }
                }

                //结束的word
                const QList<QGraphicsItem *> &endItemList = scene()->items(endPos);
                BrowserWord *endWord = nullptr;
                foreach (QGraphicsItem *item, endItemList) {
                    if (!item->isPanel()) {
                        endWord = qgraphicsitem_cast<BrowserWord *>(item);
                        break;
                    }
                }

                //应该只存这几页的words 暂时等于所有的
                const QList<QGraphicsItem *> &words = scene()->items(sceneRect());

                //先考虑字到字的
                if (beginWord != nullptr && endWord != nullptr && beginWord != endWord) {
                    //记录选取文字的最后最后位置,如果在选取文字时,鼠标在文档外释放,以此坐标为准
                    m_selectWord = beginWord;
                    m_selectStartWord = beginWord;
                    m_selectEndWord = endWord;

                    scene()->setSelectionArea(QPainterPath());
                    bool between = false;
                    for (int i = words.size() - 1; i >= 0; i--) {
                        if (words[i]->isPanel())
                            continue;

                        if (qgraphicsitem_cast<BrowserWord *>(words[i]) == beginWord || qgraphicsitem_cast<BrowserWord *>(words[i]) == endWord) {
                            if (between) {
                                words[i]->setSelected(true);
                                break;
                            } else
                                between = true;
                        }

                        if (between)
                            words[i]->setSelected(true);
                    }
                }
            }
        }
    } else if (!m_startPinch && (QGraphicsView::ScrollHandDrag == dragMode() || m_bHandAndLink)) {
        // 切换至手形样式后，也要求可是点击超链接文本并打开
        if (!magnifierOpened()) {
            if (Qt::NoButton == event->buttons()) { //鼠标处于非按压状态
                QPoint mousePos = event->pos();
                QPointF mousposF = event->pos();

                BrowserPage *page = getBrowserPageForPoint(mousposF);
                if (page) {
                    const Link &mlink = getLinkAtPoint(mousePos);
                    if (mlink.isValid()) {
                        //处理移动
                        if (!mlink.urlOrFileName.isEmpty()) { // 超链接地址为空时，不显示浮窗
                            QPoint showRealPos(QCursor::pos().x(), QCursor::pos().y() + 20);
                            m_tipsWidget->move(showRealPos);
                            m_tipsWidget->setText(mlink.urlOrFileName);
                            m_tipsWidget->show();
                        }
                        if (!m_bHandAndLink) {
                            m_bHandAndLink = true;
                            setDragMode(QGraphicsView::NoDrag);
                            setCursor(QCursor(Qt::PointingHandCursor)); //设为指针光标
                        }
                    } else {
                        // 离开超链接区域，还原为手形工具
                        if (m_bHandAndLink) {
                            m_bHandAndLink = false;
                            m_tipsWidget->hide();
                            setDragMode(QGraphicsView::ScrollHandDrag);
                        }
                    }
                }
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SheetBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_startPinch && (QGraphicsView::NoDrag == dragMode() || QGraphicsView::RubberBandDrag == dragMode())) {
        if (event->button() == Qt::LeftButton) {
            if (m_canTouchScreen) {
                m_scroller->handleInput(QScroller::InputRelease, event->pos(), static_cast<qint64>(event->timestamp()));
            } else {
                m_scroller->stop();
            }

            const QPointF &mousepoint = mapToScene(event->pos());

            m_selectEndPos = mousepoint;

            QPointF point = event->pos();

            BrowserPage *page = getBrowserPageForPoint(point);

            deepin_reader::Annotation *clickAnno = getClickAnnot(page, mousepoint);

            if (m_iconAnnot)
                clickAnno = m_iconAnnot;

            if (!m_selectIconAnnotation) {
                if (m_annotationInserting && (nullptr == m_iconAnnot)) {
                    if (clickAnno && clickAnno->type() == deepin_reader::Annotation::AText) {
                        updateAnnotation(clickAnno, clickAnno->contents());
                    } else {
                        clickAnno = addIconAnnotation(page, m_selectEndPos, "");
                    }

                    if (clickAnno)
                        showNoteEditWidget(clickAnno, mapToGlobal(event->pos()));

                    m_annotationInserting = false;
                } else {
                    if (clickAnno && m_selectWord == nullptr)
                        showNoteEditWidget(clickAnno, mapToGlobal(event->pos()));
                }
                m_selectEndPos = mapToScene(event->pos());
            } else {
                //存在选中的icon annotation
                if (m_lastSelectIconAnnotPage && m_iconAnnotationMovePos.isNull()) {
                    m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);

                    if (moveIconAnnot(m_lastSelectIconAnnotPage,  m_lastSelectIconAnnotPage->mapToScene(m_lastSelectIconAnnotPage->iconMovePos())))
                        emit sigOperaAnnotation(MSG_NOTE_MOVE, m_lastSelectIconAnnotPage->itemIndex(), clickAnno);

                } else if (clickAnno && m_lastSelectIconAnnotPage) {
                    showNoteEditWidget(clickAnno, mapToGlobal(event->pos()));
                }
            }

            m_selectPressedPos = QPointF();

            m_selectIndex = -1;

            m_selectIconAnnotation = false;

            // 选择工具时，点击高亮注释的超链接文本时，注释框优先响应
            // 手形工具时，点击高亮注释的超链接文本时，只响应超链接
            // 点击时鼠标按下和离开时坐标相差两个像素内可以打开链接
            if ((m_selectStartPos - m_selectEndPos).manhattanLength() <= 2 && (nullptr == clickAnno || m_bHandAndLink)) {
                // 跳转链接时隐藏tips
                if (nullptr != m_tipsWidget && !m_tipsWidget->isHidden()) {
                    m_tipsWidget->hide();
                }
                jump2Link(event->pos());
            }
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void SheetBrowser::focusOutEvent(QFocusEvent *event)
{
    // 鼠标为手形工具状态且在超链接处，此时失去焦点恢复成默认手形状态
    if (m_bHandAndLink) {
        m_bHandAndLink = false;
        setDragMode(QGraphicsView::ScrollHandDrag);
    }

    DGraphicsView::focusOutEvent(event);
}

void SheetBrowser::timerEvent(QTimerEvent *event)
{
    QGraphicsView::timerEvent(event);
    if (event->timerId() == m_repeatTimer.timerId()) {
        m_repeatTimer.stop();
        m_canTouchScreen = false;
    }
}

int SheetBrowser::allPages()
{
    return m_items.count();
}

int SheetBrowser::currentPage()
{
    if (m_currentPage >= 1)
        return m_currentPage;

    return currentScrollValueForPage();
}

int SheetBrowser::currentScrollValueForPage()
{
    int value = verticalScrollBar()->value();

    int index = 0;

    for (int i = 0; i < m_items.count(); ++i) {
        int y = 0;

        if (Dr::RotateBy0 == m_sheet->operation().rotation) {
            y = static_cast<int>(m_items[i]->y());
        } else if (Dr::RotateBy90 == m_sheet->operation().rotation) {
            y = static_cast<int>(m_items[i]->y());
        } else if (Dr::RotateBy180 == m_sheet->operation().rotation) {
            y = static_cast<int>(m_items[i]->y() - m_items[i]->boundingRect().height());
        } else if (Dr::RotateBy270 == m_sheet->operation().rotation) {
            y = static_cast<int>(m_items[i]->y() - m_items[i]->boundingRect().width());
        }

        if (y + m_items[i]->rect().height() >= value) {
            index = i;
            break;
        }
    }

    return index + 1;
}

void SheetBrowser::setCurrentPage(int page)
{
    if (page < 1 || page > allPages())
        return;

    m_bNeedNotifyCurPageChanged = false;

    if (Dr::RotateBy0 == m_sheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().x()));
        verticalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().y()));
    } else if (Dr::RotateBy90 == m_sheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().x() -  m_items.at(page - 1)->boundingRect().height()));
        verticalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().y()));
    } else if (Dr::RotateBy180 == m_sheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().x() - m_items.at(page - 1)->boundingRect().width()));
        verticalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().y() - m_items.at(page - 1)->boundingRect().height()));
    } else if (Dr::RotateBy270 == m_sheet->operation().rotation) {
        horizontalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().x()));
        verticalScrollBar()->setValue(static_cast<int>(m_items.at(page - 1)->pos().y() - m_items.at(page - 1)->boundingRect().width()));
    }

    m_bNeedNotifyCurPageChanged = true;

    curpageChanged(page);
}

bool SheetBrowser::getExistImage(int index, QImage &image, int width, int height)
{
    if (m_items.count() <= index)
        return false;

    image = m_items.at(index)->getCurrentImage(width, height);

    return !image.isNull();
}

BrowserPage *SheetBrowser::getBrowserPageForPoint(QPointF &viewPoint)
{
    BrowserPage *item = nullptr;

    QPoint ponit = viewPoint.toPoint();

    const QList<QGraphicsItem *> &itemlst = this->items(ponit);

    for (QGraphicsItem *itemIter : itemlst) {
        item = dynamic_cast<BrowserPage *>(itemIter);

        if (item != nullptr) {
            const QPointF &itemPoint = item->mapFromScene(mapToScene(ponit));

            if (item->contains(itemPoint)) {
                viewPoint = itemPoint;
                return item;
            }
        }
    }

    return nullptr;
}

Annotation *SheetBrowser::addIconAnnotation(BrowserPage *page, const QPointF &clickPoint, const QString &contents)
{
    Annotation *anno = nullptr;
    if (nullptr != page) {
        clearSelectIconAnnotAfterMenu();

        m_lastSelectIconAnnotPage = page;

        QRectF iconRect;

        bool isVaild = calcIconAnnotRect(page, clickPoint, iconRect);

        if (isVaild)
            anno = page->addIconAnnotation(iconRect, contents);
    }

    if (anno && !contents.isEmpty()) {
        m_selectIconAnnotation = true;

        if (m_lastSelectIconAnnotPage) {
            m_lastSelectIconAnnotPage->setSelectIconRect(true);
        }

        emit sigOperaAnnotation(MSG_NOTE_ADD, anno->page - 1, anno);
    }
    return anno;
}

void SheetBrowser::openMagnifier()
{
    if (nullptr == m_magnifierLabel) {
        m_magnifierLabel = new BrowserMagniFier(this);
    } else {
        m_magnifierLabel->raise();

        m_magnifierLabel->show();
    }

    setDragMode(QGraphicsView::NoDrag);

    setCursor(QCursor(Qt::BlankCursor));

    showMagnigierImage(this->mapFromGlobal(QCursor::pos()));
}

void SheetBrowser::closeMagnifier()
{
    if (nullptr != m_magnifierLabel) {
        m_magnifierLabel->hide();

        setCursor(QCursor(Qt::ArrowCursor));
        if (Dr::MouseShapeHand == m_sheet->operation().mouseShape) {
            setDragMode(QGraphicsView::ScrollHandDrag);
        } else if (Dr::MouseShapeNormal == m_sheet->operation().mouseShape) {
            setDragMode(QGraphicsView::NoDrag);
        }
    }
}

bool SheetBrowser::magnifierOpened()
{
    return (m_magnifierLabel && m_magnifierLabel->isVisible()) ;
}

qreal SheetBrowser::maxWidth()
{
    return m_maxWidth;
}

qreal SheetBrowser::maxHeight()
{
    return m_maxHeight;
}

void SheetBrowser::needBookmark(int index, bool state)
{
    emit sigNeedBookMark(index, state);
}

void SheetBrowser::dragEnterEvent(QDragEnterEvent *event)
{
    event->ignore();
}

void SheetBrowser::showEvent(QShowEvent *event)
{
    QTimer::singleShot(100, this, SLOT(onInit()));

    QGraphicsView::showEvent(event);
}

void SheetBrowser::handlePrepareSearch()
{
    //目前只有PDF开放搜索功能
    if (m_sheet->fileType() != Dr::FileType::PDF && m_sheet->fileType() != Dr::FileType::DOCX)
        return;

    if (m_findWidget == nullptr) {
        m_findWidget = new FindWidget(this->window());
        int windowY = this->mapTo(this->window(), QPoint(0, 0)).y();
        m_findWidget->setYOff(windowY);
        m_findWidget->setDocSheet(m_sheet);
    }

    m_findWidget->updatePosition();
    m_findWidget->setSearchEditFocus();
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if(m_findWidget && m_findWidget->isVisible()){
            m_findWidget->updatePosition();
        }
    });
#endif
}

void SheetBrowser::jumpToNextSearchResult()
{
    if (!m_isSearchResultNotEmpty) { //搜索到的结果为空，不搜索下一个
        return;
    }

    int size = m_items.size();
    for (int index = 0; index < size; index++) {
        int curIndex = m_searchCurIndex % size;
        BrowserPage *page = m_items.at(curIndex);
        int pageHighlightSize = page->searchHighlightRectSize();
        if (pageHighlightSize > 0) {
            if (m_lastFindPage && m_lastFindPage != page && m_changSearchFlag) {
                m_changSearchFlag = false;
                m_searchPageTextIndex = -1;
                m_lastFindPage->clearSelectSearchHighlightRects();
            }

            m_searchPageTextIndex++;
            if (m_searchPageTextIndex >= pageHighlightSize) {
                //当前页搜索完了，可以进行下一页
                m_changSearchFlag = true;
                m_lastFindPage = page;
                m_searchCurIndex = curIndex + 1;

                if (m_searchCurIndex >= size) { // 全部搜索完，从头搜索
                    m_searchCurIndex = 0;
                    m_searchPageTextIndex = -1;
                    index = -1;
                }

                continue;
            }

            QRectF pageHigRect = SearchResult::sectionBoundingRect(page->findSearchforIndex(m_searchPageTextIndex));
            pageHigRect = page->translateRect(pageHigRect);
            horizontalScrollBar()->setValue(static_cast<int>(page->pos().x() + pageHigRect.x()) - this->width() / 2);
            verticalScrollBar()->setValue(static_cast<int>(page->pos().y() + pageHigRect.y()) - this->height() / 2);
            break;
        } else {
            m_searchCurIndex++;
            if (m_searchCurIndex >= size) {
                m_searchCurIndex = 0;
                m_searchPageTextIndex = -1;
                m_lastFindPage = page;
                index = -1;
            }
        }
    }
}

void SheetBrowser::jumpToPrevSearchResult()
{
    if (!m_isSearchResultNotEmpty) { //搜索到的结果为空，不搜索上一个
        return;
    }

    int size = m_items.size();
    for (int index = 0; index < size; index++) {
        int curIndex = m_searchCurIndex % size;
        BrowserPage *page = m_items.at(curIndex);
        int pageHighlightSize = page->searchHighlightRectSize();
        if (pageHighlightSize > 0) {
            if (m_lastFindPage && m_lastFindPage != page && m_changSearchFlag) {
                m_changSearchFlag = false;
                m_searchPageTextIndex = pageHighlightSize;
                m_lastFindPage->clearSelectSearchHighlightRects();
            }

            m_searchPageTextIndex--;
            if (m_searchPageTextIndex < 0) {
                //当前页搜索完了，可以进行下一页
                m_changSearchFlag = true;
                m_lastFindPage = page;
                m_searchCurIndex = curIndex - 1;

                if (m_searchCurIndex < 0) { // 全部搜索完，从尾搜索
                    m_searchCurIndex = size - 1;
                    m_searchPageTextIndex = pageHighlightSize;
                    index = -1;
                }

                continue;
            }

            QRectF pageHigRect = SearchResult::sectionBoundingRect(page->findSearchforIndex(m_searchPageTextIndex));
            pageHigRect = page->translateRect(pageHigRect);
            horizontalScrollBar()->setValue(static_cast<int>(page->pos().x() + pageHigRect.x()) - this->width() / 2);
            verticalScrollBar()->setValue(static_cast<int>(page->pos().y() + pageHigRect.y()) - this->height() / 2);
            break;
        } else {
            m_searchCurIndex--;
            if (m_searchCurIndex < 0) {
                m_searchCurIndex = size - 1;
                m_searchPageTextIndex = pageHighlightSize;
                m_lastFindPage = page;
                index = -1;
            }
        }
    }
}

void SheetBrowser::handleSearchStart()
{
    m_searchCurIndex = 0;
    m_searchPageTextIndex = -1;
    m_isSearchResultNotEmpty = false;
    for (BrowserPage *page : m_items)
        page->clearSearchHighlightRects();
}

void SheetBrowser::handleSearchStop()
{
    m_searchCurIndex = 0;
    m_searchPageTextIndex = -1;
    m_isSearchResultNotEmpty = false;
    for (BrowserPage *page : m_items)
        page->clearSearchHighlightRects();
}

void SheetBrowser::handleSearchResultComming(const deepin_reader::SearchResult &res)
{
    if (res.page <= 0)
        return;

    if (res.page <= m_items.count())
        m_items[res.page - 1]->setSearchHighlightRectf(res.sections);

    //如果是第一个搜索结果来到，需要高亮第一个
    if (-1 == m_searchPageTextIndex)
        jumpToNextSearchResult();
}

void SheetBrowser::handleFindFinished(int searchcnt)
{
    if (m_findWidget)
        m_findWidget->setEditAlert(searchcnt == 0);
}

void SheetBrowser::curpageChanged(int curpage)
{
    if (m_currentPage != curpage) {
        m_currentPage = curpage;
        emit sigPageChanged(curpage);
    }
}

bool SheetBrowser::isLink(QPointF viewpoint)
{
    BrowserPage *page = getBrowserPageForPoint(viewpoint);

    if (nullptr == page)
        return false;

    viewpoint = translate2Local(viewpoint);

    //判断当前位置是否有link
    return m_sheet->renderer()->inLink(page->itemIndex(), viewpoint);
}

Link SheetBrowser::getLinkAtPoint(QPointF viewpoint)
{
    BrowserPage *page = getBrowserPageForPoint(viewpoint);

    if (nullptr == page)
        return Link();

    viewpoint = translate2Local(viewpoint);

    // 获取当前位置的link
    return m_sheet->renderer()->getLinkAtPoint(page->itemIndex(), viewpoint);
}

void SheetBrowser::setIconAnnotSelect(const bool select)
{
    if (m_lastSelectIconAnnotPage)
        m_lastSelectIconAnnotPage->setSelectIconRect(select);
    m_lastSelectIconAnnotPage = nullptr;
}

bool SheetBrowser::setDocTapGestrue(QPoint mousePos)
{
    QPointF viewpoint = mousePos;
    BrowserPage *page = getBrowserPageForPoint(viewpoint);
    if (nullptr == page)
        return true;

    BrowserAnnotation *browserAnno = page->getBrowserAnnotation(viewpoint);

    if ((browserAnno && browserAnno->annotationType() == deepin_reader::Annotation::AText) || magnifierOpened() || this->isLink(mousePos)) {
        return false;
    }

    return true;
}

void SheetBrowser::clearSelectIconAnnotAfterMenu()
{
    m_selectIconAnnotation = false;
    if (m_lastSelectIconAnnotPage) {
        m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);
        m_lastSelectIconAnnotPage->setSelectIconRect(false);
    }
}

bool SheetBrowser::jump2Link(QPointF point)
{
    BrowserPage *page = getBrowserPageForPoint(point);

    if (nullptr == page)
        return false;

    point = translate2Local(point);

    Link link = m_sheet->renderer()->getLinkAtPoint(page->itemIndex(), point);

    if (link.page > 0 && link.page <= allPages()) {
        jump2PagePos(m_items.at(link.page - 1), link.left, link.top);
        return true;
    } else if (!link.urlOrFileName.isEmpty()) {
        QString urlstr;
        if (link.urlOrFileName.startsWith(QLatin1String("file://"))) {
            urlstr = link.urlOrFileName.mid(7); //删除前缀"file://"
            if (!QFileInfo::exists(urlstr)) {
                qInfo() << urlstr << "文件不存在";
            }
        } else {
            urlstr = link.urlOrFileName;
        }

        SecurityDialog *sdialog = new SecurityDialog(link.urlOrFileName, this);
        if (DDialog::Accepted == sdialog->exec()) {
            const QUrl &url = QUrl(urlstr, QUrl::TolerantMode);
            QDesktopServices::openUrl(url);
        }
        return true;
    }

    return false;
}

void SheetBrowser::showMenu()
{
    BrowserMenu menu;
    QString selectWords = selectedWordsText();
    connect(&menu, &BrowserMenu::sigMenuHide, this, &SheetBrowser::onRemoveIconAnnotSelect);
    connect(&menu, &BrowserMenu::signalMenuItemClicked, [ & ](const QString & objectname) {
        if (objectname == "Copy") {
            Utils::setCurrentFilePath(m_sheet->openedFilePath());
            Utils::copyText(selectWords);
        } else if (objectname == "CopyAnnoText") {
            if (m_iconAnnot){
                Utils::setCurrentFilePath(m_sheet->openedFilePath());
                Utils::copyText(m_iconAnnot->contents());
            }
        } else if (objectname == "AddTextHighlight") {
            QPoint pointEnd;
            addHighLightAnnotation("", Utils::getCurHiglightColor(), pointEnd);
        } else if (objectname == "ChangeAnnotationColor") {
        } else if (objectname == "RemoveAnnotation") {
            if (m_iconAnnot) {
                m_selectIconAnnotation = false;
                if (m_lastSelectIconAnnotPage)
                    m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);
                if (m_sheet->removeAnnotation(m_iconAnnot))
                    m_iconAnnot = nullptr;
            }
        } else if (objectname == "AddAnnotationIcon") {
            if (m_iconAnnot)
                showNoteEditWidget(m_iconAnnot, this->mapToGlobal(this->mapFromScene(m_selectEndPos)));
        } else if (objectname == "AddBookmark") {
            m_sheet->setBookMark(this->currentPage() - 1, true);
        } else if (objectname == "RemoveHighlight") {
        } else if (objectname == "AddAnnotationHighlight") {
            QPoint pointEnd;
            deepin_reader::Annotation *addAnnot = nullptr;
            addAnnot = addHighLightAnnotation("", Utils::getCurHiglightColor(), pointEnd);
            if (addAnnot)
                showNoteEditWidget(addAnnot, pointEnd);
        } else if (objectname == "Search") {
            m_sheet->prepareSearch();
        } else if (objectname == "RemoveBookmark") {
            m_sheet->setBookMark(this->currentPage() - 1, false);
        } else if (objectname == "Fullscreen") {
            m_sheet->openFullScreen();
        } else if (objectname == "ExitFullscreen") {
            m_sheet->closeFullScreen();
        } else if (objectname == "SlideShow") {
            m_sheet->openSlide();
        } else if (objectname == "FirstPage") {
            this->emit sigNeedPageFirst();
        } else if (objectname == "PreviousPage") {
            this->emit sigNeedPagePrev();
        } else if (objectname == "NextPage") {
            this->emit sigNeedPageNext();
        } else if (objectname == "LastPage") {
            this->emit sigNeedPageLast();
        } else if (objectname == "RotateLeft") {
            m_sheet->rotateLeft();
        } else if (objectname == "RotateRight") {
            m_sheet->rotateRight();
        }  else if (objectname == "Print") {
            QTimer::singleShot(1, m_sheet, SLOT(onPopPrintDialog()));
        } else if (objectname == "DocumentInfo") {
            QTimer::singleShot(1, m_sheet, SLOT(onPopInfoDialog()));
        }
    });

    QPoint menuPoint(-1, -1);

    if (m_selectEndWord)
        menuPoint = this->mapFromScene(m_selectEndWord->mapToScene(m_selectEndWord->boundingRect().topRight()));
    if (!selectWords.isEmpty() && menuPoint.y() >= 0 && menuPoint.x() >= 0) {
        //选择文字
        menu.initActions(m_sheet, this->currentPage() - 1, SheetMenuType_e::DOC_MENU_SELECT_TEXT);
        menu.exec(this->mapToGlobal(menuPoint));
    } else  if (m_iconAnnot) {
        //文字注释(图标)
        if (m_lastSelectIconAnnotPage)
            m_lastSelectIconAnnotPage->setDrawMoveIconRect(false);
        menuPoint = this->mapFromScene(m_selectEndPos);
        menu.initActions(m_sheet, this->currentPage() - 1, SheetMenuType_e::DOC_MENU_ANNO_ICON, m_iconAnnot->contents());
        menu.exec(this->mapToGlobal(menuPoint));
    } else {
        //默认
        menu.initActions(m_sheet, this->currentPage() - 1, SheetMenuType_e::DOC_MENU_KEY);
        QPoint parentPos = m_sheet->mapToGlobal(m_sheet->pos());
        QPoint showPos(parentPos.x() + m_sheet->width() / 2, parentPos.y() + m_sheet->height() / 2);
        menu.exec(showPos);
    }

    clearSelectIconAnnotAfterMenu();
}

QList<BrowserPage *> SheetBrowser::pages()
{
    return m_items;
}

void SheetBrowser::showMagnigierImage(const QPoint &point)
{
    QPoint magnifierPos = point;
    if (point.y() < 122) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (122 - point.y()));
        magnifierPos.setY(122);
    }

    if (point.x() < 122) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (122 - point.x()));
        magnifierPos.setX(122);
    }

    if (point.y() > (this->size().height() - 122) && (this->size().height() - 122 > 0)) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + (point.y() - (this->size().height() - 122)));
        magnifierPos.setY(this->size().height() - 122);
    }

    if (point.x() > (this->size().width() - 122) && (this->size().width() - 122 > 0)) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (point.x() - (this->size().width() - 122)));
        magnifierPos.setX(this->size().width() - 122);
    }

    m_magnifierLabel->showMagnigierImage(point, magnifierPos, m_lastScaleFactor);
}


void SheetBrowser::moveScrollBar(const QPoint &point)
{
    //鼠标靠近显示区域的边距
    int margin = 20;
    if (point.y() < margin) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (margin - point.y()));
    }

    if (point.x() < margin) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (margin - point.x()));
    }

    if (point.y() > (this->size().height() - margin) && (this->size().height() - margin > 0)) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + (point.y() - (this->size().height() - margin)));
    }

    if (point.x() > (this->size().width() - margin) && (this->size().width() - margin > 0)) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (point.x() - (this->size().width() - margin)));
    }

}

QPointF SheetBrowser::getAnnotPosInPage(const QPointF &pos, BrowserPage *page)
{
    QPointF newPos = pos;

    if (newPos.x() < 15 * page->scaleFactor())
        newPos.setX(15 * page->scaleFactor());

    if (newPos.y() < 15 * page->scaleFactor())
        newPos.setY(15 * page->scaleFactor());

    if (newPos.x() > page->rect().width() - 15 * page->scaleFactor())
        newPos.setX(page->rect().width() - 15 * page->scaleFactor());

    if (newPos.y() > page->rect().height() - 15 * page->scaleFactor())
        newPos.setY(page->rect().height() - 15 * page->scaleFactor());

    return newPos;
}

void SheetBrowser::setIsSearchResultNotEmpty(bool isSearchResultNotEmpty)
{
    m_isSearchResultNotEmpty = isSearchResultNotEmpty;
    if (!m_findWidget.isNull()) {
        m_findWidget->setEditAlert(false);
    }
}

TextEditShadowWidget *SheetBrowser::getNoteEditWidget() const
{
    return m_noteEditWidget;
}
