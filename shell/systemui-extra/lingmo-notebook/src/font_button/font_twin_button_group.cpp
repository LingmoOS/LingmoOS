#include "font_twin_button_group.h"
#include "lingmo_settings_monitor.h"

FontTwinButtonGroup::FontTwinButtonGroup(QWidget *parent)
    : CustomPushButtonGroup(parent)
{
    initUi();
    initConnect();
}

FontTwinButtonGroup::~FontTwinButtonGroup()
{
}

void FontTwinButtonGroup::initUi()
{
    bool isClassical =LingmoUISettingsMonitor::instance().widgetThemeName() == LingmoUISettingsMonitor::WidgetThemeStatus::CLASSICAL;
    frameUpdate(isClassical);
    fontSizeBtn = new FontSizePushButton(this);
    fontColorBtn = new FontColorPushButton(this);
    fontSizeBtn->setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    fontColorBtn->setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    fontSizeBtn->setToolTip(tr("Font Size"));
    fontColorBtn->setToolTip(tr("Font Color"));
    addButton(fontSizeBtn, CustomPushButtonGroup::BtnState::Left);
    addButton(fontColorBtn, CustomPushButtonGroup::BtnState::Right);
    widgetThemeUpdate(isClassical);

}

void FontTwinButtonGroup::initConnect()
{
    QObject::connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,  [=](){

    });
    QObject::connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::widgetThemeNameUpdate,  [=](bool isClassical){
        widgetThemeUpdate(isClassical);
    });
}

FontSizePushButton *FontTwinButtonGroup::getFontSizeBtn()
{
    return fontSizeBtn;
}
FontColorPushButton *FontTwinButtonGroup::getFontColorBtn()
{
    return fontColorBtn;
}

void FontTwinButtonGroup::widgetThemeUpdate(bool isClassical)
{

    if (isClassical) {
        fontSizeBtn->setStyleSheet("border-top-left-radius: 0px;border-bottom-left-radius: 0px");
        fontColorBtn->setStyleSheet("border-top-right-radius: 0px;border-bottom-right-radius: 0px");
    }
    else {
        fontSizeBtn->setStyleSheet("border-top-left-radius: 6px;border-bottom-left-radius: 6px");
        fontColorBtn->setStyleSheet("border-top-right-radius: 6px;border-bottom-right-radius: 6px");
    }
}
