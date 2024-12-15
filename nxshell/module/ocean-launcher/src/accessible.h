// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessibledefine.h"

#include <DIconButton>
#include <DSwitchButton>
#include <DPushButton>

#include <dboxwidget.h>

#include <QScrollBar>
#include <QLineEdit>
#include <QMenu>

#include "./widgets/avatar.h"
#include "./widgets/gradientlabel.h"
#include "./widgets/miniframebutton.h"
#include "./widgets/miniframerightbar.h"
#include "./widgets/miniframeswitchbtn.h"
#include "./widgets/searchlineedit.h"
#include "./widgets/applistarea.h"

#include "./view/appgridview.h"
#include "./view/applistview.h"

#include "fullscreenframe.h"
#include "windowedframe.h"
#include "editlabel.h"
#include "iconbutton.h"

DWIDGET_USE_NAMESPACE

// 添加accessible

SET_BUTTON_ACCESSIBLE(DIconButton, m_w->objectName())
SET_BUTTON_ACCESSIBLE(DSwitchButton, m_w->text())
SET_FORM_ACCESSIBLE(AppGridView, "AppGridView")
SET_BUTTON_ACCESSIBLE(MiniFrameButton, m_w->accessibleName())
SET_FORM_ACCESSIBLE(QMenu, "Menu")
SET_FORM_ACCESSIBLE(AppListArea, "AppListArea")
SET_FORM_ACCESSIBLE(FullScreenFrame, "FullScreenFrame")
SET_LABEL_ACCESSIBLE(BoxFrame, "LauncherFrame")
SET_FORM_ACCESSIBLE(SearchWidget, "SearchWidget")
SET_EDITABLE_ACCESSIBLE(QLineEdit, "LineEdit")
SET_BUTTON_ACCESSIBLE(Avatar, "Avatar")
SET_BUTTON_ACCESSIBLE(MiniFrameSwitchBtn, "MiniFrameSwitchBtn")
SET_FORM_ACCESSIBLE(MiniFrameRightBar, "MiniFrameRightBar")
SET_FORM_ACCESSIBLE(WindowedFrame, "WindowedFrame")
SET_FORM_ACCESSIBLE(QWidget, "widget")
SET_FORM_ACCESSIBLE(QFrame, "frame")
SET_SLIDER_ACCESSIBLE(QScrollBar, "scrollerbar")
SET_LABEL_ACCESSIBLE(GradientLabel, "gradientlabel")
SET_FORM_ACCESSIBLE(QPushButton, "QPushButton")
SET_FORM_ACCESSIBLE(QToolButton, "QToolButton")
SET_FORM_ACCESSIBLE(QLabel, "QLabel")
SET_FORM_ACCESSIBLE(MaskQWidget, "MaskQWidget")
SET_FORM_ACCESSIBLE(DHBoxWidget, "DHBoxWidget")
SET_FORM_ACCESSIBLE(DBoxWidget, "DBoxWidget")
SET_FORM_ACCESSIBLE(SearchModeWidget, "SearchModeWidget")
SET_FORM_ACCESSIBLE(AppDrawerWidget, "AppDrawerWidget")
SET_FORM_ACCESSIBLE(MultiPagesView, "MultiPagesView")
SET_FORM_ACCESSIBLE(IconButton, "IconButton")
SET_FORM_ACCESSIBLE(ModeSwitch, "ModeSwitch")
SET_FORM_ACCESSIBLE(EditLabel, "EditLabel")

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    // 自动化标记确定不需要的控件，方可加入忽略列表,QLabel目前DTK没有修改完成，导致部分QLabel没有设置AccessName,导致过不了Q_ASSERT_X
    // TO_DO dtk完善DSearchEdit设置AccessName内容
    const static QStringList ignoreLst = {"QLineEditIconButton","QLabel"};

    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, AppGridView);
    USE_ACCESSIBLE(classname, AppListArea);
    USE_ACCESSIBLE(classname, FullScreenFrame);
    USE_ACCESSIBLE(classname, MiniFrameButton);
    USE_ACCESSIBLE(classname, BoxFrame);
    USE_ACCESSIBLE(classname, SearchWidget);
    USE_ACCESSIBLE(classname, QLineEdit);
    USE_ACCESSIBLE(classname, Avatar);
    USE_ACCESSIBLE(classname, WindowedFrame);
    USE_ACCESSIBLE(classname, MiniFrameRightBar);
    USE_ACCESSIBLE(classname, MiniFrameSwitchBtn);
    USE_ACCESSIBLE_BY_OBJECTNAME(QString(classname).replace("Dtk::Widget::", ""), DIconButton, object->objectName());
    USE_ACCESSIBLE_BY_OBJECTNAME(QString(classname).replace("Dtk::Widget::", ""), DSwitchButton, object->objectName());
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, QFrame);
    USE_ACCESSIBLE(classname, QScrollBar);
    USE_ACCESSIBLE(classname, GradientLabel);
    USE_ACCESSIBLE(classname, QPushButton);
    USE_ACCESSIBLE(classname, QToolButton);
    USE_ACCESSIBLE(classname, MaskQWidget);
    USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DHBoxWidget);
    USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DBoxWidget);
    USE_ACCESSIBLE(classname, SearchModeWidget)
    USE_ACCESSIBLE(classname, AppDrawerWidget)
    USE_ACCESSIBLE(classname, MultiPagesView)
    USE_ACCESSIBLE(classname, IconButton)
    USE_ACCESSIBLE(classname, ModeSwitch)
    USE_ACCESSIBLE(classname, EditLabel)

    if (!interface && object->inherits("QWidget") && !ignoreLst.contains(classname)) {
        QWidget *w = static_cast<QWidget *>(object);
        // 如果你看到这里的输出，说明代码中仍有控件未兼顾到accessible功能，请帮忙添加
        if (w->accessibleName().isEmpty())
            qWarning() << "accessibleFactory()" + QString("Class: " + classname + " cannot access");
    }

    return interface;
}
