// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dmframewidget.h"
#include "utils.h"
#include "partitioninfo.h"
#include "partedproxy/dmdbushandler.h"

#include <DFontSizeManager>
#include <DApplicationHelper>

#include <QDebug>

DmFrameWidget::DmFrameWidget(DiskInfoData data, QWidget *parent)
    : DFrame(parent)
    , m_infoData(data)
{
    m_parentPb = DApplicationHelper::instance()->palette(this);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &DmFrameWidget::onHandleChangeTheme);
}

void DmFrameWidget::setFrameData()
{
    if (DMDbusHandler::PARTITION == DMDbusHandler::instance()->getCurLevel()) {
        //获取首页相关硬盘数据
        PartitionInfo data = DMDbusHandler::instance()->getCurPartititonInfo();

        QString mountpoints;
        //    DiskInfoData temp;
        //    m_infoData = temp;
        for (QString point : data.m_mountPoints) {
            mountpoints.append(point + " ");
        }

        //    m_infoData.m_mountpoints = diffMountpoints(m_width, mountpoints);
        QString unused = Utils::formatSize(data.m_sectorsUnused, data.m_sectorSize);
        QString used = Utils::formatSize(data.m_sectorsUsed, data.m_sectorSize);
        QString fsTypeName = Utils::fileSystemTypeToString(static_cast<FSType>(data.m_fileSystemType));
        QString partitionSize = Utils::formatSize(data.m_sectorEnd - data.m_sectorStart + 1, data.m_sectorSize);

        if (data.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(data.m_path);
            if (luksInfo.isDecrypt) {
                mountpoints = "";
                for (QString point : luksInfo.m_mapper.m_mountPoints) {
                    mountpoints.append(point + " ");
                }

                unused = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUnused);
                used = Utils::LVMFormatSize(luksInfo.m_mapper.m_fsUsed);
                fsTypeName = DMDbusHandler::instance()->getEncryptionFsType(luksInfo);
            }
        }

        if (mountpoints.contains("�")) {
            mountpoints.remove(mountpoints.mid(mountpoints.indexOf("�")));
            mountpoints.append(m_str);
        }

        m_infoData.m_mountpoints = mountpoints;
        m_infoData.m_unused = unused;
        if (m_infoData.m_unused.contains("-")) {
            m_infoData.m_unused = "-";
        }

        m_infoData.m_used = used;
        if (m_infoData.m_used.contains("-")) {
            m_infoData.m_used = "-";
        }

        m_infoData.m_fstype = fsTypeName;
        m_infoData.m_partitionSize = partitionSize;

        QString partitionPath = data.m_path.remove(0, 5); // 从下标0开始，删除5个字符
        if (data.m_fileSystemLabel == "") {
            m_infoData.m_sysLabel = "";
        } else {
            m_infoData.m_sysLabel = diskVolumn(partitionPath);
            if (m_infoData.m_sysLabel.contains("�")) {
                QString sysLabel = m_infoData.m_sysLabel.split("�").at(0);
                m_infoData.m_sysLabel = sysLabel;
            }//==============gbk编码的中文到在应用下无名称格式化文件系统转换时中文乱码
        }
    }
    update();
}

void DmFrameWidget::setDiskFrameData(const QString &path, const QString &diskType, const QString &used,
                      const QString &unused, const QString &capacity, const QString &interface)
{
    m_infoData.m_mountpoints = path;
    m_infoData.m_unused = unused;
    m_infoData.m_used = used;
    m_infoData.m_fstype = diskType;
    m_infoData.m_partitionSize = capacity;
    m_infoData.m_sysLabel = interface;

    update();
}

QString DmFrameWidget::diskVolumn(QString partitionPath)
{
    DMDbusHandler *handler = DMDbusHandler::instance();
    PartitionInfo curInfo = handler->getCurPartititonInfo();
//    //将gbkｕ盘ｌａｂｅｌ中文乱码转换为正常中文显示
//    QProcess process;
//    process.start("ls", QStringList() << "-al" << "/dev/disk/by-label/");
//    if (!process.waitForStarted()) {
//        qWarning() << "Cmd Exec Failed:" << process.errorString();
//    }
//    if (!process.waitForFinished(-1)) {
//        qWarning() << "waitForFinished Failed:" << process.errorString();
//    }
//    QString standardError = process.readAllStandardOutput();
//    QStringList mountsList = standardError.split("\n").filter(partitionPath);
//    QString sr = mountsList.last();
//    QString st = sr.mid(40).remove(" -> ../../" + partitionPath);
//    qDebug() << __FUNCTION__ << st;
//    if (st.at(1) != "x") {
//        QString strstr1 = st.mid(st.indexOf("\\"));
//        qDebug() << __FUNCTION__ << strstr1 << st.at(1);
//        if (strstr1.at(1) != "x") {
//            QString stres = strstr1;
//        } else {
//            QString stres = st.remove(strstr1);;
//        }
//    }
    QString st = curInfo.m_fileSystemLabel;
//    std::string s = st.toStdString();
//    const char *strstr = s.c_str();
    QString strtem("%1");
    strtem = strtem.arg(st);
//    if (strtem.count("\\x") > 0) {
//        QByteArray arr = strstr;
//        QByteArray ba = strstr;
//        QString link(ba);
//        QByteArray destByteArray;
//        QByteArray tmpByteArray;
//        for (int i = 0; i < ba.size(); i++) {
//            if (92 == ba.at(i)) {
//                if (4 == tmpByteArray.size()) {
//                    destByteArray.append(QByteArray::fromHex(tmpByteArray));
//                } else {
//                    if (tmpByteArray.size() > 4) {
//                        destByteArray.append(QByteArray::fromHex(tmpByteArray.left(4)));
//                        destByteArray.append(tmpByteArray.mid(4));
//                    } else {
//                        destByteArray.append(tmpByteArray);
//                    }
//                }
//                tmpByteArray.clear();
//                tmpByteArray.append(ba.at(i));
//                continue;
//            } else if (tmpByteArray.size() > 0) {
//                tmpByteArray.append(ba.at(i));
//                continue;
//            } else {
//                destByteArray.append(ba.at(i));
//            }
//        }

//        if (4 == tmpByteArray.size()) {
//            destByteArray.append(QByteArray::fromHex(tmpByteArray));
//        } else {
//            if (tmpByteArray.size() > 4) {
//                destByteArray.append(QByteArray::fromHex(tmpByteArray.left(4)));
//                destByteArray.append(tmpByteArray.mid(4));
//            } else {
//                destByteArray.append(tmpByteArray);
//            }
//        }

//        link = QTextCodec::codecForName("GBK")->toUnicode(tmpByteArray);
//        int idx = link.lastIndexOf("/", link.length() - 1);
//        QString stres = link.mid(idx + 1);
//        if (strtem.count("\\x") > 0 && !strtem.contains("�")) {
//            m_str = "-";
//            return  stres;
//        }
//    } else {
//        if (!strtem.contains("�"))
//            m_str = "-";
//        return  st;
//    }
    if (!strtem.contains("�")) {
        m_str = "-";
        return  st;
    }

    return "";
}

void DmFrameWidget::paintEvent(QPaintEvent *event)//绘制首页信息展示表格
{
    QPainter painter(this);
    QWidget::paintEvent(event);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        m_parentPb = DApplicationHelper::instance()->palette(this);
        QColor color = m_parentPb.color(DPalette::Normal, DPalette::ItemBackground);
        painter.setBrush(QBrush(color));
        QColor outsideColor(qRgba(0, 0, 0, 1));
        outsideColor.setAlphaF(0.1);
        painter.setPen(outsideColor);
        QRect curRect = rect();
        curRect.setWidth(curRect.width());
        curRect.setHeight(curRect.height() - 1);
        painter.drawRoundedRect(curRect, 15, 15);
        
        QRect paintRect = QRect(curRect.topLeft().x() + 1, curRect.topLeft().y() + (curRect.height() / 3), curRect.width() - 2, curRect.height() / 3);
        QColor midColor = m_parentPb.color(DPalette::Normal, DPalette::ItemBackground);
        midColor.setAlphaF(0.1);
        painter.setBrush(QBrush(midColor));
        painter.fillRect(paintRect, midColor);
        painter.drawLine(paintRect.width() / 2, curRect.topLeft().y(), paintRect.width() / 2, curRect.bottomLeft().y());
        
        QRect textRect = QRect(curRect.width() / 2 - 300, curRect.topLeft().y() + 12, 240, 35);
        QColor textColor = this->palette().color(DPalette::Normal, DPalette::Text);
        QTextOption option;
        option.setTextDirection(Qt::LeftToRight);
        option.setAlignment(Qt::AlignRight);
        QTextOption option1;
        option1.setAlignment(Qt::AlignRight);
        painter.setPen(textColor);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6, QFont::Normal);
        painter.setFont(font);
        QRect textRect1 = QRect(curRect.width() / 2 - 265, curRect.topLeft().y() + 10, 257, 40);
        QString mountpoints = painter.fontMetrics().elidedText(m_infoData.m_mountpoints, Qt::ElideMiddle, textRect1.width() - 100); // 挂载点过长时，只显示首尾，中间用省略号代替
        painter.drawText(textRect1, mountpoints, option);
//        painter.drawText(textRect1, m_infoData.m_mountpoints, option);
        textRect.moveTo(curRect.width() / 2 - 250, curRect.topLeft().y() + 62);
        painter.drawText(textRect, m_infoData.m_unused, option1);
        textRect.moveTo(curRect.width() / 2 - 250, curRect.topLeft().y() + 113);
        painter.drawText(textRect, m_infoData.m_used, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 10);
        painter.drawText(textRect, m_infoData.m_fstype, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 62);
        painter.drawText(textRect, m_infoData.m_partitionSize, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 113);
        painter.drawText(textRect, m_infoData.m_sysLabel, option1);
        painter.restore();
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        m_parentPb = DApplicationHelper::instance()->palette(this);
        QColor color = m_parentPb.color(DPalette::Normal, DPalette::TextLively);
        color.setAlphaF(0.05);
        painter.setBrush(QBrush(color));
        QColor outsideColor(qRgba(255, 255, 255, 1));
        outsideColor.setAlphaF(0.1);
        painter.setPen(outsideColor);
        QRect curRect = rect();
        curRect.setWidth(curRect.width() - 1);
        curRect.setHeight(curRect.height() - 1);
        painter.drawRoundedRect(curRect, 15, 15);
        
        QRect paintRect = QRect(curRect.topLeft().x() + 1, curRect.topLeft().y() + (curRect.height() / 3), curRect.width() - 2, curRect.height() / 3);
        QColor midColor = QColor("#252525");
        painter.setBrush(QBrush(midColor));
        painter.fillRect(paintRect, midColor);
        painter.drawLine(paintRect.width() / 2, curRect.topLeft().y(), paintRect.width() / 2, curRect.bottomLeft().y());
        
        QRect textRect = QRect(curRect.width() / 2 - 400, curRect.topLeft().y() + 10, 240, 35);
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
        QColor textColor = palette().color(DPalette::Normal, DPalette::WindowText);
        QTextOption option;
        option.setTextDirection(Qt::LeftToRight);
        option.setAlignment(Qt::AlignRight);
        QTextOption option1;
        option1.setAlignment(Qt::AlignRight);
        painter.setPen(textColor);
        QRect textRect1 = QRect(curRect.width() / 2 - 265, curRect.topLeft().y() + 10, 257, 40);
        QString mountpoints = painter.fontMetrics().elidedText(m_infoData.m_mountpoints, Qt::ElideRight, textRect1.width() - 50);
        painter.drawText(textRect1, mountpoints, option);
//        painter.drawText(textRect1, m_infoData.m_mountpoints, option);
        textRect.moveTo(curRect.width() / 2 - 250, curRect.topLeft().y() + 62);
        painter.drawText(textRect, m_infoData.m_unused, option1);
        textRect.moveTo(curRect.width() / 2 - 250, curRect.topLeft().y() + 113);
        painter.drawText(textRect, m_infoData.m_used, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 10);
        painter.drawText(textRect, m_infoData.m_fstype, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 62);
        painter.drawText(textRect, m_infoData.m_partitionSize, option1);
        textRect.moveTo(curRect.width() - 250, curRect.topLeft().y() + 113);
        painter.drawText(textRect, m_infoData.m_sysLabel, option1);
        painter.restore();
    }
}

void DmFrameWidget::resizeEvent(QResizeEvent *event)
{
    //实时获取整体的大小
    QWidget::resizeEvent(event);
    m_width = width();
    setFrameData();
}
void DmFrameWidget::onHandleChangeTheme()
{
    m_parentPb = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
}

//QString DmFrameWidget::diffMountpoints(int width, QString mountpoints)
//{
//    //区分过长的挂载点，中间做...显示
//    QString previoustr;
//    QString laststr;

//    if (mountpoints.size() > 21) {
//        if (width < 1000) {
//            for (int i = 0; i < 9; i++) {
//                previoustr += mountpoints.at(i);
//            }
//            previoustr += "...";
//            for (int p = mountpoints.size() - 9; p < mountpoints.size(); p++) {
//                laststr += mountpoints.at(p);
//            }
//            mountpoints = previoustr + laststr;
//            return mountpoints;
//        } else {
//            return mountpoints;
//        }
//    } else {
//        return mountpoints;
//    }
//}
