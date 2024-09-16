// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jumpcalendarbutton.h"

#include <QHBoxLayout>
#include <QProcess>
#include <QMouseEvent>

#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>
#include <DDBusSender>
#include <DPaletteHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

JumpCalendarButton::JumpCalendarButton(QWidget *parent)
    : QFrame(parent)
    , m_hover(false)
    , m_mousePress(false)
    , m_iconButton(new CommonIconButton(this))
    , m_descriptionLabel(new DLabel(this))
{
    initUI();
}

JumpCalendarButton::JumpCalendarButton(const QIcon& icon, const QString& description, QWidget* parent)
    : QFrame(parent)
    , m_hover(false)
    , m_mousePress(false)
    , m_iconButton(new CommonIconButton(this))
    , m_descriptionLabel(new DLabel(this))
{
    initUI();

    m_iconButton->setIcon(icon);
    m_descriptionLabel->setText(description);
}

JumpCalendarButton::~JumpCalendarButton()
{

}

void JumpCalendarButton::initUI()
{
    setFixedHeight(36);
    setForegroundRole(QPalette::BrightText);
    m_iconButton->setFixedSize(16, 16);
    m_iconButton->setForegroundRole(QPalette::BrightText);

    m_descriptionLabel->setElideMode(Qt::ElideRight);
    m_descriptionLabel->setForegroundRole(foregroundRole());
    DFontSizeManager::instance()->bind(m_descriptionLabel, DFontSizeManager::T6);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->addWidget(m_iconButton);
    layout->addSpacing(5);
    layout->addWidget(m_descriptionLabel);
    layout->addStretch();
}

void JumpCalendarButton::setIcon(const QIcon &icon)
{
    m_iconButton->setIcon(icon, Qt::black, Qt::white);
}

void JumpCalendarButton::setDescription(const QString& description)
{
    m_descriptionLabel->setText(description);
}

bool JumpCalendarButton::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::Leave:
    case QEvent::Enter:
        m_hover = e->type() == QEvent::Enter;
        update();
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

void JumpCalendarButton::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e)
    QPainter p(this);
    QPalette palette = this->palette();
    QColor bgColor, textColor;
    if (m_mousePress) {
        textColor = palette.highlight().color();
        bgColor = palette.windowText().color();
        bgColor.setAlphaF(0.15);
    } else if(m_hover) {
        textColor = palette.windowText().color();
        bgColor = palette.windowText().color();
        bgColor.setAlphaF(0.1);
    } else {
        textColor = palette.windowText().color();
        bgColor = DPaletteHelper::instance()->palette(this).color(DPalette::ItemBackground);
        bgColor.setAlphaF(0.05);
    }
    palette.setBrush(QPalette::BrightText, textColor);
    m_iconButton->setPalette(palette);
    m_descriptionLabel->setPalette(palette);

    p.setBrush(bgColor);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 8, 8);
    return QFrame::paintEvent(e);
}

void JumpCalendarButton::mousePressEvent(QMouseEvent *event)
{
    if(m_mousePress != true) {
        m_mousePress = true;
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void JumpCalendarButton::mouseReleaseEvent(QMouseEvent* event)
{
    if(m_mousePress == true) {
        m_mousePress = false;
        update();
    }

    if (underMouse() && this->rect().contains(event->pos())) {
        Q_EMIT clicked();
        QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow");
        return;
    }
    QWidget::mouseReleaseEvent(event);
}
