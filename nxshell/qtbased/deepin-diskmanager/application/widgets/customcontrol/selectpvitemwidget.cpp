// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "selectpvitemwidget.h"
#include "common.h"
#include "partedproxy/dmdbushandler.h"

#include <DFontSizeManager>

#include <QHBoxLayout>

SelectPVItemWidget::SelectPVItemWidget(PVInfoData pvInfoData, QWidget *parent)
    : RadiusWidget(parent)
    , m_pvInfoData(pvInfoData)
{
    initUi();
    initConnection();
    initData();
}

void SelectPVItemWidget::initUi()
{
    setFixedHeight(36);

    DPalette paletteName;
    paletteName.setColor(DPalette::TextTips, QColor("#2C4767"));

    DPalette paletteSize;
    paletteName.setColor(DPalette::TextTips, QColor("#526A7F"));

    QFont fontName;
    fontName.setWeight(QFont::Medium);
    fontName.setFamily("Source Han Sans");
    fontName.setPixelSize(14);

    QFont fontSize;
    fontSize.setWeight(QFont::Normal);
    fontSize.setFamily("Source Han Sans");
    fontSize.setPixelSize(12);

    m_checkBox = new DCheckBox(this);

    m_pathLabel = new DLabel(this);
    m_pathLabel->setText("/dev/sda");
    m_pathLabel->setFont(fontName);
    m_pathLabel->setPalette(paletteName);
    m_pathLabel->setAlignment(Qt::AlignVCenter);

    m_sizeLabel = new DLabel(this);
    m_sizeLabel->setText("1GiB");
    m_sizeLabel->setFont(fontSize);
    m_sizeLabel->setPalette(paletteSize);
    m_sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_checkBox, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_pathLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(m_sizeLabel);
    mainLayout->setSpacing(0);

    if ((m_pvInfoData.m_level == DMDbusHandler::DISK) && (m_pvInfoData.m_disktype != "unrecognized")) {
        m_iconLabel = new DLabel(this);
        QIcon icon = Common::getIcon("arrow_right");
        m_iconLabel->setPixmap(icon.pixmap(QSize(8, 11)));

        mainLayout->addSpacing(13);
        mainLayout->addWidget(m_iconLabel);
        mainLayout->setContentsMargins(10, 0, 12, 0);
    } else {
        mainLayout->setContentsMargins(10, 0, 20, 0);
    }

    setLayout(mainLayout);
}

void SelectPVItemWidget::initConnection()
{
    connect(m_checkBox, &DCheckBox::stateChanged, this, &SelectPVItemWidget::onCheckBoxStateChange);
}

void SelectPVItemWidget::initData()
{
    switch (m_pvInfoData.m_selectStatus) {
    case Qt::CheckState::Unchecked: {
        setCheckBoxState(Qt::CheckState::Unchecked);
        break;
    }
    case Qt::CheckState::PartiallyChecked: {
        setCheckBoxState(Qt::CheckState::PartiallyChecked);
        break;
    }
    case Qt::CheckState::Checked: {
        setCheckBoxState(Qt::CheckState::Checked);
        break;
    }
    default:
        break;
    }

    if (m_pvInfoData.m_level == DMDbusHandler::DISK) {
        m_pathLabel->setText(m_pvInfoData.m_diskPath);
        m_sizeLabel->setText(m_pvInfoData.m_diskSize);
    } else {
        QString partitionPath = m_pvInfoData.m_partitionPath;
        if (partitionPath == "unallocated") {
            m_pathLabel->setText(partitionPath);
        } else {
            m_pathLabel->setText(partitionPath.remove(0, 5));
        }

        m_sizeLabel->setText(m_pvInfoData.m_partitionSize);
    }

    if (m_pvInfoData.m_isReadOnly) {
        m_checkBox->setDisabled(true);
    }
}

void SelectPVItemWidget::onCheckBoxStateChange(int state)
{
    if (m_pvInfoData.m_level == DMDbusHandler::DISK) {
        if (state != Qt::CheckState::PartiallyChecked) {
            if (m_lstPVInfoData.count() != 0) {
                QList<PVInfoData> lstData;
                lstData.clear();
                for (int i = 0; i < m_lstPVInfoData.count(); i++) {
                    PVInfoData pvData = m_lstPVInfoData.at(i);
                    pvData.m_selectStatus = state;

                    lstData.append(pvData);
                }

                m_lstPVInfoData.clear();
                m_lstPVInfoData = lstData;
            }
        }

        m_pvInfoData.m_selectStatus = state;
        emit checkBoxStateChange(state);
    } else {
        m_pvInfoData.m_selectStatus = state;

        emit checkBoxStateChange(state);
    }
}

void SelectPVItemWidget::setCheckBoxState(Qt::CheckState state, bool isPartiallyChecked)
{
    if(isPartiallyChecked){
        m_pvInfoData.m_selectStatus = state;
        m_checkBox->setCheckState(state);
    } else {
        blockSignals(true);
        m_pvInfoData.m_selectStatus = state;
        m_checkBox->setCheckState(state);
        blockSignals(false);
    }
}

void SelectPVItemWidget::setData(const QList<PVInfoData> &lstData)
{
    m_lstPVInfoData = lstData;
}

QList<PVInfoData> SelectPVItemWidget::getData()
{
    return m_lstPVInfoData;
}

PVInfoData SelectPVItemWidget::getCurInfo()
{
    return m_pvInfoData;
}

void SelectPVItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_pvInfoData.m_level == DMDbusHandler::DISK) {
        emit selectItem();
    }

    return QWidget::mousePressEvent(event);
}
