// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resulttextview.h"
#include <QDebug>
#include <QScroller>
#include <QClipboard>
#include <QApplication>
#include <QScrollBar>

#include <math.h>

#define TAP_MOVE_DELAY 300

ResultTextView::ResultTextView(QWidget *parent)
    : m_Menu(nullptr), m_actCopy(nullptr), m_actCut(nullptr), m_actSelectAll(nullptr)
{
    Q_UNUSED(parent)
    //鼠标滑动事件接入
//    QScroller::grabGesture(this->viewport(), QScroller::ScrollerGestureType::TouchGesture);
//    QScroller::grabGesture(this->viewport(), QScroller::TouchGesture);
    viewport()->setCursor(Qt::IBeamCursor);
    m_Menu = new QMenu(this);
    m_actCopy = new QAction(this);
    m_actCopy->setText(tr("Copy"));
    m_actCut = new QAction(this);
    m_actCut->setText(tr("Cut"));

    m_actSelectAll = new QAction(this);
    m_actSelectAll->setText(tr("Select All"));
    m_actPaste = new QAction(this);
    m_actPaste->setText(tr("Paste"));

    m_Menu->addAction(m_actSelectAll);
    m_Menu->addAction(m_actCopy);
    m_Menu->addAction(m_actCut);
    m_Menu->addAction(m_actPaste);
    this->setLineWidth(6);

    connect(m_actSelectAll, &QAction::triggered, this, [ = ]() {
        emit this->selectAll();
    });
    connect(m_actCopy, &QAction::triggered, this, [ = ]() {
        emit this->copy();
    });
    connect(m_actCut, &QAction::triggered, this, [ = ]() {
        emit this->cut();
    });
    connect(m_actPaste, &QAction::triggered, this, [ = ]() {
        emit this->paste();
    });
    connect(this, &QPlainTextEdit::selectionChanged, this, &ResultTextView::onSelectionArea);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
}

void ResultTextView::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
    //当前是否有选中文本
    QString select_str = this->textCursor().selectedText();
    if (select_str.length() < 1) {
        m_actCopy->setEnabled(false);
        m_actCut->setEnabled(false);
    } else {
        m_actCopy->setEnabled(true);
        m_actCut->setEnabled(true);
    }
    //判断剪切板的状态
    QString clip_str = QApplication::clipboard()->text();
    if (clip_str.isEmpty()) {
        m_actPaste->setEnabled(false);
    } else {
        m_actPaste->setEnabled(true);
    }
    m_Menu->exec(QCursor::pos());
}

void ResultTextView::resizeEvent(QResizeEvent *event)
{
    emit sigChangeSize();
    this->viewport()->setFixedWidth(this->width() - 15);
    QPlainTextEdit::resizeEvent(event);
}

void ResultTextView::mouseMoveEvent(QMouseEvent *e)
{
//    if (Qt::MouseEventSynthesizedByQt == e->source()) {
//        m_endY = e->y();
//        m_endX = e->x();
//    }
    //add for single refers to the sliding
    if (e->type() == QEvent::MouseMove && e->source() == Qt::MouseEventSynthesizedByQt) {
        const ulong diffTimeX = e->timestamp() - m_lastMouseTimeX;
        const ulong diffTimeY = e->timestamp() - m_lastMouseTimeY;
        const int diffYpos = e->pos().y() - m_lastMouseYpos;
        const int diffXpos = e->pos().x() - m_lastMouseXpos;
        m_lastMouseTimeX = e->timestamp();
        m_lastMouseTimeY = e->timestamp();
        m_lastMouseYpos = e->pos().y();
        m_lastMouseXpos = e->pos().x();

        if (m_gestureAction == GA_slide) {
            QFont font = this->font();

            /*开根号时数值越大衰减比例越大*/
            qreal direction = diffYpos > 0 ? 1.0 : -1.0;
            slideGestureY(-direction * sqrt(abs(diffYpos)) / font.pointSize());
            qreal directionX = diffXpos > 0 ? 1.0 : -1.0;
            slideGestureX(-directionX * sqrt(abs(diffXpos)) / font.pointSize());

            /*预算惯性滑动时间*/
            m_stepSpeedY = static_cast<qreal>(diffYpos) / static_cast<qreal>(diffTimeY + 0.000001);
            durationY = sqrt(abs(m_stepSpeedY)) * 1000;
            m_stepSpeedX = static_cast<qreal>(diffXpos) / static_cast<qreal>(diffTimeX + 0.000001);
            durationX = sqrt(abs(m_stepSpeedX)) * 1000;

            /*预算惯性滑动距离,4.0为调优数值*/
            m_stepSpeedY /= sqrt(font.pointSize() * 4.0);
            changeY = m_stepSpeedY * sqrt(abs(m_stepSpeedY)) * 100;
            m_stepSpeedX /= sqrt(font.pointSize() * 4.0);
            changeX = m_stepSpeedX * sqrt(abs(m_stepSpeedX)) * 100;

            //return true;

        }

        if (m_gestureAction != GA_null) {
            //return true;
        }
    }

    QApplication::restoreOverrideCursor();

    if (viewport()->cursor().shape() != Qt::IBeamCursor) {
        viewport()->setCursor(Qt::IBeamCursor);
    }
    QPlainTextEdit::mouseMoveEvent(e);
}

void ResultTextView::mousePressEvent(QMouseEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress && e->source() == Qt::MouseEventSynthesizedByQt) {
        m_lastMouseTimeX = e->timestamp();
        m_lastMouseTimeY = e->timestamp();
        m_lastMouseYpos = e->pos().y();
        m_lastMouseXpos = e->pos().x();

//        if (tweenY.activeY()) {
//            m_slideContinueY = true;
//            tweenY.stopY();
//        }

//        if (tweenX.activeX()) {
//            m_slideContinueX = true;
//            tweenX.stopX();
//        }
    }
    QPlainTextEdit::mousePressEvent(e);
}

void ResultTextView::mouseReleaseEvent(QMouseEvent *e)
{
    //修复一指滑动导致了ocr无法选择任何文本
    if (e->type() == QEvent::MouseButtonRelease && e->source() == Qt::MouseEventSynthesizedByQt) {
        qDebug() << "action is over" << m_gestureAction;

        m_gestureAction = GA_null;
    }

    return QPlainTextEdit::mouseReleaseEvent(e);
}

bool ResultTextView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        gestureEvent(static_cast<QGestureEvent *>(event));
    return QPlainTextEdit::event(event);
}

bool ResultTextView::gestureEvent(QGestureEvent *event)
{
    if (QGesture *tap = event->gesture(Qt::TapGesture)) {
        tapGestureTriggered(static_cast<QTapGesture *>(tap));
    }
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture))
        tapAndHoldGestureTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    // qDebug()<<event<<"this is for test";
//    if (QGesture *swipe = event->gesture(Qt::SwipeGesture))
//        swipeTriggered(static_cast<QSwipeGesture *>(swipe));
    return true;
}

void ResultTextView::tapGestureTriggered(QTapGesture *tap)
{

    switch (tap->state()) {
    case Qt::GestureStarted: {
        m_gestureAction = GA_tap;
        m_tapBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        break;
    }
    case Qt::GestureUpdated: {
        break;
    }
    case Qt::GestureCanceled: {
        //根据时间长短区分轻触滑动
        qint64 timeSpace = QDateTime::currentDateTime().toMSecsSinceEpoch() - m_tapBeginTime;
        if (timeSpace < TAP_MOVE_DELAY || m_slideContinue) {
            m_slideContinue = false;
            m_gestureAction = GA_slide;
            qDebug() << "slide start" << timeSpace;
        } else {
            m_gestureAction = GA_null;
            qDebug() << "null start" << timeSpace;
        }
        break;
    }
    case Qt::GestureFinished: {
        m_gestureAction = GA_null;
        break;
    }
    default: {
        break;
    }
    }
}

void ResultTextView::tapAndHoldGestureTriggered(QTapAndHoldGesture *tapAndHold)
{
    //单指长按
    switch (tapAndHold->state()) {
    case Qt::GestureStarted:
        m_gestureAction = GA_hold;
        break;
    case Qt::GestureUpdated:
        break;
    case Qt::GestureCanceled:
        break;
    case Qt::GestureFinished:
        m_gestureAction = GA_null;
        break;
    default:
        break;
    }
}

void ResultTextView::panTriggered(QPanGesture *pan)
{
    //两指平移
    switch (pan->state()) {
    case Qt::GestureStarted:
        m_gestureAction = GA_pan;
        break;
    case Qt::GestureUpdated:
        break;
    case Qt::GestureCanceled:
        break;
    case Qt::GestureFinished:
        m_gestureAction = GA_null;
        break;
    default:
        break;
    }
}

void ResultTextView::pinchTriggered(QPinchGesture *pinch)
{
    //两指拉伸   -----缩放or放大
    switch (pinch->state()) {
    case Qt::GestureStarted: {
        m_gestureAction = GA_pinch;
        if (static_cast<int>(m_scaleFactor) != m_fontSize) {
            m_scaleFactor = m_fontSize;
        }
        break;
    }
    case Qt::GestureUpdated: {
        QPinchGesture::ChangeFlags changeFlags = pinch->changeFlags();
        if (changeFlags & QPinchGesture::ScaleFactorChanged) {
            m_currentStepScaleFactor = pinch->totalScaleFactor();
        }
        break;
    }
    case Qt::GestureCanceled: {
        Q_ASSERT(false);
        break;
    }
    case Qt::GestureFinished: {
        m_gestureAction = GA_null;
        m_scaleFactor *= m_currentStepScaleFactor;
        m_currentStepScaleFactor = 1;
        break;
    }
    default: {
        Q_ASSERT(false);
        break;
    }
    }//switch

    //QFont font = getVTFont();
    int size = static_cast<int>(m_scaleFactor * m_currentStepScaleFactor);
    if (size < 8)
        size = 8;
    if (size > 50)
        size = 50;
    //根据移动距离设置字体大小
//    setFontSize(size);
    //同步设置界面字体栏数值
//    qobject_cast<Window *>(this->window())->changeSettingDialogComboxFontNumber(size);
}


void ResultTextView::slideGestureY(qreal diff)
{
    static qreal delta = 0.0;
    int step = static_cast<int>(diff + delta);
    delta = diff + delta - step;

    verticalScrollBar()->setValue(verticalScrollBar()->value() + step);
}

void ResultTextView::slideGestureX(qreal diff)
{
    static qreal delta = 0.0;
    int step = static_cast<int>(diff + delta);
    delta = diff + delta - step;

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + step * 10);
}

void ResultTextView::onSelectionArea()
{
    if (m_gestureAction != GA_null) {
        QTextCursor cursor = textCursor();
        if (cursor.selectedText() != "") {
            cursor.clearSelection();
            setTextCursor(cursor);
        }
    }
}
