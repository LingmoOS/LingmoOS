#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

#include <lingmosdk/applications/kwidget.h>
#include "lingmo_settings_monitor.h"

class CustomWidget : public kdk::KWidget
{
    Q_OBJECT
private:
    bool m_isSubWindow{false};
    QWidget *m_parentWindow{nullptr};
public:
    explicit CustomWidget(QWidget *parent = nullptr, bool isSubWindow = false);
    ~CustomWidget() override;
    void setWidgetName(const QString& widgetName);
public Q_SLOTS:
    void show();
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
};

#endif // CUSTOM_WIDGET_H
