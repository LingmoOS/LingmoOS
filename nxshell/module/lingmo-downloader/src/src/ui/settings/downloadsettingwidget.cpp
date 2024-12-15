// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file downloadsettingwidget.cpp
 *
 * @brief 下载设置窗口，主要实现全速下载和限速下载的切换，以及限速下载时限制参数的设置
 *
 *@date 2020-06-09 11:03
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "downloadsettingwidget.h"
#include "settinginfoinputwidget.h"

#include <DFontSizeManager>
#include <DPalette>
#include <DGuiApplicationHelper>
#include <DHorizontalLine>
#include <DBackgroundGroup>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>



DownloadSettingWidget::DownloadSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
}

// 初始化界面
void DownloadSettingWidget::initUI()
{
    m_DownloadSpeedLimitValue = 102400;
    m_UploadSpeedLimitValue = 32;

    m_fullSpeedDownloadButton = new DRadioButton(tr("Full speed")); // 全速下载
    m_fullSpeedDownloadButton->setObjectName("fullSpeedBtn");
    m_fullSpeedDownloadButton->setAccessibleName("fullSpeedBtn");
    m_fullSpeedDownloadButton->setToolTip(tr("Downloaded files will be uploaded through P2P, "
                                             "\nwhich could help other users speed up the downloading, "
                                             "\nand it does not involve user privacy."));
    //    m_fullSpeedDownloadButton->toolTip()
    //    DLabel *pFullSpeedLabel = new DLabel;
    //    // 下载文件会通过P2P的方式上传分享，帮助其他网友加速下载，不会涉及任何用户隐私。
    //    pFullSpeedLabel->setText(tr("Downloaded files will be uploaded through P2P, \nwhich could help other users speed up the downloading, \ndoes not involve the privacy."));
    //    pFullSpeedLabel->setWordWrap(true);
    m_speedLimitDownloadButton = new DRadioButton(tr("Limited speed")); // 限速下载
    m_speedLimitDownloadButton->setObjectName("speedLimitBtn");
    m_speedLimitDownloadButton->setAccessibleName("speedLimitBtn");
    m_speedLimitDownloadButton->setChecked(true);
    m_maxDownloadSpeedLimit = new SettingInfoInputWidget;
    m_maxDownloadSpeedLimit->setAccessibleName("maxDownloadSpeed");
    m_maxUploadSpeedLimit = new SettingInfoInputWidget;
    m_maxUploadSpeedLimit->setAccessibleName("maxUploadSpeed");

    m_maxDownloadSpeedLimit->setWidgetWidth(390);
    m_maxUploadSpeedLimit->setWidgetWidth(390);

    QFont font;
    font.setPointSize(DFontSizeManager::T8);
    font.setWeight(QFont::Normal);

    DPalette palette;
    palette.setBrush(DPalette::TextTips, DGuiApplicationHelper::instance()->applicationPalette().textTips());

    m_maxDownloadSpeedLimit->setRangeLabelFont(font);
    m_maxDownloadSpeedLimit->setRangeLabelPalette(palette);
    m_maxUploadSpeedLimit->setRangeLabelFont(font);
    m_maxUploadSpeedLimit->setRangeLabelPalette(palette);

    m_downloadAlertControl = new DTK_WIDGET_NAMESPACE::DAlertControl(m_maxDownloadSpeedLimit->getLineEdit(),
                                                                     m_maxDownloadSpeedLimit->getLineEdit());
    m_uploadAlertControl = new DTK_WIDGET_NAMESPACE::DAlertControl(m_maxUploadSpeedLimit->getLineEdit(), m_maxUploadSpeedLimit->getLineEdit());

    //    QHBoxLayout *pFullSpeedLabelLayout = new QHBoxLayout;
    //    pFullSpeedLabelLayout->addWidget(pFullSpeedLabel);
    //    pFullSpeedLabelLayout->addStretch();
    //    pFullSpeedLabelLayout->setContentsMargins(28, 0, 0, 0);

    //    QVBoxLayout *pVFullSpeedLayout = new QVBoxLayout;
    //    pVFullSpeedLayout->addWidget(m_fullSpeedDownloadButton);
    //    pVFullSpeedLayout->addSpacing(3);
    //    pVFullSpeedLayout->addLayout(pFullSpeedLabelLayout);
    //    pVFullSpeedLayout->setSpacing(0);
    //    pVFullSpeedLayout->setContentsMargins(0, 0, 0, 0);

    m_maxDownloadSpeedLimit->setTitleLabelText(tr("Max. download speed")); // 最大下载限速
    m_maxDownloadSpeedLimit->setUnitLabelText(tr("KB/s"));
    m_maxDownloadSpeedLimit->setRangeLabelText("(100-102400)");
    m_maxDownloadSpeedLimit->setLineEditText("102400");
    m_maxDownloadSpeedLimit->setValid(100, 102400);

    m_maxUploadSpeedLimit->setTitleLabelText(tr("Max. upload speed")); // 最大上传限速
    m_maxUploadSpeedLimit->setUnitLabelText(tr("KB/s"));
    m_maxUploadSpeedLimit->setRangeLabelText("(16-5120)");
    m_maxUploadSpeedLimit->setLineEditText("32");
    m_maxUploadSpeedLimit->setValid(16, 5120);

    DLabel *leftLabel = new DLabel(tr("Limit period")); // 限速时段
    DLabel *centerLabel = new DLabel(tr("to")); // 至
    m_startTimeEdit = new CTimeEdit(this);
    m_startTimeEdit->setTime(QTime(7,00,00));
    m_startTimeEdit->setAccessibleName("startTimeEdit");
    m_endTimeEdit = new CTimeEdit(this);
    m_endTimeEdit->setAccessibleName("endTTimeEdit");
    m_endTimeEdit->setTime(QTime(18,00,00));
    m_startTimeAlertControl = new DTK_WIDGET_NAMESPACE::DAlertControl(m_startTimeEdit->dLineEdit(), m_startTimeEdit->dLineEdit());
    m_endTimeAlertControl = new DTK_WIDGET_NAMESPACE::DAlertControl(m_endTimeEdit->dLineEdit(), m_endTimeEdit->dLineEdit());
    //m_startTimeEdit->setDisplayFormat("h:mm");
    m_startTimeEdit->setMinimumWidth(160);
    //m_endTimeEdit->setDisplayFormat("h:mm");
    m_endTimeEdit->setMinimumWidth(160);

    QHBoxLayout *timeLayout = new QHBoxLayout;
    timeLayout->addWidget(leftLabel);
    //    pTimeLayout->addSpacing(6);
    timeLayout->addWidget(m_startTimeEdit);
    //    pTimeLayout->addSpacing(1);
    timeLayout->addWidget(centerLabel);
    //    pTimeLayout->addSpacing(1);
    timeLayout->addWidget(m_endTimeEdit);
    //    pTimeLayout->setSpacing(0);
    timeLayout->addStretch();
    timeLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *speedLimitDownloadLayout = new QVBoxLayout;
    speedLimitDownloadLayout->addWidget(m_maxDownloadSpeedLimit);
    speedLimitDownloadLayout->addWidget(m_maxUploadSpeedLimit);
    speedLimitDownloadLayout->addLayout(timeLayout);
    speedLimitDownloadLayout->setContentsMargins(28, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_fullSpeedDownloadButton);
    mainLayout->addWidget(m_speedLimitDownloadButton);
    mainLayout->addLayout(speedLimitDownloadLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
}

// 初始化链接
void DownloadSettingWidget::initConnections()
{
    connect(m_fullSpeedDownloadButton, &DRadioButton::clicked, this, &DownloadSettingWidget::onRadioButtonClicked);
    connect(m_speedLimitDownloadButton, &DRadioButton::clicked, this, &DownloadSettingWidget::onRadioButtonClicked);
    connect(m_maxDownloadSpeedLimit, &SettingInfoInputWidget::textChanged, this, &DownloadSettingWidget::onTextChanged);
    connect(m_maxUploadSpeedLimit, &SettingInfoInputWidget::textChanged, this, &DownloadSettingWidget::onTextChanged);
    connect(m_startTimeEdit, &CTimeEdit::inputFinished, this, &DownloadSettingWidget::onTimeChanged);
    connect(m_endTimeEdit, &CTimeEdit::inputFinished, this, &DownloadSettingWidget::onTimeChanged);
    connect(m_maxDownloadSpeedLimit, &SettingInfoInputWidget::focusChanged, this, &DownloadSettingWidget::onFocusChanged);
    connect(m_maxUploadSpeedLimit, &SettingInfoInputWidget::focusChanged, this, &DownloadSettingWidget::onFocusChanged);
}

void DownloadSettingWidget::onRadioButtonClicked()
{
    DRadioButton *radioButton = qobject_cast<DRadioButton *>(sender());

    if (m_fullSpeedDownloadButton == radioButton) {
        m_fullSpeedDownloadButton->setChecked(true);
        m_speedLimitDownloadButton->setChecked(false);
        m_maxDownloadSpeedLimit->setLineEditIsDisabled(true);
        m_maxUploadSpeedLimit->setLineEditIsDisabled(true);
        m_startTimeEdit->setDisabled(true);
        m_endTimeEdit->setDisabled(true);

        QString text = QString("fullspeed;%1;%2;%3;%4")
                           .arg(m_maxDownloadSpeedLimit->getLineEditText().toInt())
                           .arg(m_maxUploadSpeedLimit->getLineEditText().toInt())
                           .arg(m_startTimeEdit->getTime().toString("hh:mm"))
                           .arg(m_endTimeEdit->getTime().toString("hh:mm"));

        emit speedLimitInfoChanged(text);
    } else if (m_speedLimitDownloadButton == radioButton) {
        m_fullSpeedDownloadButton->setChecked(false);
        m_speedLimitDownloadButton->setChecked(true);
        m_maxDownloadSpeedLimit->setLineEditIsDisabled(false);
        m_maxUploadSpeedLimit->setLineEditIsDisabled(false);
        m_startTimeEdit->setDisabled(false);
        m_endTimeEdit->setDisabled(false);

        QString text = QString("speedlimit;%1;%2;%3;%4")
                           .arg(m_maxDownloadSpeedLimit->getLineEditText().toInt())
                           .arg(m_maxUploadSpeedLimit->getLineEditText().toInt())
                           .arg(m_startTimeEdit->getTime().toString("hh:mm"))
                           .arg(m_endTimeEdit->getTime().toString("hh:mm"));

        emit speedLimitInfoChanged(text);
    }
}

void DownloadSettingWidget::onValueChanged(QVariant var){
    if (!var.toString().isEmpty()) {
        QString currentValue = var.toString();
        int currentSelect = 2;
        QString maxDownloadSpeedLimit;
        QString maxUploadSpeedLimit;
        QString startTime;
        QString endTime;

        if (currentValue.isEmpty()) {
            maxDownloadSpeedLimit = "10240";
            maxUploadSpeedLimit = "32";
            startTime = "08:00:00";
            endTime = "17:00:00";
        } else {
            if (currentValue.contains("fullspeed;")) {
                currentSelect = 1;
            }

            QStringList currentValueList = currentValue.split(';');

            if (currentValueList.count() > 4) {
                maxDownloadSpeedLimit = currentValueList.at(1);
                maxUploadSpeedLimit = currentValueList.at(2);
                startTime = currentValueList.at(3);
                endTime = currentValueList.at(4);
            }
        }

        setCurrentSelectRadioButton(currentSelect);
        setMaxDownloadSpeedLimit(maxDownloadSpeedLimit);
        setMaxUploadSpeedLimit(maxUploadSpeedLimit);
        setStartTime(startTime);
        setEndTime(endTime);
        //   option->blockSignals(false);
    }
}

void DownloadSettingWidget::onTimeChanged(const QString &time)
{
    CTimeEdit *timeEdit = qobject_cast<CTimeEdit *>(sender());

    if (!m_speedLimitDownloadButton->isChecked()) {
        return;
    }

    if (m_startTimeEdit == timeEdit ) {
        if(time < m_endTimeEdit->currentText()) {
            QString text = QString("speedlimit;%1;%2;%3;%4")
                               .arg(m_maxDownloadSpeedLimit->getLineEditText().toInt())
                               .arg(m_maxUploadSpeedLimit->getLineEditText().toInt())
                               .arg(time)
                               .arg(m_endTimeEdit->getTime().toString("hh:mm"));

            emit speedLimitInfoChanged(text);
        } else {
            qDebug() << "m_startTimeAlertControl";
            m_startTimeAlertControl->showAlertMessage(tr("The end time must be greater than the start time"),
                                                     m_startTimeEdit->dLineEdit()->parentWidget()->parentWidget(), 2000);
            m_startTimeEdit->dLineEdit()->setAlert(true);
            m_startTimeAlertControl->setMessageAlignment(Qt::AlignBottom | Qt::AlignRight);
        }
    } else if (m_endTimeEdit == timeEdit) {
        if(time > m_startTimeEdit->currentText()) {
            QString text = QString("speedlimit;%1;%2;%3;%4")
                               .arg(m_maxDownloadSpeedLimit->getLineEditText().toInt())
                               .arg(m_maxUploadSpeedLimit->getLineEditText().toInt())
                               .arg(m_startTimeEdit->getTime().toString("hh:mm"))
                               .arg(time);
            emit speedLimitInfoChanged(text);
        } else {
            qDebug() << "m_endTimeAlertControl";
            m_startTimeAlertControl->showAlertMessage(tr("The end time must be greater than the start time"),
                                                     m_startTimeEdit->dLineEdit()->parentWidget()->parentWidget(), 2000);
            m_startTimeEdit->dLineEdit()->setAlert(true);
            m_startTimeAlertControl->setMessageAlignment(Qt::AlignBottom | Qt::AlignRight);
        }
    }
}

void DownloadSettingWidget::onTextChanged(QString text)
{
    SettingInfoInputWidget *settingInfoInputWidget = qobject_cast<SettingInfoInputWidget *>(sender());

    if (!m_speedLimitDownloadButton->isChecked()) {
        return;
    }

    if (m_maxDownloadSpeedLimit == settingInfoInputWidget) {
        if (text.toInt() < 100 || text.toInt() > 102400) {
            // 请输入100-102400之间的整数
            m_downloadAlertControl->showAlertMessage(tr("Limited between 100-102400"),
                                                     m_maxDownloadSpeedLimit->getLineEdit()->parentWidget()->parentWidget(), -1);
            m_maxDownloadSpeedLimit->setLineEditAlert(true);
            m_downloadAlertControl->setMessageAlignment(Qt::AlignLeft);
        } else {
            m_maxDownloadSpeedLimit->setLineEditAlert(false);
            m_downloadAlertControl->hideAlertMessage();

            m_DownloadSpeedLimitValue = text.toInt();

            QString info = QString("speedlimit;%1;%2;%3;%4")
                               .arg(text.toInt())
                               .arg(m_maxUploadSpeedLimit->getLineEditText().toInt())
                               .arg(m_startTimeEdit->getTime().toString("hh:mm"))
                               .arg(m_endTimeEdit->getTime().toString("hh:mm"));

            emit speedLimitInfoChanged(info);
        }

    } else if (m_maxUploadSpeedLimit == settingInfoInputWidget) {
        if (text.toInt() < 16 || text.toInt() > 5120) {
            // 请输入16-5120之间的整数
            m_uploadAlertControl->showAlertMessage(tr("Limited between 16-5120"), m_maxUploadSpeedLimit->getLineEdit()->parentWidget()->parentWidget(), -1);
            m_maxUploadSpeedLimit->setLineEditAlert(true);
            m_uploadAlertControl->setMessageAlignment(Qt::AlignLeft);
        } else {
            m_maxUploadSpeedLimit->setLineEditAlert(false);
            m_uploadAlertControl->hideAlertMessage();

            m_UploadSpeedLimitValue = text.toInt();

            QString info = QString("speedlimit;%1;%2;%3;%4")
                               .arg(m_maxDownloadSpeedLimit->getLineEditText().toInt())
                               .arg(text.toInt())
                               .arg(m_startTimeEdit->getTime().toString("hh:mm"))
                               .arg(m_endTimeEdit->getTime().toString("hh:mm"));

            emit speedLimitInfoChanged(info);
        }
    }
}

void DownloadSettingWidget::onFocusChanged(bool onFocus)
{
    SettingInfoInputWidget *settingInfoInputWidget = qobject_cast<SettingInfoInputWidget *>(sender());

    if (m_maxDownloadSpeedLimit == settingInfoInputWidget) {
        if (!onFocus && m_maxDownloadSpeedLimit->getIsAlert()) {
            m_maxDownloadSpeedLimit->setLineEditAlert(false);
            m_downloadAlertControl->hideAlertMessage();
            m_maxDownloadSpeedLimit->setLineEditText(QString::number(100));
        }

    } else if (m_maxUploadSpeedLimit == settingInfoInputWidget) {
        if (!onFocus && m_maxUploadSpeedLimit->getIsAlert()) {
            m_maxUploadSpeedLimit->setLineEditAlert(false);
            m_uploadAlertControl->hideAlertMessage();
            m_maxUploadSpeedLimit->setLineEditText(QString::number(16));
        }
    }
}

bool DownloadSettingWidget::setCurrentSelectRadioButton(const int &currentSelect)
{
    if (1 == currentSelect) {
        m_fullSpeedDownloadButton->setChecked(true);
        m_speedLimitDownloadButton->setChecked(false);
        m_maxDownloadSpeedLimit->setLineEditIsDisabled(true);
        m_maxUploadSpeedLimit->setLineEditIsDisabled(true);
        m_startTimeEdit->setDisabled(true);
        m_endTimeEdit->setDisabled(true);
    } else {
        m_fullSpeedDownloadButton->setChecked(false);
        m_speedLimitDownloadButton->setChecked(true);
        m_maxDownloadSpeedLimit->setLineEditIsDisabled(false);
        m_maxUploadSpeedLimit->setLineEditIsDisabled(false);
        m_startTimeEdit->setDisabled(false);
        m_endTimeEdit->setDisabled(false);
    }
    return true;
}

bool DownloadSettingWidget::setMaxDownloadSpeedLimit(const QString &text)
{
    m_DownloadSpeedLimitValue = text.toInt();
    m_maxDownloadSpeedLimit->setLineEditText(text);
    return true;
}

bool DownloadSettingWidget::setMaxUploadSpeedLimit(const QString &text)
{
    m_UploadSpeedLimitValue = text.toInt();
    m_maxUploadSpeedLimit->setLineEditText(text);
    return true;
}

bool DownloadSettingWidget::setStartTime(const QString &text)
{
    QTime time = QTime::fromString(text, "hh:mm");
    m_startTimeEdit->setTime(time);
    return true;
}

bool DownloadSettingWidget::setEndTime(const QString &text)
{
    QTime time = QTime::fromString(text, "hh:mm");
    m_endTimeEdit->setTime(time);
    return true;
}
