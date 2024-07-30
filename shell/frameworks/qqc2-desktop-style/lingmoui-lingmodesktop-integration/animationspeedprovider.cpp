/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "animationspeedprovider.h"

#ifdef Q_OS_UNIX
#include <KConfigGroup>
#include <KSharedConfig>
#endif

#ifdef Q_OS_WIN
#include <QCoreApplication>
#include <Windows.h>
#endif

using namespace Qt::StringLiterals;

AnimationSpeedProvider::AnimationSpeedProvider()
{
}

AnimationSpeedProvider::~AnimationSpeedProvider()
{
}

QBindable<double> AnimationSpeedProvider::animationSpeedModifier() const
{
    return &m_animationSpeedModifier;
}

#ifdef Q_OS_UNIX
KConfigAnimationSpeedProvider::KConfigAnimationSpeedProvider(QObject *parent)
    : QObject(parent)
    , AnimationSpeedProvider()
    , m_animationSpeedWatcher(KConfigWatcher::create(KSharedConfig::openConfig()))
{
    connect(m_animationSpeedWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group, const QByteArrayList &names) {
        if (group.name() == "KDE"_L1 && names.contains(QByteArrayLiteral("AnimationDurationFactor"))) {
            m_animationSpeedModifier = std::max<double>(0.0, group.readEntry(u"AnimationDurationFactor"_s, 1.0));
        }
    });

    KConfigGroup generalCfg = KConfigGroup(KSharedConfig::openConfig(), u"KDE"_s);
    m_animationSpeedModifier = std::max<double>(0.0, generalCfg.readEntry(u"AnimationDurationFactor"_s, 1.0));
}

KConfigAnimationSpeedProvider::~KConfigAnimationSpeedProvider()
{
}
#endif

#ifdef Q_OS_WIN
WindowsAnimationSpeedProvider::WindowsAnimationSpeedProvider()
    : QAbstractNativeEventFilter()
    , AnimationSpeedProvider()
{
    update();
    QCoreApplication::instance()->installNativeEventFilter(this);
}

WindowsAnimationSpeedProvider::~WindowsAnimationSpeedProvider()
{
}

void WindowsAnimationSpeedProvider::update()
{
    bool isAnimated = true;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &isAnimated, 0)) {
        m_animationSpeedModifier = isAnimated ? 1.0 : 0.0;
    }
}

bool WindowsAnimationSpeedProvider::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
    if (eventType != "windows_generic_MSG") {
        return false;
    }

    MSG *msg = static_cast<MSG *>(message);
    if (msg->message != WM_SETTINGCHANGE || msg->wParam != SPI_SETCLIENTAREAANIMATION) {
        return false;
    }

    update();
    return false;
}
#endif
