// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "cylinderinfowidget.h"
#include "cylinderwidget.h"

#include <DPalette>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <QApplication>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QTime>

CylinderInfoWidget::CylinderInfoWidget(int cylNumber, QWidget *parent)
    : DFrame(parent)
    , m_cylNumber(cylNumber)
{
    initUI();
    initConnections();
}

void CylinderInfoWidget::initUI()
{
    m_gridLayout = new QGridLayout;

    int initCount = 360;
    if (initCount > m_cylNumber) {
        initCount = m_cylNumber;
    }

    m_initColor = "rgba(0,0,0,0.03)";
    m_excellentColor = "#6097FF";
    m_damagedColor = "#E23C3C";
    m_unknownColor = "#777990";
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        m_initColor = "rgba(255,255,255,0.05)";
        m_excellentColor = "#2B6AE3";
        m_damagedColor = "#C41E1E";
        m_unknownColor = "#909090";
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_initColor = "rgba(0,0,0,0.03)";
        m_excellentColor = "#6097FF";
        m_damagedColor = "#E23C3C";
        m_unknownColor = "#777990";
    }

    for (int i = 0; i < initCount; i ++) {
        CylinderWidget *cylinderWidget = new CylinderWidget;
        cylinderWidget->setFixedSize(20, 20);
        cylinderWidget->setFrameShape(QFrame::Box);
        cylinderWidget->setObjectName(QString("%1").arg(i));
//        pLabel->setPalette(QPalette(QColor("#6097FF")));
//        QMap<QString, QVariant> map = lst.at(i);
        cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
//        pLabel->setUserData(map);
        connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
        connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

        m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
    }

    m_gridLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *cylLayout = new QHBoxLayout;
    cylLayout->addLayout(m_gridLayout);
    cylLayout->addStretch();
    cylLayout->setContentsMargins(0, 0, 0, 0);

    m_widget = new QWidget;
    m_widget->setLayout(cylLayout);

    m_arrowRectangle = new DArrowRectangle(DArrowRectangle::ArrowTop, this);
    QColor color("#D2D2D2");
    color.setAlphaF(0.3);
    m_arrowRectangle->setBackgroundColor(color);
    m_arrowRectangle->setArrowWidth(54);
    m_arrowRectangle->setArrowHeight(18);
    m_arrowRectangle->setRadius(18);
    m_arrowRectangle->setWidth(207);
    m_arrowRectangle->setHeight(88);

    DPalette palette;
    palette.setColor(DPalette::WindowText, QColor("#000000"));

    m_LBALabel = new QLabel("LBA：772678305；");
    DFontSizeManager::instance()->bind(m_LBALabel, DFontSizeManager::T8, QFont::Normal);
    m_LBALabel->setPalette(palette);

    m_cylNumberLabel = new QLabel("柱面号：48097；");
    DFontSizeManager::instance()->bind(m_cylNumberLabel, DFontSizeManager::T8, QFont::Normal);
    m_cylNumberLabel->setPalette(palette);

    m_elapsedTimeLabel = new QLabel("柱面耗时：100ms；");
    DFontSizeManager::instance()->bind(m_elapsedTimeLabel, DFontSizeManager::T8, QFont::Normal);
    m_elapsedTimeLabel->setPalette(palette);

    m_curErrorInfoLabel = new QLabel("当前错误信息：I0 Device Timeout.");
    DFontSizeManager::instance()->bind(m_curErrorInfoLabel, DFontSizeManager::T8, QFont::Normal);
    m_curErrorInfoLabel->setPalette(palette);

    m_repairLabel =new QLabel("修复情况：已修复");
    DFontSizeManager::instance()->bind(m_repairLabel, DFontSizeManager::T8, QFont::Normal);
    m_repairLabel->setPalette(palette);

    QVBoxLayout *pLayout = new QVBoxLayout;
    pLayout->addWidget(m_LBALabel);
    pLayout->addWidget(m_cylNumberLabel);
    pLayout->addWidget(m_elapsedTimeLabel);
    pLayout->addWidget(m_curErrorInfoLabel);
    pLayout->addWidget(m_repairLabel);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(5, 5, 5, 5);

    QWidget *pInfoWidget = new QWidget;
    pInfoWidget->setLayout(pLayout);

    m_arrowRectangle->setContent(pInfoWidget);

    int rowCount = m_curCheckCount / 24;

    if ( m_curCheckCount % 24 != 0) {
        rowCount = m_curCheckCount / 24 + 1;
    }

    m_scrollBar = new DScrollBar(this);
    m_scrollBar->setRange(0, rowCount - 15);
    m_scrollBar->setPageStep(30);
    m_scrollBar->setSingleStep(5);
    m_scrollBar->hide();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_widget);
    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);

    m_settings = new QSettings("/tmp/CheckData.conf", QSettings::IniFormat, this);
}

void CylinderInfoWidget::initConnections()
{
    connect(m_scrollBar, &QScrollBar::valueChanged, this, &CylinderInfoWidget::onScrollBarValueChanged);
}

void CylinderInfoWidget::againVerify(int cylNumber)
{
    m_curCheckCount = 0;
    m_badSectorsCount = 0;
    m_scrollBar->hide();
    m_isChanged = false;
    m_startCylinder = m_settings->value("SettingData/BlockStart").toInt();
    m_endCylinder = m_settings->value("SettingData/BlockEnd").toInt();

    QList<QObject *> lstCylinderWidget = m_widget->children();

    if (cylNumber <= lstCylinderWidget.count() - 1) { // 当将要重新检测的个数小于等于360时
        for (int i = 1; i < lstCylinderWidget.count(); i ++) {
            CylinderWidget *cylinderWidget = static_cast<CylinderWidget *>(lstCylinderWidget.at(i));
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
            cylinderWidget->setUserData("");
        }
    } else { // 当将要重新检测的个数大于360
        for (int i = 0; i < 360; i ++) {
            if (i < lstCylinderWidget.count() - 1) {
                CylinderWidget *cylinderWidget = static_cast<CylinderWidget *>(lstCylinderWidget.at(i + 1));
                cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
                cylinderWidget->setUserData("");
            }else {
                CylinderWidget *cylinderWidget = new CylinderWidget;
                cylinderWidget->setFixedSize(20, 20);
                cylinderWidget->setFrameShape(QFrame::Box);
                cylinderWidget->setObjectName(QString("%1").arg(i));
                cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
                connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
                connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

                m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
            }

        }
    }

}

void CylinderInfoWidget::reset(int cylNumber)
{
    m_curCheckCount = 0;
    m_badSectorsCount = 0;
    m_scrollBar->hide();
    m_isChanged = false;

    QList<QObject *> lstCylinderWidget = m_widget->children();
    for (int i = 1; i < lstCylinderWidget.count(); i ++) {
        QObject *obj = lstCylinderWidget.at(i);
        delete obj;
        obj = nullptr;
    }

    if (cylNumber <= 360) {
        for (int i = 0; i < cylNumber; i++) {
            CylinderWidget *cylinderWidget = new CylinderWidget;
            cylinderWidget->setFixedSize(20, 20);
            cylinderWidget->setFrameShape(QFrame::Box);
            cylinderWidget->setObjectName(QString("%1").arg(i));
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
            connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
            connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

            m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
        }
    } else {
        for (int i = 0; i < 360; i++) {
            CylinderWidget *cylinderWidget = new CylinderWidget;
            cylinderWidget->setFixedSize(20, 20);
            cylinderWidget->setFrameShape(QFrame::Box);
            cylinderWidget->setObjectName(QString("%1").arg(i));
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
            connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
            connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

            m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
        }
    }

}

void CylinderInfoWidget::setChecked(bool isCheck)
{
    m_isCheck = isCheck;
}

void CylinderInfoWidget::setCylinderNumber(int cylNumber)
{
    m_cylNumber = cylNumber;
    m_curCheckCount = 0;
    m_badSectorsCount = 0;
    m_scrollBar->hide();
    m_isChanged = false;
    m_startCylinder = m_settings->value("SettingData/BlockStart").toInt();
    m_endCylinder = m_settings->value("SettingData/BlockEnd").toInt();

    QList<QObject *> lstCylinderWidget = m_widget->children();

    if (m_cylNumber == lstCylinderWidget.count() - 1) { // 当前要检测的柱面数等于初始化个数
        return;
    }

    if (m_cylNumber > lstCylinderWidget.count() - 1 && (lstCylinderWidget.count() - 1 == 360)) { // 初始化个数为360并且当前要检测的柱面数大于初始化个数
        return;
    }

    if (m_cylNumber < lstCylinderWidget.count() - 1) { // 当前要检测的柱面数小于初始化个数
        for (int i = 1; i < lstCylinderWidget.count(); i ++) {
            if(i > m_cylNumber) {
                QObject *obj = lstCylinderWidget.at(i);
                delete obj;
                obj = nullptr;
            }
        }
    } else if((m_cylNumber > lstCylinderWidget.count() - 1) && (m_cylNumber < 360)){ // 当前要检测的柱面数大于初始化个数且当前要检测的柱面数不到360个时
        for (int i = 1; i < lstCylinderWidget.count(); i ++) {
            QObject *obj = lstCylinderWidget.at(i);
            delete obj;
            obj = nullptr;
        }

        for (int i = 0; i < m_cylNumber; i ++) {
            CylinderWidget *cylinderWidget = new CylinderWidget;
            cylinderWidget->setFixedSize(20, 20);
            cylinderWidget->setFrameShape(QFrame::Box);
            cylinderWidget->setObjectName(QString("%1").arg(i));
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
            connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
            connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

            m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
        }
    } else { // 当前要检测的柱面数大于初始化个数且当前要检测的柱面数大于360个时，只显示360个柱面块
        for (int i = 1; i < lstCylinderWidget.count(); i ++) {
            QObject *obj = lstCylinderWidget.at(i);
            delete obj;
            obj = nullptr;
        }

        for (int i = 0; i < 360; i ++) {
            CylinderWidget *cylinderWidget = new CylinderWidget;
            cylinderWidget->setFixedSize(20, 20);
            cylinderWidget->setFrameShape(QFrame::Box);
            cylinderWidget->setObjectName(QString("%1").arg(i));
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
            connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
            connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

            m_gridLayout->addWidget(cylinderWidget, i / 24, i % 24);
        }
    }
}

void CylinderInfoWidget::setCurCheckBadBlocksInfo(const QString &LBANumber, const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo)
{
    ++m_curCheckCount;

    if (cylinderStatus == "bad") {
        ++m_badSectorsCount;
    }

    if (m_checkData.count() >= 360) {
        m_checkData.removeFirst();
        m_cylNumberData.removeFirst();
    }
    m_cylNumberData << cylinderNumber;
    m_checkData << QString("%1,%2,%3,%4,%5,0").arg(LBANumber).arg(cylinderNumber).arg(cylinderTimeConsuming).arg(cylinderStatus).arg(cylinderErrorInfo);

    if (m_isChanged) {
        return;
    }

//    int startCylinder = m_settings->value("SettingData/BlockStart").toInt();
    if (m_cylNumber <= 360) {
        updateCylinderInfo((cylinderNumber.toInt() - m_startCylinder) % 360, LBANumber, cylinderNumber, cylinderTimeConsuming, cylinderStatus, cylinderErrorInfo, "0");
    } else {
        if (m_curCheckCount <= 360) {
            updateCylinderInfo((cylinderNumber.toInt() - m_startCylinder) % 360, LBANumber, cylinderNumber, cylinderTimeConsuming, cylinderStatus, cylinderErrorInfo, "0");
        }else {

            int rowCount = m_curCheckCount / 24;

            if ( m_curCheckCount % 24 != 0) {
                rowCount = m_curCheckCount / 24 + 1;
            }
            m_scrollBar->setRange(0, rowCount - 15);

            if (!m_isChanged) {
                if (rowCount > 15) {
                    m_scrollBar->setValue(rowCount - 15);
                    m_scrollBar->setGeometry(600, 0, 18, 390);
                    m_scrollBar->show();
                }

                QList<QObject *> lstCylinderWidget = m_widget->children();
                int start = (rowCount - 15) * 24 + m_startCylinder;
                for (int i = start; i < start + 360; i ++) {
                    if (-1 != m_cylNumberData.indexOf(QString("%1").arg(i))) {
                        QString value = m_checkData.at(m_cylNumberData.indexOf(QString("%1").arg(i)));
                        QStringList lst = value.split(",");
                        updateCylinderInfo((i - start) % 360, lst.at(0), lst.at(1), lst.at(2), lst.at(3), lst.at(4), lst.at(5));
                    } else {
                        if (i > m_endCylinder) {
                            CylinderWidget *cylinderWidget = m_widget->findChild<CylinderWidget *>(QString("%1").arg((i - start) % 360));
                            delete cylinderWidget;
                            cylinderWidget = nullptr;
                        } else {
                            updateCylinderInfo((i - start) % 360, "", "", "", "", "", "");
                        }
                    }
                }
            }
        }
    }

//    if (m_cylNumber == m_curCheckCount) {
//        emit checkCoomplete(m_badSectorsCount);
//    }
}

void CylinderInfoWidget::updateCylinderInfo(int number, const QString &LBANumber, const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo, const QString &repair)
{
    CylinderWidget *cylinderWidget = m_widget->findChild<CylinderWidget *>(QString("%1").arg(number));

    if(cylinderWidget == nullptr) {
        return;
    }

    if (LBANumber.isEmpty()) {
        cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
        cylinderWidget->setUserData("");

    } else {
        if (cylinderStatus == "good") {
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_excellentColor));
        } else if (cylinderStatus == "bad") {
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_damagedColor));
        } else {
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_unknownColor));
        }

        QMap<QString, QVariant> mapInfo;
        mapInfo["id"] = LBANumber;
        mapInfo["number"] = cylinderNumber;
        mapInfo["time"] = cylinderTimeConsuming;
        mapInfo["errorinfo"] = cylinderErrorInfo;
        mapInfo["status"] = cylinderStatus;
        mapInfo["repair"] = repair;

        cylinderWidget->setUserData(mapInfo);
    }
}

void CylinderInfoWidget::setCurRepairBadBlocksInfo(const QString &cylinderNumber)
{
    QList<QObject *> lstCylinderWidget = m_widget->children();

    if (lstCylinderWidget.count() < 2) {
        return;
    }

    CylinderWidget *firstWidget = static_cast<CylinderWidget *>(lstCylinderWidget.at(1));
    QMap<QString, QVariant> firstValue = firstWidget->getUserData().toMap();

    CylinderWidget *lastWidget = static_cast<CylinderWidget *>(lstCylinderWidget.at(lstCylinderWidget.count() - 1));
    QMap<QString, QVariant> lastValue = lastWidget->getUserData().toMap();
    if (firstValue["number"].toInt() > cylinderNumber.toInt() || lastValue["number"].toInt() < cylinderNumber.toInt()) {
        return;
    }

    for (int i = 1; i < lstCylinderWidget.count(); i++) {
        CylinderWidget *cylinderWidget = static_cast<CylinderWidget *>(lstCylinderWidget.at(i));
        QMap<QString, QVariant> mapInfo = cylinderWidget->getUserData().toMap();

        if (mapInfo["number"].toInt() == cylinderNumber.toInt()) {
            cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_excellentColor));
            QString curCheckData = m_settings->value(QString("CheckData/%1").arg(cylinderNumber)).toString();
            QStringList lstCheckData = curCheckData.split(",");
            mapInfo["status"] = lstCheckData.at(3);
            mapInfo["repair"] = lstCheckData.at(5);
            cylinderWidget->setUserData(mapInfo);
            break;
        }
    }

}

void CylinderInfoWidget::enterSlot()
{
    CylinderWidget *label = qobject_cast<CylinderWidget *>(sender());

    QMap<QString, QVariant> mapInfo = label->getUserData().toMap();

    if (mapInfo.isEmpty()) {
        return;
    }

    m_LBALabel->setText(QString(tr("LBA: %1")).arg(mapInfo["id"].toString())); // LBA
    m_cylNumberLabel->setText(QString(tr("Cyl.: %1")).arg(mapInfo["number"].toString())); // 柱面号
    m_curErrorInfoLabel->setText(QString(tr("Error: %1")).arg(mapInfo["errorinfo"].toString())); // 当前错误信息
    m_elapsedTimeLabel->setText(QString(tr("Cyl. elapsed time: %1")).arg(mapInfo["time"].toString()) + tr("ms")); // 柱面耗时  毫秒
    m_repairLabel->setText(tr("Status: Repaired")); // 修复情况：已修复

    if (mapInfo["repair"].toInt() == 1) {
        m_curErrorInfoLabel->show();
        m_repairLabel->show();
        m_elapsedTimeLabel->hide();
    } else {
        if (mapInfo["status"].toString() == "good") {
            m_curErrorInfoLabel->hide();
            m_repairLabel->hide();
            m_elapsedTimeLabel->show();
        } else {
            m_curErrorInfoLabel->show();
            m_elapsedTimeLabel->show();
            m_repairLabel->hide();
        }
    }

    m_arrowRectangle->show(label->mapToGlobal(label->pos()).x() - label->pos().x() + (label->width() / 2),
                           label->mapToGlobal(label->pos()).y() - label->pos().y() + label->height() - 1);
}

void CylinderInfoWidget::leaveSlot()
{
    CylinderWidget *label = qobject_cast<CylinderWidget *>(sender());
    QMap<QString, QVariant> mapInfo = label->getUserData().toMap();
    if (mapInfo.isEmpty()) {
        return;
    }

    m_arrowRectangle->hide();
}

void CylinderInfoWidget::onScrollBarValueChanged(int value)
{
    if (value < m_scrollBar->maximum()) {
        m_isChanged = true;
    } else {
        m_isChanged = false;
    }

    if (m_isCheck && !m_isChanged) {
        return;
    }

    int startCylinder = m_settings->value("SettingData/BlockStart").toInt();

    QList<QObject *> lstCylinderWidget = m_widget->children();
    int start = value * 24 + startCylinder;
    for (int i = start; i < start + 360; i ++) {
        QString value = m_settings->value(QString("CheckData/%1").arg(i)).toString();
        if (!value.isEmpty()) {
            if (((i - start) % 360 + 1) > (lstCylinderWidget.count() - 1)) {
                CylinderWidget *cylinderWidget = new CylinderWidget;
                cylinderWidget->setFixedSize(20, 20);
                cylinderWidget->setFrameShape(QFrame::Box);
                cylinderWidget->setObjectName(QString("%1").arg((i - start) % 360));
                cylinderWidget->setStyleSheet(QString("background:%1;border:0px").arg(m_initColor));
                connect(cylinderWidget, &CylinderWidget::enter, this, &CylinderInfoWidget::enterSlot);
                connect(cylinderWidget, &CylinderWidget::leave, this, &CylinderInfoWidget::leaveSlot);

                m_gridLayout->addWidget(cylinderWidget, ((i - start) % 360) / 24, ((i - start) % 360) % 24);
            }

            QStringList lst = value.split(",");
            updateCylinderInfo((i - start) % 360, lst.at(0), lst.at(1), lst.at(2), lst.at(3), lst.at(4), lst.at(5));
        } else {
            if (i > m_settings->value("SettingData/BlockEnd").toInt()) {
                CylinderWidget *cylinderWidget = m_widget->findChild<CylinderWidget *>(QString("%1").arg((i - start) % 360));
                delete cylinderWidget;
                cylinderWidget = nullptr;
            } else {
                updateCylinderInfo((i - start) % 360, "", "", "", "", "", "");
            }
        }
    }
}

void CylinderInfoWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_scrollBar->isHidden()) {
        if(event->delta() > 0){
            int value = m_scrollBar->value() - 15;
            m_scrollBar->setValue(value);
            m_isChanged = true;
        }else{
            int value = m_scrollBar->value() + 15;
            m_scrollBar->setValue(value);

            if (value >= m_scrollBar->maximum()) {
                m_isChanged = false;
            }
        }
    }
}


