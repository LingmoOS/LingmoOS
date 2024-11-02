#ifndef __RESIZABLE_PUSH_BUTTON_H__
#define __RESIZABLE_PUSH_BUTTON_H__

#include <QPushButton>
#include <QSize>
#include "lingmo_settings_monitor.h"

class ResizablePushButton : public QPushButton
{
    Q_OBJECT
public:
    ResizablePushButton(QWidget *parent = nullptr);
    ~ResizablePushButton();
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
#endif // __RESIZABLE_PUSH_BUTTON_H__