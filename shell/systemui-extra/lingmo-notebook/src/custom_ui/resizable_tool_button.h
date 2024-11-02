#ifndef __RESIZABLE_TOOL_BUTTON_H__
#define __RESIZABLE_TOOL_BUTTON_H__

#include <QToolButton>
#include <QSize>
#include "lingmo_settings_monitor.h"

class ResizableToolButton : public QToolButton
{
    Q_OBJECT
public:
    ResizableToolButton(QWidget *parent = nullptr);
    ~ResizableToolButton();
    void setTabletModeButtonSize(const QSize &pcMode, const QSize &padMode);
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
    QSize m_pcModeSize{QSize()};
    QSize m_padModeSize{QSize()};

};
#endif // __RESIZABLE_TOOL_BUTTON_H__