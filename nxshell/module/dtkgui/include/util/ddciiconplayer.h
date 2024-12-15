// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <dtkgui_global.h>
#include <DObject>
#include <DDciIcon>

#include <QObject>

DGUI_BEGIN_NAMESPACE

class DDciIconImage;
class DDciIconPalette;
class DDciIconImagePlayerPrivate;
class DDciIconImagePlayer : public QObject, public DCORE_NAMESPACE::DObject
{
    D_DECLARE_PRIVATE(DDciIconImagePlayer)
    Q_OBJECT
    Q_PROPERTY(DGUI_NAMESPACE::DDciIconImagePlayer::State state READ state NOTIFY stateChanged)

public:
    enum State {
        NotRunning,
        WaitingRead,
        Running
    };
    Q_ENUM(State)

    enum Flag {
        Continue = 1,
        CacheAll = 2,
        InvertedOrder = 4,
        IgnoreLastImageLoop = 8,
        AllowNonLastImageLoop = 16,
        ClearCacheOnStop = 32
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit DDciIconImagePlayer(QObject *parent = nullptr);

    void setImages(const QVector<DDciIconImage> &images);
    QVector<DDciIconImage> images() const;
    const DDciIconImage &currentImage() const;
    bool currentLoopForever() const;

    bool setPalette(const DDciIconPalette &palette);
    DDciIconPalette palette() const;

    void setLoopCount(int count);
    int loopCount() const;
    void abortLoop();

    QImage readImage();
    State state() const;

    void clearCache();

public Q_SLOTS:
    bool start(float speed = 1.0, DGUI_NAMESPACE::DDciIconImagePlayer::Flags flags = {});
    void stop();

Q_SIGNALS:
    void started();
    void updated();
    void finished();
    void stateChanged();

private:
    void timerEvent(QTimerEvent *event) override;
};

class DDciIconPlayerPrivate;
class DDciIconPlayer : public QObject, public DCORE_NAMESPACE::DObject
{
    D_DECLARE_PRIVATE(DDciIconPlayer)
    Q_OBJECT
public:
    enum State {
        Idle,
        Busy
    };
    Q_ENUM(State)

    explicit DDciIconPlayer(QObject *parent = nullptr);

    State state() const;

    void setIcon(const DDciIcon &icon);
    DDciIcon icon() const;

    void setTheme(DDciIcon::Theme theme);
    DDciIcon::Theme theme() const;

    void setMode(DDciIcon::Mode mode);
    DDciIcon::Mode mode() const;

    void setIconSize(int size);
    int iconSize() const;

    void setDevicePixelRatio(qreal devicePixelRatio);
    qreal devicePixelRatio() const;

    void setPalette(const DDciIconPalette &palette);

    QImage currentImage() const;
    void play(DDciIcon::Mode mode);
    void stop();
    void abort();

Q_SIGNALS:
    void stateChanged();
    void updated();
    void modeChanged(DDciIcon::Mode oldMode, DDciIcon::Mode newMode);

private:
    D_PRIVATE_SLOT(void _q_playFromQueue(int))
};

DGUI_END_NAMESPACE
Q_DECLARE_OPERATORS_FOR_FLAGS(DTK_GUI_NAMESPACE::DDciIconImagePlayer::Flags)
