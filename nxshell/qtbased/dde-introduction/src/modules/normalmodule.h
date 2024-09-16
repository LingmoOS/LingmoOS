// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALMODULE_H
#define NORMALMODULE_H

#include "../widgets/bottomnavigation.h"
#include "../widgets/navigationbutton.h"
#include "../widgets/closebutton.h"
#include "../worker.h"
#include "about.h"
#include "desktopmodemodule.h"
#include "iconmodule.h"
#include "photoslide.h"
#include "support.h"
#include "wmmodemodule.h"
#ifndef DISABLE_VIDEO
#include "videowidget.h"
#endif

#include <QButtonGroup>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>

#include <DFontSizeManager>
#include <DLabel>
#include <DPalette>
#include <DSuggestButton>
#include <DSysInfo>
#include <DWidget>

#include <com_deepin_wmswitcher.h>

using WMSwitcher = com::deepin::WMSwitcher;

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class ModuleInterface;
class NavigationButton;
//日常模式类
class NormalModule : public DWidget
{
    Q_OBJECT
public:
    explicit NormalModule(DWidget *parent = nullptr);
     ~NormalModule() override;

signals:
    void closeMainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    //键盘按键事件
    void keyPressEvent(QKeyEvent *) override;
    //鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;

private:
    //根据index值时时刷新导航按钮对应的界面
    void updateCurrentWidget(const int index);
    //更新m_button缓存的button
    void updataButton(QAbstractButton *btn);

private slots:
    void initTheme(int type);
    //更新描述信息的字体颜色
    void updateLabel();
    void updateInterface(int);
    void cancelCloseFrame();

    //击m_buttonGrp中的按钮，相应对应的内容界面信息　ut002764
    void ClickShow(QAbstractButton *btn);
    void RefeshQwidget();

private:
    CloseButton *m_closeFrame;
    //左侧导航布局
    QVBoxLayout *m_leftNavigationLayout;
    //右侧内容布局
    QVBoxLayout *m_rightContentLayout;
    //存储导航按钮组
    QButtonGroup *m_buttonGrp;
    //存储button和界面标记数字
    QMap<QWidget *, int> m_buttonMap;
    //对应按钮存储title内容
    QMap<QAbstractButton *, QString> m_titleMap;
    QMap<QAbstractButton *, QString> m_describeMap;
    //将数字和右侧内容映射
    QMap<int, QWidget *> m_modules;
    //存储动画或者轮播图的导航按钮
    NavigationButton *m_button;
    QWidget *m_currentWidget;
    //窗口特效DBUS返回值
    WMSwitcher *m_wmSwitcher;
    //标题
    DLabel *m_titleLabel;
    //描述信息
    DLabel *m_describe;
    //右侧内容界面
    DWidget *m_content;
    int m_index;
    bool isx86;
    //这是产品提出的需求，需求是一开始运行不按tab导航按钮是没有边框的，按了tab之后导航按钮会有边框
    bool tab_be_press;
    bool m_supportWM;
    qint64 m_keypressTime = 0;
};

#endif  // NORMALMODULE_H
