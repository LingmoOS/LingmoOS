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

#ifndef LINGMOCONFIGSTYLEPARAMETERS_H
#define LINGMOCONFIGSTYLEPARAMETERS_H

#include <QObject>
#include <QPalette>
#include <QStyleOption>

#include "widget-parameters/config-pushbutton-parameters.h"
#include "widget-parameters/config-toolbutton-parameters.h"
#include "widget-parameters/config-lineedit-parameters.h"
#include "widget-parameters/config-spinbox-parameters.h"
#include "widget-parameters/config-combobox-parameters.h"
#include "widget-parameters/config-list-parameters.h"
#include "widget-parameters/config-tree-parameters.h"
#include "widget-parameters/config-table-parameters.h"
#include "widget-parameters/config-checkbox-parameters.h"
#include "widget-parameters/config-radiobutton-parameters.h"
#include "widget-parameters/config-slider-parameters.h"
#include "widget-parameters/config-progressbar-parameters.h"
#include "widget-parameters/config-scrollbar-parameters.h"
#include "widget-parameters/config-tooltip-parameters.h"
#include "widget-parameters/config-tabwidget-parameters.h"
#include "widget-parameters/config-menu-parameters.h"
#include "../readconfig.h"

namespace LINGMOConfigStyleSpace {

class LINGMOConfigStyleParameters: public QObject
{
    Q_OBJECT
public:
    LINGMOConfigStyleParameters(QObject *parent, bool isDark, QString cfgName = "");
    ~LINGMOConfigStyleParameters();

    enum SpinBoxControlLayout {
        Horizontal,
        Vertical
    };
    Q_ENUM(SpinBoxControlLayout)

    enum TabBarIndicatorLayout {
        TabBarIndicator_Horizontal,
        TabBarIndicator_Vertical
    };
    Q_ENUM(TabBarIndicatorLayout)

    enum AdjustColorRules {
        DefaultRGB,
        HSL
    };
    Q_ENUM(AdjustColorRules)

    void updateParameters(bool isTabletMode);

    QPalette setPalette(QPalette &palette);
    int getSpinBoxControlLayout();


    int getTabBarIndicatorLayout();
    void setTabBarIndicatorLayout(TabBarIndicatorLayout layout);

    void setLINGMOThemeColor(QPalette p, QString themeColor);
    QString themeColor();
    bool isDefaultThemeColor();

    void initialDefaultPaletteColor(QString colorPath, bool isDark);
    virtual void initPalette(bool isDark);

    virtual void initRadiusCfg();

    //pushbutton
    virtual void initConfigPushButtonBevelParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigPushButtonBevelParameters(ConfigPushButtonParameters *p, bool isDark);

    virtual void initConfigPushButtonLabelParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigPushButtonLabelParameters(ConfigPushButtonParameters *p, bool isDark);

    //toolbutton
    virtual void initConfigToolButtonPanelParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigToolButtonPanelParameters(ConfigToolButtonParameters *p, bool isDark);

    virtual void initConfigToolButtonLabelParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigToolButtonLabelParameters(ConfigToolButtonParameters *p, bool isDark);

    virtual void initConfigLineEditParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigLineEditParameters(ConfigLineEditParameters *p, bool isDark);

    virtual void initConfigSpinBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool isHorizonLayout = false);
    virtual void getDefaultConfigSpinBoxParameters(ConfigSpinBoxParameters *p, bool isDark);

    virtual void initConfigComboBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigComboBoxParameters(ConfigComboBoxParameters *p, bool isDark);

    virtual void initConfigListParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlightmode = false);
    virtual void getDefaultConfigListParameters(ConfigListParameters *p, bool isDark);
    virtual void getHighlightModeConfigListParameters(ConfigListParameters *p, bool isDark);

    virtual void initConfigTreeParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlightmode = false);
    virtual void getDefaultConfigTreeParameters(ConfigTreeParameters *p, bool isDark);
    virtual void getHighlightModeConfigTreeParameters(ConfigTreeParameters *p, bool isDark);

    virtual void initConfigTableParameters(bool isDark, const QStyleOption *option, const QWidget *widget, bool highlightmode = false);
    virtual void getDefaultConfigTableParameters(ConfigTableParameters *p, bool isDark);
    virtual void getHighlightModeConfigTableParameters(ConfigTableParameters *p, bool isDark);

    virtual void initConfigCheckBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigCheckBoxParameters(ConfigCheckBoxParameters *p, bool isDark);

    virtual void initConfigRadioButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultConfigRadioButtonParameters(ConfigRadioButtonParameters *p, bool isDark);

    virtual void initConfigSliderParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultSliderParameters(ConfigSliderParameters *p, bool isDark);

    virtual void initConfigProgressBarParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultProgressBarParameters(ConfigProgressBarParameters *p, bool isDark);

    virtual void initConfigScrollBarParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultScrollBarParameters(ConfigScrollBarParameters *p, bool isDark);

    virtual void initConfigToolTipParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultToolTipParameters(ConfigToolTipParameters *p, bool isDark);

    virtual void initConfigTabWidgetParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultTabWidgetParameters(ConfigTabWidgetParameters *p, bool isDark);

    virtual void initConfigMenuParameters(bool isDark, const QStyleOption *option, const QWidget *widget);
    virtual void getDefaultMenuParameters(ConfigMenuParameters *p, bool isDark);

    virtual QColor lanhuHSLToQtHsl(int h, int s, int l, int a = 255);
    virtual QColor adjustColor(const QColor c, int hRange = 0, int sRange = 0, int lRange = 0, int aRange = 0);
    virtual QColor adjustH(const QColor c, int range);
    virtual QColor adjustS(const QColor c, int range);
    virtual QColor adjustL(const QColor c, int range);
    virtual QColor adjustA(const QColor c, int range);
    virtual QColor highlightClick(bool isDark, QPalette p);
    virtual QColor highlightHover(bool isDark, QPalette p);
    virtual QColor buttonDarkClick(bool isDark, QPalette p);
    virtual void normalButtonColor(bool isDark, QPalette p, QPen &defaultPen, QBrush &defaultBrush,  QPen &hoverPen, QBrush &hoverBrush,
                            QPen &clickPen, QBrush &clickBrush,  QPen &disablePen, QBrush &disableBrush);
    void setCfgName(QString cfgName);
    QString cfgName();
    ReadThemeConfig* readCfg();

    LINGMORadiusInformation::LINGMORadiusStruct radiusProperty();

    int getAdjustColorRules();

    bool colorIsSame(QColor oldColor, QColor newColor);

public:
    ConfigPushButtonParameters m_PushButtonParameters;
    ConfigToolButtonParameters m_ToolButtonParameters;
    ConfigLineEditParameters m_LineEditParameters;
    ConfigSpinBoxParameters m_SpinBoxParameters;
    ConfigComboBoxParameters m_ComboBoxParameters;
    ConfigListParameters m_ListParameters;
    ConfigTreeParameters m_TreeParameters;
    ConfigTableParameters m_TableParameters;
    ConfigCheckBoxParameters m_CheckBoxParameters;
    ConfigRadioButtonParameters m_RadioButtonParameters;
    ConfigSliderParameters m_SliderParameters;
    ConfigProgressBarParameters m_ProgressBarParameters;
    ConfigScrollBarParameters m_ScrollBarParameters;
    ConfigToolTipParameters m_ToolTipParameters;
    ConfigTabWidgetParameters m_TabWidgetParameters;
    ConfigMenuParameters m_MenuParameters;

    // radius
    int radius = 6;

    // common
    int SmallIcon_Size = 16;
    int IconButton_Distance = 8;

    // button
    int Button_MarginHeight = 0;
    int ToolButton_MarginWidth = 8;
    int Button_MarginWidth = 8;
    int Button_IconSize = 16;
    int Button_IndicatorSize = 16;
    int Button_DefaultIndicatorSize = 0;
    int Button_DefaultWidth = 96;
    int Button_DefaultHeight = 36;
    int Button_DefaultHeight_origin = 36;
    int ToolButton_DefaultWidth = 60;
    int IconButton_DefaultWidth = 36;
    int IconButton_DefaultWidth_origin = 36;

    QColor DarkColoseButtonColor = QColor(242, 116, 133);

    QColor LightColoseButtonColor = QColor(242, 82, 117);
    QColor EndColoseButtonColor = QColor(221, 44, 68);

    QColor Indicator_IconDefault;
    QColor Indicator_IconHover;
    QColor Indicator_IconSunken;
    QColor Indicator_IconDisable;

    // menu
    int Menu_MarginHeight = 4;
    int Menu_Combobox_Popup_MarginHeight = 4;
    int Menu_MarginWidth = Menu_MarginHeight;
    int Menu_Combobox_Popup_MarginWidth = Menu_MarginHeight;
    int Menu_MarginPanelWidth = 0;
    int MenuItem_Spacing = 8;
    int MenuItem_HMargin = 8;
    int MenuContent_HMargin = 8;
    bool Menu_Right_Bottom_Corner = false;

    // menu-item
    int MenuItem_MarginHeight = 2;
    int MenuItem_MarginWidth = 12 + 4;
    int MenuItemSeparator_MarginHeight = 4;
    int MenuItemSeparator_MarginHeight_origin = 4;
    int MenuItemSeparator_MarginWidth = 4;
    int MenuItem_DefaultHeight = 36;
    int MenuItem_DefaultHeight_origin = 36;
    int MenuItem_DefaultWidght = 152;

    // scrollbar
    int ScroolBar_Width = 16;
    int ScroolBar_Width_origin = 16;
    int ScroolBar_Height = 68;
    int ScroolBar_LineLength = 16;
    bool ScrooBar_ShowLine = false;

    // progressbar
    int ProgressBar_DefaultLength = 388;
    int ProgressBar_DefaultThick = 16;

    // slider
    int Slider_DefaultLength = 94;
    int Slider_DefaultLength_origin = 94;
    int Slider_DefaultWidth = 34;
    int Slider_DefaultWidth_origin = 34;
    int Slider_Length = 20;
    int Slider_Length_origin = 20;
    int Slider_Thickness = 20;
    int Slider_Thickness_origin = 20;

    int Slider_GrooveLength = 4;
    int Slider_GrooveLength_origin = 4;
    int Slider_Margin = 2;

    // radiobutton
    int ExclusiveIndicator_Width = 16;
    int ExclusiveIndicator_Height = 16;
    int RadioButtonLabel_Spacing = 8;
    int RadioButton_DefaultHeight = 36;
    int RadioButton_DefaultHeight_origin = 36;

    // checkbox
    int Indicator_Width = 16;
    int Indicator_Height = 16;
    int CheckBox_DefaultHeight = 36;
    int CheckBox_DefaultHeight_origin = 36;
    int CheckBox_Radius = 4;

    // lineedit
    int LineEdit_DefaultWidth = 160;
    int LineEdit_DefaultWidth_origin = 160;
    int LineEdit_DefaultHeight = 36;
    int LineEdit_DefaultHeight_origin = 36;

    // combobox
    int ComboBox_DefaultWidth = 160;
    int ComboBox_DefaultHeight = 36;
    int ComboBox_DefaultHeight_origin = 36;
    int ComboBox_DefaultMenuItemHeight = 36;
    int ComboBox_DefaultMenuItemHeight_original = 36;
    int ComboBox_FrameWidth = 2;
    int ComboBox_VMargin = 1;

    // spinbox
    int SpinBox_DefaultWidth = 160;
    int SpinBox_DefaultHeight = 36;
    int SpinBox_DefaultHeight_origin = 36;
    int SpinBox_FrameWidth = 2;

    // tabbar
    int TabBar_DefaultMinWidth = 168;
    int TabBar_DefaultMaxWidth = 248;
    int TabBar_DefaultHeight = 42;
    int TabBar_DefaultHeight_origin = 40;
    int TabBar_ScrollButtonWidth = 16;
    int TabBar_ScrollButtonOverlap = 2;
    bool TabBar_SplitLine = true;

    // tooltip
    int ToolTip_DefaultMargin = 10;
    int ToolTip_Height = 36;
    int ToolTip_Height_origin = 36;

    //viewitem FIX：Separate to list,tree and table after
    int ViewItem_DefaultHeight = 36;
    int ViewItem_DefaultHeight_origin = 36;

    //table
    int Table_HeaderHeight = 36;
    int Table_HeaderHeight_origin = 36;

    float m_scaleRatio4_3 = 1.0;
    float m_scaleRatio3_2 = 1.0;
    float m_scaleRatio2_1 = 1.0;

    int m_headerDefaultSectionSizeVertical = 36;
    int m_headerDefaultSectionSizeVertical1_1 = 36;
    int m_headerDefaultSectionSizeVertical4_3 = 48;
    bool m_isTableMode = false;
    bool m_isInitial = true;

    float m_scaleSliderDefaultLength = 1.0;
    float m_ScaleComboBoxDefaultLength = 1.0;
    float m_ScaleLineEditDefaultLength = 1.0;
    float m_scaleTabBarHeight6_5 = 1.0;

    bool indicatorIconHoverNeedHighLight = true;

    QPalette defaultPalette;

private:
    int m_spinBoxControlLayout = SpinBoxControlLayout::Vertical;
    int m_tabBarIndicatorLayout = TabBarIndicatorLayout::TabBarIndicator_Horizontal;
    int m_adjustColorRules = AdjustColorRules::DefaultRGB;
    QString m_cfgName;
    ReadThemeConfig *m_readCfg = nullptr;
    QString m_themeColor = "default";
    LINGMORadiusInformation::LINGMORadiusStruct m_radiusStruct;
    QPalette m_stylePalette;

};

}
#endif // LINGMOCONFIGSTYLEPARAMETERS_H
