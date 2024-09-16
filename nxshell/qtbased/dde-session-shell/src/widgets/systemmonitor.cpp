// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmonitor.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>

#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE

SystemMonitor::SystemMonitor(QWidget *parent)
    : QWidget(parent)
    , m_icon(new QLabel(this))
    , m_text(new QLabel(this))
    , m_state(Leave)
{
    setObjectName(QStringLiteral("SystemMonitor"));
    setAccessibleName(QStringLiteral("SystemMonitor"));

    setMinimumHeight(40);

    initUI();
}

void SystemMonitor::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 0, 10, 0);

    m_icon->setPixmap(DHiDPIHelper::loadNxPixmap(":/img/deepin-system-monitor.svg"));
    m_text->setText(tr("Start system monitor"));

    mainLayout->addWidget(m_icon, 0, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->addWidget(m_text, 1, Qt::AlignVCenter | Qt::AlignLeft);
}

void SystemMonitor::setState(const State state)
{
    m_state = state;
    update();
}

void SystemMonitor::enterEvent(QEvent *event)
{
    m_state = Enter;
    update();

    QWidget::enterEvent(event);
}

void SystemMonitor::leaveEvent(QEvent *event)
{
    m_state = Leave;
    update();

    QWidget::leaveEvent(event);
}

void SystemMonitor::mouseReleaseEvent(QMouseEvent *event)
{
    m_state = Release;
    emit requestShowSystemMonitor();
    update();

    QWidget::mouseReleaseEvent(event);
}

void SystemMonitor::mousePressEvent(QMouseEvent *event)
{
    m_state = Press;
    update();

    QWidget::mousePressEvent(event);
}

void SystemMonitor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        m_state = Press;
    }
    update();

    QWidget::keyReleaseEvent(event);
}

void SystemMonitor::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit requestShowSystemMonitor();
        m_state = Release;
    }
    update();

    QWidget::keyReleaseEvent(event);
}

void SystemMonitor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch (m_state) {
    case Enter:
        painter.setBrush(QColor(0, 0, 0, 75));
        break;
    case Leave:
        painter.setBrush(QColor(0, 0, 0, 0));
        break;
    case Release:
        painter.setBrush(QColor(0, 0, 0, 75));
        break;
    case Press:
        painter.setBrush(QColor(0, 0, 0, 105));
        break;
    }
    painter.drawRoundedRect(QRect(1, 1, width() - 2, height() - 2), 10, 10);

    QWidget::paintEvent(event);
}
