// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/interface/frameproxyinterface.h"
#include "src/window/modules/common/common.h"
#include "src/window/modules/common/compixmap.h"
#include "dtkwidget_global.h"
#include "homepagemodel.h"

#include <DPushButton>
#include <DGuiApplicationHelper>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DCommandLinkButton;
class DSpinner;
class DSuggestButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QGraphicsDropShadowEffect;
class QPropertyAnimation;
QT_END_NAMESPACE

DGUI_USE_NAMESPACE

class ScoreProgressBar;
class CheckItem;

namespace Homepage {
// 体检项行号
enum CheckItemIndex {
    All = 0,
    AutoStartup,
    SSH,
    SysVer,
    Cleaner,
    DiskCheck,
    DevModel
};
// 体检项状态
enum CheckStatus {
    Checking = 0, // 正在扫描
    WillCheck, // 等待扫描
    Abnormal, // 扫描结果有问题
    Normal, // 扫描结果正常
    Fixing, // 正在修复
    Fixed, // 已修复
    Ignored // 已忽略
};

}; // namespace Homepage

using namespace Homepage;
DWIDGET_USE_NAMESPACE

class HomePageCheckWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentScore READ currentScore WRITE setCurrentScore USER true)
public:
    explicit HomePageCheckWidget(HomePageModel *model, QWidget *parent = nullptr);
    ~HomePageCheckWidget();

    // 设置当前显示分数
    void setCurrentScore(int value);
    // 获取当前显示分数
    inline int currentScore() const { return m_currentScore; }
    // 设置图片
    void setPixmapByTheme();

private Q_SLOTS:
    // 当自启动检测结束时
    void onCheckAutoStartFinished(int count);
    // 当自启动应用个数改变时
    void onAutoStartAppCountChanged(int count);
    // 忽略自启动项检测
    void ignoreAutoStartChecking();
    // 取消忽略自启动项检测
    void cancelIgnoreAutoStartChecking();

    // 当ssh状态改变时
    void recSSHStatus(bool opened);

    // 当检查系统更新完成时
    void OnCheckSysUpdateFinished(bool canUpdate);
    // 达到系统版本最大检测时间
    void onSysVerCheckTimeout();
    // 垃圾文件扫描完成时
    void onTrashScanFinished(const double sum);
    // 清理选中的垃圾文件完成时
    void onCleanSelectTrashFinished();

    // 忽略开发者模式检测
    void ignoreDevModeChecking();

protected:
    //  重新改变尺寸函数
    void resizeEvent(QResizeEvent *event);

private:
    // 初始化界面
    void initUI();
    // 初始化所有体检项标题
    void initCheckItemPicAndName();
    // 重置所有体检项参数
    void resetAllCheckItems();
    // 开始体检
    void startExamining();
    // 设置体检项中间提示信息
    void setCheckItemTipByStatus(CheckItemIndex itemIndex, CheckStatus status);
    // 设置体检项右侧按钮内容
    void setCheckItemBtnByStatus(CheckItemIndex itemIndex, CheckStatus status);
    // 设置体检项右侧旋转控件
    void setCheckItemSpinnerByStatus(CheckItemIndex itemIndex, CheckStatus status);
    // 设置得分
    void setScoreValue(int value);
    // 修复ssh
    void fixSSH();
    // 清理选中的垃圾文件
    void cleanSelectTrash();
    // 磁盘检测
    void checkDisk();
    // 打开磁盘管理器
    void openDiskManager();
    // 检测开发者模式
    void checkRootModel();
    // 取消体检
    void cancelChecking();
    // 一键修复
    void fixAll();
    // 更新底部各操作按钮显示状态
    void updateBottomBtns();

private:
    DGuiApplicationHelper *m_guiHelper; // 方便得到系统主题
    // 首页体检数据处理对象
    HomePageModel *m_model;
    // 分数标签
    DLabel *m_lbScore;
    // 设置的分数
    int m_scoreValue;
    // 当前显示的分数
    int m_currentScore;
    // 分数动画
    QPropertyAnimation *m_scoreValueAni;
    // 进度条背景
    ScoreProgressBar *m_scoreBg;
    // 问题项个数提示标签
    DLabel *m_lbResultTip;
    // 问题项个数
    int m_problemsCount;
    // 问题描述
    QString m_sProblemsInfo;

    // 分数进度条
    ScoreProgressBar *m_scoreProg;
    // 分数进度条阴影
    QGraphicsDropShadowEffect *m_progressBarShadow;

    // 启动项 - 标题图片
    DLabel *m_lbAutoStartPic;
    // 启动项 - 标题
    DLabel *m_lbAutoStartTitle;
    // 启动项 - 状态图片
    DLabel *m_lbAutoStartStatusPic;
    // 启动项 - 状态
    DLabel *m_lbAutoStartStatus;
    // 启动项 - 按钮
    DCommandLinkButton *m_btnAutoStart;
    // 启动项 - 忽略按钮
    DCommandLinkButton *m_btnAutoStartIgnored;
    // 启动项 - 取消忽略按钮
    DCommandLinkButton *m_btnAutoStartIgnoredCancel;
    // 启动项 - 旋转控件
    DSpinner *m_spinnerAutoStart;
    // 启动项 - 体检状态
    CheckStatus m_statusAutoStart;
    // 启动项 - 暂存的体检状态
    CheckStatus m_statusTmpAutoStart;
    // 启动项 - 自启动应用个数
    int m_nAutoStartAppCount;
    // 启动项 - 体检项样式控件
    CheckItem *m_itemAutoStart;

    // SSH - 标题图片
    DLabel *m_lbSSHPic;
    // SSH - 标题
    DLabel *m_lbSSHTitle;
    // SSH - 状态图片
    DLabel *m_lbSSHStatusPic;
    // SSH - 状态文字
    DLabel *m_lbSSHStatus;
    DCommandLinkButton *m_btnSSH;
    // SSH - 旋转控件
    DSpinner *m_spinnerSSH;
    // SSH - 体检状态
    CheckStatus m_statusSSH;
    // SSH - 体检项样式控件
    CheckItem *m_itemSSH;

    // 系统版本 - 标题图片
    DLabel *m_lbSystemVersionPic;
    // 系统版本 - 标题
    DLabel *m_lbSystemVersionTitle;
    // 系统版本 - 状态图片
    DLabel *m_lbSystemVersionStatusPic;
    // 系统版本 - 状态文字
    DLabel *m_lbSystemVersionStatus;
    // 系统版本 - 按钮
    DCommandLinkButton *m_btnSystemVersion;
    // 系统版本 - 旋转控件
    DSpinner *m_spinnerSystemVersion;
    // 系统版本 - 体检状态
    CheckStatus m_statusSystemVersion;
    // 系统版本 - 体检项样式控件
    CheckItem *m_itemSystemVersion;
    // 系统版本检测最大等待时间定时器;
    QTimer *m_sysVerCheckTimeoutTimer;

    // 垃圾清理 - 标题图片
    DLabel *m_lbCleanerPic;
    // 垃圾清理 - 标题
    DLabel *m_lbCleanerTitle;
    // 垃圾清理 - 状态图片
    DLabel *m_lbCleanerStatusPic;
    // 垃圾清理 - 状态文字
    DLabel *m_lbCleanerStatus;
    // 垃圾清理 - 按钮
    DCommandLinkButton *m_btnCleaner;
    // 垃圾清理 - 旋转控件
    DSpinner *m_spinnerCleaner;
    // 垃圾清理 - 体检状态
    CheckStatus m_statusCleaner;
    // 垃圾清理 - 体检项样式控件
    CheckItem *m_itemCleaner;

    // 磁盘检测 - 标题图片
    DLabel *m_lbDiskCheckPic;
    // 磁盘检测 - 标题
    DLabel *m_lbDiskCheckTitle;
    // 磁盘检测 - 状态图片
    DLabel *m_lbDiskCheckStatusPic;
    // 磁盘检测 - 状态文字
    DLabel *m_lbDiskCheckStatus;
    // 磁盘检测 - 按钮
    DCommandLinkButton *m_btnDiskCheck;
    // 磁盘检测 - 旋转控件
    DSpinner *m_spinnerDiskCheck;
    // 磁盘检测 - 体检状态
    CheckStatus m_statusDiskCheck;
    // 磁盘检测 - 体检项样式控件
    CheckItem *m_itemDiskCheck;

    // 开发者模式 - 标题图片
    DLabel *m_lbRootModelPic;
    // 开发者模式 - 标题
    DLabel *m_lbRootModelTitle;
    // 开发者模式 - 状态图片
    DLabel *m_lbRootModelStatusPic;
    // 开发者模式 - 状态文字
    DLabel *m_lbRootModelStatus;
    // 开发者模式 - 忽略按钮
    DCommandLinkButton *m_btnRootModelIgnored;
    // 开发者模式 - 体检状态
    CheckStatus m_statusRootModel;
    // 开发者模式 - 体检项样式控件
    CheckItem *m_itemRootModel;

    // 体检取消按钮
    DPushButton *m_btnCheckCancel;
    // 一键修复按钮
    DSuggestButton *m_btnFinish;
    // 返回按钮
    DPushButton *m_btnGoBack;
    // 重新体检按钮
    DPushButton *m_btnRecheck;
    // 一键修复按钮
    DSuggestButton *m_btnRepair;

    // 重新体检标志位
    bool m_isbStartExam;
};
