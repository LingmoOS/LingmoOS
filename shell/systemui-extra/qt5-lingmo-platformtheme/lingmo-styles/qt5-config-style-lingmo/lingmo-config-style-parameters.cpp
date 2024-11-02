/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */


#include <QPushButton>
#include <QToolButton>
#include <QApplication>
#include <QPainter>
#include <KWindowEffects>
#include <QPainterPath>
#include <QDebug>
#include <QObject>
#include <QGSettings>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QFile>
#include <QStandardPaths>

#include "settings/lingmo-style-settings.h"
#include "lingmo-config-style-parameters.h"
#include "settings/black-list.h"
#include "../platformthemedebug.h"

extern QColor configMixColor(const QColor &c1, const QColor &c2, qreal bias);
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace LINGMOConfigStyleSpace;

LINGMOConfigStyleParameters::LINGMOConfigStyleParameters(QObject *parent, bool isDark, QString cfgName)
{
    cDebug << "LINGMOConfigStyleParameters................";
    QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.statusmanager.interface",
                                                           "/",
                                                           "com.lingmo.statusmanager.interface",
                                                           "get_current_tabletmode");
    QDBusMessage ret = QDBusConnection::sessionBus().call(message);
    if (ret.type() != QDBusMessage::ReplyMessage) {
        //从返回参数获取返回值
        cDebug << "complex type failed!";
    } else {
        m_isTableMode = ret.arguments()[0].value<bool>();
    }

    updateParameters(m_isTableMode);
    auto settings = LINGMOStyleSettings::globalInstance();
    auto styleName = settings->get("styleName").toString();
    m_themeColor = settings->get("theme-color").toString();

    setCfgName(cfgName);

    initPalette(isDark);

    initRadiusCfg();

    radius = m_radiusStruct.normalRadius;

    switch (m_adjustColorRules) {
    case DefaultRGB: {
        MenuItem_HMargin = 8;
        Menu_MarginWidth = 4;
        Menu_Combobox_Popup_MarginWidth = Menu_MarginWidth;

        Slider_Thickness = 20;
        Slider_Length = 20;
        Slider_GrooveLength = 4;

        TabBar_ScrollButtonOverlap = 2;
        indicatorIconHoverNeedHighLight = true;
        ScrooBar_ShowLine = false;
        TabBar_ScrollButtonWidth = 16;

        setTabBarIndicatorLayout(TabBarIndicatorLayout::TabBarIndicator_Horizontal);

    }break;
    case HSL: {
        MenuItem_HMargin = 0;
        Menu_MarginWidth = 1;
        Menu_Combobox_Popup_MarginWidth = Menu_MarginWidth;

        Slider_Thickness = 18;
        Slider_Length = 9;
        Slider_GrooveLength = 6;
        setTabBarIndicatorLayout(TabBarIndicatorLayout::TabBarIndicator_Vertical);


        TabBar_ScrollButtonOverlap = 0;
        indicatorIconHoverNeedHighLight = false;
        ScrooBar_ShowLine = true;
        TabBar_SplitLine = true;
        TabBar_ScrollButtonWidth = 32;
    }break;
    default:
        break;
    }
}

LINGMOConfigStyleParameters::~LINGMOConfigStyleParameters()
{
    if(m_readCfg)
    {
        delete m_readCfg;
    }
}

void LINGMOConfigStyleParameters::initialDefaultPaletteColor(QString colorPath, bool isDark)
{
    if (!QFile(colorPath).exists()) {
        QString path = ColorPath;
        colorPath = path + DefaultConfigName;
    }

    if (!QFile(colorPath).exists()) {
        cDebug << "colorPath:" << colorPath << "is not exists";
        return;
    }
    if (m_readCfg) {
        m_readCfg->deleteLater();
        m_readCfg = nullptr;
    }

    m_readCfg = new ReadThemeConfig(colorPath, isDark);
    m_adjustColorRules = m_readCfg->getColorValueAdjustRules() == "HSL" ? AdjustColorRules::HSL : AdjustColorRules::DefaultRGB;
    LINGMOColorTheme::PaletteColorCfg paletteColorCfg = m_readCfg->paletteColorCfg();

    defaultPalette.setColor(QPalette::Active, QPalette::WindowText, paletteColorCfg.windowText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::WindowText, paletteColorCfg.windowText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::WindowText, paletteColorCfg.windowText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Button, paletteColorCfg.button_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Button, paletteColorCfg.button_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Button, paletteColorCfg.button_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Light, paletteColorCfg.light_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Light, paletteColorCfg.light_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Light, paletteColorCfg.light_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Midlight, paletteColorCfg.midlight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Midlight, paletteColorCfg.midlight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Midlight, paletteColorCfg.midlight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Dark, paletteColorCfg.dark_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Dark, paletteColorCfg.dark_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Dark, paletteColorCfg.dark_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Mid, paletteColorCfg.mid_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Mid, paletteColorCfg.mid_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Mid, paletteColorCfg.mid_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Text, paletteColorCfg.text_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Text, paletteColorCfg.text_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Text, paletteColorCfg.text_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::BrightText, paletteColorCfg.brightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::BrightText, paletteColorCfg.brightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::BrightText, paletteColorCfg.brightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ButtonText, paletteColorCfg.buttonText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ButtonText, paletteColorCfg.buttonText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ButtonText, paletteColorCfg.buttonText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Base, paletteColorCfg.base_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Base, paletteColorCfg.base_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Base, paletteColorCfg.base_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Window, paletteColorCfg.window_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Window, paletteColorCfg.window_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Window, paletteColorCfg.window_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Shadow, paletteColorCfg.shadow_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Shadow, paletteColorCfg.shadow_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Shadow, paletteColorCfg.shadow_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Highlight, paletteColorCfg.highLight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Highlight, paletteColorCfg.highLight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Highlight, paletteColorCfg.highLight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::HighlightedText, paletteColorCfg.highLightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, paletteColorCfg.highLightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, paletteColorCfg.highLightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Link, paletteColorCfg.link_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Link, paletteColorCfg.link_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Link, paletteColorCfg.link_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::LinkVisited, paletteColorCfg.linkVisited_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::LinkVisited, paletteColorCfg.linkVisited_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::LinkVisited, paletteColorCfg.linkVisited_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::AlternateBase, paletteColorCfg.alternateBase_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::AlternateBase, paletteColorCfg.alternateBase_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::AlternateBase, paletteColorCfg.alternateBase_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::NoRole, paletteColorCfg.noRole_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::NoRole, paletteColorCfg.noRole_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::NoRole, paletteColorCfg.noRole_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipBase, paletteColorCfg.toolTipBase_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipBase, paletteColorCfg.toolTipBase_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipBase, paletteColorCfg.toolTipBase_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipText, paletteColorCfg.toolTipText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipText, paletteColorCfg.toolTipText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipText, paletteColorCfg.toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    defaultPalette.setColor(QPalette::Active, QPalette::PlaceholderText, paletteColorCfg.placeholderText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::PlaceholderText, paletteColorCfg.placeholderText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::PlaceholderText, paletteColorCfg.placeholderText_dis);
#endif

    if(isDark){
        Indicator_IconDefault = defaultPalette.color(QPalette::Active, QPalette::Mid);
        Indicator_IconHover   = defaultPalette.color(QPalette::Active, QPalette::Text);
        Indicator_IconSunken  = defaultPalette.color(QPalette::Active, QPalette::Text);
        Indicator_IconDisable = defaultPalette.color(QPalette::Disabled, QPalette::Text);
    }
    else{
        Indicator_IconDefault = defaultPalette.color(QPalette::Active, QPalette::Mid);
        Indicator_IconHover   = defaultPalette.color(QPalette::Active, QPalette::Text);
        Indicator_IconSunken  = defaultPalette.color(QPalette::Active, QPalette::Text);
        Indicator_IconDisable = defaultPalette.color(QPalette::Disabled, QPalette::Text);
    }
}

void LINGMOConfigStyleParameters::initPalette(bool isDark)
{
    cDebug << "homelocation:" << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString cfgPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
            HomeCFGColorPath + cfgName();
    if(!QFile::exists(cfgPath))
        cfgPath = ColorPath + cfgName();
    initialDefaultPaletteColor(cfgPath, isDark);
}

void LINGMOConfigStyleParameters::initRadiusCfg()
{
    cDebug << "homelocation:" << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString cfgPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
            HomeCFGColorPath + cfgName();
    if (!QFile::exists(cfgPath))
        cfgPath = UsrRadiusPath + cfgName();

    if (!QFile(cfgPath).exists()) {
        cDebug << "cfgPath:" << cfgPath << "is not exists";
        return;
    }
    if (m_readCfg == nullptr) {
        m_readCfg = new ReadThemeConfig(cfgPath);
        m_adjustColorRules = m_readCfg->getColorValueAdjustRules() == "HSL" ? AdjustColorRules::HSL : AdjustColorRules::DefaultRGB;
    }
    m_radiusStruct = m_readCfg->getRadiusStruct(LINGMO_Radius);
}

QPalette LINGMOConfigStyleParameters::setPalette(QPalette &palette)
{
    palette.setColor(QPalette::Active, QPalette::WindowText, defaultPalette.color(QPalette::Active, QPalette::WindowText));
    palette.setColor(QPalette::Inactive, QPalette::WindowText, defaultPalette.color(QPalette::Inactive, QPalette::WindowText));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));

    palette.setColor(QPalette::Active, QPalette::Button, defaultPalette.color(QPalette::Active, QPalette::Button));
    palette.setColor(QPalette::Inactive, QPalette::Button, defaultPalette.color(QPalette::Inactive, QPalette::Button));
    palette.setColor(QPalette::Disabled, QPalette::Button, defaultPalette.color(QPalette::Disabled, QPalette::Button));

    palette.setColor(QPalette::Active, QPalette::Light, defaultPalette.color(QPalette::Active, QPalette::Light));
    palette.setColor(QPalette::Inactive, QPalette::Light, defaultPalette.color(QPalette::Inactive, QPalette::Light));
    palette.setColor(QPalette::Disabled, QPalette::Light, defaultPalette.color(QPalette::Disabled, QPalette::Light));

    palette.setColor(QPalette::Active, QPalette::Midlight, defaultPalette.color(QPalette::Active, QPalette::Midlight));
    palette.setColor(QPalette::Inactive, QPalette::Midlight, defaultPalette.color(QPalette::Inactive, QPalette::Midlight));
    palette.setColor(QPalette::Disabled, QPalette::Midlight, defaultPalette.color(QPalette::Disabled, QPalette::Midlight));

    palette.setColor(QPalette::Active, QPalette::Dark, defaultPalette.color(QPalette::Active, QPalette::Dark));
    palette.setColor(QPalette::Inactive, QPalette::Dark, defaultPalette.color(QPalette::Inactive, QPalette::Dark));
    palette.setColor(QPalette::Disabled, QPalette::Dark, defaultPalette.color(QPalette::Disabled, QPalette::Dark));

    palette.setColor(QPalette::Active, QPalette::Mid, defaultPalette.color(QPalette::Active, QPalette::Mid));
    palette.setColor(QPalette::Inactive, QPalette::Mid, defaultPalette.color(QPalette::Inactive, QPalette::Mid));
    palette.setColor(QPalette::Disabled, QPalette::Mid, defaultPalette.color(QPalette::Disabled, QPalette::Mid));

    palette.setColor(QPalette::Active, QPalette::Text, defaultPalette.color(QPalette::Active, QPalette::Text));
    palette.setColor(QPalette::Inactive, QPalette::Text, defaultPalette.color(QPalette::Inactive, QPalette::Text));
    palette.setColor(QPalette::Disabled, QPalette::Text, defaultPalette.color(QPalette::Disabled, QPalette::Text));

    palette.setColor(QPalette::Active, QPalette::BrightText, defaultPalette.color(QPalette::Active, QPalette::BrightText));
    palette.setColor(QPalette::Inactive, QPalette::BrightText, defaultPalette.color(QPalette::Inactive, QPalette::BrightText));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, defaultPalette.color(QPalette::Disabled, QPalette::BrightText));

    palette.setColor(QPalette::Active, QPalette::ButtonText, defaultPalette.color(QPalette::Active, QPalette::ButtonText));
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, defaultPalette.color(QPalette::Inactive, QPalette::ButtonText));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, defaultPalette.color(QPalette::Disabled, QPalette::ButtonText));

    palette.setColor(QPalette::Active, QPalette::Base, defaultPalette.color(QPalette::Active, QPalette::Base));
    palette.setColor(QPalette::Inactive, QPalette::Base, defaultPalette.color(QPalette::Inactive, QPalette::Base));
    palette.setColor(QPalette::Disabled, QPalette::Base, defaultPalette.color(QPalette::Disabled, QPalette::Base));

    palette.setColor(QPalette::Active, QPalette::Window, defaultPalette.color(QPalette::Active, QPalette::Window));
    palette.setColor(QPalette::Inactive, QPalette::Window, defaultPalette.color(QPalette::Inactive, QPalette::Window));
    palette.setColor(QPalette::Disabled, QPalette::Window, defaultPalette.color(QPalette::Disabled, QPalette::Window));

    palette.setColor(QPalette::Active, QPalette::Shadow, defaultPalette.color(QPalette::Active, QPalette::Shadow));
    palette.setColor(QPalette::Inactive, QPalette::Shadow, defaultPalette.color(QPalette::Inactive, QPalette::Shadow));
    palette.setColor(QPalette::Disabled, QPalette::Shadow, defaultPalette.color(QPalette::Disabled, QPalette::Shadow));

    palette.setColor(QPalette::Active, QPalette::Highlight, defaultPalette.color(QPalette::Active, QPalette::Highlight));
    palette.setColor(QPalette::Inactive, QPalette::Highlight, defaultPalette.color(QPalette::Inactive, QPalette::Highlight));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, defaultPalette.color(QPalette::Disabled, QPalette::Highlight));

    palette.setColor(QPalette::Active, QPalette::HighlightedText, defaultPalette.color(QPalette::Active, QPalette::HighlightedText));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, defaultPalette.color(QPalette::Inactive, QPalette::HighlightedText));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, defaultPalette.color(QPalette::Disabled, QPalette::HighlightedText));

    palette.setColor(QPalette::Active, QPalette::Link, defaultPalette.color(QPalette::Active, QPalette::Link));
    palette.setColor(QPalette::Inactive, QPalette::Link, defaultPalette.color(QPalette::Inactive, QPalette::Link));
    palette.setColor(QPalette::Disabled, QPalette::Link, defaultPalette.color(QPalette::Disabled, QPalette::Link));

    palette.setColor(QPalette::Active, QPalette::LinkVisited, defaultPalette.color(QPalette::Active, QPalette::LinkVisited));
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, defaultPalette.color(QPalette::Inactive, QPalette::LinkVisited));
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, defaultPalette.color(QPalette::Disabled, QPalette::LinkVisited));

    palette.setColor(QPalette::Active, QPalette::AlternateBase, defaultPalette.color(QPalette::Active, QPalette::AlternateBase));
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, defaultPalette.color(QPalette::Inactive, QPalette::AlternateBase));
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, defaultPalette.color(QPalette::Disabled, QPalette::AlternateBase));

    palette.setColor(QPalette::Active, QPalette::NoRole, defaultPalette.color(QPalette::Active, QPalette::NoRole));
    palette.setColor(QPalette::Inactive, QPalette::NoRole, defaultPalette.color(QPalette::Inactive, QPalette::NoRole));
    palette.setColor(QPalette::Disabled, QPalette::NoRole, defaultPalette.color(QPalette::Disabled, QPalette::NoRole));

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, defaultPalette.color(QPalette::Active, QPalette::ToolTipBase));
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, defaultPalette.color(QPalette::Inactive, QPalette::ToolTipBase));
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, defaultPalette.color(QPalette::Disabled, QPalette::ToolTipBase));

    palette.setColor(QPalette::Active, QPalette::ToolTipText, defaultPalette.color(QPalette::Active, QPalette::ToolTipText));
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, defaultPalette.color(QPalette::Inactive, QPalette::ToolTipText));
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, defaultPalette.color(QPalette::Disabled, QPalette::ToolTipText));

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, defaultPalette.color(QPalette::Active, QPalette::PlaceholderText));
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, defaultPalette.color(QPalette::Inactive, QPalette::PlaceholderText));
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, defaultPalette.color(QPalette::Disabled, QPalette::PlaceholderText));
#endif

    return palette;
}

int LINGMOConfigStyleParameters::getSpinBoxControlLayout()
{
    return m_spinBoxControlLayout;
}


int LINGMOConfigStyleParameters::getTabBarIndicatorLayout()
{
    return m_tabBarIndicatorLayout;
}

void LINGMOConfigStyleParameters::setTabBarIndicatorLayout(TabBarIndicatorLayout layout)
{
    m_tabBarIndicatorLayout = layout;
}

void LINGMOConfigStyleParameters::setLINGMOThemeColor(QPalette p,QString themeColor)
{
    m_themeColor = themeColor;
    m_stylePalette = p;
}

QString LINGMOConfigStyleParameters::themeColor()
{
    return m_themeColor;
}

bool LINGMOConfigStyleParameters::isDefaultThemeColor()
{
//    return false;
    return (m_themeColor == "default");
}

void LINGMOConfigStyleParameters::updateParameters(bool isTabletMode)
{
    if (isTabletMode) {//styleName == "lingmo-dark"){//
        m_scaleRatio4_3 = (4 * 1.0) / 3.0;
        m_scaleRatio3_2 = (3 * 1.0) / 2.0;
        m_scaleRatio2_1 = (2 * 1.0) / 1.0;
        m_scaleSliderDefaultLength = (241 * 1.0) / (94 * 1.0);
        m_ScaleComboBoxDefaultLength = (49 * 1.0) / (40 * 1.0);
        m_ScaleLineEditDefaultLength = (49 * 1.0) / (40 * 1.0);
        m_scaleTabBarHeight6_5 = (6 *1.0) / (5 * 1.0);

        m_isTableMode = true;
        m_spinBoxControlLayout = SpinBoxControlLayout::Horizontal;
        m_headerDefaultSectionSizeVertical = m_headerDefaultSectionSizeVertical4_3;
    } else {
        m_scaleRatio4_3 = 1.0;
        m_scaleRatio3_2 = 1.0;
        m_scaleRatio2_1 = 1.0;
        m_scaleSliderDefaultLength = 1.0;
        m_ScaleComboBoxDefaultLength = 1.0;
        m_ScaleLineEditDefaultLength = 1.0;
        m_scaleTabBarHeight6_5 = 1.0;

        m_isTableMode = false;
        m_spinBoxControlLayout = SpinBoxControlLayout::Vertical;
        m_headerDefaultSectionSizeVertical = m_headerDefaultSectionSizeVertical1_1;
    }

    ComboBox_DefaultMenuItemHeight = ComboBox_DefaultMenuItemHeight_original * m_scaleRatio4_3;
    MenuItem_DefaultHeight = MenuItem_DefaultHeight_origin * m_scaleRatio4_3;
    MenuItemSeparator_MarginHeight = MenuItemSeparator_MarginHeight_origin * m_scaleRatio4_3;
    LineEdit_DefaultHeight = LineEdit_DefaultHeight_origin * m_scaleRatio4_3;
    IconButton_DefaultWidth = IconButton_DefaultWidth_origin * m_scaleRatio4_3;
    Button_DefaultHeight = Button_DefaultHeight_origin * m_scaleRatio4_3;
    SpinBox_DefaultHeight = SpinBox_DefaultHeight_origin * m_scaleRatio4_3;
    ComboBox_DefaultHeight = ComboBox_DefaultHeight_origin * m_scaleRatio4_3;
    RadioButton_DefaultHeight = RadioButton_DefaultHeight_origin * m_scaleRatio4_3;
    CheckBox_DefaultHeight = CheckBox_DefaultHeight_origin * m_scaleRatio4_3;
    Table_HeaderHeight = Table_HeaderHeight_origin * m_scaleRatio4_3;
    ScroolBar_Width = ScroolBar_Width_origin * m_scaleRatio3_2;
    Slider_Length = Slider_Length_origin * m_scaleRatio2_1;
    Slider_Thickness = Slider_Thickness_origin * m_scaleRatio2_1;
    Slider_DefaultWidth = Slider_DefaultWidth_origin * m_scaleRatio2_1;
    Slider_GrooveLength = Slider_GrooveLength_origin * m_scaleRatio2_1;
    Slider_DefaultLength = Slider_DefaultLength_origin * m_scaleSliderDefaultLength;
    LineEdit_DefaultWidth = LineEdit_DefaultWidth_origin * m_ScaleLineEditDefaultLength;
    TabBar_DefaultHeight = TabBar_DefaultHeight_origin * m_scaleTabBarHeight6_5;
    ViewItem_DefaultHeight = ViewItem_DefaultHeight_origin * m_scaleRatio4_3;
    ToolTip_Height = ToolTip_Height_origin * m_scaleRatio4_3;
}

QColor LINGMOConfigStyleParameters::lanhuHSLToQtHsl(int h, int s, int l, int a)
{
    QColor c;
    c.setHsl(h, s * 255.0 /100.0, l * 255.0 /100.0, a);
    return c;
}

QColor LINGMOConfigStyleParameters::adjustColor(const QColor c, int hRange, int sRange, int lRange, int aRange)
{
//    cDebug << "22222" << c.red() << c.green() << c.blue() <<
//                c.hslHue() << c.hslSaturationF() << c.lightnessF() <<
//                hRange << sRange << lRange;
    QColor cc;
    int s, l, a;

    if((c.hslSaturationF() + sRange * 1.0 / 100.0) > 1){
        cWarning << "adjustColor s out of range1";
        s = 255;
    }
    else if((c.hslSaturationF() + sRange * 1.0 / 100.0) < 0){
        cWarning << "adjustColor s out of range0";
        s = 0;
    }
    else
        s = (c.hslSaturationF() + sRange * 1.0 / 100.0) *255.0;

    if((c.lightnessF() + lRange * 1.0 / 100.0) > 0.97){//根据设计稿寻光色彩识别   l应<=0.97
        cWarning << "adjustColor l out of range1" << c.lightnessF() << lRange << (c.lightnessF() + lRange * 1.0 / 100.0);
        l = 255.0 * 0.97;
    }
    else if((c.lightnessF() + lRange * 1.0 / 100.0) < 0.15){//根据设计稿上l最小取的均值
        cWarning << "adjustColor l out of range0";
        l = 0.15 * 255.0;
    }
    else
        l = (c.lightnessF() + lRange * 1.0 / 100.0) *255.0;

    if((c.alphaF() + aRange * 1.0 / 100.0) > 1){
        cWarning << "adjustColor a out of range1";
        a = 255;
    }
    else if((c.alphaF() + aRange * 1.0 / 100.0) < 0){
        cWarning << "adjustColor a out of range0";
        a = 0;
    }
    else
        a = (c.alphaF() + aRange * 1.0 / 100.0) *255.0;
    cc.setHsl(c.hslHue() + hRange, s, l, a);
//    cDebug << "33333" << c.hslHue() + hRange << (c.hslSaturationF() + sRange * 1.0 / 100.0) *255.0 <<
//                (c.lightnessF() + lRange * 1.0 / 100.0) * 255.0 <<
//                (c.alphaF() + aRange * 1.0 / 100.0) * 255.0;

    return  cc;
}

QColor LINGMOConfigStyleParameters::adjustH(const QColor c, int range)
{
    QColor cc;
    cc.setHsl(c.hslHue() + range, c.hslSaturation(), c.lightness(), c.alpha());
    return  cc;
}

QColor LINGMOConfigStyleParameters::adjustS(const QColor c, int range)
{
    QColor cc;
    int s;

    if((c.hslSaturationF() + range * 1.0 / 100.0) > 1){
        cWarning << "adjustS s out of range1";
        s = 255;
    }
    else if((c.hslSaturationF() + range * 1.0 / 100.0) < 0){
        cWarning << "adjustS s out of range0";
        s = 0;
    }
    else
        s = (c.hslSaturationF() + range * 1.0 / 100.0) *255.0;
    cc.setHsl(c.hslHue(), s, c.lightness(), c.alpha());
    return  cc;
}

QColor LINGMOConfigStyleParameters::adjustL(const QColor c, int range)
{
    QColor cc;
    int l;

    if((c.lightnessF() + range * 1.0 / 100.0) > 0.97){
        cWarning << "adjustL l out of range1";
        l = 255.0 * 0.97;
    }
    else if((c.lightnessF() + range * 1.0 / 100.0) < 0.15){
        cWarning << "adjustL l out of range0" << c.lightnessF() << range << (c.lightnessF() + range * 1.0 / 100.0);
        l = 255.0 * 0.15;
    }
    else
        l = (c.lightnessF() + range * 1.0 / 100.0) *255.0;

    cc.setHsl(c.hslHue(), c.hslSaturation(), l, c.alpha());
    return  cc;
}

QColor LINGMOConfigStyleParameters::adjustA(const QColor c, int range)
{
    QColor cc;
    int a;
    if((c.alphaF() + range * 1.0 / 100.0) > 1){
        cWarning << "adjustA a out of range1";
        a = 255;
    }
    else if((c.alphaF() + range * 1.0 / 100.0) < 0){
        cWarning << "adjustA a out of range0";
        a = 0;
    }
    else
        a = (c.alphaF() + range * 1.0 / 100.0) *255.0;


    cc.setHsl(c.hslHue(), c.hslSaturation(), c.lightness(), a);
    return  cc;
}

QColor LINGMOConfigStyleParameters::highlightClick(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -25, -34);//hightlight->210,72%,71% 210,47%,37%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 3, 10, 31);//hightlight->209,87%,57% 212,97%,88%
}

QColor LINGMOConfigStyleParameters::highlightHover(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -26, -40);//210,46%,31%
    }
    return adjustColor(p.color(QPalette::Active, QPalette::Highlight), 4, 2, 36);//213,89%,93%
}

QColor LINGMOConfigStyleParameters::buttonDarkClick(bool isDark, QPalette p)
{
    if(isDark){
        return adjustColor(p.color(QPalette::Disabled, QPalette::ButtonText), 0, 1, 5);
    }
    return QColor();
}

void LINGMOConfigStyleParameters::normalButtonColor(bool isDark, QPalette p, QPen &defaultPen, QBrush &defaultBrush,
                                                  QPen &hoverPen, QBrush &hoverBrush, QPen &clickPen, QBrush &clickBrush,
                                                  QPen &disablePen, QBrush &disableBrush)
{    
    //hover
    if (isDark) {
        //hightlight->210,72%,71% 210,67%,56%
        hoverPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    } else {
        //hightlight->209,87%,57% 213,73%,72%
        hoverPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    }
    hoverBrush = QBrush(highlightHover(isDark, p));


    //click
    if (isDark) {
        //hightlight->210,72%,71%
        clickPen = QPen(p.brush(QPalette::Active, QPalette::Highlight),
                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    } else {
        //hightlight->209,87%,57% 214,66%,60%
        clickPen = QPen(QBrush(adjustColor(p.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
                        1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    }
    clickBrush = QBrush(highlightClick(isDark, p));
}


void LINGMOConfigStyleParameters::setCfgName(QString cfgName)
{
    if(cfgName == "")
        cfgName = DefaultConfigName;
    if(m_cfgName != cfgName + ".json")
        m_cfgName = cfgName + ".json";
}

QString LINGMOConfigStyleParameters::cfgName()
{
    return m_cfgName;
}

ReadThemeConfig* LINGMOConfigStyleParameters::readCfg()
{
    if(!m_readCfg){
        m_readCfg = new ReadThemeConfig("", false);
        m_adjustColorRules = m_readCfg->getColorValueAdjustRules() == "HSL" ? AdjustColorRules::HSL : AdjustColorRules::DefaultRGB;
    }
    return m_readCfg;
}

LINGMORadiusInformation::LINGMORadiusStruct LINGMOConfigStyleParameters::radiusProperty()
{
    return m_radiusStruct;
}

int LINGMOConfigStyleParameters::getAdjustColorRules()
{
    return m_adjustColorRules;
}

bool LINGMOConfigStyleParameters::colorIsSame(QColor oldColor, QColor newColor)
{
    return (oldColor == newColor);
}

void LINGMOConfigStyleParameters::initConfigPushButtonBevelParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    bool isWindowButton = false;
    bool isWindowColoseButton = false;
    bool isImportant = false;
    bool useButtonPalette = false;
    bool needTranslucent = false;

    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::ButtonColorCfg buttonColorCfg = readCfg()->buttonColorCfg(palette);

    if (widget) {
        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (widget->property("needTranslucent").isValid()) {
            needTranslucent = widget->property("needTranslucent").toBool();
        }
    }
    if (isWindowColoseButton)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_WindowCloseButton);
    if (isWindowButton)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_WindowButton);
    if (useButtonPalette || isWindowButton) {
        if (needTranslucent)
            buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_TranslucentButton);
        else
            buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_UseButtonPalette);
    }
    if (isImportant)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_ImportButton);

    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    QBrush defaultBrush = QBrush(buttonColorCfg.defaultBrush);
    QBrush disableBrush = QBrush(buttonColorCfg.disableBrush);
    QBrush clickBrush;
    QBrush hoverBrush;
    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;
    linearGradient.setStart(option ? option->rect.topLeft() : QPoint(0, 0));
    linearGradient.setFinalStop(option ? option->rect.bottomLeft() :QPoint(0, 0));

    QColor highlight = palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

    QPen defaultPen     ;
    QPen hoverPen       ;
    QPen clickPen       ;
    QPen disablePen     ;
    QPen checkPen       ;
    QPen checkHoverPen  ;
    QPen checkClickPen  ;
    QPen checkDisablePen;
    QPen focusPen;

    QBrush checkBrush;
    QBrush checkHoverBrush;
    QBrush checkClickBrush;
    QBrush checkDisableBrush;

    defaultPen   = QPen(QBrush(buttonColorCfg.defaultPen), (buttonColorCfg.defaultPen == Qt::NoPen || buttonColorCfg.defaultPen.alpha() == 0)
                        ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    hoverPen     = QPen(QBrush(buttonColorCfg.hoverPen), (buttonColorCfg.hoverPen == Qt::NoPen || buttonColorCfg.hoverPen.alpha() == 0)
                        ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    clickPen     = QPen(QBrush(buttonColorCfg.clickPen), (buttonColorCfg.clickPen == Qt::NoPen || buttonColorCfg.clickPen.alpha() == 0)
                        ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    disablePen   = QPen(QBrush(buttonColorCfg.disablePen), (buttonColorCfg.disablePen == Qt::NoPen || buttonColorCfg.disablePen.alpha() == 0)
                        ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    auto checkableColorCfg = readCfg()->buttonColorCfg(palette, C_Button_ImportButton);
    {
        checkPen   = QPen(QBrush(checkableColorCfg.defaultPen), (checkableColorCfg.defaultPen == Qt::NoPen || checkableColorCfg.defaultPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkHoverPen     = QPen(QBrush(checkableColorCfg.hoverPen), (checkableColorCfg.hoverPen == Qt::NoPen || checkableColorCfg.hoverPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkClickPen     = QPen(QBrush(checkableColorCfg.clickPen), (checkableColorCfg.clickPen == Qt::NoPen || checkableColorCfg.clickPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkDisablePen   = QPen(QBrush(checkableColorCfg.disablePen), (checkableColorCfg.disablePen == Qt::NoPen || checkableColorCfg.disablePen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        checkBrush = palette.color(QPalette::Active, QPalette::Highlight);
        checkHoverBrush = checkableColorCfg.hoverBrush;
        checkClickBrush = checkableColorCfg.clickBrush;
        checkDisableBrush = palette.brush(QPalette::Disabled, QPalette::NoRole);
    }

    if (isWindowColoseButton) {
        if (buttonColorCfg.hoverType == Obj_Gradient_Type){
            linearGradient.setColorAt(0, buttonColorCfg.hoverGradientList.value(0));
            linearGradient.setColorAt(1, buttonColorCfg.hoverGradientList.value(1));
            hoverBrush = QBrush(linearGradient);
        }
        else
            hoverBrush = buttonColorCfg.hoverBrush;
        clickBrush = buttonColorCfg.clickBrush;
        focusPen = QPen(QBrush(clickBrush), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    } else {
        switch (m_adjustColorRules) {
        case AdjustColorRules::DefaultRGB:{

            if (buttonColorCfg.hoverType == Obj_Gradient_Type) {
                if (isDark) {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.2);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.05);
                } else {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.05);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.2);
                }
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);
                clickBrush = QBrush(configMixColor(highlight, mix, 0.1));
            } else {
                if (isDark) {
                    hoverBrush = QBrush(configMixColor(highlight, mix, 0.2));
                } else {
                    hoverBrush = QBrush(configMixColor(highlight, mix, 0.05));
                }

                if (isDark) {
                    clickBrush = QBrush(configMixColor(highlight, mix, 0.05));
                } else {
                    clickBrush = QBrush(configMixColor(highlight, mix, 0.2));
                }
            }
            checkHoverBrush = hoverBrush;
            checkClickBrush = clickBrush;

            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {//特殊三联应用
                if (isDark) {
                    mix.setAlphaF(0.28);
                    hoverBrush = QBrush(mix);
                    mix.setAlphaF(0.15);
                    clickBrush = QBrush(mix);
                } else {
                    mix.setAlphaF(0.15);
                    hoverBrush = QBrush(mix);
                    mix.setAlphaF(0.28);
                    clickBrush = QBrush(mix);
                }
            } else if (useButtonPalette || isWindowButton) {
                if(buttonColorCfg.hoverType == Obj_Gradient_Type){
                    startColor = buttonColorCfg.hoverGradientList.value(0);
                    endColor = buttonColorCfg.hoverGradientList.value(1);
                    linearGradient.setColorAt(0, startColor);
                    linearGradient.setColorAt(1, endColor);
                    hoverBrush = QBrush(linearGradient);
                    clickBrush = buttonColorCfg.clickBrush;
                } else {
                    hoverBrush = buttonColorCfg.hoverBrush;
                    clickBrush = buttonColorCfg.clickBrush;
                }
            }

            focusPen = QPen(QBrush(configMixColor(highlight, mix, 0.2)), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        }break;
        case AdjustColorRules::HSL:{
            if (buttonColorCfg.focusPen != Qt::NoPen)
                focusPen = QPen(QBrush(buttonColorCfg.focusPen), (buttonColorCfg.focusPen == Qt::NoPen || buttonColorCfg.focusPen.alpha() == 0) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {//特殊窗口三联
                QColor p = QColor(255, 255, 255);
                p.setAlphaF(0.4);
                defaultPen = QPen(QColor(Qt::NoPen), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                hoverPen = QPen(p, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                p.setAlphaF(0.35);
                clickPen = QPen(p, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                disablePen = defaultPen;
                defaultBrush = QBrush(Qt::NoBrush);
                QColor bColor = QColor(0, 0, 0);
                bColor.setAlphaF(0.1);
                hoverBrush = QBrush(bColor);
                bColor.setAlphaF(0.2);
                clickBrush = QBrush(bColor);
                disableBrush = QBrush(Qt::NoBrush);
                focusPen = clickPen;
            }
            else if (!isWindowColoseButton) {
                normalButtonColor(isDark, palette, defaultPen, defaultBrush, hoverPen, hoverBrush,
                                  clickPen, clickBrush, disablePen, disableBrush);
                focusPen = QPen(palette.brush(QPalette::Inactive, QPalette::Highlight),
                                (buttonColorCfg.focusPen == Qt::NoPen || buttonColorCfg.focusPen.alpha() == 0) ?
                                    0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                defaultPen   = QPen(QBrush(buttonColorCfg.defaultPen), (buttonColorCfg.defaultPen == Qt::NoPen || buttonColorCfg.defaultPen.alpha() == 0)
                                    ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                disablePen   = QPen(QBrush(buttonColorCfg.disablePen), (buttonColorCfg.disablePen == Qt::NoPen || buttonColorCfg.disablePen.alpha() == 0)
                                    ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

                checkHoverBrush = hoverBrush;
                checkClickBrush = clickBrush;

            } else {
                clickBrush = QBrush(buttonColorCfg.clickBrush);
                hoverBrush = QBrush(buttonColorCfg.hoverBrush);
            }
        }
        default:
            break;
        }
    }

    auto *w = const_cast<QWidget *>(widget);

    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",      defaultBrush);
        w->setProperty("clickBrush",        clickBrush);
        w->setProperty("hoverBrush",        hoverBrush);
        w->setProperty("disableBrush",      disableBrush);
        w->setProperty("checkBrush",        checkBrush);
        w->setProperty("checkHoverBrush",   checkHoverBrush);
        w->setProperty("checkClickBrush",   checkClickBrush);
        w->setProperty("checkDisableBrush", checkDisableBrush);
        w->setProperty("defaultPen",        defaultPen);
        w->setProperty("clickPen",          clickPen);
        w->setProperty("hoverPen",          hoverPen);
        w->setProperty("disablePen",        disablePen);
        w->setProperty("checkPen",          checkPen);
        w->setProperty("checkHoverPen",     checkHoverPen);
        w->setProperty("checkClickPen",     checkClickPen);
        w->setProperty("checkDisablePen",   checkDisablePen);
        w->setProperty("focusPen",          focusPen);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setClickBrush").isValid() && widget->property("setClickBrush").canConvert<QBrush>()) {
            clickBrush = widget->property("setClickBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setCheckBrush").isValid() && widget->property("setCheckBrush").canConvert<QBrush>()) {
            checkBrush = widget->property("setCheckBrush").value<QBrush>();
        }
        if (widget->property("setCheckHoverBrush").isValid() && widget->property("setCheckHoverBrush").canConvert<QBrush>()) {
            checkHoverBrush = widget->property("setCheckHoverBrush").value<QBrush>();
        }
        if (widget->property("setCheckClickBrush").isValid() && widget->property("setCheckClickBrush").canConvert<QBrush>()) {
            checkClickBrush = widget->property("setCheckClickBrush").value<QBrush>();
        }
        if (widget->property("setCheckDisableBrush").isValid() && widget->property("setCheckDisableBrush").canConvert<QBrush>()) {
            checkDisableBrush = widget->property("setCheckDisableBrush").value<QBrush>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setClickPen").isValid() && widget->property("setClickPen").canConvert<QPen>()) {
            clickPen = widget->property("setClickPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setCheckPen").isValid() && widget->property("setCheckPen").canConvert<QPen>()) {
            checkPen = widget->property("setCheckPen").value<QPen>();
        }
        if (widget->property("setCheckHoverPen").isValid() && widget->property("setCheckHoverPen").canConvert<QPen>()) {
            checkHoverPen = widget->property("setCheckHoverPen").value<QPen>();
        }
        if (widget->property("setCheckClickPen").isValid() && widget->property("setCheckClickPen").canConvert<QPen>()) {
            checkClickPen = widget->property("setCheckClickPen").value<QPen>();
        }
        if (widget->property("setCheckDisablePen").isValid() && widget->property("setCheckDisablePen").canConvert<QPen>()) {
            checkDisablePen = widget->property("setCheckDisablePen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_PushButtonParameters.radius                      = radius;
    m_PushButtonParameters.pushButtonDefaultBrush      = defaultBrush;
    m_PushButtonParameters.pushButtonClickBrush        = clickBrush;
    m_PushButtonParameters.pushButtonHoverBrush        = hoverBrush;
    m_PushButtonParameters.pushButtonDisableBrush      = disableBrush;
    m_PushButtonParameters.pushButtonCheckBrush        = checkBrush;
    m_PushButtonParameters.pushButtonCheckHoverBrush   = checkHoverBrush;
    m_PushButtonParameters.pushButtonCheckClickBrush   = checkClickBrush;
    m_PushButtonParameters.pushButtonCheckDisableBrush = checkDisableBrush;
    m_PushButtonParameters.pushButtonDefaultPen        = defaultPen;
    m_PushButtonParameters.pushButtonClickPen          = clickPen;
    m_PushButtonParameters.pushButtonHoverPen          = hoverPen;
    m_PushButtonParameters.pushButtonDisablePen        = disablePen;
    m_PushButtonParameters.pushButtonCheckPen          = checkPen;
    m_PushButtonParameters.pushButtonCheckHoverPen     = checkHoverPen;
    m_PushButtonParameters.pushButtonCheckClickPen     = checkClickPen;
    m_PushButtonParameters.pushButtonCheckDisablePen   = checkDisablePen;
    m_PushButtonParameters.pushButtonFocusPen          = focusPen;

}

void LINGMOConfigStyleParameters::getDefaultConfigPushButtonBevelParameters(ConfigPushButtonParameters *p, bool isDark)
{
    initConfigPushButtonBevelParameters(isDark, nullptr, nullptr);
    p->radius = m_PushButtonParameters.radius;
    p->pushButtonDefaultBrush      = m_PushButtonParameters.pushButtonDefaultBrush     ;
    p->pushButtonClickBrush        = m_PushButtonParameters.pushButtonClickBrush       ;
    p->pushButtonHoverBrush        = m_PushButtonParameters.pushButtonHoverBrush       ;
    p->pushButtonDisableBrush      = m_PushButtonParameters.pushButtonDisableBrush     ;
    p->pushButtonCheckBrush        = m_PushButtonParameters.pushButtonCheckBrush       ;
    p->pushButtonCheckHoverBrush   = m_PushButtonParameters.pushButtonCheckHoverBrush  ;
    p->pushButtonCheckClickBrush   = m_PushButtonParameters.pushButtonCheckClickBrush  ;
    p->pushButtonCheckDisableBrush = m_PushButtonParameters.pushButtonCheckDisableBrush;
    p->pushButtonDefaultPen        = m_PushButtonParameters.pushButtonDefaultPen       ;
    p->pushButtonClickPen          = m_PushButtonParameters.pushButtonClickPen         ;
    p->pushButtonHoverPen          = m_PushButtonParameters.pushButtonHoverPen         ;
    p->pushButtonDisablePen        = m_PushButtonParameters.pushButtonDisablePen       ;
    p->pushButtonCheckPen          = m_PushButtonParameters.pushButtonCheckPen         ;
    p->pushButtonCheckHoverPen     = m_PushButtonParameters.pushButtonCheckHoverPen    ;
    p->pushButtonCheckClickPen     = m_PushButtonParameters.pushButtonCheckClickPen    ;
    p->pushButtonCheckDisablePen   = m_PushButtonParameters.pushButtonCheckDisablePen  ;
    p->pushButtonFocusPen          = m_PushButtonParameters.pushButtonFocusPen         ;

}

void LINGMOConfigStyleParameters::initConfigPushButtonLabelParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    bool iconHighlight = false;
    bool textHighlight = false;

    QPalette palette = option ? option->palette : m_stylePalette;
    const QStyleOptionButton *button = option ? qstyleoption_cast<const QStyleOptionButton *>(option) : nullptr;
    bool isWindowButton = false;
    bool useButtonPalette = false;
    bool isImportant = false;
    bool isWindowColoseButton = false;
    bool needTranslucent = false;

    LINGMOColorTheme::ButtonColorCfg buttonColorCfg = readCfg()->buttonColorCfg(palette);

    if (widget) {
        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (widget->property("needTranslucent").isValid()) {
            needTranslucent = widget->property("needTranslucent").toBool();
        }
    }
    if (isWindowColoseButton)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_WindowCloseButton);
    if (isWindowButton)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_WindowButton);
    if (useButtonPalette || isWindowButton) {
        if (needTranslucent)
            buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_TranslucentButton);
        else
            buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_UseButtonPalette);
    }
    if (isImportant)
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_ImportButton);

    if(option && option->state & QStyle::State_On && option->state & QStyle::State_Enabled){
        buttonColorCfg = readCfg()->buttonColorCfg(palette, C_Button_ImportButton);
    }

    QColor textDefaultColor = buttonColorCfg.textDefaultColor;
    QColor textHoverColor   = buttonColorCfg.textHoverColor;
    QColor textDisableColor = buttonColorCfg.textDisableColor;

    //icon
    if (button && (isWindowButton || useButtonPalette) && !(button->state & QStyle::State_On)) {
        iconHighlight = false;
    } else {
        iconHighlight = true;
    }

    if(m_adjustColorRules == AdjustColorRules::HSL)
        iconHighlight = false;

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("iconHighlight",    iconHighlight);
        w->setProperty("textHighlight",    textHighlight);
        w->setProperty("textDefaultColor", textDefaultColor);
        w->setProperty("textHoverColor",   textHoverColor);
        w->setProperty("textDisableColor", textDisableColor);

    }
    
    if (widget) {
//        if(!widget->isEnabled())
//            textColor = palette.color(QPalette::Disabled, QPalette::ButtonText);

        if (widget->property("setIconHoverClickHighlight").isValid() && widget->property("setIconHoverClickHighlight").canConvert<bool>()) {
            iconHighlight = widget->property("setIconHoverClickHighlight").value<bool>();
        }
        if (widget->property("setTextHoverClickHighlight").isValid() && widget->property("setTextHoverClickHighlight").canConvert<bool>()) {
            textHighlight = widget->property("setTextHoverClickHighlight").value<bool>();
        }
        if (widget->property("setTextDefaultColor").isValid() && widget->property("setTextDefaultColor").canConvert<QColor>()) {
            textDefaultColor = widget->property("setTextDefaultColor").value<QColor>();
        }
        if (widget->property("setTextHoverColor").isValid() && widget->property("setTextHoverColor").canConvert<QColor>()) {
            textHoverColor = widget->property("setTextHoverColor").value<QColor>();
        }
        if (widget->property("setTextDisableColor").isValid() && widget->property("setTextDisableColor").canConvert<QColor>()) {
            textDisableColor = widget->property("setTextDisableColor").value<QColor>();
        }
    }

    m_PushButtonParameters.iconHoverClickHighlight = iconHighlight;
    m_PushButtonParameters.textHoverClickHighlight = textHighlight;
    m_PushButtonParameters.textDefaultColor        = textDefaultColor;
    m_PushButtonParameters.textHoverColor          = textHoverColor;
    m_PushButtonParameters.textDisableColor        = textDisableColor;
}

void LINGMOConfigStyleParameters::getDefaultConfigPushButtonLabelParameters(ConfigPushButtonParameters *p, bool isDark)
{
    initConfigPushButtonLabelParameters(isDark, nullptr, nullptr);
    p->iconHoverClickHighlight = m_PushButtonParameters.iconHoverClickHighlight;
    p->textHoverClickHighlight = m_PushButtonParameters.textHoverClickHighlight;
    p->textDefaultColor        = m_PushButtonParameters.textDefaultColor       ;
    p->textHoverColor          = m_PushButtonParameters.textHoverColor         ;
    p->textDisableColor        = m_PushButtonParameters.textDisableColor       ;
}

void LINGMOConfigStyleParameters::initConfigToolButtonPanelParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    bool isWindowButton = false;
    bool isWindowColoseButton = false;
    bool isImportant = false;
    bool useButtonPalette = false;
    bool needTranslucent = false;
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::ToolButtonColorCfg buttonColorCfg = readCfg()->toolButtonColorCfg(palette);

    if (widget) {
        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (widget->property("needTranslucent").isValid()) {
            needTranslucent = widget->property("needTranslucent").toBool();
        }
    }
    if (isWindowColoseButton)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_WindowCloseButton);
    if (isWindowButton)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_WindowButton);
    if (useButtonPalette || isWindowButton) {
        if (needTranslucent)
            buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_TranslucentButton);
        else
            buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_UseButtonPalette);
    }
    if (isImportant)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_ImportButton);

    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    QBrush defaultBrush = QBrush(buttonColorCfg.defaultBrush);
    QBrush disableBrush = QBrush(buttonColorCfg.disableBrush);
    QBrush clickBrush = QBrush(buttonColorCfg.clickBrush);
    QBrush hoverBrush = QBrush(buttonColorCfg.hoverBrush);
    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;
    linearGradient.setStart(option ? option->rect.topLeft() : QPoint(0, 0));
    linearGradient.setFinalStop(option ? option->rect.bottomLeft() : QPoint(0, 0));

    QColor highlight = palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

    QPen defaultPen     ;
    QPen hoverPen       ;
    QPen clickPen       ;
    QPen disablePen     ;
    QPen checkPen       ;
    QPen checkHoverPen  ;
    QPen checkClickPen  ;
    QPen checkDisablePen;
    QPen focusPen;
    QBrush checkBrush;
    QBrush checkHoverBrush;
    QBrush checkClickBrush;
    QBrush checkDisableBrush;

    defaultPen      = QPen(QBrush(buttonColorCfg.defaultPen), (buttonColorCfg.defaultPen == Qt::NoPen) ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    hoverPen        = QPen(QBrush(buttonColorCfg.hoverPen), (buttonColorCfg.hoverPen == Qt::NoPen) ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    clickPen        = QPen(QBrush(buttonColorCfg.clickPen), (buttonColorCfg.clickPen == Qt::NoPen) ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    disablePen      = QPen(QBrush(buttonColorCfg.disablePen), (buttonColorCfg.disablePen == Qt::NoPen) ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    auto checkableColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_ImportButton);

    {
        checkPen   = QPen(QBrush(checkableColorCfg.defaultPen), (checkableColorCfg.defaultPen == Qt::NoPen || checkableColorCfg.defaultPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkHoverPen     = QPen(QBrush(checkableColorCfg.hoverPen), (checkableColorCfg.hoverPen == Qt::NoPen || checkableColorCfg.hoverPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkClickPen     = QPen(QBrush(checkableColorCfg.clickPen), (checkableColorCfg.clickPen == Qt::NoPen || checkableColorCfg.clickPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        checkDisablePen   = QPen(QBrush(checkableColorCfg.disablePen), (checkableColorCfg.disablePen == Qt::NoPen || checkableColorCfg.disablePen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        checkBrush = palette.color(QPalette::Active, QPalette::Highlight);
        checkHoverBrush = checkableColorCfg.hoverBrush;
        checkClickBrush = checkableColorCfg.clickBrush;
        checkDisableBrush = palette.brush(QPalette::Disabled, QPalette::NoRole);
    }
    if (isWindowColoseButton){
        if (buttonColorCfg.hoverType == Obj_Gradient_Type) {
            linearGradient.setColorAt(0, buttonColorCfg.hoverGradientList.value(0));
            linearGradient.setColorAt(1, buttonColorCfg.hoverGradientList.value(1));
            hoverBrush = QBrush(linearGradient);
        } else {
            hoverBrush = buttonColorCfg.hoverBrush;
        }
        clickBrush = buttonColorCfg.clickBrush;
        focusPen = QPen(QBrush(clickBrush), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    } else {
        switch (m_adjustColorRules) {
        case AdjustColorRules::DefaultRGB:{
            if (buttonColorCfg.hoverType == Obj_Gradient_Type) {
                if (isDark) {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.2);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.05);
                } else {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.05);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.2);
                }
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);
                clickBrush = QBrush(configMixColor(highlight, mix, 0.1));
            }
            else {
                if (isDark) {
                    hoverBrush = QBrush(configMixColor(highlight, mix, 0.2));
                } else {
                    hoverBrush = QBrush(configMixColor(highlight, mix, 0.05));
                }
                if (isDark) {
                    clickBrush = QBrush(configMixColor(highlight, mix, 0.05));
                } else {
                    clickBrush = QBrush(configMixColor(highlight, mix, 0.2));
                }
            }

            checkHoverBrush = hoverBrush;
            checkClickBrush = clickBrush;

            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {//特殊三联应用
                if (isDark) {
                    mix.setAlphaF(0.28);
                    hoverBrush = QBrush(mix);
                    mix.setAlphaF(0.15);
                    clickBrush = QBrush(mix);
                } else {
                    mix.setAlphaF(0.15);
                    hoverBrush = QBrush(mix);
                    mix.setAlphaF(0.28);
                    clickBrush = QBrush(mix);
                }
            } else if (useButtonPalette || isWindowButton) {
                if(buttonColorCfg.hoverType == Obj_Gradient_Type){
                    startColor = buttonColorCfg.hoverGradientList.value(0);
                    endColor = buttonColorCfg.hoverGradientList.value(1);
                    linearGradient.setColorAt(0, startColor);
                    linearGradient.setColorAt(1, endColor);
                    hoverBrush = QBrush(linearGradient);
                    clickBrush = buttonColorCfg.clickBrush;
                } else {
                    hoverBrush = buttonColorCfg.hoverBrush;
                    clickBrush = buttonColorCfg.clickBrush;
                }
            }

            focusPen = QPen(QBrush(configMixColor(highlight, mix, 0.2)), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        } break;
        case AdjustColorRules::HSL:{
            if(buttonColorCfg.focusPen != Qt::NoPen)
                focusPen = QPen(QBrush(buttonColorCfg.focusPen), (buttonColorCfg.focusPen == Qt::NoPen || buttonColorCfg.focusPen.alpha() == 0)
                                ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {//特殊窗口三联
                QColor p = QColor(255, 255, 255);
                p.setAlphaF(0.4);
                defaultPen = QPen(QColor(Qt::NoPen), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                hoverPen = QPen(p, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                p.setAlphaF(0.35);
                clickPen = QPen(p, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                disablePen = defaultPen;
                defaultBrush = QBrush(Qt::NoBrush);
                QColor bColor = QColor(0, 0, 0);
                bColor.setAlphaF(0.1);
                hoverBrush = QBrush(bColor);
                bColor.setAlphaF(0.2);
                clickBrush = QBrush(bColor);
                disableBrush = QBrush(Qt::NoBrush);
                focusPen = clickPen;
            }
            else if (!isWindowColoseButton) {
                normalButtonColor(isDark, palette, defaultPen, defaultBrush, hoverPen, hoverBrush,
                                  clickPen, clickBrush, disablePen, disableBrush);
                focusPen = QPen(palette.brush(QPalette::Inactive, QPalette::Highlight),
                                (buttonColorCfg.focusPen == Qt::NoPen || buttonColorCfg.focusPen.alpha() == 0) ?
                                    0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                defaultPen   = QPen(QBrush(buttonColorCfg.defaultPen), (buttonColorCfg.defaultPen == Qt::NoPen || buttonColorCfg.defaultPen.alpha() == 0)
                                    ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                disablePen   = QPen(QBrush(buttonColorCfg.disablePen), (buttonColorCfg.disablePen == Qt::NoPen || buttonColorCfg.disablePen.alpha() == 0)
                                    ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

                checkHoverBrush = hoverBrush;
                checkClickBrush = clickBrush;

            } else {
                clickBrush = QBrush(buttonColorCfg.clickBrush);
                hoverBrush = QBrush(buttonColorCfg.hoverBrush);
            }
        }
        default:
            break;
        }
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",      defaultBrush);
        w->setProperty("clickBrush",        clickBrush);
        w->setProperty("hoverBrush",        hoverBrush);
        w->setProperty("disableBrush",      disableBrush);
        w->setProperty("checkBrush",        checkBrush);
        w->setProperty("checkHoverBrush",   checkHoverBrush);
        w->setProperty("checkClickBrush",   checkClickBrush);
        w->setProperty("checkDisableBrush", checkDisableBrush);
        w->setProperty("defaultPen",        defaultPen);
        w->setProperty("clickPen",          clickPen);
        w->setProperty("hoverPen",          hoverPen);
        w->setProperty("disablePen",        disablePen);
        w->setProperty("checkPen",          checkPen);
        w->setProperty("checkHoverPen",     checkHoverPen);
        w->setProperty("checkClickPen",     checkClickPen);
        w->setProperty("checkDisablePen",   checkDisablePen);
        w->setProperty("focusPen",          focusPen);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setClickBrush").isValid() && widget->property("setClickBrush").canConvert<QBrush>()) {
            clickBrush = widget->property("setClickBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setCheckBrush").isValid() && widget->property("setCheckBrush").canConvert<QBrush>()) {
            checkBrush = widget->property("setCheckBrush").value<QBrush>();
        }
        if (widget->property("setCheckHoverBrush").isValid() && widget->property("setCheckHoverBrush").canConvert<QBrush>()) {
            checkHoverBrush = widget->property("setCheckHoverBrush").value<QBrush>();
        }
        if (widget->property("setCheckClickBrush").isValid() && widget->property("setCheckClickBrush").canConvert<QBrush>()) {
            checkClickBrush = widget->property("setCheckClickBrush").value<QBrush>();
        }
        if (widget->property("setCheckDisableBrush").isValid() && widget->property("setCheckDisableBrush").canConvert<QBrush>()) {
            checkDisableBrush = widget->property("setCheckDisableBrush").value<QBrush>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setClickPen").isValid() && widget->property("setClickPen").canConvert<QPen>()) {
            clickPen = widget->property("setClickPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setCheckPen").isValid() && widget->property("setCheckPen").canConvert<QPen>()) {
            checkPen = widget->property("setCheckPen").value<QPen>();
        }
        if (widget->property("setCheckHoverPen").isValid() && widget->property("setCheckHoverPen").canConvert<QPen>()) {
            checkHoverPen = widget->property("setCheckHoverPen").value<QPen>();
        }
        if (widget->property("setCheckClickPen").isValid() && widget->property("setCheckClickPen").canConvert<QPen>()) {
            checkClickPen = widget->property("setCheckClickPen").value<QPen>();
        }
        if (widget->property("setCheckDisablePen").isValid() && widget->property("setCheckDisablePen").canConvert<QPen>()) {
            checkDisablePen = widget->property("setCheckDisablePen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_ToolButtonParameters.radius                      = radius;
    m_ToolButtonParameters.toolButtonDefaultBrush      = defaultBrush;
    m_ToolButtonParameters.toolButtonClickBrush        = clickBrush;
    m_ToolButtonParameters.toolButtonHoverBrush        = hoverBrush;
    m_ToolButtonParameters.toolButtonDisableBrush      = disableBrush;
    m_ToolButtonParameters.toolButtonCheckBrush        = checkBrush;
    m_ToolButtonParameters.toolButtonCheckHoverBrush   = checkHoverBrush;
    m_ToolButtonParameters.toolButtonCheckClickBrush   = checkClickBrush;
    m_ToolButtonParameters.toolButtonCheckDisableBrush = checkDisableBrush;
    m_ToolButtonParameters.toolButtonDefaultPen        = defaultPen;
    m_ToolButtonParameters.toolButtonClickPen          = clickPen;
    m_ToolButtonParameters.toolButtonHoverPen          = hoverPen;
    m_ToolButtonParameters.toolButtonDisablePen        = disablePen;
    m_ToolButtonParameters.toolButtonCheckPen          = checkPen;
    m_ToolButtonParameters.toolButtonCheckHoverPen     = checkHoverPen;
    m_ToolButtonParameters.toolButtonCheckClickPen     = checkClickPen;
    m_ToolButtonParameters.toolButtonCheckDisablePen   = checkDisablePen;
    m_ToolButtonParameters.toolButtonFocusPen          = focusPen;    
}

void LINGMOConfigStyleParameters::getDefaultConfigToolButtonPanelParameters(ConfigToolButtonParameters *p, bool isDark)
{
    initConfigToolButtonPanelParameters(isDark, nullptr, nullptr);
    p->radius                      = m_ToolButtonParameters.radius;
    p->toolButtonDefaultBrush      = m_ToolButtonParameters.toolButtonDefaultBrush     ;
    p->toolButtonClickBrush        = m_ToolButtonParameters.toolButtonClickBrush       ;
    p->toolButtonHoverBrush        = m_ToolButtonParameters.toolButtonHoverBrush       ;
    p->toolButtonDisableBrush      = m_ToolButtonParameters.toolButtonDisableBrush     ;
    p->toolButtonCheckBrush        = m_ToolButtonParameters.toolButtonCheckBrush       ;
    p->toolButtonCheckHoverBrush   = m_ToolButtonParameters.toolButtonCheckHoverBrush  ;
    p->toolButtonCheckClickBrush   = m_ToolButtonParameters.toolButtonCheckClickBrush  ;
    p->toolButtonCheckDisableBrush = m_ToolButtonParameters.toolButtonCheckDisableBrush;
    p->toolButtonDefaultPen        = m_ToolButtonParameters.toolButtonDefaultPen       ;
    p->toolButtonClickPen          = m_ToolButtonParameters.toolButtonClickPen         ;
    p->toolButtonHoverPen          = m_ToolButtonParameters.toolButtonHoverPen         ;
    p->toolButtonDisablePen        = m_ToolButtonParameters.toolButtonDisablePen       ;
    p->toolButtonCheckPen          = m_ToolButtonParameters.toolButtonCheckPen         ;
    p->toolButtonCheckHoverPen     = m_ToolButtonParameters.toolButtonCheckHoverPen    ;
    p->toolButtonCheckClickPen     = m_ToolButtonParameters.toolButtonCheckClickPen    ;
    p->toolButtonCheckDisablePen   = m_ToolButtonParameters.toolButtonCheckDisablePen  ;
    p->toolButtonFocusPen          = m_ToolButtonParameters.toolButtonFocusPen         ;

}

void LINGMOConfigStyleParameters::initConfigToolButtonLabelParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    bool iconHighlight = false;
    bool textHighlight = false;

    const QStyleOptionToolButton *button = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    bool isWindowButton = false;
    bool useButtonPalette = false;
    bool isImportant = false;
    bool isWindowColoseButton = false;
    bool needTranslucent = false;

    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::ToolButtonColorCfg buttonColorCfg = readCfg()->toolButtonColorCfg(palette);

    if (widget) {
        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (widget->property("needTranslucent").isValid()) {
            needTranslucent = widget->property("needTranslucent").toBool();
        }
    }
    if (isWindowColoseButton)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_WindowCloseButton);
    if (isWindowButton)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_WindowButton);
    if (useButtonPalette || isWindowButton) {
        if (needTranslucent)
            buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_TranslucentButton);
        else
            buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_UseButtonPalette);
    }
    if (isImportant)
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_ImportButton);

    if(option && option->state & QStyle::State_On && option->state & QStyle::State_Enabled){
        buttonColorCfg = readCfg()->toolButtonColorCfg(palette, C_Button_ImportButton);
    }

    QColor textDefaultColor = buttonColorCfg.textDefaultColor;
    QColor textHoverColor   = buttonColorCfg.textHoverColor;
    QColor textDisableColor = buttonColorCfg.textDisableColor;

    //icon
    if ((isWindowButton || useButtonPalette) && !(button->state & QStyle::State_On)) {
        iconHighlight = false;
    } else {
        iconHighlight = true;
    }

    if(m_adjustColorRules == AdjustColorRules::HSL)
        iconHighlight = false;

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("iconHighlight", iconHighlight);
        w->setProperty("textHighlight", textHighlight);
        w->setProperty("textDefaultColor", textDefaultColor);
        w->setProperty("textHoverColor",   textHoverColor);
        w->setProperty("textDisableColor", textDisableColor);
    }
    if (widget) {
//        if(!widget->isEnabled())
//            textColor = palette.color(QPalette::Disabled, QPalette::ButtonText);

        if (widget->property("setIconHoverClickHighlight").isValid() && widget->property("setIconHoverClickHighlight").canConvert<bool>()) {
            iconHighlight = widget->property("setIconHoverClickHighlight").value<bool>();
        }
        if (widget->property("setTextHoverClickHighlight").isValid() && widget->property("setTextHoverClickHighlight").canConvert<bool>()) {
            textHighlight = widget->property("setTextHoverClickHighlight").value<bool>();
        }
        if (widget->property("setTextDefaultColor").isValid() && widget->property("setTextDefaultColor").canConvert<QColor>()) {
            textDefaultColor = widget->property("setTextDefaultColor").value<QColor>();
        }
        if (widget->property("setTextHoverColor").isValid() && widget->property("setTextHoverColor").canConvert<QColor>()) {
            textHoverColor = widget->property("setTextHoverColor").value<QColor>();
        }
        if (widget->property("setTextDisableColor").isValid() && widget->property("setTextDisableColor").canConvert<QColor>()) {
            textDisableColor = widget->property("setTextDisableColor").value<QColor>();
        }
    }

    m_ToolButtonParameters.iconHoverClickHighlight = iconHighlight;
    m_ToolButtonParameters.textHoverClickHighlight = textHighlight;
    m_ToolButtonParameters.textDefaultColor        = textDefaultColor;
    m_ToolButtonParameters.textHoverColor          = textHoverColor;
    m_ToolButtonParameters.textDisableColor        = textDisableColor;
}

void LINGMOConfigStyleParameters::getDefaultConfigToolButtonLabelParameters(ConfigToolButtonParameters *p, bool isDark)
{
    initConfigToolButtonLabelParameters(isDark, nullptr, nullptr);
    p->iconHoverClickHighlight = m_ToolButtonParameters.iconHoverClickHighlight;
    p->textHoverClickHighlight = m_ToolButtonParameters.textHoverClickHighlight;
    p->textDefaultColor        = m_ToolButtonParameters.textDefaultColor       ;
    p->textHoverColor          = m_ToolButtonParameters.textHoverColor         ;
    p->textDisableColor        = m_ToolButtonParameters.textDisableColor       ;
}

void LINGMOConfigStyleParameters::initConfigLineEditParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
    bool needTranslucent = false;
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }

    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::LineEditColorCfg lineEditColorCfg = readCfg()->lineEditColorCfg(palette, needTranslucent ? C_LineEdit_Translucent : C_LineEdit_Default);

    QBrush defaultBrush = lineEditColorCfg.defaultBrush;
    QBrush hoverBrush = lineEditColorCfg.hoverBrush;
    QBrush focusBrush = lineEditColorCfg.focusBrush;
    QBrush disableBrush = lineEditColorCfg.disableBrush;

    QPen defaultPen = QPen(lineEditColorCfg.defaultPen, lineEditColorCfg.defaultPen.alpha() == 0 ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen = QPen(lineEditColorCfg.hoverPen, lineEditColorCfg.hoverPen.alpha() == 0 ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen = QPen(lineEditColorCfg.focusPen, m_adjustColorRules == 0 ? 2 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen disablePen = QPen(lineEditColorCfg.disablePen, lineEditColorCfg.disablePen.alpha() == 0 ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if(lineEditColorCfg.hoverType == Obj_Gradient_Type){
        QColor startColor;
        QColor endColor;
        QLinearGradient linearGradient;
        linearGradient.setStart(option ? option->rect.topLeft() : QPoint(0, 0));
        linearGradient.setFinalStop(option ? option->rect.bottomLeft() :QPoint(0, 0));
        startColor = lineEditColorCfg.hoverGradientList.value(0);
        endColor = lineEditColorCfg.hoverGradientList.value(1);

        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        hoverBrush = QBrush(linearGradient);
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",      defaultBrush);
        w->setProperty("focusBrush",        focusBrush);
        w->setProperty("hoverBrush",        hoverBrush);
        w->setProperty("disableBrush",      disableBrush);
        w->setProperty("defaultPen",        defaultPen);
        w->setProperty("hoverPen",          hoverPen);
        w->setProperty("disablePen",        disablePen);
        w->setProperty("focusPen",          focusPen);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setFocusBrush").isValid() && widget->property("setFocusBrush").canConvert<QBrush>()) {
            focusBrush = widget->property("setFocusBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
    }

    m_LineEditParameters.radius               = radius;
    m_LineEditParameters.lineEditDefaultBrush = defaultBrush;
    m_LineEditParameters.lineEditHoverBrush   = hoverBrush;
    m_LineEditParameters.lineEditFocusBrush   = focusBrush;
    m_LineEditParameters.lineEditDisableBrush = disableBrush;
    m_LineEditParameters.lineEditDefaultPen   = defaultPen;
    m_LineEditParameters.lineEditHoverPen     = hoverPen;
    m_LineEditParameters.lineEditFocusPen     = focusPen;
    m_LineEditParameters.lineEditDisablePen   = disablePen;
}

void LINGMOConfigStyleParameters::getDefaultConfigLineEditParameters(ConfigLineEditParameters *p, bool isDark)
{
    initConfigLineEditParameters(isDark, nullptr, nullptr);
    p->radius               = m_LineEditParameters.radius              ;
    p->lineEditDefaultBrush = m_LineEditParameters.lineEditDefaultBrush;
    p->lineEditHoverBrush   = m_LineEditParameters.lineEditHoverBrush  ;
    p->lineEditFocusBrush   = m_LineEditParameters.lineEditFocusBrush  ;
    p->lineEditDisableBrush = m_LineEditParameters.lineEditDisableBrush;
    p->lineEditDefaultPen   = m_LineEditParameters.lineEditDefaultPen  ;
    p->lineEditHoverPen     = m_LineEditParameters.lineEditHoverPen    ;
    p->lineEditFocusPen     = m_LineEditParameters.lineEditFocusPen    ;
    p->lineEditDisablePen   = m_LineEditParameters.lineEditDisablePen  ;
}

void LINGMOConfigStyleParameters::initConfigSpinBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool isHorizonLayout)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ? widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::SpinBoxColorCfg spinBoxColorCfg =
            readCfg()->spinBoxColorCfg(palette, C_SpinBox_Default, isHorizonLayout ? C_SpinBox_HorizonLayout : C_SpinBox_DefaultLayout);

    QBrush defaultBrush = spinBoxColorCfg.defaultBrush;
    QBrush hoverBrush   = spinBoxColorCfg.hoverBrush;
    QBrush focusBrush   = spinBoxColorCfg.focusBrush;
    QBrush disableBrush = spinBoxColorCfg.disableBrush;

    QPen defaultPen = QPen(spinBoxColorCfg.defaultPen, spinBoxColorCfg.defaultPen.alpha()<= 0 ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen   = QPen(spinBoxColorCfg.hoverPen, spinBoxColorCfg.defaultPen.alpha()<= 0 ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen   = QPen(spinBoxColorCfg.focusPen, m_adjustColorRules == 0 ?
                               2 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen disablePen = QPen(spinBoxColorCfg.disablePen, spinBoxColorCfg.defaultPen.alpha()<= 0 ?
                               0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QBrush upHoverBrush        = spinBoxColorCfg.upHoverBrush       ;
    QBrush upFocusHoverBrush   = spinBoxColorCfg.upFocusHoverBrush  ;
    QBrush upClickBrush        = spinBoxColorCfg.upClickBrush       ;
    QBrush upDefaultBrush      = spinBoxColorCfg.upDefaultBrush     ;
    QBrush downHoverBrush      = spinBoxColorCfg.downHoverBrush     ;
    QBrush downFocusHoverBrush = spinBoxColorCfg.downFocusHoverBrush;
    QBrush downClickBrush      = spinBoxColorCfg.downClickBrush     ;
    QBrush downDefaultBrush    = spinBoxColorCfg.downDefaultBrush   ;

    QPen upDefaultPen      = QPen(spinBoxColorCfg.upDefaultPen    , spinBoxColorCfg.upDefaultPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen upHoverPen        = QPen(spinBoxColorCfg.upHoverPen      , spinBoxColorCfg.upHoverPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen upFocusHoverPen   = QPen(spinBoxColorCfg.upFocusPen      , spinBoxColorCfg.upFocusPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen upClickPen        = QPen(spinBoxColorCfg.upClickPen      , spinBoxColorCfg.upClickPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen upDisablePen      = QPen(spinBoxColorCfg.upDisablePen    , spinBoxColorCfg.upDisablePen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen downDefaultPen    = QPen(spinBoxColorCfg.downDefaultPen  , spinBoxColorCfg.downDefaultPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen downHoverPen      = QPen(spinBoxColorCfg.downHoverPen    , spinBoxColorCfg.downHoverPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen downFocusHoverPen = QPen(spinBoxColorCfg.downFocusPen    , spinBoxColorCfg.downFocusPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen downClickPen      = QPen(spinBoxColorCfg.downClickPen    , spinBoxColorCfg.downClickPen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen downDisablePen    = QPen(spinBoxColorCfg.downDisablePen  , spinBoxColorCfg.downDisablePen.alpha()<= 0 ?
                                      0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        if (isHorizonLayout)
            hoverPen.setWidth(m_adjustColorRules == 0 ? 2 : 1);

        if(spinBoxColorCfg.hoverType == Obj_Gradient_Type) {
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0, 0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0, 0) : option->rect.bottomLeft());
            startColor = spinBoxColorCfg.hoverGradientList.value(0);
            endColor = spinBoxColorCfg.hoverGradientList.value(1);

            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            hoverBrush = QBrush(linearGradient);
        }
    }
        break;
    default:
        break;
    }       

    QColor highlight = palette.color(QPalette::Active, QPalette::Highlight);
    QColor button = palette.color(QPalette::Active, QPalette::Button);
    QColor mix = palette.color(QPalette::Active, QPalette::BrightText);
    QColor startColor;
    QColor endColor;

    QLinearGradient upLinearGradient;
    upLinearGradient.setStart(!option ? QPoint(0, 0) : QPoint(option->rect.left(), option->rect.top()));
    upLinearGradient.setFinalStop(!option ? QPoint(0, 0) : QPoint(option->rect.left(), option->rect.center().y()));
    QLinearGradient downLinearGradient;
    downLinearGradient.setStart(!option ? QPoint(0, 0) : QPoint(option->rect.left(), option->rect.center().y()));
    downLinearGradient.setFinalStop(!option ? QPoint(0, 0) : QPoint(option->rect.left(), option->rect.bottom()));

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        //up hover
        if (spinBoxColorCfg.upHoverBrushType == Obj_Gradient_Type) {
            startColor = spinBoxColorCfg.upHoverBrushGradientList.value(0);
            endColor = spinBoxColorCfg.upHoverBrushGradientList.value(1);
            upLinearGradient.setColorAt(0, startColor);
            upLinearGradient.setColorAt(1, endColor);
            upHoverBrush = QBrush(upLinearGradient);

            startColor = spinBoxColorCfg.downHoverBrushGradientList.value(0);
            endColor = spinBoxColorCfg.downHoverBrushGradientList.value(1);
            downLinearGradient.setColorAt(0, startColor);
            downLinearGradient.setColorAt(1, endColor);
            downHoverBrush = QBrush(downLinearGradient);
        }

        //up focus hover
        if(!isHorizonLayout){
            if (spinBoxColorCfg.upFocusHoverBrushType == Obj_Color_Type) {
                if (isDark) {
                    upFocusHoverBrush = QBrush(configMixColor(highlight, mix, 0.2));
                } else {
                    upFocusHoverBrush = QBrush(configMixColor(highlight, mix, 0.05));
                }
            } else if (spinBoxColorCfg.upFocusHoverBrushType == Obj_Gradient_Type) {

                if (isDark) {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.2);
                    endColor = configMixColor(highlight, QColor(Qt::white), 0.05);
                } else {
                    startColor = configMixColor(highlight, QColor(Qt::black), 0.05);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.2);
                }

                upLinearGradient.setColorAt(0, startColor);
                upLinearGradient.setColorAt(1, endColor);
                upFocusHoverBrush = QBrush(upLinearGradient);
            }

        //down focus hover
            if(spinBoxColorCfg.downFocusHoverBrushType == Obj_Color_Type) {
                if (isDark) {
                    downFocusHoverBrush = QBrush(configMixColor(highlight, mix, 0.2));
                } else {
                    downFocusHoverBrush = QBrush(configMixColor(highlight, mix, 0.05));
                }
            } else if (spinBoxColorCfg.downFocusHoverBrushType == Obj_Gradient_Type) {
                if (isDark) {
                    startColor = configMixColor(highlight, QColor(Qt::white), 0.2);
                    endColor = configMixColor(highlight, QColor(Qt::white), 0.05);
                } else {
                    startColor = configMixColor(highlight, QColor(Qt::black), 0.05);
                    endColor = configMixColor(highlight, QColor(Qt::black), 0.2);
                }

                downLinearGradient.setColorAt(0, startColor);
                downLinearGradient.setColorAt(1, endColor);
                downFocusHoverBrush = QBrush(downLinearGradient);
            }

            //click
            if (isDark) {
                    upClickBrush = QBrush(configMixColor(highlight, mix, 0.05));
                    downClickBrush = QBrush(configMixColor(highlight, mix, 0.05));

            } else {
                    upClickBrush = QBrush(configMixColor(highlight, mix, 0.2));
                    downClickBrush = QBrush(configMixColor(highlight, mix, 0.2));
            }
        }
    }
        break;
    case AdjustColorRules::HSL:{
        //up hover
        if (isDark) {
            upHoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            upHoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }
        upHoverBrush = QBrush(highlightHover(isDark, palette));

        //up focus hover
        upFocusHoverBrush = upHoverBrush;
        upFocusHoverPen = upHoverPen;

        //up click
        if (isDark) {
            upClickPen = QPen(palette.brush(QPalette::Active, QPalette::Highlight),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            upClickPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }
        upClickBrush = QBrush(highlightClick(isDark, palette));

        //up disable
        upDisablePen = disablePen;

        //down default
        downDefaultBrush = upDefaultBrush;

        //down hover
        downHoverBrush = upHoverBrush;
        downHoverPen = upHoverPen;

        //down focus hover
        downFocusHoverPen = upFocusHoverPen;
        downFocusHoverBrush = upFocusHoverBrush;

        //down click
        downClickBrush = upClickBrush;
        downClickPen = upClickPen;

        //down disable
        downDisablePen = upDisablePen;

    }

        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",        defaultBrush);
        w->setProperty("focusBrush",          focusBrush);
        w->setProperty("hoverBrush",          hoverBrush);
        w->setProperty("disableBrush",        disableBrush);
        w->setProperty("defaultPen",          defaultPen);
        w->setProperty("hoverPen",            hoverPen);
        w->setProperty("disablePen",          disablePen);
        w->setProperty("focusPen",            focusPen);
        w->setProperty("upDefaultBrush",      upDefaultBrush);
        w->setProperty("upFocusHoverBrush",   upFocusHoverBrush);
        w->setProperty("upClickBrush",        upClickBrush);
        w->setProperty("upHoverBrush",        upHoverBrush);
        w->setProperty("downDefaultBrush",    downDefaultBrush);
        w->setProperty("downHoverBrush",      downHoverBrush);
        w->setProperty("downFocusHoverBrush", downFocusHoverBrush);
        w->setProperty("downClickBrush",      downClickBrush);
        w->setProperty("upDefaultPen",        upDefaultPen);
        w->setProperty("upHoverPen",          upHoverPen);
        w->setProperty("upFocusHoverPen",     upFocusHoverPen);
        w->setProperty("upClickPen",          upClickPen);
        w->setProperty("upDisablePen",        upDisablePen);
        w->setProperty("downDefaultPen",      downDefaultPen);
        w->setProperty("downHoverPen",        downHoverPen);
        w->setProperty("downFocusHoverPen",   downFocusHoverPen);
        w->setProperty("downClickPen",        downClickPen);
        w->setProperty("downDisablePen",      downDisablePen);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setFocusBrush").isValid() && widget->property("setFocusBrush").canConvert<QBrush>()) {
            focusBrush = widget->property("setFocusBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setUpDefaultBrush").isValid() && widget->property("setUpDefaultBrush").canConvert<QBrush>()) {
            upDefaultBrush = widget->property("setUpDefaultBrush").value<QBrush>();
        }
        if (widget->property("setUpHoverBrush").isValid() && widget->property("setUpHoverBrush").canConvert<QBrush>()) {
            upHoverBrush = widget->property("setUpHoverBrush").value<QBrush>();
        }
        if (widget->property("setUpFocusHoverBrush").isValid() && widget->property("setUpFocusHoverBrush").canConvert<QBrush>()) {
            upFocusHoverBrush = widget->property("setUpFocusHoverBrush").value<QBrush>();
        }
        if (widget->property("setUpClickBrush").isValid() && widget->property("setUpClickBrush").canConvert<QBrush>()) {
            upClickBrush = widget->property("setUpClickBrush").value<QBrush>();
        }
        if (widget->property("setDownDefaultBrush").isValid() && widget->property("setDownDefaultBrush").canConvert<QBrush>()) {
            downDefaultBrush = widget->property("setDownDefaultBrush").value<QBrush>();
        }
        if (widget->property("setDownHoverBrush").isValid() && widget->property("setDownHoverBrush").canConvert<QBrush>()) {
            downHoverBrush = widget->property("setDownHoverBrush").value<QBrush>();
        }
        if (widget->property("setDownFocusHoverBrush").isValid() && widget->property("setDownFocusHoverBrush").canConvert<QBrush>()) {
            downFocusHoverBrush = widget->property("setDownFocusHoverBrush").value<QBrush>();
        }
        if (widget->property("setDownClickBrush").isValid() && widget->property("setDownClickBrush").canConvert<QBrush>()) {
            downClickBrush = widget->property("setDownClickBrush").value<QBrush>();
        }
        if (widget->property("setUpDefaultPen").isValid() && widget->property("setUpDefaultPen").canConvert<QPen>()) {
            upDefaultPen = widget->property("setUpDefaultPen").value<QPen>();
        }
        if (widget->property("setUpHoverPen").isValid() && widget->property("setUpHoverPen").canConvert<QPen>()) {
            upHoverPen = widget->property("setUpHoverPen").value<QPen>();
        }
        if (widget->property("setUpFocusHoverPen").isValid() && widget->property("setUpFocusHoverPen").canConvert<QPen>()) {
            upFocusHoverPen = widget->property("setUpFocusHoverPen").value<QPen>();
        }
        if (widget->property("setUpClickPen").isValid() && widget->property("setUpClickPen").canConvert<QPen>()) {
            upClickPen = widget->property("setUpClickPen").value<QPen>();
        }
        if (widget->property("setUpDisablePen").isValid() && widget->property("setUpDisablePen").canConvert<QPen>()) {
            upDisablePen = widget->property("setUpDisablePen").value<QPen>();
        }
        if (widget->property("setDownDefaultPen").isValid() && widget->property("setDownDefaultPen").canConvert<QPen>()) {
            downDefaultPen = widget->property("setDownDefaultPen").value<QPen>();
        }
        if (widget->property("setDownHoverPen").isValid() && widget->property("setDownHoverPen").canConvert<QPen>()) {
            downHoverPen = widget->property("setDownHoverPen").value<QPen>();
        }
        if (widget->property("setDownFocusHoverPen").isValid() && widget->property("setDownFocusHoverPen").canConvert<QPen>()) {
            downFocusHoverPen = widget->property("setDownFocusHoverPen").value<QPen>();
        }
        if (widget->property("setDownClickPen").isValid() && widget->property("setDownClickPen").canConvert<QPen>()) {
            downClickPen = widget->property("setDownClickPen").value<QPen>();
        }
        if (widget->property("setDownDisablePen").isValid() && widget->property("setDownDisablePen").canConvert<QPen>()) {
            downDisablePen = widget->property("setDownDisablePen").value<QPen>();
        }
    }

    m_SpinBoxParameters.radius = radius;
    m_SpinBoxParameters.spinBoxDefaultBrush = defaultBrush;
    m_SpinBoxParameters.spinBoxHoverBrush   = hoverBrush;
    m_SpinBoxParameters.spinBoxFocusBrush   = focusBrush;
    m_SpinBoxParameters.spinBoxDisableBrush = disableBrush;

    m_SpinBoxParameters.spinBoxDefaultPen = defaultPen;
    m_SpinBoxParameters.spinBoxHoverPen   = hoverPen;
    m_SpinBoxParameters.spinBoxFocusPen   = focusPen;
    m_SpinBoxParameters.spinBoxDisablePen = disablePen;

    m_SpinBoxParameters.spinBoxUpDefaultBrush      = upDefaultBrush;
    m_SpinBoxParameters.spinBoxUpHoverBrush        = upHoverBrush;
    m_SpinBoxParameters.spinBoxUpFocusHoverBrush   = upFocusHoverBrush;
    m_SpinBoxParameters.spinBoxUpClickBrush        = upClickBrush;
    m_SpinBoxParameters.spinBoxDownHoverBrush      = downHoverBrush;
    m_SpinBoxParameters.spinBoxDownFocusHoverBrush = downFocusHoverBrush;
    m_SpinBoxParameters.spinBoxDownClickBrush      = downClickBrush;
    m_SpinBoxParameters.spinBoxDownDefaultBrush    = downDefaultBrush;

    m_SpinBoxParameters.spinBoxUpDefaultPen    = upDefaultPen;
    m_SpinBoxParameters.spinBoxUpHoverPen      = upHoverPen;
    m_SpinBoxParameters.spinBoxUpFocusHoverPen = upFocusHoverPen;
    m_SpinBoxParameters.spinBoxUpClickPen      = upClickPen;
    m_SpinBoxParameters.spinBoxUpDisablePen    = upDisablePen;

    m_SpinBoxParameters.spinBoxDownDefaultPen    = downDefaultPen;
    m_SpinBoxParameters.spinBoxDownHoverPen      = downHoverPen;
    m_SpinBoxParameters.spinBoxDownFocusHoverPen = downFocusHoverPen;
    m_SpinBoxParameters.spinBoxDownClickPen      = downClickPen;
    m_SpinBoxParameters.spinBoxDownDisablePen    = downDisablePen;

    m_SpinBoxParameters.spinBoxUpIconHightPixMap = (m_adjustColorRules == 0 ? true : false);
    m_SpinBoxParameters.spinBoxDownIconHightPixMap = (m_adjustColorRules == 0 ? true : false);
}

void LINGMOConfigStyleParameters::getDefaultConfigSpinBoxParameters(ConfigSpinBoxParameters *p, bool isDark)
{
    initConfigSpinBoxParameters(isDark, nullptr, nullptr);
    p->radius                     = m_SpinBoxParameters.radius                     ;
    p->spinBoxDefaultBrush        = m_SpinBoxParameters.spinBoxDefaultBrush        ;
    p->spinBoxHoverBrush          = m_SpinBoxParameters.spinBoxHoverBrush          ;
    p->spinBoxFocusBrush          = m_SpinBoxParameters.spinBoxFocusBrush          ;
    p->spinBoxDisableBrush        = m_SpinBoxParameters.spinBoxDisableBrush        ;

    p->spinBoxDefaultPen          = m_SpinBoxParameters.spinBoxDefaultPen          ;
    p->spinBoxHoverPen            = m_SpinBoxParameters.spinBoxHoverPen            ;
    p->spinBoxFocusPen            = m_SpinBoxParameters.spinBoxFocusPen            ;
    p->spinBoxDisablePen          = m_SpinBoxParameters.spinBoxDisablePen          ;

    p->spinBoxUpDefaultBrush      = m_SpinBoxParameters.spinBoxUpDefaultBrush      ;
    p->spinBoxUpHoverBrush        = m_SpinBoxParameters.spinBoxUpHoverBrush        ;
    p->spinBoxUpFocusHoverBrush   = m_SpinBoxParameters.spinBoxUpFocusHoverBrush   ;
    p->spinBoxUpClickBrush        = m_SpinBoxParameters.spinBoxUpClickBrush        ;
    p->spinBoxDownHoverBrush      = m_SpinBoxParameters.spinBoxDownHoverBrush      ;
    p->spinBoxDownFocusHoverBrush = m_SpinBoxParameters.spinBoxDownFocusHoverBrush ;
    p->spinBoxDownClickBrush      = m_SpinBoxParameters.spinBoxDownClickBrush      ;
    p->spinBoxDownDefaultBrush    = m_SpinBoxParameters.spinBoxDownDefaultBrush    ;

    p->spinBoxUpDefaultPen        = m_SpinBoxParameters.spinBoxUpDefaultPen        ;
    p->spinBoxUpHoverPen          = m_SpinBoxParameters.spinBoxUpHoverPen          ;
    p->spinBoxUpFocusHoverPen     = m_SpinBoxParameters.spinBoxUpFocusHoverPen     ;
    p->spinBoxUpClickPen          = m_SpinBoxParameters.spinBoxUpClickPen          ;
    p->spinBoxUpDisablePen        = m_SpinBoxParameters.spinBoxUpDisablePen        ;

    p->spinBoxDownDefaultPen      = m_SpinBoxParameters.spinBoxDownDefaultPen      ;
    p->spinBoxDownHoverPen        = m_SpinBoxParameters.spinBoxDownHoverPen        ;
    p->spinBoxDownFocusHoverPen   = m_SpinBoxParameters.spinBoxDownFocusHoverPen   ;
    p->spinBoxDownClickPen        = m_SpinBoxParameters.spinBoxDownClickPen        ;
    p->spinBoxDownDisablePen      = m_SpinBoxParameters.spinBoxDownDisablePen      ;

    p->spinBoxUpIconHightPixMap   = m_SpinBoxParameters.spinBoxUpIconHightPixMap   ;
    p->spinBoxDownIconHightPixMap = m_SpinBoxParameters.spinBoxDownIconHightPixMap ;

}

void LINGMOConfigStyleParameters::initConfigComboBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::ComboBoxColorCfg comboBoxColorCfg = readCfg()->comboBoxColorCfg(palette, C_ComboBox_Default);

    QBrush defaultBrush = comboBoxColorCfg.defaultBrush;
    QBrush hoverBrush   = comboBoxColorCfg.hoverBrush;
    QBrush onBrush      = comboBoxColorCfg.onBrush;
    QBrush editBrush    = comboBoxColorCfg.editBrush;
    QBrush disableBrush = comboBoxColorCfg.disableBrush;

    QPen defaultPen = QPen(comboBoxColorCfg.defaultPen, (comboBoxColorCfg.defaultPen.alpha() == 0 || comboBoxColorCfg.defaultPen == Qt::NoPen)
                           ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen   = QPen(comboBoxColorCfg.hoverPen, (comboBoxColorCfg.hoverPen.alpha() == 0 || comboBoxColorCfg.hoverPen == Qt::NoPen)
                           ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onPen      = QPen(comboBoxColorCfg.onPen, (comboBoxColorCfg.onPen.alpha() == 0 || comboBoxColorCfg.onPen == Qt::NoPen)
                           ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen editPen    = QPen(comboBoxColorCfg.editPen, (m_adjustColorRules == AdjustColorRules::DefaultRGB)
                           ? 2 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen disablePen = QPen(comboBoxColorCfg.disablePen, (comboBoxColorCfg.disablePen.alpha() == 0 || comboBoxColorCfg.disablePen == Qt::NoPen)
                           ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen   = QPen(comboBoxColorCfg.focusPen, (comboBoxColorCfg.focusPen.alpha() == 0 || comboBoxColorCfg.focusPen == Qt::NoPen)
                           ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (m_adjustColorRules) {
    case AdjustColorRules::HSL:
        normalButtonColor(isDark, palette, defaultPen, defaultBrush, hoverPen, hoverBrush,
                          onPen, onBrush, disablePen, disableBrush);
        defaultPen   = QPen(QBrush(comboBoxColorCfg.defaultPen), (comboBoxColorCfg.defaultPen == Qt::NoPen || comboBoxColorCfg.defaultPen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        disablePen   = QPen(QBrush(comboBoxColorCfg.disablePen), (comboBoxColorCfg.disablePen == Qt::NoPen || comboBoxColorCfg.disablePen.alpha() == 0)
                            ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        //edit
        editPen = onPen;
        break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",   defaultBrush);
        w->setProperty("onBrush",        onBrush);
        w->setProperty("hoverBrush",     hoverBrush);
        w->setProperty("disableBrush",   disableBrush);
        w->setProperty("editBrush",      editBrush);
        w->setProperty("defaultPen",     defaultPen);
        w->setProperty("hoverPen",       hoverPen);
        w->setProperty("disablePen",     disablePen);
        w->setProperty("onPen",          onPen);
        w->setProperty("editPen",        editPen);
        w->setProperty("focusPen",       focusPen);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setOnBrush").isValid() && widget->property("setOnBrush").canConvert<QBrush>()) {
            onBrush = widget->property("setOnBrush").value<QBrush>();
        }
        if (widget->property("setBoxEditBrush").isValid() && widget->property("setBoxEditBrush").canConvert<QBrush>()) {
            editBrush = widget->property("setBoxEditBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setOnPen").isValid() && widget->property("setOnPen").canConvert<QPen>()) {
            onPen = widget->property("setOnPen").value<QPen>();
        }
        if (widget->property("setEditPen").isValid() && widget->property("setEditPen").canConvert<QPen>()) {
            editPen = widget->property("setEditPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_ComboBoxParameters.radius               = radius;

    m_ComboBoxParameters.comboBoxDefaultBrush = defaultBrush;
    m_ComboBoxParameters.comboBoxHoverBrush   = hoverBrush;
    m_ComboBoxParameters.comboBoxOnBrush      = onBrush;
    m_ComboBoxParameters.comboBoxEditBrush    = editBrush;
    m_ComboBoxParameters.comboBoxDisableBrush = disableBrush;

    m_ComboBoxParameters.comboBoxDefaultPen   = defaultPen;
    m_ComboBoxParameters.comboBoxHoverPen     = hoverPen;
    m_ComboBoxParameters.comboBoxOnPen        = onPen;
    m_ComboBoxParameters.comboBoxEditPen      = editPen;
    m_ComboBoxParameters.comboBoxDisablePen   = disablePen;
    m_ComboBoxParameters.comboBoxFocusPen     = focusPen;
}

void LINGMOConfigStyleParameters::getDefaultConfigComboBoxParameters(ConfigComboBoxParameters *p, bool isDark)
{
    initConfigComboBoxParameters(isDark, nullptr, nullptr);
    p->radius               = m_ComboBoxParameters.radius              ;
    p->comboBoxDefaultBrush = m_ComboBoxParameters.comboBoxDefaultBrush;
    p->comboBoxHoverBrush   = m_ComboBoxParameters.comboBoxHoverBrush  ;
    p->comboBoxOnBrush      = m_ComboBoxParameters.comboBoxOnBrush     ;
    p->comboBoxEditBrush    = m_ComboBoxParameters.comboBoxEditBrush   ;
    p->comboBoxDisableBrush = m_ComboBoxParameters.comboBoxDisableBrush;
    p->comboBoxDefaultPen   = m_ComboBoxParameters.comboBoxDefaultPen  ;
    p->comboBoxHoverPen     = m_ComboBoxParameters.comboBoxHoverPen    ;
    p->comboBoxOnPen        = m_ComboBoxParameters.comboBoxOnPen       ;
    p->comboBoxEditPen      = m_ComboBoxParameters.comboBoxEditPen     ;
    p->comboBoxDisablePen   = m_ComboBoxParameters.comboBoxDisablePen  ;
    p->comboBoxFocusPen     = m_ComboBoxParameters.comboBoxFocusPen    ;
}

void LINGMOConfigStyleParameters::initConfigListParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlight)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    QPalette palette = option ? option->palette : m_stylePalette;

    bool needTranslucent = false;
    bool highlightMode = highlight;
    if (widget && widget->property("highlightMode").isValid()) {
        highlightMode = widget->property("highlightMode").toBool();
    }
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }

    LINGMOColorTheme::ListViewColorCfg listViewColorCfg = readCfg()->listViewColorCfg(palette, highlightMode ?
                                 C_ListView_Highlight : (needTranslucent ? C_ListView_Translucent : C_ListView_Default));

    QBrush defaultBrush = listViewColorCfg.defaultBrush;
    QBrush hoverBrush   = listViewColorCfg.hoverBrush;
    QBrush selectBrush  = listViewColorCfg.selectBrush;
    QBrush disableBrush = listViewColorCfg.disableBrush;
    QPen textHoverPen   = QPen(listViewColorCfg.textHoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textSelectPen  = QPen(listViewColorCfg.textSelectPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textDisablePen = QPen(listViewColorCfg.textDisablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QPen hoverPen       = QPen(listViewColorCfg.hoverPen, listViewColorCfg.hoverPen.alpha() > 0 ?
                                   1 : 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen selectPen      = QPen(listViewColorCfg.selectPen, listViewColorCfg.selectPen.alpha() > 0 ?
                                   1 : 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen      = QPen(listViewColorCfg.focusPen, listViewColorCfg.focusPen.alpha() > 0 ?
                                   2 : 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:
        //hover
        if (listViewColorCfg.hoverType == Obj_Gradient_Type) {
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());

            startColor = listViewColorCfg.hoverGradientList.value(0);
            endColor = listViewColorCfg.hoverGradientList.value(1);
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            hoverBrush = QBrush(linearGradient);
        }
        break;
    case AdjustColorRules::HSL:
            hoverBrush = QBrush(highlightHover(isDark, palette));
            selectBrush = QBrush(highlightClick(isDark, palette));

            if(isDark) {
                hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                                1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            } else {
                hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                                1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
            if(isDark) {
                selectPen = QPen(QBrush(palette.color(QPalette::Active, QPalette::Highlight)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            } else {
                selectPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
        break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",   defaultBrush);
        w->setProperty("hoverBrush",     hoverBrush);
        w->setProperty("disableBrush",   disableBrush);
        w->setProperty("selectBrush",    selectBrush);
        w->setProperty("textHoverPen",   textHoverPen);
        w->setProperty("textSelectPen",  textSelectPen);
        w->setProperty("textDisablePen", textDisablePen);
        w->setProperty("hoverPen",       hoverPen);
        w->setProperty("selectPen",      selectPen);
        w->setProperty("focusPen",       focusPen);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setSelectBrush").isValid() && widget->property("setSelectBrush").canConvert<QBrush>()) {
            selectBrush = widget->property("setSelectBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setTextHoverPen").isValid() && widget->property("setTextHoverPen").canConvert<QPen>()) {
            textHoverPen = widget->property("setTextHoverPen").value<QPen>();
        }
        if (widget->property("setTextSelectPen").isValid() && widget->property("setTextSelectPen").canConvert<QPen>()) {
            textSelectPen = widget->property("setTextSelectPen").value<QPen>();
        }
        if (widget->property("setTextDisablePen").isValid() && widget->property("setTextDisablePen").canConvert<QPen>()) {
            textDisablePen = widget->property("setTextDisablePen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setSelectPen").isValid() && widget->property("setSelectPen").canConvert<QPen>()) {
            selectPen = widget->property("setSelectPen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_ListParameters.radius = radius;
    m_ListParameters.listDefaultBrush = defaultBrush;
    m_ListParameters.listHoverBrush   = hoverBrush;
    m_ListParameters.listSelectBrush  = selectBrush;
    m_ListParameters.listDisableBrush = disableBrush;
    m_ListParameters.listTextHoverPen = textHoverPen;
    m_ListParameters.listTextSelectPen  = textSelectPen;
    m_ListParameters.listTextDisablePen = textDisablePen;
    m_ListParameters.listHoverPen  = hoverPen;
    m_ListParameters.listSelectPen = selectPen;
    m_ListParameters.listFocusPen  = focusPen;
}

void LINGMOConfigStyleParameters::getDefaultConfigListParameters(ConfigListParameters *p, bool isDark)
{
    initConfigListParameters(isDark, nullptr, nullptr);

    p->radius             = m_ListParameters.radius             ;
    p->listDefaultBrush   = m_ListParameters.listDefaultBrush   ;
    p->listHoverBrush     = m_ListParameters.listHoverBrush     ;
    p->listSelectBrush    = m_ListParameters.listSelectBrush    ;
    p->listDisableBrush   = m_ListParameters.listDisableBrush   ;
    p->listTextHoverPen   = m_ListParameters.listTextHoverPen   ;
    p->listTextSelectPen  = m_ListParameters.listTextSelectPen  ;
    p->listTextDisablePen = m_ListParameters.listTextDisablePen ;
    p->listHoverPen       = m_ListParameters.listHoverPen       ;
    p->listSelectPen      = m_ListParameters.listSelectPen      ;
    p->listFocusPen       = m_ListParameters.listFocusPen       ;
}

void LINGMOConfigStyleParameters::getHighlightModeConfigListParameters(ConfigListParameters *p, bool isDark)
{
    initConfigListParameters(isDark, nullptr, nullptr, true);

    p->radius             = m_ListParameters.radius             ;
    p->listDefaultBrush   = m_ListParameters.listDefaultBrush   ;
    p->listHoverBrush     = m_ListParameters.listHoverBrush     ;
    p->listSelectBrush    = m_ListParameters.listSelectBrush    ;
    p->listDisableBrush   = m_ListParameters.listDisableBrush   ;
    p->listTextHoverPen   = m_ListParameters.listTextHoverPen   ;
    p->listTextSelectPen  = m_ListParameters.listTextSelectPen  ;
    p->listTextDisablePen = m_ListParameters.listTextDisablePen ;
    p->listHoverPen       = m_ListParameters.listHoverPen       ;
    p->listSelectPen      = m_ListParameters.listSelectPen      ;
    p->listFocusPen       = m_ListParameters.listFocusPen       ;
}

void LINGMOConfigStyleParameters::initConfigTreeParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlight)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    bool highlightMode = highlight;
    if (widget && widget->property("highlightMode").isValid()) {
        highlightMode = widget->property("highlightMode").toBool();
    }
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::TreeViewColorCfg treeViewColorCfg = readCfg()->treeViewColorCfg(palette,
                                highlightMode ? C_TreeView_HighLight : C_TreeView_Default);

    QBrush defaultBrush = treeViewColorCfg.defaultBrush;
    QBrush hoverBrush   = treeViewColorCfg.hoverBrush;
    QBrush selectBrush  = treeViewColorCfg.selectBrush;
    QBrush disableBrush = treeViewColorCfg.disableBrush;
    QPen textHoverPen   = QPen(treeViewColorCfg.textHoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textSelectPen  = QPen(treeViewColorCfg.textSelectPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textDisablePen = QPen(treeViewColorCfg.textDisablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen       = QPen(treeViewColorCfg.hoverPen, (treeViewColorCfg.hoverPen == Qt::NoPen || treeViewColorCfg.hoverPen.alpha() == 0)
                               ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen selectPen      = QPen(treeViewColorCfg.selectPen, (treeViewColorCfg.selectPen == Qt::NoPen || treeViewColorCfg.selectPen.alpha() == 0)
                               ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen      = QPen(treeViewColorCfg.focusPen, (treeViewColorCfg.focusPen == Qt::NoPen || treeViewColorCfg.focusPen.alpha() == 0)
                               ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QBrush branchDefaultBrush  = treeViewColorCfg.branchDefaultBrush;
    QBrush branchHoverBrush    = treeViewColorCfg.branchHoverBrush;
    QBrush branchSelectBrush   = treeViewColorCfg.branchSelectBrush;
    QBrush branchDisableBrush  = treeViewColorCfg.branchDisableBrush;

    branchHoverBrush = hoverBrush;
    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        //hover
        if (treeViewColorCfg.hoverType == Obj_Gradient_Type) {
            QLinearGradient linearGradient;
            QColor startColor;
            QColor endColor;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            startColor = treeViewColorCfg.hoverGradientList.value(0);
            endColor = treeViewColorCfg.hoverGradientList.value(1);
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            hoverBrush = QBrush(linearGradient);
            branchHoverBrush = QBrush(linearGradient);
        }
    }break;
    case AdjustColorRules::HSL:{
        hoverBrush = QBrush(highlightHover(isDark, palette));
        branchHoverBrush = hoverBrush;
        selectBrush = QBrush(highlightClick(isDark, palette));

        if(isDark) {
            hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }

        if(isDark) {
            selectPen = QPen(QBrush(palette.color(QPalette::Active, QPalette::Highlight)),
                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            selectPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }
    }break;
    default:
        break;
    }

    branchDefaultBrush = defaultBrush;
    branchSelectBrush = selectBrush;
    branchDisableBrush = disableBrush;

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",       defaultBrush);
        w->setProperty("hoverBrush",         hoverBrush);
        w->setProperty("disableBrush",       disableBrush);
        w->setProperty("selectBrush",        selectBrush);
        w->setProperty("branchDefaultBrush", branchDefaultBrush);
        w->setProperty("branchHoverBrush",   branchHoverBrush);
        w->setProperty("branchSelectBrush",  branchSelectBrush);
        w->setProperty("branchDisableBrush", branchDisableBrush);
        w->setProperty("textHoverPen",       textHoverPen);
        w->setProperty("textSelectPen",      textSelectPen);
        w->setProperty("textDisablePen",     textDisablePen);
        w->setProperty("hoverPen",           hoverPen);
        w->setProperty("selectPen",          selectPen);
        w->setProperty("focusPen",           focusPen);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setSelectBrush").isValid() && widget->property("setSelectBrush").canConvert<QBrush>()) {
            selectBrush = widget->property("setSelectBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setBranchDefaultBrush").isValid() && widget->property("setBranchDefaultBrush").canConvert<QBrush>()) {
            branchDefaultBrush = widget->property("setBranchDefaultBrush").value<QBrush>();
        }
        if (widget->property("setBranchHoverBrush").isValid() && widget->property("setBranchHoverBrush").canConvert<QBrush>()) {
            branchHoverBrush = widget->property("setBranchHoverBrush").value<QBrush>();
        }
        if (widget->property("setBranchSelectBrush").isValid() && widget->property("setBranchSelectBrush").canConvert<QBrush>()) {
            branchSelectBrush = widget->property("setBranchSelectBrush").value<QBrush>();
        }
        if (widget->property("setBranchDisableBrush").isValid() && widget->property("setBranchDisableBrush").canConvert<QBrush>()) {
            branchDisableBrush = widget->property("setBranchDisableBrush").value<QBrush>();
        }
        if (widget->property("setTextHoverPen").isValid() && widget->property("setTextHoverPen").canConvert<QPen>()) {
            textHoverPen = widget->property("setTextHoverPen").value<QPen>();
        }
        if (widget->property("setTextSelectPen").isValid() && widget->property("setTextSelectPen").canConvert<QPen>()) {
            textSelectPen = widget->property("setTextSelectPen").value<QPen>();
        }
        if (widget->property("setTextDisablePen").isValid() && widget->property("setTextDisablePen").canConvert<QPen>()) {
            textDisablePen = widget->property("setTextDisablePen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setSelectPen").isValid() && widget->property("setSelectPen").canConvert<QPen>()) {
            selectPen = widget->property("setSelectPen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_TreeParameters.radius           = radius;
    m_TreeParameters.treeDefaultBrush = defaultBrush;
    m_TreeParameters.treeHoverBrush   = hoverBrush;
    m_TreeParameters.treeSelectBrush  = selectBrush;
    m_TreeParameters.treeDisableBrush = disableBrush;

    m_TreeParameters.treeBranchDefaultBrush = branchDefaultBrush;
    m_TreeParameters.treeBranchHoverBrush   =  branchHoverBrush;
    m_TreeParameters.treeBranchSelectBrush  =  branchSelectBrush;
    m_TreeParameters.treeBranchDisableBrush =  branchDisableBrush;

    m_TreeParameters.treeHoverPen       = hoverPen;
    m_TreeParameters.treeSelectPen      = selectPen;
    m_TreeParameters.treeFocusPen       = focusPen;
    m_TreeParameters.treeTextHoverPen   = textHoverPen;
    m_TreeParameters.treeTextSelectPen  = textSelectPen;
    m_TreeParameters.treeTextDisablePen = textDisablePen;
}

void LINGMOConfigStyleParameters::getDefaultConfigTreeParameters(ConfigTreeParameters *p, bool isDark)
{
    initConfigTreeParameters(isDark, nullptr, nullptr);

    p->radius                 = m_TreeParameters.radius                ;
    p->treeDefaultBrush       = m_TreeParameters.treeDefaultBrush      ;
    p->treeHoverBrush         = m_TreeParameters.treeHoverBrush        ;
    p->treeSelectBrush        = m_TreeParameters.treeSelectBrush       ;
    p->treeDisableBrush       = m_TreeParameters.treeDisableBrush      ;
    p->treeBranchDefaultBrush = m_TreeParameters.treeBranchDefaultBrush;
    p->treeBranchHoverBrush   = m_TreeParameters.treeBranchHoverBrush  ;
    p->treeBranchSelectBrush  = m_TreeParameters.treeBranchSelectBrush ;
    p->treeBranchDisableBrush = m_TreeParameters.treeBranchDisableBrush;
    p->treeHoverPen           = m_TreeParameters.treeHoverPen          ;
    p->treeSelectPen          = m_TreeParameters.treeSelectPen         ;
    p->treeFocusPen           = m_TreeParameters.treeFocusPen          ;
    p->treeTextHoverPen       = m_TreeParameters.treeTextHoverPen      ;
    p->treeTextSelectPen      = m_TreeParameters.treeTextSelectPen     ;
    p->treeTextDisablePen     = m_TreeParameters.treeTextDisablePen    ;
}

void LINGMOConfigStyleParameters::getHighlightModeConfigTreeParameters(ConfigTreeParameters *p, bool isDark)
{
    initConfigTreeParameters(isDark, nullptr, nullptr, true);

    p->radius                 = m_TreeParameters.radius                ;
    p->treeDefaultBrush       = m_TreeParameters.treeDefaultBrush      ;
    p->treeHoverBrush         = m_TreeParameters.treeHoverBrush        ;
    p->treeSelectBrush        = m_TreeParameters.treeSelectBrush       ;
    p->treeDisableBrush       = m_TreeParameters.treeDisableBrush      ;
    p->treeBranchDefaultBrush = m_TreeParameters.treeBranchDefaultBrush;
    p->treeBranchHoverBrush   = m_TreeParameters.treeBranchHoverBrush  ;
    p->treeBranchSelectBrush  = m_TreeParameters.treeBranchSelectBrush ;
    p->treeBranchDisableBrush = m_TreeParameters.treeBranchDisableBrush;
    p->treeHoverPen           = m_TreeParameters.treeHoverPen          ;
    p->treeSelectPen          = m_TreeParameters.treeSelectPen         ;
    p->treeFocusPen           = m_TreeParameters.treeFocusPen          ;
    p->treeTextHoverPen       = m_TreeParameters.treeTextHoverPen      ;
    p->treeTextSelectPen      = m_TreeParameters.treeTextSelectPen     ;
    p->treeTextDisablePen     = m_TreeParameters.treeTextDisablePen    ;
}

void LINGMOConfigStyleParameters::initConfigTableParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlight)
{
    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;


    bool highlightMode = highlight;
    if (widget && widget->property("highlightMode").isValid()) {
        highlightMode = widget->property("highlightMode").toBool();
    }
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::TableColorCfg tableColorCfg = readCfg()->tableColorCfg(palette,
                              highlightMode ? C_Table_Highlight : C_Table_Default);

    QBrush defaultBrush = tableColorCfg.defaultBrush;
    QBrush hoverBrush   = QBrush(tableColorCfg.hoverBrush);
    QBrush selectBrush  = tableColorCfg.selectBrush;
    QBrush disableBrush = tableColorCfg.disableBrush;
    QPen hoverPen       = QPen(tableColorCfg.hoverPen, (tableColorCfg.hoverPen.alpha() == 0 || tableColorCfg.hoverPen == Qt::NoPen)
                               ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen selectPen      = QPen(tableColorCfg.selectPen, (tableColorCfg.selectPen.alpha() == 0 || tableColorCfg.selectPen == Qt::NoPen)
                               ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textHoverPen   = QPen(tableColorCfg.textHoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textSelectPen  = QPen(tableColorCfg.textSelectPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen textDisablePen = QPen(tableColorCfg.textDisablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen       = QPen(tableColorCfg.focusPen, (tableColorCfg.focusPen.alpha() == 0 || tableColorCfg.focusPen == Qt::NoPen)
                               ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        if(tableColorCfg.hoverType == Obj_Gradient_Type) {
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            QColor startColor;
            QColor endColor;
            startColor = tableColorCfg.hoverGradientList.value(0);
            endColor = tableColorCfg.hoverGradientList.value(1);
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            hoverBrush = QBrush(linearGradient);
        }
    } break;
    case AdjustColorRules::HSL:{
        hoverBrush = QBrush(highlightHover(isDark, palette));
        selectBrush = QBrush(highlightClick(isDark, palette));

        if(isDark) {
            hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            hoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                            1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }

        if (isDark) {
            selectPen = QPen(QBrush(palette.color(QPalette::Active, QPalette::Highlight)),
                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        } else {
            selectPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 5, -21, 3)),
                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        }
    } break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",       defaultBrush);
        w->setProperty("hoverBrush",         hoverBrush);
        w->setProperty("disableBrush",       disableBrush);
        w->setProperty("selectBrush",        selectBrush);
        w->setProperty("textHoverPen",       textHoverPen);
        w->setProperty("textSelectPen",      textSelectPen);
        w->setProperty("textDisablePen",      textSelectPen);
        w->setProperty("hoverPen",           hoverPen);
        w->setProperty("selectPen",          selectPen);
        w->setProperty("focusPen",           focusPen);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setSelectBrush").isValid() && widget->property("setSelectBrush").canConvert<QBrush>()) {
            selectBrush = widget->property("setSelectBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setSelectPen").isValid() && widget->property("setSelectPen").canConvert<QPen>()) {
            selectPen = widget->property("setSelectPen").value<QPen>();
        }
        if (widget->property("setTextHoverPen").isValid() && widget->property("setTextHoverPen").canConvert<QPen>()) {
            textHoverPen = widget->property("setTextHoverPen").value<QPen>();
        }
        if (widget->property("setTextSelectPen").isValid() && widget->property("setTextSelectPen").canConvert<QPen>()) {
            textSelectPen = widget->property("setTextSelectPen").value<QPen>();
        }
        if (widget->property("setTextDisablePen").isValid() && widget->property("setTextDisablePen").canConvert<QPen>()) {
            textSelectPen = widget->property("setTextDisablePen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QPen>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_TableParameters.radius              = radius;
    m_TableParameters.tableDefaultBrush   = defaultBrush;
    m_TableParameters.tableHoverBrush     = hoverBrush;
    m_TableParameters.tableSelectBrush    = selectBrush;
    m_TableParameters.tableDisableBrush   = disableBrush;
    m_TableParameters.tableHoverPen       = hoverPen;
    m_TableParameters.tableSelectPen      = selectPen;
    m_TableParameters.tableFocusPen       = focusPen;
    m_TableParameters.tableTextHoverPen   = textHoverPen;
    m_TableParameters.tableTextSelectPen  = textSelectPen;
    m_TableParameters.tableTextDisablePen = textDisablePen;
}

void LINGMOConfigStyleParameters::getDefaultConfigTableParameters(ConfigTableParameters *p, bool isDark)
{
    initConfigTableParameters(isDark, nullptr, nullptr);

    p->radius              = m_TableParameters.radius             ;
    p->tableDefaultBrush   = m_TableParameters.tableDefaultBrush  ;
    p->tableHoverBrush     = m_TableParameters.tableHoverBrush    ;
    p->tableSelectBrush    = m_TableParameters.tableSelectBrush   ;
    p->tableDisableBrush   = m_TableParameters.tableDisableBrush  ;
    p->tableHoverPen       = m_TableParameters.tableHoverPen      ;
    p->tableSelectPen      = m_TableParameters.tableSelectPen     ;
    p->tableFocusPen       = m_TableParameters.tableFocusPen      ;
    p->tableTextHoverPen   = m_TableParameters.tableTextHoverPen  ;
    p->tableTextSelectPen  = m_TableParameters.tableTextSelectPen ;
    p->tableTextDisablePen = m_TableParameters.tableTextDisablePen;
}

void LINGMOConfigStyleParameters::getHighlightModeConfigTableParameters(ConfigTableParameters *p, bool isDark)
{
    initConfigTableParameters(isDark, nullptr, nullptr, true);

    p->radius              = m_TableParameters.radius             ;
    p->tableDefaultBrush   = m_TableParameters.tableDefaultBrush  ;
    p->tableHoverBrush     = m_TableParameters.tableHoverBrush    ;
    p->tableSelectBrush    = m_TableParameters.tableSelectBrush   ;
    p->tableDisableBrush   = m_TableParameters.tableDisableBrush  ;
    p->tableHoverPen       = m_TableParameters.tableHoverPen      ;
    p->tableSelectPen      = m_TableParameters.tableSelectPen     ;
    p->tableFocusPen       = m_TableParameters.tableFocusPen      ;
    p->tableTextHoverPen   = m_TableParameters.tableTextHoverPen  ;
    p->tableTextSelectPen  = m_TableParameters.tableTextSelectPen ;
    p->tableTextDisablePen = m_TableParameters.tableTextDisablePen;
}

void LINGMOConfigStyleParameters::initConfigCheckBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    bool needTranslucent = false;
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::CheckBoxColorCfg checkboxColorCfg = readCfg()->checkBoxColorCfg(palette,
                                      needTranslucent ? C_CheckBox_Translucent : C_CheckBox_Default);

    int radius = (widget && widget->property("minRadius").isValid()) ?
                widget->property("minRadius").toInt() : m_radiusStruct.minRadius;

    QPen defaultPen   = QPen(checkboxColorCfg.defaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen     = QPen(checkboxColorCfg.hoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen clickPen     = QPen(checkboxColorCfg.clickPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen disablePen   = QPen(checkboxColorCfg.disablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onDefaultPen = QPen(checkboxColorCfg.onDefaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onHoverPen   = QPen(checkboxColorCfg.onHoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onClickPen   = QPen(checkboxColorCfg.onClickPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen contentPen   = QPen(checkboxColorCfg.contentPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QBrush defaultBrush   = checkboxColorCfg.defaultBrush;
    QBrush hoverBrush     = checkboxColorCfg.hoverBrush;
    QBrush clickBrush     = checkboxColorCfg.clickBrush;
    QBrush disableBrush   = checkboxColorCfg.disableBrush;
    QBrush onDefaultBrush = checkboxColorCfg.onDefaultBrush;
    QBrush onHoverBrush   = checkboxColorCfg.onHoverBrush;
    QBrush onClickBrush   = checkboxColorCfg.onClickBrush;
    QBrush pathBrush      = checkboxColorCfg.pathBrush;
    QBrush pathDisableBrush = checkboxColorCfg.pathDisableBrush;

    bool onHoverBrushIslinearGradient = false;

    QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        if (checkboxColorCfg.hoverType == Obj_Gradient_Type) {
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            startColor = checkboxColorCfg.hoverGradientList.value(0);
            endColor   = checkboxColorCfg.hoverGradientList.value(1);
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            hoverBrush = QBrush(linearGradient);
        }

        if(checkboxColorCfg.onHoverType == Obj_Color_Type){
            onHoverBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, isDark? 0.2 : 0.05));
            onClickBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, isDark ? 0.05 : 0.2));
        } else if (checkboxColorCfg.onHoverType == Obj_Gradient_Type) {
            //on default
            QColor DefaultPenColor = palette.color(QPalette::Active, QPalette::Highlight);
            QColor onDefaultPenColor = configMixColor(DefaultPenColor, mix, 0.1);
            onDefaultPen = QPen(QBrush(onDefaultPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            onDefaultBrush = palette.brush(QPalette::Active, QPalette::Highlight);
            if(needTranslucent) {
                QColor onDefaultPenColor = palette.color(QPalette::Active, QPalette::BrightText);
                onDefaultPenColor.setAlphaF(0.1);
                onDefaultPen = QPen(QBrush(onDefaultPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
//            cDebug << "ondefaultbrush..." << onDefaultBrush.color().red() << onDefaultBrush.color().green() << onDefaultBrush.color().blue();
            //on hover
            QColor onHoverPenColor = configMixColor(DefaultPenColor, mix, 0.1);
            onHoverPen = QPen(QBrush(onHoverPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            if (isDark) {
                startColor = configMixColor(onDefaultBrush.color(), QColor(Qt::white), 0.2);
                endColor = configMixColor(onDefaultBrush.color(), QColor(Qt::black), 0.05);
            } else {
                startColor = configMixColor(onDefaultBrush.color(), QColor(Qt::white), 0.05);
                endColor = configMixColor(onDefaultBrush.color(), QColor(Qt::black), 0.2);
            }
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            onHoverBrush = QBrush(linearGradient);
            if(needTranslucent) {
                QColor onHoverPenColor = palette.color(QPalette::Active, QPalette::BrightText);
                onHoverPenColor.setAlphaF(0.2);
                onHoverPen = QPen(QBrush(onHoverPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }

            //on click
            QColor onClickPenColor = configMixColor(DefaultPenColor, mix, 0.1);
            onClickPen = QPen(QBrush(onClickPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            onClickBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, 0.1));
            if(needTranslucent) {
                QColor onClickPenColor = palette.color(QPalette::Active, QPalette::BrightText);
                onClickPenColor.setAlphaF(0.2);
                onClickPen = QPen(QBrush(onClickPenColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
            onHoverBrushIslinearGradient = true;
        }
        disablePen = defaultPen;
    } break;
    case AdjustColorRules::HSL:{
        normalButtonColor(isDark, palette, defaultPen, defaultBrush, hoverPen, hoverBrush,
                          clickPen, clickBrush, disablePen, disableBrush);
        defaultPen   = QPen(checkboxColorCfg.defaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        disablePen   = QPen(checkboxColorCfg.disablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);


        //on default
        onDefaultPen = defaultPen;
        onDefaultBrush = defaultBrush;

        //on hover
        onHoverPen = hoverPen;
        onHoverBrush = hoverBrush;

        //on click
        onClickPen = clickPen;
        onClickBrush = clickBrush;

    }
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",       defaultBrush);
        w->setProperty("hoverBrush",         hoverBrush);
        w->setProperty("clickBrush",         clickBrush);
        w->setProperty("disableBrush",       disableBrush);
        w->setProperty("onDefaultBrush",     onDefaultBrush);
        w->setProperty("onHoverBrush",       onHoverBrush);
        w->setProperty("onClickBrush",       onClickBrush);
        w->setProperty("pathDisableBrush",   pathDisableBrush);
        w->setProperty("pathBrush",          pathBrush);
        w->setProperty("contentPen",         contentPen);
        w->setProperty("defaultPen",         defaultPen);
        w->setProperty("disablePen",         disablePen);
        w->setProperty("hoverPen",           hoverPen);
        w->setProperty("clickPen",           clickPen);
        w->setProperty("onDefaultPen",       onDefaultPen);
        w->setProperty("onHoverPen",         onHoverPen);
        w->setProperty("onClickPen",         onClickPen);
        w->setProperty("onHoverBrushIslinearGradient",         onHoverBrushIslinearGradient);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setContentPen").isValid() && widget->property("setContentPen").canConvert<QPen>()) {
            contentPen = widget->property("setContentPen").value<QPen>();
        }
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setClickPen").isValid() && widget->property("setClickPen").canConvert<QPen>()) {
            clickPen = widget->property("setClickPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setOnDefaultPen").isValid() && widget->property("setOnDefaultPen").canConvert<QPen>()) {
            onDefaultPen = widget->property("setOnDefaultPen").value<QPen>();
        }
        if (widget->property("setOnHoverPen").isValid() && widget->property("setOnHoverPen").canConvert<QPen>()) {
            onHoverPen = widget->property("setOnHoverPen").value<QPen>();
        }
        if (widget->property("setOnClickPen").isValid() && widget->property("setOnClickPen").canConvert<QPen>()) {
            onClickPen = widget->property("setOnClickPen").value<QPen>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setClickBrush").isValid() && widget->property("setClickBrush").canConvert<QBrush>()) {
            clickBrush = widget->property("setClickBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setOnDefaultBrush").isValid() && widget->property("setOnDefaultBrush").canConvert<QBrush>()) {
            onDefaultBrush = widget->property("setOnDefaultBrush").value<QBrush>();
        }
        if (widget->property("setOnHoverBrush").isValid() && widget->property("setOnHoverBrush").canConvert<QBrush>()) {
            onHoverBrush = widget->property("setOnHoverBrush").value<QBrush>();
        }
        if (widget->property("setOnClickBrush").isValid() && widget->property("setOnClickBrush").canConvert<QBrush>()) {
            onClickBrush = widget->property("setOnClickBrush").value<QBrush>();
        }
        if (widget->property("setPathBrush").isValid() && widget->property("setPathBrush").canConvert<QBrush>()) {
            pathBrush = widget->property("setPathBrush").value<QBrush>();
        }
        if (widget->property("setPathDisableBrush").isValid() && widget->property("setPathDisableBrush").canConvert<QBrush>()) {
            pathDisableBrush = widget->property("setPathDisableBrush").value<QBrush>();
        }
        if (widget->property("setOnHoverBrushIslinearGradient").isValid() && widget->property("setOnHoverBrushIslinearGradient").canConvert<bool>()) {
            onHoverBrushIslinearGradient = widget->property("setOnHoverBrushIslinearGradient").value<bool>();
        }
    }

//    qDebug() << "defaultpen..." << defaultPen.color().red() << defaultPen.color().green() << defaultPen.color().blue();

//    qDebug() << "defaultbrush..." << defaultBrush.color().red() << defaultBrush.color().green() << defaultBrush.color().blue();
    m_CheckBoxParameters.radius = radius;
    m_CheckBoxParameters.checkBoxDefaultPen = defaultPen;
    m_CheckBoxParameters.checkBoxHoverPen = hoverPen;
    m_CheckBoxParameters.checkBoxClickPen = clickPen;
    m_CheckBoxParameters.checkBoxDisablePen = disablePen;
    m_CheckBoxParameters.checkBoxOnDefaultPen = onDefaultPen;
    m_CheckBoxParameters.checkBoxOnHoverPen = onHoverPen;
    m_CheckBoxParameters.checkBoxOnClickPen = onClickPen;
    m_CheckBoxParameters.checkBoxContentPen = contentPen;

    m_CheckBoxParameters.checkBoxDefaultBrush = defaultBrush;
    m_CheckBoxParameters.checkBoxHoverBrush = hoverBrush;
    m_CheckBoxParameters.checkBoxClickBrush = clickBrush;
    m_CheckBoxParameters.checkBoxDisableBrush = disableBrush;
    m_CheckBoxParameters.checkBoxOnDefaultBrush = onDefaultBrush;
    m_CheckBoxParameters.checkBoxOnHoverBrush = onHoverBrush;
    m_CheckBoxParameters.checkBoxOnClickBrush = onClickBrush;
    m_CheckBoxParameters.checkBoxPathBrush = pathBrush;
    m_CheckBoxParameters.checkBoxPathDisableBrush = pathDisableBrush;

    m_CheckBoxParameters.onHoverBrushIslinearGradient = onHoverBrushIslinearGradient;
}

void LINGMOConfigStyleParameters::getDefaultConfigCheckBoxParameters(ConfigCheckBoxParameters *p, bool isDark)
{
    initConfigCheckBoxParameters(isDark, nullptr, nullptr);

    p->radius                       = m_CheckBoxParameters.radius                      ;
    p->checkBoxDefaultPen           = m_CheckBoxParameters.checkBoxDefaultPen          ;
    p->checkBoxHoverPen             = m_CheckBoxParameters.checkBoxHoverPen            ;
    p->checkBoxClickPen             = m_CheckBoxParameters.checkBoxClickPen            ;
    p->checkBoxDisablePen           = m_CheckBoxParameters.checkBoxDisablePen          ;
    p->checkBoxOnDefaultPen         = m_CheckBoxParameters.checkBoxOnDefaultPen        ;
    p->checkBoxOnHoverPen           = m_CheckBoxParameters.checkBoxOnHoverPen          ;
    p->checkBoxOnClickPen           = m_CheckBoxParameters.checkBoxOnClickPen          ;
    p->checkBoxContentPen           = m_CheckBoxParameters.checkBoxContentPen          ;
    p->checkBoxDefaultBrush         = m_CheckBoxParameters.checkBoxDefaultBrush        ;
    p->checkBoxHoverBrush           = m_CheckBoxParameters.checkBoxHoverBrush          ;
    p->checkBoxClickBrush           = m_CheckBoxParameters.checkBoxClickBrush          ;
    p->checkBoxDisableBrush         = m_CheckBoxParameters.checkBoxDisableBrush        ;
    p->checkBoxOnDefaultBrush       = m_CheckBoxParameters.checkBoxOnDefaultBrush      ;
    p->checkBoxOnHoverBrush         = m_CheckBoxParameters.checkBoxOnHoverBrush        ;
    p->checkBoxOnClickBrush         = m_CheckBoxParameters.checkBoxOnClickBrush        ;
    p->checkBoxPathBrush            = m_CheckBoxParameters.checkBoxPathBrush           ;
    p->checkBoxPathDisableBrush     = m_CheckBoxParameters.checkBoxPathDisableBrush    ;
    p->onHoverBrushIslinearGradient = m_CheckBoxParameters.onHoverBrushIslinearGradient;
}

void LINGMOConfigStyleParameters::initConfigRadioButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    m_RadioButtonParameters.radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::RadioButtonColorCfg radioButtonColorCfg = readCfg()->radioButtonColorCfg(palette, C_RadioButton_Default);
    QPen defaultPen   = QPen(radioButtonColorCfg.defaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen hoverPen     = QPen(radioButtonColorCfg.hoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen clickPen     = QPen(radioButtonColorCfg.clickPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen disablePen   = QPen(radioButtonColorCfg.disablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onDefaultPen = QPen(radioButtonColorCfg.onDefaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onHoverPen   = QPen(radioButtonColorCfg.onHoverPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen onClickPen   = QPen(radioButtonColorCfg.onClickPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    QBrush defaultBrush           = radioButtonColorCfg.defaultBrush;
    QBrush hoverBrush             = radioButtonColorCfg.hoverBrush;
    QBrush clickBrush             = radioButtonColorCfg.clickBrush;
    QBrush disableBrush           = radioButtonColorCfg.disableBrush;
    QBrush onDefaultBrush         = radioButtonColorCfg.onDefaultBrush;
    QBrush onClickBrush           = radioButtonColorCfg.onClickBrush;
    QBrush childrenOnDefaultBrush = radioButtonColorCfg.childrenOnDefaultBrush;
    QBrush childrenOnHoverBrush   = radioButtonColorCfg.childrenOnHoverBrush;
    QBrush childrenOnClickBrush   = radioButtonColorCfg.childrenOnClickBrush;
    QBrush childrenOnDisableBrush = radioButtonColorCfg.childrenOnDisableBrush;
    QBrush onHoverBrush           = QBrush(radioButtonColorCfg.onHoverBrush);

    if (radioButtonColorCfg.onHoverType == Obj_Gradient_Type) {
        QLinearGradient linearGradient;
        linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
        linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
        linearGradient.setColorAt(0, radioButtonColorCfg.onHoverGradientList.value(0));
        linearGradient.setColorAt(1, radioButtonColorCfg.onHoverGradientList.value(1));
        onHoverBrush = QBrush(linearGradient);
    }

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

        if(option)
        m_RadioButtonParameters.childrenRadius =
                (qstyleoption_cast<const QStyleOptionButton*>(option)->rect.adjusted(1, 1, -1, -1).width()) / 2 - 2;

//        //hover
//        hoverPen = defaultPen;
//        if (isDark) {
//            hoverBrush = QBrush(configMixColor(defaultBrush.color(), mix, 0.15));
//        } else {
//            hoverBrush = QBrush(configMixColor(defaultBrush.color(), mix, 0.05));
//        }

//        //click
//        clickPen = defaultPen;
//        if (isDark) {
//            clickBrush = QBrush(configMixColor(defaultBrush.color(), mix, 0.1));
//        } else {
//            clickBrush = QBrush(configMixColor(defaultBrush.color(), mix, 0.15));
//        }

        onHoverPen = onDefaultPen;
        if (radioButtonColorCfg.onHoverType == Obj_Color_Type) {
            //on hover
            if (isDark) {
                onHoverBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, 0.2));
            } else {
                onHoverBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, 0.05));
            }
            //disable
//            disablePen = defaultPen;
        } else if (radioButtonColorCfg.onHoverType == Obj_Gradient_Type) {
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            if (isDark) {
                startColor = configMixColor(onDefaultBrush.color(), QColor(Qt::white), 0.2);
                endColor = configMixColor(onDefaultBrush.color(), QColor(Qt::black), 0.05);
            } else {
                startColor = configMixColor(onDefaultBrush.color(), QColor(Qt::white), 0.05);
                endColor = configMixColor(onDefaultBrush.color(), QColor(Qt::black), 0.2);
            }
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            onHoverBrush = QBrush(linearGradient);
        }

        //on click
        onClickPen = onDefaultPen;
        if (isDark) {
            onClickBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, 0.05));
        } else {
            onClickBrush = QBrush(configMixColor(onDefaultBrush.color(), mix, 0.2));
        }
        break;
    }
    case AdjustColorRules::HSL:{
        normalButtonColor(isDark, palette, defaultPen, defaultBrush, hoverPen, hoverBrush,
                          clickPen, clickBrush, disablePen, disableBrush);

        defaultPen   = QPen(radioButtonColorCfg.defaultPen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        disablePen   = QPen(radioButtonColorCfg.disablePen, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        //on default
        onDefaultPen = defaultPen;
        onDefaultBrush = defaultBrush;

        //on hover
        onHoverPen = hoverPen;
        onHoverBrush = hoverBrush;

        //on click
        onClickPen = clickPen;
        onClickBrush = clickBrush;

        if(option)
        m_RadioButtonParameters.childrenRadius = (qstyleoption_cast<const QStyleOptionButton*>(option)->rect.width()) / 2;
        break;
    }
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("defaultBrush",           defaultBrush);
        w->setProperty("hoverBrush",             hoverBrush);
        w->setProperty("clickBrush",             clickBrush);
        w->setProperty("disableBrush",           disableBrush);
        w->setProperty("onDefaultBrush",         onDefaultBrush);
        w->setProperty("onHoverBrush",           onHoverBrush);
        w->setProperty("onClickBrush",           onClickBrush);
        w->setProperty("childrenOnDefaultBrush", childrenOnDefaultBrush);
        w->setProperty("childrenOnHoverBrush",   childrenOnHoverBrush);
        w->setProperty("childrenOnClickBrush",   childrenOnClickBrush);
        w->setProperty("childrenOnDisableBrush", childrenOnDisableBrush);
        w->setProperty("defaultPen",             defaultPen);
        w->setProperty("disablePen",             disablePen);
        w->setProperty("hoverPen",               hoverPen);
        w->setProperty("clickPen",               clickPen);
        w->setProperty("onDefaultPen",           onDefaultPen);
        w->setProperty("onHoverPen",             onHoverPen);
        w->setProperty("onClickPen",             onClickPen);
    }

    //if set property
    if (widget) {
        if (widget->property("setDefaultPen").isValid() && widget->property("setDefaultPen").canConvert<QPen>()) {
            defaultPen = widget->property("setDefaultPen").value<QPen>();
        }
        if (widget->property("setHoverPen").isValid() && widget->property("setHoverPen").canConvert<QPen>()) {
            hoverPen = widget->property("setHoverPen").value<QPen>();
        }
        if (widget->property("setClickPen").isValid() && widget->property("setClickPen").canConvert<QPen>()) {
            clickPen = widget->property("setClickPen").value<QPen>();
        }
        if (widget->property("setDisablePen").isValid() && widget->property("setDisablePen").canConvert<QPen>()) {
            disablePen = widget->property("setDisablePen").value<QPen>();
        }
        if (widget->property("setOnDefaultPen").isValid() && widget->property("setOnDefaultPen").canConvert<QPen>()) {
            onDefaultPen = widget->property("setOnDefaultPen").value<QPen>();
        }
        if (widget->property("setOnHoverPen").isValid() && widget->property("setOnHoverPen").canConvert<QPen>()) {
            onHoverPen = widget->property("setOnHoverPen").value<QPen>();
        }
        if (widget->property("setOnClickPen").isValid() && widget->property("setOnClickPen").canConvert<QPen>()) {
            onClickPen = widget->property("setOnClickPen").value<QPen>();
        }
        if (widget->property("setDefaultBrush").isValid() && widget->property("setDefaultBrush").canConvert<QBrush>()) {
            defaultBrush = widget->property("setDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHoverBrush").isValid() && widget->property("setHoverBrush").canConvert<QBrush>()) {
            hoverBrush = widget->property("setHoverBrush").value<QBrush>();
        }
        if (widget->property("setClickBrush").isValid() && widget->property("setClickBrush").canConvert<QBrush>()) {
            clickBrush = widget->property("setClickBrush").value<QBrush>();
        }
        if (widget->property("setDisableBrush").isValid() && widget->property("setDisableBrush").canConvert<QBrush>()) {
            disableBrush = widget->property("setDisableBrush").value<QBrush>();
        }
        if (widget->property("setOnDefaultBrush").isValid() && widget->property("setOnDefaultBrush").canConvert<QBrush>()) {
            onDefaultBrush = widget->property("setOnDefaultBrush").value<QBrush>();
        }
        if (widget->property("setOnHoverBrush").isValid() && widget->property("setOnHoverBrush").canConvert<QBrush>()) {
            onHoverBrush = widget->property("setOnHoverBrush").value<QBrush>();
        }
        if (widget->property("setOnClickBrush").isValid() && widget->property("setOnClickBrush").canConvert<QBrush>()) {
            onClickBrush = widget->property("setOnClickBrush").value<QBrush>();
        }
        if (widget->property("setChildrenOnDefaultBrush").isValid() && widget->property("setChildrenOnDefaultBrush").canConvert<QBrush>()) {
            childrenOnDefaultBrush = widget->property("setChildrenOnDefaultBrush").value<QBrush>();
        }
        if (widget->property("setChildrenOnHoverBrush").isValid() && widget->property("setChildrenOnHoverBrush").canConvert<QBrush>()) {
            childrenOnHoverBrush = widget->property("setChildrenOnHoverBrush").value<QBrush>();
        }
        if (widget->property("setChildrenOnClickBrush").isValid() && widget->property("setChildrenOnClickBrush").canConvert<QBrush>()) {
            childrenOnClickBrush = widget->property("setChildrenOnClickBrush").value<QBrush>();
        }
        if (widget->property("setChildrenOnDisableBrush").isValid() && widget->property("setChildrenOnDisableBrush").canConvert<QBrush>()) {
            childrenOnDisableBrush = widget->property("setChildrenOnDisableBrush").value<QBrush>();
        }
    }

    m_RadioButtonParameters.radioButtonDefaultPen = defaultPen;
    m_RadioButtonParameters.radioButtonHoverPen = hoverPen;
    m_RadioButtonParameters.radioButtonClickPen = clickPen;
    m_RadioButtonParameters.radioButtonDisablePen = disablePen;
    m_RadioButtonParameters.radioButtonOnDefaultPen = onDefaultPen;
    m_RadioButtonParameters.radioButtonOnHoverPen = onHoverPen;
    m_RadioButtonParameters.radioButtonOnClickPen = onClickPen;

    m_RadioButtonParameters.radioButtonDefaultBrush = defaultBrush;
    m_RadioButtonParameters.radioButtonHoverBrush = hoverBrush;
    m_RadioButtonParameters.radioButtonClickBrush = clickBrush;
    m_RadioButtonParameters.radioButtonDisableBrush = disableBrush;
    m_RadioButtonParameters.radioButtonOnDefaultBrush = onDefaultBrush;
    m_RadioButtonParameters.radioButtonOnHoverBrush = onHoverBrush;
    m_RadioButtonParameters.radioButtonOnClickBrush = onClickBrush;

    m_RadioButtonParameters.radioButtonChildrenOnDefaultBrush = childrenOnDefaultBrush;
    m_RadioButtonParameters.radioButtonChildrenOnHoverBrush = childrenOnHoverBrush;
    m_RadioButtonParameters.radioButtonChildrenOnClickBrush = childrenOnClickBrush;
    m_RadioButtonParameters.radioButtonChildrenOnDisableBrush = childrenOnDisableBrush;
}

void LINGMOConfigStyleParameters::getDefaultConfigRadioButtonParameters(ConfigRadioButtonParameters *p, bool isDark)
{
    initConfigRadioButtonParameters(isDark, nullptr, nullptr);
    p->radioButtonDefaultPen             = m_RadioButtonParameters.radioButtonDefaultPen            ;
    p->radioButtonHoverPen               = m_RadioButtonParameters.radioButtonHoverPen              ;
    p->radioButtonClickPen               = m_RadioButtonParameters.radioButtonClickPen              ;
    p->radioButtonDisablePen             = m_RadioButtonParameters.radioButtonDisablePen            ;
    p->radioButtonOnDefaultPen           = m_RadioButtonParameters.radioButtonOnDefaultPen          ;
    p->radioButtonOnHoverPen             = m_RadioButtonParameters.radioButtonOnHoverPen            ;
    p->radioButtonOnClickPen             = m_RadioButtonParameters.radioButtonOnClickPen            ;
    p->radioButtonDefaultBrush           = m_RadioButtonParameters.radioButtonDefaultBrush          ;
    p->radioButtonHoverBrush             = m_RadioButtonParameters.radioButtonHoverBrush            ;
    p->radioButtonClickBrush             = m_RadioButtonParameters.radioButtonClickBrush            ;
    p->radioButtonDisableBrush           = m_RadioButtonParameters.radioButtonDisableBrush          ;
    p->radioButtonOnDefaultBrush         = m_RadioButtonParameters.radioButtonOnDefaultBrush        ;
    p->radioButtonOnHoverBrush           = m_RadioButtonParameters.radioButtonOnHoverBrush          ;
    p->radioButtonOnClickBrush           = m_RadioButtonParameters.radioButtonOnClickBrush          ;
    p->radioButtonChildrenOnDefaultBrush = m_RadioButtonParameters.radioButtonChildrenOnDefaultBrush;
    p->radioButtonChildrenOnHoverBrush   = m_RadioButtonParameters.radioButtonChildrenOnHoverBrush  ;
    p->radioButtonChildrenOnClickBrush   = m_RadioButtonParameters.radioButtonChildrenOnClickBrush  ;
    p->radioButtonChildrenOnDisableBrush = m_RadioButtonParameters.radioButtonChildrenOnDisableBrush;
}

void LINGMOConfigStyleParameters::initConfigSliderParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    int grooveRadius = Slider_GrooveLength/2;

    bool needTranslucent = false;
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }
    LINGMOColorTheme::SliderBarColorCfg sliderColorCfg = readCfg()->sliderBarColorCfg(palette,
                                        needTranslucent ? C_SliderBar_Translucent : C_SliderBar_Default);

    QBrush handleDefaultBrush   = sliderColorCfg.handleDefaultBrush;
    QBrush handleHoverBrush     = sliderColorCfg.handleHoverBrush;
    QBrush handleClickBrush     = sliderColorCfg.handleClickBrush;
    QBrush handleDisableBrush   = sliderColorCfg.handleDisableBrush;

    QBrush grooveValueDefaultBrush   = sliderColorCfg.grooveValueDefaultBrush;
    QBrush grooveValueHoverBrush     = sliderColorCfg.grooveValueHoverBrush;
    QBrush grooveValueDisableBrush   = sliderColorCfg.grooveValueDisableBrush;
    QBrush grooveUnvalueDefaultBrush = sliderColorCfg.grooveUnvalueDefaultBrush;
    QBrush grooveUnvalueHoverBrush   = sliderColorCfg.grooveUnvalueHoverBrush;
    QBrush grooveUnvalueDisableBrush = sliderColorCfg.grooveUnvalueDisableBrush;

    QPen handleDefaultPen  = QPen(sliderColorCfg.handleDefaultPen, (sliderColorCfg.handleDefaultPen.alpha() == 0 ||
                              sliderColorCfg.handleDefaultPen == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen handleHoverPen    = QPen(sliderColorCfg.handleHoverPen  , (sliderColorCfg.handleHoverPen.alpha() == 0 ||
                              sliderColorCfg.handleHoverPen   == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen handleClickPen    = QPen(sliderColorCfg.handleClickPen  , (sliderColorCfg.handleClickPen.alpha() == 0 ||
                              sliderColorCfg.handleClickPen   == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen handleDisablePen  = QPen(sliderColorCfg.handleDisablePen, (sliderColorCfg.handleDisablePen.alpha() == 0 ||
                              sliderColorCfg.handleDisablePen == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen sliderGroovePen   = QPen(sliderColorCfg.sliderGroovePen , (sliderColorCfg.sliderGroovePen.alpha() == 0 ||
                              sliderColorCfg.sliderGroovePen  == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen focusPen          = QPen(sliderColorCfg.focusPen , (sliderColorCfg.focusPen.alpha() == 0 ||
                              sliderColorCfg.focusPen         == Qt::NoPen) ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPainterPath handlePath;

    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option);
    const bool horizontal = slider ? slider->orientation == Qt::Horizontal : false;

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        QRect handleRect = option ? option->rect : QRect(0,0,0,0);
        handlePath.addEllipse(handleRect);

        QColor highlight = palette.color(QPalette::Active, QPalette::Highlight);
        QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

        if (sliderColorCfg.handleHoverType == Obj_Color_Type) {
            //handle hover
            if (isDark) {
                handleHoverBrush = QBrush(configMixColor(highlight, mix, 0.2));
            } else {
                handleHoverBrush = QBrush(configMixColor(highlight, mix, 0.05));
            }
        } else if (sliderColorCfg.handleHoverType == Obj_Gradient_Type) {
            //handle hover
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            if (!horizontal) {
                linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topRight());
                linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.topLeft());
            } else {
                linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
                linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            }

            if (isDark) {
                startColor = configMixColor(highlight, QColor(Qt::white), 0.2);
                endColor = configMixColor(highlight, QColor(Qt::black), 0.05);
            } else {
                startColor = configMixColor(highlight, QColor(Qt::white), 0.05);
                endColor = configMixColor(highlight, QColor(Qt::black), 0.2);
            }
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            handleHoverBrush = QBrush(linearGradient);
        }

        //handle click
        if (isDark) {
            handleClickBrush = QBrush(configMixColor(highlight, mix, 0.05));
        } else {
            handleClickBrush = QBrush(configMixColor(highlight, mix, 0.2));
        }

        //groove value defult
        if (isDark) {
            grooveValueDefaultBrush = QBrush(configMixColor(highlight, mix, 0.05));
        } else {
            grooveValueDefaultBrush = QBrush(configMixColor(highlight, mix, 0.2));
        }

        //groove value hover
        grooveValueHoverBrush = grooveValueDefaultBrush;

    } break;
    case AdjustColorRules::HSL:{
        if (isDark) {
            handleDefaultPen = adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15);
        } else {
            handleDefaultPen = adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 5, -21, 3);
        }
        handleDefaultPen.setWidth(1);
        handleHoverPen = handleDefaultPen;
        handleHoverPen.setWidth(1);
        handleHoverBrush = highlightHover(isDark, palette);

        //handle click
        handleClickPen = handleDefaultPen;
        handleHoverPen.setWidth(1);
        handleClickBrush = highlightClick(isDark, palette);

        if (isDark) {
            grooveValueDefaultBrush = adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15);
        } else {
            grooveValueDefaultBrush = palette.color(QPalette::Active, QPalette::Highlight);
        }
        //groove value hover
        grooveValueHoverBrush = grooveValueDefaultBrush;

        QRect handleRect = !option ? QRect(0,0,0,0) : option->rect;
        if (horizontal) {
            handlePath.moveTo(handleRect.left(), handleRect.top());
            handlePath.lineTo(handleRect.left(), handleRect.top() + (2 * (handleRect.height())/3));
            handlePath.lineTo(handleRect.center().x(), handleRect.bottom());
            handlePath.lineTo(handleRect.right(), handleRect.top() + (2 * (handleRect.height())/3));
            handlePath.lineTo(handleRect.right(), handleRect.top());
            handlePath.lineTo(handleRect.left(), handleRect.top());

        } else {
            handlePath.moveTo(handleRect.right(),handleRect.top());
            handlePath.lineTo(handleRect.right() - (2 * (handleRect.width())/3) , handleRect.top());
            handlePath.lineTo(handleRect.left(), handleRect.center().y() );
            handlePath.lineTo(handleRect.right() - (2 * (handleRect.width())/3), handleRect.bottom());
            handlePath.lineTo(handleRect.right(), handleRect.bottom());
            handlePath.lineTo(handleRect.right(),handleRect.top());
        }

        grooveRadius =  radiusProperty().normalRadius;
    } break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius",                    radius);
        w->setProperty("grooveRadius",              grooveRadius);
        w->setProperty("handleDefaultBrush",        handleDefaultBrush);
        w->setProperty("handleHoverBrush",          handleHoverBrush);
        w->setProperty("handleClickBrush",          handleClickBrush);
        w->setProperty("handleDisableBrush",        handleDisableBrush);
        w->setProperty("grooveValueDefaultBrush",   grooveValueDefaultBrush);
        w->setProperty("grooveValueHoverBrush",     grooveValueHoverBrush);
        w->setProperty("grooveValueDisableBrush",   grooveValueDisableBrush);
        w->setProperty("grooveUnvalueDefaultBrush", grooveUnvalueDefaultBrush);
        w->setProperty("grooveUnvalueHoverBrush",   grooveUnvalueHoverBrush);
        w->setProperty("grooveUnvalueDisableBrush", grooveUnvalueDisableBrush);
        w->setProperty("handleDefaultPen",          handleDefaultPen);
        w->setProperty("handleHoverPen",            handleHoverPen);
        w->setProperty("handleClickPen",            handleClickPen);
        w->setProperty("handleDisablePen",          handleDisablePen);
        w->setProperty("sliderGroovePen",           sliderGroovePen);
        w->setProperty("focusPen",                  focusPen);
    }

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setGrooveRadius").isValid() && widget->property("setGrooveRadius").canConvert<int>()) {
            grooveRadius = widget->property("setGrooveRadius").value<int>();
        }
        if (widget->property("setHandleDefaultBrush").isValid() && widget->property("setHandleDefaultBrush").canConvert<QBrush>()) {
            handleDefaultBrush = widget->property("setHandleDefaultBrush").value<QBrush>();
        }
        if (widget->property("setHandleHoverBrush").isValid() && widget->property("setHandleHoverBrush").canConvert<QBrush>()) {
            handleHoverBrush = widget->property("setHandleHoverBrush").value<QBrush>();
        }
        if (widget->property("setHandleClickBrush").isValid() && widget->property("setHandleClickBrush").canConvert<QBrush>()) {
            handleClickBrush = widget->property("setHandleClickBrush").value<QBrush>();
        }
        if (widget->property("setHandleDisableBrush").isValid() && widget->property("setHandleDisableBrush").canConvert<QBrush>()) {
            handleDisableBrush = widget->property("setHandleDisableBrush").value<QBrush>();
        }
        if (widget->property("setGrooveValueDefaultBrush").isValid() && widget->property("setGrooveValueDefaultBrush").canConvert<QBrush>()) {
            grooveValueDefaultBrush = widget->property("setGrooveValueDefaultBrush").value<QBrush>();
        }
        if (widget->property("setGrooveValueHoverBrush").isValid() && widget->property("setGrooveValueHoverBrush").canConvert<QBrush>()) {
            grooveValueHoverBrush = widget->property("setGrooveValueHoverBrush").value<QBrush>();
        }
        if (widget->property("setGrooveValueDisableBrush").isValid() && widget->property("setGrooveValueDisableBrush").canConvert<QBrush>()) {
            grooveValueDisableBrush = widget->property("setGrooveValueDisableBrush").value<QBrush>();
        }
        if (widget->property("setGrooveUnvalueDefaultBrush").isValid() && widget->property("setGrooveUnvalueDefaultBrush").canConvert<QBrush>()) {
            grooveUnvalueDefaultBrush = widget->property("setGrooveUnvalueDefaultBrush").value<QBrush>();
        }
        if (widget->property("setGrooveUnvalueHoverBrush").isValid() && widget->property("setGrooveUnvalueHoverBrush").canConvert<QBrush>()) {
            grooveUnvalueHoverBrush = widget->property("setGrooveUnvalueHoverBrush").value<QBrush>();
        }
        if (widget->property("setGrooveUnvalueDisableBrush").isValid() && widget->property("setGrooveUnvalueDisableBrush").canConvert<QBrush>()) {
            grooveUnvalueDisableBrush = widget->property("setGrooveUnvalueDisableBrush").value<QBrush>();
        }

        if (widget->property("setHandleDefaultPen").isValid() && widget->property("setHandleDefaultPen").canConvert<QBrush>()) {
            handleDefaultPen = widget->property("setHandleDefaultPen").value<QPen>();
        }
        if (widget->property("setHandleHoverPen").isValid() && widget->property("setHandleHoverPen").canConvert<QBrush>()) {
            handleHoverPen = widget->property("setHandleHoverPen").value<QPen>();
        }
        if (widget->property("setHandleClickPen").isValid() && widget->property("setHandleClickPen").canConvert<QBrush>()) {
            handleClickPen = widget->property("setHandleClickPen").value<QPen>();
        }
        if (widget->property("setHandleDisablePen").isValid() && widget->property("setHandleDisablePen").canConvert<QBrush>()) {
            handleDisablePen = widget->property("setHandleDisablePen").value<QPen>();
        }
        if (widget->property("setSliderGroovePen").isValid() && widget->property("setSliderGroovePen").canConvert<QBrush>()) {
            sliderGroovePen = widget->property("setSliderGroovePen").value<QPen>();
        }
        if (widget->property("setFocusPen").isValid() && widget->property("setFocusPen").canConvert<QBrush>()) {
            focusPen = widget->property("setFocusPen").value<QPen>();
        }
    }

    m_SliderParameters.radius = radius;
    m_SliderParameters.sliderGrooveRadius = grooveRadius;

    m_SliderParameters.sliderHandleDefaultBrush = handleDefaultBrush;
    m_SliderParameters.sliderHandleHoverBrush = handleHoverBrush;
    m_SliderParameters.sliderHandleClickBrush = handleClickBrush;
    m_SliderParameters.sliderHandleDisableBrush = handleDisableBrush;

    m_SliderParameters.sliderGrooveValueDefaultBrush = grooveValueDefaultBrush;
    m_SliderParameters.sliderGrooveValueHoverBrush   = grooveValueHoverBrush;
    m_SliderParameters.sliderGrooveValueDisableBrush = grooveValueDisableBrush;

    m_SliderParameters.sliderGrooveUnvalueDefaultBrush = grooveUnvalueDefaultBrush;
    m_SliderParameters.sliderGrooveUnvalueHoverBrush   = grooveUnvalueHoverBrush;
    m_SliderParameters.sliderGrooveUnvalueDisableBrush = grooveUnvalueDisableBrush;

    m_SliderParameters.sliderHandleDefaultPen = handleDefaultPen;
    m_SliderParameters.sliderHandleHoverPen   = handleHoverPen;
    m_SliderParameters.sliderHandleClickPen   = handleClickPen;
    m_SliderParameters.sliderHandleDisablePen = handleDisablePen;
    m_SliderParameters.sliderGroovePen        = sliderGroovePen;
    m_SliderParameters.focusPen               = focusPen;


    m_SliderParameters.sliderHandlePath = handlePath;
}

void LINGMOConfigStyleParameters::getDefaultSliderParameters(ConfigSliderParameters *p, bool isDark)
{
    initConfigSliderParameters(isDark, nullptr, nullptr);
    p->radius                          = m_SliderParameters.radius                         ;
    p->sliderGrooveRadius              = m_SliderParameters.sliderGrooveRadius             ;
    p->sliderHandleDefaultBrush        = m_SliderParameters.sliderHandleDefaultBrush       ;
    p->sliderHandleHoverBrush          = m_SliderParameters.sliderHandleHoverBrush         ;
    p->sliderHandleClickBrush          = m_SliderParameters.sliderHandleClickBrush         ;
    p->sliderHandleDisableBrush        = m_SliderParameters.sliderHandleDisableBrush       ;
    p->sliderGrooveValueDefaultBrush   = m_SliderParameters.sliderGrooveValueDefaultBrush  ;
    p->sliderGrooveValueHoverBrush     = m_SliderParameters.sliderGrooveValueHoverBrush    ;
    p->sliderGrooveValueDisableBrush   = m_SliderParameters.sliderGrooveValueDisableBrush  ;
    p->sliderGrooveUnvalueDefaultBrush = m_SliderParameters.sliderGrooveUnvalueDefaultBrush;
    p->sliderGrooveUnvalueHoverBrush   = m_SliderParameters.sliderGrooveUnvalueHoverBrush  ;
    p->sliderGrooveUnvalueDisableBrush = m_SliderParameters.sliderGrooveUnvalueDisableBrush;
    p->sliderHandleDefaultPen          = m_SliderParameters.sliderHandleDefaultPen         ;
    p->sliderHandleHoverPen            = m_SliderParameters.sliderHandleHoverPen           ;
    p->sliderHandleClickPen            = m_SliderParameters.sliderHandleClickPen           ;
    p->sliderHandleDisablePen          = m_SliderParameters.sliderHandleDisablePen         ;
    p->sliderGroovePen                 = m_SliderParameters.sliderGroovePen                ;
    p->focusPen                        = m_SliderParameters.focusPen                       ;
}

void LINGMOConfigStyleParameters::initConfigProgressBarParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    int normalRadius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
    int radius = widget ? (widget->height() / 2 > normalRadius ? normalRadius : widget->height() / 2) : normalRadius;

    bool needTranslucent = false;
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }
    LINGMOColorTheme::ProgressBarColorCfg progressColorCfg = readCfg()->progressBarColorCfg(palette,
                                           needTranslucent ? C_ProgressBar_Translucent : C_ProgressBar_Default);

    QBrush contentBrush = progressColorCfg.contentBrush;
    QBrush grooveDefaultBrush = progressColorCfg.grooveDefaultBrush;
    QBrush grooveDisableBrush = progressColorCfg.grooveDisableBrush;

    QPen contentPen       = QPen(progressColorCfg.contentPen , (progressColorCfg.contentPen.alpha() == 0 ||
                              progressColorCfg.contentPen  == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen grooveDefaultPen = QPen(progressColorCfg.grooveDefaultPen , (progressColorCfg.grooveDefaultPen.alpha() == 0 ||
                              progressColorCfg.grooveDefaultPen  == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen grooveDisablePen = QPen(progressColorCfg.grooveDisablePen , (progressColorCfg.grooveDisablePen.alpha() == 0 ||
                              progressColorCfg.grooveDisablePen  == Qt::NoPen) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    const bool vertical = pb ? pb->orientation == Qt::Vertical : false;
    const bool inverted = pb ? pb->invertedAppearance : false;

    bool reverse = (!vertical && pb && (pb->direction == Qt::RightToLeft)) || vertical;
    if (inverted)
        reverse = !reverse;
    QColor startColor = progressColorCfg.contentStartColor;
    QColor endColor = progressColorCfg.contentEndColor;

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:{
        //content
        startColor = progressColorCfg.contentStartColor;
        endColor= startColor.lighter(120);
        QLinearGradient linearGradient;
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        if (vertical) {
            if (reverse) {
                linearGradient.setStart(!pb ? QPoint(0, 0) : pb->rect.bottomLeft());
                linearGradient.setFinalStop(!pb ? QPoint(0, 0) : pb->rect.topLeft());
            } else {
                linearGradient.setStart(!pb ? QPoint(0, 0) : pb->rect.topLeft());
                linearGradient.setFinalStop(!pb ? QPoint(0, 0) : pb->rect.bottomLeft());
            }
        } else {
            if (reverse) {
                linearGradient.setStart(!pb ? QPoint(0, 0) : pb->rect.topRight());
                linearGradient.setFinalStop(pb->rect.topLeft());
            } else {
                linearGradient.setStart(!pb ? QPoint(0, 0) : pb->rect.topLeft());
                linearGradient.setFinalStop(!pb ? QPoint(0, 0) : pb->rect.topRight());
            }
        }
        contentBrush = QBrush(linearGradient);
    } break;
    case AdjustColorRules::HSL:
        //content
        if(isDark)
            contentBrush = QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), -1, 15, -14));

        endColor = contentBrush.color();
        startColor = contentBrush.color();

        break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("contentPen",              contentPen);
        w->setProperty("grooveDefaultPen",        grooveDefaultPen);
        w->setProperty("grooveDisablePen",        grooveDisablePen);
        w->setProperty("contentBrush",            contentBrush);
        w->setProperty("grooveDefaultBrush",      grooveDefaultBrush);
        w->setProperty("grooveDisableBrush",      grooveDisableBrush);
        w->setProperty("endColor",                endColor);
        w->setProperty("startColor",              startColor);
    }

    if(widget){
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setContentBrush").isValid() && widget->property("setContentBrush").canConvert<QBrush>()) {
            contentBrush = widget->property("setContentBrush").value<QBrush>();
        }
        if (widget->property("setGrooveDefaultBrush").isValid() && widget->property("setGrooveDefaultBrush").canConvert<QBrush>()) {
            grooveDefaultBrush = widget->property("setGrooveDefaultBrush").value<QBrush>();
        }
        if (widget->property("setGrooveDisableBrush").isValid() && widget->property("setGrooveDisableBrush").canConvert<QBrush>()) {
            grooveDisableBrush = widget->property("setGrooveDisableBrush").value<QBrush>();
        }
        if (widget->property("setContentHightColor").isValid() && widget->property("setContentHightColor").canConvert<QColor>()) {
            startColor = widget->property("setContentHightColor").value<QColor>();
        }
        if (widget->property("setContentMidLightColor").isValid() && widget->property("setContentMidLightColor").canConvert<QColor>()) {
            endColor = widget->property("setContentMidLightColor").value<QColor>();
        }
        if (widget->property("setGrooveDefaultPen").isValid() && widget->property("setGrooveDefaultPen").canConvert<QColor>()) {
            grooveDefaultPen = widget->property("setGrooveDefaultPen").value<QPen>();
        }
        if (widget->property("setGrooveDisablePen").isValid() && widget->property("setGrooveDisablePen").canConvert<QColor>()) {
            grooveDisablePen = widget->property("setGrooveDisablePen").value<QPen>();
        }
    }

    m_ProgressBarParameters.radius = radius;

    m_ProgressBarParameters.progressBarContentPen       = contentPen;
    m_ProgressBarParameters.progressBarGrooveDefaultPen = grooveDefaultPen;
    m_ProgressBarParameters.progressBarGrooveDisablePen = grooveDisablePen;

    m_ProgressBarParameters.progressBarContentBrush       = contentBrush;
    m_ProgressBarParameters.progressBarGrooveDefaultBrush = grooveDefaultBrush;
    m_ProgressBarParameters.progressBarGrooveDisableBrush = grooveDisableBrush;
    m_ProgressBarParameters.progressBarContentHightColor  = endColor;
    m_ProgressBarParameters.progressBarContentMidLightColor = startColor;
}

void LINGMOConfigStyleParameters::getDefaultProgressBarParameters(ConfigProgressBarParameters *p, bool isDark)
{
    initConfigProgressBarParameters(isDark, nullptr, nullptr);
    p->radius                          = m_ProgressBarParameters.radius                         ;
    p->progressBarContentPen           = m_ProgressBarParameters.progressBarContentPen          ;
    p->progressBarGrooveDefaultPen     = m_ProgressBarParameters.progressBarGrooveDefaultPen    ;
    p->progressBarGrooveDisablePen     = m_ProgressBarParameters.progressBarGrooveDisablePen    ;
    p->progressBarContentBrush         = m_ProgressBarParameters.progressBarContentBrush        ;
    p->progressBarGrooveDefaultBrush   = m_ProgressBarParameters.progressBarGrooveDefaultBrush  ;
    p->progressBarGrooveDisableBrush   = m_ProgressBarParameters.progressBarGrooveDisableBrush  ;
    p->progressBarContentHightColor    = m_ProgressBarParameters.progressBarContentHightColor   ;
    p->progressBarContentMidLightColor = m_ProgressBarParameters.progressBarContentMidLightColor;
}

void LINGMOConfigStyleParameters::initConfigScrollBarParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    int sliderRadius;
    bool needTranslucent = false;
    if (widget && widget->property("needTranslucent").isValid()) {
        needTranslucent = widget->property("needTranslucent").toBool();
    }
    LINGMOColorTheme::ScrollBarColorCfg scrollBarColorCfg = readCfg()->scrollBarColorCfg(palette,
                                           needTranslucent ? C_ScrollBar_Translucent : C_ScrollBar_Default);

    QBrush grooveDefaultBrush  = scrollBarColorCfg.grooveDefaultBrush;
    QBrush grooveInactiveBrush = scrollBarColorCfg.grooveInactiveBrush;
    QBrush sliderDefaultBrush  = scrollBarColorCfg.sliderDefaultBrush;
    QBrush sliderHoverBrush    = scrollBarColorCfg.sliderHoverBrush;
    QBrush sliderClickBrush    = scrollBarColorCfg.sliderClickBrush;
    QBrush sliderDisableBrush  = scrollBarColorCfg.sliderDisableBrush;

    QColor mix = palette.color(QPalette::Active, QPalette::BrightText);

    const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option);

    //radius
    int normalRadius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
    if (bar && normalRadius != 0) {
        if (bar->orientation == Qt::Horizontal) {
            sliderRadius = (bar->rect.height()) / 4;
        } else {
            sliderRadius = (bar->rect.width()) / 4;
        }
        m_ScrollBarParameters.grooveWidthAnimation = true;
        ScrooBar_ShowLine = false;
        ScroolBar_LineLength = 0;
    } else {
        int normalRadius = (widget && widget->property("normalRadius").isValid()) ?
                    widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
        sliderRadius = normalRadius;
        m_ScrollBarParameters.grooveWidthAnimation = false;
        ScrooBar_ShowLine = true;
        ScroolBar_LineLength = 16;
    }

    //slider hover
    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;

    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:
        if (!needTranslucent) {
            if (scrollBarColorCfg.sliderHoverType == Obj_Gradient_Type) {
                if (bar && bar->orientation == Qt::Horizontal) {
                    linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
                    linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
                } else {
                    linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topRight());
                    linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.topLeft());
                }
                startColor = scrollBarColorCfg.sliderHoverGradientList.value(0);
                endColor = scrollBarColorCfg.sliderHoverGradientList.value(1);

                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                sliderHoverBrush = QBrush(linearGradient);
            }
        }
        break;
    default:
        break;
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", sliderRadius);
        w->setProperty("grooveDefaultBrush",     grooveDefaultBrush);
        w->setProperty("grooveInactiveBrush",     grooveInactiveBrush);
        w->setProperty("sliderDefaultBrush",     sliderDefaultBrush);
        w->setProperty("sliderHoverBrush",     sliderHoverBrush);
        w->setProperty("sliderClickBrush",     sliderClickBrush);
        w->setProperty("sliderDisableBrush",     sliderDisableBrush);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setGrooveDefaultBrush").isValid() && widget->property("setGrooveDefaultBrush").canConvert<QBrush>()) {
            grooveDefaultBrush = widget->property("setGrooveDefaultBrush").value<QBrush>();
        }
        if (widget->property("setGrooveInactiveBrush").isValid() && widget->property("setGrooveInactiveBrush").canConvert<QBrush>()) {
            grooveInactiveBrush = widget->property("setGrooveInactiveBrush").value<QBrush>();
        }
        if (widget->property("setSliderDefaultBrush").isValid() && widget->property("setSliderDefaultBrush").canConvert<QBrush>()) {
            sliderDefaultBrush = widget->property("setSliderDefaultBrush").value<QBrush>();
        }
        if (widget->property("setSliderHoverBrush").isValid() && widget->property("setSliderHoverBrush").canConvert<QBrush>()) {
            sliderHoverBrush = widget->property("setSliderHoverBrush").value<QBrush>();
        }
        if (widget->property("setSliderClickBrush").isValid() && widget->property("setSliderClickBrush").canConvert<QBrush>()) {
            sliderClickBrush = widget->property("setSliderClickBrush").value<QBrush>();
        }
        if (widget->property("setSliderDisableBrush").isValid() && widget->property("setSliderDisableBrush").canConvert<QBrush>()) {
            sliderDisableBrush = widget->property("setSliderDisableBrush").value<QBrush>();
        }
    }

    m_ScrollBarParameters.radius = sliderRadius;
    ScroolBar_Height= 68;
    m_ScrollBarParameters.scrollBarGrooveDefaultBrush  = grooveDefaultBrush;
    m_ScrollBarParameters.scrollBarGrooveInactiveBrush = grooveInactiveBrush;
    m_ScrollBarParameters.scrollBarSliderDefaultBrush  = sliderDefaultBrush;
    m_ScrollBarParameters.scrollBarSliderHoverBrush    = sliderHoverBrush;
    m_ScrollBarParameters.scrollBarSliderClickBrush    = sliderClickBrush;
    m_ScrollBarParameters.scrollBarSliderDisableBrush  = sliderDisableBrush;
}

void LINGMOConfigStyleParameters::getDefaultScrollBarParameters(ConfigScrollBarParameters *p, bool isDark)
{
    initConfigScrollBarParameters(isDark, nullptr, nullptr);
    p->radius                       = m_ScrollBarParameters.radius                      ;
    p->scrollBarGrooveDefaultBrush  = m_ScrollBarParameters.scrollBarGrooveDefaultBrush ;
    p->scrollBarGrooveInactiveBrush = m_ScrollBarParameters.scrollBarGrooveInactiveBrush;
    p->scrollBarSliderDefaultBrush  = m_ScrollBarParameters.scrollBarSliderDefaultBrush ;
    p->scrollBarSliderHoverBrush    = m_ScrollBarParameters.scrollBarSliderHoverBrush   ;
    p->scrollBarSliderClickBrush    = m_ScrollBarParameters.scrollBarSliderClickBrush   ;
    p->scrollBarSliderDisableBrush  = m_ScrollBarParameters.scrollBarSliderDisableBrush ;
}

void LINGMOConfigStyleParameters::initConfigToolTipParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    LINGMOColorTheme::ToolTipColorCfg tooltipColorCfg = readCfg()->toolTipColorCfg(palette, C_ToolTip_Default);

    QPen backgroundPen = QPen(tooltipColorCfg.backgroundPen, (tooltipColorCfg.backgroundPen.alpha() == 0 ||
                              tooltipColorCfg.backgroundPen == Qt::NoPen ) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //background
    auto color = tooltipColorCfg.backgroundBrush;
    if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
        auto opacity = LINGMOStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        color.setAlphaF(opacity);
    }
    if (qApp->property("blurEnable").isValid()) {
        bool blurEnable = qApp->property("blurEnable").toBool();
        if (!blurEnable) {
            color.setAlphaF(1);
        }
    }
    if (widget) {
        if (widget->property("useSystemStyleBlur").isValid() && !widget->property("useSystemStyleBlur").toBool()) {
            color.setAlphaF(1);
        }
    }
    //if blur effect is not supported, do not use transparent color.
    if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) || blackAppListWithBlurHelper().contains(qAppName())) {
        color.setAlphaF(1);
    }
    QBrush backgroundBrush = color;

    int radius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;

    auto settings = LINGMOStyleSettings::globalInstance();
    if(settings && settings->keys().contains("windowRadius"))
        radius = settings->get("windowRadius").toInt();

    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            radius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setBackgroundBrush").isValid() && widget->property("setBackgroundBrush").canConvert<QBrush>()) {
            backgroundBrush = widget->property("setBackgroundBrush").value<QBrush>();
        }
        if (widget->property("setBackgroundPen").isValid() && widget->property("setBackgroundPen").canConvert<QBrush>()) {
            backgroundPen = widget->property("setBackgroundPen").value<QPen>();
        }
    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", radius);
        w->setProperty("backgroundBrush",     backgroundBrush);
        w->setProperty("backgroundPen",     backgroundPen);
    }
    m_ToolTipParameters.radius = radius;
    m_ToolTipParameters.toolTipBackgroundBrush = backgroundBrush;
    m_ToolTipParameters.toolTipBackgroundPen = backgroundPen;
}

void LINGMOConfigStyleParameters::getDefaultToolTipParameters(ConfigToolTipParameters *p, bool isDark)
{
    initConfigToolTipParameters(isDark, nullptr, nullptr);
    p->radius                 = m_ToolTipParameters.radius                ;
    p->toolTipBackgroundBrush = m_ToolTipParameters.toolTipBackgroundBrush;
    p->toolTipBackgroundPen   = m_ToolTipParameters.toolTipBackgroundPen  ;
}

void LINGMOConfigStyleParameters::initConfigTabWidgetParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    int tabradius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
    LINGMOColorTheme::TabWidgetColorCfg tabWidgetColorCfg = readCfg()->tabWidgetColorCfg(palette, C_TabWidget_Default);

    QBrush widgetBackgroundBrush = tabWidgetColorCfg.widgetBackgroundBrush;
    QBrush tabBarBackgroundBrush = tabWidgetColorCfg.tabBarBackgroundBrush;
    QBrush tabBarDefaultBrush    = tabWidgetColorCfg.tabBarDefaultBrush;
    QBrush tabBarHoverBrush      = tabWidgetColorCfg.tabBarHoverBrush;
    QBrush tabBarSelectBrush     = tabWidgetColorCfg.tabBarSelectBrush;
    QBrush tabBarClickBrush      = tabWidgetColorCfg.tabBarClickBrush;
    QPen tabBarHoverPen          = QPen(tabWidgetColorCfg.tabBarHoverPen, (tabWidgetColorCfg.tabBarHoverPen == Qt::NoPen || tabWidgetColorCfg.tabBarHoverPen.alpha() == 0) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen tabBarClickPen          = QPen(tabWidgetColorCfg.tabBarClickPen, (tabWidgetColorCfg.tabBarClickPen == Qt::NoPen || tabWidgetColorCfg.tabBarClickPen.alpha() == 0) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen tabBarDefaultPen        = QPen(tabWidgetColorCfg.tabBarDefaultPen, (tabWidgetColorCfg.tabBarDefaultPen == Qt::NoPen || tabWidgetColorCfg.tabBarDefaultPen.alpha() == 0) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen tabBarFocusPen          = QPen(tabWidgetColorCfg.tabBarFocusPen , (tabWidgetColorCfg.tabBarFocusPen.alpha() == 0 ||
                              tabWidgetColorCfg.tabBarFocusPen         == Qt::NoPen) ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);


    switch (m_adjustColorRules) {
    case AdjustColorRules::DefaultRGB:
        if (tabWidgetColorCfg.tabBarHoverType == Obj_Gradient_Type) {
            //tabbar hover
            QColor startColor;
            QColor endColor;
            QLinearGradient linearGradient;
            linearGradient.setStart(!option ? QPoint(0,0) : option->rect.topLeft());
            linearGradient.setFinalStop(!option ? QPoint(0,0) : option->rect.bottomLeft());
            startColor = tabWidgetColorCfg.tabBarHoverGradientList.value(0);
            endColor   = tabWidgetColorCfg.tabBarHoverGradientList.value(1);
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            tabBarHoverBrush = QBrush(linearGradient);
        }
        break;
    case AdjustColorRules::HSL:{
        bool selected = !option ? QStyle::State_None : (option->state & QStyle::State_Selected);
        bool hover = !option ? QStyle::State_None : (option->state & QStyle::State_MouseOver);
        bool click = !option ? QStyle::State_None : (option->state & (QStyle::State_On | QStyle::State_Sunken));

        if (isDark) {
            tabBarHoverBrush = highlightHover(isDark, palette);
            tabBarClickBrush = highlightClick(isDark, palette);
            if (click) {
                tabBarClickPen = QPen(QBrush(palette.color(QPalette::Active, QPalette::Highlight)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            } else if(hover && !selected) {
                tabBarHoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 0, -5, -15)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
        } else {
            tabBarHoverBrush = highlightHover(isDark, palette);
            tabBarClickBrush = highlightClick(isDark, palette);

            if(click){
                tabBarClickPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 1, -21, 3)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
            else if(hover && !selected){
                tabBarHoverPen = QPen(QBrush(adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -14, 15)),
                                 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            }
        }
    } break;
    default:
        break;
    }

//    if((option->state & QStyle::State_HasFocus) && (option->state & QStyle::State_Enabled)){
//        tabBarPen = tabBarFocusPen;
//    }

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("radius", tabradius);
        w->setProperty("widgetBackgroundBrush", widgetBackgroundBrush);
        w->setProperty("tabBarBackgroundBrush", tabBarBackgroundBrush);
        w->setProperty("tabBarDefaultBrush",    tabBarDefaultBrush);
        w->setProperty("tabBarHoverBrush",      tabBarHoverBrush);
        w->setProperty("tabBarClickBrush",      tabBarClickBrush);
        w->setProperty("tabBarSelectBrush",     tabBarSelectBrush);
        w->setProperty("tabBarDefaultPen",      tabBarDefaultPen);
        w->setProperty("tabBarFocusPen",        tabBarFocusPen);
        w->setProperty("tabBarClickPen",        tabBarClickPen);
        w->setProperty("tabBarHoverPen",        tabBarHoverPen);
    }
    if (widget) {
        if (widget->property("setRadius").isValid() && widget->property("setRadius").canConvert<int>()) {
            tabradius = widget->property("setRadius").value<int>();
        }
        if (widget->property("setWidgetBackgroundBrush").isValid() && widget->property("setWidgetBackgroundBrush").canConvert<QBrush>()) {
            widgetBackgroundBrush = widget->property("setWidgetBackgroundBrush").value<QBrush>();
        }
        if (widget->property("setTabBarBackgroundBrush").isValid() && widget->property("setTabBarBackgroundBrush").canConvert<QBrush>()) {
            tabBarBackgroundBrush = widget->property("setTabBarBackgroundBrush").value<QBrush>();
        }
        if (widget->property("setTabBarDefaultBrush").isValid() && widget->property("setTabBarDefaultBrush").canConvert<QBrush>()) {
            tabBarDefaultBrush = widget->property("setTabBarDefaultBrush").value<QBrush>();
        }
        if (widget->property("setTabBarHoverBrush").isValid() && widget->property("setTabBarHoverBrush").canConvert<QBrush>()) {
            tabBarHoverBrush = widget->property("setTabBarHoverBrush").value<QBrush>();
        }
        if (widget->property("setTabBarClickBrush").isValid() && widget->property("setTabBarClickBrush").canConvert<QBrush>()) {
            tabBarClickBrush = widget->property("setTabBarClickBrush").value<QBrush>();
        }
        if (widget->property("setTabBarSelectBrush").isValid() && widget->property("setTabBarSelectBrush").canConvert<QBrush>()) {
            tabBarSelectBrush = widget->property("setTabBarSelectBrush").value<QBrush>();
        }
        if (widget->property("setTabBarDefaultPen").isValid() && widget->property("setTabBarDefaultPen").canConvert<QPen>()) {
            tabBarDefaultPen = widget->property("setTabBarDefaultPen").value<QPen>();
        }
        if (widget->property("setTabBarFocusPen").isValid() && widget->property("setTabBarFocusPen").canConvert<QPen>()) {
            tabBarFocusPen = widget->property("setTabBarFocusPen").value<QPen>();
        }
        if (widget->property("setTabBarClickPen").isValid() && widget->property("setTabBarClickPen").canConvert<QPen>()) {
            tabBarFocusPen = widget->property("setTabBarClickPen").value<QPen>();
        }
        if (widget->property("setTabBarHoverPen").isValid() && widget->property("setTabBarHoverPen").canConvert<QPen>()) {
            tabBarFocusPen = widget->property("setTabBarHoverPen").value<QPen>();
        }
    }

    m_TabWidgetParameters.radius = tabradius;
    m_TabWidgetParameters.tabWidgetBackgroundBrush = widgetBackgroundBrush;
    m_TabWidgetParameters.tabBarBackgroundBrush = tabBarBackgroundBrush;
    m_TabWidgetParameters.tabBarDefaultBrush = tabBarDefaultBrush;
    m_TabWidgetParameters.tabBarHoverBrush = tabBarHoverBrush;
    m_TabWidgetParameters.tabBarClickBrush = tabBarClickBrush;
    m_TabWidgetParameters.tabBarSelectBrush = tabBarSelectBrush;
    m_TabWidgetParameters.tabBarDefaultPen  = tabBarDefaultPen;
    m_TabWidgetParameters.tabBarHoverPen  = tabBarHoverPen;
    m_TabWidgetParameters.tabBarClickPen  = tabBarClickPen;
    m_TabWidgetParameters.tabBarFocusPen  = tabBarFocusPen;
}

void LINGMOConfigStyleParameters::getDefaultTabWidgetParameters(ConfigTabWidgetParameters *p, bool isDark)
{
    initConfigTabWidgetParameters(isDark, nullptr, nullptr);

    p->radius                   = m_TabWidgetParameters.radius                  ;
    p->tabWidgetBackgroundBrush = m_TabWidgetParameters.tabWidgetBackgroundBrush;
    p->tabBarBackgroundBrush    = m_TabWidgetParameters.tabBarBackgroundBrush   ;
    p->tabBarDefaultBrush       = m_TabWidgetParameters.tabBarDefaultBrush      ;
    p->tabBarHoverBrush         = m_TabWidgetParameters.tabBarHoverBrush        ;
    p->tabBarClickBrush         = m_TabWidgetParameters.tabBarClickBrush        ;
    p->tabBarSelectBrush        = m_TabWidgetParameters.tabBarSelectBrush       ;
    p->tabBarDefaultPen         = m_TabWidgetParameters.tabBarDefaultPen        ;
    p->tabBarHoverPen           = m_TabWidgetParameters.tabBarHoverPen          ;
    p->tabBarClickPen           = m_TabWidgetParameters.tabBarClickPen          ;
    p->tabBarFocusPen           = m_TabWidgetParameters.tabBarFocusPen          ;
}

void LINGMOConfigStyleParameters::initConfigMenuParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    QPalette palette = option ? option->palette : m_stylePalette;

    int frameradius = (widget && widget->property("maxRadius").isValid()) ?
                widget->property("maxRadius").toInt() : m_radiusStruct.maxRadius;

    auto settings = LINGMOStyleSettings::globalInstance();
    if(settings && settings->keys().contains("windowRadius"))
        frameradius = settings->get("windowRadius").toInt();

    int itemradius = (widget && widget->property("normalRadius").isValid()) ?
                widget->property("normalRadius").toInt() : m_radiusStruct.normalRadius;
//    QPixmap framePixmap(option->rect.size());
    LINGMOColorTheme::MenuColorCfg menuColorCfg = readCfg()->menuColorCfg(palette, C_Menu_Default);

    QPen menuBackgroundPen = QPen(menuColorCfg.menuBackgroundPen, (menuColorCfg.menuBackgroundPen.alpha() == 0 ||
                                  menuColorCfg.menuBackgroundPen == Qt::NoPen ) ? 0 : 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush itemSelectBrush = menuColorCfg.menuItemSelectBrush;
    QPen menuHoverPen = menuColorCfg.menuTextHoverPen;
    QPen menuItemFocusPen = QPen(menuColorCfg.menuItemFocusPen, (menuColorCfg.menuItemFocusPen.alpha() == 0 ||
                                  menuColorCfg.menuItemFocusPen == Qt::NoPen ) ? 0 : 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //item select brush
    switch (m_adjustColorRules) {
    case HSL:
        if(isDark) {
            itemSelectBrush = adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 3, 23, -11);
        } else {
            itemSelectBrush = adjustColor(palette.color(QPalette::Active, QPalette::Highlight), 4, -17, -6);
        }
        break;
    default:
        break;
    }

    auto color = menuColorCfg.menuBackgroundBrush;
    if (widget && !widget->inherits("QComboBoxPrivateContainer") && LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
        auto opacity = LINGMOStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        color.setAlphaF(opacity);
    }
    if (qApp->property("blurEnable").isValid()) {
        bool blurEnable = qApp->property("blurEnable").toBool();
        if (!blurEnable) {
            color.setAlphaF(1);
        }
    }
    if (widget) {
        if (widget->property("useSystemStyleBlur").isValid() && !widget->property("useSystemStyleBlur").toBool()) {
            color.setAlphaF(1);
        }
    }
    //if blur effect is not supported, do not use transparent color.
    if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) || blackAppListWithBlurHelper().contains(qAppName())) {
        color.setAlphaF(1);
    }
    QBrush menuBackgroundBrush = color;

    auto *w = const_cast<QWidget *>(widget);
    if (w) {
        w->setProperty("frameradius", frameradius);
        w->setProperty("itemradius",  itemradius);
        w->setProperty("menuBackgroundBrush", menuBackgroundBrush);
        w->setProperty("menuBackgroundPen",   menuBackgroundPen);
        w->setProperty("itemSelectBrush",     itemSelectBrush);
        w->setProperty("menuHoverPen",        menuHoverPen);
        w->setProperty("itemFocusPen",        menuItemFocusPen);
    }

    if (widget) {
        if (widget->property("setFrameRadius").isValid() && widget->property("setFrameRadius").canConvert<int>()) {
            frameradius = widget->property("setFrameRadius").value<int>();
        }
        if (widget->property("setItemRadius").isValid() && widget->property("setItemRadius").canConvert<int>()) {
            itemradius = widget->property("setItemRadius").value<int>();
        }
//        if (widget->property("setFramePixmap").isValid() && widget->property("setFramePixmap").canConvert<QPixmap>()) {
//            framePixmap = widget->property("setFramePixmap").value<QPixmap>();
//        }
        if (widget->property("setItemSelectBrush").isValid() && widget->property("setItemSelectBrush").canConvert<QBrush>()) {
            itemSelectBrush = widget->property("setItemSelectBrush").value<QBrush>();
        }
        if (widget->property("setTextHoverPen").isValid() && widget->property("setTextHoverPen").canConvert<QPen>()) {
            menuHoverPen = widget->property("setTextHoverPen").value<QPen>();
        }
        if (widget->property("setMenuBackgroundBrush").isValid() && widget->property("setMenuBackgroundBrush").canConvert<QBrush>()) {
            menuBackgroundBrush = widget->property("setMenuBackgroundBrush").value<QBrush>();
        }
        if (widget->property("setMenuBackgroundPen").isValid() && widget->property("setMenuBackgroundPen").canConvert<QPen>()) {
            menuBackgroundPen = widget->property("setMenuBackgroundPen").value<QPen>();
        }
        if (widget->property("setItemFocusPen").isValid() && widget->property("setItemFocusPen").canConvert<QPen>()) {
            menuItemFocusPen = widget->property("setItemFocusPen").value<QPen>();
        }
    }

    m_MenuParameters.frameRadius = frameradius;
    m_MenuParameters.itemRadius = itemradius;
//    m_MenuParameters.menuFramePixmap = framePixmap;
    m_MenuParameters.menuBackgroundBrush = menuBackgroundBrush;
    m_MenuParameters.menuBackgroundPen   = menuBackgroundPen;
    m_MenuParameters.menuItemSelectBrush = itemSelectBrush;
    m_MenuParameters.menuItemFocusPen    = menuItemFocusPen;
    m_MenuParameters.menuTextHoverPen = menuHoverPen;
}

void LINGMOConfigStyleParameters::getDefaultMenuParameters(ConfigMenuParameters *p, bool isDark)
{
    initConfigTabWidgetParameters(isDark, nullptr, nullptr);
    p->frameRadius         = m_MenuParameters.frameRadius        ;
    p->itemRadius          = m_MenuParameters.itemRadius         ;
    p->menuBackgroundBrush = m_MenuParameters.menuBackgroundBrush;
    p->menuBackgroundPen   = m_MenuParameters.menuBackgroundPen  ;
    p->menuItemSelectBrush = m_MenuParameters.menuItemSelectBrush;
    p->menuItemFocusPen    = m_MenuParameters.menuItemFocusPen   ;
    p->menuTextHoverPen    = m_MenuParameters.menuTextHoverPen   ;
}
