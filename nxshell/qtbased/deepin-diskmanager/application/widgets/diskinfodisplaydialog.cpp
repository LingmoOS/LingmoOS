// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "diskinfodisplaydialog.h"
#include "partedproxy/dmdbushandler.h"
#include "messagebox.h"
#include "common.h"

#include <DLabel>
#include <DFrame>
#include <DMessageManager>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>

DiskInfoDisplayDialog::DiskInfoDisplayDialog(const QString &devicePath, QWidget *parent)
    : DDialog(parent)
    , m_devicePath(devicePath)
{
    initUI();
    initConnections();
}

void DiskInfoDisplayDialog::initUI()
{
//    setWindowTitle("磁盘信息展示");
    setIcon(QIcon::fromTheme(appName));
    setTitle(tr("Disk Info")); // 磁盘信息
    setFixedSize(550, 554);

    DFrame *infoWidget = new DFrame;
    infoWidget->setBackgroundRole(DPalette::ItemBackground);
    infoWidget->setFixedSize(530, 444);
    infoWidget->setLineWidth(0);

    HardDiskInfo hardDiskInfo = DMDbusHandler::instance()->getHardDiskInfo(m_devicePath);
    if (hardDiskInfo.m_mediaType == "SSD") {
        hardDiskInfo.m_mediaType = tr("SSD");
    } else if (hardDiskInfo.m_mediaType == "HDD") {
        hardDiskInfo.m_mediaType = tr("HDD");
    }

    m_diskInfoNameList.clear();
    m_diskInfoNameList << tr("Model:") << tr("Vendor:") << tr("Media Type:") << tr("Size:")
                       << tr("Rotation Rate:") << tr("Interface:") << tr("Serial Number:") << tr("Version:")
                       << tr("Capabilities:") << tr("Description:") << tr("Power On Hours:")
                       << tr("Power Cycle Count:") << tr("Firmware Version:") << tr("Speed:");

    m_diskInfoValueList.clear();
    m_diskInfoValueList << tr("%1").arg(hardDiskInfo.m_model) << tr("%1").arg(hardDiskInfo.m_vendor)
                        << tr("%1").arg(hardDiskInfo.m_mediaType) << tr("%1").arg(hardDiskInfo.m_size)
                        << tr("%1").arg(hardDiskInfo.m_rotationRate) << tr("%1").arg(hardDiskInfo.m_interface)
                        << tr("%1").arg(hardDiskInfo.m_serialNumber) << tr("%1").arg(hardDiskInfo.m_version)
                        << tr("%1").arg(hardDiskInfo.m_capabilities) << tr("%1").arg(hardDiskInfo.m_description)
                        << tr("%1").arg(hardDiskInfo.m_powerOnHours) << tr("%1").arg(hardDiskInfo.m_powerCycleCount)
                        << tr("%1").arg(hardDiskInfo.m_firmwareVersion) << tr("%1").arg(hardDiskInfo.m_speed);

    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);

    DPalette palette1;
    QColor color1("#000000");
    color1.setAlphaF(0.85);
    palette1.setColor(DPalette::WindowText, color1);

    DPalette palette2;
    QColor color2("#000000");
    color2.setAlphaF(0.7);
    palette2.setColor(DPalette::WindowText, color2);

    for (int i = 0; i < m_diskInfoNameList.count(); i++) {
        DLabel *nameLabel = new DLabel;
        nameLabel->setText(m_diskInfoNameList.at(i));
        DFontSizeManager::instance()->bind(nameLabel, DFontSizeManager::T7, QFont::Medium);
        nameLabel->setFixedWidth(160);
        nameLabel->setPalette(palette1);

        DLabel *valueLabel = new DLabel;
        if (!m_diskInfoValueList.at(i).isEmpty()) {
            valueLabel->setText(m_diskInfoValueList.at(i));
        } else {
            valueLabel->setText("-");
        }

        DFontSizeManager::instance()->bind(valueLabel, DFontSizeManager::T7, QFont::Normal);
        valueLabel->setPalette(palette2);

        QHBoxLayout *labelLayout = new QHBoxLayout;
        labelLayout->addWidget(nameLabel);
        labelLayout->addSpacing(10);
        labelLayout->addWidget(valueLabel);
        labelLayout->setSpacing(0);
        labelLayout->setContentsMargins(0, 0, 0, 0);

        infoLayout->addLayout(labelLayout);
    }

    m_linkButton = new DCommandLinkButton(tr("Export", "button")); // 导出
    DFontSizeManager::instance()->bind(m_linkButton, DFontSizeManager::T8, QFont::Medium);
    QFontMetrics fmCapacity = m_linkButton->fontMetrics();
    int width = fmCapacity.width(QString(tr("Export", "button")));
    m_linkButton->setFixedWidth(width);
    m_linkButton->setAccessibleName("export");

    QHBoxLayout *exportLayout = new QHBoxLayout;
    exportLayout->addWidget(m_linkButton);
    exportLayout->addStretch();
    exportLayout->setSpacing(0);
    exportLayout->setContentsMargins(0, 0, 0, 0);

    infoLayout->addLayout(exportLayout);

//    infoLayout->addWidget(m_linkButton);
    infoLayout->setContentsMargins(10, 10, 10, 10);

    addSpacing(10);
    addContent(infoWidget);
}

void DiskInfoDisplayDialog::initConnections()
{
    connect(m_linkButton, &DCommandLinkButton::clicked, this, &DiskInfoDisplayDialog::onExportButtonClicked);
}

void DiskInfoDisplayDialog::onExportButtonClicked()
{
    qDebug() << "startExport";
    //文件保存路径
    QString fileDirPath = QFileDialog::getSaveFileName(this, tr("Save File"), "DiskInfo.txt", tr("Text files (*.txt)"));// 文件保存   磁盘信息   文件类型
    qDebug() << "fileDirPath:" << fileDirPath;
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
            for (int i = 0; i < m_diskInfoNameList.count(); i++) {
                QString strInfo = "";

                if (!m_diskInfoValueList.at(i).isEmpty()) {
                    strInfo = m_diskInfoNameList.at(i) + m_diskInfoValueList.at(i) + "\n";
                } else {
                    strInfo = m_diskInfoNameList.at(i) + "-" + "\n";
                }

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

bool DiskInfoDisplayDialog::event(QEvent *event)
{
    // 字体大小改变
    if (QEvent::ApplicationFontChange == event->type()) {
        m_linkButton->setFixedWidth(m_linkButton->fontMetrics().width(QString(tr("Export", "button"))));
        DDialog::event(event);
    }

    return DDialog::event(event);
}

void DiskInfoDisplayDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}



