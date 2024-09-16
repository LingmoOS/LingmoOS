// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCESSIBLE_H
#define ACCESSIBLE_H

#include "accessibledefine.h"

#include "../widget/yearWidget/yearwindow.h"
#include "../widget/monthWidget/monthwindow.h"
#include "../widget/weekWidget/weekwindow.h"
#include "../widget/dayWidget/daywindow.h"
#include "../customWidget/scheduleRemindWidget.h"
#include "../view/alldayeventview.h"
#include "../view/monthgraphiview.h"
#include "../view/graphicsview.h"
#include "../customWidget/customframe.h"
#include "../customWidget/animationstackedwidget.h"

#include <DSwitchButton>
#include <DBackgroundGroup>
#include <DFloatingButton>
#include <DLineEdit>
#include <DLabel>
#include <DListView>
#include <DCommandLinkButton>
#include <DSearchEdit>
#include <DTitlebar>
#include <DComboBox>
#include <DCheckBox>
#include <DTreeView>
#include <DIconButton>
#include <DToolButton>
#include <DProgressBar>
#include <DTextEdit>
#include <DDialog>
#include <DFileDialog>
#include <DFrame>
#include <DPushButton>

DWIDGET_USE_NAMESPACE
//using namespace DCC_NAMESPACE;

SET_FORM_ACCESSIBLE(CYearWindow, "YearWidget");
SET_FORM_ACCESSIBLE(CMonthWindow, "MonthWidget");
SET_FORM_ACCESSIBLE(CWeekWindow, "WeekWidget");
SET_FORM_ACCESSIBLE(CDayWindow, "DayWidget");
SET_FORM_ACCESSIBLE(ScheduleRemindWidget, "ScheduleRemindWidget");
SET_FORM_ACCESSIBLE(CAllDayEventWeekView, "CAllDayEventWeekView");
SET_FORM_ACCESSIBLE(CMonthGraphicsview, "CMonthGraphicsview");
SET_FORM_ACCESSIBLE(CGraphicsView, "CGraphicsView");
SET_FORM_ACCESSIBLE(CustomFrame, "CustomFrame");
SET_FORM_ACCESSIBLE(AnimationStackedWidget, "AnimationStackedWidget");

// Qt控件
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->text().isEmpty() ? "qpushbutton" : m_w->text())
SET_SLIDER_ACCESSIBLE(QSlider, "qslider")
SET_FORM_ACCESSIBLE(QMenu, "qmenu")
//SET_LABEL_ACCESSIBLE(QLabel, m_w->text().isEmpty() ? "qlabel" : m_w->text())

// Dtk控件
SET_FORM_ACCESSIBLE(DFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(DWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_FORM_ACCESSIBLE(DBackgroundGroup, m_w->objectName().isEmpty() ? "dbackgroundgroup" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DSwitchButton, m_w->text().isEmpty() ? "switchbutton" : m_w->text())
SET_BUTTON_ACCESSIBLE(DFloatingButton, m_w->toolTip().isEmpty() ? "DFloatingButton" : m_w->toolTip())
SET_FORM_ACCESSIBLE(DSearchEdit, m_w->objectName().isEmpty() ? "DSearchEdit" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DPushButton, m_w->objectName().isEmpty() ? "DPushButton" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DIconButton, m_w->objectName().isEmpty() ? "DIconButton" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DCheckBox, m_w->objectName().isEmpty() ? "DCheckBox" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DCommandLinkButton, "DCommandLinkButton")
SET_FORM_ACCESSIBLE(DTitlebar, m_w->objectName().isEmpty() ? "DTitlebar" : m_w->objectName())
//SET_LABEL_ACCESSIBLE(DLabel, m_w->text().isEmpty() ? "DLabel" : m_w->text())
SET_BUTTON_ACCESSIBLE(DToolButton, m_w->objectName().isEmpty() ? "DToolButton" : m_w->objectName())
SET_FORM_ACCESSIBLE(DDialog, m_w->objectName().isEmpty() ? "DDialog" : m_w->objectName())
SET_FORM_ACCESSIBLE(DFileDialog, m_w->objectName().isEmpty() ? "DFileDialog" : m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;

    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CYearWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CMonthWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CWeekWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CDayWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ScheduleRemindWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CAllDayEventWeekView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CMonthGraphicsview);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CGraphicsView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CustomFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), AnimationStackedWidget);

    //      Qt 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QSlider);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QMenu);
    //    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QLabel);

    //  dtk 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DBackgroundGroup);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DSwitchButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFloatingButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DSearchEdit);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DIconButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DCheckBox);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DCommandLinkButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DTitlebar);
    //    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DLabel);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFileDialog);

    return interface;
}

#endif // ACCESSIBLE_H
