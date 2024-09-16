// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBLE_H
#define ACCESSIBLE_H

#include "accessibledefine.h"

#include "gui/toolbar.h"
#include "gui/main_window.h"
#include "gui/base/base_table_view.h"
#include "gui/base/base_item_delegate.h"
#include "gui/base/base_header_view.h"
#include "gui/process_table_view.h"
#include "gui/process_page_widget.h"
#include "gui/service_name_sub_input_dialog.h"
#include "gui/system_service_table_view.h"
#include "gui/system_service_page_widget.h"
#include "gui/monitor_expand_view.h"
#include "gui/monitor_compact_view.h"
#include "gui/priority_slider.h"
#include "gui/kill_process_confirm_dialog.h"
#include "gui/process_attribute_dialog.h"
#include "gui/dialog/error_dialog.h"
#include "gui/xwin_kill_preview_widget.h"
#include "gui/xwin_kill_preview_background_widget.h"
#include "gui/cpu_detail_widget.h"
#include "gui/mem_detail_view_widget.h"
#include "gui/mem_summary_view_widget.h"
#include "gui/block_dev_detail_view_widget.h"
#include "gui/block_dev_summary_view_widget.h"
#include "gui/netif_detail_view_widget.h"
#include "gui/netif_summary_view_widget.h"
#include "gui/detail_view_stacked_widget.h"
#include "compact_cpu_monitor.h"
#include "compact_disk_monitor.h"
#include "compact_memory_monitor.h"
#include "compact_network_monitor.h"
#include "memory_monitor.h"
#include "network_monitor.h"
#include "cpu_monitor.h"

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

#include <QStackedWidget>

DWIDGET_USE_NAMESPACE
//using namespace DCC_NAMESPACE;

SET_FORM_ACCESSIBLE(MainWindow, "MainWindow")
SET_FORM_ACCESSIBLE(ErrorDialog, "ErrorDialog")
SET_FORM_ACCESSIBLE(Toolbar, "Toolbar")
SET_FORM_ACCESSIBLE(BaseHeaderView, "BaseHeaderView")
SET_FORM_ACCESSIBLE(ProcessPageWidget, "ProcessPageWidget")
SET_FORM_ACCESSIBLE(ServiceNameSubInputDialog, "ServiceNameSubInputDialog")
SET_FORM_ACCESSIBLE(SystemServicePageWidget, "SystemServicePageWidget")
SET_FORM_ACCESSIBLE(MonitorExpandView, "MonitorExpandView")
SET_FORM_ACCESSIBLE(MonitorCompactView, "MonitorCompactView")
SET_FORM_ACCESSIBLE(PrioritySlider, "PrioritySlider")
SET_FORM_ACCESSIBLE(KillProcessConfirmDialog, "KillProcessConfirmDialog")
SET_FORM_ACCESSIBLE(ProcessAttributeDialog, "ProcessAttributeDialog")
SET_FORM_ACCESSIBLE(XWinKillPreviewWidget, "XWinKillPreviewWidget")
SET_FORM_ACCESSIBLE(XWinKillPreviewBackgroundWidget, "XWinKillPreviewBackgroundWidget")
SET_FORM_ACCESSIBLE(CPUDetailWidget, "CPUDetailWidget")
SET_FORM_ACCESSIBLE(MemDetailViewWidget, "MemDetailViewWidget")
SET_FORM_ACCESSIBLE(MemSummaryViewWidget, "MemSummaryViewWidget")
SET_FORM_ACCESSIBLE(BlockDevDetailViewWidget, "BlockDevDetailViewWidget")
SET_FORM_ACCESSIBLE(BlockDevSummaryViewWidget, "BlockDevSummaryViewWidget")
SET_FORM_ACCESSIBLE(NetifDetailViewWidget, "NetifDetailViewWidget")
SET_FORM_ACCESSIBLE(NetifSummaryViewWidget, "NetifSummaryViewWidget")
SET_FORM_ACCESSIBLE(DetailViewStackedWidget, "DetailViewStackedWidget")
SET_FORM_ACCESSIBLE(CompactCpuMonitor, "CompactCpuMonitor")
SET_FORM_ACCESSIBLE(CompactDiskMonitor, "CompactDiskMonitor")
SET_FORM_ACCESSIBLE(CompactMemoryMonitor, "CompactMemoryMonitor")
SET_FORM_ACCESSIBLE(CompactNetworkMonitor, "CompactNetworkMonitor")
SET_FORM_ACCESSIBLE(CpuMonitor, "CpuMonitor")
SET_FORM_ACCESSIBLE(MemoryMonitor, "MemoryMonitor")
SET_FORM_ACCESSIBLE(NetworkMonitor, "NetworkMonitor")
SET_FORM_ACCESSIBLE(UserPageWidget, "UserPageWidget")

// Qt控件
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->text().isEmpty() ? "qpushbutton" : m_w->text())
SET_SLIDER_ACCESSIBLE(QSlider, "qslider")
SET_FORM_ACCESSIBLE(QStackedWidget, "qstackedwidget")
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
    // 应用主窗口
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MainWindow);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ErrorDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), Toolbar);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), BaseHeaderView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ProcessPageWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ServiceNameSubInputDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SystemServicePageWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MonitorExpandView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MonitorCompactView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), PrioritySlider);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), KillProcessConfirmDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ProcessAttributeDialog);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), XWinKillPreviewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), XWinKillPreviewBackgroundWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CPUDetailWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MemDetailViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MemSummaryViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), BlockDevDetailViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), BlockDevSummaryViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), NetifDetailViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), NetifSummaryViewWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DetailViewStackedWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompactCpuMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompactDiskMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompactMemoryMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CompactNetworkMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), CpuMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MemoryMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), NetworkMonitor);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), UserPageWidget);

    //  Qt 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QSlider);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QStackedWidget);
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
