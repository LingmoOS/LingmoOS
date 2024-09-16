// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "dtkwidget_global.h"
#include "window/interface/frameproxyinterface.h"
#include "src/window/modules/common/common.h"
#include "src/window/modules/common/compixmap.h"
#include "homepagemodel.h"

#include <DSuggestButton>
#include <DTipLabel>

#include <QWidget>
#include <QPushButton>
#include <DGuiApplicationHelper>

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
#define PERFECTLY_SAFE_SCORE_ABOVE 99
// 比较安全分数界限
#define SAFE_SCORE_ABOVE 84
// 比较危险分数界限
#define DANGEROUS_SCORE_ABOVE 59

//// 分数安全级别文字颜色
// 非常安全分数颜色
#define PERFECTLY_SAFE_SCORE_COLOR QColor(0x37, 0xD8, 0x00, 0xFF)
// 非常安全分数 HTML格式颜色
#define PERFECTLY_SAFE_SCORE_COLOR_HTML "#37D800"

// 比较安全分数 颜色
#define SAFE_SCORE_COLOR QColor(0x9E, 0xCA, 0x00, 0xFF)
// 比较安全分数 HTML格式颜色
#define SAFE_SCORE_COLOR_HTML "#9ECA00"

// 比较危险分数 颜色
#define DANGEROUS_SCORE_COLOR QColor(0xFF, 0x5D, 0x00, 0xFF)
// 比较危险分数 HTML格式颜色
#define DANGEROUS_SCORE_COLOR_HTML "#FF5D00"

// 非常危险分数 颜色
#define EXTREMELY_DANGEROUS_SCORE_COLOR QColor(0xE0, 0x20, 0x20, 0xFF)
// 非常危险分数 HTML格式颜色
#define EXTREMELY_DANGEROUS_SCORE_COLOR_HTML "#E02020"

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
    // 体检提示
    DTipLabel *m_examTip;
    // 体检按钮
    DSuggestButton *m_examBtn;
};
