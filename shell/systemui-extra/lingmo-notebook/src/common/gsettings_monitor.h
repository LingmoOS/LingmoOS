#ifndef GSETTINGS_MONITOR_H
#define GSETTINGS_MONITOR_H

#include <QObject>
#include <QGSettings>


class GsettingsMonitor : public QObject
{
    Q_OBJECT

public:
    enum StyleStatus{
        NONE = 0,
        LIGHT,
        DARK
    };

    enum WidgetThemeStatus{
        UNCLASSICAL = 0,
        CLASSICAL
    };

    GsettingsMonitor(/* args */);
    ~GsettingsMonitor() override;
    int transparency() const;
    StyleStatus styleStatus() const;
    WidgetThemeStatus widgetThemeName() const;

signals:
    void transparencyUpdate(int transparency);
    void styleStatusUpdate(StyleStatus styleStatus);
    void widgetThemeUpdate(WidgetThemeStatus widgetThemeStatus);

private:
    bool gsettingsCheck();
    void initGsettings();
    void getTransparency();
    void getStyleStatus();
    void getWidgetThemeStatus();

    StyleStatus styleNameConvert(const QString &styleName);
private:
    const char *LINGMO_STYLE = "org.lingmo.style";
    const char *LINGMO_FONT_SIZE = "systemFontSize";
    const char *LINGMO_FONT = "systemFont";
    const char *LINGMO_MENU_TRANSPARENCY = "menuTransparency";
    const char *LINGMO_STYLE_NAME = "styleName";
    const char *LINGMO_WIDGET_THEME_NAME = "widgetThemeName";
private:
    QGSettings *m_lingmoStyle;

    int m_menuTrans = -1;
    StyleStatus m_styleStatus = NONE;
    WidgetThemeStatus m_isClassical = UNCLASSICAL;
};

#endif // GSETTINGS_MONITOR_H
