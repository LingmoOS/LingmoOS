// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_face.h"

#include <QKeyEvent>
#include <QTimer>
#include <QVBoxLayout>

AuthFace::AuthFace(QWidget *parent)
    : AuthModule(AuthCommon::AT_Face, parent)
    , m_aniIndex(-1)
    , m_textLabel(new DLabel(this))
{
    setObjectName(QStringLiteral("AuthFace"));
    setAccessibleName(QStringLiteral("AuthFace"));

    initUI();
    initConnections();
}

/**
 * @brief 初始化界面
 */
void AuthFace::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(27, 0, 10, 0);
    mainLayout->setSpacing(0);
    /* 文案提示 */
    m_textLabel->setText(tr("Face ID"));
    m_textLabel->setWordWrap(true);
    mainLayout->addWidget(m_textLabel, 1, Qt::AlignHCenter);
    /* 认证状态 */
    m_authStateLabel = new DLabel(this);
    m_authStateLabel->installEventFilter(this);
    setAuthStateStyle(LOGIN_WAIT);
    mainLayout->addWidget(m_authStateLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
}

/**
 * @brief 初始化信号连接
 */
void AuthFace::initConnections()
{
    AuthModule::initConnections();
}

/**
 * @brief AuthFace::reset
 */
void AuthFace::reset()
{
    m_state = AuthCommon::AS_Ended;
    m_textLabel->setText(tr("Face ID"));
    if (m_authStateLabel) {
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_authStateLabel->show();
    }
}

/**
 * @brief 设置认证状态
 *
 * @param state
 * @param result
 */
void AuthFace::setAuthState(const int state, const QString &result)
{
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
        emit retryButtonVisibleChanged(false);
        break;
    case AuthCommon::AS_Failure: {
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_RETRY : AUTH_LOCK);
        m_showPrompt = false;
        const int leftTimes = static_cast<int>(m_limitsInfo->maxTries - m_limitsInfo->numFailures);
        if (leftTimes > 1) {
            m_textLabel->setText(tr("Verification failed, %n chances left", "", leftTimes));
        } else if (leftTimes == 1) {
            m_textLabel->setText(tr("Verification failed, only one chance left"));
        }
        emit retryButtonVisibleChanged(true);
        emit authFinished(state);
        break;
    }
    case AuthCommon::AS_Cancel:
        setAnimationState(false);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Timeout:
    case AuthCommon::AS_Error:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Verify:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_SPINNER : AUTH_LOCK);
        m_textLabel->setText(result);
        break;
    case AuthCommon::AS_Exception:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(tr("Device unavailable!"));
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Prompt:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        break;
    case AuthCommon::AS_Started:
        m_textLabel->setText(tr("Verify your Face ID"));
        break;
    case AuthCommon::AS_Ended:
        break;
    case AuthCommon::AS_Locked:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_LOCK : AUTH_LOCK);
        m_textLabel->setText(tr("Face ID locked, use password please"));
        m_showPrompt = true;
        if (DDESESSIONCC::SingleAuthFactor == m_authFactorType)
            emit retryButtonVisibleChanged(false);
        break;
    case AuthCommon::AS_Recover:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_showPrompt = true;
        break;
    case AuthCommon::AS_Unlocked:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_showPrompt = true;
        emit activeAuth(AuthCommon::AT_Face);
        break;
    default:
        setAnimationState(false);
        setAuthStateStyle(isMFA() ? LOGIN_WAIT : AUTH_LOCK);
        m_textLabel->setText(result);
        qWarning() << "Error! The state of Face Auth is wrong!" << state << result;
        break;
    }
    update();
}

/**
 * @brief 设置认证动画状态
 *
 * @param start
 */
void AuthFace::setAnimationState(const bool start)
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
void AuthFace::setLimitsInfo(const LimitsInfo &info)
{
    AuthModule::setLimitsInfo(info);
}

/**
 * @brief 更新认证锁定时的文案
 */
void AuthFace::updateUnlockPrompt()
{
    AuthModule::updateUnlockPrompt();
    if (m_limitsInfo->locked) {
        m_textLabel->setText(tr("Face ID locked, use password please"));
    } else {
        QTimer::singleShot(1000, this, [this] {
            emit activeAuth(m_type);
        });
        qInfo() << "Waiting authentication service...";
    }
    update();
}

/**
 * @brief 执行动画（认证成功，认证失败，认证中...）
 */
void AuthFace::doAnimation()
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

bool AuthFace::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_authStateLabel
        && QEvent::MouseButtonRelease == event->type()
        && !m_isAuthing
        && AuthCommon::AS_Locked != m_state
        && DDESESSIONCC::MultiAuthFactor == m_authFactorType) {
        emit activeAuth(m_type);
    }

    return false;
}

void AuthFace::setAuthFactorType(AuthFactorType authFactorType)
{
    if (DDESESSIONCC::SingleAuthFactor == authFactorType)
        layout()->setContentsMargins(10, 0, 10, 0);

    AuthModule::setAuthFactorType(authFactorType);
}
