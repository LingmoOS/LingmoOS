#include "resizable_push_button.h"
#include <QDebug>
ResizablePushButton::ResizablePushButton(QWidget *parent)
    : QPushButton(parent)
{
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
                      this, &ResizablePushButton::handleTabletMode);
}

ResizablePushButton::~ResizablePushButton()
{
}

void ResizablePushButton::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::TabletStatus::PCMode:
        handlePCMode();
        break;
    case LingmoUISettingsMonitor::TabletStatus::PADHorizontalMode:
    case LingmoUISettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}
void ResizablePushButton::handlePADMode()
{
    if (!m_padModeSize.isEmpty()) {
        setFixedSize(m_padModeSize);
    }
}

void ResizablePushButton::handlePCMode()
{
    if (!m_pcModeSize.isEmpty()) {
        setFixedSize(m_pcModeSize);
    }
}

void ResizablePushButton::setTabletModeButtonSize(const QSize &pcMode, const QSize &padMode)
{
    m_padModeSize = padMode;
    m_pcModeSize = pcMode;
}

void ResizablePushButton::showEvent(QShowEvent *event)
{
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
    QPushButton::showEvent(event);
}