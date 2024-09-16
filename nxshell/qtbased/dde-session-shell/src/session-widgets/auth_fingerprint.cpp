// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_fingerprint.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTimer>

AuthFingerprint::AuthFingerprint(QWidget *parent)
    : AuthModule(AuthCommon::AT_Fingerprint, parent)
    , m_aniIndex(-1)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthFingerprint"));
    setAccessibleName(QStringLiteral("AuthFingerprint"));

    initUI();
    initConnections();
}

/**
 * @brief 初始化界面
 */
void AuthFingerprint::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(27, 0, 10, 0);
    mainLayout->setSpacing(0);
    /* 文案提示 */
    m_textLabel->setText(tr("Verify your fingerprint"));
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignHCenter | Qt::AlignVCenter);
    /* 认证状态 */
    m_authStateLabel = new DLabel(this);
    setAuthStateStyle(LOGIN_WAIT);
    mainLayout->addWidget(m_authStateLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthFingerprint::initConnections()
{
    AuthModule::initConnections();
}

/**
 * @brief AuthFingerprint::reset
 */
void AuthFingerprint::reset()
{
    m_textLabel->setText(tr("Verify your fingerprint"));
}

/**
 * @brief 设置认证状态
 *
 * @param state
 * @param result
 */
void AuthFingerprint::setAuthState(const int state, const QString &result)
{
    qDebug() << "AuthFingerprint::setAuthResult:" << state << result;
    m_state = state;
    switch (state) {
    case AuthCommon::AS_Success:
        if (isMFA())
            setAuthStateStyle(LOGIN_CHECK);
        else
            setAnimationState(true);
        m_textLabel->setText(tr("Verification successful"));
        m_showPrompt = true;
        emit authFinished(state);
        break;
    case AuthCommon::AS_Failure: {
        setAnimationState(false);
        if (isMFA())
            setAuthStateStyle(LOGIN_WAIT);
        m_showPrompt = false;
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        emit authFinished(state);
        break;
    }
    case AuthCommon::AS_Cancel:
        setAnimationState(false);
        if (isMFA())
            setAuthStateStyle(LOGIN_WAIT);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Timeout:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AuthCommon::AS_Error:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AuthCommon::AS_Verify:
        setAnimationState(true);
        setAuthStateStyle(isMFA() ? LOGIN_SPINNER : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AuthCommon::AS_Exception:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AuthCommon::AS_Prompt:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        if (m_showPrompt) {
            m_textLabel->setText(tr("Verify your fingerprint"));
        }
        break;
    case AuthCommon::AS_Started:
        break;
    case AuthCommon::AS_Ended:
        break;
    case AuthCommon::AS_Locked:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_LOCK : AUTH_LOCK);
        m_textLabel->setText(tr("Fingerprint locked, use password please"));
        break;
    case AuthCommon::AS_Recover:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        break;
    case AuthCommon::AS_Unlocked:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        emit activeAuth(AuthCommon::AT_Fingerprint);
        break;
    default:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        qWarning() << "Error! The state of Fingerprint Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthFingerprint::setAnimationState(const bool start)
{
    m_aniIndex = 1;

    if (start) {
        m_aniTimer->start(20);
    } else {
        m_aniTimer->stop();
    }
}

/**
 * @brief 更新认证受限信息
 *
 * @param info
 */
void AuthFingerprint::setLimitsInfo(const LimitsInfo &info)
{
    AuthModule::setLimitsInfo(info);
}

void AuthFingerprint::setAuthFactorType(AuthFactorType authFactorType)
{
    if (DDESESSIONCC::SingleAuthFactor == authFactorType)
        layout()->setContentsMargins(10, 0, 10, 0);

    AuthModule::setAuthFactorType(authFactorType);
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthFingerprint::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_limitsInfo->locked) {
        m_textLabel->setText(tr("Fingerprint locked, use password please"));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(m_type);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

void AuthFingerprint::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit activeAuth(m_type);
    }
    QWidget::mouseReleaseEvent(event);
}

/**
 * @brief 执行动画（认证成功，认证失败，认证中...）
 */
void AuthFingerprint::doAnimation()
{
    if (m_state == AuthCommon::AS_Success) {
        if (m_aniIndex > 10) {
            m_aniTimer->stop();
            emit authFinished(AuthCommon::AS_Success);
        } else {
            setAuthStateStyle(QStringLiteral(":/misc/images/unlock/unlock_%1.svg").arg(m_aniIndex++));
        }
    }
}
