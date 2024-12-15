// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DDialog>
#include <DSwitchButton>
#include <DFontSizeManager>
#include <DLabel>

#include <QUrl>
#include <QSpacerItem>
#include <QPushButton>
#include <QDesktopServices>

#include "../core/dbusworker.h"
#include "storageresultwidget.h"
#include "backgroundframe.h"
#include "baselabel.h"

#define ICON_SIZE 15
#define RESULT_TEXT_SIZE 9
#define RIGHT_SPACE 35
#define RIGHT_SPACE_WITH_CLEANUP 2

StorageResultWidget::StorageResultWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QHBoxLayout(this))
    , m_leftLayout(new QVBoxLayout(this))
    , m_resultLayout(new QVBoxLayout(this))
    , m_systemResultLayout(new QHBoxLayout(this))
    , m_dataResultLayout(new QHBoxLayout(this))
    , m_tipLayout(new QHBoxLayout(this))
    , m_checkResultIconLabel(new IconLabel(this))
    , m_requirementTitleLabel(new DLabel(this))
    , m_requirementTipLabel(new SimpleLabel(this))
    , m_systemResultTextLabel(new DLabel(this))
    , m_dataResultTextLabel(new DLabel(this))
    , m_systemStatusLabel(new DLabel(this))
    , m_dataStatusLabel(new DLabel(this))
    , m_cleanupSystemLabel(new DLabel(QString("<a href=\"dummy_url\" style=\"text-decoration: none;\">%1</a>").arg(tr("Free up")), this))
    , m_cleanupDataLabel(new DLabel(QString("<a href=\"dummy_url\" style=\"text-decoration: none;\">%1</a>").arg(tr("Free up")), this))
    , m_cleanupSystemSpacerItem(new QSpacerItem(RIGHT_SPACE, 0, QSizePolicy::Maximum))
    , m_cleanupDataSpacerItem(new QSpacerItem(RIGHT_SPACE, 0, QSizePolicy::Maximum))
{
    initUI();
    initConnections();
}

void StorageResultWidget::initUI()
{
    QColor titleColor("#414D68");

    m_requirementTitleLabel->setForegroundRole(DPalette::TextTitle);
    DFontSizeManager::instance()->bind(m_requirementTitleLabel, DFontSizeManager::T6, QFont::Medium);

    m_requirementTipLabel->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_requirementTipLabel, DFontSizeManager::T8, QFont::Thin);

    DFontSizeManager::instance()->bind(m_systemResultTextLabel, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_dataResultTextLabel, DFontSizeManager::T8, QFont::Normal);

    m_systemResultLayout->addSpacerItem(new QSpacerItem(114514, 0, QSizePolicy::Maximum));
    m_systemResultLayout->addWidget(m_systemResultTextLabel);
    m_systemResultLayout->addSpacing(8);
    m_systemResultLayout->addWidget(m_systemStatusLabel, 0, Qt::AlignRight);
    m_systemResultLayout->addSpacing(3);
    m_systemResultLayout->addWidget(m_cleanupSystemLabel, 0, Qt::AlignRight);
    m_systemResultLayout->addSpacerItem(m_cleanupSystemSpacerItem);

    m_dataResultLayout->addSpacerItem(new QSpacerItem(114514, 0, QSizePolicy::Maximum));
    m_dataResultLayout->addWidget(m_dataResultTextLabel);
    m_dataResultLayout->addSpacing(8);
    m_dataResultLayout->addWidget(m_dataStatusLabel, 0, Qt::AlignRight);
    m_dataResultLayout->addSpacing(3);
    m_dataResultLayout->addWidget(m_cleanupDataLabel, 0, Qt::AlignRight);
    m_dataResultLayout->addSpacerItem(m_cleanupDataSpacerItem);

    m_resultLayout->setContentsMargins(0, 0, 0, 0);
    m_resultLayout->addLayout(m_dataResultLayout);
    m_resultLayout->addLayout(m_systemResultLayout);
    m_resultLayout->addSpacing(114514);

    m_checkResultIconLabel->addSpacing(8);
    m_checkResultIconLabel->addWidget(m_requirementTitleLabel);
    m_checkResultIconLabel->addSpacing(60);
    m_checkResultIconLabel->layout()->setContentsMargins(0, 0, 0, 0);

    m_tipLayout->addSpacing(26);
    m_tipLayout->addWidget(m_requirementTipLabel);
    m_tipLayout->setContentsMargins(0, 0, 0, 0);

    m_leftLayout->addWidget(m_checkResultIconLabel);
    m_leftLayout->addLayout(m_tipLayout);
    m_leftLayout->addSpacing(114514);
    m_leftLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addLayout(m_leftLayout);
    m_mainLayout->addLayout(m_resultLayout);
    setLayout(m_mainLayout);
}

void StorageResultWidget::initConnections()
{
    connect(m_cleanupSystemLabel, &DLabel::linkActivated, this, &StorageResultWidget::openCleanupDialog);
    connect(m_cleanupDataLabel, &DLabel::linkActivated, this, &StorageResultWidget::openCleanupDialog);
}

void StorageResultWidget::setIcon(QString iconPath)
{
    m_checkResultIconLabel->setIcon(iconPath);
}

void StorageResultWidget::setIcon(QString iconPath, int w, int h)
{
    m_checkResultIconLabel->setIcon(iconPath, w, h);
}

void StorageResultWidget::setStatus(CheckResultType status, DLabel *statusLabel, DLabel *resultTextLabel)
{
    switch(status)
    {
    case CheckResultType::PASSED:
    {
        statusLabel->setToolTip(QString(""));
        resultTextLabel->setForegroundRole(DPalette::TextTitle);
    }
    break;
    case CheckResultType::FAILED:
    {
        resultTextLabel->setForegroundRole(DPalette::TextWarning);
    }
    break;
    default:
    {
        statusLabel->setToolTip(m_toolTipText);
        resultTextLabel->setForegroundRole(DPalette::TextWarning);
    }
    }
    statusLabel->setPixmap(getIcon(status).pixmap(30, 30).scaled(ICON_SIZE, ICON_SIZE));
}

void StorageResultWidget::setDataStatus(CheckResultType status) {
    m_dataStatus = status;
    if (status == CheckResultType::FAILED)
    {
        m_cleanupDataLabel->setVisible(true);
        m_cleanupDataSpacerItem->changeSize(RIGHT_SPACE_WITH_CLEANUP, 0);
        m_mainLayout->invalidate();
    }
    else
    {
        m_cleanupDataLabel->setVisible(false);
        m_cleanupDataSpacerItem->changeSize(RIGHT_SPACE, 0);
        m_mainLayout->invalidate();
    }
    setStatus(status, m_dataStatusLabel, m_dataResultTextLabel);
}

void StorageResultWidget::setSystemStatus(CheckResultType status) {
    m_systemStatus = status;
    if (status == CheckResultType::FAILED && m_dataStatus == CheckResultType::PASSED)
    {
        m_cleanupSystemLabel->setVisible(true);
        m_cleanupSystemSpacerItem->changeSize(RIGHT_SPACE_WITH_CLEANUP, 0);
        m_mainLayout->invalidate();
    }
    else
    {
        m_cleanupSystemLabel->setVisible(false);
        m_cleanupSystemSpacerItem->changeSize(RIGHT_SPACE, 0);
        m_mainLayout->invalidate();
    }
    setStatus(status, m_systemStatusLabel, m_systemResultTextLabel);
}

void StorageResultWidget::setSystemResult(QString text) {
    m_systemResultTextLabel->setText(text);
}

void StorageResultWidget::setDataResult(QString text) {
    m_dataResultTextLabel->setText(text);
}

void StorageResultWidget::setTitle(QString text)
{
    m_requirementTitleLabel->setText(text);
}

void StorageResultWidget::setToolTip(QString text)
{
    m_toolTipText = text;
}

void StorageResultWidget::setRequirement(QString text)
{
    m_requirementTipLabel->setText(text);
}


void StorageResultWidget::scale(int w, int h)
{
    m_checkResultIconLabel->scale(w, h);
}

void StorageResultWidget::addSpacerItem(QSpacerItem *item)
{
    m_checkResultIconLabel->addSpacerItem(item);
}

void StorageResultWidget::addWidget(QWidget *widget)
{
    m_checkResultIconLabel->addWidget(widget);
}

void StorageResultWidget::openCleanupDialog()
{
    DBusWorker *dbusWorker = DBusWorker::getInstance();
    DDialog dlg(this);
    const QMap<QString, QString> resultMap = dbusWorker->getCheckResultMap();
    double dataBase = resultMap["database"].toDouble();
    double dataFree = resultMap["datafree"].toDouble();
    double systemBase = resultMap["systembase"].toDouble();
    double systemFree = resultMap["systemfree"].toDouble();

    qDebug() << "dataBase:" << dataBase;
    qDebug() << "dataFree:" << dataFree;
    qDebug() << "systemBase:" << systemBase;
    qDebug() << "systemFree:" << systemFree;

    DLabel *titleLabel = new DLabel(tr("Your system cannot be upgraded to V23"));
    titleLabel->setForegroundRole(DPalette::BrightText);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T6, QFont::Normal);
    dlg.addButton(tr("Cancel"));
    dlg.addButton(tr("Confirm"), false, DDialog::ButtonType::ButtonRecommend);

    BackgroundFrame *frameDoSystemCleanup = new BackgroundFrame;
    QHBoxLayout *doSystemCleanupLayout = new QHBoxLayout;
    doSystemCleanupLayout->setContentsMargins(10, 10, 10, 10);
    DLabel *doSystemCleanupTextLabel = new DLabel(tr("Clean system disk"));
    DFontSizeManager::instance()->bind(doSystemCleanupTextLabel,DFontSizeManager::T6, QFont::Medium);
    doSystemCleanupTextLabel->setForegroundRole(DPalette::TextTitle);
    DLabel *doSystemCleanupTipLabel = new DLabel(tr("%1 GB free space required").arg(systemBase - systemFree));
    DFontSizeManager::instance()->bind(doSystemCleanupTipLabel, DFontSizeManager::T9, QFont::Light);

    QVBoxLayout *systemLabelLayout= new QVBoxLayout;
    systemLabelLayout -> addWidget(doSystemCleanupTextLabel);
    systemLabelLayout -> addWidget(doSystemCleanupTipLabel);

    QPushButton *doSystemCleanupButton = new QPushButton(tr("Free Up"));
    doSystemCleanupButton->setFixedSize(74, 36);
    doSystemCleanupLayout->addLayout(systemLabelLayout);
    doSystemCleanupLayout->addSpacing(0);
    doSystemCleanupLayout->addWidget(doSystemCleanupButton);
    frameDoSystemCleanup->setLayout(doSystemCleanupLayout);

    BackgroundFrame *frameDoDataCleanup = new BackgroundFrame;
    QHBoxLayout *doDataCleanupLayout = new QHBoxLayout;
    doDataCleanupLayout->setContentsMargins(10, 10, 10, 10);

    DLabel *doDataCleanupTextLabel = new DLabel(tr("Clean data disk"));
    DFontSizeManager::instance()->bind(doDataCleanupTextLabel,DFontSizeManager::T6, QFont::Medium);
    doDataCleanupTextLabel->setForegroundRole(DPalette::TextTitle);

    DLabel *doDataCleanupTipLabel = new DLabel(tr("%1 GB free space required").arg(dataBase - dataFree));
    DFontSizeManager::instance()->bind(doDataCleanupTipLabel, DFontSizeManager::T9, QFont::Light);

    QVBoxLayout *dataLabelLayout= new QVBoxLayout;
    dataLabelLayout -> addWidget(doDataCleanupTextLabel);
    dataLabelLayout -> addWidget(doDataCleanupTipLabel);

    QPushButton *doDataCleanupButton = new QPushButton(tr("Free Up"));
    doDataCleanupButton->setFixedSize(74, 36);
    doDataCleanupLayout->addLayout(dataLabelLayout);
    doDataCleanupLayout->addSpacing(0);
    doDataCleanupLayout->addWidget(doDataCleanupButton);
    frameDoDataCleanup->setLayout(doDataCleanupLayout);

    // Setting actions for clean up buttons
    connect(doSystemCleanupButton, &QPushButton::clicked, [] {
        QDesktopServices::openUrl(QUrl::fromLocalFile("/"));
    });
    connect(doDataCleanupButton, &QPushButton::clicked, [] {
        QDesktopServices::openUrl(QUrl::fromLocalFile("/home"));
    });

    // Add layout for space cleanup dialog
    dlg.addContent(titleLabel, Qt::AlignCenter);
    dlg.addSpacing(24);
    if (m_systemStatus != CheckResultType::PASSED) {
        dlg.addContent(frameDoSystemCleanup);
        dlg.addSpacing(4);
    }
    if (m_dataStatus != CheckResultType::PASSED) {
        dlg.addContent(frameDoDataCleanup);
        dlg.addSpacing(4);
    }

    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    if (dlg.exec() == DDialog::Accepted)
    {
        emit dbusWorker->StartUpgradeCheck();
    }
}
