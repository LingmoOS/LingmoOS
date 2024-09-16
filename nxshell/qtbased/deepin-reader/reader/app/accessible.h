// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBLE_H
#define ACCESSIBLE_H

#include "accessibledefine.h"
#include "MainWindow.h"
#include "Central.h"
#include "CentralDocPage.h"
#include "CentralNavPage.h"
#include "DocSheet.h"
#include "DocTabBar.h"
#include "TitleMenu.h"
#include "TitleWidget.h"
#include "BrowserPage.h"
#include "BrowserMagniFier.h"
#include "SheetBrowser.h"
#include "SheetSidebar.h"
#include "ThumbnailWidget.h"
#include "CatalogWidget.h"
#include "BookMarkWidget.h"
#include "NotesWidget.h"
#include "SearchResWidget.h"

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

DWIDGET_USE_NAMESPACE

SET_FORM_ACCESSIBLE(MainWindow, "MainWindow")
SET_FORM_ACCESSIBLE(Central, "Central")
SET_FORM_ACCESSIBLE(CentralDocPage, "CentralDocPage")
SET_FORM_ACCESSIBLE(CentralNavPage, "CentralNavPage")
SET_FORM_ACCESSIBLE(DocSheet, "DocSheet")
SET_FORM_ACCESSIBLE(DocTabBar, "DocTabBar")
SET_FORM_ACCESSIBLE(TitleWidget, "TitleWidget")
SET_FORM_ACCESSIBLE(SheetSidebar, "SheetSidebar")
SET_FORM_ACCESSIBLE(SheetBrowser, "SheetBrowser")
SET_FORM_ACCESSIBLE(ThumbnailWidget, "ThumbnailWidget")
SET_FORM_ACCESSIBLE(CatalogWidget, "CatalogWidget")
SET_FORM_ACCESSIBLE(BookMarkWidget, "BookMarkWidget")
SET_FORM_ACCESSIBLE(NotesWidget, "NotesWidget")
SET_FORM_ACCESSIBLE(SearchResWidget, "SearchResWidget")

// Qt控件
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->text().isEmpty() ? "qpushbutton" : m_w->text())
SET_SLIDER_ACCESSIBLE(QSlider, "qslider")

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
SET_LABEL_ACCESSIBLE(DLabel, m_w->objectName().isEmpty() ? "DLabel" : m_w->objectName())   //不生效
SET_FORM_ACCESSIBLE(DTitlebar, m_w->objectName().isEmpty() ? "DTitlebar" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(DToolButton, m_w->objectName().isEmpty() ? "DToolButton" : m_w->objectName())
SET_FORM_ACCESSIBLE(DDialog, m_w->objectName().isEmpty() ? "DDialog" : m_w->objectName())
SET_FORM_ACCESSIBLE(DFileDialog, m_w->objectName().isEmpty() ? "DFileDialog" : m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    Q_UNUSED(object)

    QAccessibleInterface *interface = nullptr;
    // 应用主窗口
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MainWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), Central);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CentralDocPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CentralNavPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DocSheet);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DocTabBar);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), TitleWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SheetSidebar);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SheetBrowser);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ThumbnailWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CatalogWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), BookMarkWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), NotesWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SearchResWidget);

    //  Qt 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QSlider);

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
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DFileDialog);

    return interface;
}

#endif // ACCESSIBLE_H
