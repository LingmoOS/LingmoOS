// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xwin_kill_preview_background_widget.h"

#include "ui_common.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DPalette>
#include <DApplication>
#include <DStyle>

#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QPaintEvent>
#include <QDebug>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QHideEvent>
#include <QRegion>

DWIDGET_USE_NAMESPACE

// default help tip icon size
static const QSize kDefaultIconSize {50, 50};

// constructor
XWinKillPreviewBackgroundWidget::XWinKillPreviewBackgroundWidget(QPixmap &background, QWidget *parent)
    : QWidget(parent)
    , m_background(background)
{
    initUI();
    initConnection();
}

// update selected region (window currently being hovered by mouse & intersects with current screen's preview widget)
void XWinKillPreviewBackgroundWidget::updateSelection(const QRegion &region)
{
    // coordinate translate
    m_selRegion = region.translated(-geometry().x(), -geometry().y());
    update();
}

// paint event handler
void XWinKillPreviewBackgroundWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPainterPath path;

    // paint screenshot pixmap as background
    painter.setOpacity(1);
    painter.drawPixmap(rect(), m_background);

    // paint selected region as red masked rect
    painter.setOpacity(0.25);
    path.addRegion(m_selRegion);
    painter.fillPath(path, Qt::red);

    // global palette
    auto palette = DApplicationHelper::instance()->applicationPalette();
    // DStyle instance
    auto *style = dynamic_cast<DStyle *>(this->style());
    // frame radius
    auto radius = style->pixelMetric(DStyle::PM_FrameRadius);
    // content margin
    auto margin = 10;
    // background color
    auto background = palette.color(DPalette::Current, DPalette::Background);
    // forground color
    auto foreground = palette.color(DPalette::Current, DPalette::Text);

    // calculate help tooltip rect's minimum size
    QSize minSize(std::max(m_textSize.width(), kDefaultIconSize.width()) + margin * 4,
                  margin * 6 + m_textSize.height() + kDefaultIconSize.height());
    // help tooltip rect
    QRect tooltipRect {(rect().width() - minSize.width()) / 2,
                       (rect().height() - minSize.height()) / 2,
                       minSize.width(),
                       minSize.height()};

    // paint help tooltip background rect
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(.8);
    QPainterPath bgpath;
    bgpath.addRoundedRect(tooltipRect, radius, radius);
    painter.fillPath(bgpath, background);

    painter.setOpacity(1);

    // calculate help tooltip icon's rect
    QRect iconRect(tooltipRect.x() + (tooltipRect.width() - kDefaultIconSize.width()) / 2,
                   tooltipRect.y() + margin * 2,
                   kDefaultIconSize.width(),
                   kDefaultIconSize.height());
    // paint help tooltip icon
    m_icon.paint(&painter, iconRect, Qt::AlignHCenter | Qt::AlignVCenter);

    // draw tooltip text
    QPen pen(foreground);
    painter.setPen(pen);
    painter.setFont(m_font);
    painter.setOpacity(.8);
    // calculate tooltip text's bouding rect
    QRect textRect(tooltipRect.x() + (tooltipRect.width() - m_textSize.width()) / 2,
                   tooltipRect.y() + (tooltipRect.height() - m_textSize.height()) - margin * 2,
                   m_textSize.width(),
                   m_textSize.height());
    painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, m_text);
}

// initialize ui components
void XWinKillPreviewBackgroundWidget::initUI()
{
    Qt::WindowFlags flags {};
    flags |= Qt::Window;
    // always stay on top
    flags |= Qt::WindowStaysOnTopHint;
    // frameless window style
    flags |= Qt::FramelessWindowHint;
    // bypass window manager hint
    flags |= Qt::BypassWindowManagerHint;
    setWindowFlags(flags);
    // delete on itself when window closed
    setAttribute(Qt::WA_DeleteOnClose);
    // set transparent background attribute
    setAttribute(Qt::WA_TranslucentBackground);

    // styled kill help tooltip icon
    m_icon = QIcon::fromTheme(iconPathFromQrc("kill.svg"));
    // help tooltip text
    m_text = QApplication::translate("Process.Choose.Window.Dialog",
                                     "Click the application you want to end");

    // calc preferred size for this widget
    m_font = this->font();
    // binding font size to global font manager
    m_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    QFontMetrics fm(m_font);
    // adjust text size
    m_textSize = fm.size(Qt::TextSingleLine, m_text);
}

// initialize connections (nothing to do here)
void XWinKillPreviewBackgroundWidget::initConnection()
{
}
