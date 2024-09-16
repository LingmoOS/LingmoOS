// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btinfodialog.cpp
 *
 * @brief BT窗口类
 *
 * @date 2020-06-09 10:50
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
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

#include "btinfodialog.h"
#include "aria2rpcinterface.h"
#include "btinfodelegate.h"
#include "btheaderview.h"
#include "btinfotableview.h"
#include "messagebox.h"
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QProcess>
#include <QPixmap>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QSharedMemory>
#include <dpinyin.h>
#include <QFont>
#include <QTimer>

BtInfoDialog::BtInfoDialog(QString torrentFile, QString btLastSavePath,QWidget *parent)
    : DDialog(parent)
    , m_torrentFile(torrentFile)
    , m_defaultDownloadDir(btLastSavePath)
{
    setFixedSize(500, 525);
    setIcon(QIcon::fromTheme(":/icons/icon/downloader3.svg"));
    initUI();
    setObjectName("btInfoDialog");
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged, this, &BtInfoDialog::onPaletteTypeChanged);
}

BtInfoDialog::~BtInfoDialog()
{
    delete m_model;
    delete m_delegate;
    delete m_btnOK;
    delete m_labelTitle;
    delete m_labelSelectedFileNum;
    delete m_labelFileSize;
    delete m_labelCapacityFree;
    delete m_labelInfoName;
    delete m_editDir;
    delete m_checkAll;
    delete m_checkVideo;
    delete m_checkAudio;
    delete m_checkPicture;
    delete m_checkOther;
    delete m_widget;
}

QString BtInfoDialog::getSelected()
{
    QString selected = "";
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString checked = m_model->data(m_model->index(i, 0)).toString();
        if (checked == "1") {
            QString index = m_model->data(m_model->index(i, 4)).toString();
            selected += index;
            selected += ",";
        }
    }
    return selected.mid(0, selected.length() - 1);
}

QString BtInfoDialog::getSaveto()
{
    return m_editDir->text();
}

QString BtInfoDialog::getName()
{
    return m_labelInfoName->text();
}

void BtInfoDialog::initUI()
{
    m_ariaInfo = Aria2RPCInterface::instance()->getBtInfo(m_torrentFile);
//    setTitle(" ");
//    setWindowTitle(tr(""));
    QFont font;
    font.setFamily("Source Han Sans");


    m_labelTitle = new DLabel(this);
    m_labelTitle->setAlignment(Qt::AlignCenter);
    m_labelTitle->setText(tr("New Task"));
    m_labelTitle->setFont(font);
    addContent(m_labelTitle, Qt::AlignHCenter);
    addSpacing(20);
    QHBoxLayout *hlinfo = new QHBoxLayout();
    m_folderIcon = new DLabel(this);
    m_folderIcon->setPixmap(QPixmap(":/icons/icon/folder.svg")); ///usr/share/icons/bloom/places/32/folder.svg
    //下载信息名称
    m_labelInfoName = new DLabel(this);
    m_labelInfoName->setMinimumWidth(400);
    if(m_ariaInfo.name.isEmpty() && !m_ariaInfo.files.isEmpty()){
        m_ariaInfo.name = m_ariaInfo.files[0].path;
    }
    m_labelInfoName->setText(m_ariaInfo.name);
    m_labelInfoName->setFont(font);
    m_labelInfoName->setWordWrap(true);
    hlinfo->addWidget(m_folderIcon);
    hlinfo->addWidget(m_labelInfoName);
    hlinfo->addStretch();
    QWidget *infoWidget = new QWidget(this);
    infoWidget->setLayout(hlinfo);
    addContent(infoWidget);

    //总大小标签
    QFont font2;
    QPalette pal;
    pal.setColor(QPalette::WindowText, QColor("#8AA1B4"));
    m_labelFileSize = new DLabel(this);
    m_labelFileSize->setAlignment(Qt::AlignRight);
    QString size = Aria2RPCInterface::instance()->bytesFormat(m_ariaInfo.totalLengthByets);
    m_labelFileSize->setText(QString(tr("Total ") + size));
    m_labelFileSize->setFont(font2);
    m_labelFileSize->setPalette(pal);

    //选中文件数
    m_labelSelectedFileNum = new DLabel(this);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(m_ariaInfo.files.size())).arg(size));
    m_labelSelectedFileNum->setFont(font2);
    m_labelSelectedFileNum->setPalette(pal);

    //文件列表
    m_widget = new DWidget(this);
    m_tableView = new BtInfoTableView(m_widget);
    m_tableView->setMouseTracking(true);
    m_widget->setAutoFillBackground(true);
    QVBoxLayout *vb = new QVBoxLayout(m_widget);
    vb->setContentsMargins(10, 0, 10, 5);
    vb->addWidget(m_tableView);
    m_widget->setLayout(vb);
    addContent(m_widget);
    QHBoxLayout* hb = new QHBoxLayout;
    hb->addWidget(m_labelSelectedFileNum, Qt::AlignLeft);
    hb->addStretch();
    hb->addWidget(m_labelFileSize, Qt::AlignRight);
    QWidget *SelinfoWidget = new QWidget(this);
    SelinfoWidget->setLayout(hb);
    addContent(SelinfoWidget);

    //Checkbox
    m_checkAll = new DCheckBox(this);
    m_checkAll->setObjectName("checkAll");
    m_checkAll->setGeometry(15, 401, 95, 29);
    m_checkAll->setText(tr("All"));
    m_checkAll->setFont(font);
    m_checkAll->setChecked(true);
    connect(m_checkAll, SIGNAL(clicked()), this, SLOT(onAllCheck()));

    m_checkVideo = new DCheckBox(this);
    m_checkVideo->setObjectName("checkVideo");
    m_checkVideo->setText(tr("Videos"));
    m_checkVideo->setFont(font);
    m_checkVideo->setChecked(true);
    connect(m_checkVideo, SIGNAL(clicked()), this, SLOT(onVideoCheck()));

    m_checkPicture = new DCheckBox(this);
    m_checkPicture->setObjectName("checkPicture");
    m_checkPicture->setText(tr("Pictures"));
    m_checkPicture->setFont(font);
    m_checkPicture->setChecked(true);
    connect(m_checkPicture, SIGNAL(clicked()), this, SLOT(onPictureCheck()));

    m_checkAudio = new DCheckBox(this);
    m_checkAudio->setObjectName("checkAudio");
    m_checkAudio->setText(tr("Music"));
    m_checkAudio->setFont(font);
    m_checkAudio->setChecked(true);
    connect(m_checkAudio, SIGNAL(clicked()), this, SLOT(onAudioCheck()));

    m_checkOther = new DCheckBox(this);
    m_checkOther->setObjectName("checkOther");
    m_checkOther->setText(tr("Others"));
    m_checkOther->setFont(font);
    m_checkOther->setChecked(true);
    connect(m_checkOther, SIGNAL(clicked()), this, SLOT(onOtherCheck()));
    QWidget *checkWidget = new QWidget(this);
    QHBoxLayout *hlyt = new QHBoxLayout();
    checkWidget->setLayout(hlyt);
    hlyt->addWidget(m_checkAll);
    hlyt->addWidget(m_checkVideo);
    hlyt->addWidget(m_checkPicture);
    hlyt->addWidget(m_checkAudio);
    hlyt->addWidget(m_checkOther);
    hlyt->addWidget(m_checkAll);
    addContent(checkWidget);

    //下载路径所在分区剩余磁盘容量

    //
    m_editDir = new DFileChooserEdit(this);
    m_editDir->setFont(font);
    //   QString text = getFileEditText(m_defaultDownloadDir);
    QString flieEditText = tr("Available:") + Aria2RPCInterface::instance()->getCapacityFree(m_defaultDownloadDir);

    m_labelCapacityFree = new DLabel(this);
    m_labelCapacityFree->setText(flieEditText);
    m_labelCapacityFree->setPalette(pal);
    m_labelCapacityFree->setFont(font2);
    QString str = getFileEditText(m_defaultDownloadDir);
    m_editDir->setText(str);
    m_editDir->setClearButtonEnabled(false);
    m_editDir->setFileMode(QFileDialog::DirectoryOnly);
    m_editDir->lineEdit()->setEnabled(false);
    connect(m_editDir, &DFileChooserEdit::fileChoosed, this, &BtInfoDialog::onFilechoosed);
    QList<DSuggestButton *> btnList = m_editDir->findChildren<DSuggestButton *>();
    for (int i = 0; i < btnList.size(); i++) {
        btnList[i]->setToolTip(tr("Change download folder"));
    }
    addContent(m_editDir);
    //确定按钮
    m_btnOK = new DPushButton(this);
    addContent(m_btnOK, Qt::AlignHCenter);
    m_btnOK->setObjectName("sureBtn");
    //this->btnOK->setFixedWidth(190);
    m_btnOK->setText(tr("Download Now"));
    m_btnOK->setFont(font);
    m_btnOK->setAccessibleName("downloadNow");
    connect(m_btnOK, SIGNAL(clicked()), this, SLOT(onBtnOK()));
    //文件列表配置
    m_tableView->setShowGrid(false);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setFrameShape(QAbstractItemView::NoFrame);

    m_tableView->setFont(font);
    m_tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_tableView->horizontalHeader()->setHighlightSections(false);
    m_tableView->horizontalHeader()->setFont(font);

    m_tableView->verticalHeader()->hide();
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
    } else {
        m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
        } else {
            m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
        }
    });
#else
    m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
#endif


    m_delegate = new BtInfoDelegate(this);
    m_tableView->setItemDelegate(m_delegate);

    m_model = new QStandardItemModel();
    m_tableView->setModel(m_model);
    m_tableView->setAccessibleName("btTableView");
    DFontSizeManager::instance()->bind(m_tableView, DFontSizeManager::SizeType::T6, 0);

    m_model->setColumnCount(5);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_model->setHeaderData(1, Qt::Horizontal, "");
    m_model->setHeaderData(2, Qt::Horizontal, tr("Type"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("Size"));
    m_model->setHeaderData(4, Qt::Horizontal, "index");

    for (int i = 0; i < m_ariaInfo.files.length(); i++) {
        Aria2cBtFileInfo file = m_ariaInfo.files.at(i);
        QList<QStandardItem *> list;

        QStandardItem *item1 = new QStandardItem("1");
        item1->setAccessibleText(QString("1_%1").arg(QString::number(i)));

        QStandardItem *item2 = new QStandardItem(file.path.mid(file.path.lastIndexOf("/") + 1));
        item2->setAccessibleText(file.path.mid(file.path.lastIndexOf("/") + 1).append("_").append(QString::number(i)));

        QStandardItem *item3 = new QStandardItem(file.path.mid(file.path.lastIndexOf(".") + 1));
        item3->setAccessibleText(file.path.mid(file.path.lastIndexOf(".") + 1).append("_").append(QString::number(i)));

        QStandardItem *item4 = new QStandardItem(file.length);
        item4->setAccessibleText(file.length.append("_").append(QString::number(i)));

        QStandardItem *item5 = new QStandardItem(QString::number(file.index));
        item5->setAccessibleText(QString::number(file.index).append("_").append(QString::number(i)));

        QStandardItem *item6 = new QStandardItem(QString::number(file.lengthBytes));
        item6->setAccessibleText(QString::number(file.lengthBytes).append("_").append(QString::number(i)));

        list << item1 << item2 << item3 << item4 << item5 << item6;

//        list << new QStandardItem("1");
//        list << new QStandardItem(file.path.mid(file.path.lastIndexOf("/") + 1));
//        list << new QStandardItem(file.path.mid(file.path.lastIndexOf(".") + 1));
//        list << new QStandardItem(file.length);
//        list << new QStandardItem(QString::number(file.index));
//        list << new QStandardItem(QString::number(file.lengthBytes));
        m_model->appendRow(list);
        m_listBtInfo.append(file);
    }

    m_tableView->setColumnHidden(1, true);
    m_tableView->setColumnHidden(4, true);
    m_tableView->setColumnHidden(5, true);

    m_tableView->setColumnWidth(0, 290);
    m_tableView->setColumnWidth(2, 60);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSortingEnabled(false);
    m_tableView->horizontalHeader()->setSortIndicatorShown(true);
    m_tableView->horizontalHeader()->setSectionsClickable(true);

    DFontSizeManager::instance()->bind(m_tableView, DFontSizeManager::SizeType::T6, 0);
    connect(m_tableView, &BtInfoTableView::hoverChanged, m_delegate, &BtInfoDelegate::onhoverChanged);
   // connect(m_tableView->horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(Sort(int)));
    onPaletteTypeChanged(DGuiApplicationHelper::ColorType::LightType);
}

int BtInfoDialog::exec()
{
    QHBoxLayout layout(m_editDir->lineEdit());
    layout.addWidget(m_labelCapacityFree, 0, Qt::AlignRight | Qt::AlignCenter);
    m_editDir->setFixedHeight(36);

    return DDialog::exec();
}

bool BtInfoDialog::onBtnOK()
{
    if (getSelected().isEmpty()) {
        return false;
    }

    long free = Aria2RPCInterface::instance()->getCapacityFreeByte(m_defaultDownloadDir);
    long total = 0; //选中文件总大小（字节）
    for (int i = 0; i < m_model->rowCount(); i++) {
        if (m_model->data(m_model->index(i, 0)).toString() == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
        }
    }
    if (free < (total / 1024)) { //剩余空间比较 KB
        qDebug() << "Disk capacity is not enough!";
        MessageBox msg(this);
        msg.setWarings(tr("Insufficient disk space, please change the download folder"), tr("OK"), tr(""));
        msg.exec();
        return false;
    }
    Settings::getInstance()->setCustomFilePath(m_defaultDownloadDir);
    close();
    accept();
    return true;
}

void BtInfoDialog::onAllCheck()
{
    int state = m_checkAll->checkState();
    if (state == Qt::Checked) {
        for (int i = 0; i < m_model->rowCount(); i++) {
            m_model->setData(m_model->index(i, 0), "1");
        }

        QString size = Aria2RPCInterface::instance()->bytesFormat(m_ariaInfo.totalLengthByets);
        m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(m_model->rowCount())).arg(size));
        m_checkVideo->setCheckState(Qt::Checked);
        m_checkAudio->setCheckState(Qt::Checked);
        m_checkPicture->setCheckState(Qt::Checked);
        m_checkOther->setCheckState(Qt::Checked);
        setOkBtnStatus(m_model->rowCount());
    } else if (state == Qt::Unchecked) {
        for (int i = 0; i < m_model->rowCount(); i++) {
            m_model->setData(m_model->index(i, 0), "0");
            m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(0)).arg("0B"));
        }
        m_checkVideo->setCheckState(Qt::Unchecked);
        m_checkAudio->setCheckState(Qt::Unchecked);
        m_checkPicture->setCheckState(Qt::Unchecked);
        m_checkOther->setCheckState(Qt::Unchecked);
        setOkBtnStatus(0);
    }
}

bool BtInfoDialog::isVideo(QString ext)
{
    QString types = "avi,mp4,mkv,flv,f4v,wmv,rmvb,rm,mpeg,mpg,mov,ts,m4v,vob";
    return types.indexOf(ext) != -1;
}

bool BtInfoDialog::isAudio(QString ext)
{
    QString types = "mp3,ogg,wav,ape,flac,wma,midi,aac,cda";
    return types.indexOf(ext) != -1;
}

bool BtInfoDialog::isPicture(QString ext)
{
    QString types = "jpg,jpeg,gif,png,bmp,svg,psd,tif,ico";
    return types.indexOf(ext) != -1;
}

void BtInfoDialog::onVideoCheck()
{
    int state = m_checkVideo->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2)).toString();
        if (isVideo(ext)) {
            m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
        }
        if (m_model->data(m_model->index(i, 0)).toString() == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    setOkBtnStatus(cnt);
}

void BtInfoDialog::onAudioCheck()
{
    int state = m_checkAudio->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2)).toString();
        if (isAudio(ext)) {
            m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
        }
        if (m_model->data(m_model->index(i, 0)).toString() == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    setOkBtnStatus(cnt);
}

void BtInfoDialog::onPictureCheck()
{
    int state = m_checkPicture->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2)).toString();
        if (isPicture(ext)) {
            m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
        }
        if (m_model->data(m_model->index(i, 0)).toString() == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    setOkBtnStatus(cnt);
}

void BtInfoDialog::onOtherCheck()
{
    int state = m_checkOther->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }
    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2)).toString();
        if (!isVideo(ext) && !isAudio(ext) && !isPicture(ext)) {
            m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
        }
        if (m_model->data(m_model->index(i, 0)).toString() == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    setOkBtnStatus(cnt);
}

void BtInfoDialog::updateSelectedInfo()
{
    int cnt = 0;
    long total = 0;
    int selectVideoCount = 0;
    int selectAudioCount = 0;
    int selectPictureCount = 0;
    int selectOtherCount = 0;
    int allVideo = 0;
    int allAudio = 0;
    int allPic = 0;
    int allOther = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString isCheck = m_model->data(m_model->index(i, 0)).toString();
        QString type = m_model->data(m_model->index(i, 2)).toString();
        if (isCheck == "1") {
            total += m_model->data(m_model->index(i, 5)).toString().toLong();
            if (isVideo(type)) {
                selectVideoCount++;
            } else if (isAudio(type)) {
                selectAudioCount++;
            } else if (isPicture(type)) {
                selectPictureCount++;
            } else {
                selectOtherCount++;
            }
            cnt++;
        }
    }
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString type = m_model->data(m_model->index(i, 2)).toString();
        if (isVideo(type)) {
            allVideo++;
        } else if (isAudio(type)) {
            allAudio++;
        } else if (isPicture(type)) {
            allPic++;
        } else {
            allOther++;
        }
    }
    allVideo == selectVideoCount &&allVideo > 0 ? m_checkVideo->setCheckState(Qt::Checked) : m_checkVideo->setCheckState(Qt::Unchecked);
    allAudio == selectAudioCount &&allAudio > 0 ? m_checkAudio->setCheckState(Qt::Checked) : m_checkAudio->setCheckState(Qt::Unchecked);
    allPic == selectPictureCount &&allPic > 0 ? m_checkPicture->setCheckState(Qt::Checked) : m_checkPicture->setCheckState(Qt::Unchecked);
    allOther == selectOtherCount &&allOther > 0 ? m_checkOther->setCheckState(Qt::Checked) : m_checkOther->setCheckState(Qt::Unchecked);
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    cnt == m_model->rowCount() ? m_checkAll->setCheckState(Qt::Checked) : m_checkAll->setCheckState(Qt::Unchecked);
    setOkBtnStatus(cnt);
}

void BtInfoDialog::onFilechoosed(const QString &filename)
{
    QFileInfo fileinfo;
    QString strPath;
    fileinfo.setFile(filename);
    if (!fileinfo.isWritable()) {
        MessageBox msg(this);
        msg.setFolderDenied();
        msg.exec();
        strPath = m_editDir->directoryUrl().toString();
        QString text = getFileEditText(m_defaultDownloadDir);
        m_editDir->lineEdit()->setText(text);
        m_editDir->setDirectoryUrl(m_defaultDownloadDir);
        return;
    }
    //获取到更改后的大小
    QString freeSize = Aria2RPCInterface::instance()->getCapacityFree(filename);
    QPalette pal;
    pal.setColor(QPalette::WindowText, QColor("#8AA1B4"));
    QFont font;
    m_labelCapacityFree->setText(QString(tr("Available:") + freeSize));
    m_labelCapacityFree->setPalette(pal);
    m_labelCapacityFree->setFont(font);
    QString text = getFileEditText(filename);
    m_editDir->lineEdit()->setText(text);
    m_editDir->setDirectoryUrl(filename);
    m_defaultDownloadDir = filename;
}

void BtInfoDialog::onPaletteTypeChanged(DGuiApplicationHelper::ColorType type)
{
    int themeType = DGuiApplicationHelper::instance()->themeType();
    QPalette p;
    m_delegate->setHoverColor(DGuiApplicationHelper::instance()->applicationPalette().frameBorder());

    if (themeType == 1) {
        p.setColor(QPalette::Background, Qt::white);
        m_delegate->setHoverColor(QColor(0, 0, 0, 13));

        QPalette pal;
        pal.setColor(QPalette::WindowText, QColor("#8AA1B4"));
        m_labelFileSize->setPalette(pal);
        m_labelSelectedFileNum->setPalette(pal);
    } else {
        p = DGuiApplicationHelper::instance()->applicationPalette();
        m_delegate->setHoverColor(QColor(255, 255, 255, 26));

        m_labelFileSize->setPalette(DGuiApplicationHelper::instance()->applicationPalette());
        m_labelSelectedFileNum->setPalette(DGuiApplicationHelper::instance()->applicationPalette());
    }
    m_widget->setPalette(p);
}

void BtInfoDialog::getBtInfo(QMap<QString, QVariant> &opt, QString &infoName, QString &infoHash)
{
    opt.insert("dir", m_editDir->text());
    opt.insert("select-file", getSelected());
    infoName = m_labelInfoName->text();
    infoHash = m_ariaInfo.infoHash;
}

QString BtInfoDialog::getFileEditText(QString text)
{
    QString flieEditText = text + "    " + tr("Available:") + Aria2RPCInterface::instance()->getCapacityFree(text);
    int count = text.count();
    int hasLongStr = 0;

    for (int i = 0; i < flieEditText.size(); i++) {
        //判断字符中是否包含中文或者大写字母
        if ((flieEditText[i] >= 'A' && flieEditText[i] <= 'Z')
            || (flieEditText[i] >= 0x4E00 && flieEditText[i] <= 0x9FA5)) {
            count++;
        }
    }
    if (count > 45) {
        text = text.right(45);
    }
    for (int i = 0; i < text.size(); i++) {
        //判断字符中是否包含中文或者大写字母
        if ((text[i] >= 'A' && text[i] <= 'Z')
            || (text[i] >= 0x4E00 && text[i] <= 0x9FA5)) {
            hasLongStr++;
        }
    }
    text = text.right(45 - hasLongStr / 2);
    return text;
}

void BtInfoDialog::setOkBtnStatus(int count)
{
    if (count == 0) {
        m_btnOK->setEnabled(false);
    } else {
        m_btnOK->setEnabled(true);
    }
}

void BtInfoDialog::closeEvent(QCloseEvent *event)
{
    QTimer::singleShot(5000, this->parent(), [=](){
        QSharedMemory sharedMemory;
        sharedMemory.setKey("downloader");
        if (sharedMemory.attach()) //设置成单例程序
        {
            sharedMemory.lock();
            char *to = static_cast<char *>(sharedMemory.data());
            int num = sharedMemory.size();
            memset(to, 0, num);
            sharedMemory.unlock();
        }
    });
}
