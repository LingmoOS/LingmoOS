// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkprocessingwidget.h"

#include <DFontSizeManager>
#include <DPaletteHelper>

#include <QFont>
#include <QGridLayout>

#define PROGRESS_MAX 100
#define PROGRESS_MIN 0
#define TIME_FORMAT "hh:mm:ss"

DWIDGET_USE_NAMESPACE;

#define FIXED_ROW(row, height)                        \
    {                                                 \
        mainLayout->setRowStretch(row, height);       \
        mainLayout->setRowMinimumHeight(row, height); \
    }

#define FIXED_COL(column, weight)                          \
    {                                                      \
        mainLayout->setColumnStretch(column, weight);      \
        mainLayout->setColumnMinimumWidth(column, weight); \
    }

#define SET_ACC_NAME(control, name) SET_ACCESS_NAME_T(control, checkProgressingWidget, name)

CheckProcessingWidget::CheckProcessingWidget(QWidget *parent)
    : DFrame(parent)
    , m_movieLable(nullptr)
    , m_stageLable(nullptr)
    , m_infoLable(nullptr)
    , m_progressBar(nullptr)
    , m_timeLable(nullptr)
    , m_progressValue(PROGRESS_MIN)
{
    this->setFixedSize(770, 590);
    this->setContentsMargins(0, 10, 0, 10);
    this->setFocusPolicy(Qt::NoFocus);
    this->setLineWidth(0);

    initUI();
    initConnection();

    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &CheckProcessingWidget::ticktock);

    // 唯一窗口实例
    this->setObjectName("checkProgressingWidget");
    this->setAccessibleName("checkProgressingWidget");
}

void CheckProcessingWidget::onCheckMissionCompleted(int point)
{
    int current = m_progressBar->value();
    current += point;
    m_progressBar->setValue(current < PROGRESS_MAX ? current : PROGRESS_MAX);
}

void CheckProcessingWidget::onCheckMissionStarted(SysCheckItemID id)
{
    if (SysCheckItemID::SSH == id) {
        m_stageLable->setText(kStageinfo3);
    } else if (SysCheckItemID::Disk == id) {
        m_stageLable->setText(kStageinfo4);
    } else if (SysCheckItemID::Trash == id) {
        m_stageLable->setText(kStageinfo5);
    } else if (SysCheckItemID::SystemUpdate == id) {
        m_stageLable->setText(kStageinfo6);
    } else if (SysCheckItemID::DevMode == id) {
        m_stageLable->setText(kStageinfo7);
    } else if (SysCheckItemID::AutoStartApp == id) {
        m_stageLable->setText(kStageinfo8);
    } else if (SysCheckItemID::MaxLimit == id) {
        m_stageLable->setText(kStageinfo8);
    }
    m_progressBar->setValue(m_progressValue);
    m_progressValue += PROGRESS_MAX / (int(SysCheckItemID::MaxLimit) - 2);
}

// 检查初始化
void CheckProcessingWidget::onCheckInitial()
{
    m_startTime = QTime::currentTime();
    m_progressBar->setValue(PROGRESS_MIN);
    m_stageLable->setText("");
    m_timeLable->setText(QTime(0, 0, 0, 0).toString(TIME_FORMAT));

    m_infoLable->setText(kProcessingText);

    m_timer.start();

    m_progressValue = PROGRESS_MIN;

    // 在这里通过modal开始检查
    Q_EMIT checkPrepared();
}

void CheckProcessingWidget::onCheckDone()
{
    m_timer.stop();

    m_progressBar->setValue(PROGRESS_MAX);
    // 通知上层显示结果页
    Q_EMIT checkDone();
}

void CheckProcessingWidget::ticktock()
{
    QTime now = QTime::currentTime();
    QTime info(0, 0, 0, 0);
    info = info.addSecs(m_startTime.secsTo(now));
    m_timeLable->setText(info.toString(TIME_FORMAT));

    QString infoText = m_infoLable->text();
    infoText.append(".");
    infoText.remove("....");
    m_infoLable->setText(infoText);
}

void CheckProcessingWidget::initUI()
{
    m_movieLable = new DLabel(this);
    m_movieLable->setFixedSize(340, 340);
    m_movieLable->setAlignment(Qt::AlignCenter);
    m_movieLable->setPixmap(QIcon::fromTheme(k1stPageLogoPath).pixmap(320, 320));
    SET_ACC_NAME(m_movieLable, movieLable);

    m_stageLable = new DLabel(this);
    SET_ACC_NAME(m_stageLable, stageLable);
    FIXED_FONT_PIXEL_SIZE(m_stageLable, 16);

    m_infoLable = new DLabel(this);
    SET_ACC_NAME(m_infoLable, infoLable);
    FIXED_FONT_PIXEL_SIZE(m_infoLable, 30);

    m_progressBar = new DProgressBar(this);
    SET_ACC_NAME(m_progressBar, progressBar);
    m_progressBar->setRange(PROGRESS_MIN, PROGRESS_MAX);
    m_progressBar->setValue(50);
    m_progressBar->setFixedSize(573, 12);
    m_progressBar->setTextVisible(false);

    m_cancelButton = new DPushButton(this);
    SET_ACC_NAME(m_cancelButton, cancelButton);
    m_cancelButton->setFixedSize(22, 22);
    m_cancelButton->setFlat(true);
    m_cancelButton->setIcon(QIcon::fromTheme(kProgressCancelIconPath));
    m_cancelButton->setIconSize(QSize(22, 22));

    m_timeLable = new DTipLabel("", this);
    SET_ACC_NAME(m_timeLable, timeLable);
    m_timeLable->setText(QTime(0, 0, 0, 0).toString(TIME_FORMAT));
    DFontSizeManager::instance()->unbind(m_timeLable);
    FIXED_FONT_PIXEL_SIZE(m_timeLable, 12);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    FIXED_COL(0, 87);
    FIXED_COL(1, 573);
    FIXED_COL(2, 0);
    FIXED_COL(3, 78);

    FIXED_ROW(0, 340); // logo
    mainLayout->addWidget(m_movieLable, 0, 0, 1, -1, Qt::AlignBottom | Qt::AlignHCenter);
    FIXED_ROW(1, 24); // stage
    mainLayout->addWidget(m_stageLable, 1, 0, 1, -1, Qt::AlignHCenter | Qt::AlignBottom);
    FIXED_ROW(2, 54); // info
    mainLayout->addWidget(m_infoLable, 2, 0, 1, -1, Qt::AlignHCenter | Qt::AlignBottom);
    FIXED_ROW(3, 20); // space
    FIXED_ROW(4, 22); // progressbar
    mainLayout->addWidget(m_progressBar, 4, 1, 1, 1, Qt::AlignVCenter);
    mainLayout->addWidget(m_cancelButton, 4, 3, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);
    FIXED_ROW(5, 27); // time
    mainLayout->addWidget(m_timeLable, 5, 0, 1, -1, Qt::AlignCenter);
    FIXED_ROW(6, 78); // space

    this->setLayout(mainLayout);
}

void CheckProcessingWidget::initConnection()
{
    connect(m_cancelButton, &DPushButton::clicked, this, [this] {
        Q_EMIT checkCanceled();
    });
}
