// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeheaderview.h"

#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DFontSizeManager>

#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <QPainterPath>

static const int kSpacingMargin = 4;



PreviousLabel::PreviousLabel(TreeHeaderView *parent)
    : DLabel(parent)
    , headerView_(parent)
{
    setFocusPolicy(Qt::TabFocus);
}

PreviousLabel::~PreviousLabel()
{

}

void PreviousLabel::setPrePath(const QString &strPath)
{
    QString tmp = '/' + strPath;
    setToolTip(tr("Current path:") + tmp); // 悬停提示

    tmp = tmp.left(tmp.lastIndexOf('/'));
    if (tmp.isEmpty()) {
        tmp.push_front('/');
    }
    QFontMetrics elideFont(tmp);
    QString str = elideFont.elidedText(tmp, Qt::ElideMiddle, width() - 15);
    setText("     .. " + tr("Back to: %1").arg(str));
}

void PreviousLabel::setFocusValue(bool bFocus)
{
    focusIn_ = bFocus;
}

void PreviousLabel::paintEvent(QPaintEvent *e)
{
    QRectF rectangle(0, 0, width(), height());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bgColor;
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        if (focusIn_) {
            bgColor = QColor(212, 212, 212);
        } else {
            bgColor = QColor(247, 247, 247);
        }
    } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        if (focusIn_) {
            bgColor = QColor(44, 44, 44);
        } else {
            if(DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
                bgColor = QColor(255, 255, 255, 12);
            } else {
                bgColor = QColor(38, 38, 38);
            }
        }
    }

    QPainterPath pp;
    pp.addRoundedRect(rectangle, 8, 8);
    painter.fillPath(pp, bgColor);
    QLabel::paintEvent(e);
}

void PreviousLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClickedSignal();
    QLabel::mouseDoubleClickEvent(event);
}

void PreviousLabel::enterEvent(QEvent *event)
{
    if(!hasFocus()) {
        focusIn_ = true;
    }
    DLabel::enterEvent(event);
    update();
}

void PreviousLabel::leaveEvent(QEvent *event)
{
    if(!hasFocus()) {
        focusIn_ = false;
    }
    DLabel::leaveEvent(event);
    update();
}

void PreviousLabel::focusInEvent(QFocusEvent *event)
{
    focusIn_ = true;
    DLabel::focusInEvent(event);
}

void PreviousLabel::focusOutEvent(QFocusEvent *event)
{
    focusIn_ = false;
    DLabel::focusOutEvent(event);
}

void PreviousLabel::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key::Key_Enter == event->key() || Qt::Key::Key_Return == event->key()) {
        clearFocus(); //返回上一级时需主动移除焦点
        emit doubleClickedSignal();
    } else {
        DLabel::keyPressEvent(event);
    }
}



TreeHeaderView::TreeHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    viewport()->setAutoFillBackground(false);
    setSectionsClickable(true);
    setHighlightSections(true);
    setFrameShape(DHeaderView::NoFrame);

    m_pPreLbl = new PreviousLabel(this);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        setFixedHeight(TABLE_HEIGHT_NormalMode + 2);
        m_pPreLbl->setFixedHeight(TABLE_HEIGHT_NormalMode);
        m_pPreLbl->setObjectName("gotoPreviousLabel");
        m_pPreLbl->move(SCROLLMARGIN, TABLE_HEIGHT_NormalMode + 2);
        m_pPreLbl->hide();
    } else {
        setFixedHeight(TABLE_HEIGHT_CompactMode + 2);
        m_pPreLbl->setFixedHeight(TABLE_HEIGHT_CompactMode);
        m_pPreLbl->setObjectName("gotoPreviousLabel");
        m_pPreLbl->move(SCROLLMARGIN, TABLE_HEIGHT_CompactMode + 2);
        m_pPreLbl->hide();
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_pPreLbl->setFixedHeight(TABLE_HEIGHT_NormalMode);
            setPreLblVisible(m_pPreLbl->isVisible());
            m_pPreLbl->move(SCROLLMARGIN, TABLE_HEIGHT_NormalMode + 2);
        } else {
            m_pPreLbl->setFixedHeight(TABLE_HEIGHT_CompactMode);
            setPreLblVisible(m_pPreLbl->isVisible());
            m_pPreLbl->move(SCROLLMARGIN, TABLE_HEIGHT_CompactMode + 2);
        }
    });
#else
    setFixedHeight(TABLE_HEIGHT_NormalMode + 2);
    m_pPreLbl->setFixedHeight(TABLE_HEIGHT_NormalMode);
    m_pPreLbl->setObjectName("gotoPreviousLabel");
    m_pPreLbl->move(SCROLLMARGIN, 38);
    m_pPreLbl->hide();
#endif
    DFontSizeManager::instance()->bind(m_pPreLbl, DFontSizeManager::T6, QFont::Weight::Medium);
}

TreeHeaderView::~TreeHeaderView()
{

}

QSize TreeHeaderView::sizeHint() const
{
    QSize size = sectionSizeFromContents(0);
    return QSize(size.width(), size.height() + m_spacing);
}

int TreeHeaderView::sectionSizeHint(int logicalIndex) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QFontMetrics fm(DApplication::font());
    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
    if (sortIndicatorSection() == logicalIndex) {
        return fm.width(buf) + margin * 3 + 8;
    } else {
        return fm.width(buf) + margin * 2;
    }
}

PreviousLabel *TreeHeaderView::getpreLbl()
{
    return m_pPreLbl;
}

void TreeHeaderView::setPreLblVisible(bool bVisible)
{
    m_pPreLbl->setVisible(bVisible);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        if (bVisible) {
            setFixedHeight(TABLE_HEIGHT_NormalMode*2 + 4); // 36+38+2 与item间隔2px
        } else {
            setFixedHeight(TABLE_HEIGHT_NormalMode + 2);
        }
    } else {
        if (bVisible) {
            setFixedHeight(TABLE_HEIGHT_CompactMode*2 + 4); // 36+38+2 与item间隔2px
        } else {
            setFixedHeight(TABLE_HEIGHT_CompactMode + 2);
        }
    }
#else
    if (bVisible) {
        setFixedHeight(TABLE_HEIGHT_NormalMode*2 + 4); // 36+38+2 与item间隔2px
    } else {
        setFixedHeight(TABLE_HEIGHT_NormalMode + 2);
    }
#endif

}

void TreeHeaderView::setLabelFocus(bool focus)
{
    if (focus) {
        m_pPreLbl->setFocus();

    } else {
        m_pPreLbl->clearFocus();
    }
    m_pPreLbl->setFocusValue(focus);
}
/*
void TreeHeaderView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    painter.save();

        DPalette::ColorGroup cg;
        //是否有为激活窗口状态
    #ifdef ENABLE_INACTIVE_DISPLAY
        QWidget *wnd = DApplication::activeWindow();
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    #else
        cg = DPalette::Active;
    #endif

        DApplicationHelper *dAppHelper = DApplicationHelper::instance();
        DPalette palette = dAppHelper->applicationPalette();

        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

        QBrush bgBrush(palette.color(cg, DPalette::Base));

        QStyleOptionHeader option;
        initStyleOption(&option);
        int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

        QRect rect = viewport()->rect();
        QRectF clipRect(rect.x(), rect.y(), rect.width(), rect.height() * 2);
        QRectF subRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height());
        QPainterPath clipPath, subPath;
        clipPath.addRoundedRect(clipRect, radius, radius);
        subPath.addRect(subRect);
        clipPath = clipPath.subtracted(subPath);

        painter.fillPath(clipPath, bgBrush);


DHeaderView::paintEvent(e);
painter.restore();
}
*/
void TreeHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    QStyleOptionHeader option;
    initStyleOption(&option);
    /*int margin = */style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    int nHeight = TABLE_HEIGHT_NormalMode;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        nHeight = TABLE_HEIGHT_NormalMode;
    } else {
        nHeight = TABLE_HEIGHT_CompactMode;
    }
#endif
    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), nHeight - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height() - 1, rect.width(),
                       nHeight - contentRect.height() - 2);

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // vertical spacing
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin + 2, m_spacing,
                        nHeight - kSpacingMargin * 2 - 6);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

    painter->fillRect(hSpacingRect, clearBrush);
    painter->fillRect(hSpacingRect, hSpacingBrush);

    if (visualIndex(logicalIndex) > 0) {
        painter->fillRect(vSpacingRect, clearBrush);
        painter->fillRect(vSpacingRect, vSpacingBrush);
    }

    // TODO: dropdown icon (8x5)
    QRect textRect(contentRect.x() + 6, contentRect.y(), contentRect.width() - 23,
                   nHeight);
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    //    int align = model()->headerData(logicalIndex, orientation(),
    //    Qt::TextAlignmentRole).toInt();
    static int align = Qt::AlignLeft | Qt::AlignVCenter;
    QFont pFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    pFont.setWeight(QFont::Weight::Medium);
    painter->setFont(pFont);
    if (0 == logicalIndex) {
        QRect col0Rect = textRect;
        col0Rect.setX(textRect.x() + 4 + SCROLLMARGIN);
        col0Rect.setWidth(col0Rect.width() + SCROLLMARGIN);
        painter->drawText(col0Rect, static_cast<int>(align), title);
    }
//    else if(logicalIndex == 3){
//        painter->drawText(textRect, static_cast<int>(Qt::AlignRight | Qt::AlignVCenter), title);
//    }
    else {
        painter->drawText(textRect, static_cast<int>(align), title);
    }

    // sort indicator
    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex) {
        // TODO: arrow size (8x5)
        QRect sortIndicator(textRect.x() + textRect.width() - ((0 == logicalIndex) ? 0 : 1),
                            textRect.y() + (textRect.height() - 5) / 2, 8, 8);
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }

    painter->restore();
}

void TreeHeaderView::resizeEvent(QResizeEvent *event)
{
    m_pPreLbl->setFixedWidth(event->size().width() - 2 * SCROLLMARGIN);
    DHeaderView::resizeEvent(event);
}
