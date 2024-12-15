// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/accessibledefine.h"

#include "bubbleitem.h"
#include "bubbletitlewidget.h"
#include "itemdelegate.h"
#include "notifycenterwidget.h"
#include "notifylistview.h"
#include "notifymodel.h"
#include "notifywidget.h"
#include "overlapwidet.h"
#include "notification/iconbutton.h"
#include "notification/notificationentity.h"
#include "notification/appicon.h"
#include "notification/appbody.h"
#include "notification/actionbutton.h"
#include "notification/button.h"
#include "notification/icondata.h"
#include "notification/bubbletool.h"
#include "notification/iconbutton.h"
#include "notification/appbodylabel.h"
#include "notification/persistence.h"
#include "notification/signalbridge.h"

#include <DSwitchButton>
#include <DPushButton>
#include <DListView>
#include <DSwitchButton>
#include <DSpinner>
#include <dloadingindicator.h>

#include <QScrollBar>
#include <QLabel>
#include <QAccessibleWidget>
#include <DButtonBox>

// 添加accessible
SET_FORM_ACCESSIBLE(AppBody, "AppBody")
SET_LABEL_ACCESSIBLE(AppBodyLabel, "AppBodyLabel")
SET_FORM_ACCESSIBLE(AppIcon, "AppIcon")
SET_FORM_ACCESSIBLE(ButtonContent, "ButtonContent")
SET_FORM_ACCESSIBLE(ButtonMenu, "ButtonMenu")
SET_BUTTON_ACCESSIBLE(ActionButton, "ActionButton")
SET_BUTTON_ACCESSIBLE(CicleIconButton, "CicleIconButton")

SET_FORM_ACCESSIBLE(BubbleItem, "BubbleItem")
SET_FORM_ACCESSIBLE(AlphaWidget, "AlphaWidget")
SET_FORM_ACCESSIBLE(BubbleTitleWidget, "BubbleTitleWidget")
SET_FORM_ACCESSIBLE(NotifyCenterWidget, "NotifyCenterWidget")
SET_FORM_ACCESSIBLE(NotifyListView, "NotifyListView")
SET_FORM_ACCESSIBLE(NotifyWidget, "NotifyWidget")
SET_FORM_ACCESSIBLE(HalfRoundedRectWidget, "HalfRoundedRectWidget")
SET_FORM_ACCESSIBLE(OverLapWidet, "OverLapWidet")

QAccessibleInterface *notificationAccessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(classname, AppBody)
            ELSE_USE_ACCESSIBLE(classname, AppBodyLabel)
            ELSE_USE_ACCESSIBLE(classname, AppIcon)
            ELSE_USE_ACCESSIBLE(classname, ButtonContent)
            ELSE_USE_ACCESSIBLE(classname, ButtonMenu)
            ELSE_USE_ACCESSIBLE(classname, ActionButton)
            ELSE_USE_ACCESSIBLE(classname, CicleIconButton)
            ELSE_USE_ACCESSIBLE(classname, BubbleItem)
            ELSE_USE_ACCESSIBLE(classname, AlphaWidget)
            ELSE_USE_ACCESSIBLE(classname, BubbleTitleWidget)
            ELSE_USE_ACCESSIBLE(classname, NotifyCenterWidget)
            ELSE_USE_ACCESSIBLE(classname, NotifyListView)
            ELSE_USE_ACCESSIBLE(classname, NotifyWidget)
            ELSE_USE_ACCESSIBLE(classname, HalfRoundedRectWidget)
            ELSE_USE_ACCESSIBLE(classname, OverLapWidet)

    return interface;
}
