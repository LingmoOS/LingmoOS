// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBLE_H
#define ACCESSIBLE_H

#include "utils/accessibledefine.h"
#include "view/pages/singleinstallpage.h"
#include "view/pages/uninstallconfirmpage.h"
#include "view/pages/multipleinstallpage.h"
#include "view/pages/AptConfigMessage.h"
#include "view/widgets/choosefilebutton.h"
#include "view/widgets/debinfolabel.h"
#include "view/pages/debinstaller.h"
#include "view/widgets/workerprogress.h"
#include "view/widgets/droundbgframe.h"
#include "view/widgets/filechoosewidget.h"
#include "view/widgets/InfoCommandLinkButton.h"
#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/installprocessinfoview.h"
#include "model/packagelistview.h"
#include "model/deblistmodel.h"

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

DWIDGET_USE_NAMESPACE
// using namespace DCC_NAMESPACE;
SET_FORM_ACCESSIBLE(SingleInstallPage, "SingleInstallPage")
SET_FORM_ACCESSIBLE(UninstallConfirmPage, "UninstallConfirmPage")
SET_FORM_ACCESSIBLE(MultipleInstallPage, "MultipleInstallPage")
SET_FORM_ACCESSIBLE(AptConfigMessage, "AptConfigMessage")
SET_FORM_ACCESSIBLE(ChooseFileButton, "ChooseFileButton")
SET_FORM_ACCESSIBLE(DebInfoLabel, "DebInfoLabel")
SET_FORM_ACCESSIBLE(DebInstaller, "DebInstaller")
SET_FORM_ACCESSIBLE(WorkerProgress, "WorkerProgress")
SET_FORM_ACCESSIBLE(DRoundBgFrame, "DRoundBgFrame")
SET_FORM_ACCESSIBLE(FileChooseWidget, "FileChooseWidget")
SET_FORM_ACCESSIBLE(InfoCommandLinkButton, "InfoCommandLinkButton")
SET_FORM_ACCESSIBLE(InfoControlButton, "InfoControlButton")
SET_FORM_ACCESSIBLE(InstallProcessInfoView, "InstallProcessInfoView")

// Qt控件
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName().isEmpty() ? "frame" : m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName().isEmpty() ? "widget" : m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->text().isEmpty() ? "qpushbutton" : m_w->text())
SET_SLIDER_ACCESSIBLE(QSlider, "qslider")
SET_FORM_ACCESSIBLE(QMenu, "qmenu")
// SET_LABEL_ACCESSIBLE(QLabel, m_w->text().isEmpty() ? "qlabel" : m_w->text())

// Dtk控件

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
// SET_LABEL_ACCESSIBLE(DLabel, m_w->text().isEmpty() ? "DLabel" : m_w->text())
SET_BUTTON_ACCESSIBLE(DToolButton, m_w->objectName().isEmpty() ? "DToolButton" : m_w->objectName())
SET_FORM_ACCESSIBLE(DDialog, m_w->objectName().isEmpty() ? "DDialog" : m_w->objectName())
SET_FORM_ACCESSIBLE(DFileDialog, m_w->objectName().isEmpty() ? "DFileDialog" : m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    // 应用主窗口
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), AptConfigMessage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), ChooseFileButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DebInfoLabel);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DebInstaller);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), DRoundBgFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), FileChooseWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), InfoCommandLinkButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), InfoControlButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), InstallProcessInfoView);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), MultipleInstallPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), SingleInstallPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), UninstallConfirmPage);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), WorkerProgress);

    //  Qt 控件
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QFrame);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QWidget);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QPushButton);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QSlider);
    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QMenu);
    //    USE_ACCESSIBLE(QString(classname).replace("dccV20::", ""), QLabel);

    //  dtk 控件
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

#endif  // ACCESSIBLE_H
