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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef THEMEINFORMATION_H
#define THEMEINFORMATION_H

#include <QObject>
#include <QSettings>
#include <QColor>
#include <QBrush>
#include <QPen>

#define ConfigUserPath "/usr/share/qt5-lingmo-platformtheme/themeconfig/"
#define ColorPath "/usr/share/qt5-lingmo-platformtheme/themeconfig/"
#define UsrRadiusPath "/usr/share/qt5-lingmo-platformtheme/themeconfig/"
#define HomeCFGColorPath "/.config/qt5-lingmo-platformthemem/themeconfig/"
#define DefaultConfigName "default.json"

#define Obj_Value             "value"
#define Obj_Type              "type"
#define Obj_Color_Type        "color"
#define Obj_Gradient_Type     "gradient"
#define Obj_Int_Type          "int"
#define Obj_PaletteRole_Type  "paletterole"

#define LINGMO_Radius        "Radius"
#define Max_Radius    "Max_Radius"
#define Normal_Radius "Normal_Radius"
#define Min_Radius    "Min_Radius"

#define LINGMODarkTheme  "DarkTheme"
#define LINGMOLightTheme "LightTheme"
#define LINGMOPalette    "Palette"
#define ColorAdjustRules "ColorAdjustRules"

#define LINGMOPushButton                  "PushButton"
#define C_Button_DefaultPushButton      "DefaultPushButton"
#define C_Button_WindowCloseButton      "WindowCloseButton"
#define C_Button_WindowButton           "WindowButton"
#define C_Button_UseButtonPalette       "UseButtonPalette"
#define C_Button_ImportButton           "ImportButton"
#define C_Button_TranslucentButton      "TranslucentButton"
#define C_Button_DefaultBrush           "DefaultBrush"
#define C_Button_HoverBrush             "HoverBrush"
#define C_Button_DefaultPen             "DefaultPen"
#define C_Button_HoverPen               "HoverPen"
#define C_Button_ClickPen               "ClickPen"
#define C_Button_DisablePen             "DisablePen"
#define C_Button_CheckPen               "CheckPen"
#define C_Button_CheckHoverPen          "CheckHoverPen"
#define C_Button_CheckClickPen          "CheckClickPen"
#define C_Button_CheckDisablePen        "CheckDisablePen"
#define C_Button_FocusPen               "FocusPen"
#define C_Button_TextColor              "TextColor"
#define C_Button_TextHoverColor         "TextHoverColor"
#define C_Button_TextDisableColor       "TextDisableColor"

#define C_Button_ClickBrush             "ClickBrush"
#define C_Button_DisableBrush           "DisableBrush"
#define C_Button_FocusPenColor          "FocusPenColor"

#define LINGMOToolButton         "ToolButton"
#define C_Button_DefaultToolButton      "DefaultToolButton"

#define LINGMOCheckBox                   "CheckBox"
#define C_CheckBox_Default             "DefaultCheckBox"
#define C_CheckBox_Translucent         "TranslucentCheckBox"
#define C_CheckBox_DefaultPen          "DefaultPen"
#define C_CheckBox_HoverPen            "HoverPen"
#define C_CheckBox_ClickPen            "ClickPen"
#define C_CheckBox_DisablePen          "DisablePen"
#define C_CheckBox_OnDefaultPen        "OnDefaultPen"
#define C_CheckBox_OnHoverPen          "OnHoverPen"
#define C_CheckBox_OnClickPen          "OnClickPen"
#define C_CheckBox_ContentPen          "ContentPen"
#define C_CheckBox_DefaultBrush        "DefaultBrush"
#define C_CheckBox_HoverBrush          "HoverBrush"
#define C_CheckBox_ClickBrush          "ClickBrush"
#define C_CheckBox_DisableBrush        "DisableBrush"
#define C_CheckBox_OnDefaultBrush      "OnDefaultBrush"
#define C_CheckBox_OnHoverBrush        "OnHoverBrush"
#define C_CheckBox_OnClickBrush        "OnClickBrush"
#define C_CheckBox_PathBrush           "PathBrush"
#define C_CheckBox_PathDisableBrush    "PathDisableBrush"


#define LINGMORadioButton                       "RadioButton"
#define C_RadioButton_Default                 "DefaultRadioButton"
#define C_RadioButton_Translucent             "TranslucentRadioButton"
#define C_RadioButton_DefaultPen              "DefaultPen"
#define C_RadioButton_HoverPen                "HoverPen"
#define C_RadioButton_ClickPen                "ClickPen"
#define C_RadioButton_DisablePen              "DisablePen"
#define C_RadioButton_OnDefaultPen            "OnDefaultPen"
#define C_RadioButton_OnHoverPen              "OnHoverPen"
#define C_RadioButton_OnClickPen              "OnClickPen"
#define C_RadioButton_DefaultBrush            "DefaultBrush"
#define C_RadioButton_HoverBrush              "HoverBrush"
#define C_RadioButton_ClickBrush              "ClickBrush"
#define C_RadioButton_DisableBrush            "DisableBrush"
#define C_RadioButton_OnDefaultBrush          "OnDefaultBrush"
#define C_RadioButton_OnHoverBrush            "OnHoverBrush"
#define C_RadioButton_OnClickBrush            "OnClickBrush"
#define C_RadioButton_ChildrenOnDefaultBrush  "ChildrenOnDefaultBrush"
#define C_RadioButton_ChildrenOnHoverBrush    "ChildrenOnHoverBrush"
#define C_RadioButton_ChildrenOnClickBrush    "ChildrenOnClickBrush"
#define C_RadioButton_ChildrenOnDisableBrush  "ChildrenOnDisableBrush"

#define LINGMOLineEdit                       "LineEdit"
#define C_LineEdit_Default                 "DefaultLineEdit"
#define C_LineEdit_Translucent             "TranslucentLineEdit"
#define C_LineEdit_DefaultPen              "DefaultPen"
#define C_LineEdit_HoverPen                "HoverPen"
#define C_LineEdit_FocusPen                "FocusPen"
#define C_LineEdit_DisablePen              "DisablePen"
#define C_LineEdit_DefaultBrush            "DefaultBrush"
#define C_LineEdit_HoverBrush              "HoverBrush"
#define C_LineEdit_FocusBrush              "FocusBrush"
#define C_LineEdit_DisableBrush            "DisableBrush"

#define LINGMOComboBox                       "ComboBox"
#define C_ComboBox_Default                 "DefaultComboBox"
#define C_ComboBox_Translucent             "TranslucentComboBox"
#define C_ComboBox_DefaultPen              "DefaultPen"
#define C_ComboBox_HoverPen                "HoverPen"
#define C_ComboBox_OnPen                   "OnPen"
#define C_ComboBox_EditPen                 "EditPen"
#define C_ComboBox_DisablePen              "DisablePen"
#define C_ComboBox_FocusPen                "FocusPen"
#define C_ComboBox_DefaultBrush            "DefaultBrush"
#define C_ComboBox_HoverBrush              "HoverBrush"
#define C_ComboBox_OnBrush                 "OnBrush"
#define C_ComboBox_EditBrush               "EditBrush"
#define C_ComboBox_DisableBrush            "DisableBrush"

#define LINGMOSpinBox                        "SpinBox"
#define C_SpinBox_Default                  "DefaultSpinBox"
#define C_SpinBox_Translucent              "TranslucentSpinBox"
#define C_SpinBox_DefaultLayout            "DefaultLayout"
#define C_SpinBox_HorizonLayout            "HorizonLayout"
#define C_SpinBox_DefaultPen               "DefaultPen"
#define C_SpinBox_HoverPen                 "HoverPen"
#define C_SpinBox_FocusPen                 "FocusPen"
#define C_SpinBox_DisablePen               "DisablePen"
#define C_SpinBox_DefaultBrush             "DefaultBrush"
#define C_SpinBox_HoverBrush               "HoverBrush"
#define C_SpinBox_FocusBrush               "FocusBrush"
#define C_SpinBox_DisableBrush             "DisableBrush"
#define C_SpinBox_UpDefaultPen             "UpDefaultPen"
#define C_SpinBox_UpHoverPen               "UpHoverPen"
#define C_SpinBox_UpFocusPen               "UpFocusPen"
#define C_SpinBox_UpClickPen               "UpClickPen"
#define C_SpinBox_UpDisablePen             "UpDisablePen"
#define C_SpinBox_UpHoverBrush             "UpHoverBrush"
#define C_SpinBox_UpFocusHoverBrush        "UpFocusHoverBrush"
#define C_SpinBox_UpClickBrush             "UpClickBrush"
#define C_SpinBox_UpDefaultBrush           "UpDefaultBrush"

#define C_SpinBox_DownDefaultPen           "DownDefaultPen"
#define C_SpinBox_DownHoverPen             "DownHoverPen"
#define C_SpinBox_DownFocusPen             "DownFocusPen"
#define C_SpinBox_DownClickPen             "DownClickPen"
#define C_SpinBox_DownDisablePen           "DownDisablePen"
#define C_SpinBox_DownHoverBrush           "DownHoverBrush"
#define C_SpinBox_DownFocusHoverBrush      "DownFocusHoverBrush"
#define C_SpinBox_DownClickBrush           "DownClickBrush"
#define C_SpinBox_DownDefaultBrush         "DownDefaultBrush"

#define LINGMOTable                          "Table"
#define C_Table_Default                    "DefaultTable"
#define C_Table_Highlight                  "HighlightTable"
#define C_Table_DefaultBrush               "DefaultBrush"
#define C_Table_HoverBrush                 "HoverBrush"
#define C_Table_SelectBrush                "SelectBrush"
#define C_Table_DisableBrush               "DisableBrush"
#define C_Table_HoverPen                   "HoverPen"
#define C_Table_SelectPen                  "SelectPen"
#define C_Table_TextHoverPen               "TextHoverPen"
#define C_Table_TextSelectPen              "TextSelectPen"
#define C_Table_TextDisablePen             "TextDisablePen"
#define C_Table_FocusPen                   "FocusPen"

#define LINGMOListView                       "ListView"
#define C_ListView_Default                 "DefaultListView"
#define C_ListView_Highlight               "HighlightListView"
#define C_ListView_Translucent             "TranslucentListView"
#define C_ListView_DefaultBrush            "DefaultBrush"
#define C_ListView_HoverBrush              "HoverBrush"
#define C_ListView_SelectBrush             "SelectBrush"
#define C_ListView_DisableBrush            "DisableBrush"
#define C_ListView_TextHoverPen            "TextHoverPen"
#define C_ListView_TextSelectPen           "TextSelectPen"
#define C_ListView_TextDisablePen          "TextDisablePen"
#define C_ListView_HoverPen                "HoverPen"
#define C_ListView_SelectPen               "SelectPen"
#define C_ListView_FocusPen                "FocusPen"

#define LINGMOTreeView                       "TreeView"
#define C_TreeView_Default                 "DefaultTreeView"
#define C_TreeView_HighLight               "HighlightTreeView"
#define C_TreeView_Translucent             "TranslucentTreeView"
#define C_TreeView_DefaultBrush            "DefaultBrush"
#define C_TreeView_HoverBrush              "HoverBrush"
#define C_TreeView_SelectBrush             "SelectBrush"
#define C_TreeView_DisableBrush            "DisableBrush"
#define C_TreeView_BranchDefaultBrush      "BranchDefaultBrush"
#define C_TreeView_BranchHoverBrush        "BranchHoverBrush"
#define C_TreeView_BranchSelectBrush       "BranchSelectBrush"
#define C_TreeView_BranchDisableBrush      "BranchDisableBrush"
#define C_TreeView_TextHoverPen            "TextHoverPen"
#define C_TreeView_TextSelectPen           "TextSelectPen"
#define C_TreeView_TextDisablePen          "TextDisablePen"
#define C_TreeView_HoverPen                "HoverPen"
#define C_TreeView_SelectPen               "SelectPen"
#define C_TreeView_FocusPen                "FocusPen"
#define C_TreeView_BranchHoverPen          "BranchHoverPen"
#define C_TreeView_BranchSelectPen         "BranchSelectPen"

#define LINGMOSliderBar                           "SliderBar"
#define C_SliderBar_Default                     "DefaultSliderBar"
#define C_SliderBar_Translucent                 "TranslucentSliderBar"
#define C_SliderBar_HandleDefaultBrush          "HandleDefaultBrush"
#define C_SliderBar_HandleHoverBrush            "HandleHoverBrush"
#define C_SliderBar_HandleClickBrush            "HandleClickBrush"
#define C_SliderBar_HandleDisableBrush          "HandleDisableBrush"
#define C_SliderBar_GrooveValueDefaultBrush     "GrooveValueDefaultBrush"
#define C_SliderBar_GrooveValueHoverBrush       "GrooveValueHoverBrush"
#define C_SliderBar_GrooveValueDisableBrush     "GrooveValueDisableBrush"
#define C_SliderBar_GrooveUnvalueDefaultBrush   "GrooveUnvalueDefaultBrush"
#define C_SliderBar_GrooveUnvalueHoverBrush     "GrooveUnvalueHoverBrush"
#define C_SliderBar_GrooveUnvalueDisableBrush   "GrooveUnvalueDisableBrush"
#define C_SliderBar_HandleDefaultPen            "HandleDefaultPen"
#define C_SliderBar_HandleHoverPen              "HandleHoverPen"
#define C_SliderBar_HandleClickPen              "HandleClickPen"
#define C_SliderBar_HandleDisablePen            "HandleDisablePen"
#define C_SliderBar_SliderGroovePen             "SliderGroovePen"
#define C_SliderBar_FocusPen                    "FocusPen"


#define LINGMOProgressBar                         "ProgressBar"
#define C_ProgressBar_Default                   "DefaultProgressBar"
#define C_ProgressBar_Translucent               "TranslucentProgressBar"
#define C_ProgressBar_ContentStartColor         "ContentStart"
#define C_ProgressBar_ContentEndColor           "ContentEnd"
#define C_ProgressBar_ContentBrush              "ContentBrush"
#define C_ProgressBar_GrooveDefaultBrush        "GrooveDefaultBrush"
#define C_ProgressBar_GrooveDisableBrush        "GrooveDisableBrush"
#define C_ProgressBar_ContentPen                "ContentPen"
#define C_ProgressBar_GrooveDefaultPen          "GrooveDefaultPen"
#define C_ProgressBar_GrooveDisablePen          "GrooveDisablePen"

#define LINGMOToolTip                             "ToolTip"
#define C_ToolTip_Default                       "DefaultToolTip"
#define C_ToolTip_BackgroundBrush               "BackgroundBrush"
#define C_ToolTip_BackgroundPen                 "BackgroundPen"

#define LINGMOScrollBar                           "ScrollBar"
#define C_ScrollBar_Default                     "DefaultScrollBar"
#define C_ScrollBar_Translucent                 "TranslucentScrollBar"
#define C_ScrollBar_GrooveDefaultBrush          "GrooveDefaultBrush"
#define C_ScrollBar_GrooveInactiveBrush         "GrooveInactiveBrush"
#define C_ScrollBar_SliderDefaultBrush          "SliderDefaultBrush"
#define C_ScrollBar_SliderHoverBrush            "SliderHoverBrush"
#define C_ScrollBar_SliderClickBrush            "SliderClickBrush"
#define C_ScrollBar_SliderDisableBrush          "SliderDisableBrush"

#define LINGMOTabWidget                           "TabWidget"
#define C_TabWidget_Default                     "DefaultTabWidget"
#define C_TabWidget_WidgetBackgroundBrush       "WidgetBackgroundBrush"
#define C_TabWidget_TabBarBackgroundBrush       "TabBarBackgroundBrush"
#define C_TabWidget_TabBarDefaultBrush          "TabBarDefaultBrush"
#define C_TabWidget_TabBarHoverBrush            "TabBarHoverBrush"
#define C_TabWidget_TabBarSelectBrush           "TabBarSelectBrush"
#define C_TabWidget_TabBarClickBrush            "TabBarClickBrush"
#define C_TabWidget_TabBarHoverPen              "TabBarHoverPen"
#define C_TabWidget_TabBarClickPen              "TabBarClickPen"
#define C_TabWidget_TabBarFocusPen              "TabBarFocusPen"


#define LINGMOMenu                                "Menu"
#define C_Menu_Default                          "DefaultMenu"
#define C_Menu_MenuBackgroundBrush              "MenuBackgroundBrush"
#define C_Menu_MenuBackgroundPen                "MenuBackgroundPen"
#define C_Menu_MenuItemSelectBrush              "MenuItemSelectBrush"
#define C_Menu_MenuItemFocusPen                 "MenuItemFocusPen"
#define C_Menu_MenuTextHoverPen                 "MenuTextHoverPen"

#define c_windowText_at            "WindowText_at"
#define c_windowText_iat           "WindowText_iat"
#define c_windowText_dis           "WindowText_dis"
#define c_button_at                "Button_at"
#define c_button_iat               "Button_iat"
#define c_button_dis               "Button_dis"
#define c_light_at                 "Light_at"
#define c_light_iat                "Light_iat"
#define c_light_dis                "Light_dis"
#define c_midlight_at              "Midlight_at"
#define c_midlight_iat             "Midlight_iat"
#define c_midlight_dis             "Midlight_dis"
#define c_dark_at                  "Dark_at"
#define c_dark_iat                 "Dark_iat"
#define c_dark_dis                 "Dark_dis"
#define c_mid_at                   "Mid_at"
#define c_mid_iat                  "Mid_iat"
#define c_mid_dis                  "Mid_dis"
#define c_text_at                  "Text_at"
#define c_text_iat                 "Text_iat"
#define c_text_dis                 "Text_dis"
#define c_brightText_at            "BrightText_at"
#define c_brightText_iat           "BrightText_iat"
#define c_brightText_dis           "BrightText_dis"
#define c_buttonText_at            "ButtonText_at"
#define c_buttonText_iat           "ButtonText_iat"
#define c_buttonText_dis           "ButtonText_dis"
#define c_base_at                  "Base_at"
#define c_base_iat                 "Base_iat"
#define c_base_dis                 "Base_dis"
#define c_window_at                "Window_at"
#define c_window_iat               "Window_iat"
#define c_window_dis               "Window_dis"
#define c_shadow_at                "Shadow_at"
#define c_shadow_iat               "Shadow_iat"
#define c_shadow_dis               "Shadow_dis"
#define c_highLight_at             "HighLight_at"
#define c_highLight_iat            "HighLight_iat"
#define c_highLight_dis            "HighLight_dis"
#define c_highLightText_at         "HighLightText_at"
#define c_highLightText_iat        "HighLightText_iat"
#define c_highLightText_dis        "HighLightText_dis"
#define c_link_at                  "Link_at"
#define c_link_iat                 "Link_iat"
#define c_link_dis                 "Link_dis"
#define c_linkVisited_at           "LinkVisited_at"
#define c_linkVisited_iat          "LinkVisited_iat"
#define c_linkVisited_dis          "LinkVisited_dis"
#define c_alternateBase_at         "AlternateBase_at"
#define c_alternateBase_iat        "AlternateBase_iat"
#define c_alternateBase_dis        "AlternateBase_dis"
#define c_noRole_at                "NoRole_at"
#define c_noRole_iat               "NoRole_iat"
#define c_noRole_dis               "NoRole_dis"
#define c_toolTipBase_at           "ToolTipBase_at"
#define c_toolTipBase_iat          "ToolTipBase_iat"
#define c_toolTipBase_dis          "ToolTipBase_dis"
#define c_toolTipText_at           "ToolTipText_at"
#define c_toolTipText_iat          "ToolTipText_iat"
#define c_toolTipText_dis          "ToolTipText_dis"
#define c_placeholderText_at       "PlaceholderText_at"
#define c_placeholderText_iat      "PlaceholderText_iat"
#define c_placeholderText_dis      "PlaceholderText_dis"

namespace LINGMORadiusInformation {

typedef struct LINGMORadiusStruct
{
    int maxRadius = 8;
    int normalRadius = 6;
    int minRadius = 4;
}LINGMORadiusStruct;
}


class LINGMOPaletteRole : public QObject
{
    Q_OBJECT
public:
    enum PaletteRole
      {
        WindowText_at  = 0,
        WindowText_iat,
        WindowText_dis,
        Button_at,
        Button_iat,
        Button_dis,
        Light_at,
        Light_iat,
        Light_dis,
        Midlight_at,
        Midlight_iat,
        Midlight_dis,
        Dark_at,
        Dark_iat,
        Dark_dis,
        Mid_at,
        Mid_iat,
        Mid_dis,
        Text_at,
        Text_iat,
        Text_dis,
        BrightText_at,
        BrightText_iat,
        BrightText_dis,
        ButtonText_at,
        ButtonText_iat,
        ButtonText_dis,
        Base_at,
        Base_iat,
        Base_dis,
        Window_at,
        Window_iat,
        Window_dis,
        Shadow_at,
        Shadow_iat,
        Shadow_dis,
        HighLight_at,
        HighLight_iat,
        HighLight_dis,
        HighLightText_at,
        HighLightText_iat,
        HighLightText_dis,
        Link_at,
        Link_iat,
        Link_dis,
        LinkVisited_at,
        LinkVisited_iat,
        LinkVisited_dis,
        AlternateBase_at,
        AlternateBase_iat,
        AlternateBase_dis,
        NoRole_at,
        NoRole_iat,
        NoRole_dis,
        ToolTipBase_at,
        ToolTipBase_iat,
        ToolTipBase_dis,
        ToolTipText_at,
        ToolTipText_iat,
        ToolTipText_dis,
        PlaceholderText_at,
        PlaceholderText_iat,
        PlaceholderText_dis
      };
      Q_ENUM(PaletteRole)

};


namespace LINGMOColorTheme {
typedef struct PaletteColorStruct
{
    QColor windowText_at=       QColor(38, 38, 38);
    QColor windowText_iat=      QColor(0, 0, 0, 255*0.55);
    QColor windowText_dis=      QColor(0, 0, 0, 255*0.3);
    QColor button_at=           QColor(230, 230, 230 );
    QColor button_iat=          QColor(230, 230, 230);
    QColor button_dis=          QColor(233, 233, 233);
    QColor light_at=            QColor(255, 255, 255);
    QColor light_iat=           QColor(255, 255, 255);
    QColor light_dis=           QColor(242, 242, 242);
    QColor midlight_at=         QColor(218, 218, 218);
    QColor midlight_iat=        QColor(218, 218, 218);
    QColor midlight_dis=        QColor(230, 230, 230);
    QColor dark_at=             QColor(77, 77, 77);
    QColor dark_iat=            QColor(77, 77, 77);
    QColor dark_dis=            QColor(64, 64, 64);
    QColor mid_at=              QColor(115, 115, 115);
    QColor mid_iat=             QColor(115, 115, 115);
    QColor mid_dis=             QColor(102, 102, 102);
    QColor text_at=             QColor(38, 38, 38   );
    QColor text_iat=            QColor(38, 38, 38);
    QColor text_dis=            QColor(0, 0, 0, 255 * 0.3 );
    QColor brightText_at=       QColor(0, 0, 0);
    QColor brightText_iat=      QColor(0, 0, 0);
    QColor brightText_dis=      QColor(0, 0, 0);
    QColor buttonText_at=       QColor(38, 38, 38);
    QColor buttonText_iat=      QColor(38, 38, 38);
    QColor buttonText_dis=      QColor(179, 179, 179);
    QColor base_at=             QColor(255, 255, 255);
    QColor base_iat=            QColor(245, 245, 245);
    QColor base_dis=            QColor(237, 237, 237);
    QColor window_at=           QColor(245, 245, 245);
    QColor window_iat=          QColor(237, 237, 237);
    QColor window_dis=          QColor(230, 230, 230);
    QColor shadow_at=           QColor(0, 0, 0, 255 * 0.16);
    QColor shadow_iat=          QColor(0, 0, 0, 255 * 0.16);
    QColor shadow_dis=          QColor(0, 0, 0, 255 * 0.21);
    QColor highLight_at=        QColor(55, 144, 250);
    QColor highLight_iat=       QColor(55, 144, 250);
    QColor highLight_dis=       QColor(233, 233, 233 );
    QColor highLightText_at=    QColor(255, 255, 255);
    QColor highLightText_iat=   QColor(255, 255, 255);
    QColor highLightText_dis=   QColor(179, 179, 179);
    QColor link_at=             QColor(55, 144, 250  );
    QColor link_iat=            QColor(55, 144, 250  );
    QColor link_dis=            QColor(55, 144, 250  );
    QColor linkVisited_at=      QColor(114, 46, 209  );
    QColor linkVisited_iat=     QColor(114, 46, 209  );
    QColor linkVisited_dis=     QColor(114, 46, 209  );
    QColor alternateBase_at=    QColor(245, 245, 245 );
    QColor alternateBase_iat=   QColor(245, 245, 245 );
    QColor alternateBase_dis=   QColor(245, 245, 245 );
    QColor noRole_at=           QColor(240, 240, 240 );
    QColor noRole_iat=          QColor(240, 240, 240 );
    QColor noRole_dis=          QColor(217, 217, 217 );
    QColor toolTipBase_at=      QColor(255, 255, 255 );
    QColor toolTipBase_iat=     QColor(255, 255, 255 );
    QColor toolTipBase_dis=     QColor(255, 255, 255 );
    QColor toolTipText_at=      QColor(38, 38, 38    );
    QColor toolTipText_iat=     QColor(38, 38, 38    );
    QColor toolTipText_dis=     QColor(38, 38, 38    );
    QColor placeholderText_at=  QColor(0, 0, 0, 255 * 0.35 );
    QColor placeholderText_iat= QColor(0, 0, 0, 255 * 0.35 );
    QColor placeholderText_dis= QColor(0, 0, 0, 255 * 0.3  );
}PaletteColorCfg;

typedef struct ButtonColorStruct
{
    QColor defaultBrush  = QColor(230, 230, 230);
    QColor hoverBrush    = QColor(52, 137, 238);
    QColor clickBrush    = QColor(44, 115, 200);
    QColor disableBrush  = QColor(233, 233, 233);

    QColor defaultPen      =  Qt::NoPen;
    QColor hoverPen        =  Qt::NoPen;
    QColor clickPen        =  Qt::NoPen;
    QColor disablePen      =  Qt::NoPen;
    QColor focusPen        =  Qt::NoPen;

    QColor textDefaultColor = QColor(38, 38, 38);
    QColor textHoverColor   = QColor(38, 38, 38);
    QColor textDisableColor = QColor(38, 38, 38);

    QList<QColor> hoverGradientList;
    QString hoverType    = "color";

}ButtonColorCfg;

typedef struct ToolButtonColorStruct
{
    QColor defaultBrush  = QColor(230, 230, 230);
    QColor hoverBrush    = QColor(52, 137, 238);
    QList<QColor> hoverGradientList;
    QString hoverType    = "color";
    QColor clickBrush    = QColor(44, 115, 200);
    QColor disableBrush  = QColor(233, 233, 233);

    QColor defaultPen      =  Qt::NoPen;
    QColor hoverPen        =  Qt::NoPen;
    QColor clickPen        =  Qt::NoPen;
    QColor disablePen      =  Qt::NoPen;
    QColor focusPen        =  Qt::NoPen;

    QColor textDefaultColor = QColor(38, 38, 38);
    QColor textHoverColor   = QColor(38, 38, 38);
    QColor textDisableColor = QColor(38, 38, 38);

}ToolButtonColorCfg;

typedef struct CheckBoxColorStruct
{
    QColor defaultPen =        QColor(166, 166, 166);
    QColor hoverPen =          QColor(166, 166, 166);
    QColor clickPen =          QColor(115, 115, 115);
    QColor disablePen =        QColor(166, 166, 166);
    QColor onDefaultPen =      QColor(0, 0, 0, 0.1);
    QColor onHoverPen =        QColor(0, 0, 0, 0.1);
    QColor onClickPen =        QColor(0, 0, 0, 0.1);
    QColor contentPen =        QColor(255, 255, 255);
    QColor defaultBrush =      QColor(255, 255, 255);
    QColor hoverBrush =        QColor(0, 0, 0, 0.05);
    QColor clickBrush =        QColor(0, 0, 0, 0.15);
    QColor onDefaultBrush =    QColor(55, 144, 250);
    QColor onHoverBrush =      QColor(52, 137, 238);
    QColor onClickBrush =      QColor(44, 115, 200);
    QColor disableBrush =      QColor(233, 233, 233);
    QColor pathBrush =         QColor(255, 255, 255);
    QColor pathDisableBrush =  QColor(179, 179, 179);

    QList<QColor> hoverGradientList;
    QString hoverType = "color";
    QList<QColor> onHoverGradientList;
    QString onHoverType = "color";
    QList<QColor> onClickGradientList;
    QString onClickType = "color";
}CheckBoxColorCfg;

typedef struct RadioButtonColorStruct
{
    QColor defaultPen             = QColor(166, 166, 166);
    QColor hoverPen               = QColor(166, 166, 166);
    QColor clickPen               = QColor(115, 115, 115);
    QColor disablePen             = QColor(166, 166, 166);
    QColor onDefaultPen           = QColor(0, 0, 0, 0.1 );
    QColor onHoverPen             = QColor(0, 0, 0, 0.1 );
    QColor onClickPen             = QColor(0, 0, 0, 0.1 );
    QColor defaultBrush           = QColor(255, 255, 255);
    QColor hoverBrush             = QColor(0, 0, 0, 0.05);
    QColor clickBrush             = QColor(0, 0, 0, 0.15);
    QColor disableBrush           = QColor(55, 144, 250 );
    QColor onDefaultBrush         = QColor(52, 137, 238 );
    QColor onHoverBrush           = QColor(44, 115, 200 );
    QColor onClickBrush           = QColor(233, 233, 233);
    QColor childrenOnDefaultBrush = QColor(255, 255, 255);
    QColor childrenOnHoverBrush   = QColor(255, 255, 255);
    QColor childrenOnClickBrush   = QColor(230, 230, 230);
    QColor childrenOnDisableBrush = QColor(179, 179, 179);
    QList<QColor> onHoverGradientList;
    QString onHoverType = "color";
}RadioButtonColorCfg;

typedef struct LineEditColorStruct
{
    QColor defaultPen   = QColor(0, 0, 0, 0  );
    QColor hoverPen     = QColor(0, 0, 0, 0  );
    QColor disablePen   = QColor(0, 0, 0, 0  );
    QColor focusPen     = QColor(55, 144, 250);
    QColor defaultBrush = QColor(230, 230, 230);
    QColor hoverBrush   = QColor(216, 216, 216);
    QColor focusBrush   = QColor(255, 255, 255);
    QColor disableBrush = QColor(233, 233, 233);
    QList<QColor> hoverGradientList;
    QString hoverType = "color";
}LineEditColorCfg;

typedef struct ComboBoxColorStruct
{
    QColor defaultPen   = QColor(0, 0, 0, 0   );
    QColor hoverPen     = QColor(0, 0, 0, 0   );
    QColor disablePen   = QColor(0, 0, 0, 0   );
    QColor onPen        = QColor(0, 0, 0, 0   );
    QColor editPen      = QColor(44, 115, 200 );
    QColor defaultBrush = QColor(230, 230, 230);
    QColor hoverBrush   = QColor(216, 216, 216);
    QColor onBrush      = QColor(255, 255, 255);
    QColor editBrush    = QColor(255, 255, 255);
    QColor disableBrush = QColor(233, 233, 233);
    QColor focusPen     = QColor(44, 115, 200 );
}ComboBoxColorCfg;

typedef struct SpinBoxColorStruct
{
    QColor defaultPen          = QColor(0, 0, 0, 0   );
    QColor hoverPen            = QColor(0, 0, 0, 0   );
    QColor disablePen          = QColor(0, 0, 0, 0   );
    QColor focusPen            = QColor(55, 144, 250 );
    QColor defaultBrush        = QColor(230, 230, 230);
    QColor hoverBrush          = QColor(216, 216, 216);
    QColor focusBrush          = QColor(216, 216, 216);
    QColor disableBrush        = QColor(233, 233, 233);

    QColor upDefaultPen        = QColor(0, 0, 0, 0   );
    QColor upHoverPen          = QColor(0, 0, 0, 0   );
    QColor upDisablePen        = QColor(0, 0, 0, 0   );
    QColor upClickPen          = QColor(0, 0, 0, 0   );
    QColor upFocusPen          = QColor(0, 0, 0, 0   );
    QColor upHoverBrush        = QColor(218, 218, 218);
    QColor upFocusHoverBrush   = QColor(218, 218, 218);
    QColor upClickBrush        = QColor(184, 184, 184);
    QColor upDefaultBrush      = QColor(230, 230, 230);

    QColor downDefaultPen      = QColor(0, 0, 0, 0   );
    QColor downHoverPen        = QColor(0, 0, 0, 0   );
    QColor downDisablePen      = QColor(0, 0, 0, 0   );
    QColor downClickPen        = QColor(0, 0, 0, 0   );
    QColor downFocusPen        = QColor(0, 0, 0, 0   );
    QColor downHoverBrush      = QColor(218, 218, 218);
    QColor downFocusHoverBrush = QColor(218, 218, 218);
    QColor downClickBrush      = QColor(184, 184, 184);
    QColor downDefaultBrush    = QColor(230, 230, 230);

    QList<QColor> hoverGradientList;
    QString hoverType = "color";
    QList<QColor> upHoverBrushGradientList;
    QString upHoverBrushType = "color";
    QList<QColor> upFocusHoverBrushGradientList;
    QString upFocusHoverBrushType = "color";
    QList<QColor> downHoverBrushGradientList;
    QString downHoverBrushType = "color";
    QList<QColor> downFocusHoverBrushGradientList;
    QString downFocusHoverBrushType = "color";
}SpinBoxColorCfg;

typedef struct TableColorStruct
{
    QColor defaultBrush  = QColor(255, 255, 255, 0 );
    QColor hoverBrush    = QColor(0, 0, 0, 0.05    );
    QColor selectBrush   = QColor(55, 144, 250     );
    QColor disableBrush  = QColor(255, 255, 255, 0 );
    QColor hoverPen      = QColor(0, 0, 0, 0       );
    QColor selectPen     = QColor(0, 0, 0, 0       );
    QColor textHoverPen  = QColor(38, 38, 38       );
    QColor textSelectPen = QColor(255, 255, 255    );
    QColor textDisablePen= QColor(179, 179, 179    );
    QColor focusPen      = QColor(55, 144, 250     );
    QList<QColor> hoverGradientList;
    QString hoverType = "color";
}TableColorCfg;

typedef struct ListViewColorStruct
{
    QColor defaultBrush  = QColor(255, 255, 255, 0 );
    QColor hoverBrush    = QColor(0, 0, 0, 0.05    );
    QColor selectBrush   = QColor(55, 144, 250     );
    QColor disableBrush  = QColor(255, 255, 255, 0 );
    QColor textHoverPen  = QColor(38, 38, 38       );
    QColor textSelectPen = QColor(255, 255, 255    );
    QColor textDisablePen= QColor(179, 179, 179    );
    QColor hoverPen      = QColor(0, 0, 0, 0       );
    QColor selectPen     = QColor(0, 0, 0, 0       );
    QColor focusPen      = QColor(55, 144, 250     );
    QList<QColor> hoverGradientList;
    QString hoverType = "color";
}ListViewColorCfg;

typedef struct TreeViewColorStruct
{
    QColor defaultBrush       = QColor(255, 255, 255, 0 );
    QColor hoverBrush         = QColor(0, 0, 0, 0.05    );
    QColor selectBrush        = QColor(55, 144, 250     );
    QColor disableBrush       = QColor(255, 255, 255, 0 );
    QColor branchDefaultBrush = QColor(255, 255, 255, 0 );
    QColor branchHoverBrush   = QColor(0, 0, 0, 0.05    );
    QColor branchSelectBrush  = QColor(55, 144, 250     );
    QColor branchDisableBrush = QColor(255, 255, 255, 0 );
    QColor textHoverPen       = QColor(38, 38, 38       );
    QColor textSelectPen      = QColor(255, 255, 255    );
    QColor textDisablePen     = QColor(179, 179, 179    );
    QColor hoverPen           = QColor(0, 0, 0, 0   );
    QColor selectPen          = QColor(0, 0, 0, 0   );
    QColor branchhoverPen     = QColor(0, 0, 0, 0   );
    QColor branchselectPen    = QColor(0, 0, 0, 0   );
    QColor focusPen           = QColor(55, 144, 250 );
    QList<QColor> hoverGradientList;
    QString hoverType = "color";
    QList<QColor> branchHoverGradientList;
    QString branchHoverType = "color";

}TreeViewColorCfg;

typedef struct SliderBarColorStruct
{
    QColor handleDefaultBrush        = QColor(55, 144, 250  );
    QColor handleHoverBrush          = QColor(55, 137, 238  );
    QColor handleClickBrush          = QColor(44, 115, 200  );
    QColor handleDisableBrush        = QColor(179, 179, 179 );
    QColor grooveValueDefaultBrush   = QColor(55, 144, 250  );
    QColor grooveValueHoverBrush     = QColor(44, 115, 200  );
    QColor grooveValueDisableBrush   = QColor(179, 179, 179 );
    QColor grooveUnvalueDefaultBrush = QColor(230, 230, 230 );
    QColor grooveUnvalueHoverBrush   = QColor(230, 230, 230 );
    QColor grooveUnvalueDisableBrush = QColor(233, 233, 233 );
    QColor handleDefaultPen = Qt::NoPen;
    QColor handleHoverPen   = Qt::NoPen;
    QColor handleClickPen   = Qt::NoPen;
    QColor handleDisablePen = Qt::NoPen;
    QColor sliderGroovePen  = Qt::NoPen;
    QColor focusPen  = Qt::NoPen;

    QList<QColor> handleHoverGradientList;
    QString handleHoverType = "color";
}SliderBarColorCfg;

typedef struct ProgressBarColorStruct
{
    QColor contentStartColor  = QColor(55, 144, 250  );
    QColor contentEndColor    = QColor(97, 173, 255  );
    QColor contentBrush       = QColor(55, 144, 250  );
    QColor grooveDefaultBrush = QColor(230, 230, 230 );
    QColor grooveDisableBrush = QColor(233, 233, 233 );
    QColor contentPen         = QColor(0, 0, 0, 0);
    QColor grooveDefaultPen   = QColor(0, 0, 0, 0);
    QColor grooveDisablePen   = QColor(0, 0, 0, 0);
}ProgressBarColorCfg;

typedef struct ScrollBarColorStruct
{
    QColor grooveDefaultBrush  = QColor(255, 255, 255);
    QColor grooveInactiveBrush = QColor(245, 245, 245);
    QColor sliderDefaultBrush  = QColor(230, 230, 230);
    QColor sliderHoverBrush    = QColor(218, 218, 218);
    QColor sliderClickBrush    = QColor(184, 184, 184);
    QColor sliderDisableBrush  = QColor(233, 233, 233);
    QList<QColor> sliderHoverGradientList;
    QString sliderHoverType = "color";
}ScrollBarColorCfg;

typedef struct ToolTipColorStruct
{
    QColor backgroundBrush = QColor(255, 255, 255);
    QColor backgroundPen = QColor(38, 38, 38, 0.15);
}ToolTipColorCfg;

typedef struct TabWidgetColorStruct
{
    QColor widgetBackgroundBrush = QColor(255, 255, 255);
    QColor tabBarBackgroundBrush = QColor(245, 245, 245);
    QColor tabBarDefaultBrush    = QColor(245, 245, 245);
    QColor tabBarHoverBrush      = QColor(232, 232, 232);
    QColor tabBarSelectBrush     = QColor(255, 255, 255);
    QColor tabBarClickBrush      = QColor(255, 255, 255);
    QColor tabBarDefaultPen      = QColor(0, 0, 0, 0);
    QColor tabBarHoverPen        = QColor(0, 0, 0, 0);
    QColor tabBarClickPen        = QColor(0, 0, 0, 0);
    QColor tabBarFocusPen        = QColor(55, 144, 250);

    QList<QColor> tabBarHoverGradientList;
    QString tabBarHoverType = "color";
}TabWidgetColorCfg;

typedef struct MenuColorStruct
{
    QColor menuBackgroundBrush = QColor(255, 255, 255);
    QColor menuBackgroundPen   = QColor(38, 38, 38, 0.15);
    QColor menuItemSelectBrush = QColor(55, 144, 250);
    QColor menuTextHoverPen    = QColor(255, 255, 255);
    QColor menuItemFocusPen    = QColor(55, 144, 250);
}MenuColorCfg;
}

#endif // THEMEINFORMATION_H
