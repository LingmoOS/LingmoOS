/*
 * SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#ifndef COMMON_H
#define COMMON_H

#include <QtGlobal>

namespace chameleon {

enum Metrics
{
    // painter
    Painter_PenWidth = 1,

    // frames
    Frame_FrameWidth = 2,
    Frame_FrameRadius = 8,
    Frame_BorderSpacing = 2,

    // shadow
    Shadow_Radius = 6,
    Shadow_XOffset = 0,
    Shadow_YOffset = 2,

    // layout
    Layout_TopLevelMarginWidth = 10,
    Layout_ChildMarginWidth = 4,
    Layout_DefaultSpacing = 6,

    // line editors
    LineEdit_FrameWidth = 2,
    LineEdit_LeftMargin = 3,

    // combobox
    ComboBox_FrameWidth = 5,
    ComboBox_ArrowButtonWidth = 40,
    ComboBox_ContentLeftMargin = 6,

    // spinbox
    SpinBox_FrameWidth = 1,
    SpinBox_ArrowButtonWidth = 20,
    SpinBox_ButtonIconWidth = 2,
    SpinBox_MiniHeight = 44,

    // groupbox title margin
    GroupBox_TitleMarginWidth = 4,

    // buttons
    Button_MinWidth = 80,
    Button_MarginWidth = 6,
    Button_ItemSpacing = 4,

    // tool buttons
    ToolButton_MarginWidth = 6,
    ToolButton_ItemSpacing = 4,
    ToolButton_InlineIndicatorWidth = 12,
    ToolButton_AlignLeftPadding = 10,
    ToolButton_AlignCenterPadding = 8,
    ToolButton_AlignRightPadding = 9,
    ToolButton_AlignLeftMargin = 7,

    // checkboxes and radio buttons
    CheckBox_Size = 20,
    CheckBox_FocusMarginWidth = 2,
    CheckBox_ItemSpacing = 4,
    CheckBox_FrameWidth = 14,

    // menu
    Menu_ItemHMargin = 0,
    Menu_ItemVMargin = -1,
    Menu_ArrowHMargin = 6,
    Menu_CheckMarkWidth = 12,
    Menu_RightBorder = 10,
    Menu_ItemFrameWidth = 8,
    Menu_ItemHTextMargin = 3,
    Menu_ItemVTextMargin = 4,
    Menu_FrameWidth = 0,
    Menu_HMargin = 0,
    Menu_VMargin = 10,
    Menu_SeparatorItemHMargin = 5,
    Menu_PanelRightPadding = 10,
    MenuItem_MarginWidth = 3,
    MenuItem_ItemSpacing = 4,
    MenuItem_AcceleratorSpace = 16,
    MenuItem_TabSpacing = 16,
    MenuButton_IndicatorMargin = 8,

    // menu bar
    MenuBar_ItemSpacing = 6,
    MenuBarItem_MarginWidth = 10,
    MenuBarItem_MarginHeight = 6,

    // scrollbars
    ScrollBar_Extend = 12,
    ScrollBar_SliderWidth = 16,
    ScrollBar_MinSliderHeight = 40,
    ScrollBar_NoButtonHeight = (ScrollBar_Extend-ScrollBar_SliderWidth)/2,
    ScrollBar_SingleButtonHeight = ScrollBar_Extend,
    ScrollBar_DoubleButtonHeight = 2*ScrollBar_Extend,
    ScrollBar_SliderMinWidget = 36,

    // toolbars
    ToolBar_FrameWidth = 2,
    ToolBar_HandleExtent = 10,
    ToolBar_HandleWidth = 6,
    ToolBar_SeparatorWidth = 8,
    ToolBar_ExtensionWidth = 20,
    ToolBar_ItemSpacing = 0,

    // progressbars
    ProgressBar_BusyIndicatorSize = 14,
    ProgressBar_ItemSpacing = 4,
    ProgressBar_MinimumStyleHeight = 8,

    // mdi title bar
    TitleBar_MarginWidth = 4,

    // sliders
    Slider_TickLength = 20,         //滑块的长度
    Slider_TickMarginWidth = 6,
    Slider_GrooveThickness = 2,
    Slider_ControlThickness = 24,   //滑块的高度
    Slider_TickmarkOffset = 14,      //刻度的高度

    // tabbar
    TabBar_TabMarginHeight = 8,
    TabBar_TabMarginWidth = 12,
    TabBar_TabMinWidth = 80,
    TabBar_TabMinHeight = 20,
    TabBar_TabItemSpacing = 8,
    TabBar_TabOverlap = 0,
    TabBar_BaseOverlap = 0,
    TabBar_TabMargin = 10,
    TabBar_TabButtonSize = 22,      // 标签页按钮大小
    TabletTabBar_TabButtonSize = 36,// 平板环境标签页按钮大小

    // tab widget
    TabWidget_MarginWidth = 4,

    // toolbox
    ToolBox_TabMinWidth = 80,
    ToolBox_TabItemSpacing = 4,
    ToolBox_TabMarginWidth = 8,

    // tooltips
    ToolTip_FrameWidth = 3,

    // list headers
    Header_MarginWidth = 6,
    Header_ItemSpacing = 4,
    Header_ArrowSize = 12,

    // tree view
    ItemView_ArrowSize = 10,
    ItemView_ItemMarginWidth = 3,
    SidePanel_ItemMarginWidth = 4,

    // splitter
    Splitter_SplitterWidth = 1,

    // shadow dimensions
    Shadow_Overlap = 2,

    // group box
    GroupBox_TopMargin = 3,

    // progress bar
    ProgressBar_BorderWidth = 1,
    ProgressBar_Radius = 4,

    // icon
    Icon_Size = 64,
    Icon_Margins = 10
};

//* corners
enum Corner
{
    CornerTopLeft = 0x1,
    CornerTopRight = 0x2,
    CornerBottomLeft = 0x4,
    CornerBottomRight = 0x8,
    CornersTop = CornerTopLeft|CornerTopRight,
    CornersBottom = CornerBottomLeft|CornerBottomRight,
    CornersLeft = CornerTopLeft|CornerBottomLeft,
    CornersRight = CornerTopRight|CornerBottomRight,
    AllCorners = CornerTopLeft|CornerTopRight|CornerBottomLeft|CornerBottomRight
};

Q_DECLARE_FLAGS(Corners, Corner)

} // namespace chameleon

Q_DECLARE_OPERATORS_FOR_FLAGS(chameleon::Corners)

#endif // COMMON_H
