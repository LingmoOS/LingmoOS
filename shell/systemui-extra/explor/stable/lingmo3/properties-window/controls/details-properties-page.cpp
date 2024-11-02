/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#include "details-properties-page.h"

#include "file-info.h"
#include "file-utils.h"

#include <QLabel>
#include <QtConcurrent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImageReader>
#include "linux-pwd-helper.h"
#include "global-settings.h"
#include "file-watcher.h"
#include "global-settings.h"

using namespace Peony;

#define FIXED_LABEL_WIDTH 150
//460 - 150 - 22 - 22 =
#define FIXED_CONTENT_WIDTH 266

DetailsPropertiesPage::DetailsPropertiesPage(const QString &uri, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_uri = uri;

    m_watcher = std::make_shared<FileWatcher>(m_uri);
    m_watcher->startMonitor();

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,10,0,0);
    m_tableWidget = new QTableWidget(this);

    this->initTableWidget();

    m_layout->addWidget(m_tableWidget);
    this->setLayout(m_layout);

    //FIXME: replace BLOCKING api in ui thread.
    this->getFIleInfo();

    this->initDetailsPropertiesPage();
}

void DetailsPropertiesPage::getFIleInfo()
{
    m_fileInfo = FileInfo::fromUri(m_uri);
    FileInfoJob *fileInfoJob = new FileInfoJob(m_fileInfo);
    fileInfoJob->setAutoDelete(true);
    fileInfoJob->querySync();
}

QLabel *DetailsPropertiesPage::createFixedLabel(quint64 minWidth, quint64 minHeight, QString text, QWidget *parent)
{
    QLabel *l = new QLabel(parent);
    if(minWidth != 0)
        l->setMinimumWidth(minWidth);
    if(minHeight != 0)
        l->setMinimumHeight(minHeight);

    l->setText(text);
    return l;
}

QWidget *DetailsPropertiesPage::createTableRow(QString labelText, QLabel *contentLabel)
{
    QWidget *row = new QWidget(m_tableWidget);
    QHBoxLayout *boxLayout = new QHBoxLayout(row);

    boxLayout->setMargin(0);
    row->setLayout(boxLayout);

    QLabel *label1 = this->createFixedLabel(FIXED_LABEL_WIDTH,0,labelText,row);
    label1->setContentsMargins(22,0,0,0);

    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    contentLabel->setCursor(Qt::IBeamCursor);
    boxLayout->addWidget(label1);
    boxLayout->addWidget(contentLabel);
    boxLayout->addStretch(1);

    return row;
}

QWidget *DetailsPropertiesPage::createTableRow(QString labelText, QString content)
{
    QWidget *row = new QWidget(m_tableWidget);
    QHBoxLayout *boxLayout = new QHBoxLayout(row);
    boxLayout->setMargin(0);

    row->setLayout(boxLayout);
    QLabel *label1 = this->createFixedLabel(FIXED_LABEL_WIDTH,0,labelText,row);
    label1->setContentsMargins(22,0,0,0);

    QLabel *labelContent = this->createFixedLabel(0,0,content,row);
    labelContent->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelContent->setCursor(Qt::IBeamCursor);
    boxLayout->addWidget(label1);
    boxLayout->addWidget(labelContent);
    boxLayout->addStretch(1);

    return row;
}

void DetailsPropertiesPage::addRow(QString labelText, QString content)
{
    int rowCount = m_tableWidget->rowCount();
    m_tableWidget->setRowCount( rowCount + 1);

    m_tableWidget->setCellWidget((rowCount - 1),0,createTableRow(labelText, content));
}

void DetailsPropertiesPage::addRow(QString labelText, QLabel *contentLabel)
{
    int rowCount = m_tableWidget->rowCount();
    m_tableWidget->setRowCount( rowCount + 1);

    m_tableWidget->setCellWidget((rowCount - 1),0,createTableRow(labelText, contentLabel));
}

void DetailsPropertiesPage::initTableWidget()
{
    m_tableWidget->setColumnCount(1);
    m_tableWidget->setRowCount(1);

    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setVisible(false);

    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_tableWidget->setFrameShape(QFrame::NoFrame);
    m_tableWidget->setSelectionMode(QTableWidget::NoSelection);
    m_tableWidget->setShowGrid(false);
    m_tableWidget->setAlternatingRowColors(true);

    m_tableWidget->rowHeight(36);

    m_tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
}

void DetailsPropertiesPage::initDetailsPropertiesPage()
{
    if (!m_tableWidget)
        return;

    QFontMetrics fm = this->fontMetrics();

    //name
    QString fileName =  m_fileInfo->displayName();
    m_nameLabel = this->createFixedLabel(0, 0, "", m_tableWidget);
    if (fm.width(fileName) > FIXED_CONTENT_WIDTH) {
        m_nameLabel->setToolTip(fileName);
        fileName = m_tableWidget->fontMetrics().elidedText(fileName, Qt::ElideMiddle, FIXED_CONTENT_WIDTH);
    }
    m_nameLabel->setText(fileName);
    this->addRow(tr("Name:"), m_nameLabel);

    //type
    this->addRow(tr("File type:"),m_fileInfo->fileType());

    //location
    QUrl url = FileUtils::getParentUri(m_fileInfo->uri());
    QString location = url.toDisplayString();
    if (location.startsWith("file://"))
        location = location.split("file://").last();

    m_localLabel = this->createFixedLabel(0, 0, "", m_tableWidget);
    if (fm.width(location) > FIXED_CONTENT_WIDTH)
    {
        m_localLabel->setToolTip(location);
        location =  m_tableWidget->fontMetrics().elidedText(location, Qt::ElideMiddle,FIXED_CONTENT_WIDTH);
    }
    m_localLabel->setText(location);

    this->addRow(tr("Location:"), m_localLabel);

    //createTime
    if (m_fileInfo->isDir())
    {
      m_createDateLabel = this->createFixedLabel(0,0,"",m_tableWidget);
      this->addRow(tr("Create time:"),m_createDateLabel);
    }

    //modifiedTime
    m_modifyDateLabel = this->createFixedLabel(0,0,"",m_tableWidget);
    this->addRow(tr("Modify time:"),m_modifyDateLabel);

    //default format
    this->setSystemTimeFormat(tr("yyyy-MM-dd, HH:mm:ss"));
    // set time
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (LINGMO_CONTROL_CENTER_PANEL_PLUGIN_TIME == key) {
//            if ("12" == GlobalSettings::getInstance()->getValue(key)) {
//                setSystemTimeFormat(tr("yyyy-MM-dd, hh:mm:ss AP"));
//            } else if ("24" == GlobalSettings::getInstance()->getValue(key)) {
//                setSystemTimeFormat(tr("yyyy-MM-dd, HH:mm:ss"));
//            }
            updateFileInfo(m_fileInfo.get()->uri());
        }
    });

#ifdef LINGMO_SDK_DATE
    connect(GlobalSettings::getInstance(),
            &GlobalSettings::updateLongDataFormat,
            this,
            &DetailsPropertiesPage::updateDateFormat);
#endif

    //size
    this->addRow(tr("File size:"),m_fileInfo->fileSize());

    //判断文件类型
    if (m_fileInfo->isImageFile()) {
        if (m_fileInfo->canRead()) {
            //image info
            m_imageWidthLabel = this->createFixedLabel(0, 0, "", m_tableWidget);
            this->addRow(tr("Width:"), m_imageWidthLabel);

            m_imageHeightLabel = this->createFixedLabel(0, 0, "", m_tableWidget);
            this->addRow(tr("Height:"), m_imageHeightLabel);

            m_imageDepthLabel = this->createFixedLabel(0, 0, "", m_tableWidget);
            //FIXME:缺少图片位深
//        this->addRow(tr("Depth:"),m_imageDepthLabel);
        }
    }

    m_ownerLabel = this->createFixedLabel(0,0,tr("Owner"),m_tableWidget);
    this->addRow(tr("Owner:"), m_ownerLabel);

    m_computerLabel = this->createFixedLabel(0,0,tr("Computer"),m_tableWidget);
    this->addRow(tr("Computer:"), m_computerLabel);

    m_tableWidget->hideRow(m_tableWidget->rowCount() - 1);
    this->updateFileInfo(m_uri);

    connect(m_watcher.get(), &FileWatcher::locationChanged, [=](const QString&, const QString &uri) {
        this->updateFileInfo(m_uri);
    });

}

DetailsPropertiesPage::~DetailsPropertiesPage()
{
}

void DetailsPropertiesPage::saveAllChange()
{
    if (!m_thisPageChanged)
        return;

}

void DetailsPropertiesPage::setSystemTimeFormat(QString format)
{
    this->m_systemTimeFormat = format;
}

void DetailsPropertiesPage::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        updateFileInfo(m_fileInfo.get()->uri());
        m_date_format = dateFormat;
    }
}

void DetailsPropertiesPage::updateFileInfo(const QString &uri)
{
    this->getFIleInfo();
    QUrl url(uri);
    if(uri.startsWith("filesafe:///")){
        QUrl newUrl = QUrl(m_fileInfo->targetUri());
        url = newUrl;
    }
    QFontMetrics fm = this->fontMetrics();

    //FIXME:暂时不处理除了本地文件外的文件信息,希望添加对其他文件的支持
    if (url.isLocalFile()) {
        QString path = url.path();
        QFileInfo qFileInfo(path);

        m_ownerLabel->setText(qFileInfo.owner());
        //FIXME:明确当前文件所属计算机
        if (qFileInfo.isNativePath()) {
            QString str_m_computerLabel = tr("%1 (this computer)").arg(LinuxPWDHelper::localHost());
            if (fm.width(str_m_computerLabel) > FIXED_CONTENT_WIDTH) {
                m_computerLabel->setToolTip(str_m_computerLabel);
                str_m_computerLabel = m_tableWidget->fontMetrics().elidedText(str_m_computerLabel, Qt::ElideMiddle, FIXED_CONTENT_WIDTH);
            }
            m_computerLabel->setText(str_m_computerLabel);
        } else {
            m_computerLabel->setText(tr("Unknown"));
        }

        //FIXME:文件的创建时间会随着文件被修改而发生改变，甚至会出现创建时间晚于修改时间问题 后期将qt的方法替换为gio的方法
        //参考：https://www.oschina.net/news/126468/gnome-40-alpha-preview
        if (qFileInfo.isDir() && m_createDateLabel)
        {
            QDateTime date1 = qFileInfo.birthTime();
            QString time1 = date1.toString(m_systemTimeFormat);
            m_createDateLabel->setText(time1);
        }

        GFile     *file = g_file_new_for_uri(uri.toUtf8().constData());
        GFileInfo *info = g_file_query_info(file,
                                            "time::*",
                                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                            nullptr,
                                            nullptr);
        g_object_unref(file);

        quint64 timeNum2 = g_file_info_get_attribute_uint64(info,"time::modified");
//        QDateTime date2 = QDateTime::fromMSecsSinceEpoch(timeNum2*1000);
//        QString time2 = date2.toString(m_systemTimeFormat);
        QString time2 = GlobalSettings::getInstance()->transToSystemTimeFormat(timeNum2, true);
        m_modifyDateLabel->setText(time2);

        g_object_unref(info);

    } else {
        if (m_createDateLabel)
            m_createDateLabel->setText(tr("Can't get remote file information"));
        m_modifyDateLabel->setText(tr("Can't get remote file information"));
    }

    //image file
    if (m_fileInfo->isImageFile()) {
        //image width
        QImageReader r(url.path());

        if (m_imageHeightLabel && m_imageWidthLabel && m_imageDepthLabel) {
            m_imageWidthLabel->setText(tr("%1 px").arg(r.size().width()));
            m_imageHeightLabel->setText(tr("%1 px").arg(r.size().height()));
            //FIXME:获取图片文件的位深
//                m_imageDepthLabel->setText(32);
        }
    }
}
