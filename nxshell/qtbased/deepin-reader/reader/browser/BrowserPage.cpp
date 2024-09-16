// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserPage.h"
#include "Model.h"
#include "PageRenderThread.h"
#include "SheetBrowser.h"
#include "BrowserWord.h"
#include "BrowserAnnotation.h"
#include "Application.h"
#include "Utils.h"
#include "Global.h"
#include "SheetRenderer.h"

#include <DApplicationHelper>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QTime>
#include <QMutexLocker>
#include <QTimer>
#include <QUuid>
#include <QDesktopServices>
#include <QDebug>
#include <QMutexLocker>

const int ICON_SIZE = 23;

BrowserPage::BrowserPage(SheetBrowser *parent, int index, DocSheet *sheet) :
    QGraphicsItem(), m_sheet(sheet), m_parent(parent), m_index(index)
{
    setAcceptHoverEvents(true);

    setFlag(QGraphicsItem::ItemIsPanel);

    m_originSizeF = sheet->renderer()->getPageSize(index);
}

BrowserPage::~BrowserPage()
{
    PageRenderThread::clearImageTasks(m_sheet, this);

    qDeleteAll(m_annotations);

    qDeleteAll(m_annotationItems);

    qDeleteAll(m_words);

}

QRectF BrowserPage::boundingRect() const
{
    return QRectF(0, 0, m_originSizeF.width() * m_scaleFactor, m_originSizeF.height() * m_scaleFactor);
}

QRectF BrowserPage::rect()
{
    switch (m_rotation) {
    case Dr::RotateBy90:
    case Dr::RotateBy270:
        return QRectF(0, 0, static_cast<double>(m_originSizeF.height() * m_scaleFactor), static_cast<double>(m_originSizeF.width() * m_scaleFactor));
    default: break;
    }

    return QRectF(0, 0, static_cast<double>(m_originSizeF.width() * m_scaleFactor), static_cast<double>(m_originSizeF.height() * m_scaleFactor));
}

qreal BrowserPage::scaleFactor()
{
    return m_scaleFactor;
}

QRectF BrowserPage::bookmarkRect()
{
    return QRectF(boundingRect().width() - 40, 1, 39, 39);
}

QRectF BrowserPage::bookmarkMouseRect()
{
    return QRectF(boundingRect().width() - 27, 10, 14, 20);
}

void BrowserPage::setBookmark(const bool &hasBookmark)
{
    m_bookmark = hasBookmark;

    if (hasBookmark)
        m_bookmarkState = 3;
    else
        m_bookmarkState = 0;

    update();
}

void BrowserPage::updateBookmarkState()
{
    if (m_bookmark)
        m_bookmarkState = 3;
    else
        m_bookmarkState = 0;

    update();
}

void BrowserPage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(option)

    if (!qFuzzyCompare(m_renderPixmapScaleFactor, m_scaleFactor)) {
        render(m_scaleFactor, m_rotation);
    }

    if (!m_viewportRendered && !m_pixmapHasRendered && isBigDoc())
        renderViewPort();

    painter->drawPixmap(0, 0, m_renderPixmap);  //m_renderPixmap的大小存在系统缩放，可能不等于option->rect()，需要按坐标绘制

    if (1 == m_bookmarkState)
        painter->drawPixmap(static_cast<int>(bookmarkRect().x()), static_cast<int>(bookmarkRect().y()), QIcon::fromTheme("dr_bookmark_hover").pixmap(QSize(39, 39)));
    if (2 == m_bookmarkState)
        painter->drawPixmap(static_cast<int>(bookmarkRect().x()), static_cast<int>(bookmarkRect().y()), QIcon::fromTheme("dr_bookmark_pressed").pixmap(QSize(39, 39)));
    if (3 == m_bookmarkState)
        painter->drawPixmap(static_cast<int>(bookmarkRect().x()), static_cast<int>(bookmarkRect().y()), QIcon::fromTheme("dr_bookmark_checked").pixmap(QSize(39, 39)));

    painter->setPen(Qt::NoPen);

    painter->setBrush(QColor(238, 220, 0, 100));

    //search
    for (const PageSection &section : m_searchLightrectLst) {
        for (const PageLine &line : section) {
            painter->drawRect(getNorotateRect(line.rect));
        }
    }

    //search and Select
    painter->setBrush(QColor(59, 148, 1, 100));
    for (const PageLine &line : m_searchSelectLighRectf) {
        painter->drawRect(getNorotateRect(line.rect));
    }

    if (m_drawMoveIconRect) {
        QPen pen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
        painter->setPen(pen);

        qreal iconWidth = (nullptr != m_lastClickIconAnnotationItem && m_annotationItems.contains(m_lastClickIconAnnotationItem)) ? (m_lastClickIconAnnotationItem->boundingRect().width()) : ICON_SIZE;
        qreal iconHeight = (nullptr != m_lastClickIconAnnotationItem && m_annotationItems.contains(m_lastClickIconAnnotationItem)) ? (m_lastClickIconAnnotationItem->boundingRect().height()) : ICON_SIZE;

        int x = static_cast<int>(m_drawMoveIconPoint.x() - iconWidth / 2.0);
        int y = static_cast<int>(m_drawMoveIconPoint.y() - iconHeight / 2.0);

        QRect rect = QRect(x, y, static_cast<int>(iconHeight), static_cast<int>(iconHeight));

        painter->drawRect(rect);
    }

}

void BrowserPage::render(const double &scaleFactor, const Dr::Rotation &rotation, const bool &renderLater, const bool &force)
{
    if (!force && renderLater && qFuzzyCompare(scaleFactor, m_scaleFactor) && rotation == m_rotation)
        return;

    if (m_lastClickIconAnnotationItem && m_annotationItems.contains(m_lastClickIconAnnotationItem))
        m_lastClickIconAnnotationItem->setScaleFactor(scaleFactor);

    m_scaleFactor = scaleFactor;

    if (m_rotation != rotation) {
        m_rotation = rotation;
        if (Dr::RotateBy0 == m_rotation)
            this->setRotation(0);
        else if (Dr::RotateBy90 == m_rotation)
            this->setRotation(90);
        else if (Dr::RotateBy180 == m_rotation)
            this->setRotation(180);
        else if (Dr::RotateBy270 == m_rotation)
            this->setRotation(270);
    }

    if (!renderLater && !qFuzzyCompare(m_renderPixmapScaleFactor, m_scaleFactor)) {
        m_renderPixmapScaleFactor = m_scaleFactor;

        if (m_pixmap.isNull()) {
            m_pixmap = QPixmap(static_cast<int>(boundingRect().width() * dApp->devicePixelRatio()),
                               static_cast<int>(boundingRect().height() * dApp->devicePixelRatio()));
            m_pixmap.fill(Qt::white);
            m_renderPixmap = m_pixmap;
            m_renderPixmap.setDevicePixelRatio(dApp->devicePixelRatio());
        } else {
            m_renderPixmap = m_pixmap.scaled(static_cast<int>(boundingRect().width() * dApp->devicePixelRatio()),
                                             static_cast<int>(boundingRect().height() * dApp->devicePixelRatio()));
            m_renderPixmap.setDevicePixelRatio(dApp->devicePixelRatio());
        }

        ++m_pixmapId;

        PageRenderThread::clearImageTasks(m_sheet, this, m_pixmapId);

        if (isBigDoc()) {
            DocPageBigImageTask task;

            task.sheet = m_sheet;

            task.page = this;

            task.pixmapId = m_pixmapId;

            task.rect = QRect(0, 0,
                              static_cast<int>(boundingRect().width() * dApp->devicePixelRatio()),
                              static_cast<int>(boundingRect().height() * dApp->devicePixelRatio()));

            PageRenderThread::appendTask(task);
        } else {
            DocPageNormalImageTask task;

            task.sheet = m_sheet;

            task.page = this;

            task.pixmapId = m_pixmapId;

            task.rect = QRect(0, 0,
                              static_cast<int>(boundingRect().width() * dApp->devicePixelRatio()),
                              static_cast<int>(boundingRect().height() * dApp->devicePixelRatio()));

            PageRenderThread::appendTask(task);
        }

        if (!m_hasLoadedAnnotation || !m_annotatinIsRendering) {
            m_annotatinIsRendering = true;

            DocPageAnnotationTask task;

            task.sheet = m_sheet;

            task.page = this;

            PageRenderThread::appendTask(task);
        }
    }

    if (!m_parent->property("pinchgetsturing").toBool()) {
        //在触摸屏捏合状态时,不要进行word缩放,不然会被卡的
        scaleWords();
        scaleAnnots();
    }

    update();
}

void BrowserPage::renderRect(const QRectF &rect)
{
    if (nullptr == m_parent)
        return;

    QRect validRect = boundingRect().intersected(rect).toRect();

    DocPageSliceImageTask task;

    task.sheet = m_sheet;

    task.page = this;

    task.pixmapId = m_pixmapId;

    task.whole = QRect(0, 0,
                       static_cast<int>(boundingRect().width() * dApp->devicePixelRatio()),
                       static_cast<int>(boundingRect().height() * dApp->devicePixelRatio()));

    task.slice = QRect(static_cast<int>(validRect.x() * dApp->devicePixelRatio()),
                       static_cast<int>(validRect.y() * dApp->devicePixelRatio()),
                       static_cast<int>(validRect.width() * dApp->devicePixelRatio()),
                       static_cast<int>(validRect.height() * dApp->devicePixelRatio()));

    PageRenderThread::appendTask(task);
}

void BrowserPage::renderViewPort()
{
    if (nullptr == m_parent)
        return;

    QRect viewPortRect = QRect(0, 0, m_parent->size().width(), m_parent->size().height());

    QRectF visibleSceneRectF = m_parent->mapToScene(viewPortRect).boundingRect();

    QRectF intersectedRectF = this->mapToScene(this->boundingRect()).boundingRect().intersected(visibleSceneRectF);

    //如果不在当前可视范围则不加载 强制也没用
    if (intersectedRectF.height() <= 0 && intersectedRectF.width() <= 0)
        return;

    QRectF viewRenderRectF = mapFromScene(intersectedRectF).boundingRect();

    QRect viewRenderRect = QRect(static_cast<int>(viewRenderRectF.x()), static_cast<int>(viewRenderRectF.y()),
                                 static_cast<int>(viewRenderRectF.width()), static_cast<int>(viewRenderRectF.height()));

    //扩大加载的视图窗口范围 防止小范围的拖动
    int expand = 200;

    viewRenderRect.setX(viewRenderRect.x() - expand < 0 ? 0 : viewRenderRect.x() - expand);

    viewRenderRect.setY(viewRenderRect.y() - expand < 0 ? 0 : viewRenderRect.y() - expand);

    viewRenderRect.setWidth(viewRenderRect.x() + viewRenderRect.width() + expand * 2 > boundingRect().width() ? viewRenderRect.width() :  viewRenderRect.width() + expand * 2);

    viewRenderRect.setHeight(viewRenderRect.y() + viewRenderRect.height() + expand * 2 > boundingRect().height() ? viewRenderRect.height() :  viewRenderRect.height() + expand * 2);

    renderRect(viewRenderRect);

    m_viewportRendered = true;
}

void BrowserPage::handleRenderFinished(const int &pixmapId, const QPixmap &pixmap, const QRect &slice)
{
    if (m_pixmapId != pixmapId)
        return;

    if (!slice.isValid()) { //不是切片，整体更新
        m_pixmapHasRendered = true;
        m_pixmap = pixmap;
    } else { //局部
        QPainter painter(&m_pixmap);
        painter.drawPixmap(slice, pixmap);
    }

    m_renderPixmap = m_pixmap;

    m_renderPixmap.setDevicePixelRatio(dApp->devicePixelRatio());

    update();
}

void BrowserPage::handleWordLoaded(const QList<Word> &words)
{
    m_wordIsRendering = false;

    if (!m_wordNeeded)
        return;

    if (m_wordHasRendered)
        return;

    m_wordHasRendered = true;

    for (int i = 0; i < words.count(); ++i) {
        BrowserWord *word = new BrowserWord(this, words[i]);

        word->setSelectable(m_wordSelectable);

        m_words.append(word);
    }

    scaleWords(true);
}

void BrowserPage::handleAnnotationLoaded(const QList<Annotation *> &annots)
{
    m_annotatinIsRendering = false;

    //如果已经加载了，则过滤本次加载 (存在不通过线程加载的情况)
    if (m_hasLoadedAnnotation) {
        qDeleteAll(annots);
        return;
    }

    //在reload之前将上一次选中去掉,避免操作野指针
    if (m_lastClickIconAnnotationItem && m_annotationItems.contains(m_lastClickIconAnnotationItem)) {
        m_lastClickIconAnnotationItem->setDrawSelectRect(false);
        m_lastClickIconAnnotationItem = nullptr;
    }

    qDeleteAll(m_annotations);
    m_annotations.clear();

    qDeleteAll(m_annotationItems);
    m_annotationItems.clear();

    m_annotations = annots;

    for (int i = 0; i < m_annotations.count(); ++i) {
        m_annotations[i]->page = m_index + 1;
        foreach (const QRectF &rect, m_annotations[i]->boundary()) {
            BrowserAnnotation *annotationItem = new BrowserAnnotation(this, rect, m_annotations[i], m_scaleFactor);
            m_annotationItems.append(annotationItem);
        }
    }

    m_hasLoadedAnnotation = true;
}

QImage BrowserPage::getCurrentImage(int width, int height)
{
    if (m_pixmap.isNull())
        return QImage();

    //获取图片比原图还大,就不需要原图了
    if (qMin(width, height) > qMax(m_pixmap.width(), m_pixmap.height()))
        return QImage();

    QImage image = m_pixmap.toImage().scaled(static_cast<int>(width), static_cast<int>(height), Qt::KeepAspectRatio);

    return image;
}

QImage BrowserPage::getImagePoint(double scaleFactor, QPoint point)
{
    QTransform transform;

    transform.rotate(m_rotation * 90);

    int ss = static_cast<int>(122 * scaleFactor / m_scaleFactor);

    QRect rect = QRect(qRound(point.x() * scaleFactor / m_scaleFactor - ss / 2.0),
                       qRound(point.y() * scaleFactor / m_scaleFactor - ss / 2.0), ss, ss);

    return m_sheet->renderer()->getImage(itemIndex(),
                                         static_cast<int>(m_originSizeF.width() * scaleFactor),
                                         static_cast<int>(m_originSizeF.height() * scaleFactor),
                                         rect)
           .transformed(transform, Qt::SmoothTransformation);
}

QImage BrowserPage::getCurImagePoint(QPointF point)
{
    int ds = static_cast<int>(122 * dApp->devicePixelRatio());
    QTransform transform;
    transform.rotate(m_rotation * 90);
    const QImage &image = Utils::copyImage(m_renderPixmap.toImage(), qRound(point.x() * dApp->devicePixelRatio() - ds / 2.0), qRound(point.y() * dApp->devicePixelRatio()  - ds / 2.0), ds, ds).transformed(transform, Qt::SmoothTransformation);
    return image;
}

int BrowserPage::itemIndex()
{
    return m_index;
}

QString BrowserPage::selectedWords()
{
    QString text;
    foreach (BrowserWord *word, m_words) {
        if (word->isSelected()) {
            text += word->text();
        }
    }

    return text;
}

void BrowserPage::setWordSelectable(bool selectable)
{
    m_wordSelectable = selectable;
    foreach (BrowserWord *word, m_words) {
        word->setSelectable(selectable);
    }
}

void BrowserPage::loadWords()
{
    m_wordNeeded = true;

    if (m_wordIsRendering)
        return;

    if (m_wordHasRendered) {
        //如果已经加载则取消隐藏和改变大小

        if (m_words.count() <= 0)
            return;

        prepareGeometryChange();

        if (!qFuzzyCompare(m_wordScaleFactor, m_scaleFactor)) {
            m_wordScaleFactor = m_scaleFactor;
            foreach (BrowserWord *word, m_words) {
                word->setScaleFactor(m_scaleFactor);
            }
        }

        return;
    }

    DocPageWordTask task;

    task.sheet = m_sheet;

    task.page = this;

    PageRenderThread::appendTask(task);

    m_wordHasRendered = false;

    m_wordIsRendering = true;
}

void BrowserPage::clearPixmap()
{
    if (m_renderPixmapScaleFactor < -0.0001)
        return;

    m_pixmap = QPixmap();

    m_renderPixmap = m_pixmap;

    ++m_pixmapId;

    m_pixmapHasRendered = false;

    m_viewportRendered  = false;

    m_renderPixmapScaleFactor = -1;

    PageRenderThread::clearImageTasks(m_sheet, this);
}

void BrowserPage::clearWords()
{
    if (!m_wordNeeded)
        return;

    if (!m_wordHasRendered)
        return;

    foreach (BrowserWord *word, m_words) {
        if (word->isSelected())
            return;
    }

    QList<BrowserWord *> t_word = m_words;
    m_words.clear();

    m_wordHasRendered = false;
    m_wordNeeded = false;

    foreach (BrowserWord *word, t_word) {
        word->setParentItem(nullptr);
        scene()->removeItem(word);
        delete word;
    }
}

void BrowserPage::scaleAnnots(bool force)
{
    if (!m_annotatinIsRendering || m_annotationItems.count() <= 0)
        return;

    prepareGeometryChange();

    if (force || !qFuzzyCompare(m_annotScaleFactor, m_scaleFactor)) {
        m_wordScaleFactor = m_scaleFactor;
        foreach (BrowserAnnotation *annot, m_annotationItems) {
            annot->setScaleFactor(m_scaleFactor);
        }
    }
}

void BrowserPage::scaleWords(bool force)
{
    if (!m_wordHasRendered || m_words.count() <= 0)
        return;

    prepareGeometryChange();

    if (force || !qFuzzyCompare(m_wordScaleFactor, m_scaleFactor)) {
        m_wordScaleFactor = m_scaleFactor;
        foreach (BrowserWord *word, m_words) {
            word->setScaleFactor(m_scaleFactor);
        }
    }
}

QList<deepin_reader::Annotation *> BrowserPage::annotations()
{
    if (!m_hasLoadedAnnotation) {
        handleAnnotationLoaded(m_sheet->renderer()->getAnnotations(itemIndex()));
    }

    return m_annotations;
}

bool BrowserPage::updateAnnotation(deepin_reader::Annotation *annotation, const QString &text, const QColor &color)
{
    if (nullptr == annotation)
        return false;

    if (!m_annotations.contains(annotation))
        return false;

    if (!m_sheet->renderer()->updateAnnotation(itemIndex(), annotation, text, color))
        return false;

    QRectF renderBoundary;
    const QList<QRectF> &annoBoundaries = annotation->boundary();
    for (int i = 0; i < annoBoundaries.size(); i++) {
        renderBoundary = renderBoundary | annoBoundaries.at(i);
    }

    renderBoundary.adjust(-10, -10, 10, 10);
    renderRect(QRectF(renderBoundary.x() * m_scaleFactor,
                      renderBoundary.y() * m_scaleFactor,
                      renderBoundary.width() * m_scaleFactor,
                      renderBoundary.height() * m_scaleFactor));

    m_sheet->handlePageModified(m_index);

    return true;
}

Annotation *BrowserPage::addHighlightAnnotation(QString text, QColor color)
{
    Annotation *highLightAnnot = nullptr;
    QList<QRectF> boundaries;

    //加载文档文字无旋转情况下的文字(即旋转0度时的所有文字)
    const QList<deepin_reader::Word> &twords = m_sheet->renderer()->getWords(itemIndex());

    if (m_words.size() != twords.size())
        return nullptr;

    int wordCnt = twords.size();

    QRectF selectBoundRectF;
    bool bresetSelectRect = true;

    for (int index = 0; index < wordCnt; index++) {
        if (m_words.at(index) && m_words.at(index)->isSelected()) {
            m_words.at(index)->setSelected(false);

            const QRectF &textRectf = twords[index].wordBoundingRect();
            if (bresetSelectRect) {
                bresetSelectRect = false;
                selectBoundRectF = textRectf;
            } else {
                if (qFuzzyCompare(selectBoundRectF.height(), textRectf.height()) && qFuzzyCompare(selectBoundRectF.y(), textRectf.y()) && selectBoundRectF.right() > (textRectf.x() - 1)) {
                    selectBoundRectF = selectBoundRectF.united(textRectf);
                } else if (qFuzzyCompare(selectBoundRectF.right(), textRectf.x())) {
                    selectBoundRectF = selectBoundRectF.united(textRectf);
                } else {
                    if (selectBoundRectF.width() > 0.00001 && selectBoundRectF.height() > 0.00001)
                        boundaries << selectBoundRectF;
                    selectBoundRectF = textRectf;
                }
            }
        }
    }

    if (selectBoundRectF.width() > 0.00001 && selectBoundRectF.height() > 0.00001)
        boundaries << selectBoundRectF;

    if (boundaries.count() > 0) {
        //需要保证已经加载注释
        if (!m_hasLoadedAnnotation) {
            handleAnnotationLoaded(m_sheet->renderer()->getAnnotations(itemIndex()));
        }

        highLightAnnot = m_sheet->renderer()->addHighlightAnnotation(itemIndex(), boundaries, text, color);
        if (highLightAnnot == nullptr)
            return nullptr;

        highLightAnnot->page = m_index + 1;
        m_annotations.append(highLightAnnot);

        foreach (const QRectF &rect, highLightAnnot->boundary()) {
            BrowserAnnotation *annotationItem = new BrowserAnnotation(this, rect, highLightAnnot, m_scaleFactor);
            m_annotationItems.append(annotationItem);
        }

        QRectF renderBoundary;
        for (int i = 0; i < boundaries.size(); i++) {
            renderBoundary = renderBoundary | boundaries.at(i);
        }

        renderBoundary.adjust(-10, -10, 10, 10);
        renderRect(QRectF(renderBoundary.x() * m_scaleFactor, renderBoundary.y() * m_scaleFactor,
                          renderBoundary.width() * m_scaleFactor, renderBoundary.height() * m_scaleFactor));
    }

    m_sheet->handlePageModified(m_index);

    return highLightAnnot;
}

bool BrowserPage::hasAnnotation(deepin_reader::Annotation *annotation)
{
    return m_annotations.contains(annotation);
}

void BrowserPage::setSelectIconRect(const bool draw, Annotation *iconAnnot)
{
    QList<QRectF> rectList;

    if (iconAnnot) {
        foreach (BrowserAnnotation *annotation, m_annotationItems) {
            if (annotation && annotation->isSame(iconAnnot)) {
                m_lastClickIconAnnotationItem = annotation;
                m_lastClickIconAnnotationItem->setScaleFactor(m_scaleFactor);
                if (iconAnnot->type() == deepin_reader::Annotation::AText)
                    m_lastClickIconAnnotationItem->setDrawSelectRect(draw);
            }
        }
    } else {
        if (m_lastClickIconAnnotationItem && m_annotationItems.contains(m_lastClickIconAnnotationItem))
            m_lastClickIconAnnotationItem->setDrawSelectRect(draw);
    }
}

void BrowserPage::setDrawMoveIconRect(const bool draw)
{
    m_drawMoveIconRect = draw;

    update();
}

QPointF BrowserPage::iconMovePos()
{
    return m_drawMoveIconPoint;
}

void BrowserPage::setIconMovePos(const QPointF movePoint)
{
    m_drawMoveIconPoint = movePoint;

    update();
}

bool BrowserPage::moveIconAnnotation(const QRectF &moveRect)
{
    if (nullptr == m_lastClickIconAnnotationItem)
        return false;

    QList<QRectF> annoBoundaries;

    m_annotationItems.removeAll(m_lastClickIconAnnotationItem);
    annoBoundaries << m_lastClickIconAnnotationItem->annotation()->boundary();
    annoBoundaries << moveRect;
    Annotation *annot = m_sheet->renderer()->moveIconAnnotation(itemIndex(), m_lastClickIconAnnotationItem->annotation(), moveRect);

    if (annot && m_annotations.contains(annot)) {
        delete m_lastClickIconAnnotationItem;
        m_lastClickIconAnnotationItem = nullptr;
        annot->page = m_index + 1;
        foreach (const QRectF &rect, annot->boundary()) {
            BrowserAnnotation *annotationItem = new BrowserAnnotation(this, rect, annot, m_scaleFactor);
            m_annotationItems.append(annotationItem);
            m_lastClickIconAnnotationItem = annotationItem;
        }

        if (annot->type() == 1) {
            if (m_lastClickIconAnnotationItem) {
                m_lastClickIconAnnotationItem->setScaleFactor(m_scaleFactor);
                m_lastClickIconAnnotationItem->setDrawSelectRect(true);
            }
        }

        QRectF renderBoundary;
        for (int i = 0; i < annoBoundaries.size(); i++) {
            renderBoundary = renderBoundary | annoBoundaries.at(i);
        }

        renderBoundary.adjust(-10, -10, 10, 10);
        renderRect(QRectF(renderBoundary.x() * m_scaleFactor, renderBoundary.y() * m_scaleFactor, renderBoundary.width() * m_scaleFactor, renderBoundary.height() * m_scaleFactor));
    }

    m_sheet->handlePageModified(m_index);

    return true;
}

bool BrowserPage::removeAllAnnotation()
{
    //未加载注释时则直接加载 无需添加图元
    if (!m_hasLoadedAnnotation) {
        m_annotations = m_sheet->renderer()->getAnnotations(itemIndex());
        m_hasLoadedAnnotation = true;
    }

    m_lastClickIconAnnotationItem = nullptr;

    if (m_annotations.isEmpty())
        return false;

    //所有的注释区域 需要被重新加载界面
    QList<QRectF> annoBoundaries;

    for (int index = 0; index < m_annotations.size(); index++) {
        deepin_reader::Annotation *annota = m_annotations.at(index);

        if (!m_annotations.contains(annota) || (annota && annota->contents().isEmpty()))
            continue;

        annoBoundaries << annota->boundary();

        if (!m_sheet->renderer()->removeAnnotation(itemIndex(), annota))
            continue;

        m_annotations.removeAt(index);

        index--;

        foreach (BrowserAnnotation *annotation, m_annotationItems) {
            if (annotation && annotation->isSame(annota)) {
                if (m_lastClickIconAnnotationItem == annotation)
                    m_lastClickIconAnnotationItem = nullptr;
                m_annotationItems.removeAll(annotation);
                delete annotation;
                annotation = nullptr;
            }
        }
    }

    QRectF renderBoundary;

    for (int i = 0; i < annoBoundaries.size(); i++) {
        renderBoundary = renderBoundary | annoBoundaries.at(i);
    }

    renderBoundary.adjust(-10, -10, 10, 10);

    renderRect(QRectF(renderBoundary.x() * m_scaleFactor, renderBoundary.y() * m_scaleFactor, renderBoundary.width() * m_scaleFactor, renderBoundary.height() * m_scaleFactor));

    m_sheet->handlePageModified(m_index);

    return true;
}

void BrowserPage::setPageBookMark(const QPointF clickPoint)
{
    if (bookmarkMouseRect().contains(clickPoint)) {
        m_bookmarkState = 2;
        if (nullptr != m_parent) {
            m_parent->needBookmark(m_index, !m_bookmark);
            if (!m_bookmark && bookmarkMouseRect().contains(clickPoint))
                m_bookmarkState = 1;
            else if (m_bookmark)
                m_bookmarkState = 3;
            else
                m_bookmarkState = 0;
        }
        update();
    }
}

QPointF BrowserPage::getTopLeftPos()
{
    QPointF p;
    switch (m_rotation) {
    default:
    case Dr::RotateBy0:
        p = pos();
        break;
    case Dr::RotateBy270:
        p.setX(pos().x());
        p.setY(pos().y() - rect().height());
        break;
    case Dr::RotateBy180:
        p.setX(pos().x() - rect().width());
        p.setY(pos().y() - rect().height());
        break;
    case Dr::RotateBy90:
        p.setX(pos().x() - rect().width());
        p.setY(pos().y());
        break;
    }
    return p;
}

bool BrowserPage::removeAnnotation(deepin_reader::Annotation *annota)
{
    if (nullptr == annota)
        return false;

    if (!m_annotations.contains(annota))
        return false;

    m_annotations.removeAll(annota);

    const QList<QRectF> &annoBoundaries = annota->boundary();

    if (!m_sheet->renderer()->removeAnnotation(itemIndex(), annota))
        return false;

    foreach (BrowserAnnotation *annotation, m_annotationItems) {
        if (annotation && annotation->isSame(annota)) {
            if (m_lastClickIconAnnotationItem == annotation)
                m_lastClickIconAnnotationItem = nullptr;
            m_annotationItems.removeAll(annotation);
            delete annotation;
            annotation = nullptr;
        }
    }

    QRectF renderBoundary;

    for (int i = 0; i < annoBoundaries.size(); i++) {
        renderBoundary = renderBoundary | annoBoundaries.at(i);
    }

    renderBoundary.adjust(-10, -10, 10, 10);

    renderRect(QRectF(renderBoundary.x() * m_scaleFactor, renderBoundary.y() * m_scaleFactor, renderBoundary.width() * m_scaleFactor, renderBoundary.height() * m_scaleFactor));

    m_sheet->handlePageModified(m_index);

    return true;
}

Annotation *BrowserPage::addIconAnnotation(const QRectF &rect, const QString &text)
{
    Annotation *annot = m_sheet->renderer()->addIconAnnotation(itemIndex(), rect, text);

    if (annot) {
        annot->page = m_index + 1;

        m_annotations.append(annot);

        foreach (const QRectF &arect, annot->boundary()) {
            BrowserAnnotation *annotationItem = new BrowserAnnotation(this, arect, annot, m_scaleFactor);
            m_annotationItems.append(annotationItem);
            m_lastClickIconAnnotationItem = annotationItem;
        }

        if (annot->type() == deepin_reader::Annotation::AText) {
            if (m_lastClickIconAnnotationItem) {
                m_lastClickIconAnnotationItem->setScaleFactor(m_scaleFactor);
                m_lastClickIconAnnotationItem->setDrawSelectRect(true);
            }
        }

        QRectF renderBoundary;

        const QList<QRectF> &annoBoundaries = annot->boundary();

        for (int i = 0; i < annoBoundaries.size(); i++) {
            renderBoundary = renderBoundary | annoBoundaries.at(i);
        }

        renderBoundary.adjust(-10, -10, 10, 10);

        renderRect(QRectF(renderBoundary.x() * m_scaleFactor, renderBoundary.y() * m_scaleFactor, renderBoundary.width() * m_scaleFactor, renderBoundary.height() * m_scaleFactor));

        m_sheet->handlePageModified(m_index);
    }

    return annot;
}

bool BrowserPage::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneHoverMove) {
        QGraphicsSceneHoverEvent *moveevent = dynamic_cast<QGraphicsSceneHoverEvent *>(event);
        if (!m_bookmark && bookmarkMouseRect().contains(moveevent->pos()))
            m_bookmarkState = 1;
        else if (m_bookmark)
            m_bookmarkState = 3;
        else
            m_bookmarkState = 0;
        update();
    }
    return QGraphicsItem::sceneEvent(event);
}

void BrowserPage::setSearchHighlightRectf(const QVector<PageSection> &sections)
{
    if (sections.size() > 0) {
        if (m_parent->currentPage() == this->itemIndex() + 1)
            m_searchSelectLighRectf = sections.first();
        m_searchLightrectLst = sections;
        update();
    }
}

void BrowserPage::clearSearchHighlightRects()
{
    m_searchSelectLighRectf.clear();
    m_searchLightrectLst.clear();
    update();
}

void BrowserPage::clearSelectSearchHighlightRects()
{
    m_searchSelectLighRectf.clear();
    update();
}

int BrowserPage::searchHighlightRectSize()
{
    return m_searchLightrectLst.size();
}

PageSection BrowserPage::findSearchforIndex(int index)
{
    if (index >= 0 && index < m_searchLightrectLst.size()) {
        m_searchSelectLighRectf = m_searchLightrectLst[index];
        update();
        return m_searchSelectLighRectf;
    }

    return PageSection();
}

QRectF BrowserPage::getNorotateRect(const QRectF &rect)
{
    QRectF newrect;
    newrect.setX(rect.x()*m_scaleFactor);
    newrect.setY(rect.y()*m_scaleFactor);
    newrect.setWidth(rect.width()*m_scaleFactor);
    newrect.setHeight(rect.height()*m_scaleFactor);
    return newrect;
}

QRectF BrowserPage::translateRect(const QRectF &rect)
{
    //旋转角度逆时针增加
    QRectF newrect;
    switch (m_rotation) {
    case Dr::RotateBy0: {
        newrect.setX(rect.x()*m_scaleFactor);
        newrect.setY(rect.y()*m_scaleFactor);
        newrect.setWidth(rect.width()*m_scaleFactor);
        newrect.setHeight(rect.height()*m_scaleFactor);
        break;
    }
    case Dr::RotateBy90: {
        newrect.setX((m_originSizeF.height() - rect.y() - rect.height())*m_scaleFactor - boundingRect().height());
        newrect.setY(rect.x()*m_scaleFactor);
        newrect.setWidth(rect.height()*m_scaleFactor);
        newrect.setHeight(rect.width()*m_scaleFactor);
        break;
    }
    case Dr::RotateBy180: {
        newrect.setX((m_originSizeF.width() - rect.x() - rect.width())*m_scaleFactor - boundingRect().width());
        newrect.setY((m_originSizeF.height() - rect.y() - rect.height())*m_scaleFactor - boundingRect().height());
        newrect.setWidth(rect.width()*m_scaleFactor);
        newrect.setHeight(rect.height()*m_scaleFactor);
        break;
    }
    case Dr::RotateBy270: {
        newrect.setX(rect.y()*m_scaleFactor);
        newrect.setY((m_originSizeF.width() - rect.x() - rect.width())*m_scaleFactor - boundingRect().width());
        newrect.setWidth(rect.height()*m_scaleFactor);
        newrect.setHeight(rect.width()*m_scaleFactor);
        break;
    }
    default:
        break;
    }
    return  newrect;
}

BrowserAnnotation *BrowserPage::getBrowserAnnotation(const QPointF &point)
{
    BrowserAnnotation *item = nullptr;
    const QList<QGraphicsItem *> &itemlst = scene()->items(this->mapToScene(point));
    for (QGraphicsItem *itemIter : itemlst) {
        item = dynamic_cast<BrowserAnnotation *>(itemIter);
        if (item != nullptr) {
            return item;
        }
    }

    return nullptr;
}

BrowserWord *BrowserPage::getBrowserWord(const QPointF &point)
{
    BrowserWord *item = nullptr;
    const QList<QGraphicsItem *> &itemlst = scene()->items(this->mapToScene(point));
    for (QGraphicsItem *itemIter : itemlst) {
        item = dynamic_cast<BrowserWord *>(itemIter);
        if (item != nullptr) {
            return item;
        }
    }

    return nullptr;
}

bool BrowserPage::isBigDoc()
{
    if (Dr::PDF == m_sheet->fileType() && boundingRect().width() > 1000 && boundingRect().height() > 1000)
        return true;

    return false;
}
