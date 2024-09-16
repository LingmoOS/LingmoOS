// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "formatedialog.h"
#include "partedproxy/dmdbushandler.h"
#include "common.h"
#include "customcontrol/passwordinputdialog.h"
#include "messagebox.h"

#include <DFontSizeManager>
#include <DWindowCloseButton>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QTimer>

FormateDialog::FormateDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
    initConnection();
}

void FormateDialog::initUi()
{
    m_height = 355;
    setFixedSize(450, m_height);

    m_curWipeMethod = WipeType::FAST;
    int fileSystemType = FSType::FS_EXT4;
    double size = 0;
    LUKSFlag luksFlag = LUKSFlag::NOT_CRYPT_LUKS;
    bool isSystemDevice = false;
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION) {
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        m_pathInfo = info.m_path;
        fileSystemType = info.m_fileSystemType;
        DeviceInfo diskInfo = DMDbusHandler::instance()->getCurDeviceInfo();
        m_curDiskMediaType = diskInfo.m_mediaType;
        size = Utils::sectorToUnit(info.m_sectorEnd - info.m_sectorStart + 1, info.m_sectorSize, SIZE_UNIT::UNIT_MIB);
        luksFlag = info.m_luksFlag;
        isSystemDevice = DMDbusHandler::instance()->getIsSystemDisk(info.m_devicePath);
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        m_pathInfo = info.m_path;
        m_curDiskMediaType = info.m_mediaType;
        size = Utils::sectorToUnit(info.m_length, info.m_sectorSize, SIZE_UNIT::UNIT_MIB);
        luksFlag = info.m_luksFlag;
        isSystemDevice = DMDbusHandler::instance()->getIsSystemDisk(info.m_path);
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        m_height = 315;
        setFixedSize(450, m_height);
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
        m_pathInfo = lvInfo.m_lvPath;
        fileSystemType = lvInfo.m_lvFsType;
        size = Utils::LVMSizeToUnit(lvInfo.m_lvLECount * lvInfo.m_LESize, SIZE_UNIT::UNIT_MIB);
        luksFlag = lvInfo.m_luksFlag;
        isSystemDevice = DMDbusHandler::instance()->getIsSystemDisk(lvInfo.m_vgName);
    }

    DPalette palette1;
    palette1.setColor(DPalette::Text, "#666666");

    DPalette palette2;
    palette2.setColor(DPalette::Text, QColor("#526a7f"));

    DPalette palette3;
    palette3.setColor(DPalette::TextTitle, QColor("#001a2e"));

    DPalette palette4;
    QColor color4("#000000");
    color4.setAlphaF(0.9);
    palette4.setColor(DPalette::ToolTipText, color4);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    setTitle(tr("Wipe %1").arg(m_pathInfo));
    DLabel *tipLabel = new DLabel(tr("It will erase all data on this disk, which will not be recovered"), this);
    tipLabel->setWordWrap(true);
    tipLabel->setFixedHeight(50);
    tipLabel->setAlignment(Qt::AlignCenter);
    QFont fontTip = DFontSizeManager::instance()->get(DFontSizeManager::T9);
    tipLabel->setFont(fontTip);
    tipLabel->setPalette(palette1);

    DLabel *fileName = new DLabel(tr("Name:"), this);
    fileName->setFont(fontTip);
    fileName->setPalette(palette3);
    fileName->setFixedHeight(36);
    m_fileNameEdit = new DLineEdit(this);
    m_fileNameEdit->setAccessibleName("partName");
    QRegExp re("^[\u4E00-\u9FA5A-Za-z0-9_]+$");
    QRegExpValidator *validator = new QRegExpValidator(re, this);
    m_fileNameEdit->lineEdit()->setValidator(validator );
    m_fileNameEdit->setFixedHeight(36);
    if (m_fileNameEdit->text().isEmpty())
        m_fileNameEdit->lineEdit()->setPlaceholderText(tr("Name"));

    DLabel *formatName = new DLabel(tr("File system:"), this);
    formatName->setFont(fontTip);
    formatName->setPalette(palette3);
    formatName->setFixedHeight(36);
    m_formatComboBox = new DComboBox(this);
    m_formatComboBox->setAccessibleName("File system");
    m_formatComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QStringList fslist = DMDbusHandler::instance()->getAllSupportFileSystem();
    fslist.removeOne("linux-swap");
//    QStringList formateList = fslist;
    if (size > 100 && !isSystemDevice && DMDbusHandler::instance()->getCurLevel() != DMDbusHandler::LOGICALVOLUME) {
        fslist = DMDbusHandler::instance()->getEncryptionFormate(fslist);
    }

    m_formatComboBox->addItems(fslist);
    m_formatComboBox->setFixedHeight(36);

    QString fsTypeName = Utils::fileSystemTypeToString(static_cast<FSType>(fileSystemType));
    // 判断当前设备是否加密
    if (luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
        LUKS_INFO luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(m_pathInfo);
        // 加密设备是否打开，打开格式设为加密格式，未打开默认设为ext4
        if (luksInfo.isDecrypt) {
            fileSystemType = luksInfo.m_mapper.m_luksFs;
            if (-1 == fslist.indexOf(Utils::fileSystemTypeToString(static_cast<FSType>(fileSystemType)))) {
                fsTypeName = Utils::fileSystemTypeToString(FSType::FS_EXT4);
            } else {
                if (luksInfo.m_crypt == CRYPT_CIPHER::AES_XTS_PLAIN64) {
                    fsTypeName = QString("%1 (%2)").arg(Utils::fileSystemTypeToString(static_cast<FSType>(fileSystemType)))
                            .arg(tr("AES Encryption"));
                } else if (luksInfo.m_crypt == CRYPT_CIPHER::SM4_XTS_PLAIN64){
                    fsTypeName = QString("%1 (%2)").arg(Utils::fileSystemTypeToString(static_cast<FSType>(fileSystemType)))
                            .arg(tr("SM4 Encryption"));
                }
            }
        } else {
            fsTypeName = Utils::fileSystemTypeToString(FSType::FS_EXT4);
        }
    }

    if (-1 == fslist.indexOf(fsTypeName)) {
        m_formatComboBox->setCurrentText("ext4");
    } else {
        m_formatComboBox->setCurrentText(fsTypeName);
    }

    DLabel *securityLabel = new DLabel(tr("Security:"), this);
    securityLabel->setFont(fontTip);
    securityLabel->setPalette(palette3);
    securityLabel->setFixedHeight(36);
    m_labelTmp = new DLabel(this);
    m_label = new DLabel(this);
    m_label->setFixedHeight(36);
    m_securityComboBox = new DComboBox(this);
    m_securityComboBox->setAccessibleName("Security");
    m_securityComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    if ("SSD" == m_curDiskMediaType || DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        QStringList securitylist;
        securitylist << tr("Fast") << tr("Secure");
        m_securityComboBox->addItems(securitylist);
    } else {
        QStringList securitylist;
        securitylist << tr("Fast") << tr("Secure") << tr("Advanced");
        m_securityComboBox->addItems(securitylist);
    }
    m_securityComboBox->setFixedHeight(34);

    m_describeInfo = new DLabel(this);
    QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T10);
    m_describeInfo->setFont(font);
    m_describeInfo->setPalette(palette2);
    m_describeInfo->adjustSize();
    m_describeInfo->setWordWrap(true);
    m_describeInfo->setText(tr("It only deletes the partition info without erasing the files on the disk. "
                               "Disk recovery tools may recover the files at a certain probability."));
    m_describeInfo->setFixedHeight(36);
    m_describeInfo->setAccessibleName("describeInfo");
    m_labelTmp->setFixedHeight(35);

    DLabel *wipingLabel = new DLabel(tr("Wiping method:"), this);
    wipingLabel->setFont(fontTip);
    wipingLabel->setPalette(palette3);

    m_wipingMethodComboBox = new DComboBox(this);
    m_wipingMethodComboBox->setAccessibleName("Wiping method");
    m_wipingMethodComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QStringList wipingMethodlist;
    wipingMethodlist << tr("DoD 5220.22-M, 7 passes") << tr("Gutmann, 35 passes");
    m_wipingMethodComboBox->addItems(wipingMethodlist);
    m_wipingMethodComboBox->setFixedHeight(34);

    QHBoxLayout *wipingLayout = new QHBoxLayout;
    wipingLayout->addWidget(wipingLabel);
    wipingLayout->addSpacing(10);
    wipingLayout->addWidget(m_wipingMethodComboBox);
    wipingLayout->setSpacing(0);
    wipingLayout->setContentsMargins(0, 5, 0, 0);

    m_wipingMethodWidget = new QWidget(this);
    m_wipingMethodWidget->setLayout(wipingLayout);
    m_label->hide();
    m_wipingMethodWidget->hide();

    m_encryptionInfo = new DLabel(this);
    m_encryptionInfo->setFont(font);
    m_encryptionInfo->setPalette(palette2);
    m_encryptionInfo->adjustSize();
    m_encryptionInfo->setWordWrap(true);
    m_encryptionInfo->setAccessibleName("encryptionInfo");
    m_encryptionInfo->hide();
    m_emptyLabel = new DLabel(this);
    m_emptyLabel->hide();
    m_encryptionInfoHeight = 0;

    QVBoxLayout *layoutName = new QVBoxLayout;
    layoutName->addWidget(fileName);
    layoutName->addSpacing(10);
    layoutName->addWidget(formatName);
    layoutName->addSpacing(5);
    layoutName->addWidget(m_emptyLabel);
    layoutName->addSpacing(5);
    layoutName->addWidget(securityLabel);
    layoutName->addSpacing(5);
    layoutName->addWidget(m_labelTmp);
    layoutName->addSpacing(5);
    layoutName->addWidget(m_label);
    layoutName->setSpacing(0);
    layoutName->setContentsMargins(10, 0, 0, 0);

    QVBoxLayout *layoutFormat = new QVBoxLayout;
    layoutFormat->addWidget(m_fileNameEdit);
    layoutFormat->addSpacing(10);
    layoutFormat->addWidget(m_formatComboBox);
    layoutFormat->addSpacing(5);
    layoutFormat->addWidget(m_encryptionInfo);
    layoutFormat->addSpacing(5);
    layoutFormat->addWidget(m_securityComboBox);
    layoutFormat->addSpacing(5);
    layoutFormat->addWidget(m_describeInfo);
    layoutFormat->addWidget(m_wipingMethodWidget);
    layoutFormat->setSpacing(0);
    layoutFormat->setContentsMargins(0, 0, 10, 0);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(layoutName);
    layout->addLayout(layoutFormat);
    layout->setContentsMargins(0, 0, 0, 0);

    m_pushButton = new DPushButton(tr("Cancel", "button"), this);
    m_pushButton->setAccessibleName("cancel");
    m_pushButton->setFixedHeight(36);
    m_warningButton = new DWarningButton(this);
    m_warningButton->setText(tr("Wipe", "button"));
    m_warningButton->setAccessibleName("wipeButton");
    m_warningButton->setFixedHeight(36);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(0);
    m_buttonLayout->addWidget(m_pushButton);
    m_buttonLayout->addSpacing(10);
    m_buttonLayout->addWidget(m_warningButton);
    m_buttonLayout->setSpacing(0);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *tipLayout = new QHBoxLayout;
    tipLayout->addSpacing(50);
    tipLayout->addWidget(tipLabel);
    tipLayout->addSpacing(50);
    tipLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *wipeLayout = new QVBoxLayout;
    wipeLayout->addLayout(tipLayout);
    wipeLayout->addLayout(layout);
    wipeLayout->addLayout(m_buttonLayout);
    wipeLayout->setContentsMargins(0, 0, 0, 0);
    QWidget *wipeWidget = new QWidget(this);
    wipeWidget->setLayout(wipeLayout);

    QWidget *spinnerWidget = new QWidget(this);
    m_waterLoadingWidget = new WaterLoadingWidget(this);
    m_waterLoadingWidget->setFixedSize(128, 128);

    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    spinnerLayout->addSpacing(50);
    spinnerLayout->addWidget(m_waterLoadingWidget, 0, Qt::AlignCenter);
    spinnerLayout->addStretch();
    spinnerWidget->setLayout(spinnerLayout);

    DLabel *failPicture = new DLabel(this);
    QIcon icon = QIcon::fromTheme("://icons/deepin/builtin/fail.svg");
    failPicture->setPixmap(icon.pixmap(128, 128));

    m_failLabel = new DLabel(this);
    m_failLabel->setObjectName("failreason");
    QFont failFont/* = DFontSizeManager::instance()->get(DFontSizeManager::T8)*/;
    failFont.setWeight(QFont::Normal);
    failFont.setFamily("Source Han Sans");
    failFont.setPixelSize(12);
    m_failLabel->setFont(failFont);
    m_failLabel->setPalette(palette3);
    m_failLabel->setText(tr("Failed to find the disk"));

    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        tipLabel->setText(tr("The action cannot be undone, please proceed with caution"));
        tipLabel->setFixedHeight(30);
        fileName->setText(tr("LV name:"));
        m_fileNameEdit->setText(DMDbusHandler::instance()->getCurLVInfo().m_lvName);
        m_fileNameEdit->lineEdit()->setReadOnly(true);
        m_fileNameEdit->setAccessibleName("lvName");
        m_fileNameEdit->lineEdit()->setPlaceholderText(tr("LV name"));
        m_fileNameEdit->setClearButtonEnabled(false);
        formatName->setText(tr("LV file system:"));
        m_formatComboBox->setAccessibleName("LV file system");
        m_describeInfo->setText(tr("You may be able to recover files after the wipe."));
        m_describeInfo->setFixedHeight(10);
        m_labelTmp->setFixedHeight(9);
        m_failLabel->setText(tr("Failed to submit the request to the kernel"));
    }

    QVBoxLayout *failLayout = new QVBoxLayout;
    failLayout->addWidget(failPicture, 0, Qt::AlignCenter);
    failLayout->addSpacing(10);
    failLayout->addWidget(m_failLabel, 0, Qt::AlignCenter);
    failLayout->addStretch();
    failLayout->setSpacing(0);
    failLayout->setContentsMargins(0, 20, 0, 0);

    QWidget *failWidget = new QWidget(this);
    failWidget->setLayout(failLayout);

    m_stackedWidget = new DStackedWidget(this);
    m_stackedWidget->addWidget(wipeWidget);
    m_stackedWidget->addWidget(spinnerWidget);
    m_stackedWidget->addWidget(failWidget);

    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    if (m_formatComboBox->currentText().contains("AES") || m_formatComboBox->currentText().contains("SM4")) {
        QFontMetrics fm(formatName->font());
        updateEncryptionInfo(m_formatComboBox->currentText(), width() - fm.boundingRect(formatName->text()).width() - 40);
    }
}

void FormateDialog::initConnection()
{
    connect(m_pushButton, &DPushButton::clicked, this, &FormateDialog::onCancelButtonClicked);
    connect(m_warningButton, &DWarningButton::clicked, this, &FormateDialog::onWipeButtonClicked);
    connect(m_fileNameEdit, &DLineEdit::textChanged, this, &FormateDialog::onTextChanged);
    connect(m_formatComboBox, &DComboBox::currentTextChanged, this, &FormateDialog::onComboxFormatTextChange);
    connect(m_securityComboBox, static_cast<void (DComboBox:: *)(const int)>(&DComboBox::currentIndexChanged),
            this, &FormateDialog::onSecurityCurrentIndexChanged);
    connect(m_wipingMethodComboBox, static_cast<void (DComboBox:: *)(const int)>(&DComboBox::currentIndexChanged),
            this, &FormateDialog::onWipingMethodCurrentIndexChanged);
    connect(DMDbusHandler::instance(), &DMDbusHandler::wipeMessage, this, &FormateDialog::onWipeResult);
}

void FormateDialog::onTextChanged(const QString &text)
{
    if (!text.isEmpty()) {
        QByteArray byteArray = text.toUtf8();
        if (m_formatComboBox->currentText().contains("fat32")) {
            if (byteArray.size() > 11) {
                m_fileNameEdit->setAlert(true);
                m_fileNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);

                m_warningButton->setEnabled(false);
            } else {
                m_fileNameEdit->setAlert(false);
                m_fileNameEdit->hideAlertMessage();

                m_warningButton->setEnabled(true);
            }
        } else {
            if (byteArray.size() > 16) {
                m_fileNameEdit->setAlert(true);
                m_fileNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);

                m_warningButton->setEnabled(false);
            } else {
                m_fileNameEdit->setAlert(false);
                m_fileNameEdit->hideAlertMessage();

                m_warningButton->setEnabled(true);
            }
        }
    }
}

void FormateDialog::onComboxFormatTextChange(const QString &text)
{
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION ||
            DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        QByteArray byteArray = m_fileNameEdit->text().toUtf8();
        if (text.contains("fat32")) {
            if (byteArray.size() > 11) {
                m_fileNameEdit->setAlert(true);
                m_fileNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);

                m_warningButton->setEnabled(false);
            } else {
                m_fileNameEdit->setAlert(false);
                m_fileNameEdit->hideAlertMessage();

                m_warningButton->setEnabled(true);
            }
        } else {
            if (byteArray.size() > 16) {
                m_fileNameEdit->setAlert(true);
                m_fileNameEdit->showAlertMessage(tr("The length exceeds the limit"), -1);

                m_warningButton->setEnabled(false);
            } else {
                m_fileNameEdit->setAlert(false);
                m_fileNameEdit->hideAlertMessage();

                m_warningButton->setEnabled(true);
            }
        }
    }

    updateEncryptionInfo(text, m_formatComboBox->width());
}

void FormateDialog::updateEncryptionInfo(const QString &text, const int &width)
{
    if (text.contains("AES")) {
        QString text = tr("Use the aes-xts-plain64 standard algorithm to encrypt the disk. "
                          "You should decrypt it before mounting it again.");
        m_encryptionInfoHeight = Common::getLabelAdjustHeight(width, text, m_encryptionInfo->font());
        m_encryptionInfo->setText(text);
        setFixedSize(450, m_height + m_encryptionInfoHeight);
        m_encryptionInfo->setFixedHeight(m_encryptionInfoHeight);
        m_emptyLabel->setFixedHeight(m_encryptionInfoHeight - 1);
        m_encryptionInfo->show();
        m_emptyLabel->show();
    } else if (text.contains("SM4")) {
        QString text = tr("Use the sm4-xts-plain state cryptographic algorithm to encrypt the disk. "
                          "You should decrypt it before mounting it again. Operating Systems that do not support the state "
                          "cryptographic algorithm will not be able to decrypt the disk.");
        m_encryptionInfoHeight = Common::getLabelAdjustHeight(width, text, m_encryptionInfo->font());
        m_encryptionInfo->setText(text);
        setFixedSize(450, m_height + m_encryptionInfoHeight);
        m_encryptionInfo->setFixedHeight(m_encryptionInfoHeight);
        m_emptyLabel->setFixedHeight(m_encryptionInfoHeight - 1);
        m_encryptionInfo->show();
        m_emptyLabel->show();
    } else {
        m_encryptionInfoHeight = 0;
        setFixedSize(450, m_height);
        m_encryptionInfo->hide();
        m_emptyLabel->hide();
        m_encryptionInfo->setText("");
    }
}

void FormateDialog::onSecurityCurrentIndexChanged(int index)
{
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION ||
            DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        switch (index) {
        case 0: {
            m_describeInfo->setText(tr("It only deletes the partition info without erasing the files on the disk. "
                                       "Disk recovery tools may recover the files at a certain probability."));
            int height = Common::getLabelAdjustHeight(m_securityComboBox->width(), m_describeInfo->text(), m_describeInfo->font());
            m_height = 325 + height;
            setFixedSize(450, m_height + m_encryptionInfoHeight);
            m_describeInfo->setFixedHeight(height);
            m_labelTmp->setFixedHeight(height - 1);
            m_buttonLayout->setContentsMargins(0, 0, 0, 0);
            m_label->hide();
            m_wipingMethodWidget->hide();
            m_curWipeMethod = WipeType::FAST;

            break;
        }
        case 1: {
            m_describeInfo->setText(tr("It is a one-time secure wipe that complies with NIST 800-88 and writes 0, 1, "
                                       "and random data to the entire disk once. You will not be able to recover files, "
                                       "and the process will be slow."));
            int height = Common::getLabelAdjustHeight(m_securityComboBox->width(), m_describeInfo->text(), m_describeInfo->font());
            m_height = 325 + height;
            setFixedSize(450, m_height + m_encryptionInfoHeight);
            m_describeInfo->setFixedHeight(height);
            m_labelTmp->setFixedHeight(height - 1);
            m_buttonLayout->setContentsMargins(0, 0, 0, 0);
            m_label->hide();
            m_wipingMethodWidget->hide();
            m_curWipeMethod = WipeType::SECURE;
            break;
        }
        case 2: {
            m_describeInfo->setText(tr("It writes 0, 1, and random data to the entire disk several times. You can set the "
                                       "number of times to erase disks and overwrite data, but the process will be very slow."));
            int height = Common::getLabelAdjustHeight(m_securityComboBox->width(), m_describeInfo->text(), m_describeInfo->font());
            m_height = 372 + height;
            setFixedSize(450, m_height + m_encryptionInfoHeight);
            m_describeInfo->setFixedHeight(height);
            m_labelTmp->setFixedHeight(height - 1);
            m_buttonLayout->setContentsMargins(0, 10, 0, 0);
            m_label->show();
            m_wipingMethodWidget->show();
            m_wipingMethodComboBox->setCurrentIndex(0);
            m_curWipeMethod = WipeType::DOD;
            break;
        }
        default:
            break;
        }
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        switch (index) {
        case 0: {
            m_describeInfo->setText(tr("You may be able to recover files after the wipe."));
            int height = Common::getLabelAdjustHeight(m_securityComboBox->width(), m_describeInfo->text(), m_describeInfo->font());
            m_height = 305 + height;
            setFixedSize(450, m_height + m_encryptionInfoHeight);
            m_describeInfo->setFixedHeight(height);
            m_labelTmp->setFixedHeight(height - 1);
            m_buttonLayout->setContentsMargins(0, 0, 0, 0);
            m_label->hide();
            m_wipingMethodWidget->hide();
            m_curWipeMethod = WipeType::FAST;
            break;
        }
        case 1: {
            m_describeInfo->setText(tr("You will not be able to recover files after the wipe, and the process will be slow."));
            int height = Common::getLabelAdjustHeight(m_securityComboBox->width(), m_describeInfo->text(), m_describeInfo->font());
            m_height = 305 + height;
            setFixedSize(450, m_height + m_encryptionInfoHeight);
            m_describeInfo->setFixedHeight(height);
            m_labelTmp->setFixedHeight(height - 1);
            m_buttonLayout->setContentsMargins(0, 0, 0, 0);
            m_label->hide();
            m_wipingMethodWidget->hide();
            m_curWipeMethod = WipeType::SECURE;
            break;
        }
        default:
            break;
        }
    }
}

void FormateDialog::onWipingMethodCurrentIndexChanged(int index)
{
    switch (index) {
    case 0: {
        m_curWipeMethod = WipeType::DOD;
        break;
    }
    case 1: {
        m_curWipeMethod = WipeType::GUTMANN;
        break;
    }
    default:
        break;
    }
}

void FormateDialog::onCancelButtonClicked()
{
    close();
}

void FormateDialog::onWipeButtonClicked()
{
    QString userName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    userName.remove(0, 6);

    QString formate = m_formatComboBox->currentText();
    QString password = "";
    QString passwordHint = "";
    QString encryptName = "";
    CRYPT_CIPHER encryption = CRYPT_CIPHER::NOT_CRYPT;
    bool isEncryption = false;
    if (formate.contains("AES") || formate.contains("SM4")) {
        PasswordInputDialog passwordInputDialog(this);
        passwordInputDialog.setDeviceName(m_pathInfo);
        passwordInputDialog.setAccessibleName("passwordInputDialog");
        if (passwordInputDialog.exec() != DDialog::Accepted) {
            return;
        } else {
            password = passwordInputDialog.getPassword();
            passwordHint = passwordInputDialog.getPasswordHint();
            if (formate.contains("AES")) {
                encryption = CRYPT_CIPHER::AES_XTS_PLAIN64;
                encryptName = "aesE";
            } else if (formate.contains("SM4")) {
                encryption = CRYPT_CIPHER::SM4_XTS_PLAIN64;
                encryptName = "sm4E";
            }
            isEncryption = true;

            MessageBox warningBox(this);
            warningBox.setObjectName("messageBox");
            warningBox.setAccessibleName("messageBox");
            // 为防止遗忘密码，请您自行备份密码，并妥善保存！  确 定
            warningBox.setWarings(tr("To avoid losing the password, please back up your password and keep it properly!"),
                                  tr("OK", "button"), DDialog::ButtonRecommend, "OK");
            warningBox.setGeometry(pos().x() + (width() - warningBox.width()) / 2, pos().y() + (height() - warningBox.height()) / 2,
                                   warningBox.width(), warningBox.height());
            warningBox.exec();
        }

        formate = formate.trimmed().split(" ").at(0);
    }

    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION ||
            DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        WipeAction wipe;
        wipe.m_fstype = formate;
        wipe.m_path = m_pathInfo;
        wipe.m_user = userName;
        wipe.m_diskType = DMDbusHandler::instance()->getCurLevel();
        wipe.m_clearType = m_curWipeMethod;
        wipe.m_crypt = encryption;
        wipe.m_decryptStr = password;
        if (isEncryption) {
            wipe.m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            QStringList tokenList;
            if (!passwordHint.isEmpty()) {
                tokenList.append(passwordHint);
            }

            wipe.m_tokenList = tokenList;
            if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
                wipe.m_dmName = "";
            } else {
                m_pathInfo == "unallocated" ? wipe.m_dmName = "" : wipe.m_dmName = QString("%1_%2").arg(m_pathInfo.remove("/dev/")).arg(encryptName);
            }
        } else {
            wipe.m_luksFlag = LUKSFlag::NOT_CRYPT_LUKS;
            wipe.m_tokenList = QStringList();
            wipe.m_dmName = "";
        }

        if (m_fileNameEdit->text().isEmpty()) {
            wipe.m_fileSystemLabel  = " ";
            DMDbusHandler::instance()->clear(wipe);
        } else {
            wipe.m_fileSystemLabel  = m_fileNameEdit->text();
            DMDbusHandler::instance()->clear(wipe);
        }
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();

        LVAction lvAction;
        lvAction.m_vgName = lvInfo.m_vgName;
        lvAction.m_lvName = lvInfo.m_lvName;
        lvAction.m_lvFs = Utils::stringToFileSystemType(formate);
        lvAction.m_user = userName;
        lvAction.m_lvSize = lvInfo.m_lvSize;
        lvAction.m_lvByteSize = lvInfo.m_lvLECount * lvInfo.m_LESize;
        lvAction.m_lvAct = (m_curWipeMethod == WipeType::FAST) ? LVMAction::LVM_ACT_LV_FAST_CLEAR : LVMAction::LVM_ACT_LV_SECURE_CLEAR;
        lvAction.m_crypt = encryption;
        lvAction.m_decryptStr = password;
        if (isEncryption) {
            lvAction.m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            QStringList tokenList;
            if (!passwordHint.isEmpty()) {
                tokenList.append(passwordHint);
            }

            lvAction.m_tokenList = tokenList;
            lvAction.m_dmName = QString("%1_%2_%3").arg(lvInfo.m_vgName).arg(lvInfo.m_lvName).arg(encryptName);
        } else {
            lvAction.m_luksFlag = LUKSFlag::NOT_CRYPT_LUKS;
            lvAction.m_tokenList = QStringList();
            lvAction.m_dmName = "";
        }

        DMDbusHandler::instance()->onClearLV(lvAction);
    }

    // 擦除等待动画
    setTitle(tr("Wiping %1").arg(m_pathInfo) + "...");
    m_stackedWidget->setCurrentIndex(1);
    DWindowCloseButton *button = findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (button != nullptr) {
        button->setDisabled(true);
        button->hide();
    }

    m_waterLoadingWidget->setStartTime(1000);
}

void FormateDialog::onWipeResult(const QString &info)
{
    QStringList infoList = info.split(":");

    if (infoList.count() <= 2) {
        return;
    }

    m_waterLoadingWidget->stopTimer();
    bool isSuccess = false;
    if (infoList.at(0) == "DISK_ERROR" && infoList.at(1).toInt() == DISK_ERROR::DISK_ERR_NORMAL) {
        isSuccess = true;
    }

    if (isSuccess) {
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                 QIcon::fromTheme("://icons/deepin/builtin/ok.svg"),
                                                 tr("\"%1\" wiped").arg(m_pathInfo));
        DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                       QMargins(0, 0, 0, 20));

        close();
    } else {
        QString failedReason = DMDbusHandler::instance()->getFailedMessage(infoList.at(0), infoList.at(1).toInt(), "");
        setTitle(tr("Failed to wipe %1").arg(m_pathInfo));
        m_stackedWidget->setCurrentIndex(2);

        DWindowCloseButton *button = findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        button->setDisabled(false);
        button->show();

        m_failLabel->setText(failedReason);
    }
}

