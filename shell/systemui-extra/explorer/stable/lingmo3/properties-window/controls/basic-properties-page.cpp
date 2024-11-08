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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "basic-properties-page.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFormLayout>
#include <fcntl.h>
#include <sys/types.h>
#include <glib/gstdio.h>
#include <gio/gdesktopappinfo.h>

#include <QDateTime>
#include <QThreadPool>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <file-launch-manager.h>
#include <global-settings.h>

#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

#include <QFileDialog>
#include <QtConcurrent>
#include <QGSettings>

#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "thumbnail-manager.h"
#include "file-operation-utils.h"
#include "file-watcher.h"
#include "file-count-operation.h"
#include "file-operation-manager.h"
#include "file-meta-info.h"
#include "global-settings.h"
#include "generic-thumbnailer.h"
#include "file-operation-manager.h"
#include "open-with-properties-page.h"
#include "global-settings.h"

#include <QApplication>

#define DEBUG qDebug() << "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"

using namespace Peony;

static PushButtonStyle *global_instance = nullptr;

PushButtonStyle *PushButtonStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new PushButtonStyle;
    }
    return global_instance;
}

void PushButtonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);

            return;
        }
        break;
    }

    default:
        break;
    }
    qApp->style()->drawControl(element, option, painter, widget);
}

int PushButtonStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ButtonMargin:
    {
        return 0;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QRect PushButtonStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            QRect rect = option->rect;
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                rect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
            } else if (!text && icon && !(button->features & QStyleOptionButton::HasMenu)) {

            } else {
                rect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
            }
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }

    default:
        break;
    }

    return QProxyStyle::subElementRect(element, option, widget);
}

BasicPropertiesPage::BasicPropertiesPage(const QStringList &uris, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_uris = uris;
    m_timer = new QTimer(this);
    m_timer->setInterval(750);
    connect(m_timer, &QTimer::timeout, this, [=]{
        updateCountInfo(true);
    });
    //getFIleInfo
    this->getFIleInfo(m_uris.first());
    this->init();
//    QFuture<void> future = QtConcurrent::run([=]() {
//        getFIleInfo(m_uris.first());
//    });
//
//    m_futureWatcher = new QFutureWatcher<void>;
//    m_futureWatcher->setFuture(future);
//
//    connect(m_futureWatcher,&QFutureWatcher<void>::finished,this,&BasicPropertiesPage::init);
}

void BasicPropertiesPage::init()
{
//    if (m_futureWatcher) {
//        delete m_futureWatcher;
//        m_futureWatcher = nullptr;
//    }
    //Time Modified: （是左侧显示最长的字符串）
    m_labelWidth = fontMetrics().width(tr("Time Modified:")) + 5;
    m_labelWidth = m_labelWidth < 90 ? 90 : m_labelWidth;

    //check file type and search fileinfo
    FileType fileType = this->checkFileType(m_uris);
    //单个文件才能换icon
    if (fileType != BP_MultipleFIle) {
        m_watcher = std::make_shared<FileWatcher>(m_uris.first());
        m_watcher->connect(m_watcher.get(), &FileWatcher::locationChanged, this, &BasicPropertiesPage::onSingleFileChanged);
        m_watcher->startMonitor();

        m_thumbnail_watcher = std::make_shared<FileWatcher>("thumbnail:///");
        connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri){
            auto icon = ThumbnailManager::getInstance()->tryGetThumbnail(uri);
            m_iconButton->setIcon(icon);
            //QMessageBox::information(0, 0, "icon updated");
        });
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    this->setLayout(m_layout);

    this->initFloorOne(m_uris,fileType);
    this->addSeparator();

    this->initFloorTwo(m_uris,fileType);
    this->addSeparator();

    if(fileType != BP_MultipleFIle ) {
        this->initFloorThree(fileType);
        this->addSeparator();
        this->initFloorFour();
    }

    m_layout->addStretch(1);
}

BasicPropertiesPage::~BasicPropertiesPage()
{
    disconnect();
    cancelCount();
}

void BasicPropertiesPage::addSeparator()
{
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    m_layout->addWidget(separator);
}

void BasicPropertiesPage::addOpenWithLayout(QWidget *parent)
{
    if (m_openWithLayout) {
        m_defaultOpenWithWidget = OpenWithPropertiesPage::createDefaultOpenWithWidget(m_info->uri(), parent);
        m_openWithLayout->setContentsMargins(0,0,16,0);
        m_openWithLayout->setAlignment(Qt::AlignVCenter);
        m_openWithLayout->addWidget(m_defaultOpenWithWidget);
        m_openWithLayout->addStretch(1);

        QPushButton *moreAppButton = new QPushButton(parent);
        moreAppButton->setStyle(PushButtonStyle::getStyle());
        moreAppButton->setText(tr("Change"));
        moreAppButton->setMinimumSize((moreAppButton->fontMetrics().width(tr("Change")) + 5), 30);
        m_openWithLayout->addWidget(moreAppButton);

        connect(moreAppButton,&QPushButton::clicked,this,[=](){
            NewFileLaunchDialog dialog(m_info.get()->uri());
            if (QDialog::Accepted == dialog.exec()) {
                m_defaultOpenWithWidget->setLaunchAction(FileLaunchManager::getDefaultAction(m_info->uri()));
            }
        });
    }
}

/*!
 *
 * \brief BasicPropertiesPage::initFloorOne
 */
void BasicPropertiesPage::initFloorOne(const QStringList &uris,BasicPropertiesPage::FileType fileType)
{
    QFrame      *floor1  = new QFrame(this);
    QGridLayout *layout1 = new QGridLayout(floor1);
    layout1->setContentsMargins(22,16,0,16);

    floor1->setMinimumHeight(100);
    floor1->setLayout(layout1);

    m_iconButton      = new QPushButton(floor1);
    m_displayNameEdit = new QLineEdit(floor1);
    m_locationEdit    = new QLineEdit(floor1);

    m_iconButton->setFixedSize(QSize(60,60));
    m_iconButton->setIconSize(QSize(48,48));
    m_iconButton->setProperty("isIcon", true);

    auto form1 = new QFormLayout(floor1);
    layout1->addLayout(form1,0,0);
    form1->setContentsMargins(0,0,30,0);
    form1->setFormAlignment(Qt::AlignVCenter);
    form1->addRow(m_iconButton);

    //icon area
    if (fileType != BP_MultipleFIle) {
        connect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        this->onSingleFileChanged(nullptr, uris.first());
    } else {
        m_iconButton->setIcon(QIcon::fromTheme("unknown"));
    }

    //text area
    auto form2 = new QFormLayout(floor1);
    layout1->addLayout(form2, 0, 1);

    m_displayNameEdit->setMinimumHeight(32);
    m_locationEdit->setMinimumHeight(32);

    form2->addRow(tr("Name"), m_displayNameEdit);
    form2->addRow(tr("Location"), m_locationEdit);

    if(uris.first() == "filesafe:///"){
        form2->itemAt(1,QFormLayout::LabelRole)->widget()->setEnabled(false);
    }

    //select multiplefiles
    if (fileType == BP_MultipleFIle || !m_info->canRename())
        m_displayNameEdit->setReadOnly(true);

    //选中多个文件时，使用另外的线程获取文件名称并拼接
    if (fileType == BP_MultipleFIle) {
        FileNameThread *getNameThread = new FileNameThread(uris);
        getNameThread->start();

        connect(getNameThread, &FileNameThread::fileNameReady, this, [=](QString fileName) {
            m_displayNameEdit->setText(fileName);
            m_displayNameEdit->setToolTip(fileName);
            delete getNameThread;
        });
    }

    connect(m_displayNameEdit, &QLineEdit::textChanged, [=]() {
        if (isNameChanged()) {
            this->thisPageChanged();
        }
    });

    m_locationEdit->setReadOnly(true);
    QUrl url = FileUtils::getParentUri(uris.first());
    QString location = url.toDisplayString();
    if (location.startsWith("file://"))
        location = location.split("file://").last();
    m_locationEdit->setText(location);

    //move -button
    if(fileType == BP_Folder) {
        m_moveButton = new QPushButton(floor1);

        auto form3 = new QFormLayout(floor1);
        form3->setContentsMargins(0,0,16,0);
        form3->setFormAlignment(Qt::AlignBottom);

        m_moveButton->setText(tr("move"));
        m_moveButton->setMinimumSize(70,32);
        //暂时使用设置最大高度的方式解决与输入框自适应不一致问题
        m_moveButton->setMaximumSize(70, 38);

        form3->addRow(m_moveButton);
        //home目录不支持移动和重命名
        //不能改名的文件自然不能移动  标准位置文件夹也不能移动
        if(m_info.get()->uri() == ("file://"+QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())
            || !m_info->canRename()
            || FileUtils::isStandardPath(m_info->uri())) {
            m_displayNameEdit->setReadOnly(true);
            m_moveButton->setDisabled(true);
        }

        connect(m_moveButton,&QPushButton::clicked,this,&BasicPropertiesPage::moveFile);
        layout1->addLayout(form3,0,2);
    } else {
        layout1->setContentsMargins(22,16,16,16);
    }

    QString fileUri = uris.at(0);
    //filesafe插件中保护箱下目录不能移动——fix bug 76864
    //fix show properties in filesafe path crash issue, link to bug#74350
    if(fileUri.startsWith("filesafe:///") && m_moveButton) {
        m_moveButton->setVisible(false);
    }

    if(fileUri.startsWith("filesafe:///") && (fileUri.remove("filesafe:///").indexOf("/") == -1)) {
        disconnect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
        m_displayNameEdit->setReadOnly(true);
        m_locationEdit->setDisabled(true);
    }

    if (uris.first() == ("file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation))) {
        m_iconButton->setFocusPolicy(Qt::NoFocus);
        m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);
        disconnect(m_iconButton, &QPushButton::clicked, this, &BasicPropertiesPage::chooseFileIcon);
    }

    //add floor1 to context
    m_layout->addWidget(floor1);
}

void BasicPropertiesPage::initFloorTwo(const QStringList &uris,BasicPropertiesPage::FileType fileType)
{
    QFrame      *floor2  = new QFrame(this);
    QFormLayout *layout2 = new QFormLayout(floor2);
    layout2->setContentsMargins(22,16,0,16);
    layout2->setVerticalSpacing(8);
    floor2->setLayout(layout2);
    //144px 为多选文件情况下占用的最大高度 - 144px is the maximum height occupied in the case of multiple selection files
    floor2->setMinimumHeight(144);

    m_fileTypeLabel      = this->createFixedLabel(0,32,floor2);
    m_fileSizeLabel      = this->createFixedLabel(0,32,floor2);
    m_fileTotalSizeLabel = this->createFixedLabel(0,32,floor2);

    layout2->addRow(tr("Type:"),m_fileTypeLabel);

    if(fileType != BP_MultipleFIle) {
        //FIX:目前只是在这里对文件快捷方式进行判断并显示，这并不合适，希望在底层文件fileType直接增加快捷方式。
        //FIX:At present, only the file shortcuts are judged and displayed here. This is not appropriate.
        //I hope to directly add shortcuts to the underlying file file Type.
        m_fileTypeLabel->setText(m_info->isSymbolLink() ? tr("symbolLink") : m_info.get()->fileType());
    }

    if(fileType == BP_Folder && uris.first() == "filesafe:///"){
        m_fileTypeLabel->setText(tr("Folder"));
    }

    //根据文件类型添加组件 - Add components based on file type
    switch (fileType) {
    case BP_Folder:
        m_folderContainLabel = this->createFixedLabel(0,32,floor2);
        layout2->addRow(this->createFixedLabel(m_labelWidth,32,tr("Include:"),floor2),m_folderContainLabel);
        break;
    case BP_File:
        m_openWithLayout = new QHBoxLayout(floor2);
        layout2->addRow(this->createFixedLabel(m_labelWidth,32,tr("Open with:"),floor2),m_openWithLayout);
        this->addOpenWithLayout(floor2);
        break;
    case BP_Application:
    {
        m_descrptionLabel = this->createFixedLabel(0,32,floor2);
        layout2->addRow(this->createFixedLabel(m_labelWidth,32,tr("Description:"),floor2),m_descrptionLabel);
        //fix bug#53504, not show duplicated name issue
        QString displayName = m_info.get()->displayName();
        if (m_info->isDesktopFile())
        {
            displayName = FileUtils::handleDesktopFileName(m_info->uri(), displayName);
        }
        m_descrptionLabel->setText(displayName);
    }

        break;
    case BP_MultipleFIle:
        m_fileTypeLabel->setText(tr("Select multiple files"));
    default:
        break;
    }

    layout2->addRow(this->createFixedLabel(m_labelWidth,32,tr("Size:"),floor2),m_fileSizeLabel);
    layout2->addRow(this->createFixedLabel(m_labelWidth,32,tr("Space Useage:"),floor2),m_fileTotalSizeLabel);

    this->countFilesAsync(uris);

    m_layout->addWidget(floor2);
}

void BasicPropertiesPage::initFloorThree(BasicPropertiesPage::FileType fileType)
{
    this->setSysTimeFormat();
    // set time
    if (QGSettings::isSchemaInstalled("org.lingmo.control-center.panel.plugins")) {
        QGSettings* settings = new QGSettings("org.lingmo.control-center.panel.plugins", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if ("hoursystem" == key) {
                setSysTimeFormat();
                updateInfo(m_info->uri());
            }
        });
    }

#ifdef LINGMO_SDK_DATE
    connect(GlobalSettings::getInstance(),
            &GlobalSettings::updateLongDataFormat,
            this,
            &BasicPropertiesPage::updateDateFormat);
#endif

    auto floor3 = new QFrame(this);
    QFormLayout *layout3 = new QFormLayout(floor3);
    layout3->setVerticalSpacing(8);
    floor3->setLayout(layout3);

    floor3->setMinimumHeight(64);

    layout3->setContentsMargins(22,16,0,16);

    switch (fileType) {
    case BP_File:
    case BP_Application:
    case BP_Folder:
        m_timeCreatedLabel  = this->createFixedLabel(0,32,floor3);
        m_timeCreatedFormLeft = this->createFixedLabel(m_labelWidth,32,tr("Time Created:"),floor3);
        layout3->addRow(m_timeCreatedFormLeft, m_timeCreatedLabel);
        m_timeModifiedLabel = this->createFixedLabel(0,32,floor3);
        m_timeAccessLabel   = this->createFixedLabel(0,32,floor3);
        layout3->addRow(this->createFixedLabel(m_labelWidth,32,tr("Time Modified:"),floor3), m_timeModifiedLabel);
        layout3->addRow(this->createFixedLabel(m_labelWidth,32,tr("Time Access:"),floor3), m_timeAccessLabel);
        break;
    default:
        break;
    }

    m_layout->addWidget(floor3);

    updateInfo(m_info.get()->uri());
    connect(m_watcher.get(), &FileWatcher::locationChanged, [=](const QString&, const QString &uri) {
        this->updateInfo(uri);
    });
}

/*!
 * 底部隐藏多选框和只读选择框
 * \brief BasicPropertiesPage::initFloorFour
 */
void BasicPropertiesPage::initFloorFour()
{
    QFrame      *floor4  = new QFrame(this);
    QFormLayout *layout4 = new QFormLayout(floor4);
    floor4->setMaximumHeight(64);
    layout4->setContentsMargins(22,16,0,16);
    m_readOnly = new QCheckBox(tr("Readonly"), floor4);
    m_hidden   = new QCheckBox(tr("Hidden"), floor4);

    QHBoxLayout *hBoxLayout = new QHBoxLayout(floor4);
    hBoxLayout->addWidget(m_readOnly);
    hBoxLayout->addStretch(1);
    hBoxLayout->addWidget(m_hidden);
    hBoxLayout->addStretch(2);

    if(m_info.get()->canRead() && !m_info.get()->canWrite())
        m_readOnly->setCheckState(Qt::Checked);

    if(m_info.get()->displayName().startsWith("."))
        m_hidden->setCheckState(Qt::Checked);

    m_readOnly->setDisabled(!m_info->canRename());

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    bool isDesktop = FileUtils::isSamePath(m_info->uri(), desktopPath);
    //fix bug#113890,hiden Desktop folder change desktop show
    m_hidden->setDisabled(!m_info->canRename() || isDesktop);
    m_isReadOnly = m_readOnly->isChecked();

    layout4->addRow(this->createFixedLabel(m_labelWidth,32,tr("Property:"),floor4),hBoxLayout);

    //确认被修改
    connect(m_readOnly,&QCheckBox::stateChanged,this,&BasicPropertiesPage::thisPageChanged);
    connect(m_hidden,&QCheckBox::stateChanged,this,&BasicPropertiesPage::thisPageChanged);

    m_layout->addWidget(floor4);
}

BasicPropertiesPage::FileType BasicPropertiesPage::checkFileType(const QStringList &uris)
{
    if(uris.count() != 1) {
        return BP_MultipleFIle;
    } else {
        if (m_info->displayName().isNull() || m_info->displayName().isEmpty()) {
            this->getFIleInfo(uris.first());
        }

        if(m_info.get()->isDir())
            return BP_Folder;
        if(m_info.get()->isDesktopFile())
            return BP_Application;
        return BP_File;
    }
}

//异步获取数据
void BasicPropertiesPage::getFIleInfo(QString uri)
{
    //将在收藏夹的文件路径替换为真实路径 - Replace the file path in the favorites with the real path
    //FIX:如果是远程文件夹或者其它非本地文件夹添加到收藏夹呢？ - What if remote folders or other non-local folders are added to favorites?
    if (uri.startsWith("favorite://")) {
        QUrl url(uri);
        uri = "file://" + url.path();
        m_uris.clear();
        m_uris.append(uri);
    }

    std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
    FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
    fileInfoJob->setAutoDelete();
    fileInfoJob->querySync();
    m_info = fileInfo;
}

QIcon generateThumbnail(const QString &uri)
{
    QUrl url = uri;
    if (! uri.startsWith("file:///")) {
        url = FileUtils::getTargetUri(uri);
    }

    auto _desktop_file = g_desktop_app_info_new_from_filename(url.path().toUtf8().constData());
    auto _icon_string = g_desktop_app_info_get_string(_desktop_file, "Icon");
    QIcon thumbnail = QIcon::fromTheme(_icon_string);
    QString string = _icon_string;

    //fix desktop file set customer icon issue, link to bug#77638
    auto info = FileInfo::fromUri(uri);
    if (! info->customIcon().isEmpty()){
        thumbnail = GenericThumbnailer::generateThumbnail(info->customIcon(), true);
    }

    if (thumbnail.isNull() && string.startsWith("/")) {
        thumbnail = GenericThumbnailer::generateThumbnail(_icon_string, true);
    }
    DEBUG << "thumbnail" << thumbnail <<_icon_string;
    g_free(_icon_string);
    g_object_unref(_desktop_file);

    return thumbnail;
}

void BasicPropertiesPage::onSingleFileChanged(const QString &oldUri, const QString &newUri)
{
    this->getFIleInfo(newUri);

    ThumbnailManager::getInstance()->createThumbnail(m_info.get()->uri(), m_thumbnail_watcher);
    auto icon = QIcon::fromTheme(m_info.get()->iconName(), QIcon::fromTheme("unknown"));
    auto thumbnail = ThumbnailManager::getInstance()->tryGetThumbnail(m_info.get()->uri());

    m_iconButton->setIcon(thumbnail.isNull() ? icon : thumbnail);
    //fix bug#53504, not show duplicated name issue.
    QString fileName = m_info->displayName();
    if (m_info->isDesktopFile() && !fileName.endsWith(".desktop")) {
        fileName = FileUtils::handleDesktopFileName(m_info->uri(), fileName);
    }
    //fix bug:#82320
    if (QRegExp("^file:///data/usershare(/{,1})$").exactMatch(m_info->uri())) {
        fileName = tr("usershare");
    }

    m_displayNameEdit->setText(fileName);
    m_displayNameEdit->setToolTip(fileName);

    if (thumbnail.isNull()) {
        ThumbnailManager::getInstance()->createThumbnail(m_info.get()->uri(), m_thumbnail_watcher);
    }

    m_iconButton->setIcon(thumbnail.isNull() ? icon : thumbnail);
    QUrl url = FileUtils::getParentUri(m_info.get()->uri());

    QString location = url.toDisplayString();
    if (location.startsWith("file://"))
        location = location.split("file://").last();
    m_locationEdit->setText(location);
}

void BasicPropertiesPage::countFilesAsync(const QStringList &uris)
{
    m_timer->start();

    //old op will delete later
    if (m_countOp) {
        m_countOp->disconnect();
        m_countOp->cancel();
    }
    //clear old data
    m_folderContainFiles   = 0;
    m_folderContainFolders = 0;
    m_fileSizeCount        = 0;
    m_fileTotalSizeCount   = 0;

    if (uris.count() == 1) {
        auto uri = uris.first();
        auto info = FileInfo::fromUri(uri);

        QStringList realUris;
        //判断是不是快捷方式 - Determine if it is a shortcut
        if (info->isSymbolLink()) {
            if (!info.get()->symlinkTarget().isEmpty())
                realUris << ("file://" + info.get()->symlinkTarget());
        }
        m_countOp = new FileCountOperation(realUris.isEmpty() ? uris : realUris);
        DEBUG << "symlinkTarget:" << info.get()->symlinkTarget();
    } else {
        QStringList lists;
        for (auto uri : uris) {
            auto info = FileInfo::fromUri(uri);
            if (info->isSymbolLink()) {
                if (!info.get()->symlinkTarget().isEmpty()) {
                    QString temp = "file://" + info.get()->symlinkTarget();
                    lists.append(temp);
                } else {
                    lists.append(uri);
                }
            } else {
                lists.append(uri);
            }
        }
        m_countOp = new FileCountOperation(lists);
    }

    m_countOp->setAutoDelete(true);

    connect(m_countOp, &FileOperation::operationPreparedOne, this, &BasicPropertiesPage::onFileCountOne, Qt::BlockingQueuedConnection);

    connect(m_countOp, &FileCountOperation::countDone, this, [=](quint64 file_count, quint64 hidden_file_count, quint64 total_size) {
        m_timer->stop();

        m_countOp = nullptr;
        m_folderContainFiles = file_count - m_folderContainFolders;
        m_fileSizeCount = total_size;
        //不统计文件夹本身 - Do not count the folder itself
        if (m_folderContainFolders != 0) {
            m_folderContainFolders--;
        }
        this->updateCountInfo(true);

        //使用du -s 命令查看文件实际占用的磁盘空间。
        for (QString uri : m_uris) {
            QUrl url(uri);
            if (url.isLocalFile()) {
                std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
                FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
                fileInfoJob->setAutoDelete();
                fileInfoJob->querySync();
                //某些带空格的文件名称会导致命令错误，加上引号解决此问题。
                QString path;
                if(uri == "filesafe:///") {
                    path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.box";
                } else {
    //                g_autoptr (GFile) gfile = g_file_new_for_uri(uri.toUtf8().constData());
    //                g_autofree gchar *gpath = g_file_get_path(gfile);
    //                if (gpath) {
    //                    path = gpath;
    //                } else {
                        path = QString("%1%2%3").arg("\"").arg(url.path()).arg("\"");
    //                }
                }


                QProcess process;
                if (fileInfo->isSymbolLink()) {
                    process.start("/usr/bin/du -L " + path);
                } else {
                    process.start("/usr/bin/du -s " + path);
                }
                process.waitForFinished();
                QString result = process.readAllStandardOutput();
                //du -s xxx 输出格式：4	xxx  (大小单位为KB)
                m_fileTotalSizeCount += result.split(QRegExp("\\s+")).first().toLong();
            }
        }
        //转换为 xx Bytes
        m_fileTotalSizeCount *= CELL1K;

        if (m_fileTotalSizeCount == 0) {
            quint64 a = 0;
            quint64 b = 0;
            a = m_fileSizeCount % CELL4K;
            b = m_fileSizeCount / CELL4K;
            quint64 cell4k = (a == 0) ? b : (b + 1);
            m_fileTotalSizeCount = cell4k * CELL4K;
        }
        //gio格式化工具
        char *fileTotalSizeFormat = g_format_size_full(m_fileTotalSizeCount,G_FORMAT_SIZE_IEC_UNITS);
        QString fileTotalSizeFormatString(fileTotalSizeFormat);
        fileTotalSizeFormatString.replace("iB", "B");

        QString fileTotalSizeText(tr("%1 (%2 Bytes)").arg(fileTotalSizeFormatString).arg(m_fileTotalSizeCount));
        g_free(fileTotalSizeFormat);

        m_fileTotalSizeLabel->setText(fileTotalSizeText);
    });

    QThreadPool::globalInstance()->start(m_countOp);
}

void BasicPropertiesPage::onFileCountOne(const QString &uri, quint64 size)
{
    //...FIX:第一版本在当前位置对文件夹进行统计,慢
//    std::shared_ptr<FileInfo> l_fileInfo = FileInfo::fromUri(uri);
//    FileInfoJob *fileInfoJob = new FileInfoJob(l_fileInfo);
//    fileInfoJob->setAutoDelete();
//    fileInfoJob->querySync();
//    bool a = l_fileInfo.get()->isDir();

    bool isDir = false;
    g_autoptr (GFile) file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autoptr (GFileInfo) info = g_file_query_info(file,
                                        "standard::*",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    GFileType fileType = g_file_info_get_file_type(info);
    QString contentType = g_file_info_get_content_type (info);
    if (fileType == G_FILE_TYPE_DIRECTORY || contentType == "inode/directory") {
        isDir = true;
    }

    if (isDir)
        m_folderContainFolders ++;
    else
        m_folderContainFiles ++;

    m_fileSizeCount += size;
    m_fileDoneCount ++;
    //500 files update ui
    updateCountInfo((m_fileDoneCount % 500 == 0));
}

void BasicPropertiesPage::cancelCount()
{
    m_timer->stop();
    if (m_countOp)
        m_countOp->cancel();
}

void BasicPropertiesPage::moveFile(){
    auto newDirPath = QString(QFileDialog::getExistingDirectoryUrl(nullptr, tr("Choose a new folder:"),m_info.get()->uri()).toEncoded());
    DEBUG << "new path:" << newDirPath ;

    if(newDirPath.isEmpty())
        return;

    if(newDirPath == m_info.get()->uri()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("cannot move a folder to itself !"), QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    if(newDirPath.startsWith(m_info.get()->uri())) {
        QMessageBox::critical(nullptr, tr("Error"), tr("cannot move a folder to itself !"), QMessageBox::Yes, QMessageBox::Yes);
        return;
    }

    DEBUG << "old Path:" << m_info.get()->uri() << newDirPath;

    QStringList uriList;
    uriList << m_info.get()->uri();

    auto fileOpe = FileOperationUtils::move(uriList,newDirPath,true);

    connect(fileOpe,&FileOperation::operationFinished,[=](){
        if (!fileOpe->hasError()) {
            QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.setProgram("/usr/bin/explorer");
            p.setArguments(QStringList()<<"--show-folders" << newDirPath);
            p.startDetached();
#else
            p.startDetached("/usr/bin/explorer", QStringList()<<"--show-folders"<< newDirPath);
#endif
            Q_EMIT requestCloseMainWindow();
        }
    });
}
/*!
 * 响应窗口确定按钮
 * \brief BasicPropertiesPage::saveAllChange
 */
void BasicPropertiesPage::saveAllChange()
{
    if (m_watcher)
        m_watcher->stopMonitor();
    if (m_thumbnail_watcher)
        m_thumbnail_watcher->stopMonitor();
    //未发生修改
    if (!this->m_thisPageChanged)
        return;

    //拒绝修改home目录
    if (m_info.get()->uri() == ("file://"+QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())) {
        return;
    }
    //修改图标
    this->changeFileIcon();

    if (m_readOnly && m_isReadOnly != m_readOnly->isChecked()) {
        mode_t mod = 0;
        quint32 mode = 0;
        if(m_readOnly->isChecked()) {
//            mod |= S_IRUSR;
//            mod |= S_IRGRP;
//            mod |= S_IROTH;

            g_autoptr(GFile) file = g_file_new_for_uri(m_info.get()->uri().toUtf8().constData());
            if (file) {
                g_autoptr(GError) error = NULL;
                g_autoptr(GFileInfo) info = g_file_query_info(file,
                                                              "unix::mode",
                                                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                              nullptr,
                                                              &error);
                bool has_unix_mode = g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                if (has_unix_mode) {
                    mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                    auto metaInfo = FileMetaInfo::fromUri(m_info.get()->uri());
                    if (metaInfo) {
                        metaInfo->setMetaInfoInt(TEMP_PERMISSIONS, mode);
                    }
                }
            }
            //去除写权限
            mode &= ~S_IWUSR;
            mode &= ~S_IWGRP;
            mode &= ~S_IWOTH;
            mod = mode;
        } else {
            auto metaInfo = FileMetaInfo::fromUri(m_info.get()->uri());
            if (metaInfo && metaInfo->getMetaInfoInt(TEMP_PERMISSIONS)) {
                mod = metaInfo->getMetaInfoInt(TEMP_PERMISSIONS);
            } else {
                mod |= S_IRUSR;
                mod |= S_IRGRP;
                mod |= S_IROTH;

                mod |= S_IWUSR;
//                mod |= S_IWGRP;
//                mod |= S_IWOTH;
            }
        }
        //FIX:如果该文件之前就是可执行，那么应该保留可执行权限
        if (m_info->canExecute())
            mod |= S_IXUSR;

        //.desktop文件给予可执行,.desktop文件原本可执行才给可执行权限
        if (((m_info.get()->isDesktopFile()) || m_info.get()->displayName().endsWith(".desktop")) && m_info->canExecute()) {
            //FIX:可执行范围 目前只给拥有者执行权限
            mod |= S_IXUSR;
            //mod |= S_IXGRP;
            //mod |= S_IXOTH;
        }
        QUrl url = m_info.get()->uri();
        g_chmod(url.path().toUtf8(), mod);

    }

    //是否进行文件隐藏操作 - Whether to hide files
    bool existHiddenOpt = false;
    if (m_hidden) {
        QString newName = m_info.get()->displayName();

        if (newName.startsWith("."))
            newName = newName.mid(1,-1);

        if (isNameChanged()) {
            newName = m_displayNameEdit->text();
        }

        bool isHidden = m_info.get()->displayName().startsWith(".");

        //以前没隐藏，并且选中隐藏框
        if(!isHidden && m_hidden->isChecked()) {
            newName = "." + newName;
            FileOperationUtils::rename(m_info.get()->uri(), newName, true);
            existHiddenOpt = true;

        } else if(isHidden && !m_hidden->isChecked()) {
            //以前已经隐藏，并且取消选中隐藏框
            FileOperationUtils::rename(m_info.get()->uri(), newName, true);
            existHiddenOpt = true;
        }
    }

    if (!existHiddenOpt) {
        if (isNameChanged()) {
            FileOperationUtils::rename(m_info.get()->uri(), m_displayNameEdit->text(), true);
        }
    }

    //FIX:修复桌面快捷方式文件的缩略图改变后需要手动刷新才更新的问题
    //fix the problem that the thumbnails of desktop shortcut files need to be manually refreshed before they are updated after being changed.
    QString desktopPath = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
    //if (m_info.get()->uri().contains(desktopUri) && m_info.get()->isSymbolLink()) {
        QProcess p;
        p.setProgram("/usr/bin/touch");
        p.setArguments(QStringList()<<"-h"<<m_info->filePath());
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        p.startDetached();
    #else
        p.startDetached("/usr/bin/touch", QStringList()<<"-h"<<m_info->filePath());
    #endif
        p.waitForFinished(-1);
    //}
}

void BasicPropertiesPage::chooseFileIcon()
{
    QUrl iconPathUrl;
    iconPathUrl.setPath("/usr/share/icons");
    auto picture = QFileDialog::getOpenFileName(nullptr, tr("Choose a custom icon"), "/usr/share/icons", "*.png *.jpg *.jpeg *.svg");

    QFileInfo fileInfo(picture);
    if (fileInfo.exists()) {
        qint64 fileSize = fileInfo.size();
        double fileSizeMB = fileSize / (1024.0 * 1024.0);
        qDebug() << "fileSize is ：" << fileSizeMB << "MB";
        if (fileSizeMB > 1.0) {
            QMessageBox::warning(nullptr, "", tr("Please select a image that is smaller than 1MB."));
            return;
        }
    } else {
        qDebug() << "file does not exist ：" << picture;
    }

    if (!picture.isEmpty()) {
        qDebug()<<"chose new file icon:"<< picture;
        m_iconButton->setIcon(QIcon(picture));
        this->m_newFileIconPath = picture;
        this->thisPageChanged();
    }

}

void BasicPropertiesPage::changeFileIcon()
{
    if (!m_newFileIconPath.isEmpty()) {
        auto metaInfo = FileMetaInfo::fromUri(m_info.get()->uri());
//        QFileInfo fileInfo(m_newFileIconPath);
//        if (!QIcon::fromTheme(fileInfo.baseName()).isNull())
//            metaInfo.get()->setMetaInfoString("custom-icon", fileInfo.baseName());
//        else
            metaInfo.get()->setMetaInfoString("custom-icon", m_newFileIconPath);

        ThumbnailManager::getInstance()->createThumbnail(m_info.get()->uri(), m_thumbnail_watcher, true);
    }
}

void BasicPropertiesPage::updateCountInfo(bool isDone)
{
    if (isDone) {
        QString fileSizeText;

        quint64 a = 0;
        a = m_fileSizeCount / CELL1K;
        //小于1KB
        if (a < 1) {
            fileSizeText = tr("%1 Bytes").arg(m_fileSizeCount);
        } else {
            char *fileSizeFormat = g_format_size_full(m_fileSizeCount,G_FORMAT_SIZE_IEC_UNITS);
            QString fileSizeFormatString(fileSizeFormat);
            //根据设计要求，按照1024字节对数据进行格式化（1GB = 1024MB），同时将GiB改为GB显示，以便于用户理解。参考windows显示样式。
            fileSizeFormatString.replace("iB", "B");

            fileSizeText = tr("%1 (%2 Bytes)").arg(fileSizeFormatString).arg(m_fileSizeCount);
            g_free(fileSizeFormat);
        }

        m_fileSizeLabel->setText(fileSizeText);
        //在为完成统计前，先显示文件大小而不是占用空间大小
        m_fileTotalSizeLabel->setText(fileSizeText);

        if(m_folderContainLabel)
            m_folderContainLabel->setText(tr("%1 files, %2 folders").arg(m_folderContainFiles).arg(m_folderContainFolders));
    }
}

void BasicPropertiesPage::updateDateFormat(QString dateFormat)
{
    //update date and time show format, task #101605
    qDebug() << "sdk format signal:"<<dateFormat;
    if (m_date_format != dateFormat){
        updateInfo(m_info->uri());
        m_date_format = dateFormat;
    }
}

void BasicPropertiesPage::updateInfo(const QString &uri)
{
    //QT获取文件相关时间 ,
    m_info = FileInfo::fromUri(uri);
    FileInfoJob *fileInfoJob = new FileInfoJob(m_info);
    fileInfoJob->setAutoDelete();
    connect(fileInfoJob, &FileInfoJob::queryAsyncFinished, this, [=](){
        QUrl url(uri);
        //FIXME:暂时不处理除了本地文件外的文件信息,希望添加对其他文件的支持
        //if (url.isLocalFile()) {
        if(m_info->accessTime() != 0 && m_info->modifiedTime() != 0){
//            if(m_timeModifiedLabel) {
//                QDateTime date2 = qFileInfo.lastModified();
//                QString time2 = date2.toString(m_systemTimeFormat);
//                m_timeModifiedLabel->setText(time2);
//            }
//            if(m_timeAccessLabel) {
//                QDateTime date3 = qFileInfo.lastRead();
//                QString time3 = date3.toString(m_systemTimeFormat);
//                m_timeAccessLabel->setText(time3);
//            }

            GFile     *file = g_file_new_for_uri(uri.toUtf8().constData());
            GFileInfo *info = g_file_query_info(file,
                                                "time::*",
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                nullptr,
                                                nullptr);
            g_object_unref(file);


            m_timeModified = g_file_info_get_attribute_uint64(info,"time::modified");
            if(m_timeModifiedLabel) {
//                QDateTime date2 = QDateTime::fromMSecsSinceEpoch(m_timeModified*1000);
//                QString time2 = date2.toString(m_systemTimeFormat);
                QString time2 = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeModified, true);
                m_timeModifiedLabel->setText(time2);
            }

            if(m_timeAccessLabel) {
                m_timeAccess = g_file_info_get_attribute_uint64(info,"time::access");
//                QDateTime date3 = QDateTime::fromMSecsSinceEpoch(m_timeAccess*1000);
//                QString time3 = date3.toString(m_systemTimeFormat);
                QString time3 = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeAccess, true);
                m_timeAccessLabel->setText(time3);
            }

            QString path = url.path();
            QFileInfo qFileInfo(path);
            bool visible = true;
            if (/*qFileInfo.isDir() && */m_timeCreatedLabel) {
                m_timeCreated = g_file_info_get_attribute_uint64(info, "time::created");
                visible = m_timeCreated;

                // 客户需要必须显示创建时间，因此使用三个时间最小时间戳为创建时间
                quint64 minTime = m_timeCreated != 0 ? m_timeCreated : m_timeModified;
                minTime = qMin (minTime, m_timeModified);
                if (m_timeAccess != 0)
                    minTime = qMin (minTime, m_timeAccess);
                m_timeCreated = minTime;
//                QDateTime createDate = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
//                QString createTime = createDate.toString(m_systemTimeFormat);
                QString createTime = GlobalSettings::getInstance()->transToSystemTimeFormat(m_timeCreated, true);
                m_timeCreatedLabel->setText(createTime);

//                // FIXME:目前只是文件夹显示创建时间，当创建时间获取失败的时候，将修改时间作为创建时间
//                QDateTime date1 = qFileInfo.birthTime();
//                if (date1.isValid()) {
//                    QString time1 = date1.toString(m_systemTimeFormat);
//                    m_timeCreatedLabel->setText(time1);
//                } else {
//                    m_timeCreated = g_file_info_get_attribute_uint64(info, "time::modified");
//                    QDateTime createDate = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
//                    QString createTime = createDate.toString(m_systemTimeFormat);
//                    m_timeCreatedLabel->setText(createTime);
//                }

            }
            g_object_unref(info);

            m_timeCreatedFormLeft->setVisible(visible);
            m_timeCreatedLabel->setVisible(visible);
        } else {
            if (m_timeCreatedLabel){
                if (QGSettings::isSchemaInstalled("org.lingmo.style"))
                {
                    //font monitor
                    QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
                    connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
                        if (key == "systemFontSize") {
                            QFontMetrics fontWidth(m_timeCreatedLabel->font());
                            QString elideNote = fontWidth.elidedText(tr("Can't get remote file information"),Qt::ElideRight,280);
                            m_timeCreatedLabel->setText(elideNote);
                            m_timeCreatedLabel->setToolTip(tr("Can't get remote file information"));
                        }
                    });
                }
                QFontMetrics fontWidth(m_timeCreatedLabel->font());
                QString elideNote = fontWidth.elidedText(tr("Can't get remote file information"),Qt::ElideRight,280);
                m_timeCreatedLabel->setText(elideNote);
                m_timeCreatedLabel->setToolTip(tr("Can't get remote file information"));
            }
        }

        //FIXME:GVFS底层暂未实现文件创建时间获取API,暂时使用QT获取文件创建时间
        /*
         m_timeCreated = g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_TIME_CREATED);
        //m_timeCreated = g_file_info_get_attribute_uint64(info, "time::created");
        QDateTime date1 = QDateTime::fromMSecsSinceEpoch(m_timeCreated*1000);
        QString time1 = date1.toString(m_systemTimeFormat);
        m_timeCreatedLabel->setText(time1);

         if(m_timeModifiedLabel) {
            m_timeModified = g_file_info_get_attribute_uint64(info,
                                                              "time::modified");
            QDateTime date2 = QDateTime::fromMSecsSinceEpoch(m_timeModified*1000);
            QString time2 = date2.toString(m_systemTimeFormat);

            m_timeModifiedLabel->setText(time2);
        }
        if(m_timeAccessLabel) {
            m_timeAccess = g_file_info_get_attribute_uint64(info,
                                                            "time::access");
            QDateTime date3 = QDateTime::fromMSecsSinceEpoch(m_timeAccess*1000);
            QString time3 = date3.toString(m_systemTimeFormat);
            m_timeAccessLabel->setText(time3);
        }

        g_object_unref(info);

         */
    });

    fileInfoJob->queryAsync();
}

QLabel *BasicPropertiesPage::createFixedLabel(quint64 minWidth, quint64 minHeight, QString text, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if (minWidth != 0)
        label->setMinimumWidth(minWidth);
    if (minHeight != 0)
        label->setMinimumHeight(minHeight);

    label->setText(text);
    return label;
}

QLabel *BasicPropertiesPage::createFixedLabel(quint64 minWidth, quint64 minHeight, QWidget *parent)
{
    QLabel *label = new QLabel(parent);
    if (minWidth != 0)
        label->setMinimumWidth(minWidth);
    if (minHeight != 0)
        label->setMinimumHeight(minHeight);
    return label;
}

bool BasicPropertiesPage::isNameChanged()
{
    if (!m_displayNameEdit->isReadOnly() && !m_displayNameEdit->text().isEmpty()) {
        QString fileName(m_info->displayName());
        //桌面文件
        if (m_info->isDesktopFile() && !fileName.endsWith(".desktop")) {
            //做过处理的名称
            QString handledName = FileUtils::handleDesktopFileName(m_info->uri(), fileName);
            if (fileName != handledName) {
                //用户是否手动修改
                if (handledName != m_displayNameEdit->text())
                    return true;
                else
                    return false;
            }
        }
        //文件名称被修改过
        if (fileName != m_displayNameEdit->text())
            return true;
        else
            return false;

    }

    return false;
}

void BasicPropertiesPage::setSysTimeFormat()
{
    this->m_systemTimeFormat = GlobalSettings::getInstance()->getSystemTimeFormat();
}

void FileNameThread::run()
{
    QString fileName = "";
    if (m_uris.count() == 1) {
        std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(m_uris.first());
        FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
        fileInfoJob->setAutoDelete();
        fileInfoJob->querySync();
        fileName = fileInfo.get()->displayName();
        //fix bug#53504, not show duplicated name issue
        if (fileInfo->isDesktopFile())
        {
            fileName = FileUtils::handleDesktopFileName(fileInfo->uri(), fileName);
        }
    } else {
        QStringList stringList;
        for (auto uri : m_uris) {
            //FIXME: replace BLOCKING api in ui thread.(finish) **
            std::shared_ptr<FileInfo> fileInfo = FileInfo::fromUri(uri);
            FileInfoJob *fileInfoJob = new FileInfoJob(fileInfo);
            fileInfoJob->setAutoDelete();
            fileInfoJob->querySync();
            stringList<< fileInfo.get()->displayName();
        }
        auto text = stringList.join(",");
        fileName = QString(text);
    }

    Q_EMIT fileNameReady(fileName);
}
