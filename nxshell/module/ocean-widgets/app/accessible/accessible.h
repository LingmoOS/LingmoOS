// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/accessibledefine.h"

#include "mainview.h"
#include "displaymodepanel.h"
#include "editmodepanel.h"
#include "widgetstore.h"
#include "animationviewcontainer.h"
#include "instanceproxy.h"
#include "aboutdialog.h"
#include "button.h"

#include <DIconButton>
#include <DSwitchButton>
#include <DPushButton>
#include <DListView>
#include <DSwitchButton>
#include <DSpinner>
#include <DLabel>
#include <DTipLabel>
#include <dloadingindicator.h>

#include <QScrollBar>
#include <QLabel>
#include <QAccessibleWidget>
#include <DButtonBox>

WIDGETS_FRAME_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

// 添加accessible
SET_FORM_ACCESSIBLE(MainView, "mainview")
SET_FORM_ACCESSIBLE(AnimationViewContainer, "attentioncontainer")
SET_FORM_ACCESSIBLE(WidgetStore, "widgetstore")
SET_FORM_ACCESSIBLE(EditModePanel, "editmodepanel")
SET_FORM_ACCESSIBLE(DisplayModePanel, "displaymodepanel")
SET_FORM_ACCESSIBLE(PluginCell, "plugincell")
SET_FORM_ACCESSIBLE(WidgetStoreCell, "widgetstorecell")
SET_FORM_ACCESSIBLE(DragDropWidget, "DragDropWidget")
SET_FORM_ACCESSIBLE(InstancePanelCell, "InstancePanelCell")
SET_FORM_ACCESSIBLE(EditModePanelCell, "editmodepanelcell")
SET_FORM_ACCESSIBLE(DisplayModePanelCell, "displaymodepanelcell")
SET_FORM_ACCESSIBLE(WidgetContainer, "WidgetContainer")
SET_FORM_ACCESSIBLE(InstanceAboutDialog, "InstanceAboutDialog")
SET_FORM_ACCESSIBLE(TransparentButton, "TransparentButton")

SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_LABEL_ACCESSIBLE(QLabel, m_w->objectName().isEmpty() ? m_w->text().isEmpty() ? "text" : m_w->text() : m_w->objectName())
// 几个没什么用的标记，但为了提醒大家不要遗漏标记控件，还是不要去掉
SET_BUTTON_ACCESSIBLE(DIconButton, m_w->objectName().isEmpty() ? "imagebutton" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DButtonBox, "ButtonBox")
SET_BUTTON_ACCESSIBLE(DSwitchButton, m_w->text().isEmpty() ? "switchbutton" : m_w->text())
SET_BUTTON_ACCESSIBLE(DButtonBoxButton, m_w->objectName().isEmpty() ? "boxbutton" : m_w->objectName())
SET_LABEL_ACCESSIBLE(DLabel, m_w->objectName().isEmpty() ? "DLabel" : m_w->objectName())
SET_LABEL_ACCESSIBLE(DTipLabel, m_w->objectName().isEmpty() ? "DTipLabel" : m_w->objectName())
SET_FORM_ACCESSIBLE(DBlurEffectWidget, "DBlurEffectWidget")
SET_FORM_ACCESSIBLE(DListView, "DListView")
SET_FORM_ACCESSIBLE(DLoadingIndicator, "DLoadingIndicator")
SET_FORM_ACCESSIBLE(DSpinner, "DSpinner")
SET_FORM_ACCESSIBLE(QMenu, "QMenu")
SET_FORM_ACCESSIBLE(QPushButton, "QPushButton")
SET_FORM_ACCESSIBLE(QSlider, "QSlider")
SET_FORM_ACCESSIBLE(QScrollBar, "QScrollBar")
SET_FORM_ACCESSIBLE(QScrollArea, "QScrollArea")
SET_FORM_ACCESSIBLE(QFrame, "QFrame")
SET_FORM_ACCESSIBLE(QGraphicsView, "QGraphicsView")

static const QString getWidgetsFrameNamespaceString()
{
#define WIDGETS_FRAME_GET_NAMESPACE_STR_IMPL(M) #M "::"
#define WIDGETS_FRAME_GET_NAMESPACE_STR(M) WIDGETS_FRAME_GET_NAMESPACE_STR_IMPL(M)
    return WIDGETS_FRAME_GET_NAMESPACE_STR(WIDGETS_FRAME_NAMESPACE);
#undef WIDGETS_FRAME_GET_NAMESPACE_STR
#undef WIDGETS_FRAME_GET_NAMESPACE_STR_IMPL
}

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    // 自动化标记确定不需要的控件，方可加入忽略列表
    const static QStringList ignoreLst = {};

    QAccessibleInterface *interface = nullptr;

    const QString NamespaceString(getWidgetsFrameNamespaceString());

    USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), MainView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), AnimationViewContainer)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), WidgetStore)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), EditModePanel)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), DisplayModePanel)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), PluginCell)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), WidgetStoreCell)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), EditModePanelCell)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), DisplayModePanelCell)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), WidgetContainer)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), InstanceAboutDialog)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), InstancePanelCell)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), DragDropWidget)
            ELSE_USE_ACCESSIBLE(QString(classname).replace(NamespaceString, ""), TransparentButton)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), QWidget)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), QLabel)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DBlurEffectWidget)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DListView)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DLoadingIndicator)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DSpinner)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DSwitchButton)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DIconButton)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DButtonBox)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DButtonBoxButton)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DLabel)
            ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DTipLabel)
            ELSE_USE_ACCESSIBLE(classname, QMenu)
            ELSE_USE_ACCESSIBLE(classname, QPushButton)
            ELSE_USE_ACCESSIBLE(classname, QSlider)
            ELSE_USE_ACCESSIBLE(classname, QScrollBar)
            ELSE_USE_ACCESSIBLE(classname, QScrollArea)
            ELSE_USE_ACCESSIBLE(classname, QFrame)
            ELSE_USE_ACCESSIBLE(classname, QGraphicsView)

    if (!interface && object->inherits("QWidget") && !ignoreLst.contains(classname)) {
        QWidget *w = static_cast<QWidget *>(object);
        // 如果你看到这里的输出，说明代码中仍有控件未兼顾到accessible功能，请帮忙添加
        if (w->accessibleName().isEmpty())
            qWarning(dwLog()) << "accessibleFactory()" + QString("Class: " + classname + " cannot access");
    }

    return interface;
}

DWIDGET_END_NAMESPACE
