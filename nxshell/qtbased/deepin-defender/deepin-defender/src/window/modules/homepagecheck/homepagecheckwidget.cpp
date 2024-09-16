// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagecheckwidget.h"
#include "homepage.h"
#include "widgets/scoreprogressbar.h"
#include "checkitem.h"
#include "src/window/dialogmanage.h"

#include <DLabel>
#include <DCommandLinkButton>
#include <DSuggestButton>
#include <DSpinner>
#include <DFontSizeManager>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>

// 进度条背景色
const QColor ScoreBackgroundLight = QColor(0, 0, 0, 25);
const QColor ScoreBackgroundDark = QColor(255, 255, 255, 25);

// 体检项提示文字颜色
#define CHECK_ITEM_TIP_NORMAL_COLOR_HTML "#6DD400"
#define CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML "#E02020"

// 系统版本检测最大时间/s
#define SYS_VER_CHECK_TIMEOUT 40

HomePageCheckWidget::HomePageCheckWidget(HomePageModel *model, QWidget *parent)
    : QWidget(parent)
    , m_guiHelper(nullptr)
    , m_model(model)
    , m_lbScore(nullptr)
    , m_scoreValue(100)
    , m_currentScore(100)
    , m_scoreValueAni(nullptr)
    , m_scoreBg(nullptr)
    , m_lbResultTip(nullptr)
    , m_problemsCount(0)
    , m_scoreProg(nullptr)
    , m_progressBarShadow(nullptr)
    // 自启动控件
    , m_lbAutoStartPic(nullptr)
    , m_lbAutoStartTitle(nullptr)
    , m_lbAutoStartStatusPic(nullptr)
    , m_lbAutoStartStatus(nullptr)
    , m_btnAutoStart(nullptr)
    , m_btnAutoStartIgnored(nullptr)
    , m_btnAutoStartIgnoredCancel(nullptr)
    , m_spinnerAutoStart(nullptr)
    , m_statusAutoStart(CheckStatus::Normal)
    , m_statusTmpAutoStart(CheckStatus::Normal)
    , m_nAutoStartAppCount(0)
    , m_itemAutoStart(nullptr)
    // ssh控件
    , m_lbSSHPic(nullptr)
    , m_lbSSHTitle(nullptr)
    , m_lbSSHStatusPic(nullptr)
    , m_lbSSHStatus(nullptr)
    , m_btnSSH(nullptr)
    , m_spinnerSSH(nullptr)
    , m_statusSSH(CheckStatus::Normal)
    , m_itemSSH(nullptr)
    // 系统版本控件
    , m_lbSystemVersionPic(nullptr)
    , m_lbSystemVersionTitle(nullptr)
    , m_lbSystemVersionStatusPic(nullptr)
    , m_lbSystemVersionStatus(nullptr)
    , m_btnSystemVersion(nullptr)
    , m_spinnerSystemVersion(nullptr)
    , m_statusSystemVersion(CheckStatus::Normal)
    , m_itemSystemVersion(nullptr)
    , m_sysVerCheckTimeoutTimer(nullptr)
    // 垃圾清理控件
    , m_lbCleanerPic(nullptr)
    , m_lbCleanerTitle(nullptr)
    , m_lbCleanerStatusPic(nullptr)
    , m_lbCleanerStatus(nullptr)
    , m_btnCleaner(nullptr)
    , m_spinnerCleaner(nullptr)
    , m_statusCleaner(CheckStatus::Normal)
    , m_itemCleaner(nullptr)
    // 磁盘检测控件
    , m_lbDiskCheckPic(nullptr)
    , m_lbDiskCheckTitle(nullptr)
    , m_lbDiskCheckStatusPic(nullptr)
    , m_lbDiskCheckStatus(nullptr)
    , m_btnDiskCheck(nullptr)
    , m_spinnerDiskCheck(nullptr)
    , m_statusDiskCheck(CheckStatus::Normal)
    , m_itemDiskCheck(nullptr)
    // 开发者模式控件
    , m_lbRootModelPic(nullptr)
    , m_lbRootModelTitle(nullptr)
    , m_lbRootModelStatusPic(nullptr)
    , m_lbRootModelStatus(nullptr)
    , m_btnRootModelIgnored(nullptr)
    , m_statusRootModel(CheckStatus::Normal)
    , m_itemRootModel(nullptr)
    // 底部按钮控件
    , m_btnCheckCancel(nullptr)
    , m_btnFinish(nullptr)
    , m_btnRepair(nullptr)
    , m_isbStartExam(false)
{
    this->setAccessibleName("rightWidget_homeCheckWidget");
    //主题模式
    m_guiHelper = DGuiApplicationHelper::instance();
    // 主题变换 改变部分图标颜色
    connect(m_guiHelper, &DGuiApplicationHelper::themeTypeChanged, this, &HomePageCheckWidget::setPixmapByTheme);
    // 初始化界面
    initUI();

    //// 自启动信号初始化
    // 自启动检测完成
    connect(m_model, &HomePageModel::checkAutoStartFinished, this, &HomePageCheckWidget::onCheckAutoStartFinished);
    // 自启动应用个数改变
    connect(m_model, &HomePageModel::autoStartAppCountChanged, this, &HomePageCheckWidget::onAutoStartAppCountChanged);
    // 连接 到 打开自启动管理界面
    connect(m_btnAutoStart, &DPushButton::clicked, m_model, &HomePageModel::openStartupControlPage);
    // 忽略自启动项检测
    connect(m_btnAutoStartIgnored, &DPushButton::clicked, this, &HomePageCheckWidget::ignoreAutoStartChecking);
    // 取消忽略自启动项检测
    connect(m_btnAutoStartIgnoredCancel, &DPushButton::clicked, this, &HomePageCheckWidget::cancelIgnoreAutoStartChecking);

    //// ssh信号初始化
    // 修复ssh
    connect(m_btnSSH, &DPushButton::clicked, this, &HomePageCheckWidget::fixSSH);
    connect(m_model, &HomePageModel::sendSSHStatus, this, &HomePageCheckWidget::recSSHStatus);

    //// 系统更新信号初始化
    // 检查系统更新完成
    connect(m_model, &HomePageModel::CheckSysUpdateFinished, this, &HomePageCheckWidget::OnCheckSysUpdateFinished);
    // 连接 到 打开控制系统更新界面
    connect(m_btnSystemVersion, &DPushButton::clicked, m_model, &HomePageModel::openSysUpdatePage);
    // 达到系统版本最大检测时间
    connect(m_sysVerCheckTimeoutTimer, &QTimer::timeout, this, &HomePageCheckWidget::onSysVerCheckTimeout);

    //// 垃圾清理信号初始化
    // 垃圾文件扫描完成时
    connect(m_model, &HomePageModel::trashScanFinished, this, &HomePageCheckWidget::onTrashScanFinished);
    // 清理选中的垃圾文件完成时
    connect(m_model, &HomePageModel::cleanSelectTrashFinished, this, &HomePageCheckWidget::onCleanSelectTrashFinished);
    // 清理选中的垃圾文件
    connect(m_btnCleaner, &DPushButton::clicked, this, &HomePageCheckWidget::cleanSelectTrash);

    //// 磁盘管理信号初始化
    // 连接 到 打开磁盘管理器
    connect(m_btnDiskCheck, &DPushButton::clicked, this, &HomePageCheckWidget::openDiskManager);

    //// 开发者模式信号初始化
    // 忽略开发者模式检测
    connect(m_btnRootModelIgnored, &DPushButton::clicked, this, &HomePageCheckWidget::ignoreDevModeChecking);

    //// 底部按钮信号初始化
    // 连接 取消体检按钮信号
    connect(m_btnCheckCancel, &DPushButton::clicked, this, &HomePageCheckWidget::cancelChecking);
    // 连接 返回按钮信号
    connect(m_btnGoBack, &DPushButton::clicked, this, &HomePageCheckWidget::cancelChecking);
    // 连接 完成按钮信号
    connect(m_btnFinish, &DSuggestButton::clicked, this, &HomePageCheckWidget::cancelChecking);
    // 连接 重新体检按钮信号
    connect(m_btnRecheck, &DPushButton::clicked, this, &HomePageCheckWidget::startExamining);
    // 连接 一键修复按钮信号
    connect(m_btnRepair, &DSuggestButton::clicked, this, &HomePageCheckWidget::fixAll);

    //// 开始体检
    startExamining();
}

HomePageCheckWidget::~HomePageCheckWidget()
{
}

// 设置当前显示分数
void HomePageCheckWidget::setCurrentScore(int value)
{
    m_currentScore = value;
    m_lbScore->setNum(m_currentScore);
}

// 当自启动检测结束时
void HomePageCheckWidget::onCheckAutoStartFinished(int count)
{
    // 设置自启动应用个数
    m_nAutoStartAppCount = count;

    // 暂存自启动项实际的检查状态
    if (0 < m_nAutoStartAppCount) {
        m_statusTmpAutoStart = CheckStatus::Abnormal;
    } else {
        m_statusTmpAutoStart = CheckStatus::Normal;
    }

    // 若当前不处于检查自启动状态，则不处理
    if (!m_model->autoStartAppChecking()) {
        return;
    }

    // 设置是否处于自启动检查状态
    m_model->SetAutoStartAppChecking(false);

    // 当前状态处于检测中，才进行处理
    if (CheckStatus::Checking != m_statusAutoStart) {
        return;
    }

    if (0 < m_nAutoStartAppCount) {
        // 减分
        m_scoreValue -= SCORE_AUTOSTART_ABNORMAL;
        m_problemsCount++;
        if (!m_sProblemsInfo.isEmpty()) {
            m_sProblemsInfo += ", ";
        }
        m_sProblemsInfo += tr("Startup programs");

        // 设置分数
        setScoreValue(m_scoreValue);
        // 设置 自启动 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
        // 设置 自启动 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
        // 设置 自启动 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
    } else {
        // 设置 自启动 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Normal);
        // 设置 自启动 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Normal);
    }
    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

// 当自启动应用个数改变时
void HomePageCheckWidget::onAutoStartAppCountChanged(int count)
{
    // 设置自启动应用个数
    m_nAutoStartAppCount = count;

    // 暂存自启动项实际的检查状态
    if (0 < m_nAutoStartAppCount) {
        m_statusTmpAutoStart = CheckStatus::Abnormal;
    } else {
        m_statusTmpAutoStart = CheckStatus::Fixed;
    }

    // 若自启动项 正在检测 或 无异常 或 忽略 时，不改变启动体检项的显示信息
    if (CheckStatus::Checking == m_statusAutoStart || CheckStatus::Normal == m_statusAutoStart
        || CheckStatus::Ignored == m_statusAutoStart || CheckStatus::Fixed == m_statusAutoStart) {
        return;
    }

    if (0 < count) {
        // 设置 自启动 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
        // 设置 自启动 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
        // 设置 自启动 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Abnormal);
    } else {
        // 设置 启动项 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Fixed);
        // 设置 启动项 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Fixed);
        // 设置 启动项 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Fixed);
    }

    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

// 忽略自启动项检测
void HomePageCheckWidget::ignoreAutoStartChecking()
{
    m_model->setIgnoreAutoStartChecking(true);
    setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);
    setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);
    setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);

    m_model->addSecurityLog(tr("Startup programs checking dismissed"));
}

// 取消忽略自启动项检测
void HomePageCheckWidget::cancelIgnoreAutoStartChecking()
{
    m_model->setIgnoreAutoStartChecking(false);
    setCheckItemTipByStatus(CheckItemIndex::AutoStartup, m_statusTmpAutoStart);
    setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, m_statusTmpAutoStart);
    setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, m_statusTmpAutoStart);

    m_model->addSecurityLog(tr("Startup programs checking resumed"));
}

void HomePageCheckWidget::recSSHStatus(bool opened)
{
    // 如果检测完成
    if (CheckStatus::Checking == m_statusSSH) {
        // 根据状态改变显示信息
        // 设置 ssh 体检项中间提示信息
        CheckStatus settingStatus;
        if (opened) {
            // 扣分
            m_scoreValue -= SCORE_SSH_ABNORMAL; // 减分
            // 问题数增加
            m_problemsCount++;
            if (!m_sProblemsInfo.isEmpty()) {
                m_sProblemsInfo += ", ";
            }
            m_sProblemsInfo += tr("RDP port");

            setScoreValue(m_scoreValue);
            // 异常状态
            settingStatus = CheckStatus::Abnormal;
        } else {
            // 正常状态
            settingStatus = CheckStatus::Normal;
        }

        setCheckItemTipByStatus(CheckItemIndex::SSH, settingStatus);
        // 设置 ssh 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::SSH, settingStatus);
        // 设置 ssh 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::SSH, settingStatus);
        // 更新底部各操作按钮显示状态
        updateBottomBtns();
        return;
    }

    // 接收到修复状态后, 设置界面
    // 如果当前状态处于 检测中 或 无异常 或 已修复，则不进行处理
    if (CheckStatus::Checking == m_statusSSH
        || CheckStatus::Normal == m_statusSSH
        || CheckStatus::Fixed == m_statusSSH) {
        return;
    }

    // 当开启ssh 信号发送过来时，不处理该信号
    if (opened) {
        return;
    }

    // 根据状态改变显示信息
    // 设置 ssh 体检项中间提示信息
    CheckStatus settingStatus = CheckStatus::Fixed;
    setCheckItemTipByStatus(CheckItemIndex::SSH, settingStatus);
    // 设置 ssh 体检项右侧按钮内容
    setCheckItemBtnByStatus(CheckItemIndex::SSH, settingStatus);
    // 设置 ssh 体检项右侧旋转控件
    setCheckItemSpinnerByStatus(CheckItemIndex::SSH, settingStatus);
    // 添加日志
    m_model->addSecurityLog(tr("RDP port closed"));

    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

void HomePageCheckWidget::OnCheckSysUpdateFinished(bool canUpdate)
{
    // 设置正在系统版本检测标志位
    m_model->SetSysVersionChecking(false);

    // 当前状态处于检测中，才进行处理
    if (CheckStatus::Checking != m_statusSystemVersion) {
        return;
    }

    if (canUpdate) {
        m_scoreValue -= SCORE_SYS_VER_ABNORMAL; // 减分
        m_problemsCount++;
        if (!m_sProblemsInfo.isEmpty()) {
            m_sProblemsInfo += ", ";
        }
        m_sProblemsInfo += tr("System version");

        setScoreValue(m_scoreValue);
        // 设置 系统更新 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::SysVer, CheckStatus::Abnormal);
        // 设置 系统更新 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::SysVer, CheckStatus::Abnormal);
        // 设置 系统更新 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::SysVer, CheckStatus::Abnormal);
    } else {
        // 设置 系统更新 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::SysVer, CheckStatus::Normal);
        // 设置 系统更新 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::SysVer, CheckStatus::Normal);
    }
    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

void HomePageCheckWidget::onSysVerCheckTimeout()
{
    if (m_model->sysVersionChecking()) {
        Q_EMIT m_model->notifyStopCheckSysUpdate();
        m_model->CheckSysUpdateFinished(false);
        m_model->SetSysVersionChecking(false);
    }
}

// 垃圾文件扫描完成时
void HomePageCheckWidget::onTrashScanFinished(const double sum)
{
    if (100 * MB_COUNT <= sum) {
        // 减分
        if (1 * GB_COUNT <= sum) {
            m_scoreValue -= SCORE_CLEANER_ABNORMAL_SEC_STAGE;
        } else {
            m_scoreValue -= SCORE_CLEANER_ABNORMAL_FIRST_STAGE;
        }
        m_problemsCount++;
        if (!m_sProblemsInfo.isEmpty()) {
            m_sProblemsInfo += ", ";
        }
        m_sProblemsInfo += tr("System cleanup");

        setScoreValue(m_scoreValue);
        // 设置 垃圾清理 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::Cleaner, CheckStatus::Abnormal);
        // 设置 垃圾清理 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::Cleaner, CheckStatus::Abnormal);
        // 设置 垃圾清理 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::Cleaner, CheckStatus::Abnormal);
    } else {
        // 设置 垃圾清理 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::Cleaner, CheckStatus::Normal);
        // 设置 垃圾清理 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::Cleaner, CheckStatus::Fixed);
    }
    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

// 清理选中的垃圾文件完成时
void HomePageCheckWidget::onCleanSelectTrashFinished()
{
    // 设置 垃圾清理 体检项中间提示信息
    setCheckItemTipByStatus(CheckItemIndex::Cleaner, CheckStatus::Normal);
    // 设置 垃圾清理 体检项右侧按钮内容
    setCheckItemBtnByStatus(CheckItemIndex::Cleaner, CheckStatus::Fixed);
    // 设置 垃圾清理 体检项右侧旋转控件
    setCheckItemSpinnerByStatus(CheckItemIndex::Cleaner, CheckStatus::Fixed);

    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

// 忽略开发者模式检测
void HomePageCheckWidget::ignoreDevModeChecking()
{
    m_model->setIgnoreDevModeChecking(true);
    setCheckItemTipByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);
    setCheckItemBtnByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);
    setCheckItemSpinnerByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);

    m_model->addSecurityLog(tr("Developer mode checking dismissed"));
}

void HomePageCheckWidget::setPixmapByTheme()
{
    initCheckItemPicAndName();
    // 判断当前主题类型
    if (m_guiHelper->themeType() == DGuiApplicationHelper::ColorType::LightType) {
        // 进度条 背景色
        m_scoreBg->setColor(ScoreBackgroundLight);
    } else {
        // 进度条 背景色
        m_scoreBg->setColor(ScoreBackgroundDark);
    }
}

//  重新改变尺寸函数
void HomePageCheckWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // 固定分数标签宽度为"100"时的宽度
    int width = m_lbScore->fontMetrics().width("100");
    m_lbScore->setFixedWidth(width);
}

// 初始化界面
void HomePageCheckWidget::initUI()
{
    /******************体检界面******************/
    // 新建一个垂直布局 安置所有组件
    QVBoxLayout *layoutCheck = new QVBoxLayout(this);
    layoutCheck->setObjectName(QStringLiteral("layoutCheckTop"));
    layoutCheck->setContentsMargins(45, 0, 45, 15);
    layoutCheck->setSpacing(0);
    // 新建一个水平布局安置上方分数以及结果组件
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    // 分数标签
    m_lbScore = new DLabel(this);
    m_lbScore->setAccessibleName("homeCheckWidget_scoreLabel");
    m_lbScore->setObjectName(QStringLiteral("lbScore"));
    m_lbScore->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_lbScore->setContentsMargins(0, 0, 0, 0);
    m_lbScore->setMargin(0);

    // 分数字体
    QFont font1 = m_lbScore->font();
    font1.setPixelSize(60);
    font1.setWeight(QFont::Weight::Medium);
    m_lbScore->setFont(font1);
    // 分数颜色
    QPalette paTmp = m_lbScore->palette();
    paTmp.setColor(QPalette::Text, PERFECTLY_SAFE_SCORE_COLOR);
    m_lbScore->setPalette(paTmp);
    // 设置初始值为100
    m_lbScore->setNum(100);
    // 将分数标签添加到布局中
    horizontalLayout->addWidget(m_lbScore);
    // 设置的分数
    m_scoreValue = 100;
    // 当前显示分数
    m_currentScore = 100;
    // 分数动画
    m_scoreValueAni = new QPropertyAnimation(this, "currentScore", this);

    // "分"文字标签
    DLabel *pointLabel = new DLabel(tr("points"), this);
    pointLabel->setAccessibleName("homeCheckWidget_pointLabel");
    pointLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    // 如果语言不是中文就隐藏
    if (!m_model->getLocalLang().startsWith("zh")) {
        pointLabel->setVisible(false);
    }
    // 设置边缘间距
    pointLabel->setContentsMargins(0, 0, 0, 15);
    // 设置字体
    DFontSizeManager::instance()->bind(pointLabel, DFontSizeManager::T5);
    // 添加到布局中
    horizontalLayout->addWidget(pointLabel, 0, Qt::AlignLeft | Qt::AlignBottom);

    // 分数标签右侧布局
    horizontalLayout->addSpacing(30);
    QVBoxLayout *scoreRightLayout = new QVBoxLayout();
    scoreRightLayout->setContentsMargins(0, 0, 0, 0);
    // 设置对齐方式
    scoreRightLayout->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    horizontalLayout->addLayout(scoreRightLayout);
    scoreRightLayout->addStretch(0);

    // 问题项个数提示标签
    m_lbResultTip = new DLabel(this);
    m_lbResultTip->setAccessibleName("homeCheckWidget_issueCountLabel");
    // 设置字体
    QFont ft = m_lbResultTip->font();
    ft.setWeight(QFont::Weight::DemiBold);
    m_lbResultTip->setFont(ft);
    DFontSizeManager::instance()->bind(m_lbResultTip, DFontSizeManager::T5);
    scoreRightLayout->addWidget(m_lbResultTip, 0, Qt::AlignLeft);
    scoreRightLayout->setContentsMargins(0, 0, 0, 20);
    scoreRightLayout->addSpacing(5);
    // 分数进度条
    // 采用栈布局,上层放分数进度条,下层放背景进度条
    QStackedLayout *scoreProgressLayout = new QStackedLayout();
    scoreProgressLayout->setStackingMode(QStackedLayout::StackAll);
    scoreProgressLayout->setContentsMargins(0, 0, 0, 0);
    scoreProgressLayout->setMargin(0);
    scoreProgressLayout->setSpacing(0);
    scoreRightLayout->addLayout(scoreProgressLayout);
    // 进度条尺寸
    QSize scoreBarSize = QSize(220, 12);
    // 分数进度条背景
    m_scoreBg = new ScoreProgressBar(this);
    m_scoreBg->setFixedSize(scoreBarSize);
    // 设置背景为满进度
    m_scoreBg->setCurrentValue(100);
    // 加入到进度条栈布局中
    scoreProgressLayout->addWidget(m_scoreBg);
    // 背景下沉
    m_scoreBg->lower();

    // 进度条
    m_scoreProg = new ScoreProgressBar(this);
    m_scoreProg->setFixedSize(scoreBarSize);
    m_scoreProg->setCurrentValue(100);
    m_scoreProg->setValue(100);
    // 设置初始颜色
    m_scoreProg->setColor(PERFECTLY_SAFE_SCORE_COLOR);
    scoreProgressLayout->addWidget(m_scoreProg);
    // 显示在上层
    m_scoreProg->raise();

    // 进度条阴影
    m_progressBarShadow = new QGraphicsDropShadowEffect(this);
    // 设置偏移
    m_progressBarShadow->setOffset(0, 3);
    // 设置模糊程度
    m_progressBarShadow->setBlurRadius(20);
    // 设置阴影颜色
    m_progressBarShadow->setColor(PERFECTLY_SAFE_SCORE_SHADOW_COLOR);
    // 将阴影设置给进度条控件
    m_scoreProg->setGraphicsEffect(m_progressBarShadow);
    // 将分数结果布局加入整体布局
    layoutCheck->addLayout(horizontalLayout);

    // 分割线
    QFrame *line = new QFrame(this);
    line->setObjectName(QStringLiteral("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    // 将分割线加入整体布局
    layoutCheck->addWidget(line);

    // 新建可滑动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->viewport()->setAccessibleName("scrollArea_viewPort");
    scrollArea->setAccessibleName("homeCheckWidget_scrollArea");
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    // 将柵格布局加入到主布局
    layoutCheck->addWidget(scrollArea, 1);

    // 新建水平布局 安置所有体检项 组件
    QVBoxLayout *checkItemsLayout = new QVBoxLayout;
    checkItemsLayout->setContentsMargins(0, 10, 0, 15);
    checkItemsLayout->setSpacing(10);

    QWidget *checkItemsContentWidget = new QWidget(scrollArea);
    checkItemsContentWidget->setAccessibleName("viewPort_contentWidget");
    checkItemsContentWidget->setLayout(checkItemsLayout);
    scrollArea->setWidget(checkItemsContentWidget);

    /**启动项**/
    // 启动项 - 图片
    m_lbAutoStartPic = new DLabel(this);
    m_lbAutoStartPic->setObjectName(QStringLiteral("lbAustartPic"));
    m_lbAutoStartPic->setFixedSize(QSize(30, 30));

    // 启动项 - 标签
    m_lbAutoStartTitle = new DLabel(this);
    m_lbAutoStartTitle->setAccessibleName("autoStartFrame_startTitleLable");
    m_lbAutoStartTitle->setObjectName(QStringLiteral("lbAnstart"));

    // 启动项 - 状态 - 图片
    m_lbAutoStartStatusPic = new DLabel(this);
    m_lbAutoStartStatusPic->setObjectName(QStringLiteral("lbAutoStartStatusPic"));
    m_lbAutoStartStatusPic->setFixedSize(QSize(20, 20));

    // 启动项 - 状态 - 文字
    m_lbAutoStartStatus = new DLabel(this);
    m_lbAutoStartStatus->setAccessibleName("autoStartFrame_startStatusLable");
    m_lbAutoStartStatus->setObjectName(QStringLiteral("lbStartShow"));
    m_lbAutoStartStatus->setWordWrap(true);
    m_lbAutoStartStatus->setFont(m_lbAutoStartStatus->font());
    DFontSizeManager::instance()->bind(m_lbAutoStartStatus, DFontSizeManager::T8);

    // 启动项 - 按钮
    m_btnAutoStart = new DCommandLinkButton("", this);
    m_btnAutoStart->setAccessibleName("autoStartFrame_startButton");
    m_btnAutoStart->setObjectName(QStringLiteral("btnAustart"));

    // 启动项 - 忽略按钮
    m_btnAutoStartIgnored = new DCommandLinkButton("", this);
    m_btnAutoStartIgnored->setAccessibleName("autoStartFrame_ignoredButton");
    m_btnAutoStartIgnored->setObjectName(QStringLiteral("btnAutoStartIgnored"));

    // 启动项 - 取消忽略按钮
    m_btnAutoStartIgnoredCancel = new DCommandLinkButton("", this);
    m_btnAutoStartIgnoredCancel->setAccessibleName("autoStartFrame_ignoreCancelButton");
    m_btnAutoStartIgnoredCancel->setObjectName(QStringLiteral("btnAutoStartIgnoredCancel"));

    // 启动项 - 转圈控件
    m_spinnerAutoStart = new DSpinner(this);
    m_spinnerAutoStart->setObjectName(QStringLiteral("spinnerAutoStart"));
    m_spinnerAutoStart->setFixedSize(20, 20);
    m_spinnerAutoStart->hide();

    // 创建体检项样式控件
    m_itemAutoStart = new CheckItem(m_lbAutoStartPic, m_lbAutoStartTitle,
                                    m_lbAutoStartStatusPic, m_lbAutoStartStatus,
                                    m_btnAutoStart, m_btnAutoStartIgnored,
                                    m_btnAutoStartIgnoredCancel, m_spinnerAutoStart, this);
    m_itemAutoStart->setAccessibleName("contentWidget_autoStartFrame");
    checkItemsLayout->addWidget(m_itemAutoStart);

    /**ssh**/
    // ssh - 图片
    m_lbSSHPic = new DLabel(this);
    m_lbSSHPic->setObjectName(QStringLiteral("lbSSHPic"));
    m_lbSSHPic->setFixedSize(QSize(30, 30));

    // ssh - 标签
    m_lbSSHTitle = new DLabel(this);
    m_lbSSHTitle->setAccessibleName("sshTitle");
    m_lbSSHTitle->setObjectName(QStringLiteral("sshCheckFrame_sshTitleLable"));

    // ssh - 状态 - 图片
    m_lbSSHStatusPic = new DLabel(this);
    m_lbSSHStatusPic->setObjectName(QStringLiteral("lbSSHStatusPic"));
    m_lbSSHStatusPic->setFixedSize(QSize(20, 20));

    // ssh - 状态 - 文字
    m_lbSSHStatus = new DLabel(this);
    m_lbSSHStatus->setAccessibleName("sshStatus");
    m_lbSSHStatus->setObjectName(QStringLiteral("sshCheckFrame_sshStatusLable"));
    m_lbSSHStatus->setWordWrap(true);
    m_lbSSHStatus->setFont(m_lbAutoStartStatus->font());
    DFontSizeManager::instance()->bind(m_lbSSHStatus, DFontSizeManager::T8);

    // ssh - 按钮
    m_btnSSH = new DCommandLinkButton("", this);
    m_btnSSH->setAccessibleName("sshBtn");
    m_btnSSH->setObjectName(QStringLiteral("sshCheckFrame_sshButton"));

    // ssh - 转圈控件
    m_spinnerSSH = new DSpinner(this);
    m_spinnerSSH->setObjectName(QStringLiteral("spinnerSSH"));
    m_spinnerSSH->setFixedSize(20, 20);
    m_spinnerSSH->hide();

    // 创建体检项样式控件
    m_itemSSH = new CheckItem(m_lbSSHPic, m_lbSSHTitle,
                              m_lbSSHStatusPic, m_lbSSHStatus,
                              m_btnSSH, nullptr,
                              nullptr, m_spinnerSSH, this);
    m_itemSSH->setAccessibleName("contentWidget_sshCheckFrame");
    checkItemsLayout->addWidget(m_itemSSH);

    /**系统版本**/
    // 系统版本 - 图片
    m_lbSystemVersionPic = new DLabel(this);
    m_lbSystemVersionPic->setObjectName(QStringLiteral("lbSystemVersionPic"));
    m_lbSystemVersionPic->setFixedSize(QSize(30, 30));

    // 系统版本 - 标题
    m_lbSystemVersionTitle = new DLabel(this);
    m_lbSystemVersionTitle->setAccessibleName("sysVerFrame_sysVerTitleLable");
    m_lbSystemVersionTitle->setObjectName(QStringLiteral("lbSystemVersion"));

    // 系统版本 - 状态 - 图片
    m_lbSystemVersionStatusPic = new DLabel(this);
    m_lbSystemVersionStatusPic->setObjectName(QStringLiteral("lbSystemVersionStatusPic"));
    m_lbSystemVersionStatusPic->setFixedSize(QSize(20, 20));

    // 系统版本 - 状态 - 文字
    m_lbSystemVersionStatus = new DLabel(this);
    m_lbSystemVersionStatus->setAccessibleName("sysVerFrame_sysVerStatusLable");
    m_lbSystemVersionStatus->setObjectName(QStringLiteral("lbSystemVersionShow"));
    m_lbSystemVersionStatus->setWordWrap(true);
    m_lbSystemVersionStatus->setFont(m_lbSystemVersionStatus->font());
    DFontSizeManager::instance()->bind(m_lbSystemVersionStatus, DFontSizeManager::T8);

    // 系统版本 - 按钮
    m_btnSystemVersion = new DCommandLinkButton("", this);
    m_btnSystemVersion->setAccessibleName("sysVerFrame_sysVerButton");
    m_btnSystemVersion->setObjectName(QStringLiteral("btnSystemVersion"));

    // 系统版本 - 转圈控件
    m_spinnerSystemVersion = new DSpinner(this);
    m_spinnerSystemVersion->setObjectName(QStringLiteral("spinnerSystemVersion"));
    m_spinnerSystemVersion->setFixedSize(20, 20);
    m_spinnerSystemVersion->hide();

    // 创建体检项样式控件
    m_itemSystemVersion = new CheckItem(m_lbSystemVersionPic, m_lbSystemVersionTitle,
                                        m_lbSystemVersionStatusPic, m_lbSystemVersionStatus,
                                        m_btnSystemVersion, nullptr, nullptr, m_spinnerSystemVersion, this);
    m_itemSystemVersion->setAccessibleName("contentWidget_sysVerFrame");
    checkItemsLayout->addWidget(m_itemSystemVersion);

    // 初始化系统版本检测最大等待时间定时器
    m_sysVerCheckTimeoutTimer = new QTimer(this);
    m_sysVerCheckTimeoutTimer->setSingleShot(true);

    /**垃圾清理**/
    // 垃圾清理 - 图片
    m_lbCleanerPic = new DLabel(this);
    m_lbCleanerPic->setObjectName(QStringLiteral("lbCleanerPic"));
    m_lbCleanerPic->setFixedSize(QSize(30, 30));

    // 垃圾清理 - 标签
    m_lbCleanerTitle = new DLabel(this);
    m_lbCleanerTitle->setAccessibleName("cleanFrame_cleanerTitleLable");
    m_lbCleanerTitle->setObjectName(QStringLiteral("lbCleanerTitle"));

    // 垃圾清理 - 状态 - 图片
    m_lbCleanerStatusPic = new DLabel(this);
    m_lbCleanerStatusPic->setObjectName(QStringLiteral("lbCleanerStatusPic"));
    m_lbCleanerStatusPic->setFixedSize(QSize(20, 20));

    // 垃圾清理 - 状态 - 文字
    m_lbCleanerStatus = new DLabel(this);
    m_lbCleanerStatus->setAccessibleName("cleanFrame_cleanerStatusLable");
    m_lbCleanerStatus->setObjectName(QStringLiteral("lbCleanerStatus"));
    m_lbCleanerStatus->setWordWrap(true);
    m_lbCleanerStatus->setFont(m_lbCleanerStatus->font());
    DFontSizeManager::instance()->bind(m_lbCleanerStatus, DFontSizeManager::T8);

    // 垃圾清理 - 按钮
    m_btnCleaner = new DCommandLinkButton("", this);
    m_btnCleaner->setAccessibleName("cleanFrame_cleanerButton");
    m_btnCleaner->setObjectName(QStringLiteral("btnCleaner"));

    // 垃圾清理 - 转圈控件
    m_spinnerCleaner = new DSpinner(this);
    m_spinnerCleaner->setObjectName(QStringLiteral("spinnerCleaner"));
    m_spinnerCleaner->setFixedSize(20, 20);
    m_spinnerCleaner->hide();

    // 创建体检项样式控件
    m_itemCleaner = new CheckItem(m_lbCleanerPic, m_lbCleanerTitle,
                                  m_lbCleanerStatusPic, m_lbCleanerStatus,
                                  m_btnCleaner, nullptr, nullptr, m_spinnerCleaner, this);
    m_itemCleaner->setAccessibleName("contentWidget_cleanFrame");
    checkItemsLayout->addWidget(m_itemCleaner);

    /**磁盘检测**/
    // 磁盘检测 - 图片
    m_lbDiskCheckPic = new DLabel(this);
    m_lbDiskCheckPic->setObjectName(QStringLiteral("lbDiskCheckPic"));
    m_lbDiskCheckPic->setFixedSize(QSize(30, 30));

    // 磁盘检测 - 标签
    m_lbDiskCheckTitle = new DLabel(this);
    m_lbDiskCheckTitle->setAccessibleName("diskCheckFrame_diskCheckTitleLable");
    m_lbDiskCheckTitle->setObjectName(QStringLiteral("lbDiskCheck"));

    // 磁盘检测 - 状态 - 图片
    m_lbDiskCheckStatusPic = new DLabel(this);
    m_lbDiskCheckStatusPic->setObjectName(QStringLiteral("lbDiskCheckStatusPic"));
    m_lbDiskCheckStatusPic->setFixedSize(QSize(20, 20));

    // 磁盘检测 - 状态 - 文字
    m_lbDiskCheckStatus = new DLabel(this);
    m_lbDiskCheckStatus->setAccessibleName("diskCheckFrame_diskCheckStatusLable");
    m_lbDiskCheckStatus->setObjectName(QStringLiteral("lbDiskCheckStatus"));
    m_lbDiskCheckStatus->setWordWrap(true);
    m_lbDiskCheckStatus->setFont(m_lbDiskCheckStatus->font());
    DFontSizeManager::instance()->bind(m_lbDiskCheckStatus, DFontSizeManager::T8);

    // 磁盘检测 - 按钮
    m_btnDiskCheck = new DCommandLinkButton("", this);
    m_btnDiskCheck->setAccessibleName("diskCheckFrame_diskCheckButton");
    m_btnDiskCheck->setObjectName(QStringLiteral("btnDiskCheck"));

    // 垃圾清理 - 转圈控件
    m_spinnerDiskCheck = new DSpinner(this);
    m_spinnerDiskCheck->setObjectName(QStringLiteral("spinnerDiskCheck"));
    m_spinnerDiskCheck->setFixedSize(20, 20);
    m_spinnerDiskCheck->hide();

    // 创建体检项样式控件
    m_itemDiskCheck = new CheckItem(m_lbDiskCheckPic, m_lbDiskCheckTitle,
                                    m_lbDiskCheckStatusPic, m_lbDiskCheckStatus,
                                    m_btnDiskCheck, nullptr, nullptr, m_spinnerDiskCheck, this);
    m_itemDiskCheck->setAccessibleName("contentWidget_diskCheckFrame");
    checkItemsLayout->addWidget(m_itemDiskCheck);

    /**开发者模式**/
    // 开发者模式 - 图片
    m_lbRootModelPic = new DLabel(this);
    m_lbRootModelPic->setObjectName(QStringLiteral("lbRootModelPic"));
    m_lbRootModelPic->setFixedSize(QSize(30, 30));

    // 开发者模式 - 标签
    m_lbRootModelTitle = new DLabel(this);
    m_lbRootModelTitle->setAccessibleName("rootModelFrame_rootModelTitleLable");
    m_lbRootModelTitle->setObjectName(QStringLiteral("lbRootModel"));

    // 开发者模式 - 状态 - 图片
    m_lbRootModelStatusPic = new DLabel(this);
    m_lbRootModelStatusPic->setObjectName(QStringLiteral("lbRootModelStatusPic"));
    m_lbRootModelStatusPic->setFixedSize(QSize(20, 20));

    // 开发者模式 - 状态 - 文字
    m_lbRootModelStatus = new DLabel(this);
    m_lbRootModelStatus->setAccessibleName("rootModelFrame_rootModelStatusLable");
    m_lbRootModelStatus->setObjectName(QStringLiteral("lbRootModelShow"));
    m_lbRootModelStatus->setWordWrap(true);
    m_lbRootModelStatus->setFont(m_lbRootModelStatus->font());
    DFontSizeManager::instance()->bind(m_lbRootModelStatus, DFontSizeManager::T8);

    // 开发者模式 - 取消忽略按钮
    m_btnRootModelIgnored = new DCommandLinkButton("", this);
    m_btnRootModelIgnored->setAccessibleName("rootModelFrame_rootModelButton");
    m_btnAutoStartIgnoredCancel->setObjectName(QStringLiteral("btnRootModelIgnored"));

    // 创建体检项样式控件
    m_itemRootModel = new CheckItem(m_lbRootModelPic, m_lbRootModelTitle,
                                    m_lbRootModelStatusPic, m_lbRootModelStatus,
                                    nullptr, m_btnRootModelIgnored, nullptr, nullptr, this);
    m_itemRootModel->setAccessibleName("contentWidget_rootModelFrame");
    checkItemsLayout->addWidget(m_itemRootModel);

    // 创建取消体检按钮 重新体检按钮 返回按钮 和 一键修复按钮 整体布局
    QHBoxLayout *layoutBottomBtn = new QHBoxLayout();
    layoutBottomBtn->setObjectName(QStringLiteral("layoutCheckBtn"));
    layoutBottomBtn->setSpacing(20);

    // 取消按钮
    m_btnCheckCancel = new DPushButton(tr("Cancel"), this);
    m_btnCheckCancel->setAccessibleName("homeCheckWidget_cancleButton");
    m_btnCheckCancel->setObjectName(QStringLiteral("btnCheckCancel"));
    m_btnCheckCancel->setMinimumWidth(170);
    // 加入控件到布局
    layoutBottomBtn->addWidget(m_btnCheckCancel);

    // 返回按钮
    m_btnGoBack = new DPushButton(tr("Back"), this);
    m_btnGoBack->setAccessibleName("homeCheckWidget_backButton");
    m_btnGoBack->setObjectName(QStringLiteral("btnGoBack"));
    m_btnGoBack->setMinimumWidth(170);
    // 加入控件到布局
    layoutBottomBtn->addWidget(m_btnGoBack);

    // 添加一个弹簧到按钮布局区域
    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layoutBottomBtn->addItem(horizontalSpacer);

    // 重新体检按钮
    m_btnRecheck = new DPushButton(tr("Check Again"), this);
    m_btnRecheck->setAccessibleName("homeCheckWidget_restartCheckButton");
    m_btnRecheck->setObjectName(QStringLiteral("btnRecheck"));
    m_btnRecheck->setMinimumWidth(170);
    // 加入控件到布局
    layoutBottomBtn->addWidget(m_btnRecheck);
    // 完成按钮
    m_btnFinish = new DSuggestButton(tr("Done"), this);
    m_btnFinish->setAccessibleName("homeCheckWidget_finishButton");
    m_btnFinish->setObjectName(QStringLiteral("btnFinish"));
    m_btnFinish->setMinimumWidth(170);
    // 加入控件到布局
    layoutBottomBtn->addWidget(m_btnFinish);

    // 一键修复按钮
    m_btnRepair = new DSuggestButton(tr("Fix All"), this);
    m_btnRepair->setAccessibleName("homeCheckWidget_allRepairButton");
    m_btnRepair->setObjectName(QStringLiteral("btnRepair"));
    m_btnRepair->setMinimumWidth(170);
    // 加入控件到布局
    layoutBottomBtn->addWidget(m_btnRepair);

    // 将按钮区域布局加入到主布局
    layoutCheck->addLayout(layoutBottomBtn);
    // 设置主布局
    setLayout(layoutCheck);
    // 初始化所有体检项标题
    setPixmapByTheme();
}

// 初始化所有体检项标题
void HomePageCheckWidget::initCheckItemPicAndName()
{
    // 设置 自启动项 标题图片
    m_lbAutoStartPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_AUTO_START).pixmap(30, 30));
    // 设置 自启动项 标题
    m_lbAutoStartTitle->setText(tr("Startup programs"));

    // 设置 ssh 标题图片
    m_lbSSHPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_SSH).pixmap(30, 30));
    // 设置 ssh 标题
    m_lbSSHTitle->setText(tr("RDP port"));

    // 设置 系统更新 标题图片
    m_lbSystemVersionPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_SYS_UPDATE).pixmap(30, 30));
    // 设置 系统更新 标题
    m_lbSystemVersionTitle->setText(tr("System version"));

    // 设置 垃圾清理 标题图片
    m_lbCleanerPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_CLEANER).pixmap(30, 30));
    // 设置 垃圾清理 标题
    m_lbCleanerTitle->setText(tr("System cleanup"));

    // 设置 磁盘检测 标题图片
    m_lbDiskCheckPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_DISK_CHECK).pixmap(30, 30));
    // 设置 磁盘检测 标题
    m_lbDiskCheckTitle->setText(tr("Disk checking"));

    // 设置 开发者模式 标题图片
    m_lbRootModelPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ROOT_MODE).pixmap(30, 30));
    // 设置 开发者模式 标题
    m_lbRootModelTitle->setText(tr("Developer mode"));
}

// 重置所有体检项参数
void HomePageCheckWidget::resetAllCheckItems()
{
    // 初始化所有体检项的提示内容
    setCheckItemTipByStatus(CheckItemIndex::All, CheckStatus::WillCheck);

    // 初始化所有体检项标题
    initCheckItemPicAndName();

    m_btnAutoStart->hide();
    m_spinnerAutoStart->hide();

    m_btnSystemVersion->hide();
    m_spinnerSystemVersion->hide();

    m_btnCleaner->hide();
    m_spinnerCleaner->hide();

    m_btnDiskCheck->hide();
    m_spinnerDiskCheck->hide();
}

// 开始体检
void HomePageCheckWidget::startExamining()
{
    // 重新体检标志位
    m_isbStartExam = true;

    // 初始化分数为100
    setScoreValue(100);
    m_problemsCount = 0;
    m_sProblemsInfo = "";

    // 重置所有体检项参数
    resetAllCheckItems();

    // 开始自启动应用检测
    QTimer::singleShot(2000, this, [this] {
        m_model->checkAutoStart();
    });
    // 暂存自启动项实际的检查状态
    m_statusTmpAutoStart = CheckStatus::Checking;
    // 设置正在获取自启动应用个数中
    m_model->SetAutoStartAppChecking(true);
    if (m_model->ignoreAutoStartChecking()) {
        // 设置 自启动 体检项中间提示信息 为 “已忽略”
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);

        // 显示 “取消忽略”按钮
        setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);
        // 设置 自启动 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Ignored);
    } else {
        // 设置 自启动 体检项中间提示信息 为 “正在扫描中”
        setCheckItemTipByStatus(CheckItemIndex::AutoStartup, CheckStatus::Checking);

        // 设置 自启动 按钮
        setCheckItemBtnByStatus(CheckItemIndex::AutoStartup, CheckStatus::Checking);

        // 设置 自启动 体检项右侧旋转控件 为 旋转状态
        setCheckItemSpinnerByStatus(CheckItemIndex::AutoStartup, CheckStatus::Checking);
    }

    // ssh
    m_model->asyncGetSSHStatus();
    // 设置 SSH 体检项中间提示信息 为 “正在扫描中”
    setCheckItemTipByStatus(CheckItemIndex::SSH, CheckStatus::Checking);
    // 设置 SSH 按钮
    setCheckItemBtnByStatus(CheckItemIndex::SSH, CheckStatus::Checking);
    // 设置 SSH 体检项右侧旋转控件 为 旋转状态
    setCheckItemSpinnerByStatus(CheckItemIndex::SSH, CheckStatus::Checking);

    // 通知去检查系统更新
    Q_EMIT m_model->NotifyCheckSysUpdate();
    // 设置正在检查系统更新中
    m_model->SetSysVersionChecking(true);
    // 设置检测系统版本的最大时间
    m_sysVerCheckTimeoutTimer->stop();
    m_sysVerCheckTimeoutTimer->start(1000 * SYS_VER_CHECK_TIMEOUT);
    // 设置 系统更新 体检项中间提示信息 为 “正在扫描中”
    setCheckItemTipByStatus(CheckItemIndex::SysVer, CheckStatus::Checking);
    // 设置 系统更新 体检项右侧旋转控件 为 旋转状态
    setCheckItemSpinnerByStatus(CheckItemIndex::SysVer, CheckStatus::Checking);

    // 磁盘管理器服务能正常运行，则进行磁盘检测
    if (m_model->isDiskManagerInterValid()) {
        // 磁盘检测
        m_itemDiskCheck->show();
        checkDisk();
    }
    // 磁盘管理器服务不能正常运行，则屏蔽磁盘管理项
    else {
        // 隐藏磁盘管理项
        m_itemDiskCheck->hide();
    }

    // 垃圾清理 检测
    setCheckItemTipByStatus(CheckItemIndex::Cleaner, CheckStatus::Checking);
    setCheckItemSpinnerByStatus(CheckItemIndex::Cleaner, CheckStatus::Checking);
    QTimer::singleShot(4000, this, [this] {
        Q_EMIT m_model->requestStartTrashScan();
    });

    // 检测开发者模式
    if (m_model->ignoreDevModeChecking()) {
        // 设置 开发者模式 体检项中间提示信息 为 “已忽略”
        setCheckItemTipByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);

        // 显示 “忽略”按钮
        setCheckItemBtnByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);
        // 设置 开发者模式 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::DevModel, CheckStatus::Ignored);

        m_itemRootModel->hide();
    } else {
        checkRootModel();
    }

    // 更新底部各操作按钮显示状态
    updateBottomBtns();
}

// 设置体检项中间提示信息
void HomePageCheckWidget::setCheckItemTipByStatus(CheckItemIndex itemIndex, CheckStatus status)
{
    // 启动项
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::AutoStartup == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbAutoStartStatusPic->setVisible(false);
            m_lbAutoStartStatus->setText(tr("Scanning..."));
        }
        // 如果是等待扫描状态
        else if (CheckStatus::WillCheck == status) {
            m_lbAutoStartStatusPic->setVisible(false);
            m_lbAutoStartStatus->setText(tr("Waiting"));
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_lbAutoStartStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbAutoStartStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbAutoStartStatus->setTextFormat(Qt::RichText);
            m_lbAutoStartStatus->setText(tr("<font color=%1>%2 apps</font>")
                                             .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                             .arg(m_nAutoStartAppCount));
        }
        // 如果是忽略状态
        else if (CheckStatus::Ignored == status) {
            m_lbAutoStartStatusPic->setVisible(false);
            m_lbAutoStartStatus->setText(tr("Dismissed"));
        }
        // 其他为无异常状态
        else {
            m_lbAutoStartStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbAutoStartStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbAutoStartStatus->setTextFormat(Qt::RichText);
            m_lbAutoStartStatus->setText(tr("<font color=%1>%2 apps</font>")
                                             .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                             .arg(m_nAutoStartAppCount));
        }
    }

    // ssh
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::SSH == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbSSHStatusPic->setVisible(false);
            m_lbSSHStatus->setText(tr("Scanning..."));
        } else if (CheckStatus::Abnormal == status) {
            // 如果是有问题状态
            m_lbSSHStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbSSHStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbSSHStatus->setTextFormat(Qt::RichText);
            m_lbSSHStatus->setText(QString("<font color=%1>%2</font>")
                                       .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                       .arg(tr("Open")));
        } else {
            m_lbSSHStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbSSHStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbSSHStatus->setTextFormat(Qt::RichText);
            m_lbSSHStatus->setText(QString("<font color=%1>%2</font>")
                                       .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                       .arg(tr("Closed")));
        }
    }

    // 系统版本
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::SysVer == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbSystemVersionStatusPic->setVisible(false);
            m_lbSystemVersionStatus->setText(tr("Scanning..."));
        }
        // 如果是等待扫描状态
        else if (CheckStatus::WillCheck == status) {
            m_lbSystemVersionStatusPic->setVisible(false);
            m_lbSystemVersionStatus->setText(tr("Waiting"));
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_lbSystemVersionStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbSystemVersionStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbSystemVersionStatus->setTextFormat(Qt::RichText);
            m_lbSystemVersionStatus->setText(QString("<font color=%1>%2</font>")
                                                 .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                                 .arg(tr("Out of date")));
        }
        // 其他为无异常状态
        else {
            m_lbSystemVersionStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbSystemVersionStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbSystemVersionStatus->setTextFormat(Qt::RichText);
            m_lbSystemVersionStatus->setText(QString("<font color=%1>%2</font>")
                                                 .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                                 .arg(tr("Up to date")));
        }
    }

    // 垃圾清理
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::Cleaner == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbCleanerStatusPic->setVisible(false);
            m_lbCleanerStatus->setText(tr("Scanning..."));
        }
        // 如果是等待扫描状态
        else if (CheckStatus::WillCheck == status) {
            m_lbCleanerStatusPic->setVisible(false);
            m_lbCleanerStatus->setText(tr("Waiting"));
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_lbCleanerStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbCleanerStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbCleanerStatus->setTextFormat(Qt::RichText);
            m_lbCleanerStatus->setText(QString("<font color=%1>%2</font>")
                                           .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                           .arg(tr("There are junk files")));
        }
        // 其他为无异常状态
        else {
            m_lbCleanerStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbCleanerStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbCleanerStatus->setTextFormat(Qt::RichText);
            m_lbCleanerStatus->setText(QString("<font color=%1>%2</font>")
                                           .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                           .arg(tr("Clean")));
        }
    }

    // 磁盘检测
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::DiskCheck == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbDiskCheckStatusPic->setVisible(false);
            m_lbDiskCheckStatus->setText(tr("Scanning..."));
        }
        // 如果是等待扫描状态
        else if (CheckStatus::WillCheck == status) {
            m_lbDiskCheckStatusPic->setVisible(false);
            m_lbDiskCheckStatus->setText(tr("Waiting"));
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_lbDiskCheckStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbDiskCheckStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbDiskCheckStatus->setTextFormat(Qt::RichText);
            m_lbDiskCheckStatus->setText(QString("<font color=%1>%2</font>")
                                             .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                             .arg(tr("Found errors")));
        }
        // 其他为无异常状态
        else {
            m_lbDiskCheckStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbDiskCheckStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbDiskCheckStatus->setTextFormat(Qt::RichText);
            m_lbDiskCheckStatus->setText(QString("<font color=%1>%2</font>")
                                             .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                             .arg(tr("No errors found")));
        }
    }

    // 开发者模式
    if (CheckItemIndex::All == itemIndex || CheckItemIndex::DevModel == itemIndex) {
        // 如果是扫描状态
        if (CheckStatus::Checking == status) {
            m_lbRootModelStatusPic->setVisible(false);
            m_lbRootModelStatus->setText(tr("Scanning..."));
        }
        // 如果是等待扫描状态
        else if (CheckStatus::WillCheck == status) {
            m_lbRootModelStatusPic->setVisible(false);
            m_lbRootModelStatus->setText(tr("Waiting"));
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_lbRootModelStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_ABNORMAL).pixmap(12, 12));
            m_lbRootModelStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbRootModelStatus->setTextFormat(Qt::RichText);
            m_lbRootModelStatus->setText(QString("<font color=%1>%2</font>")
                                             .arg(CHECK_ITEM_TIP_ABNORMAL_COLOR_HTML)
                                             .arg(tr("Root access allowed, which is risky")));
        }
        // 如果是忽略状态
        else if (CheckStatus::Ignored == status) {
            m_lbRootModelStatusPic->setVisible(false);
            m_lbRootModelStatus->setText(tr("Dismissed"));
        }
        // 其他为无异常状态
        else {
            m_lbRootModelStatusPic->setPixmap(QIcon::fromTheme(CHECK_ITEM_NORMAL).pixmap(12, 12));
            m_lbRootModelStatusPic->setVisible(true);
            // 支持富文本(html)
            m_lbRootModelStatus->setTextFormat(Qt::RichText);
            m_lbRootModelStatus->setText(QString("<font color=%1>%2</font>")
                                             .arg(CHECK_ITEM_TIP_NORMAL_COLOR_HTML)
                                             .arg(tr("No root access")));
        }
    }
}

// 设置体检项右侧按钮内容
void HomePageCheckWidget::setCheckItemBtnByStatus(CheckItemIndex itemIndex, CheckStatus status)
{
    switch (itemIndex) {
    case CheckItemIndex::AutoStartup: {
        // 自启动项
        m_btnAutoStart->setVisible(false);
        m_btnAutoStartIgnored->setVisible(false);
        m_btnAutoStartIgnoredCancel->setVisible(false);
        m_spinnerAutoStart->setVisible(false);

        // 如果是已修复状态
        if (CheckStatus::Fixed == status) {
            m_btnAutoStart->setText(tr("Fixed"));
            m_btnAutoStart->setDisabled(true);
            // 显示 自启动项 的按钮
            m_btnAutoStart->setVisible(true);
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_btnAutoStart->setText(tr("Check up"));
            m_btnAutoStart->setDisabled(false);
            // 显示 自启动项 的按钮
            m_btnAutoStart->setVisible(true);
            // 忽略按钮
            m_btnAutoStartIgnored->setText(tr("Dismiss"));
            m_btnAutoStartIgnored->setVisible(true);
        }
        // 如果是忽略状态
        else if (CheckStatus::Ignored == status) {
            m_btnAutoStartIgnoredCancel->setText(tr("No dismission"));
            // 显示 取消忽略按钮
            m_btnAutoStartIgnoredCancel->setVisible(true);
        }
        break;
    }
    case CheckItemIndex::SSH: {
        // ssh检测项
        // 如果是已修复状态
        if (CheckStatus::Fixed == status) {
            m_btnSSH->setText(tr("Fixed"));
            m_btnSSH->setDisabled(true);
            // 显示 ssh检测项 的按钮
            m_btnSSH->setVisible(true);
        } else if (CheckStatus::Abnormal == status) { // 如果是有问题状态
            m_btnSSH->setText(tr("Close it"));
            m_btnSSH->setDisabled(false);
            // 显示 ssh检测项 的按钮
            m_btnSSH->setVisible(true);
        } else {
            // 隐藏 ssh检测项 的按钮
            m_btnSSH->setVisible(false);
        }
        break;
    }
    case CheckItemIndex::SysVer: {
        // 系统更新项
        // 如果是已修复状态
        if (CheckStatus::Fixed == status) {
            m_btnSystemVersion->setText(tr("Fixed"));
            m_btnSystemVersion->setDisabled(true);
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_btnSystemVersion->setText(tr("Check for updates"));
            m_btnSystemVersion->setDisabled(false);
        }
        // 显示 系统更新项 的按钮
        m_btnSystemVersion->setVisible(true);
        // 隐藏 系统更新项 的旋转控件
        m_spinnerSystemVersion->setVisible(false);
        break;
    }
    case CheckItemIndex::Cleaner: {
        // 垃圾清理项
        // 如果是已修复状态
        if (CheckStatus::Fixed == status) {
            m_btnCleaner->setText(tr("Fixed"));
            m_btnCleaner->setDisabled(true);
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_btnCleaner->setText(tr("Clean up"));
            m_btnCleaner->setDisabled(false);
        }
        // 显示 垃圾清理项 的按钮
        m_btnCleaner->setVisible(true);
        // 隐藏 垃圾清理项 的旋转控件
        m_spinnerCleaner->setVisible(false);
        break;
    }
    case CheckItemIndex::DiskCheck: {
        // 磁盘检测项
        // 如果是已修复状态
        if (CheckStatus::Fixed == status) {
            m_btnDiskCheck->setText(tr("Fixed"));
            m_btnDiskCheck->setDisabled(true);
        }
        // 如果是有问题状态
        else if (CheckStatus::Abnormal == status) {
            m_btnDiskCheck->setText(tr("Go to check"));
            m_btnDiskCheck->setDisabled(false);
        }
        // 显示 磁盘检测项 的按钮
        m_btnDiskCheck->setVisible(true);
        // 隐藏 磁盘检测项 的旋转控件
        m_spinnerDiskCheck->setVisible(false);
        break;
    }
    case CheckItemIndex::DevModel: {
        // 开发者模式
        // 忽略按钮
        m_btnRootModelIgnored->setText(tr("Dismiss"));
        m_btnRootModelIgnored->setVisible(true);
        // 如果是忽略状态
        if (CheckStatus::Ignored == status) {
            // 隐藏 忽略按钮
            m_btnRootModelIgnored->setVisible(false);
        }
        break;
    }
    default:
        break;
    }
}

// 设置体检项右侧旋转控件
void HomePageCheckWidget::setCheckItemSpinnerByStatus(CheckItemIndex itemIndex, CheckStatus status)
{
    switch (itemIndex) {
    case CheckItemIndex::AutoStartup: {
        // 自启动项
        // 如果是正在扫描状态 和 正在修复状态
        if (CheckStatus::Checking == status || CheckStatus::Fixing == status) {
            // 显示 旋转控件
            m_spinnerAutoStart->start();
            m_spinnerAutoStart->setVisible(true);
            // 隐藏 按钮
            m_btnAutoStart->setVisible(false);
        }
        // 其他情况隐藏 旋转控件
        else {
            m_spinnerAutoStart->stop();
            m_spinnerAutoStart->setVisible(false);
        }
        // 设置 体检项 操作状态
        m_statusAutoStart = status;
        break;
    }
    case CheckItemIndex::SSH: {
        // ssh项
        // 如果是正在扫描状态 和 正在修复状态
        if (CheckStatus::Checking == status || CheckStatus::Fixing == status) {
            // 显示 旋转控件
            m_spinnerSSH->start();
            m_spinnerSSH->setVisible(true);
            // 隐藏 按钮
            m_btnSSH->setVisible(false);
        }
        // 其他情况隐藏 旋转控件
        else {
            m_spinnerSSH->stop();
            m_spinnerSSH->setVisible(false);
        }

        // 设置状态
        m_statusSSH = status;
        break;
    }
    case CheckItemIndex::SysVer: {
        // 系统更新项
        // 如果是正在扫描状态 和 正在修复状态
        if (CheckStatus::Checking == status || CheckStatus::Fixing == status) {
            // 显示 旋转控件
            m_spinnerSystemVersion->start();
            m_spinnerSystemVersion->setVisible(true);
            // 隐藏 按钮
            m_btnSystemVersion->setVisible(false);
        }
        // 其他情况隐藏 旋转控件
        else {
            m_spinnerSystemVersion->stop();
            m_spinnerSystemVersion->setVisible(false);
        }
        // 设置 体检项 操作状态
        m_statusSystemVersion = status;
        break;
    }
    case CheckItemIndex::Cleaner: {
        // 垃圾清理项
        // 如果是正在扫描状态 和 正在修复状态
        if (CheckStatus::Checking == status || CheckStatus::Fixing == status) {
            // 显示 旋转控件
            m_spinnerCleaner->start();
            m_spinnerCleaner->setVisible(true);
            // 隐藏 按钮
            m_btnCleaner->setVisible(false);
        }
        // 其他情况隐藏 旋转控件
        else {
            m_spinnerCleaner->stop();
            m_spinnerCleaner->setVisible(false);
        }
        // 设置 体检项 操作状态
        m_statusCleaner = status;
        break;
    }
    case CheckItemIndex::DiskCheck: {
        // 磁盘检测项
        // 如果是正在扫描状态 和 正在修复状态
        if (CheckStatus::Checking == status || CheckStatus::Fixing == status) {
            // 显示 旋转控件
            m_spinnerDiskCheck->start();
            m_spinnerDiskCheck->setVisible(true);
            // 隐藏 按钮
            m_btnDiskCheck->setVisible(false);
        }
        // 其他情况隐藏 旋转控件
        else {
            m_spinnerDiskCheck->stop();
            m_spinnerDiskCheck->setVisible(false);
        }
        // 设置 体检项 操作状态
        m_statusDiskCheck = status;
        break;
    }
    case CheckItemIndex::DevModel: {
        // 开发者模式检测项
        // 设置 体检项 操作状态
        m_statusRootModel = status;
        break;
    }
    default:
        break;
    }
}

// 设置得分
void HomePageCheckWidget::setScoreValue(int value)
{
    // 设置分数
    m_scoreValue = value;

    // 设置分数动画
    // 起始值
    m_scoreValueAni->setStartValue(m_currentScore);
    // 终止值
    m_scoreValueAni->setEndValue(m_scoreValue);
    // 计算动画间隔
    int duration = VALUE_CHANGE_INTERVAL_MS * qAbs(m_scoreValue - m_currentScore);
    m_scoreValueAni->setDuration(duration);
    m_scoreValueAni->start();

    // 将分数存在gsetting中
    m_model->setSafetyScore(value);
    // 将体检时间存在gsetting中
    m_model->SetLastCheckTime(QDateTime::currentDateTime());

    // 设置进度条
    m_scoreProg->setValue(m_scoreValue);

    // 随分值改变颜色
    if (PERFECTLY_SAFE_SCORE_ABOVE < m_scoreValue && m_scoreValue <= 100) {
        // 改变分数标签颜色
        QPalette paTmp = m_lbScore->palette();
        paTmp.setColor(QPalette::Text, PERFECTLY_SAFE_SCORE_COLOR);
        m_lbScore->setPalette(paTmp);
        // 更改问题项数提示 文字
        m_lbResultTip->setText(tr("<font color=%1>%2</font> issue(s) found")
                                   .arg(PERFECTLY_SAFE_SCORE_COLOR_HTML)
                                   .arg(m_problemsCount));
        // 改变进度条颜色
        m_scoreProg->setColor(PERFECTLY_SAFE_SCORE_COLOR);
        // 改变进度条阴影颜色
        m_progressBarShadow->setColor(PERFECTLY_SAFE_SCORE_SHADOW_COLOR);
    } else if (SAFE_SCORE_ABOVE < m_scoreValue) {
        // 改变分数标签颜色
        QPalette paTmp = m_lbScore->palette();
        paTmp.setColor(QPalette::Text, SAFE_SCORE_COLOR);
        m_lbScore->setPalette(paTmp);
        // 更改问题项数提示 文字
        m_lbResultTip->setText(tr("<font color=%1>%2</font> issue(s) found")
                                   .arg(SAFE_SCORE_COLOR_HTML)
                                   .arg(m_problemsCount));
        // 改变进度条颜色
        m_scoreProg->setColor(SAFE_SCORE_COLOR);
        // 改变进度条阴影颜色
        m_progressBarShadow->setColor(SAFE_SCORE_SHADOW_COLOR);
    } else if (DANGEROUS_SCORE_ABOVE < m_scoreValue) {
        // 改变分数标签颜色
        QPalette paTmp = m_lbScore->palette();
        paTmp.setColor(QPalette::Text, DANGEROUS_SCORE_COLOR);
        m_lbScore->setPalette(paTmp);
        // 更改问题项数提示 文字
        m_lbResultTip->setText(tr("<font color=%1>%2</font> issue(s) found")
                                   .arg(DANGEROUS_SCORE_COLOR_HTML)
                                   .arg(m_problemsCount));
        // 改变进度条颜色
        m_scoreProg->setColor(DANGEROUS_SCORE_COLOR);
        // 改变进度条阴影颜色
        m_progressBarShadow->setColor(DANGEROUS_SCORE_SHADOW_COLOR);
    } else {
        // 改变分数标签颜色
        QPalette paTmp = m_lbScore->palette();
        paTmp.setColor(QPalette::Text, EXTREMELY_DANGEROUS_SCORE_COLOR);
        m_lbScore->setPalette(paTmp);
        // 更改问题项数提示 文字
        m_lbResultTip->setText(tr("<font color=%1>%2</font> issue(s) found")
                                   .arg(EXTREMELY_DANGEROUS_SCORE_COLOR_HTML)
                                   .arg(m_problemsCount));
        // 改变进度条颜色
        m_scoreProg->setColor(EXTREMELY_DANGEROUS_SCORE_COLOR);
        // 改变进度条阴影颜色
        m_progressBarShadow->setColor(EXTREMELY_DANGEROUS_SCORE_SHADOW_COLOR);
    }
}

void HomePageCheckWidget::fixSSH()
{
    // 关闭ssh
    m_model->asyncSetSSHStatus(false);

    // 设置 ssh 按钮
    setCheckItemBtnByStatus(CheckItemIndex::SSH, CheckStatus::Fixing);
    // 设置 ssh 体检项右侧旋转控件 为 正在修复状态
    setCheckItemSpinnerByStatus(CheckItemIndex::SSH, CheckStatus::Fixing);
}

// 清理选中的垃圾文件
void HomePageCheckWidget::cleanSelectTrash()
{
    Q_EMIT m_model->requestCleanSelectTrash();
    this->setCheckItemSpinnerByStatus(CheckItemIndex::Cleaner, CheckStatus::Fixing);
    updateBottomBtns();

    m_model->addSecurityLog(tr("Junk files cleaned up"));
}

// 磁盘检测
void HomePageCheckWidget::checkDisk()
{
    bool normal = m_model->getDiskStatus();
    if (!normal) {
        m_scoreValue -= SCORE_DISCK_CHECK_ABNORMAL; // 减分
        // 问题数增加
        m_problemsCount++;
        if (!m_sProblemsInfo.isEmpty()) {
            m_sProblemsInfo += ", ";
        }
        m_sProblemsInfo += tr("Disk checking");

        setScoreValue(m_scoreValue);
        // 设置 磁盘检测 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::DiskCheck, CheckStatus::Abnormal);
        // 设置 磁盘检测 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::DiskCheck, CheckStatus::Abnormal);
        // 设置 磁盘检测 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::DiskCheck, CheckStatus::Abnormal);
    } else {
        // 设置 磁盘检测 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::DiskCheck, CheckStatus::Normal);
        // 设置 磁盘检测 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::DiskCheck, CheckStatus::Fixed);
    }
}

// 打开磁盘管理器
void HomePageCheckWidget::openDiskManager()
{
    QProcess::startDetached("deepin-diskmanager");

    m_model->addSecurityLog(tr("Disk errors checked"));
}

// 检测开发者模式
void HomePageCheckWidget::checkRootModel()
{
    bool rootModel = m_model->IsDeveloperMode();
    if (rootModel) {
        m_scoreValue -= SCORE_ROOT_MODEL_ABNORMAL; // 减分
        // 问题数增加
        m_problemsCount++;
        if (!m_sProblemsInfo.isEmpty()) {
            m_sProblemsInfo += ", ";
        }
        m_sProblemsInfo += tr("Developer mode");

        setScoreValue(m_scoreValue);
        // 设置 开发者模式 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::DevModel, CheckStatus::Abnormal);
        // 设置 开发者模式 体检项右侧按钮内容
        setCheckItemBtnByStatus(CheckItemIndex::DevModel, CheckStatus::Abnormal);
        // 设置 开发者模式 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::DevModel, CheckStatus::Abnormal);
    } else {
        // 设置 开发者模式 体检项中间提示信息
        setCheckItemTipByStatus(CheckItemIndex::DevModel, CheckStatus::Normal);
        // 设置 开发者模式 体检项右侧旋转控件
        setCheckItemSpinnerByStatus(CheckItemIndex::DevModel, CheckStatus::Fixed);
    }
}

// 取消体检
void HomePageCheckWidget::cancelChecking()
{
    m_model->notifyStopCheckSysUpdate();
    Q_EMIT m_model->notifyShowHomepage();
}

// 一键修复
void HomePageCheckWidget::fixAll()
{
    // 一建清理 扫描出的垃圾文件
    if (CheckStatus::Abnormal == m_statusCleaner) {
        cleanSelectTrash();
    } else {
        m_statusCleaner = CheckStatus::Fixed;
    }

    // 远程登陆端口 开启
    if (CheckStatus::Abnormal == m_statusSSH) {
        fixSSH();
    } else {
        m_statusSSH = CheckStatus::Fixed;
    }

    // 更新底部按钮
    updateBottomBtns();
}

void HomePageCheckWidget::updateBottomBtns()
{
    // 若存在某一体检项正在检测中
    if (CheckStatus::Checking == m_statusAutoStart
        || CheckStatus::Checking == m_statusSystemVersion
        || CheckStatus::Checking == m_statusCleaner
        || CheckStatus::Checking == m_statusDiskCheck
        || CheckStatus::Checking == m_statusRootModel) {
        // 显示取消按钮
        m_btnCheckCancel->setVisible(true);
        // 使能取消按钮
        m_btnCheckCancel->setDisabled(false);
        // 隐藏完成按钮
        m_btnFinish->setVisible(false);
        // 隐藏返回按钮
        m_btnGoBack->setVisible(false);
        // 隐藏重新体检按钮
        m_btnRecheck->setVisible(false);
        // 隐藏一键修复按钮
        m_btnRepair->setVisible(false);
        return;
    } else {
        // 若所有体检项都检测完成
        // 隐藏取消按钮
        m_btnCheckCancel->setVisible(false);
        // 隐藏完成按钮
        m_btnFinish->setVisible(false);
        // 显示返回按钮
        m_btnGoBack->setVisible(true);
        // 显示重新体检按钮
        m_btnRecheck->setVisible(true);
        // 显示一键修复按钮
        m_btnRepair->setVisible(true);

        if (m_isbStartExam) {
            // 添加安全日志
            m_model->addSecurityLog(tr("%1 issues found in %2").arg(m_problemsCount).arg(m_sProblemsInfo));
            m_isbStartExam = false;
        }
    }

    // // 若存在某一体检项正在修复中
    if (CheckStatus::Fixing == m_statusAutoStart
        || CheckStatus::Fixing == m_statusSystemVersion
        || CheckStatus::Fixing == m_statusCleaner
        || CheckStatus::Fixing == m_statusDiskCheck
        || CheckStatus::Fixing == m_statusRootModel) {
        // 禁用取消按钮
        m_btnCheckCancel->setEnabled(false);
        // 禁用完成按钮
        m_btnFinish->setEnabled(false);
        // 禁用返回按钮
        m_btnGoBack->setEnabled(false);
        // 禁用重新体检按钮
        m_btnRecheck->setEnabled(false);
        // 禁用一键修复按钮
        m_btnRepair->setEnabled(false);
        return;
    }
    if (((CheckStatus::Normal == m_statusCleaner) || (CheckStatus::Fixed == m_statusCleaner))
        && ((CheckStatus::Normal == m_statusSSH) || (CheckStatus::Fixed == m_statusSSH))) {
        // 若所有体检项修复完成（目前只能一键修复病毒木马、病毒库版本更新和垃圾清理，ssh处于关闭状态）
        // 隐藏取消按钮
        m_btnCheckCancel->setVisible(false);
        // 使能取消按钮
        m_btnCheckCancel->setEnabled(true);
        // 显示完成按钮
        m_btnFinish->setVisible(true);
        // 使能完成按钮
        m_btnFinish->setEnabled(true);
        // 隐藏返回按钮
        m_btnGoBack->setVisible(false);
        // 使能返回按钮
        m_btnGoBack->setEnabled(true);
        // 显示重新体检按钮
        m_btnRecheck->setVisible(true);
        // 使能重新体检按钮
        m_btnRecheck->setEnabled(true);
        // 隐藏一键修复按钮
        m_btnRepair->setVisible(false);
        // 使能一键修复按钮
        m_btnRepair->setEnabled(true);
    } else {
        //　如果是当前没有修复中的项
        // 可以操作返回按钮
        m_btnGoBack->setEnabled(true);
        // 可以操作重新体检按钮
        m_btnRecheck->setEnabled(true);
        // 可以一键修复按钮
        m_btnRepair->setEnabled(true);
    }
}
