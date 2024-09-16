// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"
#include "widgets/scorebar.h"

#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DSuggestButton>
#include <DSysInfo>
#include <DLabel>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>

HomePage::HomePage(HomePageModel *mode, QWidget *parent)
    : QWidget(parent)
    , m_mode(mode)
    , m_guiHelper(nullptr)
    , m_scorePic(nullptr)
    , m_lbScoreTip(nullptr)
    , m_examTip(nullptr)
    , m_examBtn(nullptr)
{
    this->setAccessibleName("rightWidget_homeWidget");
    // 获取应用界面工具实例
    m_guiHelper = DGuiApplicationHelper::instance();

    // 主布局器
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setAlignment(Qt::AlignHCenter);
    setLayout(mainLayout);

    // 首页展现的图片
    m_scorePic = new QLabel(this);
    m_scorePic->setAccessibleName("homeWidget_scorePictureLable");
    // 水平居中对齐
    m_scorePic->setAlignment(Qt::AlignHCenter);
    m_scorePic->setPixmap(QIcon::fromTheme(HOMEPAGE_UOS_OEM_LIGHT).pixmap(300, 300));
    mainLayout->addWidget(m_scorePic);

    // 分数提示项
    m_lbScoreTip = new DLabel(this);
    m_lbScoreTip->setAccessibleName("homeWidget_scoreTipLable");
    m_lbScoreTip->setAlignment(Qt::AlignHCenter);
    // 设置分数提示字体样式
    QFont ft = m_lbScoreTip->font();
    ft.setBold(true);
    m_lbScoreTip->setFont(ft);
    DFontSizeManager::instance()->bind(m_lbScoreTip, DFontSizeManager::T3);
    mainLayout->addWidget(m_lbScoreTip);

    // 体检提示标签
    m_examTip = new DTipLabel("", this);
    m_examTip->setAccessibleName("homeWidget_examTipLable");
    m_examTip->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(m_examTip, DFontSizeManager::T5);
    mainLayout->addWidget(m_examTip);
    // 添加间隔
    mainLayout->addSpacing(25);
    // 体检按钮
    m_examBtn = new DSuggestButton(this);
    m_examBtn->setAccessibleName("homeWidget_examButton");
    m_examBtn->setFixedWidth(200);
    mainLayout->addWidget(m_examBtn, 0, Qt::AlignHCenter);
    // 添加布局弹簧
    mainLayout->addStretch(1);
    // 更新界面
    updateUI();
    // 连接信号
    connect(m_examBtn, &DSuggestButton::clicked, this, &HomePage::notifyShowCheckPage);
    // 主题变换
    connect(m_guiHelper, &DGuiApplicationHelper::themeTypeChanged, this, &HomePage::changePicWithTheme);
}

HomePage::~HomePage()
{
}

// 设置数据处理对象
void HomePage::setModel(HomePageModel *mode)
{
    m_mode = mode;
}

// 更新界面
void HomePage::updateUI()
{
    // 先隐藏所有控件
    m_lbScoreTip->hide();
    m_examTip->hide();
    m_examBtn->hide();

    // 根据体检分数，改变显示
    int score = m_mode->GetSafetyScore();
    // 若分数为“-1”，则表示没有体检过
    if (-1 == score) {
        // 分数提示项
        m_lbScoreTip->show();
        m_lbScoreTip->setText(tr("Your computer has not been checked yet"));
        m_lbScoreTip->setWordWrap(true);
        m_examTip->show();
        m_examTip->setText(tr("Fully check security issues with one click"));
        m_examTip->setWordWrap(true);
        m_examBtn->show();
        m_examBtn->setText(tr("Check Now"));
        return;
    }
    // 非常安全分数阶段
    else if (PERFECTLY_SAFE_SCORE_ABOVE < score) {
        // 分数提示项
        m_lbScoreTip->show();
        m_lbScoreTip->setText(tr("Last Result: <font color=%1>%2</font> points")
                                  .arg(PERFECTLY_SAFE_SCORE_COLOR_HTML)
                                  .arg(score));
        m_examTip->show();
        m_examBtn->show();
        m_examBtn->setText(tr("Check Now"));
    }
    // 安全分数阶段
    else if (SAFE_SCORE_ABOVE < score) {
        // 分数提示项
        m_lbScoreTip->show();
        m_lbScoreTip->setText(tr("Last Result: <font color=%1>%2</font> points")
                                  .arg(SAFE_SCORE_COLOR_HTML)
                                  .arg(score));
        m_examTip->show();
        m_examBtn->show();
        m_examBtn->setText(tr("Check Now"));
    }
    // 危险分数阶段
    else if (DANGEROUS_SCORE_ABOVE < score) {
        // 分数提示项
        m_lbScoreTip->show();
        m_lbScoreTip->setText(tr("Last Result: <font color=%1>%2</font> points")
                                  .arg(DANGEROUS_SCORE_COLOR_HTML)
                                  .arg(score));
        m_examTip->show();
        m_examBtn->show();
        m_examBtn->setText(tr("Check Now"));
    }
    // 非常危险分数阶段
    else {
        // 分数提示项
        m_lbScoreTip->show();
        m_lbScoreTip->setText(tr("Last Result: <font color=%1>%2</font> points")
                                  .arg(EXTREMELY_DANGEROUS_SCORE_COLOR_HTML)
                                  .arg(score));
        m_examTip->show();
        m_examBtn->show();
        m_examBtn->setText(tr("Check Now"));
    }

    //根据上次体检时间，调整体检提示标签文字
    QDateTime lastCheckTime = m_mode->GetLastCheckTime();
    qint64 deltaTime = lastCheckTime.secsTo(QDateTime::currentDateTime());
    // 计算时间间隔
    float minutes = deltaTime / 60;
    float hours = minutes / 60;
    float days = hours / 24;
    // 若上次体检时间为当天
    if (lastCheckTime.date() == QDate::currentDate()) {
        // 若小于2小时
        if (2 > hours) {
            m_examTip->setText(tr("Checked just now"));
        }
        // 若大于2小时
        else {
            m_examTip->setText(tr("Checked %1 hours ago").arg(int(hours)));
        }
    }
    // 若距上次体检超过1天
    else {
        // 小于0天
        if (0 > days) {
            m_examTip->setText(tr("Checked just now"));
        } else if (1 >= days) { // 不满1天，记1天间隔
            m_examTip->setText(tr("Checked %1 days ago").arg(1));
        } else { // 超过1天
            m_examTip->setText(tr("Checked %1 days ago").arg(int(days)));
        }
    }

    // 随主题类型改变首页图
    changePicWithTheme(m_guiHelper->themeType());
}

void HomePage::changePicWithTheme(DGuiApplicationHelper::ColorType themeType)
{
    QIcon icon;
    // 只有是社区版才是社区  其他的都是专业
    if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosCommunity) {
        // 社区版本
        if (themeType == DGuiApplicationHelper::ColorType::LightType) {
            icon = QIcon::fromTheme(HOMEPAGE_DEEPIN_LIGHT);
        } else {
            icon = QIcon::fromTheme(HOMEPAGE_DEEPIN_DARK);
        }
    } else {
        // 专业版本
        if (themeType == DGuiApplicationHelper::ColorType::LightType) {
            icon = QIcon::fromTheme(HOMEPAGE_UOS_OEM_LIGHT);
        } else {
            icon = QIcon::fromTheme(HOMEPAGE_UOS_OEM_DARK);
        }
    }
    m_scorePic->setPixmap(icon.pixmap(300, 300));
}
