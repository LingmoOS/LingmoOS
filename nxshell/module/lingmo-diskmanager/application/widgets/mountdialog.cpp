// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "mountdialog.h"
#include "partedproxy/dmdbushandler.h"
#include "messagebox.h"

#include <DLabel>
#include <DFrame>
#include <DFontSizeManager>

#include <QDir>
#include <QVBoxLayout>

MountDialog::MountDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
    initConnection();
}

void MountDialog::initUi()
{
    setMinimumSize(QSize(420, 200));
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION) {
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        setTitle(tr("Mount %1").arg(info.m_path));
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
        setTitle(tr("Mount %1").arg(lvInfo.m_lvPath));
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        setTitle(tr("Mount %1").arg(info.m_path));
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    DLabel *tipLabel = new DLabel(tr("Choose a mount point please"), this);
    tipLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(tipLabel, DFontSizeManager::T6);
    DLabel *mountLabel = new DLabel(tr("Mount point:"));
    DFontSizeManager::instance()->bind(mountLabel, DFontSizeManager::T6);
    m_fileChooserEdit = new DFileChooserEdit(this);
    m_fileChooserEdit->setDirectoryUrl(QUrl("file:///mnt"));
    m_fileChooserEdit->setFileMode(QFileDialog::Directory);
    m_fileChooserEdit->fileDialog()->setOption(QFileDialog::ShowDirsOnly);
    m_fileChooserEdit->fileDialog()->setAcceptMode(QFileDialog::AcceptOpen);
    m_fileChooserEdit->setAccessibleName("mountPointComboBox");
    m_fileChooserEdit->setText("/mnt");

    //    m_ComboBox = new DComboBox;
    //    m_ComboBox->setEditable(true);
    //    m_ComboBox->addItem("/mnt");
    //    m_ComboBox->addItem("/boot");
    //    m_ComboBox->addItem("/");
    //    m_ComboBox->addItem("/tmp");
    //    m_ComboBox->addItem("/var");
    //    m_ComboBox->addItem("/srv");
    //    m_ComboBox->addItem("/opt");
    //    m_ComboBox->addItem("/usr");
    //    m_ComboBox->addItem("/local");
    //    m_ComboBox->addItem("/media");
    //    m_ComboBox->setAccessibleName("mountPointComboBox");

    mainLayout->addWidget(tipLabel);
    mainLayout->addWidget(mountLabel);
    mainLayout->addWidget(m_fileChooserEdit);

    m_mountPointSuggest = new DLabel;
    m_mountPointSuggest->setText(tr("Please select /mnt or /media, or its subdirectories."));
    m_mountPointSuggest->setAlignment(Qt::AlignLeft);
    m_mountPointSuggest->setWordWrap(false);
    m_mountPointSuggest->setElideMode(Qt::ElideMiddle);
    m_mountPointSuggest->setToolTip(tr("Please select /mnt or /media, or its subdirectories."));
    mainLayout->addWidget(m_mountPointSuggest);
    DFontSizeManager::instance()->bind(m_mountPointSuggest, DFontSizeManager::T8);
    m_warnning = new DLabel;
    m_warnning->setText(tr("The mount point is illegal. Please select /mnt or /media, or its subdirectories."));
    m_warnning->setToolTip(tr("The mount point is illegal. Please select /mnt or /media, or its subdirectories."));
    m_warnning->setAlignment(Qt::AlignLeft);
    m_warnning->setVisible(false);
    m_warnning->setWordWrap(false);
    m_warnning->setElideMode(Qt::ElideMiddle);
    DFontSizeManager::instance()->bind(m_warnning, DFontSizeManager::T8);
    m_warnning->setStyleSheet("color:#FF5736;");
    mainLayout->addWidget(m_warnning);
    mainLayout->addStretch();

    int index = addButton(tr("Cancel"), true, ButtonNormal);
    m_okCode = addButton(tr("Mount"), false, ButtonRecommend);
    //    getButton(okcode)->setDisabled(true);
    setOnButtonClickedClose(false);

    getButton(index)->setAccessibleName("cancel");
    getButton(m_okCode)->setAccessibleName("mountButton");
}

void MountDialog::initConnection()
{
    connect(m_fileChooserEdit->lineEdit(), &QLineEdit::textChanged, this, &MountDialog::onEditContentChanged);
    connect(this, &MountDialog::buttonClicked, this, &MountDialog::onButtonClicked);
}

void MountDialog::onEditContentChanged(const QString &content)
{
    if (content.isEmpty()) {
        getButton(m_okCode)->setDisabled(true);
    } else {
        QDir dir(content);
        if (dir.exists()) {
            if (isSystemDirectory(content)) {
                getButton(m_okCode)->setDisabled(true);
                m_warnning->setVisible(true);
                m_mountPointSuggest->setVisible(false);
            } else {
                getButton(m_okCode)->setDisabled(false);
                m_warnning->setVisible(false);
                m_mountPointSuggest->setVisible(true);
            }
            //            m_fileChooserEdit->setAlert(false);
        } else {
            getButton(m_okCode)->setDisabled(true);
            //            m_fileChooserEdit->setAlert(true);
        }
    }
}

bool MountDialog::isExistMountPoint(const QString &mountPoint)
{
    // 判断是否有分区已经挂载在该路径
    DeviceInfoMap infoMap = DMDbusHandler::instance()->probDeviceInfo();
    for (auto devInfo = infoMap.begin(); devInfo != infoMap.end(); devInfo++) {
        DeviceInfo info = devInfo.value();
        for (auto it = info.m_partition.begin(); it != info.m_partition.end(); it++) {
            QString mountpoints;
            for (int i = 0; i < it->m_mountPoints.size(); i++) {
                mountpoints += it->m_mountPoints[i];
                if (it->m_mountPoints.size() > 1 && mountPoint == it->m_mountPoints[i]) {
                    return true;
                }
            }

            if (mountPoint == mountpoints) {
                return true;
            }
        }
    }

    // 判断是否有逻辑卷已经挂载在该路径
    LVMInfo lvmInfo = DMDbusHandler::instance()->probLVMInfo();
    QMap<QString, VGInfo> lstVGInfo = lvmInfo.m_vgInfo;
    for (auto vgInfo = lstVGInfo.begin(); vgInfo != lstVGInfo.end(); vgInfo++) {
        VGInfo vgInformation = vgInfo.value();
        for (int i = 0; i < vgInformation.m_lvlist.count(); i++) {
            LVInfo info = vgInformation.m_lvlist.at(i);
            QString mountpoints;
            for (int j = 0; j < info.m_mountPoints.size(); j++) {
                mountpoints += info.m_mountPoints[j];
                if (info.m_mountPoints.size() > 1 && mountPoint == info.m_mountPoints[j]) {
                    return true;
                }
            }

            if (mountPoint == mountpoints) {
                return true;
            }
        }
    }

    return false;
}

bool MountDialog::isSystemDirectory(const QString &directory)
{
    if (directory.startsWith("/mnt") || directory.startsWith("/media")) {
        return false;
    }
    return true;
}

void MountDialog::mountCurPath()
{
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION) {
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        if (!onCheckMountPoint(static_cast<FSType>(info.m_fileSystemType), m_fileChooserEdit->text())) {
            return;
        }
        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            if (!onCheckMountPoint(luksInfo.m_mapper.m_luksFs, m_fileChooserEdit->text())) {
                return;
            }
            QVector<QString> mountPoints;
            mountPoints.append(m_fileChooserEdit->text());
            luksInfo.m_mapper.m_mountPoints = mountPoints;

            DMDbusHandler::instance()->cryptMount(luksInfo, info.m_path);
        } else {
            DMDbusHandler::instance()->mount(m_fileChooserEdit->text());
        }
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
        if (!onCheckMountPoint(lvInfo.m_lvFsType, m_fileChooserEdit->text())) {
            return;
        }

        if (lvInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(lvInfo.m_lvPath);
            if (!onCheckMountPoint(luksInfo.m_mapper.m_luksFs, m_fileChooserEdit->text())) {
                return;
            }

            QVector<QString> mountPoints;
            mountPoints.append(m_fileChooserEdit->text());
            luksInfo.m_mapper.m_mountPoints = mountPoints;

            DMDbusHandler::instance()->cryptMount(luksInfo, lvInfo.m_lvName);
        } else {
            LVAction lvAction;
            lvAction.m_vgName = lvInfo.m_vgName;
            lvAction.m_lvName = lvInfo.m_lvName;
            lvAction.m_lvFs = lvInfo.m_lvFsType;
            lvAction.m_lvSize = lvInfo.m_lvSize;
            lvAction.m_lvByteSize = lvInfo.m_lvLECount * lvInfo.m_LESize;
            lvAction.m_lvAct = LVMAction::LVM_ACT_LV_MOUNT;
            lvAction.m_mountPoint = m_fileChooserEdit->text();

            DMDbusHandler::instance()->onMountLV(lvAction);
        }
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
            LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(info.m_path);
            QVector<QString> mountPoints;
            mountPoints.append(m_fileChooserEdit->text());
            luksInfo.m_mapper.m_mountPoints = mountPoints;

            DMDbusHandler::instance()->cryptMount(luksInfo, info.m_path);
        }
    }
}

void MountDialog::onButtonClicked(int index, const QString &text)
{
    Q_UNUSED(text);
    if (index == m_okCode) {
        QString mountPath = m_fileChooserEdit->text();
        if (mountPath.endsWith("/")) {
            mountPath.chop(1);
        }

        QDir dir(mountPath);
        if (!dir.isEmpty() || isExistMountPoint(mountPath)) {
            MessageBox messageBox(this);
            messageBox.setObjectName("mountMessageBox");
            messageBox.setAccessibleName("messageBox");
            messageBox.setWarings(tr("The data under this mount point would be lost, please mount the directory to another location"), "",
                                  tr("Continue", "button"), "ok", tr("Cancel"), "cancelBtn");
            if (messageBox.exec() == 1) {
                mountCurPath();
                close();
            }
        } else {
            mountCurPath();
            close();
        }
    } else {
        close();
    }
}

bool MountDialog::onCheckMountPoint(FSType fsType, const QString &mountPoint)
{
    //exfat文件系统的挂载点必须为空目录
    if (fsType == FS_EXFAT) {
        QDir dir(m_fileChooserEdit->text());
        if (!dir.isEmpty()) {
            MessageBox messageBox(this);
            messageBox.setObjectName("mountPointMessageBox");
            messageBox.setAccessibleName("mountPointMessageBox");
            messageBox.setWarings("", tr("Mounting failed: The selected mount point is not empty. Please select another one!"),
                                  tr("OK", "button"), ButtonNormal);
            if (messageBox.exec() == 1) {
                close();
            }
            return false;
        }
    }
    return true;
}
