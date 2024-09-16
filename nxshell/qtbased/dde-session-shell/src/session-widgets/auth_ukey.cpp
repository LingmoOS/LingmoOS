// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_ukey.h"

#include "authcommon.h"
#include "dlineeditex.h"

#include <DHiDPIHelper>

#include <QKeyEvent>
#include <QTimer>

AuthUKey::AuthUKey(QWidget *parent)
    : AuthModule(AuthCommon::AT_Ukey, parent)
    , m_capsLock(new DLabel(this))
    , m_lineEdit(new DLineEditEx(this))
{
    setObjectName(QStringLiteral("AuthUkey"));
    setAccessibleName(QStringLiteral("AuthUkey"));

    initUI();
    initConnections();

    m_lineEdit->installEventFilter(this);
    setFocusProxy(m_lineEdit);
}

/**
 * @brief 初始化界面
 */
void AuthUKey::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->setFocusPolicy(Qt::StrongFocus);
    m_lineEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    setLineEditInfo(tr("Enter your PIN"), PlaceHolderText);

    QHBoxLayout *UKeyLayout = new QHBoxLayout(m_lineEdit->lineEdit());
    UKeyLayout->setContentsMargins(0, 0, 10, 0);
    UKeyLayout->setSpacing(5);
    /* 缩放因子 */
    UKeyLayout->addStretch(1);
    /* 大小写状态 */
    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(CAPS_LOCK);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_capsLock->setPixmap(pixmap);
    UKeyLayout->addWidget(m_capsLock, 0, Qt::AlignRight | Qt::AlignVCenter);
    /* 认证状态 */
    m_authStateLabel = new DLabel(m_lineEdit);
    setAuthStateStyle(LOGIN_WAIT);
    UKeyLayout->addWidget(m_authStateLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    mainLayout->addWidget(m_lineEdit);
}

/**
 * @brief 初始化信号连接
 */
void AuthUKey::initConnections()
{
    AuthModule::initConnections();
    /* PIN 码输入框 */
    connect(m_lineEdit, &DLineEditEx::focusChanged, this, [this](const bool focus) {
        if (!focus) m_lineEdit->setAlert(false);
        m_authStateLabel->setVisible(!focus);
        emit focusChanged(focus);
    });
    connect(m_lineEdit, &DLineEditEx::textChanged, this, [this](const QString &text) {
        m_lineEdit->setAlert(false);
        m_lineEdit->hideAlertMessage();
        emit lineEditTextChanged(text);
    });
    connect(m_lineEdit, &DLineEditEx::returnPressed, this, [ this ] {
        if (!m_lineEdit->lineEdit()->isReadOnly())
            emit requestAuthenticate();
    });
}

/**
 * @brief AuthUKey::reset
 */
void AuthUKey::reset()
{
    m_lineEdit->clear();
    m_lineEdit->setAlert(false);
    m_lineEdit->hideAlertMessage();
    setLineEditEnabled(true);
    setLineEditInfo(tr("Enter your PIN"), PlaceHolderText);
}

/**
 * @brief 设置认证状态
 *
 * @param state
 * @param result
 */
void AuthUKey::setAuthState(const int state, const QString &result)
{
    qDebug() << "AuthUKey::setAuthResult:" << state << result;
    m_state = state;
    switch (state) {
    case AuthCommon::AS_Success:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_CHECK);
        m_lineEdit->setAlert(false);
        m_lineEdit->clear();
        setLineEditEnabled(false);
        setLineEditInfo(tr("Verification successful"), PlaceHolderText);
        m_showPrompt = true;
        m_lineEdit->hideAlertMessage();
        emit authFinished(state);
        emit requestChangeFocus();
        break;
    case AuthCommon::AS_Failure:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        m_lineEdit->clear();
        m_lineEdit->setFocus();
        if (m_limitsInfo->locked) {
            m_lineEdit->setAlert(false);
            setLineEditEnabled(false);
            if (m_integerMinutes == 1) {
                setLineEditInfo(tr("Please try again 1 minute later"), PlaceHolderText);
            } else {
                setLineEditInfo(tr("Please try again %n minutes later", "", static_cast<int>(m_integerMinutes)), PlaceHolderText);
            }
        } else {
            setLineEditEnabled(true);
            const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
            if (leftTimes > 1) {
                setLineEditInfo(tr("Verification failed, %n chances left", "", leftTimes), PlaceHolderText);
            } else if (leftTimes == 1) {
                setLineEditInfo(tr("Verification failed, only one chance left"), PlaceHolderText);
            }
            setLineEditInfo(tr("Wrong PIN"), AlertText);
            m_showPrompt = false;
        }
        emit authFinished(state);
        break;
    case AuthCommon::AS_Cancel:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Timeout:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        setLineEditInfo(result, AlertText);
        break;
    case AuthCommon::AS_Error:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        setLineEditInfo(result, AlertText);
        break;
    case AuthCommon::AS_Verify:
        setAnimationState(true);
        setAuthStateStyle(LOGIN_SPINNER);
        break;
    case AuthCommon::AS_Exception:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        setLineEditInfo(tr("UKey is required"), PlaceHolderText);
        break;
    case AuthCommon::AS_Prompt:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        if (m_showPrompt) {
            setLineEditInfo(tr("Enter your PIN"), PlaceHolderText);
        }
        break;
    case AuthCommon::AS_Started:
        break;
    case AuthCommon::AS_Ended:
        break;
    case AuthCommon::AS_Locked:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_LOCK);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Recover:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Unlocked:
        setAuthStateStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    default:
        setAnimationState(false);
        setAuthStateStyle(LOGIN_WAIT);
        setLineEditInfo(result, AlertText);
        m_showPrompt = true;
        qWarning() << "Error! The state of UKey Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthUKey::setAnimationState(const bool start)
{
    start ? m_lineEdit->startAnimation() : m_lineEdit->stopAnimation();
}

/**
 * @brief 设置大小写图标状态
 *
 * @param isCapsOn
 */
void AuthUKey::setCapsLockVisible(const bool on)
{
    m_capsLock->setVisible(on);
}

/**
 * @brief 设置认证受限信息
 *
 * @param info
 */
void AuthUKey::setLimitsInfo(const LimitsInfo &info)
{
    qDebug() << "AuthUKey::setLimitsInfo" << info.unlockTime;
    AuthModule::setLimitsInfo(info);
}

/**
 * @brief 设置 LineEdit 是否可输入
 *
 * @param enable
 */
void AuthUKey::setLineEditEnabled(const bool enable)
{
    if (enable) {
        m_lineEdit->setFocusPolicy(Qt::StrongFocus);
        m_lineEdit->setFocus();
        m_lineEdit->lineEdit()->setReadOnly(false);
    } else {
        m_lineEdit->setFocusPolicy(Qt::NoFocus);
        m_lineEdit->clearFocus();
        m_lineEdit->lineEdit()->setReadOnly(true);
    }
}

/**
 * @brief 设置输入框中的文案
 *
 * @param text
 * @param type
 */
void AuthUKey::setLineEditInfo(const QString &text, const TextType type)
{
    switch (type) {
    case AlertText:
        m_lineEdit->showAlertMessage(text, this, 5000);
        m_lineEdit->setAlert(true);
        break;
    case InputText: {
        const int cursorPos = m_lineEdit->lineEdit()->cursorPosition();
        m_lineEdit->setText(text);
        m_lineEdit->lineEdit()->setCursorPosition(cursorPos);
        break;
    }
    case PlaceHolderText:
        m_lineEdit->setPlaceholderText(text);
        break;
    }
}

/**
 * @brief 获取输入框中的文字
 *
 * @return QString
 */
QString AuthUKey::lineEditText() const
{
    return m_lineEdit->text();
}

/**
 * @brief 更新认证锁定时的文案
 *
 */
void AuthUKey::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_integerMinutes == 1) {
        m_lineEdit->setPlaceholderText(tr("Please try again 1 minute later"));
    } else if (m_integerMinutes > 1) {
        m_lineEdit->setPlaceholderText(tr("Please try again %n minute(s) later", "", static_cast<int>(m_integerMinutes)));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(m_type);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

bool AuthUKey::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<DLineEditEx *>(watched) == m_lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Cut)
            || keyEvent->matches(QKeySequence::Copy)
            || keyEvent->matches(QKeySequence::Paste)) {
            return true;
        }
    }
    return false;
}


void AuthUKey::hide()
{
    AuthModule::hide();
}
