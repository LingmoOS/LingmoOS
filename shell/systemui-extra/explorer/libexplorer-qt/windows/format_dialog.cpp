/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: ZJAY998 <zhangjun@kylinos.cn>
 *
 */

#include "file-utils.h"
#include "format_dialog.h"
#include "side-bar-menu.h"
#include "volume-manager.h"
#include "ui_format_dialog.h"
#include "side-bar-abstract-item.h"
#include "linux-pwd-helper.h"
#include "volumeManager.h"
#include "file-info.h"
#include "file-info-job.h"
#include "global-settings.h"
#include "format-dlg-create-delegate.h"

#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include "format-dlg-create-delegate.h"

#include <QObject>
#include <QMessageBox>
#include <KWindowSystem>
#include <QCloseEvent>
#include <QGridLayout>

#include <QInputDialog>
#include <QValidator>

#include <QTime>

using namespace  Peony;
#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

static bool b_finished = false;
static bool b_failed = false;
static bool b_canClose = true;
static double m_before_progress = 0;

static QHash<Format_Dialog *, GVolume *> dialogVolumes;

static ButtonStyle *global_instance = nullptr;

ButtonStyle *ButtonStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new ButtonStyle;
    }
    return global_instance;
}

void ButtonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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

int ButtonStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
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

QRect ButtonStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
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


QCheckBox *findPasswdCheckBox(Format_Dialog *dlg) {
    return dlg->findChild<QCheckBox *>("cryptCheckBox");
}

Format_Dialog::Format_Dialog(const QString &m_uris,SideBarAbstractItem *m_item,QWidget *parent) /*:
    QDialog (parent),
    ui(new Ui::Format_Dialog)*/
{
    setAutoFillBackground(true);
    setWindowTitle(tr("Format"));
    setBackgroundRole(QPalette::Base);
    setContentsMargins(24, 24, 24, 24);
    setFixedSize(mFixWidth, mFixHeight);
    setWindowIcon(QIcon::fromTheme("system-file-manager"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setMargin (0);
    mainLayout->setHorizontalSpacing (10);
    mainLayout->setVerticalSpacing (28);

    QLabel* romSizeLabel = new QLabel;
    romSizeLabel->setText(tr("Rom size:"));
    mRomSizeCombox = new QComboBox;
    mainLayout->addWidget(romSizeLabel, 1, 1, 1, 4);
    mainLayout->addWidget(mRomSizeCombox, 1, 5, 1, 8);

    QLabel* fsLabel = new QLabel;
    fsLabel->setText(tr("Filesystem:"));
    mFSCombox = new QComboBox;
    mFSCombox->addItem("vfat/fat32");
    mFSCombox->addItem("exfat");
    mFSCombox->addItem("ntfs");
    mFSCombox->addItem("ext4");
    mainLayout->addWidget(fsLabel, 2, 1, 1, 4);
    mainLayout->addWidget(mFSCombox, 2, 5, 1, 8);

    QLabel* uNameLabel = new QLabel;
    uNameLabel->setText(tr("Disk name:"));
    mNameEdit = new QLineEdit;
    mainLayout->addWidget(uNameLabel, 3, 1, 1, 4);
    mainLayout->addWidget(mNameEdit, 3, 5, 1, 8);
    //fix can give name more than 11 characters,link to bug#113257
    //FIXME 设置最大长度为11，但是汉字也可以输入11个，Qt控件提供方法存在的问题
    connect(mFSCombox, &QComboBox::currentTextChanged, [=]()
    {
        auto fsIndex = mFSCombox->currentIndex();
        qDebug() <<"index:"<<fsIndex<<"text:"<<mFSCombox->currentText();
        //set default as 11, match with case 0
        quint8 maxLength = 11;
        switch (fsIndex) {
        case 1:
            maxLength = 15;
            break;
        case 2:
            maxLength = 20;
            break;
        case 3:
            maxLength = 16;
            break;
        default:
            break;
        }

        mNameEdit->setMaxLength(maxLength);
    });

    mEraseCkbox = new QCheckBox;
    mEraseCkbox->setText (tr("Completely erase(Time is longer, please confirm!)"));
    mEraseCkbox->setToolTip(mEraseCkbox->text());
    mainLayout->addWidget(mEraseCkbox, 4, 1, 1, 12, Qt::AlignLeft);

    mProgress = new QProgressBar;
    // show progress bar tooltip when inactived
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    mProgress->setMouseTracking(true);
    mProgress->setMinimum(0);
    mProgress->setValue (0);
    mProgress->setMaximum(100);
    mainLayout->addWidget(mProgress, 5, 1, 1, 12);

    auto cryptCheckBox = new QCheckBox(this);
    // avoid #140543, guestos can not do luks format.
    if (GlobalSettings::getInstance()->isGuestOSMachine()) {
        cryptCheckBox->setVisible(false);
    }
    cryptCheckBox->setText(tr("Set password"));
    cryptCheckBox->setToolTip(tr("Set password for volume based on LUKS (only ext4)"));
    cryptCheckBox->setObjectName("cryptCheckBox");
    mainLayout->addWidget(cryptCheckBox, 6, 1, 1, 6, Qt::AlignLeft);

    connect(mFSCombox, &QComboBox::currentTextChanged, this, [=]{
        if (mFSCombox->currentText() == "ext4") {
            QMessageBox::warning(nullptr, tr("Warning"), tr("Formatting to the ext4 file system may cause other users to be unable to read or write to the USB drive"), QMessageBox::Ok);
            cryptCheckBox->setEnabled(true);
        } else {
            cryptCheckBox->setChecked(false);
            cryptCheckBox->setDisabled(true);
        }
    });

    mCancelBtn = new QPushButton(tr("Cancel"));
    mFormatBtn = new QPushButton(tr("OK"));
    mCancelBtn->setStyle(new ButtonStyle());
    mFormatBtn->setDefault(true);
    mFormatBtn->setProperty("isImportant", true);
    mainLayout->addWidget(mCancelBtn, 6, 7, 1, 3, Qt::AlignRight);
    mainLayout->addWidget(mFormatBtn, 6, 10, 1, 3, Qt::AlignRight);

    mTimer = new QTimer(this);
    mTimer->setInterval(1000);
    connect(mTimer, &QTimer::timeout, this, [=] () {
        int val = mProgress->value();
        if (b_finished) {
            mProgress->setValue(100);
            mTimer->stop();
            return;
        } else if (b_failed) {
            mTimer->stop();
            return;
        }

        //需要使用老的预估机制，修复完全擦除格式化在100%等待问题，关联 bug#105901
        //Fix full clean format way waiting in 100% issue, link to bug#105901.
        formatloop();
//        if (val <= mProgress->maximum()) {
//            mProgress->setValue(++val);
//        }
    });

    fm_uris = m_uris;
    fm_item = m_item;
    m_parent = parent;
    b_canClose = true;


    g_autoptr (GError) error = NULL;
    g_autoptr (GFile) file = g_file_new_for_uri (m_uris.toUtf8 ().constData ());
    if (G_IS_FILE (file)) {
        g_autoptr (GFileInfo) fileInfo = g_file_query_info (file, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE, G_FILE_QUERY_INFO_NONE, NULL, &error);
        if (G_IS_FILE_INFO (fileInfo)) {
            mVolumeName = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);
        }
    }

    mVolumeMonitor = g_volume_monitor_get ();

    g_signal_connect (mVolumeMonitor, "drive-disconnected", GCallback(volume_disconnect), this);

    /*!
      如果没有uri，尝试从computer:///获取uri，实际上未必能获取到computer:///的uri，比如#90081这种情况
     */
    if (fm_uris.isEmpty() && fm_item && !fm_item->getDevice().isEmpty()) {
        auto itemUris = FileUtils::getChildrenUris("computer:///");
        for (auto itemUri : itemUris) {
            auto unixDevice = FileUtils::getUnixDevice(itemUri);
            if (unixDevice == fm_item->getDevice()) {
                fm_uris = itemUri;
                break;
            }
        }
    }

    //from uris get the rom size
    //FIXME: replace BLOCKING api in ui thread.
    auto targetUri = FileUtils::getTargetUri(fm_uris);
    if (targetUri.isEmpty()) {
        targetUri = fm_uris;
    }
    g_autoptr(GFile) fm_file = g_file_new_for_uri(targetUri .toUtf8().constData());

    g_autoptr(GFileInfo) fm_info = g_file_query_filesystem_info(fm_file, "*", nullptr, nullptr);
    quint64 total = g_file_info_get_attribute_uint64(fm_info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);

    //add the rom size value into  rom_size combox
    //fix system Udisk calculate size wrong issue
    QString m_volume_name, m_unix_device, m_display_name,m_fs_type;
    FileUtils::queryVolumeInfo(m_uris, m_volume_name, m_unix_device, m_display_name);
    bool hasSetRomSize = false;
    m_fs_type = FileUtils::getFileSystemType(m_uris);
    //U disk or other mobile device, only use in iso system install device
    if (! m_unix_device.isEmpty() && m_fs_type.startsWith("iso")
        && ! m_uris.startsWith("computer:///WDC")) {
        char dev_name[256] ={0};
        strncpy(dev_name, m_unix_device.toUtf8().constData(),sizeof(m_unix_device.toUtf8().constData()-1));
        auto size = FileUtils::getDeviceSize(dev_name);
        if (size > 0) {
            QString sizeInfo = QString::number(size, 'f', 1);
            qDebug() << "size:" <<size;
            sizeInfo += "G";
            mRomSizeCombox->clear ();
            mRomSizeCombox->addItem (sizeInfo);
            hasSetRomSize = true;
        }
    }

    if (! hasSetRomSize) {
        //Calculated by 1024 bytes
        char *total_format = strtok(g_format_size_full(total,G_FORMAT_SIZE_IEC_UNITS),"iB");
        mRomSizeCombox->clear ();
        mRomSizeCombox->addItem (total_format);
    }

    auto mount = VolumeManager::getMountFromUri(targetUri);
    //fix name not show complete in bottom issue, bug#36887
    if (mount.get()) {
        auto gvolume = g_mount_get_volume(mount->getGMount());
        dialogVolumes.insert(this, gvolume);

        g_autoptr (GDrive) gdrive = g_volume_get_drive(gvolume);
        auto volumes = g_drive_get_volumes(gdrive);
        int volumeCount = g_list_length(volumes);
        g_list_free_full(volumes, g_object_unref);
        setProperty("formatDriveVolumesCount", volumeCount);

        if(m_uris == "file:///data" || targetUri == "file:///data"){
            mNameEdit->setText(tr("Data"));
        }else{
            mNameEdit->setText(mount->name());
        }
    } else {
        mNameEdit->setText(LinuxPWDHelper::getCurrentUser().fullName());
    }

    mProgress->setValue(0);

    // vfat/fat32 格式存在卷标相关问题，目前无法解决只能规避，参考#83826
    // 如果是可移动设备，格式化时默认使用ntfs格式，可以支持长卷标名，不可移动设备默认按Ext4格式格式化
    g_autoptr(GFile) computer_file = g_file_new_for_uri(m_uris.toUtf8().constData());
    g_autoptr(GFileInfo) gfileinfo = g_file_query_info(computer_file, "*", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    bool isRemoveable = g_file_info_get_attribute_boolean(gfileinfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT) ||g_file_info_get_attribute_boolean(gfileinfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_STOP);
    bool mountable = g_file_info_get_attribute_boolean(gfileinfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_MOUNT) || g_file_info_get_attribute_boolean(gfileinfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT);
    if (isRemoveable && mountable) {
        mFSCombox->setCurrentText("ntfs");
    } else {
        mFSCombox->setCurrentText("ext4");
    }

    connect(mFormatBtn, SIGNAL(clicked(bool)), this, SLOT(acceptFormat(bool)), Qt::UniqueConnection);

    connect(mCancelBtn, SIGNAL(clicked(bool)), this, SLOT(colseFormat(bool)), Qt::UniqueConnection);
    connect(this,&Format_Dialog::ensure_format, this, &Format_Dialog::slot_format, Qt::UniqueConnection);
}

void Format_Dialog::colseFormat(bool)
{

    char dev_name[256] ={0};
    //get device name
    QString volname, devName, voldisplayname;

    //FIXME: replace BLOCKING api in ui thread.
    FileUtils::queryVolumeInfo(fm_uris, volname, devName, voldisplayname);
    strncpy(dev_name,devName.toUtf8().constData(),sizeof(devName.toUtf8().constData()-1));

    //cancel format function
    cancel_format(dev_name);
}

void Format_Dialog::slot_format(bool enable)
{
    if(!enable)
        return;

    /*!
      try fix #125189, format encrypted volume failed sometimes.

      note that device name might not be updated during unmount callback.
      to make sure volume changed signal was handled, we need to delay a
      few times.
      */
    QTimer::singleShot(100, this, [=]{
        int full_clean = 0;
        full_clean = mEraseCkbox->isChecked();
        //恢复之前被删除的代码，尝试修复在100%进度等待问题，bug#105901
        if(full_clean){
            //完全擦除方式格式化，从udisksjob获取进度，1秒更新一次
            mTimer->setInterval(1000);
            m_total_predict = -1;
        }else{
            //快速格式化，预估时间为75S,0.5秒更新一次
            mTimer->setInterval(500);
            m_total_predict = 150;
        }

        mTimer->start();

        // set ui button disable
        mFormatBtn->setDisabled(TRUE);
        mCancelBtn->setDisabled(TRUE);
        //ui->lineEdit_device_name->setDisabled(TRUE);
        //use set readonly property, fix exit issue link to task#33686
        mNameEdit->setReadOnly(true);
        mEraseCkbox->setDisabled(TRUE);

        auto cryptCheckBox = findPasswdCheckBox(this);
        cryptCheckBox->setDisabled(true);

        //init the value
        char rom_size[1024] ={0},rom_type[1024]={0},rom_name[1024]={0},dev_name[1024]={0};


        QString romType = mFSCombox->currentText();
        if (QString("vfat/fat32") == romType) {
            romType = "vfat";
            if (mNameEdit->text().trimmed ().toUtf8().length() <= 11){
               strncpy(rom_name,mNameEdit->text().trimmed ().toUtf8().constData(), sizeof (rom_name) - 1);
            }                    
        } else {
            strncpy(rom_name,mNameEdit->text().trimmed ().toUtf8().constData(), sizeof (rom_name) - 1);
        }

        //get values from ui
        strncpy(rom_size,mRomSizeCombox->currentText ().toUtf8().constData(), strlen(mRomSizeCombox->currentText ().toUtf8().constData()));
        strncpy(rom_type, romType.toUtf8().constData(), strlen(romType.toUtf8().constData()));

        //disable name and rom size list
        //ui->comboBox_rom_size->setDisabled(true);
        this->mFSCombox->setDisabled(true);

        QString volname, devName, voldisplayname ,devtype;
        //get device name
        //FIXME: replace BLOCKING api in ui thread.
        FileUtils::queryVolumeInfo(fm_uris, volname, devName, voldisplayname);

        strncpy(dev_name,devName.toUtf8().constData(), sizeof (dev_name) - 1);
        devtype = rom_type;

        int format_value = 0;

        bool canUseAtaSecureErase = false;
        // judge fastest erase type
        //fix bug#211997, all block use "zero" way, to avoid format fail issue
        //comment canUseAtaSecureErase way
//        if (full_clean) {
//            // judge if drive only has one volume
//            if (property("formatDriveVolumesCount").toInt() <= 1) {
//                // judge drive has ata commands supported
//                g_autoptr (UDisksClient) client = udisks_client_new_sync(0, 0);
//                if (client) {
//                    struct stat statbuf;
//                    int ret = stat(dev_name, &statbuf);
//                    if (ret == 0) {
//                        g_autoptr (UDisksBlock) block = udisks_client_get_block_for_dev(client, statbuf.st_rdev);
//                        if (block) {
//                            auto drive_path = udisks_block_get_drive(block);
//                            if (drive_path) {
//                                g_autoptr (UDisksObject) drive_object = udisks_client_get_object(client, drive_path);
//                                if (drive_object) {
//                                    g_autoptr (UDisksDriveAta) drive_ata = udisks_object_get_drive_ata(drive_object);
//                                    if (drive_ata) {
//                                        canUseAtaSecureErase = true;
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }

        //do format
        kdisk_format(dev_name, devtype.toLower().toUtf8().constData(),
                     full_clean? (canUseAtaSecureErase? "ata-secure-erase": "zero"): NULL, rom_name, &format_value);
    });
}


static void unmount_finished(GFile* file, GAsyncResult* result, gpointer udata)
{
    qDebug()<<"unmount finished";

    int flags = 0;
    GError *err = nullptr;
    Format_Dialog *pthis = (Format_Dialog *)udata;

    if (g_file_unmount_mountable_with_operation_finish (file, result, &err) == TRUE){
        flags = 1;
        char *uri = g_file_get_uri(file);
        VolumeManager::getInstance()->fileUnmounted(uri);
        if (uri)
            g_free(uri);
    }

    //add QMessageBox error ,let user know the error messages
    
    if (err) {
        flags = 0;
        b_failed = true;
        if(G_IO_ERROR_BUSY == err->code){/* 卷被占用时，防止二次弹出信息提示框 */
            g_error_free(err);
            /* 在此处需要做取消格式化操作，格式化对话框恢复到初始化状态，以备再次格式化 */
            pthis->setBtnStatus(true);
            return;
        }
        QMessageBox message_error(pthis);
          
        message_error.setText(QObject::tr("Error: %1\n").arg(err->message));

        message_error.setWindowTitle(QObject::tr("Format failed"));

        QPushButton *okButton = message_error.addButton(QObject::tr("YES"),QMessageBox::YesRole);

        message_error.exec();
          
        if(message_error.clickedButton() == okButton)
        {    
            pthis->close();
        }    
        
        g_error_free(err);
    }

    
    if(flags){
        Q_EMIT pthis->ensure_format(true);
    }
}

void Format_Dialog::acceptFormat(bool)
{
    if (!mNameEdit->text().isEmpty() && mNameEdit->text().at(0) == '.') {
        QMessageBox::warning(nullptr, tr("Warning"), tr("Device name cannot start with a decimal point, Please re-enter!"), QMessageBox::Ok);
        return;
    }

    bool setPassword = false;
    QString password = nullptr;
    auto cryptCheckBox = findPasswdCheckBox(this);
inputpasswd:
    if (cryptCheckBox->isChecked()) {
        setPassword = true;
        QInputDialog dlg;
        dlg.setLabelText(tr("Enter Password:"));
        dlg.setTextEchoMode(QLineEdit::Password);
        dlg.setFixedSize(370, 150);
        auto lineEdit = dlg.findChild<QLineEdit *>();
        lineEdit->setMaxLength(16);
        if (lineEdit) {
            QRegExp rx("^[\\S]*$");
            auto validator = new QRegExpValidator(rx, lineEdit);
            lineEdit->setValidator(validator);
        }
        if (dlg.exec()) {
            password = dlg.textValue();
        } else {
            this->close();
            return;
        }

        if (password.length() < 6) {
            QMessageBox::warning(0, 0, tr("Password too short, please retype a password more than 6 characters"));
            goto inputpasswd;
        }
    }

    this->setProperty("password", password);

    cryptCheckBox->setDisabled(true);

    mFormatBtn->setDisabled(true);
    mCancelBtn->setDisabled(true);

    //check format or not 
    if(!format_makesure_dialog()){
        mFormatBtn->setDisabled(false);
        mCancelBtn->setDisabled(false);
        return;
    };

    //keep this window above, fix bug #41227
//    KWindowSystem::setState(this->winId(), KWindowSystem::KeepAbove);

    bool bEnsureFormat = false;
    auto info = FileInfo::fromUri(fm_uris);
    if (info.get()->isEmptyInfo()) {
        FileInfoJob j(info);
        j.querySync();
    }
    auto targetUri = FileUtils::getTargetUri(fm_uris);
    auto mount = VolumeManager::getInstance()->getMountFromUri(targetUri);
    if(mount) {
       /* unmount */
       auto files = wrapGFile(g_file_new_for_uri(this->fm_uris.toUtf8().constData()));
       auto mount_op = Experimental_Peony::VolumeManager::getInstance()->getOccupiedInfoThread()->getMountOp();
       g_file_unmount_mountable_with_operation(files.get()->get(),
                                               G_MOUNT_UNMOUNT_NONE,
                                               mount_op,
                                               nullptr,
                                               GAsyncReadyCallback(unmount_finished),
                                               this);
    } else {
        bEnsureFormat = true;
    } 

    //set init value, fix init progress is not 0 issue
    //修复开始格式化时，进度直接跳到上一次格式化的百分比问题
    m_before_progress = 0;
    m_cost_seconds = 0;
    m_simulate_progress = 0;
    b_finished = false;
    b_failed = false;

    if(bEnsureFormat){
        Q_EMIT ensure_format(true);
        bEnsureFormat = false;
    }
}

double Format_Dialog::get_format_bytes_done(const gchar * device_name)
{
    UDisksObject *object ;
    UDisksClient *client =udisks_client_new_sync (NULL,NULL);
    object = get_object_from_block_device(client,device_name);
    GList *jobs = NULL;
    if (object) {
        jobs = udisks_client_get_jobs_for_object(client,object);
        g_clear_object(&client);
        g_object_unref(object);
    }
    if(jobs!=NULL)
    {
        mProgress->setRange(0, 100);
        UDisksJob *job =(UDisksJob *)jobs->data;
        if(udisks_job_get_progress_valid (job))
        {
            setProperty("hasProgress", true);
                double res = udisks_job_get_progress(job);
                guint64 rate = udisks_job_get_rate(job);
                guint64 expect_end_time = udisks_job_get_expected_end_time(job);
                guint64 time_left = expect_end_time - g_get_real_time();
                if (rate > 0) {
                    g_autofree gchar* size_format = g_format_size(rate);
                    QString speed = size_format;
                    bool showMinutes = false;
                    bool showHours = false;
                    auto seconds = time_left/1000000;
                    auto minutes = seconds/60;
                    if (minutes) {
                        showMinutes = true;
                        seconds = seconds%60;
                    }
                    auto hours = minutes/60;
                    if (hours) {
                        showHours = true;
                        minutes = minutes%60;
                    }

                    QTime t(hours, minutes, seconds);
                    auto timeString = t.toString();
                    mProgress->setToolTip(tr("%1/sec, %2 remaining.").arg(speed).arg(timeString.isEmpty()? tr("over one day"): timeString));
                } else {
                    mProgress->setToolTip(tr("getting progress..."));
                }

                g_list_foreach (jobs, (GFunc) g_object_unref, NULL);
                g_list_free (jobs);

                return res;
        }

        g_list_foreach (jobs, (GFunc) g_object_unref, NULL);
        g_list_free (jobs);
    } else {
        if (!property("hasProgress").toBool()) {
            mProgress->setRange(0, 0);
        }
    }

    return 0;
}

void Format_Dialog::formatloop(){

//    if (b_finished)
//    {
//        mProgress->setValue(100);
////        ui->label_process->setText("100%");
//        mTimer->stop();
//        return;
//    }
//    else if (b_failed)
//    {
//        mTimer->stop();
//        return;
//    }

    QString volname, devName, voldisplayname;
    static char name_dev[256] ={0};
//    char prestr[10] = {0};

    //cost time count
    m_cost_seconds++;

    //FIXME: replace BLOCKING api in ui thread.
    FileUtils::queryVolumeInfo(fm_uris, volname, devName, voldisplayname);

    if(nullptr != devName)
    strcpy(name_dev,devName.toUtf8().constData());

    if (m_total_predict > 0) {
        double cost = m_cost_seconds * 100.0/m_total_predict;
        if (cost >= 99) {
            cost = 99;
            mTimer->stop();
        }
        m_simulate_progress = cost;
        qDebug()<<"get format 0.5s timer count"<<cost;
    } else {
        double pre = (get_format_bytes_done(name_dev) * 100);
        m_simulate_progress = pre;
        qDebug()<<"get erase-format progress"<<pre;
    }

    //防止看起来回退现象，进度值比之前还小了
    if (m_simulate_progress < m_before_progress)
        m_simulate_progress = m_before_progress;

    m_before_progress = m_simulate_progress;
//    sprintf(prestr,"%.1f",m_simulate_progress);
//    strcat(prestr,"%");

    if(m_simulate_progress>=100){
        b_finished = true;
        mProgress->setValue(100);
//        ui->label_process->setText("100%");
        mTimer->stop();
    }
    else{
        mProgress->setValue(m_simulate_progress);
//          ui->label_process->setText(prestr);
    }
}

void Format_Dialog::volume_disconnect(GVolumeMonitor *vm, GDrive *v, gpointer data)
{
    g_autofree char* devName = NULL;

    Format_Dialog* fd = (Format_Dialog*) data;

    if (G_IS_DRIVE (v)) {
        devName = g_drive_get_identifier ((GDrive*) v, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    }

    if (fd && devName && !fd->mVolumeName.isNull () && !g_ascii_strcasecmp (devName, fd->mVolumeName.toUtf8 ().constData ())) {
        fd->reject ();
    }

    Q_UNUSED (vm)
}

void Format_Dialog::cancel_format(const gchar* device_name){
    this->close();

//    UDisksObject *object ;
//    UDisksBlock *block;
//    UDisksClient *client =udisks_client_new_sync (NULL,NULL);
//    object = get_object_from_block_device(client,device_name);
//    block = udisks_object_get_block (object);
//    GList *jobs;

//    jobs = udisks_client_get_jobs_for_object (client,object);

//    if(jobs != NULL){
//                udisks_job_call_cancel_sync ((UDisksJob *)jobs->data,
//                           g_variant_new ("a{sv}", NULL),
//                           NULL,
//                           NULL);
//    }

//    g_list_foreach (jobs, (GFunc) g_object_unref, NULL);
//    g_list_free (jobs);
//    g_clear_object(&client);
//    g_object_unref(object);
//    g_object_unref(block);


};


/*  is_iso  function to  check is iso or not
 *
 *  args : const gchar *device_path
 *
 *  device_path: device path
 */
gboolean Format_Dialog::is_iso(const gchar *device_path){
    UDisksObject *object;
    UDisksClient *client;
    UDisksBlock *block;
    client = udisks_client_new_sync(NULL,NULL);
    object = get_object_from_block_device(client,device_path);

    //fix format U disk crash issue
    if (! object)
        return FALSE;

    block = udisks_object_get_block(object);

    if(g_strcmp0(udisks_block_get_id_type(block),"iso9660")==0)
    {

        g_object_unref(object);
        g_object_unref(block);
        g_clear_object(&client);
        return TRUE;
    }

    g_object_unref(object);
    g_object_unref(block);
    g_clear_object(&client);

    return FALSE;

};


/*
 * ensure_unused_cb   ensure to unused this
 *
 * args:
 *
 * CreateformatData *data: the data info device
 *
 * if is not iso , do ensure_format_cb.
 *
 * if is do format_disk
 *
 */
void Format_Dialog::ensure_unused_cb(CreateformatData *data)
{
    b_canClose = false;

    if(is_iso(data->device_name)==FALSE) {     
        ensure_format_cb (data);
    }
    else {
        ensure_format_disk(data);
    }
}

static void createformatfree(CreateformatData *data)
{
    g_variant_builder_clear(data->builder);
    if (data->object) {
        g_object_unref(data->object);
    }
    if (data->block) {
        g_object_unref(data->block);
    }
    if(data->drive_object!=NULL)
    {
        g_object_unref(data->drive_object);
    }
    if(data->drive_block!=NULL)
    {
        g_object_unref(data->drive_block);
    }
    g_clear_object(&(data->client));

    g_free(data);
}

UDisksObject* getObjectFromBlockDevice(UDisksClient* client, const gchar* bdevice)
{
    struct stat statbuf;
    UDisksBlock* block = NULL;
    UDisksObject* object = NULL;
    UDisksObject* cryptoBackingObject = NULL;
    g_autofree const gchar* cryptoBackingDevice = NULL;

    g_return_val_if_fail(stat(bdevice, &statbuf) == 0, object);

    block = udisks_client_get_block_for_dev (client, statbuf.st_rdev);
    g_return_val_if_fail(block != NULL, object);

    object = UDISKS_OBJECT (g_dbus_interface_dup_object (G_DBUS_INTERFACE (block)));

    cryptoBackingDevice = udisks_block_get_crypto_backing_device ((udisks_object_get_block (object)));
    cryptoBackingObject = udisks_client_get_object (client, cryptoBackingDevice);
    if (cryptoBackingObject != NULL) {
        g_object_unref (object);
        object = cryptoBackingObject;
    }

    g_object_unref (block);

    return object;
}

// format
void Format_Dialog::format_cb (GObject *source_object, GAsyncResult *res ,gpointer user_data)
{
    qDebug()<<"format cb start";
    static int end_flag = -1;

    CreateformatData *data = (CreateformatData *)user_data;
    GError *error = NULL;
    QString curName = "";
    if (!udisks_block_call_format_finish (UDISKS_BLOCK (source_object), res,&error))
    {
        char *p = NULL;
        if(NULL != error && NULL != (p=strstr(error->message, "wipefs:"))){
            notify_init("Peony");
            NotifyNotification *notify;
            QString title = QObject::tr("File Manager");
            notify = notify_notification_new (title.toUtf8().constData(),
                                              p,
                                              "drive-removable-media-usb"
                                              );
            notify_notification_show(notify,NULL);
            g_object_unref(G_OBJECT(notify));
            notify_uninit();
            g_clear_error(&error);
        }
        end_flag = -1;
        *(data->format_finish) =  -1; //format error
    }
    else
    {
        g_autoptr (UDisksClient) client = udisks_client_new_sync(NULL, NULL);
        if (client) {
            g_autoptr (UDisksObject) udiskObj = getObjectFromBlockDevice(client, data->dl->mVolumeName.toStdString().c_str());
            if (udiskObj) {
                g_autoptr (UDisksBlock) diskBlock = udisks_object_get_block (udiskObj);
                if (diskBlock) {
                    curName = udisks_block_get_id_label (diskBlock);
                    qDebug () << data->dl->mVolumeName << "  --  " << data->filesystem_name << "  --  " << curName;
                } else {
                    // might be crypted volume
                    curName = data->dl->mNameEdit->text().trimmed();
                }
            } else {
                // might be crypted volume
                curName = data->dl->mNameEdit->text().trimmed();
            }
        }

        // rename fail
        // fixme: deal with crypt volume.
        // fixme: send to device is enabled for crypt volume
        if (data->dl->mEraseCkbox->isChecked()) {
            if (!curName.isEmpty() && data->dl->mNameEdit->text ().trimmed () != curName && data->dl->property("password").isNull()) {
                data->dl->renameOK = false;
            }
        } else {
            if (data->dl->mNameEdit->text ().trimmed () != curName && data->dl->property("password").isNull()) {
                data->dl->renameOK = false;
            }
        }

        end_flag = 1;
        *(data->format_finish) =  1; //format success
    }

    if(end_flag == 1){
        b_finished = true;
        data->dl->setProperty("isFinished", true);
        data->dl->mProgress->setValue(100);
//        data->dl->ui->label_process->setText("100%");
        data->dl->format_ok_dialog();
    }
    else{
        b_failed = true;
        data->dl->format_err_dialog();  
    }

    b_canClose = true;

    data->dl->setProperty("password", QVariant());
    data->dl->mTimer->stop();
    data->dl->close();
    data->dl->deleteLater();

    createformatfree(data);
    qDebug()<<"format cb end";
};


void Format_Dialog::format_ok_dialog()
{
    //fix bug#177146, play finish sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
    kdk::KSoundEffects::playSound(SoundType::COMPLETE);
#endif
    if (renameOK) {
        QMessageBox::about(this,QObject::tr("format"),QObject::tr("Format operation has been finished successfully."));
    } else {
        QMessageBox::about(this,QObject::tr("format"),QObject::tr("Formatting successful! But failed to set the device name."));
    }

    mCancelBtn->setEnabled(true);

    auto cryptCheckBox = findPasswdCheckBox(this);
    if (mFSCombox->currentText() == "ext4") {
        cryptCheckBox->setEnabled(true);
    }
}


void Format_Dialog::format_err_dialog()
{
#ifdef LINGMO_SDK_SOUND_EFFECTS
    kdk::KSoundEffects::playSound(SoundType::DIALOG_ERROR);
#endif
    QMessageBox::warning(this,QObject::tr("qmesg_notify"),QObject::tr("Sorry, the format operation is failed!"));
    mCancelBtn->setEnabled(true);

    auto cryptCheckBox = findPasswdCheckBox(this);
    if (mFSCombox->currentText() == "ext4") {
        cryptCheckBox->setEnabled(true);
    }
}

bool Format_Dialog::format_makesure_dialog(){

    QMessageBox* message_format = new QMessageBox (this);

    message_format->setText(QObject::tr("Formatting this volume will erase all data on it. Please backup all retained data before formatting. Do you want to continue ?"));

    message_format->setWindowTitle(QObject::tr("Format"));

    QPushButton *okButton = message_format->addButton(QObject::tr("Begin Format"),QMessageBox::YesRole);
    okButton->setDefault(true);

    QPushButton *cancelButton = message_format->addButton(QObject::tr("Close"),QMessageBox::NoRole);

    message_format->connect (this, &QDialog::finished, message_format, [=] (int) {
        Q_EMIT cancelButton->clicked ();
        message_format->deleteLater ();
    });

    //fix bug#177143, play warning sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
    kdk::KSoundEffects::playSound(SoundType::DIALOG_WARNING);
#endif
    message_format->exec();

    if(message_format->clickedButton() == cancelButton)
    {
        mFormatBtn->setDisabled(false);
        mCancelBtn->setDisabled(false);
        return false;
    }else if(message_format->clickedButton() == okButton){
        return true;
    }

    //close window, return false, link to bug#92330
    return false;
}

/* ensure_format_cb ,function ensure to do format
 *
 * args: CreateformatData *data
 *
 */
void Format_Dialog::ensure_format_cb (CreateformatData *data){

    qDebug()<<"ensure format cb start";
    GVariantBuilder *options_builder = data->builder;

    g_variant_builder_init(options_builder, G_VARIANT_TYPE ("a{sv}"));

    if (g_strcmp0 (data->format_type, "empty") != 0){
        g_variant_builder_add (options_builder, "{sv}", "label",
                               g_variant_new_string (data->filesystem_name));
    };

    if (g_strcmp0 (data->format_type, "vfat") != 0 &&
            g_strcmp0 (data->format_type, "ntfs") != 0 &&
            g_strcmp0 (data->format_type, "exfat") != 0) {
        g_variant_builder_add (options_builder, "{sv}", "take-ownership",
                               g_variant_new_boolean (TRUE));
    }

    QString password = data->dl->property("password").toString();
    if (!password.isEmpty()) {
        const gchar *passphrase = password.toUtf8().constData();
        g_variant_builder_add (options_builder, "{sv}", "encrypt.passphrase",
                               g_variant_new_string(passphrase));
    }

    if (data->erase_type != NULL){
        g_variant_builder_add (options_builder, "{sv}", "erase",
                               g_variant_new_string (data->erase_type));
    }

    g_variant_builder_add (options_builder, "{sv}", "update-partition-type",
                           g_variant_new_boolean (TRUE));

    udisks_block_call_format (data->block,
                              data->format_type,
                              g_variant_builder_end (options_builder),
                              NULL,
                              format_cb,
                              data);

    qDebug()<<"ensure format cb end";
};


/*
 * ensure format iso disk
 *
 *
*/
void Format_Dialog::ensure_format_disk(CreateformatData *data){

        char ch[10]={0};
        for(int i=0;i<=7;i++)
            ch[i]=(data->device_name)[i];
        data->drive_object = get_object_from_block_device(data->client,ch);
        g_return_if_fail (data->drive_object);

        data->drive_block = udisks_object_get_block(data->drive_object);
        g_return_if_fail (data->drive_block);


        GVariantBuilder *options_builder = data->builder;
        g_variant_builder_init(options_builder,G_VARIANT_TYPE_VARDICT);


        if (g_strcmp0 (data->format_type, "empty") != 0){
            g_variant_builder_add (options_builder, "{sv}", "label",
                                   g_variant_new_string (data->filesystem_name));
        };


        if (g_strcmp0 (data->format_type, "vfat") != 0 &&
                g_strcmp0 (data->format_type, "ntfs") != 0 &&
                g_strcmp0 (data->format_type, "exfat") != 0) {
            g_variant_builder_add (options_builder, "{sv}", "take-ownership",
                                   g_variant_new_boolean (TRUE));
        }

        if (data->erase_type != NULL){
            g_variant_builder_add (options_builder, "{sv}", "erase",
                                   g_variant_new_string (data->erase_type));
        }

        g_variant_builder_add (options_builder, "{sv}", "update-partition-type",
                               g_variant_new_boolean (TRUE));


        udisks_block_call_format(data->drive_block,
                        data->format_type,
                        g_variant_builder_end(options_builder),
                        NULL,
                        format_cb,
                        data);

}

UDisksObject *Format_Dialog::get_object_from_block_device (UDisksClient *client,const gchar *block_device)
{
    struct stat statbuf;
    const gchar *crypto_backing_device;
    UDisksObject *object, *crypto_backing_object;
    UDisksBlock *block;

    object = NULL;

    if (stat (block_device, &statbuf) != 0)
    {
        return object;
    }

    block = udisks_client_get_block_for_dev (client, statbuf.st_rdev);
    if (block == NULL)
    {
        return object;
    }

    object = UDISKS_OBJECT (g_dbus_interface_dup_object (G_DBUS_INTERFACE (block)));
    g_object_unref (block);

    crypto_backing_device = udisks_block_get_crypto_backing_device ((udisks_object_get_block (object)));
    crypto_backing_object = udisks_client_get_object (client, crypto_backing_device);
    if (crypto_backing_object != NULL)
    {
        g_object_unref (object);
        object = crypto_backing_object;
    }
    return object;
}

/*  kdisk_format  function to format device
 *
 *  args:
 *  kdisk_format format the disk and  get_object_from_device
 *  device_name: device name
 *  format_type: device type to format
 *  erase_type:  clean it quick or not ,if zero not NULL
 *  filesystem_name: the device name display in filesystem
 *  format_finish: format finish is 1 ,not 0
 */
void Format_Dialog::kdisk_format(const gchar * device_name,const gchar *format_type,const gchar * erase_type,
                                const gchar * filesystem_name,int *format_finish){

    qDebug()<<"do format";
    CreateformatData *data;
    data = g_new(CreateformatData,1);

    data->format_finish = 0;
    data->device_name = device_name;
    data->format_type = format_type;
    data->erase_type = erase_type;
    data->filesystem_name = filesystem_name;
    data->format_finish = format_finish;

    data->builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);

    data->object = NULL;
    data->block = NULL;
    data->drive_object = NULL;
    data->drive_block = NULL;
    data->dl = this;

    data->client = udisks_client_new_sync(NULL,NULL);

    data->object = get_object_from_block_device(data->client,data->device_name);
    if (data->object) {
        data->block = udisks_object_get_block(data->object);

        ensure_unused_cb(data);
    } else {
        // 也许是加密分区卸载后device name变更导致，需要先做处理
        auto gvolume = dialogVolumes.value(this);
        qInfo()<<"try to get latest device name from gvolume";
        if (gvolume) {
            g_autofree gchar* unixDevice = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            data->object = get_object_from_block_device(data->client, unixDevice);
            if (data->object) {
                qInfo()<<"use latest device name:"<<unixDevice;
                data->block = udisks_object_get_block(data->object);
                ensure_unused_cb(data);
                return;
            } else {
                qInfo()<<"failed to get latest valid device name from gvolume, latest device name:"<<unixDevice;
            }
        } else {
            qWarning()<<"can not find gvolume for current dialog, unexpected error!";
        }

        // fix #103344
        QMessageBox::critical(0, tr("Error"), tr("Block not existed!"));
        mTimer->stop();
        this->cancel_format(data->device_name);
        createformatfree(data);
        this->deleteLater();
    }
}


Format_Dialog::~Format_Dialog()
{
    FormatDlgCreateDelegate::getInstance()->removeFromUdiskMap(this->fm_uris);
    g_signal_handlers_disconnect_by_data(mVolumeMonitor, this);
//    delete ui;
    if (mTimer)             mTimer->deleteLater();
    if (mNameEdit)          mNameEdit->deleteLater();
    if (mFSCombox)          mFSCombox->deleteLater();
    if (mProgress)          mProgress->deleteLater();
    if (mCancelBtn)         mCancelBtn->deleteLater();
    if (mFormatBtn)         mFormatBtn->deleteLater();
    if (mEraseCkbox)        mEraseCkbox->deleteLater();
    if (mRomSizeCombox)     mRomSizeCombox->deleteLater();
    if (mVolumeMonitor)     g_object_unref (mVolumeMonitor);

    auto gvolume = dialogVolumes.take(this);
    if (gvolume) {
        g_object_unref(gvolume);
    }

    b_canClose = true;
}

void Format_Dialog::setBtnStatus(bool enable)
{
    auto cryptCheckBox = findPasswdCheckBox(this);
    if (mFSCombox->currentText() == "ext4") {
        cryptCheckBox->setEnabled(true);
    }

    mFormatBtn->setEnabled(enable);
    mCancelBtn->setEnabled(enable);
}

void Format_Dialog::closeEvent(QCloseEvent *e)
{
    if (!b_canClose) {

        QMessageBox::warning(nullptr, tr("Formatting. Do not close this window"), tr("Formatting. Do not close this window"), QMessageBox::Ok);

        e->ignore();
        return;
    }

    FormatDlgCreateDelegate::getInstance()->removeFromUdiskMap(this->fm_uris);
}

void Format_Dialog::resizeEvent(QResizeEvent *event)
{
    int width = mEraseCkbox->width() - 25;

    if (mEraseCkbox->fontMetrics().width(mEraseCkbox->text()) > width) {
        mEraseCkbox->setText(mEraseCkbox->fontMetrics().elidedText(mEraseCkbox->text(), Qt::ElideRight, width));
    }

    QWidget::resizeEvent(event);
}
