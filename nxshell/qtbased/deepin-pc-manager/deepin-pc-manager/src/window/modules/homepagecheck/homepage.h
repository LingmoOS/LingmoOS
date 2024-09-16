// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dtkwidget_global.h"
#include "homepagemodel.h"
#include "window/namespace.h"

#include <DGuiApplicationHelper>
#include <DSuggestButton>
#include <DTipLabel>

#include <QPushButton>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DSuggestButton;
class DLabel;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

//// 安全级别界限分数
// 非常安全分数界限
#define PERFECTLY_SAFE_SCORE_ABOVE 90
// 比较安全分数界限
#define SAFE_SCORE_ABOVE 75
// 比较危险分数界限
#define DANGEROUS_SCORE_ABOVE 60

//// 分数安全级别文字颜色
// 非常安全分数颜色
#define PERFECTLY_SAFE_SCORE_COLOR QColor("#00C134")
// 非常安全分数 HTML格式颜色
#define PERFECTLY_SAFE_SCORE_COLOR_HTML "#00C134"

// 比较安全分数 颜色
#define SAFE_SCORE_COLOR QColor("#8CD400")
// 比较安全分数 HTML格式颜色
#define SAFE_SCORE_COLOR_HTML "#8CD400"

// 比较危险分数 颜色
#define DANGEROUS_SCORE_COLOR QColor("#FF5C00")
// 比较危险分数 HTML格式颜色
#define DANGEROUS_SCORE_COLOR_HTML "#FF5C00"

// 非常危险分数 颜色
#define EXTREMELY_DANGEROUS_SCORE_COLOR QColor("#D80000")
// 非常危险分数 HTML格式颜色
#define EXTREMELY_DANGEROUS_SCORE_COLOR_HTML "#D80000"

// 未体检时，要求进行体检 或 第一次体检
#define REQUEST_PROCESS_CHECK_COLOR QColor("#FF5C00")
// 要求体检时HTML格式颜色
#define REQUEST_PROCESS_CHECK_COLOR_HTML "#FF5C00"

//// 分数安全级别阴影颜色
// 非常安全阴影颜色
#define PERFECTLY_SAFE_SCORE_SHADOW_COLOR QColor(35, 196, 0, 127)
// 比较安全阴影颜色
#define SAFE_SCORE_SHADOW_COLOR QColor(158, 202, 0, 127)
// 比较危险阴影颜色
#define DANGEROUS_SCORE_SHADOW_COLOR QColor(255, 93, 0, 127)
// 非常危险阴影颜色
#define EXTREMELY_DANGEROUS_SCORE_SHADOW_COLOR QColor(216, 49, 108, 127)

// 评分规则
#define SCORE_ANTIVIRUS_ABNORMAL 30 // 病毒扫描结果异常扣除的分数
#define SCORE_VIRUS_LIB_VER_ABNORMAL 10 // 病毒库版本可更新扣除的分数
#define SCORE_AUTOSTART_ABNORMAL 5 // 自启动应用数异常扣除的分数
#define SCORE_SSH_ABNORMAL 5 // SSH异常扣除的分数
#define SCORE_SYS_VER_ABNORMAL 5 // 系统本可更新异常扣除的分数
#define SCORE_CLEANER_ABNORMAL_FIRST_STAGE 5 // 垃圾清理第一阶段异常扣除的分数
#define SCORE_CLEANER_ABNORMAL_SEC_STAGE 10 // 垃圾清理第二阶段异常扣除的分数
#define SCORE_DISCK_CHECK_ABNORMAL 15 // 磁盘检测异常扣除的分数
#define SCORE_ROOT_MODEL_ABNORMAL 5 // 处于开发者模式扣除的分数

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(HomePageModel *mode, QWidget *parent = nullptr);
    ~HomePage();
    // 设置数据处理对象
    void setModel(HomePageModel *mode);
    // 更新界面
    void updateUI();

private:
Q_SIGNALS:
    // 通知打开体检页
    void notifyShowCheckPage();

public Q_SLOTS:

    // 随主题类型改变首页图片
    void changePicWithTheme(DGuiApplicationHelper::ColorType themeType);

private:
    // 首页体检数据处理类
    HomePageModel *m_mode;
    // 应用界面工具对象，方便使用系统主题相关方法
    DGuiApplicationHelper *m_guiHelper;
    // 体检图片
    QLabel *m_scorePic;
    // 分数提示项
    DLabel *m_lbScoreTip;
    // 欢迎语
    DLabel *m_greetingInfo;
    // 体检提示
    DTipLabel *m_examTip;
    // 体检按钮
    DSuggestButton *m_examBtn;
};
