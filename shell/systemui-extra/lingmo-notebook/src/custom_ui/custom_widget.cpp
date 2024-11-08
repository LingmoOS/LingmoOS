#include "custom_widget.h"
#include "common_ui.h"
#include "global_variable.h"

CustomWidget::CustomWidget(QWidget *parent, bool isSubWindow) :
    kdk::KWidget(parent),
    m_isSubWindow(isSubWindow),
    m_parentWindow(parent)
{
    if (m_parentWindow != nullptr && m_isSubWindow == false) {
        m_isSubWindow = true;
    }

    if (m_isSubWindow == true) {
        setWindowFlags(Qt::Tool);
        setWindowModality(Qt::ApplicationModal);
    }
    setIcon(APP_ICON_NAME);

    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
            this, &CustomWidget::handleTabletMode);
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::show()
{
    if (m_isSubWindow == true && m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    kdk::KWidget::show();
}

void CustomWidget::setWidgetName(const QString& widgetName)
{
    kdk::KWidget::setWidgetName(widgetName);
    QWidget::setWindowTitle(widgetName);
}

void CustomWidget::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
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

void CustomWidget::handlePADMode()
{

}

void CustomWidget::handlePCMode()
{

}

void CustomWidget::showEvent(QShowEvent *event)
{
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
    kdk::KWidget::showEvent(event);
}
