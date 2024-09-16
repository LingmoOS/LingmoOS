// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AuthDialog.h"
#include "usersmanager.h"

#include <QProcess>
#include <QPainter>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QUrl>
#include <QAbstractButton>
#include <QButtonGroup>

#include <DHiDPIHelper>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Details>

#include <libintl.h>

DWIDGET_USE_NAMESPACE

AuthDialog::AuthDialog(const QString &message,
                       const QString &iconName)
    : DDialog(message, QString(), nullptr)
    , m_message(message)
    , m_iconName(iconName)
    , m_adminsCombo(new QComboBox(this))
    , m_passwordInput(new DPasswordEdit(this))
    , m_numTries(0)
    , m_lockLimitTryNum(getLockLimitTryNum())
    , m_authStatus(AuthStatus::None)
{
    initUI();

    setlocale(LC_ALL, "");

    qDebug() << "lock limit: " << m_lockLimitTryNum;

    // 始终显示用户名 (bug:9145,降低用户理解成本)
    connect(m_adminsCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AuthDialog::on_userCB_currentIndexChanged);
    connect(this, &AuthDialog::aboutToClose, this, &AuthDialog::rejected);
}

AuthDialog::~AuthDialog()
{
}

void AuthDialog::setError(const QString &error, bool alertImmediately)
{
    //由于无法获取到dgetText的临时方案
    QString dgetText = "";
    if ("Finger moved too fast, please do not lift until prompted" == error) {
        dgetText = tr("Finger moved too fast, please do not lift until prompted");
    } else if ("Verification failed, 2 chances left" == error) {
        dgetText = tr("Verification failed, two chances left");
    } else {
        dgetText = QString(dgettext("deepin-authentication", error.toUtf8()));
    }
    m_errorMsg = dgetText;
    if (alertImmediately)
        m_passwordInput->showAlertMessage(dgetText);
}

void AuthDialog::setAuthInfo(const QString &info)
{
    if ("Password" == info)
        m_passwordInput->lineEdit()->setFocus();

    m_passwordInput->lineEdit()->setPlaceholderText(QString(dgettext("deepin-authentication", info.toStdString().c_str())));

    setButtonText(1, tr("Confirm", "button"));
    getButton(1)->setAccessibleName("Confirm");
    update();
}

void AuthDialog::addOptions(QButtonGroup *bg)
{
    QList<QAbstractButton *> btns = bg->buttons();

    if (btns.length() > 0) {
        addSpacing(10);
    }

    for (QAbstractButton *btn : btns) {
        addContent(btn);
    }
}

bool AuthDialog::hasSecurityHighLever(QString userName)
{
    bool re = false;
    QDBusInterface securityEnhance("com.deepin.daemon.SecurityEnhance",
                                   "/com/deepin/daemon/SecurityEnhance",
                                   "com.deepin.daemon.SecurityEnhance",
                                   QDBusConnection::systemBus());

    QDBusReply<QString> reply = securityEnhance.call("GetSEUserByName", userName);
    if(reply.isValid()){
        QString value = reply.value();
        re = (value == "sysadm_u");
    }

    return re;
}

bool AuthDialog::hasOpenSecurity()
{
    bool hasOpen = false;
    QDBusInterface securityEnhance("com.deepin.daemon.SecurityEnhance",
                            "/com/deepin/daemon/SecurityEnhance",
                            "com.deepin.daemon.SecurityEnhance",
                            QDBusConnection::systemBus());
     QDBusReply<QString> reply = securityEnhance.call("Status");
     if(reply.isValid()){
         QString value = reply.value();
         hasOpen = (value == "open");
     }

    return hasOpen;
}

void AuthDialog::createUserCB(const PolkitQt1::Identity::List &identities)
{
    // Clears the combobox in the case some user be added
    m_adminsCombo->clear();
    bool isOpen = hasOpenSecurity();

    // For each user
    foreach (const PolkitQt1::Identity &identity, identities) {
        if (!identity.isValid()) {
            continue;
        }

        // appends the user item
        QString username = identity.toString().remove("unix-user:");
        QString fullname = UsersManager::instance()->getFullName(username);

        QString displayName = fullname.isEmpty() ? username : fullname;
        if (passwordIsExpired(identity))
            displayName += QString("(%1)").arg(tr("Expired"));
        if (isOpen) {
            if (hasSecurityHighLever(username) && identities.count() > 1) {
                m_adminsCombo->clear();
                if (username == qgetenv("USER"))
                    m_adminsCombo->insertItem(0, displayName, identity.toString());
                else
                    m_adminsCombo->addItem(displayName, identity.toString());
                break;
            }
        }

        if (username == qgetenv("USER"))
            m_adminsCombo->insertItem(0, displayName, identity.toString());
        else
            m_adminsCombo->addItem(displayName, identity.toString());
    }
    if (m_adminsCombo->count() > 0) {
        m_adminsCombo->setCurrentIndex(0); // select the current user.
    } else {
        qWarning() << "ERROR, no valid user";
    }
    m_adminsCombo->show();
}

// 判断用户密码是否在有效期内
bool AuthDialog::passwordIsExpired(PolkitQt1::Identity identity)
{
    QDBusInterface accounts("org.deepin.dde.Accounts1", "/org/deepin/dde/Accounts1", "org.deepin.dde.Accounts1", QDBusConnection::systemBus());
    QDBusReply<QString> reply = accounts.call("FindUserById", QString::number(identity.toUnixUserIdentity().uid()));
    if (reply.isValid()) {
        const QString path = reply.value();
        if (!path.isEmpty()) {
            QDBusInterface accounts_user("org.deepin.dde.Accounts1", path, "org.deepin.dde.Accounts1.User", QDBusConnection::systemBus());
            QDBusReply<bool> expiredReply = accounts_user.call("IsPasswordExpired");
            if (expiredReply.isValid())
                return expiredReply.value();
        }
    }

    return false;
}

PolkitQt1::Identity AuthDialog::selectedAdminUser() const
{
    if (m_adminsCombo->currentIndex() == -1)
        return PolkitQt1::Identity();

    const QString &id = m_adminsCombo->currentData().toString();
    if (id.isEmpty())
        return PolkitQt1::Identity();

    return PolkitQt1::Identity::fromString(id);
}

int AuthDialog::getLockLimitTryNum()
{
    const QString path = "/usr/share/dde-session-shell/dde-session-shell.conf";
    int count = 5;
    QFile file(path);
    if (!file.exists()) {
        return count;
    }

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("LockTime");
    count = settings.value("lockLimitTryNum").toInt();
    settings.endGroup();
    return count;
}

void AuthDialog::on_userCB_currentIndexChanged(int /*index*/)
{
    PolkitQt1::Identity identity = selectedAdminUser();
    // 清除上一个用户已经输入的密码
    m_passwordInput->clear();
    m_passwordInput->setAlert(false);
    m_passwordInput->lineEdit()->setPlaceholderText("");
    m_errorMsg = "";
    m_numTries = 0;

    // itemData is Null when "Select user" is selected
    if (!identity.isValid()) {
        // 清理警告信息
        m_passwordInput->setEnabled(false);
    } else {
        // 如果密码已过期
        if (passwordIsExpired(identity)) {
            m_passwordInput->setEnabled(false);
            m_passwordInput->lineEdit()->setPlaceholderText(tr("Unavailable"));
            // 密码过期不会执行到验证失败的逻辑，需要立即弹出提醒
            setError(tr("The password of this user has expired. Please authenticate using another user account or change the password and try again."), true);
        } else {
            // 清理警告信息
            m_passwordInput->setEnabled(true);
            m_passwordInput->hideAlertMessage();
            // We need this to restart the auth with the new user
            emit adminUserSelected(identity);
            // git password label focus
            m_passwordInput->lineEdit()->setFocus();
        }
    }
}

QString AuthDialog::password() const
{
    return m_passwordInput->text();
}

void AuthDialog::lock()
{
    m_passwordInput->setEnabled(false);
    getButton(1)->setEnabled(false);
}

void AuthDialog::authenticationFailure(bool &isLock)
{
    m_numTries++;
    if (!isLock) {
        // 不存在DA的情况，由次数来判定是否锁定
        if (m_lockLimitTryNum <= m_numTries) {
            isLock = true;
        }
    }

    if (m_errorMsg.isEmpty()) {
        // 专业版错误信息现在由DA提供，考虑没有DA的版本，保留以前由agent提供错误的方案
        qDebug() << "authentication failed, error message is empty, set error message by agent.";
        if (isLock) {
            setError(tr("Locked, please try again later"));
        } else {
            setError(tr("Wrong password"));
        }
    }

    m_passwordInput->setEnabled(true);
    m_passwordInput->showAlertMessage(m_errorMsg);
    m_passwordInput->setAlert(true);
    m_passwordInput->clear();
    m_passwordInput->lineEdit()->setFocus();
    getButton(1)->setEnabled(true);

    if (isLock) {
        lock();
    }
    activateWindow();
}

bool AuthDialog::event(QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        activateWindow();
        m_passwordInput->setFocus();
    }
    return DDialog::event(event);
}

void AuthDialog::initUI()
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Tool);
    setWindowFlag(Qt::BypassWindowManagerHint, true);
    setMinimumWidth(380);
    setOnButtonClickedClose(false);

    // 设置图标
    QPixmap icon;
    const qreal dpr = devicePixelRatioF();
    if (!m_iconName.isEmpty() && QIcon::hasThemeIcon(m_iconName)) {
        icon = QIcon::fromTheme(m_iconName).pixmap(static_cast<int>(48 * dpr), static_cast<int>(48 * dpr));
    } else {
        icon = DHiDPIHelper::loadNxPixmap(":/images/default.svg");
    }
    icon.setDevicePixelRatio(dpr);
    setIcon(icon);

    // 禁用剪切、复制
    m_passwordInput->setCopyEnabled(false);
    m_passwordInput->setCutEnabled(false);

    int cancelId = addButton(tr("Cancel", "button"));
    int confirmId = addButton(tr("Confirm", "button"), true, ButtonType::ButtonRecommend);
    setDefaultButton(1);

    getButton(confirmId)->setEnabled(true);

    getButton(cancelId)->setAccessibleName("Cancel");
    getButton(confirmId)->setAccessibleName("Confirm");
    m_passwordInput->setAccessibleName("PasswordInput");
    m_adminsCombo->setAccessibleName("AdminUsers");

    m_adminsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_adminsCombo->hide();
    m_passwordInput->setEchoMode(QLineEdit::Password);

    addSpacing(10);
    addContent(m_adminsCombo);
    addSpacing(6);
    addContent(m_passwordInput);

    connect(this, &AuthDialog::buttonClicked, [this](int index, QString) {
        switch (index) {
        case 0:
            emit rejected();
            break;
        case 1: {
            emit accepted();
            break;
        }
        default:;
        }
    });

    connect(m_passwordInput, &DPasswordEdit::textChanged, [ = ](const QString & text) {
        getButton(confirmId)->setEnabled(Authenticating != m_authStatus && None != m_authStatus);
        if (text.length() == 0)
            return;

        m_passwordInput->setAlert(false);
        m_errorMsg = "";
    });
}

void AuthDialog::setInAuth(AuthStatus authStatus)
{
    m_authStatus = authStatus;
    getButton(1)->setEnabled(Authenticating != authStatus);
}
