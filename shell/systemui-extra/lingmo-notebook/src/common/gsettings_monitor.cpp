#include <QVariant>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>

#include "gsettings_monitor.h"

GsettingsMonitor::GsettingsMonitor()
{
    m_lingmoStyle = new QGSettings(LINGMO_STYLE);
    if (gsettingsCheck()) {
        initGsettings();
    }
}

GsettingsMonitor::~GsettingsMonitor()
{

}

bool GsettingsMonitor::gsettingsCheck()
{
    if(!QGSettings::isSchemaInstalled(LINGMO_STYLE)) {
        qCritical() << QString("gsettings Error: %1 not found!").arg(LINGMO_STYLE);
        return false;
    }
    return true;
}


void GsettingsMonitor::initGsettings()
{
    connect(m_lingmoStyle, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == LINGMO_MENU_TRANSPARENCY) {
            getTransparency();
        }
        else if (key == LINGMO_STYLE_NAME) {
            getStyleStatus();
        }
        else if (key == LINGMO_WIDGET_THEME_NAME) {
            getWidgetThemeStatus();
        }
    });
    getTransparency();
    getStyleStatus();
    getWidgetThemeStatus();
    return ;
}

void GsettingsMonitor::getTransparency()
{
    if (!gsettingsCheck()) {
        return;
    }

    int tmpMenuTrans = m_lingmoStyle->get(LINGMO_MENU_TRANSPARENCY).toInt();
    if (tmpMenuTrans != m_menuTrans) {
        if (m_menuTrans != -1) {
            emit transparencyUpdate(tmpMenuTrans);
        }
        m_menuTrans = tmpMenuTrans;
    }

    return ;
}

void GsettingsMonitor::getStyleStatus()
{
    if (!gsettingsCheck()) {
        return;
    }

    StyleStatus tmpStyleStatus = styleNameConvert(m_lingmoStyle->get(LINGMO_STYLE_NAME).toString());
    if (tmpStyleStatus != m_styleStatus) {
        if (m_styleStatus != NONE) {
            emit styleStatusUpdate(tmpStyleStatus);
        }
        m_styleStatus = tmpStyleStatus;
    }
    return ;
}

GsettingsMonitor::StyleStatus GsettingsMonitor::styleNameConvert(const QString &styleName)
{
    if (styleName == "lingmo-default"
     || styleName == "lingmo-white"
     || styleName == "lingmo-light"
     || styleName == "lingmo") {
                
        return LIGHT;
    }
    
    if (styleName == "lingmo-dark" || styleName == "lingmo-black") {
        return DARK;
    }

    return NONE;
}

int GsettingsMonitor::transparency() const
{
    return m_menuTrans;
}

GsettingsMonitor::StyleStatus GsettingsMonitor::styleStatus() const
{
    return m_styleStatus;
}

GsettingsMonitor::WidgetThemeStatus GsettingsMonitor::widgetThemeName() const
{
    return m_isClassical;
}


void GsettingsMonitor::getWidgetThemeStatus()
{
    if ("classical" == m_lingmoStyle->get(LINGMO_WIDGET_THEME_NAME).toString()) {
        m_isClassical = CLASSICAL;
    }
    else {
        m_isClassical = UNCLASSICAL;
    }

    emit widgetThemeUpdate(m_isClassical);
}
