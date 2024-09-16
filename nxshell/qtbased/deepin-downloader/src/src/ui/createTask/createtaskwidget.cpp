// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file newtaskwidget.cpp
 *
 * @brief 新建任务类
 *
 * @date 2020-06-09 10:52
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

#include "createtaskwidget.h"
#include <QPalette>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QMutex>
#include <QThread>
#include <QStandardPaths>
#include <QProcess>
#include <QStandardItemModel>
#include <QMimeDatabase>
#include <QDesktopWidget>
#include <QFont>
#include "btinfodialog.h"
#include "messagebox.h"
#include "btinfotableview.h"
#include "taskdelegate.h"
#include "btheaderview.h"
#include "analysisurl.h"
#include "func.h"
#include "../database/dbinstance.h"
#include "aria2rpcinterface.h"
#include "taskModel.h"

CreateTaskWidget::CreateTaskWidget(QWidget *parent)
    : DDialog(parent)
    , m_analysisUrl(new AnalysisUrl)
{
    setObjectName("newTaskWidget");
    //DDialog 内将回车键绑定在action上，所以在解析框内无法出发回车函数，下列代码将DDialog中绑定回车的aciton删除
    QObjectList objectList = children();
    for (int i = 0; i < objectList.size(); i++) {
        QAction *action = qobject_cast<QAction *>(objectList[i]);
        if (action == nullptr) {
            continue;
        }
        if (!action->autoRepeat()) {
            delete action;
            action = nullptr;
        }
    }
    initUi();
}

CreateTaskWidget::~CreateTaskWidget()
{
//    delete m_delegate;
    if(m_analysisUrl != nullptr){
        delete m_analysisUrl;
        m_analysisUrl = nullptr;
    }
    if(m_delegate != nullptr){
        delete m_delegate;
        m_delegate = nullptr;
    }

}

void CreateTaskWidget::initUi()
{
    setCloseButtonVisible(true);
    setAcceptDrops(true);


    QIcon tryIcon(QIcon::fromTheme(":/icons/icon/downloader2.svg"));
    setIcon(tryIcon);
    setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    //setTitle(tr("New Task"));

    QFont font;
    font.setFamily("Source Han Sans");

    DLabel *msgTitle = new DLabel(this);
    QString titleMsg = tr("New Task");

    msgTitle->setText(titleMsg);
    msgTitle->setAccessibleName("newTask");
    msgTitle->setFont(font);
    addContent(msgTitle, Qt::AlignHCenter);
    QString msg = tr("Please enter one URL per line");
    addSpacing(10);
    DLabel *msgLab = new DLabel(this);
    msgLab->setText(msg);
    msgLab->setFont(font);
    addContent(msgLab, Qt::AlignHCenter);
    addSpacing(10);
    m_texturl = new DTextEdit(this);
    m_texturl->setObjectName("textUrl");
    m_texturl->setAccessibleName("textUrl");
    m_texturl->setReadOnly(false);
    m_texturl->setAcceptDrops(false);
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland")) {
        m_texturl->setAcceptRichText(false);
    }

    m_texturl->setPlaceholderText(tr("Enter download links or drag a torrent file here"));
    m_texturl->setMinimumSize(QSize(500, 154));
    m_texturl->setFont(font);
    m_texturl->setWordWrapMode(QTextOption::NoWrap);
    // m_texturl->document()->setMaximumBlockCount(60);
    connect(m_texturl, &DTextEdit::textChanged, this, &CreateTaskWidget::onTextChanged);
   // connect(m_texturl, &D, this, &CreateTaskWidget::onTextChanged);
    QPalette pal;
    pal.setColor(QPalette::Base, QColor(0, 0, 0, 20));
    m_texturl->setPalette(pal);
    addContent(m_texturl);
    addSpacing(10);

    m_tableView = new BtInfoTableView(this);
    m_tableView->setObjectName("tableView");
    m_tableView->setAccessibleName("taskTableView");
    m_tableView->setMouseTracking(true);
    m_tableView->setShowGrid(false);
    m_tableView->setFrameShape(QAbstractItemView::NoFrame);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setFont(font);
    headerView *header = new headerView(Qt::Horizontal, m_tableView);
    m_tableView->setHorizontalHeader(header);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->setHighlightSections(false);
    m_tableView->verticalHeader()->hide();
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
        m_texturl->setMinimumHeight(154);
    } else {
        m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
        m_texturl->setMinimumHeight(184);
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
            m_texturl->setMinimumHeight(154);
        } else {
            m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
            m_texturl->setMinimumHeight(184);
        }
    });
#else
    m_tableView->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
#endif
    m_delegate = new TaskDelegate(this);
    m_delegate->setObjectName("taskDelegate");
    m_tableView->setItemDelegate(m_delegate);
    m_model = new TaskModel(this);
    m_tableView->setModel(m_model);

    m_tableView->setColumnHidden(1, true);
    m_tableView->setColumnHidden(4, true);
    m_tableView->setColumnHidden(5, true);
    m_tableView->setColumnHidden(7, true);

    m_tableView->setColumnWidth(0, 290);
    m_tableView->setColumnWidth(2, 60);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setFont(font);
    DFontSizeManager::instance()->bind(m_tableView, DFontSizeManager::SizeType::T6, 0);
    connect(m_tableView, &BtInfoTableView::hoverChanged, m_delegate, &TaskDelegate::onhoverChanged);
    addContent(m_tableView);

    QWidget *labelWidget = new QWidget(this);
    QHBoxLayout *hllyt = new QHBoxLayout(labelWidget);
    //总大小标签
    QFont font2;
    QPalette pal2;
    pal2.setColor(QPalette::WindowText, QColor("#8AA1B4"));
    m_labelFileSize = new DLabel(this);
    m_labelFileSize->setAccessibleName("fileSizeLabel");
    m_labelFileSize->setAlignment(Qt::AlignRight);
    m_labelFileSize->setText(QString(tr("Total ") + "0"));
    m_labelFileSize->setFont(font2);
    m_labelFileSize->setPalette(pal2);

    //选中文件数
    m_labelSelectedFileNum = new DLabel(this);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(0)).arg("0"));
    m_labelSelectedFileNum->setFont(font2);
    m_labelSelectedFileNum->setPalette(pal2);
    hllyt->addWidget(m_labelSelectedFileNum);
    hllyt->addStretch();
    hllyt->addWidget(m_labelFileSize);
    labelWidget->setLayout(hllyt);
    addContent(labelWidget);

    //Checkbox
    m_checkWidget = new QWidget(this);
    QHBoxLayout *hlyt = new QHBoxLayout(m_checkWidget);
    m_checkAll = new DCheckBox(this);
    m_checkAll->setText(tr("All"));
    m_checkAll->setFont(font);
    m_checkAll->setObjectName("checkAll");
    connect(m_checkAll, SIGNAL(clicked()), this, SLOT(onAllCheck()));

    m_checkVideo = new DCheckBox(this);
    m_checkVideo->setText(tr("Videos"));
    m_checkVideo->setFont(font);
    m_checkVideo->setObjectName("checkVideo");
    connect(m_checkVideo, SIGNAL(clicked()), this, SLOT(onVideoCheck()));

    m_checkPicture = new DCheckBox(this);
    m_checkPicture->setText(tr("Pictures"));
    m_checkPicture->setFont(font);
    m_checkPicture->setObjectName("checkPicture");
    connect(m_checkPicture, SIGNAL(clicked()), this, SLOT(onPictureCheck()));

    m_checkAudio = new DCheckBox(this);
    m_checkAudio->setText(tr("Music"));
    m_checkAudio->setFont(font);
    m_checkAudio->setObjectName("checkAudio");
    connect(m_checkAudio, SIGNAL(clicked()), this, SLOT(onAudioCheck()));

    m_checkOther = new DCheckBox(this);
    m_checkOther->setText(tr("Others"));
    m_checkOther->setFont(font);
    m_checkOther->setObjectName("checkOther");
    connect(m_checkOther, SIGNAL(clicked()), this, SLOT(onOtherCheck()));

    m_checkDoc = new DCheckBox(this);
    m_checkDoc->setText(tr("Documents"));
    m_checkDoc->setFont(font);
    m_checkDoc->setObjectName("checkDoc");
    connect(m_checkDoc, SIGNAL(clicked()), this, SLOT(onDocCheck()));

    m_checkZip = new DCheckBox(this);
    m_checkZip->setText(tr("Archives"));
    m_checkZip->setFont(font);
    m_checkZip->setObjectName("checkZip");
    connect(m_checkZip, SIGNAL(clicked()), this, SLOT(onZipCheck()));

    hlyt->addWidget(m_checkAll);
    hlyt->addWidget(m_checkVideo);
    hlyt->addWidget(m_checkPicture);
    hlyt->addWidget(m_checkAudio);
    hlyt->addWidget(m_checkDoc);
    hlyt->addWidget(m_checkZip);
    hlyt->addWidget(m_checkOther);

    m_checkWidget->setLayout(hlyt);
    addContent(m_checkWidget);
    addSpacing(4);

    //下载路径所在分区剩余磁盘容量
    m_defaultDownloadDir = Settings::getInstance()->getDownloadSavePath();
    QHBoxLayout *siezhlyt = new QHBoxLayout(this);
    siezhlyt->addStretch();
    QPalette palFree;
    palFree.setColor(QPalette::WindowText, QColor("#8AA1B4"));
    QFont fontFree;
    m_labelCapacityFree = new DLabel();
    QString freeSize = Aria2RPCInterface::instance()->getCapacityFree(m_defaultDownloadDir);
    m_labelCapacityFree->setPalette(palFree);
    m_labelCapacityFree->setFont(fontFree);
    m_labelCapacityFree->setText(QString(tr("Available:") + freeSize));
    siezhlyt->addWidget(m_labelCapacityFree, 0, Qt::AlignRight);

    m_editDir = new DFileChooserEdit(this);
    m_editDir->lineEdit()->setReadOnly(true);
    m_editDir->lineEdit()->setClearButtonEnabled(false);
    m_editDir->lineEdit()->setEnabled(false);
    m_editDir->lineEdit()->setTextMargins(0, 0, m_editDir->lineEdit()->width(), 0);
    m_editDir->lineEdit()->setLayout(siezhlyt);
    m_editDir->setFont(font);
    m_editDir->setFileMode(QFileDialog::FileMode::DirectoryOnly);
    connect(m_editDir, &DFileChooserEdit::fileChoosed, this, &CreateTaskWidget::onFilechoosed);

    m_editDir->setDirectoryUrl(QUrl(m_defaultDownloadDir));
    m_editDir->setText(m_defaultDownloadDir);

    for(int i = 0; i < m_editDir->children().count(); i++){
        for (int j = 0; j < m_editDir->children().at(i)->children().count(); j++) {
            DSuggestButton * dsbtn = qobject_cast<DSuggestButton*>(m_editDir->children().at(i)->children().at(j));
            if(dsbtn != nullptr){
                connect(dsbtn, &DSuggestButton::released, [=](){
                    for(int k = 0; k < m_editDir->children().count(); k++){
                        auto temp = qobject_cast<QFileDialog *>(m_editDir->children().at(k));
                        if(temp != nullptr) {
                           temp->setDirectory(m_defaultDownloadDir);
                        }
                    }
                });
                break;
            }
        }
    }

    QList<DSuggestButton *> btnList = m_editDir->findChildren<DSuggestButton *>();
    for (int i = 0; i < btnList.size(); i++) {
        btnList[i]->setToolTip(tr("Change download folder"));
    }
    addContent(m_editDir);
    addSpacing(10);

    QWidget *boxBtn = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(boxBtn);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    DIconButton *iconBtn = new DIconButton(boxBtn);
    iconBtn->setAccessibleName("bticonBtn");
    //  QIcon tryIcon1(QIcon::fromTheme("dcc_bt"));
    iconBtn->setIcon(QIcon::fromTheme("dcc_bt"));
    connect(iconBtn, &DIconButton::clicked, this, &CreateTaskWidget::onFileDialogOpen);
    iconBtn->setToolTip(tr("Select file"));
    layout->addWidget(iconBtn);

    DIconButton *mlBtn = new DIconButton(boxBtn);
    iconBtn->setAccessibleName("mliconBtn");
    //  QIcon tryIcon1(QIcon::fromTheme("dcc_bt"));
    mlBtn->setIcon(QIcon::fromTheme("dcc_ml"));
    connect(mlBtn, &DIconButton::clicked, this, &CreateTaskWidget::onMLFileDialogOpen);
    mlBtn->setToolTip(tr("Select file"));
    layout->addWidget(mlBtn);

    QWidget *rightBox = new QWidget(boxBtn);
    QHBoxLayout *layout_right = new QHBoxLayout(rightBox);
    layout->setSpacing(10);
    layout_right->setContentsMargins(0, 0, 0, 0);

    DPushButton *cancelButton = new DPushButton(boxBtn);
    cancelButton->setAccessibleName("cancelButton");
    QSizePolicy policy;
    policy = cancelButton->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    cancelButton->setSizePolicy(policy);
    cancelButton->setText(tr("Cancel", "button"));
    cancelButton->setObjectName("cancelButton");
    cancelButton->setFont(font);
    connect(cancelButton, &DPushButton::clicked, this, &CreateTaskWidget::onCancelBtnClicked);
    layout_right->addWidget(cancelButton);

    m_sureButton = new DSuggestButton(boxBtn);
    m_sureButton->setText(tr("Confirm", "button"));
    m_sureButton->setObjectName("sureBtn");
    m_sureButton->setFont(font);
    policy = m_sureButton->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_sureButton->setSizePolicy(policy);
    m_sureButton->setAccessibleName("createSureBtn");
    connect(m_sureButton, &DPushButton::clicked, this, &CreateTaskWidget::onSureBtnClicked);
    layout_right->addSpacing(10);
    layout_right->addWidget(m_sureButton);
    layout->addWidget(rightBox);
    addContent(boxBtn);

    setMaximumSize(541, 575);
    setMinimumSize(541, 575);
    connect(m_analysisUrl, SIGNAL(sendFinishedUrl(LinkInfo *)), this, SLOT(updataTabel(LinkInfo *)));
}

void CreateTaskWidget::onFileDialogOpen()
{
    QString btFile = DFileDialog::getOpenFileName(this, tr("Choose Torrent File"), QDir::homePath(), "*.torrent");
    if (btFile != "") {
        hide();
        BtInfoDialog dialog(btFile, m_defaultDownloadDir, this); //= new BtInfoDialog(); //torrent文件路径
        if (dialog.exec() == QDialog::Accepted) {
            QMap<QString, QVariant> opt;
            opt.clear();
            QString infoName;
            QString infoHash;
            dialog.getBtInfo(opt, infoName, infoHash);
            emit downLoadTorrentCreate(btFile, opt, infoName, infoHash);
        }
        close();
    }
}

void CreateTaskWidget::onMLFileDialogOpen()
{
    QString mlFile = DFileDialog::getOpenFileName(this, tr("Choose Torrent File"), QDir::homePath(), "*.metalink");
    if (mlFile != "") {
        hide();
        BtInfoDialog dialog(mlFile, m_defaultDownloadDir, this); //= new BtInfoDialog(); //torrent文件路径
        if (dialog.exec() == QDialog::Accepted) {
            QMap<QString, QVariant> opt;
            opt.clear();
            QString infoName;
            QString infoHash;
            dialog.getBtInfo(opt, infoName, infoHash);
            emit downLoadMetaLinkCreate(mlFile, opt, infoName);
        }
        close();
    }
}

void CreateTaskWidget::onCancelBtnClicked()
{
    m_texturl->clear();
    if (m_analysisUrl != nullptr) {
        delete m_analysisUrl;
        m_analysisUrl = nullptr;
    }
    close();
}

void CreateTaskWidget::onSureBtnClicked()
{
    if (!Func::isNetConnect()) {
        showNetErrorMsg();
        return;
    }
    QFile f(m_defaultDownloadDir);
    if(!f.exists()){
        MessageBox msg(this);
        msg.setAccessibleName("FolderNotExists");
        msg.setFolderNotExists();
        msg.exec();
        return;
    }
    double dSelectSize = getSelectSize();
    QString freeSize = Aria2RPCInterface::instance()->getCapacityFree(m_defaultDownloadDir);
    double dCapacity = formatSpeed(freeSize);
    if (dSelectSize > dCapacity) { //剩余空间比较 KB
        qDebug() << "Disk capacity is not enough!";
        MessageBox msg(this);
        msg.setAccessibleName("FolderNotExists");
        msg.setWarings(tr("Insufficient disk space, please change the download folder"), tr("OK", "button"), tr(""));
        msg.exec();
        return;
    }
    static QMutex mutex;
    bool ret = mutex.tryLock();
    if (!ret) {
        return;
    }

    QVector<LinkInfo> urlList;
    for (int i = 0; i < m_model->rowCount(); i++) {
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            LinkInfo linkInfo;
            linkInfo.urlName = m_model->data(m_model->index(i, 1),1).toString();
            linkInfo.type = m_model->data(m_model->index(i, 2),2).toString();
            linkInfo.urlSize = m_model->data(m_model->index(i, 3),3).toString();
            linkInfo.length = m_model->data(m_model->index(i, 4),4).toLongLong();
            linkInfo.url = m_model->data(m_model->index(i, 5),5).toString();
            linkInfo.urlTrueLink = m_model->data(m_model->index(i, 6),6).toString();
            urlList.append(linkInfo);
        }
    }
    Settings::getInstance()->setCustomFilePath(m_defaultDownloadDir);
    hide();
    emit downloadWidgetCreate(urlList, m_defaultDownloadDir);


    m_texturl->clear();
    if (m_analysisUrl != nullptr) {
        delete m_analysisUrl;
        m_analysisUrl = nullptr;
    }
    mutex.unlock();
}

void CreateTaskWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //数据中是否包含URL
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction(); //如果是则接受动作
    } else {
        event->ignore();
    }
}

void CreateTaskWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mineData = event->mimeData(); //获取MIME数据
    if (!mineData->hasUrls()) { //如数据中包含URL
        return;
    }
    QList<QUrl> urlList = mineData->urls(); //获取URL列表
    if (urlList.isEmpty())
        return;
    QString fileName;
    for (int i = 0; i < urlList.size(); ++i) {
        fileName = urlList.at(i).toString();
        if (!fileName.isEmpty()) { //若文件路径不为空
            if (fileName.startsWith("file:") && (fileName.endsWith(".torrent") || fileName.endsWith(".metalink"))) {
                fileName = fileName.right(fileName.length() - 7);
                hide();
                BtInfoDialog dialog(fileName, m_defaultDownloadDir, this); //= new BtInfoDialog(); //torrent文件路径
                int ret = dialog.exec();
                if (ret == QDialog::Accepted) {
                    QMap<QString, QVariant> opt;
                    QString infoName;
                    QString infoHash;
                    dialog.getBtInfo(opt, infoName, infoHash);
                    if (fileName.endsWith(".torrent")) {
                        emit downLoadTorrentCreate(fileName, opt, infoName, infoHash);
                    } else {
                        emit downLoadMetaLinkCreate(fileName, opt, infoName);
                    }
                }
                close();
            }
        }
    }
}

void CreateTaskWidget::setUrl(QString url)
{
    if (m_analysisUrl == nullptr) {
        m_analysisUrl = new AnalysisUrl;
        connect(m_analysisUrl, SIGNAL(sendFinishedUrl(LinkInfo *)), this, SLOT(updataTabel(LinkInfo *)));
    }
    QString setTextUrl;
    QString textUrl = m_texturl->toPlainText();
    if (textUrl.isEmpty()) {
        setTextUrl = url;
    } else {
        setTextUrl = textUrl + "\n" + url;
    }
    m_texturl->setText(setTextUrl);
    QString savePath = Settings::getInstance()->getDownloadSavePath();
    m_editDir->setText(savePath);
    m_defaultDownloadDir = savePath;

    QString freeSize = Aria2RPCInterface::instance()->getCapacityFree(m_defaultDownloadDir);
    m_labelCapacityFree->setText(QString(tr("Available:") + freeSize));
}

bool CreateTaskWidget::isMagnet(QString url)
{
    url = url.toLower();
    return url.mid(0, 20) == "magnet:?xt=urn:btih:";
}

bool CreateTaskWidget::isHttp(QString url)
{
    url = url.toLower();
    if ((url.startsWith("ftp:")) && (url.startsWith("http://")) && (url.startsWith("https://"))) {
        return false;
    }
    return true;
}

bool CreateTaskWidget::isFtp(QString url)
{
    url = url.toLower();
    if (url.startsWith("ftp:")) {
        return true;
    }
    return false;
}

void CreateTaskWidget::onTextChanged()
{

    if(nullptr == m_analysisUrl) {
        return;
    }
    m_texturl->toPlainText().isEmpty() ? hideTableWidget() : showTableWidget();

    QStringList urlList = m_texturl->toPlainText().split("\n");
    for (int i = 0; i < urlList.size(); i++) {
        if (urlList[i].isEmpty()) {
            if (i <= m_model->rowCount()) {
                m_model->removeRow(i);
            }
        }
    }
    urlList.removeAll(QString(""));
    QMap<QString, LinkInfo> urlInfoMap;
    for (int i = 0; i < urlList.size(); i++) {
        urlList[i] = urlList[i].simplified();
    }
    urlList.removeAll(QString(""));
    urlList.removeDuplicates();

    for (int i = 0; i < urlList.size(); i++) {
        QString name;
        QString type;
        if (isMagnet(urlList[i])) {
            name = urlList[i].split("&")[0];
            if (name.contains("btih:")) {
                name = name.split("btih:")[1] + ".torrent";
            } else {
                name = urlList[i].right(40);
            }
            setData(i, name.mid(0, name.size() - 8), "torrent", "0KB", urlList[i], 1024, urlList[i]);
            continue;
        }
        if (isFtp(urlList[i])) {
//                        double ftpSize = getFtpFileSize(urlList[i]);
//                        QString strSize = "";
//                        if(ftpSize > 0){
//                            strSize = Aria2RPCInterface::instance()->bytesFormat(ftpSize);
//                        }
            QStringList nameList = urlList[i].split("/");
            nameList.removeAll(QString(""));
            name = nameList[nameList.size() - 1];
            QMimeDatabase db;
            QString type = db.suffixForFileName(urlList[i]);

            if (type.isEmpty()) {
                QStringList tempList = urlList[i].split('.');
                if(!tempList.isEmpty()){
                    if(isExistType(tempList[tempList.size()-1])){
                        type = tempList[tempList.size()-1];
                        name = name.left(name.size()- type.size() -1);
                    }else {
                        type = "html";
                    }
                }else {
                    type = "html";
                }
            } else {
                int index = name.lastIndexOf('.');
                name.truncate(index);
            }
            setData(i, name, type, "0KB", urlList[i], 1024, urlList[i]);
            continue;
        }

        getUrlToName(urlList[i], name, type);
        setData(i, name, "", "", urlList[i], 0, urlList[i]);

        LinkInfo urlInfo;
        urlInfo.url = urlList[i];
        urlInfo.index = i;
        urlInfoMap.insert(urlList[i], urlInfo);
    }

    m_analysisUrl->setUrlList(urlInfoMap);

    while (urlList.size() < m_model->rowCount()) {
        m_model->removeRow(m_model->rowCount() - 1);
    }
}

void CreateTaskWidget::onFilechoosed(const QString &filename)
{
 //   m_editDir->setFileDialog(m_defaultDownloadDir);
    QFileInfo fileinfo;
    QString strPath;
    fileinfo.setFile(filename);
    if (!fileinfo.isWritable()) {
        MessageBox msg(this); //= new MessageBox();
        msg.setAccessibleName("FolderDenied");
        msg.setFolderDenied();
        msg.exec();
        strPath = m_editDir->directoryUrl().toString();
        // QString _text = this->getFileEditText(m_defaultDownloadDir);
        m_editDir->lineEdit()->setText(m_defaultDownloadDir);
        m_editDir->setDirectoryUrl(m_defaultDownloadDir);

        return;
    }
    m_editDir->lineEdit()->setText(filename);
    m_editDir->setDirectoryUrl(filename);
    m_defaultDownloadDir = filename;
    //获取到更改后的大小
    QString freeSize = Aria2RPCInterface::instance()->getCapacityFree(filename);
    QPalette pal;
    pal.setColor(QPalette::WindowText, QColor("#8AA1B4"));
    QFont font;
    m_labelCapacityFree->setText(QString(tr("Available:") + freeSize));
    m_labelCapacityFree->setPalette(pal);
    m_labelCapacityFree->setFont(font);
}

void CreateTaskWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    while (m_model->rowCount()) {
        m_model->removeRow(m_model->rowCount() -1);
    }
    m_texturl->clear();

}

void CreateTaskWidget::showNetErrorMsg()
{
    MessageBox msg(this); //= new MessageBox();
    //QString title = tr("Network error, check your network and try later");
   // msg.setWarings(getNetErrTip(), tr("OK"), ""); //网络连接失败
    msg.setAccessibleName("Networkerror");
    msg.setNetWorkError(getNetErrTip());
    msg.exec();
}

void CreateTaskWidget::updataTabel(LinkInfo *linkInfo)
{
    setData(linkInfo->index, linkInfo->urlName, linkInfo->type, linkInfo->urlSize, linkInfo->url, linkInfo->length, linkInfo->urlTrueLink);
}

void CreateTaskWidget::onAllCheck()
{
    int state = m_checkAll->checkState();
    for (int i = 0; i < m_model->rowCount(); i++) {
        if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
            m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
        }
    }
    bool ret = state == Qt::Checked;
    m_checkVideo->setCheckState(ret ? Qt::Checked : Qt::Unchecked);
    m_checkAudio->setCheckState(ret ? Qt::Checked : Qt::Unchecked);
    m_checkPicture->setCheckState(ret ? Qt::Checked : Qt::Unchecked);
    m_checkOther->setCheckState(ret ? Qt::Checked : Qt::Unchecked);
    m_checkDoc->setCheckState(ret ? Qt::Checked : Qt::Unchecked);
    m_checkZip->setCheckState(ret ? Qt::Checked : Qt::Unchecked);

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isVideo(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));

    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onVideoCheck()
{
    int state = m_checkVideo->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isVideo(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onAudioCheck()
{
    int state = m_checkAudio->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isAudio(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onPictureCheck()
{
    int state = m_checkPicture->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isPicture(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onZipCheck()
{
    int state = m_checkZip->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isZip(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onDocCheck()
{
    int state = m_checkDoc->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }

    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (isDoc(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::onOtherCheck()
{
    int state = m_checkOther->checkState();
    if (m_checkVideo->checkState() == Qt::Checked
        && m_checkAudio->checkState() == Qt::Checked
        && m_checkPicture->checkState() == Qt::Checked
        && m_checkOther->checkState() == Qt::Checked
        && m_checkDoc->checkState() == Qt::Checked
        && m_checkZip->checkState() == Qt::Checked) {
        m_checkAll->setCheckState(Qt::Checked);
    } else {
        m_checkAll->setCheckState(Qt::Unchecked);
    }
    long total = 0;
    int cnt = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString ext = m_model->data(m_model->index(i, 2),2).toString();
        if (!isVideo(ext) && !isAudio(ext) && !isPicture(ext) && !isDoc(ext) && !isZip(ext)) {
            if (m_model->data(m_model->index(i, 4),4).toString().toLong() > 0) {
                m_model->setData(m_model->index(i, 0), state == Qt::Checked ? "1" : "0");
            }
        }
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            cnt++;
        }
    }
    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::getUrlToName(QString url, QString &name, QString &type)
{
    // 获取url文件名
    if (url.startsWith("magnet")) {
        if (url.split("&").size() < 0) {
            return;
        }
        name = url.split("&")[0];
        if (name.contains("btih:")) {
            if (name.split("btih:").size() < 1) {
                return;
            }
            name = name.split("btih:")[1] + ".torrent";
            type = ".torrent";
        } else {
            name = url.right(40);
            type = ".torrent";
        }
        return;
    }
    name = QString(url).right(url.length() - url.lastIndexOf('/') - 1);
    // 对url进行转码
    if (!name.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
        const QByteArray byte = name.toLatin1();
        QString decode = QUrl::fromPercentEncoding(byte);
        if (decode.contains("?")) {
            decode = decode.split("?")[0];
        }
        name = decode;
    }
    QMimeDatabase db;
    type = db.suffixForFileName(name);
    if (!type.isEmpty()) {
        name = name.mid(0, name.size() - type.size() - 1);
    } else {
        QStringList splitStr = name.split("?");
        if (splitStr.size() > 0)
            ;
        {
            type = db.suffixForFileName(splitStr[0]);
        }
        if (type.isEmpty()) {
            type = "error";
        }
    }
    name = QUrl::fromPercentEncoding(name.toUtf8());
}

void CreateTaskWidget::setData(int index, QString name, QString type, QString size, QString url, long length, QString trueUrl)
{
    m_model->insertRows(index, 0, QModelIndex());
    m_model->setData(m_model->index(index, 0, QModelIndex()),size == "" ? "0" : "1");
    if (!name.isEmpty()) {
         m_model->setData(m_model->index(index, 1, QModelIndex()),name);
    }

    QString str = m_model->data(m_model->index(index, 1),1).toString();  //获取不到名称就加默认名称
    if(!str.size() && !size.isEmpty() && !type.isEmpty()){
        QTime t;
        t.start();
        while(t.elapsed()<10);
        QDateTime dateTime = QDateTime::currentDateTime();
//        // 字符串格式化
//        QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
//        // 获取毫秒值
//        int ms = dateTime.time().msec();
        // 转换成时间戳
        qint64 epochTime = dateTime.toMSecsSinceEpoch();
        QString defaultName = "index" + QString::number(epochTime);
        m_model->setData(m_model->index(index, 1, QModelIndex()),defaultName);
    }
    m_model->setData(m_model->index(index, 2, QModelIndex()),type);
    if (type == "html" && size.isEmpty()) {
        m_model->setData(m_model->index(index, 3, QModelIndex()),"0KB");
        m_model->setData(m_model->index(index, 4, QModelIndex()), QString::number(1024));
    } else {
        m_model->setData(m_model->index(index, 3, QModelIndex()), size);
        m_model->setData(m_model->index(index, 4, QModelIndex()), QString::number(length));
    }

    m_model->setData(m_model->index(index, 5, QModelIndex()), url);
    m_model->setData(m_model->index(index, 6, QModelIndex()), trueUrl);

    m_tableView->setColumnWidth(0, 290);
    m_tableView->setColumnWidth(2, 60);
    m_tableView->setColumnHidden(1, true);
    m_tableView->setColumnHidden(4, true);
    m_tableView->setColumnHidden(5, true);
    m_tableView->setColumnHidden(6, true);
    m_tableView->setColumnHidden(7, true);

    long total = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        total += m_model->data(m_model->index(i, 4),4).toString().toLong();
    }
    QString totalSize = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelFileSize->setText(QString(tr("Total ") + totalSize));

    updateSelectedInfo();

}

void CreateTaskWidget::updateSelectedInfo()
{
    int cnt = 0;
    long total = 0;
    int selectVideoCount = 0;
    int selectAudioCount = 0;
    int selectPictureCount = 0;
    int selectOtherCount = 0;
    int selectZipCount = 0;
    int selectDocCount = 0;
    int allVideo = 0;
    int allAudio = 0;
    int allPic = 0;
    int allOther = 0;
    int allZip = 0;
    int allDoc = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QString v = m_model->data(m_model->index(i, 0),0).toString();
        QString type = m_model->data(m_model->index(i, 2),2).toString();
        if(m_model->data(m_model->index(i, 3),3).toString().isEmpty()){
            continue;
        }
        if (v == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
            if (isVideo(type)) {
                selectVideoCount++;
            } else if (isAudio(type)) {
                selectAudioCount++;
            } else if (isPicture(type)) {
                selectPictureCount++;
            } else if (isZip(type)) {
                selectZipCount++;
            } else if (isDoc(type)) {
                selectDocCount++;
            } else {
                selectOtherCount++;
            }
            cnt++;
        }
        if (isVideo(type)) {
            allVideo++;
        } else if (isAudio(type)) {
            allAudio++;
        } else if (isPicture(type)) {
            allPic++;
        } else if (isDoc(type)) {
            allDoc++;
        } else if (isZip(type)) {
            allZip++;
        } else {
            allOther++;
        }
    }
    for (int i = 0; i < m_model->rowCount(); i++) {
    }
    (allVideo == selectVideoCount) && (allVideo > 0) ? m_checkVideo->setCheckState(Qt::Checked) : m_checkVideo->setCheckState(Qt::Unchecked);
    (allAudio == selectAudioCount) && (allAudio > 0) ? m_checkAudio->setCheckState(Qt::Checked) : m_checkAudio->setCheckState(Qt::Unchecked);
    (allPic == selectPictureCount) && (allPic > 0) ? m_checkPicture->setCheckState(Qt::Checked) : m_checkPicture->setCheckState(Qt::Unchecked);
    (allOther == selectOtherCount) && (allOther > 0) ? m_checkOther->setCheckState(Qt::Checked) : m_checkOther->setCheckState(Qt::Unchecked);
    (allZip == selectZipCount) && (allZip > 0) ? m_checkZip->setCheckState(Qt::Checked) : m_checkZip->setCheckState(Qt::Unchecked);
    (allDoc == selectDocCount) && (allDoc > 0) ? m_checkDoc->setCheckState(Qt::Checked) : m_checkDoc->setCheckState(Qt::Unchecked);

    QString size = Aria2RPCInterface::instance()->bytesFormat(total);
    m_labelSelectedFileNum->setText(QString(tr("%1 files selected, %2")).arg(QString::number(cnt)).arg(size));
    m_sureButton->setEnabled(cnt > 0 ? true : false);
}

void CreateTaskWidget::setUrlName(int index, QString name)
{
        QList<TaskInfo> taskList;
        DBInstance::getAllTask(taskList);
        QString curName = name + "." + m_model->data(m_model->index(index, 2),2).toString();
        for (int i = 0; i < taskList.size(); i++) {
            if (taskList[i].downloadFilename == curName) {
                name = m_delegate->getCurName();
               // return;
            }
        }
        for (int j = 0; j < m_model->rowCount(); j++) {
            if (j == index)
                continue;
            if (name == m_model->data(m_model->index(index, 2),2).toString())
                return;
        }

    m_model->setData(m_model->index(index,1),name);
    m_tableView->setColumnHidden(1, true);
}

void CreateTaskWidget::hideTableWidget()
{
    if (m_tableView->isHidden()) {
        return;
    }
    m_tableView->hide();
    m_labelSelectedFileNum->hide();
    m_labelFileSize->hide();
    m_checkWidget->hide();
    m_editDir->hide();
    setMaximumSize(541, 325);
    setMinimumSize(541, 325);
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland")) {
        adjustSize();
    }

    m_sureButton->setEnabled(false);
}

void CreateTaskWidget::showTableWidget()
{
    if (!m_tableView->isHidden()) {
        return;
    }
    m_tableView->show();
    m_labelSelectedFileNum->show();
    m_labelFileSize->show();
    m_checkWidget->show();
    m_editDir->show();
    setMaximumSize(541, 575);
    setMinimumSize(541, 575);

    QRect rect = geometry();
    move(rect.x(), rect.y());

    m_sureButton->setEnabled(true);
}

bool CreateTaskWidget::isVideo(QString ext)
{
    QStringList types;
    types << "avi" << "mp4" << "mkv" << "flv" << "f4v" << "wmv"
          << "rmvb" << "rm" << "mpeg" << "mpg" << "mov" << "ts" << "m4v" << "vob";
    return types.contains(ext,Qt::CaseInsensitive);
}

bool CreateTaskWidget::isAudio(QString ext)
{
    QStringList types;
    types << "mp3" << "ogg" << "wav" << "ape" << "flac" << "wma" << "midi" << "aac" << "cda";
    return types.contains(ext,Qt::CaseInsensitive);
}

bool CreateTaskWidget::isPicture(QString ext)
{
    QStringList types;
    types << "jpg" << "jpeg" << "gif" << "png" << "bmp" << "svg" << "psd" << "tif" << "ico";
    return types.contains(ext,Qt::CaseInsensitive);
}

bool CreateTaskWidget::isZip(QString ext)
{
    QStringList types;
    types << "rar" << "zip" << "cab" << "iso" << "jar" << "ace"
          << "7z" << "tar" << "gz" << "arj" << "lzh" << "uue" << "bz2" << "z" << "tar.gz";
    return types.contains(ext,Qt::CaseInsensitive);
}

bool CreateTaskWidget::isDoc(QString ext)
{
    QStringList types;
    types << "txt" << "doc" << "xls" << "ppt" << "docx" << "xlsx" << "pptx";
    return types.contains(ext,Qt::CaseInsensitive);
}

QString CreateTaskWidget::getNetErrTip()
{
    return QString(tr("Network error, check your network and try later"));
}

void CreateTaskWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

double CreateTaskWidget::formatSpeed(QString str)
{
    QString number = str;
    if (str.contains("KB")) {
        number.remove("KB");
    } else if (str.contains("MB")) {
        number.remove("MB");
    } else if (str.contains("GB")) {
        number.remove("GB");
    } else if (str.contains("TB")) {
        number.remove("TB");
    } else if (str.contains("B")) {
        number.remove("B");
    }
    double num = number.toDouble();
    if (str.contains("KB")) {
        num = num * 1024;
    } else if (str.contains("MB")) {
        num = num * 1024 * 1024;
    } else if (str.contains("GB")) {
        num = num * 1024 * 1024 * 1024;
    } else if (str.contains("TB")) {
        num = num * 1024 * 1024 * 1024 * 1024;
    }

    return num;
}

double CreateTaskWidget::getSelectSize()
{
    long total = 0;
    for (int i = 0; i < m_model->rowCount(); i++) {
        if (m_model->data(m_model->index(i, 0),0).toString() == "1") {
            total += m_model->data(m_model->index(i, 4),4).toString().toLong();
        }
    }
    return total;
}


bool CreateTaskWidget::isExistType(QString type)
{
    QStringList typeList;
    typeList << "asf" << "avi" << "exe" << "iso" << "mp3" << "mpeg" << "mpg" << "mpga" << "ra" <<
                "rar" << "rm" << "rmvb" << "tar" << "wma" << "wmp" << "wmv" << "mov" << "zip" <<
                "3gp" << "chm" << "mdf" << "torrent" << "jar" << "msi" << "arj" << "bin" << "dll" <<
                "psd" << "hqx" << "sit" << "lzh" << "gz" << "tgz" << "xlsx" << "xls" << "doc" << "docx" <<
                "ppt" << "pptx" << "flv" << "swf" << "mkv" << "tp" << "ts" << "flac" << "ape" << "wav" <<
                "aac" << "txt" << "crx" << "dat" << "7z" << "ttf" << "bat" << "xv" << "xvx" << "pdf" <<
                "mp4" << "apk" << "ipa" << "epub" << "mobi" << "deb";
    return typeList.contains(type , Qt::CaseSensitivity::CaseSensitive);
}

