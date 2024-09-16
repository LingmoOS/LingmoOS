// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "decryptdialog.h"
#include "common.h"

#include <DWindowCloseButton>
#include <DMessageManager>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>

DecryptDialog::DecryptDialog(QWidget *parent)
    : DDBase(parent)
{
    initUi();
    initConnection();
    initData();
}

void DecryptDialog::initUi()
{
    m_height = 240;
    setFixedSize(406, m_height);

    DPalette palette1;
    QColor color1("#000000");
    color1.setAlphaF(0.9);
    palette1.setColor(DPalette::ToolTipText, color1);

    QFont font1;
    font1.setWeight(QFont::Medium);
    font1.setFamily("Source Han Sans");
    font1.setPixelSize(14);

    m_titleText = tr("Enter the password to decrypt the disk");
    if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::PARTITION) {
        PartitionInfo info = DMDbusHandler::instance()->getCurPartititonInfo();
        m_devPath = info.m_path;
        m_devName = info.m_path.remove("/dev/");
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::DISK) {
        DeviceInfo info = DMDbusHandler::instance()->getCurDeviceInfo();
        m_devPath = info.m_path;
        m_devName = info.m_path.remove("/dev/");
    } else if (DMDbusHandler::instance()->getCurLevel() == DMDbusHandler::LOGICALVOLUME) {
        LVInfo lvInfo = DMDbusHandler::instance()->getCurLVInfo();
        m_devPath = lvInfo.m_lvPath;
        m_titleText = tr("Enter the password to decrypt the volume group");
        m_devName = QString("%1_%2").arg(lvInfo.m_vgName).arg(lvInfo.m_lvName);
    }

    m_luksInfo = DMDbusHandler::instance()->probLUKSInfo().m_luksMap.value(m_devPath);

    setTitle(m_titleText);

    // 限制中文和中文字符
    QRegExp regExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\/ ]+$");
    QRegExpValidator *regExpValidator = new QRegExpValidator(regExp, this);

    m_passwordEdit = new DPasswordEdit(this);
    m_passwordEdit->setAccessibleName("passwordInput");
    m_passwordEdit->lineEdit()->setValidator(regExpValidator);
    m_passwordEdit->lineEdit()->setPlaceholderText(tr("Enter a password "));
    m_passwordEdit->setFixedHeight(37);

    m_pushButton = new DPushButton(this);
    m_pushButton->setAccessibleName("passwordHintButton");
    m_pushButton->setToolTip(tr("Password hint"));
    m_pushButton->setFixedSize(QSize(36, 36));
    m_pushButton->setIcon(Common::getIcon("passwordhint"));
    m_pushButton->setIconSize(QSize(18, 18));

    m_passwordHint = new DLabel(this);
    m_passwordHint->setAccessibleName("passwordHint");
    m_passwordHint->setFont(font1);
    m_passwordHint->setPalette(palette1);
    m_passwordHint->adjustSize();
    m_passwordHint->setWordWrap(true);

    QHBoxLayout *hintButtonLayout = new QHBoxLayout;
    hintButtonLayout->addWidget(m_pushButton);
    hintButtonLayout->setSpacing(0);
    hintButtonLayout->setContentsMargins(10, 0, 0, 0);
    QWidget *hintButtonWdiget = new QWidget(this);
    hintButtonWdiget->setLayout(hintButtonLayout);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->addWidget(m_passwordEdit);
    passwordLayout->addWidget(hintButtonWdiget);
    passwordLayout->setSpacing(0);
    passwordLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *passwordHintLayout = new QHBoxLayout;
    passwordHintLayout->addWidget(m_passwordHint);
    passwordHintLayout->setContentsMargins(10, 5, 10, 5);

    m_frame = new DFrame(this);
    m_frame->setBackgroundRole(DPalette::ItemBackground);
    m_frame->setLineWidth(0);
    m_frame->setLayout(passwordHintLayout);
    m_frame->hide();

    if (m_luksInfo.m_tokenList.isEmpty()) {
        hintButtonWdiget->hide();
    }

    QVBoxLayout *contentLayout = new QVBoxLayout;
    //contentLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    contentLayout->addSpacing(20);
    contentLayout->addLayout(passwordLayout);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_frame);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(10, 0, 10, 0);

    m_cancelButton = new DPushButton(tr("Cancel", "button"), this);
    m_cancelButton->setAccessibleName("cancel");
    m_cancelButton->setFont(font1);
    m_cancelButton->setFixedSize(183, 36);

    m_decryptButton = new DSuggestButton(tr("Decrypt", "button"), this);
    m_decryptButton->setAccessibleName("decrypt");
    m_decryptButton->setFont(font1);
    m_decryptButton->setFixedSize(183, 36);

    DVerticalLine *line = new DVerticalLine;
    line->setObjectName("VLine");
    line->setFixedHeight(28);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addSpacing(9);
    buttonLayout->addWidget(line, 0, Qt::AlignCenter);
    buttonLayout->addSpacing(9);
    buttonLayout->addWidget(m_decryptButton);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *firstLayout = new QVBoxLayout;
    firstLayout->addLayout(contentLayout);
    firstLayout->addStretch();
    firstLayout->addLayout(buttonLayout);
    firstLayout->setSpacing(0);
    firstLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *firstWidget = new QWidget(this);
    firstWidget->setLayout(firstLayout);

    m_waterLoadingWidget = new WaterLoadingWidget(this);
    m_waterLoadingWidget->setFixedSize(128, 128);

    QVBoxLayout *secondLayout = new QVBoxLayout;
    secondLayout->addSpacing(4);
    secondLayout->addWidget(m_waterLoadingWidget, 0, Qt::AlignCenter);
    secondLayout->addStretch();
    secondLayout->setSpacing(0);
    secondLayout->setContentsMargins(0, 0, 0, 10);

    QWidget *secondWidget = new QWidget(this);
    secondWidget->setLayout(secondLayout);

    m_stackedWidget = new DStackedWidget(this);
    m_stackedWidget->addWidget(firstWidget);
    m_stackedWidget->addWidget(secondWidget);
    m_stackedWidget->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainFrame);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
}

void DecryptDialog::initConnection()
{
    connect(m_passwordEdit, &DPasswordEdit::textChanged, this, &DecryptDialog::onPasswordEditTextChanged);
    connect(m_pushButton, &DPushButton::clicked, this, &DecryptDialog::onPasswordHintButtonClicked);
    connect(m_decryptButton, &DSuggestButton::clicked, this, &DecryptDialog::onButtonClicked);
    connect(m_cancelButton, &DPushButton::clicked, this, &DecryptDialog::onCancelButtonClicked);
    connect(&m_timer, &QTimer::timeout, this, &DecryptDialog::onTimeOut);
    connect(DMDbusHandler::instance(), &DMDbusHandler::deCryptMessage, this, &DecryptDialog::onDecryptMessage);
}

void DecryptDialog::initData()
{
    if (m_luksInfo.m_decryptErrCount > 0) {
        if (m_luksInfo.m_decryptErrCount >= 5) {
            QString text = tr("Please try again %1 minutes later");
            QDateTime lastErrorTime = QDateTime::fromString(m_luksInfo.m_decryptErrorLastTime, "yyyy-MM-dd hh:mm:ss");
            QDateTime curDateTime = QDateTime::fromString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"), "yyyy-MM-dd hh:mm:ss");
            qint64 sec = lastErrorTime.secsTo(curDateTime);
            int minutes = static_cast<int>(sec / 60);

            if (m_luksInfo.m_decryptErrCount == 5) {
                updateData(3, minutes, text);
            } else if (m_luksInfo.m_decryptErrCount == 6) {
                updateData(5, minutes, text);
            } else if (m_luksInfo.m_decryptErrCount == 7) {
                updateData(15, minutes, text);
            } else if (m_luksInfo.m_decryptErrCount >= 8) {
                updateData(60, minutes, text);
            }
        }
    }

    m_passwordEdit->setFocus();
}

void DecryptDialog::setPasswordEditStatus(const bool &isDisabled, const bool &isAlert, const QString &message)
{
    m_passwordEdit->setDisabled(isDisabled);
    m_passwordEdit->setAlert(isAlert);

    if (isAlert) {
        m_passwordEdit->clear();
        m_passwordEdit->showAlertMessage(message);
    } else {
        m_passwordEdit->hideAlertMessage();
    }
}

void DecryptDialog::updateData(const int &baseMinute, const int &minutes, const QString &message)
{
    if (minutes < baseMinute) {
        setPasswordEditStatus(true, true, message.arg(baseMinute - minutes));
        m_timer.start((baseMinute - minutes) * 60 * 1000);
        m_decryptButton->setDisabled(true);
    } else {
        setPasswordEditStatus(false, false);
        m_decryptButton->setDisabled(false);
    }
}

void DecryptDialog::onPasswordEditTextChanged(const QString &)
{
    if (m_passwordEdit->isAlert()) {
        m_passwordEdit->setAlert(false);
        m_passwordEdit->hideAlertMessage();
    }
}

void DecryptDialog::onPasswordHintButtonClicked()
{
    if (!m_frame->isHidden()) {
        m_height = 200;
        setFixedSize(406, m_height);
        m_frame->hide();
        return;
    }

    m_passwordHint->setText(m_luksInfo.m_tokenList.join("\n"));
    int heightLabel = Common::getLabelAdjustHeight(width() - 60, m_passwordHint->text(), m_passwordHint->font());
    m_passwordHint->setFixedHeight(heightLabel);
    m_frame->show();
    m_height = 215 + heightLabel;
    setFixedSize(406, m_height);
}

void DecryptDialog::onCancelButtonClicked()
{
    reject();
}

void DecryptDialog::onButtonClicked()
{
    // 密码不能为空
    if (m_passwordEdit->text().isEmpty()) {
        m_passwordEdit->setAlert(true);
        m_passwordEdit->showAlertMessage(tr("The password cannot be empty"));
        m_passwordEdit->setFocus();
        return;
    }

    LUKS_INFO luksInfo = m_luksInfo;
    luksInfo.m_decryptStr = m_passwordEdit->text();
    luksInfo.m_devicePath = m_devPath;
    if (m_luksInfo.m_crypt == CRYPT_CIPHER::AES_XTS_PLAIN64) {
        luksInfo.m_mapper.m_dmName = QString("%1_aesE").arg(m_devName);
    } else if (m_luksInfo.m_crypt == CRYPT_CIPHER::SM4_XTS_PLAIN64) {
        luksInfo.m_mapper.m_dmName = QString("%1_sm4E").arg(m_devName);
    } else {
        luksInfo.m_mapper.m_dmName = "";
    }

    DMDbusHandler::instance()->deCrypt(luksInfo);

    setFixedSize(406, 226);
    m_stackedWidget->setCurrentIndex(1);
    setTitle(tr("Decrypting..."));
    DWindowCloseButton *button = findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
    if (button != nullptr) {
        button->setDisabled(true);
        button->hide();
    }
    m_waterLoadingWidget->setStartTime(1000);
}

void DecryptDialog::onDecryptMessage(const LUKS_INFO &luks)
{
    DMDbusHandler::instance()->updateLUKSInfo(m_devPath, luks);
    m_luksInfo = luks;

    if (luks.isDecrypt && luks.m_decryptErrCount == 0) {
        DMDbusHandler::instance()->refreshMainWindowData();
        if (luks.m_cryptErr == CRYPTError::CRYPT_ERR_DECRYPT_FAILED) {
            DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                     QIcon::fromTheme("://icons/deepin/builtin/warning.svg"), tr("Decryption failed"));
            DMessageManager::instance()->setContentMargens(this->parentWidget()->parentWidget()->parentWidget()->parentWidget(),
                                                           QMargins(0, 0, 0, 20));
            reject();
        } else {
            accept();
        }
    } else {
        setFixedSize(406, m_height);
        m_stackedWidget->setCurrentIndex(0);
        DWindowCloseButton *button = findChild<DWindowCloseButton *>("DTitlebarDWindowCloseButton");
        if (button != nullptr) {
            button->setDisabled(false);
            button->show();
        }

        QString text = tr("Password locked, please try again %1 minutes later");
        if (luks.m_decryptErrCount < 5 && luks.m_decryptErrCount > 0) {
            text = tr("Wrong password, %1 chances left").arg(5 - luks.m_decryptErrCount);
            if ((5 - luks.m_decryptErrCount) == 1) {
                text = tr("Wrong password, only one chance left");
            }

            setPasswordEditStatus(false, true, text);
        } else if (luks.m_decryptErrCount == 5) {
            setPasswordEditStatus(true, true, text.arg(3));
            m_decryptButton->setDisabled(true);
            m_timer.start(3 * 60 * 1000);
        } else if (luks.m_decryptErrCount == 6) {
            setPasswordEditStatus(true, true, text.arg(5));
            m_decryptButton->setDisabled(true);
            m_timer.start(5 * 60 * 1000);
        } else if (luks.m_decryptErrCount == 7) {
            setPasswordEditStatus(true, true, text.arg(15));
            m_decryptButton->setDisabled(true);
            m_timer.start(15 * 60 * 1000);
        } else if (luks.m_decryptErrCount >= 8) {
            setPasswordEditStatus(true, true, text.arg(60));
            m_decryptButton->setDisabled(true);
            m_timer.start(60 * 60 * 1000);
        }

        m_passwordEdit->setFocus();
    }
}

void DecryptDialog::onTimeOut()
{
    m_timer.stop();
    setPasswordEditStatus(false, false);
    m_decryptButton->setDisabled(false);
    m_passwordEdit->setFocus();
}

void DecryptDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key::Key_Enter || event->key() == Qt::Key::Key_Return) {
        onButtonClicked();
    } else if (event->key() == Qt::Key::Key_Escape) {
        event->ignore();
    } else {
        DDialog::keyPressEvent(event);
    }
}
