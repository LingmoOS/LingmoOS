/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QBindable>

#ifdef Q_OS_UNIX
#include <QObject>

#include <KConfigWatcher>
#endif

#ifdef Q_OS_WIN
#include <QAbstractNativeEventFilter>
#endif

class AnimationSpeedProvider
{
public:
    explicit AnimationSpeedProvider();
    virtual ~AnimationSpeedProvider();
    Q_DISABLE_COPY_MOVE(AnimationSpeedProvider)

    QBindable<double> animationSpeedModifier() const;

protected:
    QProperty<double> m_animationSpeedModifier{1.0};
};

#ifdef Q_OS_UNIX
class KConfigAnimationSpeedProvider : public QObject, public AnimationSpeedProvider
{
    Q_OBJECT

public:
    explicit KConfigAnimationSpeedProvider(QObject *parent = nullptr);
    ~KConfigAnimationSpeedProvider() override;
    Q_DISABLE_COPY_MOVE(KConfigAnimationSpeedProvider)

private:
    KConfigWatcher::Ptr m_animationSpeedWatcher;
};
#endif

#ifdef Q_OS_WIN
class WindowsAnimationSpeedProvider : public QAbstractNativeEventFilter, public AnimationSpeedProvider
{
public:
    explicit WindowsAnimationSpeedProvider();
    ~WindowsAnimationSpeedProvider() override;
    Q_DISABLE_COPY_MOVE(WindowsAnimationSpeedProvider)

private:
    void update();
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
};
#endif
