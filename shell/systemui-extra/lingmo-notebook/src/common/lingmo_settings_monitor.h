#ifndef LINGMO_SETTINGS_MONITOR_H
#define LINGMO_SETTINGS_MONITOR_H

#include <QObject>
#include "singleton.h"

class GsettingsMonitor;
class HorizontalOrVerticalMode;

class LingmoUISettingsMonitor final : public QObject,
                                  public Singleton<LingmoUISettingsMonitor>
{
    Q_OBJECT
    friend class Singleton<LingmoUISettingsMonitor>;
private:
    LingmoUISettingsMonitor();
    ~LingmoUISettingsMonitor() override;
public:
    enum TabletStatus {
        None = 0,
        PADHorizontalMode = 1, //平板横屏
        PADVerticalMode = 2,   //平板竖屏
        PCMode = 3             // pc模式
    };

    enum StyleStatus{
        NONE = 0,
        LIGHT,
        DARK
    };

    enum WidgetThemeStatus{
        UNCLASSICAL = 0,
        CLASSICAL,
    };

    void startTest(TabletStatus status = None);
    int transparency() const;
    StyleStatus styleStatus() const;
    TabletStatus tabletMode() const;
    WidgetThemeStatus widgetThemeName() const;

signals:
    void transparencyUpdate(int transparency);
    void styleStatusUpdate(StyleStatus styleStatus);
    void tabletModeUpdate(TabletStatus tabletMode);
    void widgetThemeNameUpdate(WidgetThemeStatus widgetThemeName);

private:
    GsettingsMonitor *d_ptrGsetting{nullptr};
    HorizontalOrVerticalMode *d_ptrDbus{nullptr};
};




#endif // LINGMO_SETTINGS_MONITOR_H
