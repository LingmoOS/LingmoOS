// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colortoolwidget.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"

#include <DSlider>
#include <DLineEdit>
#include <DPalette>
#include <QButtonGroup>
#include <QStyleFactory>
#include <QDebug>


DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 50;
const int TOOLBAR_WIDTH = 120;
const QSize TOOL_BUTTON_SIZE = QSize(18, 18);
const QSize TOOL_ICON_SIZE = QSize(14, 14);
}

ColorToolWidget::ColorToolWidget(DWidget *parent) : DLabel(parent)
{
    initWidget();
    m_function = "rectangle";
}

ColorToolWidget::~ColorToolWidget()
{

}

void ColorToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setMouseTracking(true);
    setAcceptDrops(true);
//    this->setAttribute(Qt::WA_StyledBackground,true);
//    this->setStyleSheet("background-color:rgb(120,120,120)");
    initColorLabel();
}


void ColorToolWidget::initColorLabel()
{
    //颜色按钮租
    m_colorButtonGroup = new QButtonGroup(this);
    m_colorButtonGroup->setExclusive(true);

    //主布局
    m_baseLayout = new QGridLayout();
    //获取颜色枚举对象
    m_buttonColors = QMetaEnum::fromType<BaseUtils::ButtonColors>();
    qDebug() << "Utils::pixelRatio: " << Utils::pixelRatio;
    for (int i = 0; i < m_buttonColors.keyCount(); i++) {
        //qDebug() << "==========colorButton" << i << "===========";

        //颜色按钮
        ToolButton *colorButton = new ToolButton();
//        colorButton->setAttribute(Qt::WA_StyledBackground,true);
//        colorButton->setStyleSheet("background-color:rgb(0,120,120)");
        //设置自定义属性
        colorButton->setProperty("name", m_buttonColors.key(i));
        colorButton->setCheckable(true);
        //设置自动化名称
        Utils::setAccessibility(colorButton, QString("%1_button").arg(m_buttonColors.key(i)));
        //是固定按钮的高度
        colorButton->setFixedSize(TOOL_BUTTON_SIZE);
        //设置按钮为圆形20
//        colorButton->setStyleSheet(QString("QPushButton#{border-radius:%1px}").arg(TOOL_BUTTON_SIZE.width() / 2));
        //设置按钮图标
        QString iconPath = QString(":/color_pen/%1.svg").arg(m_buttonColors.key(i));
        QIcon icon(iconPath);
        icon.actualSize(TOOL_ICON_SIZE);
        if(Utils::pixelRatio != 1){
            //缩放情况下需要通过此方式进行图标加载，否则出现图标被遮挡的情况
            colorButton->setStyleSheet(QString("QPushButton{image:url(%1);"
                                               "image-position:center;"
                                               "padding-left:2.2px;padding-top:2.2px;padding-right:2.2px;padding-bottom:2.2px;}").arg(iconPath));
        }else{
            colorButton->setIcon(icon);
            colorButton->setIconSize(TOOL_ICON_SIZE-QSize(2,2));
        }
        m_colorButtonGroup->addButton(colorButton);
        m_colorButtonGroup->setId(colorButton, m_buttonColors.value(i));
        if (i < m_buttonColors.keyCount() / 2) {
            //第一排
            m_baseLayout->addWidget(colorButton,0,i);
        } else {
            //第二排
            m_baseLayout->addWidget(colorButton,1,i-m_buttonColors.keyCount() / 2);
        }
        //qDebug() << "==========colorButton" << i << "===========";
    }

//    m_baseLayout->addLayout(upHBox);
//    m_baseLayout->addSpacing(2);
//    m_baseLayout->addLayout(downHBox);
//    m_baseLayout->addSpacing(2);
    setLayout(m_baseLayout);

    //响应点击颜色按钮事件
    connect(m_colorButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
    [ = ](QAbstractButton * button) {
        //获取当前点击的按钮
        ToolButton *tempColorBtn = static_cast<ToolButton *>(button) ;
        if (tempColorBtn->isChecked()) {
            m_isChecked = true;
            tempColorBtn->update();
            //发射当前点击按钮的名称
            emit colorChecked(tempColorBtn->property("name").toString());
            //将当前点击的颜色按钮写入到配置文件
            ConfigSettings::instance()->setValue(m_function, "color_index", m_buttonColors.keyToValue(tempColorBtn->property("name").toString().toLatin1()));
        }
    });
}

void ColorToolWidget::setFunction(const QString &func)
{
    if (func == "effect") {
        this->hide();
        return;
    } else {
        this->show();
    }
    m_function = func;
    int t_color = 0;
    t_color = ConfigSettings::instance()->getValue(m_function, "color_index").toInt();

    m_colorButtonGroup->button(t_color)->click();

}
