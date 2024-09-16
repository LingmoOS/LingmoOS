// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DFontSizeManager>

#include <QColor>
#include <QFile>
#include <QFont>
#include <QIcon>

#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "../../widgets/statusicon.h"
#include "checkimageresultwidget.h"

#define ICON_SIZE 128

CheckImageResultWidget::CheckImageResultWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_frameLayout(new QHBoxLayout())
    , m_backgroundFrame(new BackgroundFrame(this, 18))
    , m_iconLabel(new IconLabel(this, Orientation::Vertical))
    , m_resultLabel(new SimpleLabel)
    , m_imageSizeTitleLabel(new SimpleLabel)
    , m_imageSizeLabel(new SimpleLabel)
    , m_imageVersionTitleLabel(new SimpleLabel)
    , m_imageVersionLabel(new SimpleLabel)
    , m_appTipLabel(new SimpleLabel)
{
    initUI();
    initConnections();
}

void CheckImageResultWidget::initUI()
{
    DFontSizeManager::instance()->bind(m_resultLabel, DFontSizeManager::T4, QFont::Bold);
    m_resultLabel->setForegroundRole(DPalette::TextTitle);

    DFontSizeManager::instance()->bind(m_imageSizeTitleLabel, DFontSizeManager::T6, QFont::Medium);
    m_imageSizeTitleLabel->setForegroundRole(DPalette::Text);

    DFontSizeManager::instance()->bind(m_imageSizeLabel, DFontSizeManager::T8, QFont::Normal);
    DFontSizeManager::instance()->bind(m_imageVersionTitleLabel, DFontSizeManager::T6, QFont::Medium);
    m_imageVersionTitleLabel->setForegroundRole(DPalette::Text);

    DFontSizeManager::instance()->bind(m_imageVersionLabel, DFontSizeManager::T8, QFont::Normal);

    m_imageSizeTitleLabel->setText(tr("File size:"));
    m_imageVersionTitleLabel->setText(tr("ISO version:"));

    m_imageSizeLabel->setColor(QColor("#526a7f"));
    m_imageVersionLabel->setColor(QColor("#526a7f"));

    DFontSizeManager::instance()->bind(m_appTipLabel, DFontSizeManager::T8, QFont::Normal);
    m_appTipLabel->setForegroundRole(DPalette::TextTitle);

    m_frameLayout->addSpacing(20);
    m_frameLayout->addWidget(m_imageSizeTitleLabel);
    m_frameLayout->addWidget(m_imageSizeLabel);
    m_frameLayout->addSpacing(6);
    m_frameLayout->addWidget(new QLabel("/"));
    m_frameLayout->addSpacing(6);
    m_frameLayout->addWidget(m_imageVersionTitleLabel);
    m_frameLayout->addWidget(m_imageVersionLabel);
    m_frameLayout->addSpacing(20);
    m_backgroundFrame->setLayout(m_frameLayout);

    m_iconLabel->addSpacing(10);
    m_iconLabel->addWidget(m_resultLabel);
    m_iconLabel->addSpacing(16);
    m_iconLabel->addWidget(m_backgroundFrame);
    m_iconLabel->addSpacing(30);
    m_iconLabel->addWidget(m_appTipLabel);
    m_iconLabel->addSpacing(98);

    m_mainLayout->setContentsMargins(0, 30, 0, 0);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    setLayout(m_mainLayout);
}

void CheckImageResultWidget::initConnections()
{
    connect(this, SIGNAL(SetResult(bool)), this, SLOT(onSetResult(bool)));
    connect(this, &CheckImageResultWidget::ReadImageInfo, this, [this](QString filepath) {
        QFile file(filepath);
        m_resultIsoPath = filepath;
        m_imageSizeLabel->setText(tr("%1 GB").arg(file.size() / 1024. / 1024. / 1024., 0, 'f', 2));
        m_imageVersionLabel->setText(tr("Checking"));
        m_imageVersionLabel->setText("V" + DBusWorker::getInstance()->GetISOVersion());
    });
}

void CheckImageResultWidget::onSetResult(bool passed)
{
    m_result = passed;
    if (passed)
    {
        m_iconLabel->setIcon(QString(":/icons/icon_success.svg"), ICON_SIZE, ICON_SIZE);
        m_resultLabel->setText(tr("The image file is ready"));
        m_appTipLabel->setText(tr("Some applications may not work properly after the upgrade."));

        m_resultLabel->setVisible(true);
        m_appTipLabel->setVisible(true);
        m_backgroundFrame->setVisible(true);
    }
    else
    {
        m_iconLabel->setIcon(QString(":/icons/icon_fail.svg"), ICON_SIZE, ICON_SIZE);
        m_resultLabel->setText(tr("The version or integrity of the image file does not meet requirements."));
        m_resultLabel->setVisible(true);
        m_appTipLabel->setVisible(false);
        m_backgroundFrame->setVisible(false);
    }
}
