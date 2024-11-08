#include "gsettings_monitor.h"
#include "horizontalorverticalmode.h"
#include "lingmo_settings_monitor.h"
#include "QTimer"
LingmoUISettingsMonitor::LingmoUISettingsMonitor()
{
    d_ptrGsetting = new GsettingsMonitor();
    d_ptrDbus = new HorizontalOrVerticalMode();

    connect(d_ptrGsetting, &GsettingsMonitor::transparencyUpdate, this, &LingmoUISettingsMonitor::transparencyUpdate);
    connect(d_ptrGsetting, &GsettingsMonitor::styleStatusUpdate, this, [=](GsettingsMonitor::StyleStatus status){
        emit styleStatusUpdate(StyleStatus(status));
    });
    connect(d_ptrGsetting, &GsettingsMonitor::widgetThemeUpdate, this, [=](GsettingsMonitor::WidgetThemeStatus status){
        emit widgetThemeNameUpdate(WidgetThemeStatus(status));
    });
    connect(d_ptrDbus,     &HorizontalOrVerticalMode::RotationSig, this, [=](deviceMode mode){
        emit tabletModeUpdate(TabletStatus(mode));
    });

}

LingmoUISettingsMonitor::~LingmoUISettingsMonitor()
{
    delete d_ptrGsetting;
    delete d_ptrDbus;
}

int LingmoUISettingsMonitor::transparency() const
{
    return d_ptrGsetting->transparency();
}

LingmoUISettingsMonitor::StyleStatus LingmoUISettingsMonitor::styleStatus() const
{
    return StyleStatus(d_ptrGsetting->styleStatus());
}

LingmoUISettingsMonitor::TabletStatus LingmoUISettingsMonitor::tabletMode() const
{
    return TabletStatus(d_ptrDbus->defaultModeCapture());
}

LingmoUISettingsMonitor::WidgetThemeStatus LingmoUISettingsMonitor::widgetThemeName() const
{
    return WidgetThemeStatus(d_ptrGsetting->widgetThemeName());
}

void LingmoUISettingsMonitor::startTest(TabletStatus status)
{
    static int cnt = 0;
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [=](){
        if (status != None) {
            emit this->tabletModeUpdate(status);
            return ;
        }
        cnt++;

        if (cnt % 2 == 0) {
            emit this->tabletModeUpdate(TabletStatus::PCMode);
        } else {
            emit this->tabletModeUpdate(TabletStatus::PADHorizontalMode);
        }
    });
    timer->start(5000);

}
