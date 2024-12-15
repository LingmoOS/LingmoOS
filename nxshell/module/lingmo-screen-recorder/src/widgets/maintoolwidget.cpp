// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maintoolwidget.h"
#include "../utils/configsettings.h"
#include "tooltips.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"

#include <DSlider>
#include <DLineEdit>
#include <DFontSizeManager>
#include <DWindowManagerHelper>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>

DWIDGET_USE_NAMESPACE

namespace {
    //const int TOOLBAR_HEIGHT = 43;
    //const int TOOLBAR_WIDTH = 155;
    const int BUTTON_SPACING = 3;
    //const int COLOR_NUM = 16;
    //const QSize TOOL_BUTTON_SIZE = QSize(70, 40);
    //const QSize MIN_TOOL_BUTTON_SIZE = QSize(50, 40);
}


MainToolWidget::MainToolWidget(DWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

MainToolWidget::~MainToolWidget()
{
    if(nullptr != hintFilter){
        delete hintFilter;
        hintFilter = nullptr;
    }
}

void MainToolWidget::initWidget()
{
    if(nullptr == hintFilter){
        hintFilter = new HintFilter(this);
    }
    initMainLabel();
    setCurrentWidget(m_mainTool);
}

void MainToolWidget::initMainLabel()
{
    m_mainTool = new DLabel(this);
    QList<ToolButton *> toolBtnList;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    DPalette pa;

    //    QString record_button_style = "DPushButton:press{QIcon(:/image/newUI/press/screencap-press.svg)}";

    m_recordBtn = new ToolButton();
    if (Utils::isTabletEnvironment || Utils::is3rdInterfaceStart) {
        m_recordBtn->hide();
    }
    DFontSizeManager::instance()->bind(m_recordBtn, DFontSizeManager::T8);
    m_recordBtn->setCheckable(true);
    m_recordBtn->setText(tr("Record"));
    Utils::setAccessibility(m_recordBtn, AC_MAINTOOLWIDGET_RECORD_BUT);
    m_recordBtn->setMinimumHeight(40);
    m_recordBtn->setIconSize(QSize(20, 20));
    m_recordBtn->setIcon(QIcon::fromTheme("screencap-normal"));
    toolBtnList.append(m_recordBtn);


    m_shotBtn = new ToolButton();
    if (Utils::isTabletEnvironment) {
        m_shotBtn->hide();
    }
    DFontSizeManager::instance()->bind(m_shotBtn, DFontSizeManager::T8);
    m_shotBtn->setText(tr("Screenshot"));
    Utils::setAccessibility(m_shotBtn, AC_MAINTOOLWIDGET_SHOT_BUT);
    m_shotBtn->setMinimumHeight(40);
    m_shotBtn->setIconSize(QSize(20, 20));
    m_shotBtn->setIcon(QIcon::fromTheme("screenshot-normal"));
    toolBtnList.append(m_shotBtn);

    m_baseLayout = new QHBoxLayout();
    m_baseLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_baseLayout->setContentsMargins(4, 0, 0, 0);
    m_baseLayout->setSpacing(0);
    m_baseLayout->addSpacing(5);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }
    m_mainTool->setLayout(m_baseLayout);
    addWidget(m_mainTool);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [ = ](int status) {
        Q_UNUSED(status);
        //DPalette pa;
        if (m_recordBtn->isChecked()) {
            m_recordBtn->setIconSize(QSize(20, 20));
            m_isChecked = true;
            m_recordBtn->update();
            emit buttonChecked(m_isChecked, "record");
        }else{
            m_recordBtn->setIconSize(QSize(20, 20));
        }

        if (m_shotBtn->isChecked()) {
            m_shotBtn->setIconSize(QSize(20, 20));
            m_isChecked = true;
            m_shotBtn->update();
            emit buttonChecked(m_isChecked, "shot");
        }else{
            m_shotBtn->setIconSize(QSize(20, 20));
        }
    });
    m_shotBtn->click();

}

void MainToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", this->parentWidget()->parentWidget()->parentWidget());
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void MainToolWidget::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    if(nullptr != hintFilter){
        w->installEventFilter(hintFilter);
    }
}

void MainToolWidget::setRecordButtonOut()
{
    m_recordBtn->setDisabled(true);
}

void MainToolWidget::setRecordLauchMode(const unsigned int funType)
{
    if (funType == MainWindow::record) {
        m_recordBtn->click();
    }
}
