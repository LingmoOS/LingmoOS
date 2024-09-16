// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SlideWidget.h"
#include "MainWindow.h"
#include "DocSheet.h"
#include "SlidePlayWidget.h"
#include "Application.h"
#include "ReaderImageThreadPoolManager.h"

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DTitlebar>

#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDebug>

SlideWidget::SlideWidget(DocSheet *docsheet)
    : DWidget(MainWindow::windowContainSheet(docsheet))
    , m_docSheet(docsheet)
{
    initControl();
    initImageControl();
    show();
}

SlideWidget::~SlideWidget()
{
    setWidgetState(false);
}

void SlideWidget::initControl()
{
    m_offset = 0;
    m_curPageIndex = m_docSheet->currentIndex();
    m_preIndex = m_curPageIndex;

    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setWidgetState(true);
    if (parentWidget()) {
        parentWidget()->stackUnder(this);
        connect(parentWidget(), &QObject::destroyed, this, &SlideWidget::onParentDestroyed);
    }
    QDesktopWidget *dk = QApplication::desktop();
    int screenNum = dk->screenNumber(parentWidget()); //该应用在哪块屏幕上显示
    int width; //幻灯片宽
    int height; //幻灯片高
    if (screenNum < dApp->screens().count() && screenNum >= 0) {
        width = dApp->screens().at(screenNum)->size().width();
        height = dApp->screens().at(screenNum)->size().height();
    } else {
        //若screenNum不合法，默认主屏幕
        width = dApp->primaryScreen()->size().width();
        height = dApp->primaryScreen()->size().height();
    }
    qInfo() << QString("screenNum:%1 width:%2 height:%3").arg(screenNum).arg(width).arg(height);
    this->setGeometry(0, 0, width, height);

    m_loadSpinner = new DSpinner(this);
    m_loadSpinner->setFixedSize(40, 40);
    m_loadSpinner->stop();
    m_loadSpinner->hide();

    m_slidePlayWidget = new SlidePlayWidget(this);
    connect(m_slidePlayWidget, &SlidePlayWidget::signalPreBtnClicked,  this, &SlideWidget::onPreBtnClicked);
    connect(m_slidePlayWidget, &SlidePlayWidget::signalPlayBtnClicked, this, &SlideWidget::onPlayBtnClicked);
    connect(m_slidePlayWidget, &SlidePlayWidget::signalNextBtnClicked, this, &SlideWidget::onNextBtnClicked);
    connect(m_slidePlayWidget, &SlidePlayWidget::signalExitBtnClicked, this, &SlideWidget::onExitBtnClicked);
    m_slidePlayWidget->setFixedWidth(250);
    m_slidePlayWidget->move((width - m_slidePlayWidget->width()) / 2, height - 100);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_slidePlayWidget->setFixedWidth(250);
        m_slidePlayWidget->move((width - m_slidePlayWidget->width()) / 2, height - 100);
    } else {
        m_slidePlayWidget->setFixedWidth(194);
        m_slidePlayWidget->move((width - m_slidePlayWidget->width()) / 2, height - 100);
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_slidePlayWidget->setFixedWidth(250);
            m_slidePlayWidget->move((width - m_slidePlayWidget->width()) / 2, height - 100);
        } else {
            m_slidePlayWidget->setFixedWidth(194);
            m_slidePlayWidget->move((width - m_slidePlayWidget->width()) / 2, height - 100);
        }
    });
#endif
    onFetchImage(m_curPageIndex);
    onFetchImage(m_preIndex);
}

void SlideWidget::initImageControl()
{
    m_imageTimer = new QTimer(this);
    m_imageTimer->setInterval(3000);
    connect(m_imageTimer, &QTimer::timeout, this, &SlideWidget::onImageShowTimeOut);
    //    m_imageTimer->start();

    m_imageAnimation = new QPropertyAnimation(this, "", this);
    m_imageAnimation->setEasingCurve(QEasingCurve::Linear);
    m_imageAnimation->setDuration(500);
    connect(m_imageAnimation, &QPropertyAnimation::valueChanged, this, &SlideWidget::onImagevalueChanged);
    connect(m_imageAnimation, &QPropertyAnimation::finished, this, &SlideWidget::onImageAniFinished);
}

void SlideWidget::onImagevalueChanged(const QVariant &variant)
{
    m_offset = variant.toInt();
    update();
}

void SlideWidget::onImageAniFinished()
{
    bool autoplay = m_slidePlayWidget->getPlayStatus();
    if (autoplay) {
        m_imageTimer->start();
    } else {
        m_imageTimer->stop();
    }
}

void SlideWidget::onParentDestroyed()
{
    m_parentIsDestroyed = true;
}

void SlideWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);

    QPainter painter(this);

    painter.setRenderHints(QPainter::SmoothPixmapTransform);

    painter.fillRect(this->rect(), Qt::black);

    int roffset = 0;

    if (m_blefttoright) {
        roffset = m_offset - width();
        painter.drawPixmap(m_offset, 0, m_lpixmap);
        painter.drawPixmap(roffset, 0, m_rpixmap);
    } else {
        roffset = m_offset + width();
        painter.drawPixmap(m_offset, 0, m_lpixmap);
        painter.drawPixmap(roffset, 0, m_rpixmap);
    }

    if (m_rpixmap.isNull()) {
        if (m_curPageIndex == m_preIndex) roffset = 0;
        m_loadSpinner->move(roffset + (this->width() - m_loadSpinner->width()) / 2, (this->height() - m_loadSpinner->height()) / 2);
        m_loadSpinner->start();
        m_loadSpinner->show();
    } else {
        m_loadSpinner->hide();
        m_loadSpinner->stop();
    }
}

void SlideWidget::mouseMoveEvent(QMouseEvent *event)
{
    DWidget::mouseMoveEvent(event);
    m_slidePlayWidget->showControl();
}

void SlideWidget::setWidgetState(bool full)
{
    if (m_parentIsDestroyed || !parentWidget())
        return;

    if (full) {
        m_nOldState = parentWidget()->windowState(); // 保存幻灯片播放前的windowstate
        if (!m_nOldState.testFlag(Qt::WindowFullScreen)) {
            parentWidget()->setWindowState(parentWidget()->windowState() | Qt::WindowFullScreen);
        }
    } else if (parentWidget()->windowState().testFlag(Qt::WindowFullScreen) && !m_nOldState.testFlag(Qt::WindowFullScreen)) {
        parentWidget()->setWindowState(parentWidget()->windowState() & ~Qt::WindowFullScreen);
        m_nOldState = parentWidget()->windowState();
    }
}

void SlideWidget::onPreBtnClicked()
{
    //已到第一页时，再次点击上一页按钮
    if(m_curPageIndex <= 0) {
        m_slidePlayWidget->updateProcess(m_curPageIndex - 1, m_docSheet->pageCount());
        return;
    }

    m_preIndex = m_curPageIndex;

    m_curPageIndex--;

    if (m_curPageIndex >= m_docSheet->pageCount()) {
        m_curPageIndex = 0;
    } else if (m_curPageIndex < 0) {
        m_curPageIndex = m_docSheet->pageCount() - 1;
    }

    playImage();
}

void SlideWidget::onPlayBtnClicked()
{
    if (m_slidePlayWidget->getPlayStatus()) {
        //最后一页点播放时，则返回开始播放
        if(m_curPageIndex >= m_docSheet->pageCount() - 1) {
            m_curPageIndex = 0;
            m_preIndex = 0;
            playImage();
        }
        m_canRestart = true;
        m_imageTimer->start();
    } else {
        m_imageTimer->stop();
    }
}

void SlideWidget::onNextBtnClicked()
{
    //已到最后一页时，再次点击下一页按钮
    if(m_curPageIndex >= m_docSheet->pageCount() - 1) {
        m_slidePlayWidget->updateProcess(m_curPageIndex + 1, m_docSheet->pageCount());
        return;
    }
    m_preIndex = m_curPageIndex;

    m_curPageIndex++;

    if (m_curPageIndex >= m_docSheet->pageCount()) {
        m_curPageIndex = 0;
    } else if (m_curPageIndex < 0) {
        m_curPageIndex = m_docSheet->pageCount() - 1;
    }

    playImage();
}

void SlideWidget::onExitBtnClicked()
{
    m_docSheet->closeSlide();
}

void SlideWidget::playImage()
{
    m_imageAnimation->stop();

    if (m_preIndex <= m_curPageIndex) { //正序切换(包括第一页切到最后一页)
        m_blefttoright = false;
        m_imageAnimation->setStartValue(0);
        m_imageAnimation->setEndValue(0 - width());
    } else { //逆序切换(包括最后一页切到第一页)
        m_blefttoright = true;
        m_imageAnimation->setStartValue(0);
        m_imageAnimation->setEndValue(width());
    }

    m_lpixmap = m_rpixmap;
    m_rpixmap = QPixmap();

    m_slidePlayWidget->updateProcess(m_curPageIndex, m_docSheet->pageCount());
    onFetchImage(m_curPageIndex);
    m_imageAnimation->start();
    m_imageTimer->stop();
}

void SlideWidget::onImageShowTimeOut()
{
    m_preIndex = m_curPageIndex;

    m_curPageIndex++;

    if (m_curPageIndex >= m_docSheet->pageCount()) {
        if (m_canRestart) {
            m_curPageIndex = 0;

        } else {
            m_curPageIndex = m_docSheet->pageCount() - 1;

            m_slidePlayWidget->setPlayStatus(false);

            return;
        }
    }

    m_canRestart = false;

    playImage();
}

QPixmap SlideWidget::drawImage(const QPixmap &srcImage)
{
    QPixmap pixmap(static_cast<int>(this->width() * dApp->devicePixelRatio()), static_cast<int>(this->height() * dApp->devicePixelRatio()));
    pixmap.setDevicePixelRatio(dApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    qreal iwidth = srcImage.width();
    qreal iheight = srcImage.height();
    painter.drawPixmap(static_cast<int>((pixmap.width() - iwidth) * 0.5 / dApp->devicePixelRatio()),
                       static_cast<int>((pixmap.height() - iheight) * 0.5 / dApp->devicePixelRatio()), srcImage);
    return pixmap;
}

void SlideWidget::mousePressEvent(QMouseEvent *event)
{
    DWidget::mousePressEvent(event);
    if (event->button() == Qt::RightButton)
        onExitBtnClicked();
    else if (event->button() == Qt::LeftButton)
        m_slidePlayWidget->showControl();
}

void SlideWidget::mouseReleaseEvent(QMouseEvent *event)
{
    DWidget::mouseReleaseEvent(event);
    onNextBtnClicked();
}

void SlideWidget::wheelEvent(QWheelEvent *event)
{
    DWidget::wheelEvent(event);
    if (event->delta() > 0) {
        onPreBtnClicked();
    } else {
        onNextBtnClicked();
    }
}

void SlideWidget::handleKeyPressEvent(const QString &sKey)
{
    if (sKey == Dr::key_space) {
        bool autoplay = m_slidePlayWidget->getPlayStatus();
        m_slidePlayWidget->setPlayStatus(!autoplay);
    } else if (sKey == Dr::key_left || sKey == Dr::key_up) {
        onPreBtnClicked();
    } else if (sKey == Dr::key_right || sKey == Dr::key_down) {
        onNextBtnClicked();
    }
}

void SlideWidget::onFetchImage(int index)
{
    const QPixmap &pix = ReaderImageThreadPoolManager::getInstance()->getImageForDocSheet(m_docSheet, index);

    if (!pix.isNull() && qMax(pix.width(), pix.height()) == qMin(this->width() - 40, this->height() - 20)) {
        onUpdatePageImage(index);
        return;
    }

    ReaderImageParam_t tParam;
    tParam.pageIndex = index;
    tParam.sheet = m_docSheet;
    tParam.receiver = this;
    tParam.maxPixel = qMin(this->width() - 40, this->height() - 20);
    tParam.boundedRect = QSize(this->width(), this->height());
    tParam.slotFun = "onUpdatePageImage";
    ReaderImageThreadPoolManager::getInstance()->addgetDocImageTask(tParam);
}

void SlideWidget::onUpdatePageImage(int pageIndex)
{
    if (pageIndex == m_preIndex) {
        const QPixmap &lPix = ReaderImageThreadPoolManager::getInstance()->getImageForDocSheet(m_docSheet, pageIndex);
        m_lpixmap = drawImage(lPix);
        update();
    }

    if (pageIndex == m_curPageIndex) {
        const QPixmap &rPix = ReaderImageThreadPoolManager::getInstance()->getImageForDocSheet(m_docSheet, pageIndex);
        m_rpixmap = drawImage(rPix);
        update();
    }
}
