#include "resizable_tool_button.h"
#include <QDebug>
ResizableToolButton::ResizableToolButton(QWidget *parent)
    : QToolButton(parent)
{
    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
                      this, &ResizableToolButton::handleTabletMode);
}

ResizableToolButton::~ResizableToolButton()
{
}

void ResizableToolButton::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
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
void ResizableToolButton::handlePADMode()
{
    if (!m_padModeSize.isEmpty()) {
        setFixedSize(m_padModeSize);
    }
}

void ResizableToolButton::handlePCMode()
{
    if (!m_pcModeSize.isEmpty()) {
        setFixedSize(m_pcModeSize);
    }
}

void ResizableToolButton::setTabletModeButtonSize(const QSize &pcMode, const QSize &padMode)
{
    m_padModeSize = padMode;
    m_pcModeSize = pcMode;
}

void ResizableToolButton::showEvent(QShowEvent *event)
{
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
    QToolButton::showEvent(event);
}