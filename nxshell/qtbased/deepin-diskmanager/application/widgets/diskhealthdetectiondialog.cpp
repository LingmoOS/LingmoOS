// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "diskhealthdetectiondialog.h"
#include "common.h"
#include "diskhealthdetectiondelegate.h"
#include "messagebox.h"
#include "diskhealthheaderview.h"

#include <DFrame>
#include <DGuiApplicationHelper>
#include <DMessageManager>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTableWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QKeyEvent>

DiskHealthDetectionDialog::DiskHealthDetectionDialog(const QString &devicePath, HardDiskStatusInfoList hardDiskStatusInfoList, QWidget *parent)
    : DDialog(parent)
    , m_devicePath(devicePath)
    , m_hardDiskStatusInfoList(hardDiskStatusInfoList)
{
    initUI();
    initConnections();
}

DiskHealthDetectionDialog::~DiskHealthDetectionDialog()
{
    delete m_diskHealthDetectionDelegate;
}

void DiskHealthDetectionDialog::initUI()
{
    setIcon(QIcon::fromTheme(appName));
    setTitle(tr("Check Health")); // 硬盘健康检测
    setFixedSize(726, 700);

    QIcon iconDisk = Common::getIcon("disk");
    DLabel *diskLabel = new DLabel;
    diskLabel->setPixmap(iconDisk.pixmap(85, 85));

    DPalette palette1;
    palette1.setColor(DPalette::Text, "#666666");

    DPalette palette2;
    palette2.setColor(DPalette::Text, "#000000");

    // 状态提示字体颜色
    DPalette palette4;
    palette4.setColor(DPalette::WindowText, QColor("#526A7F"));

    // 表格内容颜色
    DPalette palette5;
    palette5.setColor(DPalette::Text, QColor("#001A2E"));

    // 表头字体颜色
    DPalette palette6;
    palette6.setColor(DPalette::Text, QColor("#414D68"));

    // 硬盘信息
    HardDiskInfo hardDiskInfo = DMDbusHandler::instance()->getHardDiskInfo(m_devicePath);

    DLabel *serialNumberNameLabel = new DLabel(tr("Serial number")); // 序列号
    DFontSizeManager::instance()->bind(serialNumberNameLabel, DFontSizeManager::T8, QFont::Medium);
    serialNumberNameLabel->setPalette(palette1);

    m_serialNumberValue = new DLabel;
    m_serialNumberValue->setText(hardDiskInfo.m_serialNumber);
    DFontSizeManager::instance()->bind(m_serialNumberValue, DFontSizeManager::T6, QFont::Medium);
    m_serialNumberValue->setPalette(palette2);
    m_serialNumberValue->setAccessibleName("Serial number");

    DLabel *userCapacityNameLabel = new DLabel(tr("Storage")); // 用户容量
    DFontSizeManager::instance()->bind(userCapacityNameLabel, DFontSizeManager::T10, QFont::Medium);
    userCapacityNameLabel->setPalette(palette1);

    m_userCapacityValue = new DLabel;
    m_userCapacityValue->setText(hardDiskInfo.m_size);
    DFontSizeManager::instance()->bind(m_userCapacityValue, DFontSizeManager::T10, QFont::Normal);
    m_userCapacityValue->setPalette(palette2);
    m_userCapacityValue->setAccessibleName("Storage");

    QVBoxLayout *diskInfoLayout = new QVBoxLayout;
    diskInfoLayout->addSpacing(7);
    diskInfoLayout->addWidget(serialNumberNameLabel);
    diskInfoLayout->addWidget(m_serialNumberValue);
    diskInfoLayout->addWidget(userCapacityNameLabel);
    diskInfoLayout->addWidget(m_userCapacityValue);
    diskInfoLayout->addSpacing(5);
    diskInfoLayout->setContentsMargins(0, 0, 0, 0);

    // 硬盘健康状态
    QString healthStateValue = DMDbusHandler::instance()->getDeviceHardStatus(m_devicePath);

    DLabel *healthStateLabel = new DLabel(tr("Health Status")); // 健康状态
    DFontSizeManager::instance()->bind(healthStateLabel, DFontSizeManager::T6, QFont::Medium);
    healthStateLabel->setPalette(palette2);

    QIcon iconHealth = Common::getIcon("good");
    DLabel *iconHealthLabel = new DLabel;
    m_healthStateValue = new DLabel;
    DFontSizeManager::instance()->bind(m_healthStateValue, DFontSizeManager::T2, QFont::Medium);
    m_healthStateValue->setAccessibleName("healthState");

    // 状态颜色
    DPalette paletteStateColor;

    if (0 == healthStateValue.compare("PASSED", Qt::CaseInsensitive) || 0 == healthStateValue.compare("OK", Qt::CaseInsensitive)) {
        iconHealth = Common::getIcon("good");
        iconHealthLabel->setPixmap(iconHealth.pixmap(30, 30));
        m_healthStateValue->setText(tr("Good")); // 良好    【警告】Warning
        paletteStateColor.setColor(DPalette::Text, QColor("#00c800"));
        m_healthStateValue->setPalette(paletteStateColor);
    } else if (0 == healthStateValue.compare("Failure", Qt::CaseInsensitive)) {
        iconHealth = Common::getIcon("damage");
        iconHealthLabel->setPixmap(iconHealth.pixmap(30, 30));
        m_healthStateValue->setText(tr("Damaged")); // 损坏
        paletteStateColor.setColor(DPalette::Text, QColor("#E02020"));
        m_healthStateValue->setPalette(paletteStateColor);
    } else {
        iconHealth = Common::getIcon("unknown");
        iconHealthLabel->setPixmap(iconHealth.pixmap(30, 30));
        m_healthStateValue->setText(tr("Unknown")); // 未知
        paletteStateColor.setColor(DPalette::Text, QColor("#777777"));
        m_healthStateValue->setPalette(paletteStateColor);
    }

    QHBoxLayout *healthValueLayout = new QHBoxLayout;
    healthValueLayout->addWidget(iconHealthLabel);
    healthValueLayout->addWidget(m_healthStateValue);
    healthValueLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *healthStateLayout = new QVBoxLayout;
    healthStateLayout->addSpacing(5);
    healthStateLayout->addWidget(healthStateLabel, 0, Qt::AlignCenter);
    healthStateLayout->addSpacing(10);
    healthStateLayout->addLayout(healthValueLayout);
    healthStateLayout->addSpacing(7);
    healthStateLayout->setContentsMargins(0, 0, 0, 0);

    // 温度
    DLabel *temperatureLabel = new DLabel(tr("Temperature")); // 温度
    DFontSizeManager::instance()->bind(temperatureLabel, DFontSizeManager::T6, QFont::Medium);
    temperatureLabel->setPalette(palette2);

    m_temperatureValue = new DLabel("-°C");
    DFontSizeManager::instance()->bind(m_temperatureValue, DFontSizeManager::T2, QFont::Medium);
    m_temperatureValue->setPalette(palette2);
    m_temperatureValue->setAccessibleName("temperature");

    QVBoxLayout *temperatureLayout = new QVBoxLayout;
    temperatureLayout->addSpacing(5);
    temperatureLayout->addWidget(temperatureLabel, 0, Qt::AlignRight);
    temperatureLayout->addSpacing(10);
    temperatureLayout->addWidget(m_temperatureValue);
    temperatureLayout->addSpacing(7);
    temperatureLayout->setContentsMargins(0, 0, 10, 0);

    DFrame *infoWidget = new DFrame;
    infoWidget->setBackgroundRole(DPalette::ItemBackground);
    infoWidget->setMinimumSize(706, 118);
    infoWidget->setLineWidth(0);

    QHBoxLayout *topLayout = new QHBoxLayout(infoWidget);
    topLayout->addWidget(diskLabel);
    topLayout->addLayout(diskInfoLayout);
    topLayout->addStretch();
    topLayout->addLayout(healthStateLayout);
    topLayout->addSpacing(50);
    topLayout->addLayout(temperatureLayout);
    topLayout->setContentsMargins(10, 10, 10, 10);

    // 属性列表
    m_tableView = new DTableView(this);
    m_standardItemModel = new QStandardItemModel(this);

    m_tableView->setShowGrid(false);
    m_tableView->setFrameShape(QAbstractItemView::NoFrame);
    m_tableView->verticalHeader()->setVisible(false); //隐藏列表头
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setPalette(palette5);

    m_diskHealthHeaderView = new DiskHealthHeaderView(Qt::Horizontal, this);
    m_tableView->setHorizontalHeader(m_diskHealthHeaderView);

    DFontSizeManager::instance()->bind(m_tableView->horizontalHeader(), DFontSizeManager::T6, QFont::Medium);
    m_tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignCenter);
    m_tableView->horizontalHeader()->setPalette(palette6);


    m_diskHealthDetectionDelegate = new DiskHealthDetectionDelegate(this);
    m_tableView->setItemDelegate(m_diskHealthDetectionDelegate);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        m_diskHealthDetectionDelegate->setTextColor(QColor("#C0C6D4"));
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_diskHealthDetectionDelegate->setTextColor(QColor("#001A2E"));
    }

    m_standardItemModel->setColumnCount(7);
    m_standardItemModel->setHeaderData(0, Qt::Horizontal, QString("   %1").arg(tr("ID"))); // 第一个需要向右对齐内容,15px
    m_standardItemModel->setHeaderData(1, Qt::Horizontal, tr("Status")); // 状态
    m_standardItemModel->setHeaderData(2, Qt::Horizontal, tr("Current")); // 当前值
    m_standardItemModel->setHeaderData(3, Qt::Horizontal, tr("Worst")); // 历史最差值
    m_standardItemModel->setHeaderData(4, Qt::Horizontal, tr("Threshold")); // 临界值
    m_standardItemModel->setHeaderData(5, Qt::Horizontal, tr("Raw Value")); // 原始数据
    m_standardItemModel->setHeaderData(6, Qt::Horizontal, tr("Attribute name")); // 属性名称

    m_tableView->setModel(m_standardItemModel);
    m_tableView->horizontalHeader()->setStretchLastSection(true);// 设置最后一列自适应
//    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//    m_tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // 设置第三列自适应列宽
//    m_tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // 设置第四列自适应列宽

    m_tableView->setColumnWidth(0, 70);
    m_tableView->setColumnWidth(1, 70);
    m_tableView->setColumnWidth(2, 70);
    m_tableView->setColumnWidth(3, 100);
    m_tableView->setColumnWidth(4, 70);
    m_tableView->setColumnWidth(5, 130);
//    m_tableView->setColumnWidth(6, 186);

    for (int i = 0; i < m_hardDiskStatusInfoList.count(); i++) {
        HardDiskStatusInfo hardDiskStatusInfo = m_hardDiskStatusInfoList.at(i);

        if (hardDiskStatusInfo.m_id == "194" || hardDiskStatusInfo.m_attributeName == "Temperature") {
            QString value;
            for (int i = 0; i < hardDiskStatusInfo.m_rawValue.size(); i++) {
                if (hardDiskStatusInfo.m_rawValue.at(i) >= "0" && hardDiskStatusInfo.m_rawValue.at(i) <= "9") {
                    value += hardDiskStatusInfo.m_rawValue.at(i);
                } else {
                    break;
                }
            }

            if (!value.isEmpty()) {
                m_temperatureValue->setText(QString("%1°C").arg(value));
            }
        }

        QList<QStandardItem*> itemList;

        if (!hardDiskStatusInfo.m_id.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_id);
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_whenFailed.isEmpty()) {
            if (hardDiskStatusInfo.m_whenFailed == "-") {
                itemList << new QStandardItem("G");
            } else if(0 == hardDiskStatusInfo.m_whenFailed.compare("In_the_past", Qt::CaseInsensitive)) {
                itemList << new QStandardItem("W");
            } else if(0 == hardDiskStatusInfo.m_whenFailed.compare("FAILING_NOW", Qt::CaseInsensitive)) {
                itemList << new QStandardItem("D");
            } else {
                itemList << new QStandardItem("U");
            }
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_value.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_value);
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_worst.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_worst);
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_thresh.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_thresh);
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_rawValue.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_rawValue);
        } else {
            itemList << new QStandardItem("-");
        }

        if (!hardDiskStatusInfo.m_attributeName.isEmpty()) {
            itemList << new QStandardItem(hardDiskStatusInfo.m_attributeName);
        } else {
            itemList << new QStandardItem("-");
        }

        m_standardItemModel->appendRow(itemList);
    }

    DFrame *tableWidget = new DFrame;
    tableWidget->setMinimumSize(706, 410);
    QHBoxLayout *tableLayout = new QHBoxLayout(tableWidget);
    tableLayout->addWidget(m_tableView);
    tableLayout->setSpacing(0);
    tableLayout->setContentsMargins(0, 0, 0, 10);

    DLabel *stateTipsLabel = new DLabel;
    stateTipsLabel->setText(tr("Status: (G: Good | W: Warning | D: Damaged | U: Unknown)")); // 状态:(G: 良好 | W: 警告 | D: 损坏 | U: 未知)
    DFontSizeManager::instance()->bind(stateTipsLabel, DFontSizeManager::T8, QFont::Normal);
    stateTipsLabel->setPalette(palette4);

    m_linkButton = new DCommandLinkButton(tr("Export", "button")); // 导出
    DFontSizeManager::instance()->bind(m_linkButton, DFontSizeManager::T8, QFont::Medium);
    m_linkButton->setFixedWidth(m_linkButton->fontMetrics().width(QString(tr("Export", "button"))));
    m_linkButton->setAccessibleName("export");

//    QWidget *bottomWidget = new QWidget;
    QHBoxLayout *bottomLayout = new QHBoxLayout/*(bottomWidget)*/;
    bottomLayout->addWidget(stateTipsLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_linkButton);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoWidget);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(tableWidget);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(bottomLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *widget = new QWidget;
    widget->setLayout(mainLayout);

    addSpacing(10);
    addContent(widget);
//    addContent(infoWidget);
//    addSpacing(10);
//    addContent(tableWidget);
//    addSpacing(10);
//    addContent(bottomWidget);
}

void DiskHealthDetectionDialog::initConnections()
{
    connect(m_linkButton, &DCommandLinkButton::clicked, this, &DiskHealthDetectionDialog::onExportButtonClicked);
}

void DiskHealthDetectionDialog::onExportButtonClicked()
{
    //文件保存路径
    QString fileDirPath = QFileDialog::getSaveFileName(this, tr("Save File"), "CheckHealthInfo.txt", tr("Text files (*.txt)"));// 文件保存   硬盘健康检测信息   文件类型
    if (fileDirPath.isEmpty()) {
        return;
    }

    QString strFileName = "/" + fileDirPath.split("/").last();
    QString fileDir = fileDirPath.split(strFileName).first();

    QFileInfo fileInfo;
    fileInfo.setFile(fileDir);
    QDir dir(fileDir);

    if (!dir.exists()) {
        // 路径错误
        DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Wrong path"));
        DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 20));

        return;
    }

    if (!fileInfo.isWritable()) {
        // 您无权访问该路径
        DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("You do not have permission to access this path"));
        DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 20));
    } else {
        if (!fileDirPath.contains(".txt")) {
            fileDirPath = fileDirPath + ".txt";
        }

        QFile file(fileDirPath);
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
            QTextStream out(&file);

            QString headers = tr("ID") + "," + tr("Status") + "," + tr("Current") + "," + tr("Worst")
                    + "," + tr("Threshold") + "," + tr("Raw Value") + "," + tr("Attribute name") + "\n";
            out << headers;

            for (int i = 0; i < m_standardItemModel->rowCount(); i++) {
                QString strInfo = m_standardItemModel->item(i, 0)->text() + ","
                        + m_standardItemModel->item(i, 1)->text() + ","
                        + m_standardItemModel->item(i, 2)->text() + ","
                        + m_standardItemModel->item(i, 3)->text() + ","
                        + m_standardItemModel->item(i, 4)->text() + ","
                        + m_standardItemModel->item(i, 5)->text() + ","
                        + m_standardItemModel->item(i, 6)->text() + "\n";

                out << strInfo;
                out.flush();
            }

            file.close();

            DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("://icons/deepin/builtin/ok.svg"), tr("Export successful")); // 导出成功
            DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 20));
        } else {
            DMessageManager::instance()->sendMessage(this, QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Export failed")); // 导出失败
            DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 20));
        }
    }
}

bool DiskHealthDetectionDialog::event(QEvent *event)
{
    // 字体大小改变
    if (QEvent::ApplicationFontChange == event->type()) {
        m_linkButton->setFixedWidth(m_linkButton->fontMetrics().width(QString(tr("Export", "button"))));

        if (QApplication::font().pointSizeF() / 0.75 >= 18 ) {
            setFixedSize(726, 705);
        } else {
            setFixedSize(726, 700);
        }
        DDialog::event(event);
    }

    return DDialog::event(event);
}

void DiskHealthDetectionDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}



