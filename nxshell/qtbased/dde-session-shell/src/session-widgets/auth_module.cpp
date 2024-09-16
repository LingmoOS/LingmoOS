// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_module.h"

#include <DHiDPIHelper>

#include <QDateTime>
#include <QTimer>
#include <QtMath>

void LimitsInfo::operator=(const LimitsInfo &info)
{
    locked = info.locked;
    maxTries = info.maxTries;
    numFailures = info.numFailures;
    unlockSecs = info.unlockSecs;
    unlockTime = info.unlockTime;
}

AuthModule::AuthModule(const AuthCommon::AuthType type, QWidget *parent)
    : QWidget(parent)
    , m_inputType(AuthCommon::IT_Default)
    , m_state(AuthCommon::AS_None)
    , m_type(type)
    , m_showPrompt(true)
    , m_integerMinutes(0)
    , m_limitsInfo(new LimitsInfo())
    , m_aniTimer(new QTimer(this))
    , m_unlockTimer(new QTimer(this))
    , m_showAuthState(true)
    , m_isAuthing(false)
    , m_authFactorType(DDESESSIONCC::SingleAuthFactor)
{
    setMinimumHeight(37);

    m_limitsInfo->locked = false;
    m_limitsInfo->maxTries = 0;
    m_limitsInfo->numFailures = 0;
    m_limitsInfo->unlockSecs = 0;
    m_limitsInfo->unlockTime = QString("0001-01-01T00:00:00Z");

    m_unlockTimer->setSingleShot(false);
    m_unlockTimer->setInterval(1000);
}

AuthModule::~AuthModule()
{
    delete m_limitsInfo;
}

/**
 * @brief 初始化信号连接
 */
void AuthModule::initConnections()
{
    /* 认证解锁时间 */
    connect(m_unlockTimer, &QTimer::timeout, this, [ this ] {
        updateIntegerMinutes();
        if (m_integerMinutes == 0)
            m_unlockTimer->stop();
        updateUnlockPrompt();
    });
    /* 解锁动画 */
    connect(m_aniTimer, &QTimer::timeout, this, &AuthModule::doAnimation);
}

/**
 * @brief 设置认证动画状态，由派生类重载，自定义动画样式。
 *
 * @param start
 */
void AuthModule::setAnimationState(const bool start)
{
    Q_UNUSED(start)
}

/**
 * @brief 设置认证的状态，由派生类重载，自定义认证状态显示样式。
 *
 * @param state
 * @param result
 */
void AuthModule::setAuthState(const int state, const QString &result)
{
    Q_UNUSED(result)

    if (AuthCommon::AS_Started == state)
        m_isAuthing = true;
    else if (AuthCommon::AS_Ended == state)
        m_isAuthing = false;
}

/**
 * @brief 设置认证状态图标的样式
 *
 * @param path
 */
void AuthModule::setAuthStateStyle(const QString &path)
{
    if (!m_authStateLabel)
        return;

    QPixmap pixmap = DHiDPIHelper::loadNxPixmap(path);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_authStateLabel->setPixmap(pixmap);
}

/**
 * @brief 设置认证受限信息
 *
 * @param info
 */
void AuthModule::setLimitsInfo(const LimitsInfo &info)
{
    *m_limitsInfo = info;
    updateUnlockTime();
}

void AuthModule::setShowAuthState(bool showAuthState)
{
    m_showAuthState = showAuthState;
}

void AuthModule::setAuthStatueVisible(bool visible)
{
    if (m_authStateLabel)
        m_authStateLabel->setVisible(visible);
}

/**
 * @brief 更新认证剩余解锁时间提示
 */
void AuthModule::updateUnlockPrompt()
{
    qInfo() << m_type << "has" << m_integerMinutes << "minutes left to unlock.";
}

/**
 * @brief 更新认证锁定后的解锁时间
 */
void AuthModule::updateUnlockTime()
{
    if (QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs) <= QDateTime::currentDateTime()) {
        m_integerMinutes = 0;
        m_unlockTimer->stop();
        if (m_limitsInfo->locked)
            updateUnlockPrompt();
        return;
    }
    updateIntegerMinutes();
    updateUnlockPrompt();
    m_unlockTimer->start();
}

void AuthModule::updateIntegerMinutes()
{
    if (QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs) > QDateTime::currentDateTime()) {
        qreal intervalSeconds = QDateTime::fromString(m_limitsInfo->unlockTime, Qt::ISODateWithMs).toLocalTime().toTime_t()
                               - QDateTime::currentDateTimeUtc().toTime_t();
        m_integerMinutes = static_cast<uint>(qCeil(intervalSeconds / 60));
    } else {
        m_integerMinutes = 0;
    }
}

void AuthModule::setAuthStateLabel(DLabel *label)
{
    if (!label)
        return;

    if (m_authStateLabel) {
        delete m_authStateLabel;
        m_authStateLabel = nullptr;
    }

    /* 认证状态 */
    m_authStateLabel = label;
    setAuthStateStyle(AUTH_LOCK);
}

void AuthModule::setAuthFactorType(AuthFactorType authFactorType)
{
    m_authFactorType = authFactorType;
}

bool AuthModule::isLocked() const
{
    return m_limitsInfo->locked;
}