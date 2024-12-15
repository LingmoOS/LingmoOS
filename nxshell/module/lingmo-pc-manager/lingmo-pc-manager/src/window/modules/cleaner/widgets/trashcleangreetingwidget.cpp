// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleangreetingwidget.h"

#include "../trashcleandefinition.h"
#include "trashcleanconfigitem.h"

#include <DFontSizeManager>
#include <DLabel>

#include <QGridLayout>
#include <QPixmap>

DWIDGET_USE_NAMESPACE

TrashCleanGreetingWidget::TrashCleanGreetingWidget(QWidget *parent)
    : DFrame(parent)
    , m_logo(nullptr)
    , m_title(nullptr)
    , m_greeting(nullptr)
    , m_sysItem(nullptr)
    , m_appItem(nullptr)
    , m_hisItem(nullptr)
    , m_cookiesItem(nullptr)
    , m_scanBtn(nullptr)
//, m_guiHelper(nullptr)
{
    this->setFixedSize(770, 590);
    this->setFocusPolicy(Qt::NoFocus);
    this->setFrameShape(QFrame::NoFrame);
    m_scanConfigList = { 0, 0, 0, 0 };
    initUI();
    initConnection();

    m_sysItem->setCheckBoxStatus(true);
    m_appItem->setCheckBoxStatus(true);
    m_hisItem->setCheckBoxStatus(false);
    m_cookiesItem->setCheckBoxStatus(false);

    // 主题变换 改变部分图标颜色
    //    m_guiHelper = DGuiApplicationHelper::instance();
    //    setPixmapByTheme(m_guiHelper->themeType());
    //    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
    //    &TrashCleanGreetingWidget::setPixmapByTheme);
}

void TrashCleanGreetingWidget::setGreeing(const QString &info)
{
    if (!info.isEmpty()) {
        m_greeting->setText(tr("Last clean: %1").arg(info));
    } else {
        m_greeting->setText(tr("Start cleaning now to refresh your computer"));
    }
}

void TrashCleanGreetingWidget::onItemStageChanged(int id, int stage)
{
    if (id < 0 || id > m_scanConfigList.length() - 1) {
        return;
    }
    m_scanConfigList[id] = stage;

    m_scanBtn->setDisabled(m_scanConfigList.count(Qt::Unchecked) == m_scanConfigList.length());
}

void TrashCleanGreetingWidget::initUI()
{
    m_logo = new DLabel(this);
    m_logo->setFixedSize(400, 220);
    m_logo->setPixmap(QIcon::fromTheme(kGreetingLogoPath).pixmap(400, 220));
    m_logo->setAccessibleName("greetingLogo");

    m_title = new DLabel(this);
    m_title->setText(tr("Clean out junk files and free up disk space"));
    m_title->setAccessibleName("greetingTitle");
    QFont titleFont = m_logo->font();
    titleFont.setPixelSize(30);
    titleFont.setWeight(QFont::DemiBold);
    m_title->setFont(titleFont);

    m_greeting = new DLabel(this);
    m_greeting->setAccessibleName("greetingInfo");
    m_greeting->setText(tr("Start cleaning now to refresh your computer"));
    QFont greFont = m_greeting->font();
    greFont.setPixelSize(14);
    greFont.setWeight(QFont::Normal);
    m_greeting->setFont(greFont);

    m_sysItem = new TrashCleanConfigItem(this);
    m_sysItem->setIconPath(kSysIconPath);
    m_sysItem->setTitles(tr("System junk"), tr("Useless files created by the system"));
    m_sysItem->setIndex(SYS_CONFIG_INDEX);
    m_sysItem->setAccessibleName("systemJunkFrame");

    m_appItem = new TrashCleanConfigItem(this);
    m_appItem->setIconPath(kAppIconPath);
    m_appItem->setTitles(tr("Application junk"), tr("Unneeded files created by applications"));
    m_appItem->setIndex(APP_CONFIG_INDEX);
    m_appItem->setAccessibleName("appJunkFrame");

    m_hisItem = new TrashCleanConfigItem(this);
    m_hisItem->setIconPath(kHisIconPath);
    m_hisItem->setTitles(tr("Traces"), tr("Activity traces of the system and applications"));
    m_hisItem->setIndex(HISTORY_CONFIG_INDEX);
    m_hisItem->setAccessibleName("tracesFrame");

    m_cookiesItem = new TrashCleanConfigItem(this);
    m_cookiesItem->setIconPath(kCookiesIconPath);
    m_cookiesItem->setTitles(tr("Cookies"), tr("Cookies from Internet browsers"));
    m_cookiesItem->setIndex(COOKIES_CONFIG_INDEX);
    m_cookiesItem->setAccessibleName("cookiesFrame");

    m_scanBtn = new DSuggestButton(this);
    m_scanBtn->setAccessibleName("scanButton");
    DFontSizeManager::instance()->unbind(m_scanBtn);
    QFont scanBtnFont = m_scanBtn->font();
    scanBtnFont.setPixelSize(14);
    m_scanBtn->setFont(scanBtnFont);
    m_scanBtn->setFixedSize(150, 36);
    m_scanBtn->setText(tr("Scan Now", "button"));
    m_scanBtn->setFocusPolicy(Qt::NoFocus);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->setColumnStretch(0, 107);
    mainLayout->setColumnMinimumWidth(0, 107);
    mainLayout->setColumnStretch(1, 516);
    mainLayout->setColumnMinimumWidth(1, 516);
    mainLayout->setColumnStretch(2, 147);
    mainLayout->setColumnMinimumWidth(2, 147);

    mainLayout->setRowMinimumHeight(0, 230);
    mainLayout->addWidget(m_logo, 0, 0, 1, -1, Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->setRowMinimumHeight(1, 54);
    mainLayout->addWidget(m_title, 1, 0, 1, -1, Qt::AlignCenter);
    mainLayout->setRowMinimumHeight(2, 24);
    mainLayout->addWidget(m_greeting, 2, 0, 1, -1, Qt::AlignCenter);
    mainLayout->setRowMinimumHeight(3, 22);

    mainLayout->setRowMinimumHeight(4, 42);
    mainLayout->setRowMinimumHeight(5, 42);
    mainLayout->setRowMinimumHeight(6, 42);
    mainLayout->setRowMinimumHeight(7, 42);
    mainLayout->addWidget(m_sysItem, 4, 1);
    mainLayout->addWidget(m_appItem, 5, 1);
    mainLayout->addWidget(m_hisItem, 6, 1);
    mainLayout->addWidget(m_cookiesItem, 7, 1);

    mainLayout->setRowMinimumHeight(8, 88);
    mainLayout->addWidget(m_scanBtn, 8, 0, 1, -1, Qt::AlignCenter);

    mainLayout->setRowMinimumHeight(9, 4);
}

void TrashCleanGreetingWidget::initConnection()
{
    connect(m_sysItem,
            &TrashCleanConfigItem::stageChanged,
            this,
            &TrashCleanGreetingWidget::onItemStageChanged);
    connect(m_appItem,
            &TrashCleanConfigItem::stageChanged,
            this,
            &TrashCleanGreetingWidget::onItemStageChanged);
    connect(m_hisItem,
            &TrashCleanConfigItem::stageChanged,
            this,
            &TrashCleanGreetingWidget::onItemStageChanged);
    connect(m_cookiesItem,
            &TrashCleanConfigItem::stageChanged,
            this,
            &TrashCleanGreetingWidget::onItemStageChanged);

    connect(m_scanBtn, &QPushButton::clicked, this, &TrashCleanGreetingWidget::notifyStartScan);
}

// 随主题变化，改变图标
void TrashCleanGreetingWidget::setPixmapByTheme(DGuiApplicationHelper::ColorType themeType)
{
    // 判断当前主题类型
    if (themeType == DGuiApplicationHelper::ColorType::LightType) {
        m_sysItem->setIconPath(kSysIconPath);
        m_appItem->setIconPath(kAppIconPath);
        m_hisItem->setIconPath(kHisIconPath);
        m_cookiesItem->setIconPath(kCookiesIconPath);
    } else {
        m_sysItem->setIconPath(kSysIconPathDark);
        m_appItem->setIconPath(kAppIconPathDark);
        m_hisItem->setIconPath(kHisIconPathDark);
        m_cookiesItem->setIconPath(kCookiesIconPathDark);
    }
}
