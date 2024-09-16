// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconmodule.h"

IconModule::IconModule(QWidget *parent)
    : ModuleInterface(parent)
    , m_scroll(new QScrollArea)
    , m_layout(new DFlowLayout)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_scroll, 0, Qt::AlignCenter);
    setLayout(layout);

    m_scrollWidget = new QWidget;
    QVBoxLayout *widgetLayout = new QVBoxLayout;
    m_scrollWidget->setLayout(widgetLayout);

    m_scrollWidget->installEventFilter(this);

    setMouseTracking(true);

    widgetLayout->setContentsMargins(0, 30, 0, 25);
    widgetLayout->setMargin(0);
    widgetLayout->setSpacing(0);
    widgetLayout->addStretch();
    widgetLayout->addLayout(m_layout);
    widgetLayout->addStretch();

    m_layout->setSpacing(10);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(10, 5, 0, 14);
    m_layout->setVerticalSpacing(36);

    m_selectBtn->setParent(m_scrollWidget);

    m_scroll->setWidget(m_scrollWidget);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFocusPolicy(Qt::NoFocus);
    m_scroll->setFrameStyle(DFrame::NoFrame);
    m_scroll->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_scroll->setContentsMargins(0, 0, 0, 0);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_model, &Model::iconAdded, this, &IconModule::addIcon);
    connect(m_model, &Model::iconRemoved, this, &IconModule::removeIcon);
    connect(m_model, &Model::iconChanged, this, &IconModule::currentIconChanged);

    //m_model是在父类ModuleInterface中new的
    for (const IconStruct &icon : m_model->iconList()) {
        addIcon(icon);
    }

    if (QSysInfo::currentCpuArchitecture().contains(QString("mips"))) {
        m_bIsMispCpuArch = true;
    }

    updateSmallIcon();
}

/*******************************************************************************
 1. @函数:    addIcon
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    拷贝主题图标到当前界面并保存
*******************************************************************************/
void IconModule::addIcon(const IconStruct &icon)
{
     if (m_iconList.keys().contains(icon) && icon.Id.isEmpty()) {
         return;
     }

     BaseWidget *base = new BaseWidget(this);

     QPixmap pixmap(icon.Pixmap);
     pixmap.setDevicePixelRatio(devicePixelRatioF());

     base->setPixmap(pixmap);
     base->setTitle(icon.Id);

     connect(base, &BaseWidget::clicked, this, [=] {
         m_worker->setIcon(icon);
     });

     //保存主题图标信息
     m_iconList[icon] = base;

     //显示到主题图标界面
     m_layout->insertWidget(m_model->iconList().indexOf(icon), base);
}

/*******************************************************************************
 1. @函数:    removeIcon
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    删除图标，删除界面和本地保存
*******************************************************************************/
void IconModule::removeIcon(const IconStruct &icon)
{
    BaseWidget *base = m_iconList[icon];

    m_layout->removeWidget(base);

    m_iconList.remove(icon);
    if(base)
        base->deleteLater();
}

/*******************************************************************************
 1. @函数:    currentIconChanged
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    当前界面主题图标变化响应函数
*******************************************************************************/
void IconModule::currentIconChanged(const IconStruct &icon)
{
    if (icon.Id.isEmpty()) {
        return;
    }

    m_selectBtn->raise();

    BaseWidget * w = m_iconList[icon];
    if(w == nullptr) //添加为空判断，防止奔溃
        return;
    QPoint p = w->mapTo(m_scrollWidget, m_iconList[icon]->rect().topRight())  - QPoint(9, 5);
    m_scroll->ensureVisible(p.x(), p.y() + m_height, 50, m_height);
    m_selectBtn->move(w->mapTo(m_scrollWidget, m_iconList[icon]->rect().topRight())  - QPoint(11, 7));

    for (BaseWidget *base : m_iconList.values()) {
        base->setChecked(base == w);
    }
}

//首次启动初始化图片
void IconModule::updateBigIcon()
{
    const QSize size(700, 278);
    setFixedSize(size);
    m_scroll->setFixedSize(size);
    m_scrollWidget->setFixedWidth(size.width());
    m_layout->setContentsMargins(10, 45, 0, 15);
    updateSelectBtnPos();

    QMapIterator<IconStruct, BaseWidget*>map(m_iconList);

    while (map.hasNext()) {
        map.next();
        QPixmap pixmap(map.key().Pixmap);
        pixmap = pixmap.scaled(QSize(320, 70) * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        map.value()->setPixmap(pixmap);
    }
    m_height = 71;
}

//日常启动初始化图片
void IconModule::updateSmallIcon()
{
    const QSize size(550, 270);
    setFixedSize(size);
    m_scroll->setFixedSize(size);
    m_scrollWidget->setFixedWidth(size.width());
    updateSelectBtnPos();

    QMapIterator<IconStruct, BaseWidget*>map(m_iconList);

    while (map.hasNext()) {
        map.next();
        QPixmap pixmap(map.key().Pixmap);
        pixmap = pixmap.scaled(QSize(240, 60) * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        map.value()->setPixmap(pixmap);
        map.value()->setLayoutSpacing(10);
    }
    m_height = 60;
}

//更新选择按钮位置
void IconModule::updateSelectBtnPos()
{
    currentIconChanged(m_model->currentIcon());
}

//键盘按键事件
void IconModule::keyPressEvent(QKeyEvent *e)
{
    IconStruct icon = m_model->currentIcon();
    int index = -1;
    for (int i = 0; i < m_model->iconList().size(); ++i) {
        if (m_model->iconList().at(i) == icon) {
            index = i;
            break;
        }
    }
    if (e->key() == Qt::Key_Left) {
        if (index == 2){
            QScrollBar *pScrollBar = m_scroll->verticalScrollBar();
            pScrollBar->setValue(pScrollBar->value()-pScrollBar->pageStep());
        }
        if (index < 1)
            return;
        m_worker->setIcon(m_model->iconList().at(index - 1));
    } else if (e->key() == Qt::Key_Right) {
        if (index == m_model->iconList().size() - 1)
            return;
        m_worker->setIcon(m_model->iconList().at(index + 1));
    } else if (e->key() == Qt::Key_Up) {//首次启动时有效果
        if (index == 2 || index == 3){
            QScrollBar *pScrollBar = m_scroll->verticalScrollBar();
            pScrollBar->setValue(pScrollBar->value()-pScrollBar->pageStep());
        }
        if (index < 2)
            return;
        m_worker->setIcon(m_model->iconList().at(index - 2));
    } else if (e->key() == Qt::Key_Down) {//首次启动时有效果
        if (index > m_model->iconList().size() - 3)
            return;
        m_worker->setIcon(m_model->iconList().at(index + 2));
    } else {
        return;
    }
    updateSelectBtnPos();
}

bool IconModule::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_scrollWidget && event->type() == QEvent::Resize) {
        m_updateSelectBtnTimer->start();
    }

    return ModuleInterface::eventFilter(watched, event);
}

//鼠标点击事件
void IconModule::mousePressEvent(QMouseEvent *event)
{
    //鼠标点击时，取消选中框
    emit cancelCloseFrame();
    m_TempPoint = event->pos();
}

//滑动鼠标下位， 鼠标左键按住之后，可以上下拖动
void IconModule::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        auto pos = event->pos();
        auto vbar = m_scroll->verticalScrollBar();
        //向上滑动 向下滚动
        auto offset = m_TempPoint.y() - pos.y();
        //获取当前滚动条位置
        auto val = vbar->value();
        //获取滚动条pagetemp区域高度
        auto step = vbar->pageStep();
        //设计移动位置
        auto move   = offset * step / m_scrollWidget->height();
        if(move + val < 0 || move + val > m_scrollWidget->height())
            return;
        //移动
        vbar->setValue(move + val);
    }
}



