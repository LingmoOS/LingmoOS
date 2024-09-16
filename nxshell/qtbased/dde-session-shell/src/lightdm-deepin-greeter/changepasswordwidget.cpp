// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "changepasswordwidget.h"
#include "dlineeditex.h"
#include "useravatar.h"
#include "pwqualitymanager.h"
#include "passwordlevelwidget.h"
#include "constants.h"

#include <QVBoxLayout>
#include <QProcess>

#include <DPasswordEdit>
#include <DLabel>
#include <DDesktopServices>
#include <DFloatingMessage>
#include <DMessageManager>
#include <DAnchors>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
using namespace DDESESSIONCC;

/**
 * @brief The ResetPasswdWidget class   重置密码界面
 * @note 管理员账户密码过期时，登录时会自动跳到此界面
 */
ChangePasswordWidget::ChangePasswordWidget(std::shared_ptr<User> user, QWidget *parent)
    : QWidget(parent)
    , m_user(user)
    , m_mainLayout(new QVBoxLayout(this))
    , m_lockBtn(new DLabel(this))
    , m_avatar(new UserAvatar(this))
    , m_nameLabel(new DLabel(m_user->displayName(), this))
    , m_tipsLabel(new DLabel(tr("Your password has expired. Please change it and log in again."), this))
    , m_oldPasswdEdit(new DPasswordEdit(this))
    , m_newPasswdEdit(new DPasswordEdit(this))
    , m_levelWidget(new PasswordLevelWidget(this))
    , m_repeatPasswdEdit(new DPasswordEdit(this))
    , m_passwordHints(new DLineEditEx(this))
    , m_okBtn(new DPushButton(tr("Save"), this))
{
    setAccessibleName("ResetPasswdWidget");

    initUI();
    initConnections();
}

void ChangePasswordWidget::initUI()
{
    m_lockBtn->setPixmap(QIcon::fromTheme(":/misc/images/login_lock.svg").pixmap(24, 24));
    m_lockBtn->setFixedSize(24, 24);
    m_avatar->setFocusPolicy(Qt::NoFocus);
    m_avatar->setIcon(m_user->avatar());
    m_avatar->setAvatarSize(UserAvatar::AvatarLargeSize);

    m_nameLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_nameLabel, DFontSizeManager::T2);

    m_tipsLabel->setAlignment(Qt::AlignCenter);

    m_oldPasswdEdit->setClearButtonEnabled(false);
    m_oldPasswdEdit->setEchoMode(QLineEdit::Password);
    m_oldPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_oldPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_oldPasswdEdit->setPlaceholderText(tr("Old password"));
    m_oldPasswdEdit->setFixedSize(PASSWDLINEEIDT_WIDTH, PASSWDLINEEDIT_HEIGHT);

    m_newPasswdEdit->setClearButtonEnabled(false);
    m_newPasswdEdit->setEchoMode(QLineEdit::Password);
    m_newPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_newPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_newPasswdEdit->setPlaceholderText(tr("New password"));
    m_newPasswdEdit->setFixedSize(PASSWDLINEEIDT_WIDTH, PASSWDLINEEDIT_HEIGHT);

    m_levelWidget->reset();

    m_repeatPasswdEdit->setClearButtonEnabled(false);
    m_repeatPasswdEdit->setEchoMode(QLineEdit::Password);
    m_repeatPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_repeatPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_repeatPasswdEdit->setPlaceholderText(tr("Repeat new password"));
    m_repeatPasswdEdit->setFixedSize(PASSWDLINEEIDT_WIDTH, PASSWDLINEEDIT_HEIGHT);

    m_passwordHints->setClearButtonEnabled(false);
    m_passwordHints->setEchoMode(QLineEdit::Normal);
    m_passwordHints->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordHints->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordHints->setPlaceholderText(tr("Password hint (Optional)"));
    m_passwordHints->setFixedSize(PASSWDLINEEIDT_WIDTH, PASSWDLINEEDIT_HEIGHT);

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_lockBtn, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_avatar, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_tipsLabel, 0, Qt::AlignCenter);
    auto lineEditWidget = new QWidget;
    auto lineEditLayout = new QVBoxLayout(lineEditWidget);
    lineEditLayout->setMargin(0);
    lineEditLayout->setSpacing(0);
    lineEditLayout->addWidget(m_oldPasswdEdit, 0, Qt::AlignCenter);
    m_levelWidget->setMinimumHeight(LINEEDIT_SPACING);
    lineEditLayout->addWidget(m_levelWidget, 0, Qt::AlignRight);
    lineEditLayout->addWidget(m_newPasswdEdit, 0, Qt::AlignCenter);
    lineEditLayout->addSpacing(LINEEDIT_SPACING);
    lineEditLayout->addWidget(m_repeatPasswdEdit, 0, Qt::AlignCenter);
    lineEditLayout->addSpacing(LINEEDIT_SPACING);
    lineEditLayout->addWidget(m_passwordHints, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(lineEditWidget, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_okBtn, 0, Qt::AlignCenter);
    m_okBtn->setFixedWidth(160);

    setLayout(m_mainLayout);

    // 默认焦点处于旧密码输入框中
    m_oldPasswdEdit->setFocus();
}

void ChangePasswordWidget::initConnections()
{
    connect(m_newPasswdEdit, &DLineEditEx::textChanged, this, &ChangePasswordWidget::onNewPasswordTextChanged);

    connect(m_repeatPasswdEdit, &DLineEditEx::textChanged, this, &ChangePasswordWidget::onRepeatPasswordTextEdited);
    connect(m_repeatPasswdEdit, &DLineEditEx::editingFinished, this, &ChangePasswordWidget::onRepeatPasswordEditFinished);

    connect(m_passwordHints, &DLineEdit::textEdited, this, &ChangePasswordWidget::onPasswordHintsChanged);

    connect(m_okBtn, &DPushButton::clicked, this, &ChangePasswordWidget::onOkClicked);
}

void ChangePasswordWidget::onNewPasswordTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        m_newPasswdEdit->hideAlertMessage();
        m_newPasswdEdit->setAlert(false);
        m_levelWidget->reset();
        return;
    }

    PASSWORD_LEVEL_TYPE level = PwqualityManager::instance()->newPassWdLevel(text);
    m_levelWidget->setLevel(level);

    PwqualityManager::ERROR_TYPE error = PwqualityManager::instance()->verifyPassword(m_user->displayName(), text);
    // TODO 这里的error返回异常
    if (error != PwqualityManager::ERROR_TYPE::PW_NO_ERR) {
        qDebug() << "password error type: " << error;
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(PwqualityManager::instance()->getErrorTips(error), m_newPasswdEdit, 2000);
    }
}

void ChangePasswordWidget::onRepeatPasswordTextEdited(const QString &text)
{
    Q_UNUSED(text)
    if (!m_repeatPasswdEdit->isAlert())
        return;

    // 清除警告信息
    m_repeatPasswdEdit->hideAlertMessage();
    m_repeatPasswdEdit->setAlert(false);
}

void ChangePasswordWidget::onRepeatPasswordEditFinished()
{
    if (sender() != m_repeatPasswdEdit)
        return;

    // 新旧密码不一致
    if (m_repeatPasswdEdit->text() != m_newPasswdEdit->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
    }
}

void ChangePasswordWidget::onPasswordHintsChanged(const QString &text)
{
    // 限制最长14个字符
    if (text.size() > 14) {
        m_passwordHints->lineEdit()->backspace();
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Error);
        return;
    }

    // 清除警告信息
    if (m_passwordHints->isAlert()) {
        m_passwordHints->hideAlertMessage();
        m_passwordHints->setAlert(false);
    }
}

void ChangePasswordWidget::onOkClicked()
{
    if (!isInfoValid()) {
        return;
    }

    const QString &oldPassword = m_oldPasswdEdit->text();
    const QString &newPassword = m_newPasswdEdit->text();
    const QString &repeatPassword = m_repeatPasswdEdit->text();

    qDebug() << "start change user password, user: " << m_user->name();
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::MergedChannels);

    // 登录界面修改密码时，当前用户是lightdm，需要先切换到对应的用户再修改用户的密码(如果密码已经过期，直接提权就会触发修改密码的流程)
    // 修改完密码删除keyring文件，避免弹窗(此处keyring无法解锁，登录后会解锁一次，但使用的是修改后的密码会解锁失败)
    process.start("/bin/bash", QStringList() << "-c" << QString("su %1 -c \"rm -f /home/%2/.local/share/keyrings/login.keyring\"").arg(m_user->name()).arg(m_user->name()));
    if (!m_user->isPasswordValid()) {
        process.write(QString("%1\n%2\n%3\n").arg(oldPassword).arg(newPassword).arg(repeatPassword).toLatin1());
    } else {
        process.write(QString("%1\n%2\n%3\n%4").arg(oldPassword).arg(oldPassword).arg(newPassword).arg(repeatPassword).toLatin1());
    }

    process.closeWriteChannel();
    process.waitForFinished();

    // exitCode = 0 表示密码修改成功
    int exitCode = process.exitCode();
    const QString& output = process.readAll();

    parseProcessResult(exitCode, output);
}

// 检查输入内容是否有效
bool ChangePasswordWidget::isInfoValid()
{
    m_oldPasswdEdit->setAlert(false);
    m_oldPasswdEdit->hideAlertMessage();
    // 旧密码为空
    if (m_oldPasswdEdit->text().isEmpty()) {
        m_oldPasswdEdit->setAlert(true);
        m_oldPasswdEdit->showAlertMessage("Password cannot be empty", m_oldPasswdEdit, 2000);
        return false;
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 新密码为空
    if (m_newPasswdEdit->text().isEmpty()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage("Password cannot be empty", m_newPasswdEdit, 2000);
        return false;
    }

    m_repeatPasswdEdit->setAlert(false);
    m_repeatPasswdEdit->hideAlertMessage();
    // 重复密码为空
    if (m_repeatPasswdEdit->text().isEmpty()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage("Password cannot be empty", m_repeatPasswdEdit, 2000);
        return false;
    }

    m_passwordHints->setAlert(false);
    m_passwordHints->hideAlertMessage();
    // 密码提示信息包含密码字段
    for (auto c : m_newPasswdEdit->text()) {
        if (m_passwordHints->text().contains(c)) {
            m_passwordHints->setAlert(true);
            m_passwordHints->showAlertMessage(tr("The hint is visible to all users. Do not include the password here."), m_passwordHints, 2000);
            return false;
        }
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 新旧密码一致
    if (m_newPasswdEdit->text() == m_oldPasswdEdit->text()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(tr("New password should differ from the current one"), m_repeatPasswdEdit, 2000);
        return false;
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 两次输入的新密码不一致
    if (m_repeatPasswdEdit->text() != m_newPasswdEdit->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
        return false;
    }

    return true;
}

void ChangePasswordWidget::parseProcessResult(int exitCode, const QString &output)
{
    qDebug() << "output: " << output;
    if (exitCode == 0) {
        Q_EMIT changePasswordSuccessed();

        // 提示修改成功
        DFloatingMessage *message = new DFloatingMessage(DFloatingMessage::MessageType::TransientType);
        QPalette pa;
        pa.setColor(QPalette::Background, QColor(247, 247, 247, 51));
        message->setPalette(pa);
        message->setIcon(QIcon::fromTheme("dialog-ok"));
        message->setMessage(tr("Password changed"));
        message->setAttribute(Qt::WA_DeleteOnClose);
        DMessageManager::instance()->sendMessage(qobject_cast<QWidget *>(parent()), message);

        // 更新密码提示信息
        UserInter userInter("org.deepin.dde.Accounts1", m_user->path(), QDBusConnection::systemBus(), this);
        userInter.SetPasswordHint(m_passwordHints->lineEdit()->text());
        return;
    }

    if (output.startsWith("Current Password: passwd:", Qt::CaseInsensitive)
            || output.contains("su: Authentication failure",Qt::CaseInsensitive)) {
        m_oldPasswdEdit->setAlert(true);
        m_oldPasswdEdit->showAlertMessage(tr("Wrong password"));
        return;
    }

    if (m_newPasswdEdit->lineEdit()->text() == m_oldPasswdEdit->lineEdit()->text()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(tr("New password should differ from the current one"), m_oldPasswdEdit, 2000);
        return;
    }

    PwqualityManager::ERROR_TYPE error = PwqualityManager::instance()->verifyPassword(m_user->displayName(), m_newPasswdEdit->text());
    if (error == PW_NO_ERR && m_newPasswdEdit->lineEdit()->text() != m_repeatPasswdEdit->lineEdit()->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
        return;
    }

    qDebug() << "password error type: " << error;
    m_newPasswdEdit->setAlert(true);
    m_newPasswdEdit->showAlertMessage(PwqualityManager::instance()->getErrorTips(error));
}

void ChangePasswordWidget::paintEvent(QPaintEvent *event)
{
#ifdef CONTRAST_BACKGROUND
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::red);
#else
    return QWidget::paintEvent(event);
#endif
}
