/*******************************************************************
 * statuswidget.cpp
 * SPDX-FileCopyrightText: 2009, 2010 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/
#include "statuswidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QSizePolicy>

#include <KBusyIndicatorWidget>

StatusWidget::StatusWidget(QWidget *parent)
    : QStackedWidget(parent)
    , m_cursorStackCount(0)
    , m_busy(false)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    // Main layout
    m_statusPage = new QWidget(this);
    m_busyPage = new QWidget(this);

    addWidget(m_statusPage);
    addWidget(m_busyPage);

    // Status widget
    m_statusLabel = new WrapLabel();
    m_statusLabel->setOpenExternalLinks(true);
    m_statusLabel->setTextFormat(Qt::RichText);
    // m_statusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    auto *statusLayout = new QHBoxLayout();
    statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusPage->setLayout(statusLayout);

    statusLayout->addWidget(m_statusLabel);

    // Busy widget
    m_throbberWidget = new KBusyIndicatorWidget(this);
    m_throbberWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    m_busyLabel = new WrapLabel();
    // m_busyLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    auto *busyLayout = new QHBoxLayout();
    busyLayout->setContentsMargins(0, 0, 0, 0);
    m_busyPage->setLayout(busyLayout);

    busyLayout->addWidget(m_busyLabel);
    busyLayout->addWidget(m_throbberWidget);
    busyLayout->setAlignment(m_throbberWidget, Qt::AlignVCenter);
}

void StatusWidget::setBusy(const QString &busyMessage)
{
    m_statusLabel->clear();
    m_busyLabel->setText(busyMessage);
    setCurrentWidget(m_busyPage);
    setBusyCursor();
    m_busy = true;
}

void StatusWidget::setIdle(const QString &idleMessage)
{
    m_busyLabel->clear();
    m_statusLabel->setText(idleMessage);
    setCurrentWidget(m_statusPage);
    setIdleCursor();
    m_busy = false;
}

void StatusWidget::addCustomStatusWidget(QWidget *widget)
{
    auto *statusLayout = static_cast<QHBoxLayout *>(m_statusPage->layout());

    statusLayout->addWidget(widget);
    statusLayout->setAlignment(widget, Qt::AlignVCenter);
    widget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
}

void StatusWidget::setBusyCursor()
{
    if (isVisible()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_cursorStackCount++;
    }
}

void StatusWidget::setIdleCursor()
{
    while (m_cursorStackCount != 0) {
        QApplication::restoreOverrideCursor();
        m_cursorStackCount--;
    }
}

void StatusWidget::hideEvent(QHideEvent *)
{
    if (m_busy) {
        setIdleCursor();
    }
}

void StatusWidget::showEvent(QShowEvent *)
{
    if (m_busy) {
        setBusyCursor();
    }
}

#include "moc_statuswidget.cpp"
