// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddciiconplayer.h"
#include "ddciicon.h"

#include <DObjectPrivate>
#include <QTimerEvent>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QDebug>

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(diPlayer, "dtk.dciicon.player")
#else
Q_LOGGING_CATEGORY(diPlayer, "dtk.dciicon.player", QtInfoMsg)
#endif

class DDciIconImagePlayerPrivate : DObjectPrivate
{
public:
    DDciIconImagePlayerPrivate(DDciIconImagePlayer *qq)
        : DObjectPrivate(qq) {}

    bool initCurrent();
    bool ensureCurrent();
    void clearCache();
    void setState(DDciIconImagePlayer::State newState);

    inline bool reversed() const {
        return flags.testFlag(DDciIconImagePlayer::InvertedOrder);
    }
    inline bool hasCache(int imageIndex, int frameNumber) const {
        if (imageIndex < 0 || frameNumber < 0)
            return false;
        if (cachedFrames.size() <= imageIndex)
            return false;
        const auto &i = cachedFrames.at(imageIndex);
        if (i.size() <= frameNumber)
            return false;
        return true;
    }
    inline bool currentHasCache() const {
        return current >= 0 && currentFrameNumber >= 0 && hasCache(current, currentFrameNumber);
    }

    D_DECLARE_PUBLIC(DDciIconImagePlayer)
    QVector<DDciIconImage> images;
    DDciIconPalette palette;
    inline DDciIconImage &currentImage() {
        return images[current];
    }

    DDciIconImagePlayer::State state = DDciIconImagePlayer::NotRunning;
    DDciIconImagePlayer::Flags flags;
    float speed = 1.0;
    // loop count for all images sequential animation
    int userLoopCount = 1;

    struct Frame {
        QImage image;
        int duration;
    };
    QVector<QVector<Frame>> cachedFrames;
    inline QVector<Frame> &currentCache() {
        return cachedFrames[current];
    }

    int timerId = 0;
    // loop count for all images sequential animation, init from "userLoopCount" when start
    int loopCount = 1;
    int current = 0;
    int currentLoopCount = 0;
    int currentFrameNumber = 0;
};

inline static bool jumpImageTo(DDciIconImage &image, int number)
{
    if (number < 0)
        return false;

    if (image.currentImageNumber() > number)
        image.reset();
    for (int i = image.currentImageNumber(); i < number; ++i)
        if (!image.jumpToNextImage())
            return false;;
    return true;
}

bool DDciIconImagePlayerPrivate::initCurrent()
{
    if (!currentImage().supportsAnimation())
        return false;

    if (reversed()) {
        if (currentFrameNumber < 0)
            currentFrameNumber = currentCache().size() - 1;
    } else {
        if (currentFrameNumber < 0)
            currentFrameNumber = 0;
    }

    if (!currentHasCache()) {
        if (!jumpImageTo(currentImage(), currentFrameNumber))
            return false;
    }

    if ((flags & DDciIconImagePlayer::CacheAll) && cachedFrames.size() <= current) {
        cachedFrames.append(QVector<DDciIconImagePlayerPrivate::Frame>());
    }

    if (current == images.size() - 1) {
        if (flags.testFlag(DDciIconImagePlayer::IgnoreLastImageLoop)) {
            currentLoopCount = 1;
        } else {
            currentLoopCount = currentImage().loopCount();
        }
    } else if (flags.testFlag(DDciIconImagePlayer::AllowNonLastImageLoop)) {
        currentLoopCount = currentImage().loopCount();
    } else {
        currentLoopCount = 1;
    }

    if (currentLoopCount == 0)
        currentLoopCount = 1;

    return true;
}

bool DDciIconImagePlayerPrivate::ensureCurrent()
{
    while (current >= 0 && current < images.size()) {
        if (initCurrent())
            return true;
        current += reversed() ? -1 : 1;
        currentFrameNumber = -1;
    }

    return false;
}

void DDciIconImagePlayerPrivate::clearCache()
{
    cachedFrames.clear();
}

void DDciIconImagePlayerPrivate::setState(DDciIconImagePlayer::State newState)
{
    if (state == newState)
        return;
    state = newState;
    Q_EMIT q_func()->stateChanged();
}

DDciIconImagePlayer::DDciIconImagePlayer(QObject *parent)
    : QObject(parent)
    , DObject(*new DDciIconImagePlayerPrivate(this))
{

}

void DDciIconImagePlayer::setImages(const QVector<DDciIconImage> &images)
{
    D_D(DDciIconImagePlayer);
    if (d->images == images)
        return;

    if (d->state != NotRunning)
        stop();
    Q_ASSERT(d->state == NotRunning);
    d->images = images;
    d->current = -1;
    d->currentFrameNumber = -1;
    d->clearCache();
}

QVector<DDciIconImage> DDciIconImagePlayer::images() const
{
    D_DC(DDciIconImagePlayer);
    return d->images;
}

const DDciIconImage &DDciIconImagePlayer::currentImage() const
{
    D_DC(DDciIconImagePlayer);
    static DDciIconImage nullImage;
    if (d->state == NotRunning)
        return nullImage;
    return d->images.at(d->current);
}

bool DDciIconImagePlayer::currentLoopForever() const
{
    D_DC(DDciIconImagePlayer);
    return d->state != NotRunning && d->currentLoopCount < 0;
}

bool DDciIconImagePlayer::setPalette(const DDciIconPalette &palette)
{
    D_D(DDciIconImagePlayer);
    if (d->palette == palette)
        return false;
    d->palette = palette;

    bool hasPalette = false;
    for (const auto &i : qAsConst(d->images))
        if (i.hasPalette())
            hasPalette = true;

    if (!hasPalette)
        return true;

    if (d->state == NotRunning) {
        d->clearCache();
    } else {
        d->flags |= ClearCacheOnStop;
    }

    return true;
}

DDciIconPalette DDciIconImagePlayer::palette() const
{
    D_DC(DDciIconImagePlayer);
    return d->palette;
}

void DDciIconImagePlayer::setLoopCount(int count)
{
    D_D(DDciIconImagePlayer);
    if (count <= 0)
        return;
    if (d->state != NotRunning) {
        d->loopCount += (count - d->userLoopCount);
    }
    d->userLoopCount = count;
}

int DDciIconImagePlayer::loopCount() const
{
    D_DC(DDciIconImagePlayer);
    return d->userLoopCount;
}

void DDciIconImagePlayer::abortLoop()
{
    D_D(DDciIconImagePlayer);

    if (d->state == NotRunning)
        return;

    d->flags = (d->flags | IgnoreLastImageLoop) & (~AllowNonLastImageLoop);
    d->currentLoopCount = 0;
    d->loopCount = 0;
}

QImage DDciIconImagePlayer::readImage()
{
    D_D(DDciIconImagePlayer);

    if (d->state != WaitingRead)
        return QImage();

    QImage image;
    int timerIntervel = 0;

    if (d->currentHasCache()) {
        image = d->currentCache().at(d->currentFrameNumber).image;
        timerIntervel = qRound(d->currentCache().at(d->currentFrameNumber).duration / d->speed);
    } else {
        Q_ASSERT(!d->reversed());
        Q_ASSERT(d->currentImage().currentImageNumber() == d->currentFrameNumber);
        image = d->currentImage().toImage(d->palette);
        if (d->flags & CacheAll) {
            Q_ASSERT(d->currentImage().currentImageNumber() == d->currentFrameNumber);
            Q_ASSERT(d->currentCache().size() == d->currentFrameNumber);
            d->currentCache().append({image, d->currentImage().currentImageDuration()});
        }
        timerIntervel = qRound(d->currentImage().currentImageDuration() / d->speed);
    }

    d->timerId = startTimer(timerIntervel < 0 ? 0 : timerIntervel);
    if (d->timerId == 0) {
        qCWarning(diPlayer, "Can't start timer, will abort the animations.");
        stop();
        Q_EMIT finished();
        return image;
    }

    d->setState(Running);

    return image;
}

DDciIconImagePlayer::State DDciIconImagePlayer::state() const
{
    D_DC(DDciIconImagePlayer);
    return d->state;
}

void DDciIconImagePlayer::clearCache()
{
    D_D(DDciIconImagePlayer);
    d->clearCache();
}

bool DDciIconImagePlayer::start(float speed, Flags flags)
{
    D_D(DDciIconImagePlayer);

    if (d->state != NotRunning)
        return false;

    if (d->images.isEmpty())
        return false;

    d->flags = flags;
    if (!d->flags.testFlag(Continue) || d->current < 0) {
        d->current = d->reversed() ? d->images.size() - 1 : 0;
    }

    if (d->reversed()) {
        d->flags |= CacheAll;

        if (d->cachedFrames.isEmpty()) {
            d->cachedFrames.resize(d->images.size());
            for (int i = 0; i <= d->current; ++i) {
                DDciIconImage &image = d->images[i];
                if (!image.supportsAnimation())
                    continue;
                image.reset();

                do {
                    d->cachedFrames[i].append({image.toImage(d->palette), image.currentImageDuration()});
                } while (image.jumpToNextImage());
            }
        } else if (!flags.testFlag(Continue)) {
            auto &image = d->images[d->cachedFrames.size() - 1];
            if (image.supportsAnimation()
                    && jumpImageTo(image, d->cachedFrames.last().size())) {
                do {
                    d->cachedFrames.last().append({image.toImage(d->palette), image.currentImageDuration()});
                } while (image.jumpToNextImage());
            }

            for (int i = d->cachedFrames.size(); i < d->images.size(); ++i) {
                d->cachedFrames.append(QVector<DDciIconImagePlayerPrivate::Frame>());
                auto &image = d->images[i];
                if (!image.supportsAnimation())
                    continue;

                image.reset();
                do {
                    d->cachedFrames[i].append({image.toImage(d->palette), image.currentImageDuration()});
                } while (image.jumpToNextImage());
            }

            d->currentFrameNumber = d->currentCache().size() - 1;
        }
    } else {
        if (flags.testFlag(Continue)) {
            if (d->current < 0)
                d->current = 0;
        } else {
            d->current = 0;
            d->currentFrameNumber = 0;
        }

        // Init cache
        if (flags.testFlag(CacheAll) && d->cachedFrames.isEmpty()
                && (d->current > 0 || d->currentFrameNumber > 0)) {
            d->cachedFrames.resize(d->images.size());
            DDciIconImage &image = d->images[d->current];
            if (image.supportsAnimation()) {
                image.reset();

                do {
                    d->cachedFrames[d->current].append({image.toImage(d->palette), image.currentImageDuration()});
                } while (image.jumpToNextImage());
            }
        }
    }

    d->speed = speed > 0.0 ? speed : 1.0;
    d->loopCount = d->userLoopCount;

    if (!d->ensureCurrent())
        return false;
    d->setState(WaitingRead);
    Q_EMIT started();
    Q_EMIT updated();

    return true;
}

void DDciIconImagePlayer::stop()
{
    D_D(DDciIconImagePlayer);
    if (d->state == NotRunning)
        return;
    if (d->timerId > 0) {
        killTimer(d->timerId);
        d->timerId = 0;
    }

    if (d->flags & ClearCacheOnStop)
        d->clearCache();

    d->setState(NotRunning);
}

void DDciIconImagePlayer::timerEvent(QTimerEvent *event)
{
    D_D(DDciIconImagePlayer);
    if (event->timerId() != d->timerId)
        return QObject::timerEvent(event);

    killTimer(d->timerId);
    d->timerId = 0;

    bool finished = false;
    const int newFrameNumber =  d->currentFrameNumber + (d->reversed() ? -1 : 1);
    if (!d->hasCache(d->current, newFrameNumber)) {
        if (d->reversed()) {
            finished = true;
        } else {
            finished = !jumpImageTo(d->currentImage(), newFrameNumber);
        }
    }

#define IGNORE_LOOP qEnvironmentVariableIsSet("D_DTK_DCI_PLAYER_IGNORE_ANIMATION_LOOP")

    if (finished) {
        if (d->currentLoopCount != 0 && --d->currentLoopCount && !IGNORE_LOOP) {
            // reset value
            d->currentFrameNumber = -1;
            // replay current image
            bool ok = d->initCurrent();
            Q_ASSERT(ok);
            finished = false;
        } else {
            const int newCurrent = d->current + (d->reversed() ? -1 : 1);
            if (newCurrent >= 0 && newCurrent < d->images.size()) {
                d->current = newCurrent;
                d->currentFrameNumber = -1;
                if (d->ensureCurrent())
                    finished = false;
            }
        }
    } else {
        d->currentFrameNumber = newFrameNumber;
    }

    if (finished && d->loopCount != 0 && (--d->loopCount) && !IGNORE_LOOP) {
        d->current = d->reversed() ? d->images.size() - 1 : 0;
        d->currentFrameNumber = -1;
        if (d->ensureCurrent())
            finished = false;
    }

    if (finished) {
        stop();
        Q_EMIT this->finished();
    } else {
        d->setState(WaitingRead);
        Q_EMIT updated();
    }
}

static QString iconModeToString(DDciIcon::Mode mode)
{
    switch (mode) {
    case DDciIcon::Normal:
        return QStringLiteral("Normal");
    case DDciIcon::Hover:
        return QStringLiteral("Hover");
    case DDciIcon::Pressed:
        return QStringLiteral("Pressed");
    case DDciIcon::Disabled:
        return QStringLiteral("Disabled");
    default:
        break;
    }

    return nullptr;
}

class DDciIconPlayerPrivate : public DObjectPrivate
{
public:
    DDciIconPlayerPrivate(DDciIconPlayer *qq)
        : DObjectPrivate(qq) {}

    void ensureInit();
    void initPlayer();
    bool ensureHoverModeLastImage();
    void play(DDciIcon::Mode mode, DDciIconImagePlayer::Flags flags);
    bool play(DDciIcon::Mode fromMode, DDciIcon::Mode toMode,
              DDciIconImagePlayer::Flags extraflags = {});
    inline void _q_playFromQueue(int extraflags = 0) {
        if (player && player->state() != DDciIconImagePlayer::NotRunning)
            return;

        if (animationJobs.isEmpty())
            return;

        const auto &job = animationJobs.first();
        bool ok = play(static_cast<DDciIcon::Mode>(job.from), static_cast<DDciIcon::Mode>(job.to),
                       DDciIconImagePlayer::Flags(static_cast<DDciIconImagePlayer::Flag>(extraflags)));
        if (!ok) {
            qCDebug(diPlayer, "Don't play any animations, from mode is \"%s\", to mode is \"%s\"",
                    qPrintable(iconModeToString(job.from)), qPrintable(iconModeToString(job.to)));
            animationJobs.removeFirst();
        } // else: the job will remove on animation finished
    }
    void playToQueue();
    inline bool start(DDciIcon::Mode forMode, qreal speed = 1.0, DDciIconImagePlayer::Flags flags = {}) {
        qCDebug(diPlayer) << "Start animation for" << iconModeToString(forMode);
        const bool ok = player->start(speed, flags);
        if (ok && forMode == DDciIcon::Hover && !flags.testFlag(DDciIconImagePlayer::InvertedOrder)) {
            // When from Pressed mode to Hover mode, needs stop at the hover mode animation last frame.
            saveImageForHoverModeOnFinished = true;
        }

        if (ok) {
            setState(DDciIconPlayer::Busy);
        } else if (diPlayer().isDebugEnabled()) {
            qCDebug(diPlayer, "Failed on start animation for \"%s\"", qPrintable(iconModeToString(forMode)));
        }

        return ok;
    }

    inline void reset() {
        if (player && player->state() != DDciIconImagePlayer::NotRunning) {
            player->stop();
        }
        normal = DDciIconImage();
        hover = DDciIconImage();
        pressed = DDciIconImage();
        disabled = DDciIconImage();
        hoverModeLastImage = QImage();
    }
    inline bool isValid() const {
        return !normal.isNull();
    }
    inline void setImage(const QImage &image) {
        this->image = image;
        Q_EMIT q_func()->updated();
    }
    inline void setImage(DDciIcon::Mode mode) {
        auto &image = getImage(mode);
        if (image.atBegin()) {
            setImage(image.toImage(player->palette()));
        } else {
            setImage(icon.pixmap(devicePixelRatio, iconSize, theme, mode, player->palette()).toImage());
        }
    }
    inline void setFinishedImage(DDciIcon::Mode mode) {
        finishedImage = icon.pixmap(devicePixelRatio, iconSize, theme, mode, player->palette()).toImage();
    }
    inline const DDciIconImage &getImage(DDciIcon::Mode mode) const {
        if (mode == DDciIcon::Hover) {
            return hover;
        } else if (mode == DDciIcon::Pressed) {
            return pressed;
        } else if (mode == DDciIcon::Disabled) {
            return disabled;
        }

        Q_ASSERT(mode == DDciIcon::Normal);
        return normal;
    }

    inline void setState(DDciIconPlayer::State newState) {
        if (state == newState)
            return;
        state = newState;
        Q_EMIT q_func()->stateChanged();
    }

    DDciIconPlayer::State state = DDciIconPlayer::Idle;

    DDciIcon icon;
    DDciIcon::Theme theme = DDciIcon::Light;
    DDciIcon::Mode mode = DDciIcon::Normal;
    DDciIcon::Mode lastMode = DDciIcon::Normal;
    int iconSize = -1;
    qreal devicePixelRatio = 1.0;

    DDciIconImage normal;
    DDciIconImage hover;
    DDciIconImage pressed;
    DDciIconImage disabled;
    DDciIconImagePlayer *player = nullptr;

    struct Animation {
        DDciIcon::Mode from;
        DDciIcon::Mode to;
    };
    QVector<Animation> animationJobs;

    bool saveImageForHoverModeOnFinished = false;

    QImage image;
    QImage finishedImage;
    QImage hoverModeLastImage;

    D_DECLARE_PUBLIC(DDciIconPlayer)
};

void DDciIconPlayerPrivate::ensureInit()
{
    initPlayer();

    if (!normal.isNull())
        return;
    if (icon.isNull())
        return;
    normal = icon.image(icon.matchIcon(iconSize, theme, DDciIcon::Normal, DDciIcon::DontFallbackMode), iconSize, devicePixelRatio);
    hover = icon.image(icon.matchIcon(iconSize, theme, DDciIcon::Hover, DDciIcon::DontFallbackMode), iconSize, devicePixelRatio);
    pressed = icon.image(icon.matchIcon(iconSize, theme, DDciIcon::Pressed, DDciIcon::DontFallbackMode), iconSize, devicePixelRatio);
    disabled = icon.image(icon.matchIcon(iconSize, theme, DDciIcon::Disabled, DDciIcon::DontFallbackMode), iconSize, devicePixelRatio);
}

void DDciIconPlayerPrivate::initPlayer()
{
    if (player)
        return;
    player = new DDciIconImagePlayer(q_func());

    D_Q(DDciIconPlayer);
    QObject::connect(player, &DDciIconImagePlayer::updated, q, [this]() {
        setImage(player->readImage());
    });
    QObject::connect(player, &DDciIconImagePlayer::finished, q, [q, this]() {
        qCDebug(diPlayer, "Current animation finished!");

        if (saveImageForHoverModeOnFinished) {
            saveImageForHoverModeOnFinished = false;
            hoverModeLastImage = image;
        }

        if (!animationJobs.isEmpty()) {
            // Remove the finished animation
            animationJobs.removeFirst();

            qCDebug(diPlayer, "Number of animations remaining is %i", animationJobs.size());
            if (!animationJobs.isEmpty()) {
                _q_playFromQueue();
                return;
            }
        }
        if (!finishedImage.isNull()) {
            setImage(finishedImage);
            finishedImage = QImage();
        }

        if (mode == DDciIcon::Normal || mode == DDciIcon::Disabled) {
            player->clearCache();
        }

        setState(DDciIconPlayer::Idle);
    });
}

bool DDciIconPlayerPrivate::ensureHoverModeLastImage()
{
    if (!hoverModeLastImage.isNull())
        return true;
    if (!hover.supportsAnimation())
        return false;
    DDciIconImage image;
    const bool currentHoverImageIsUsable = hover.atEnd() || !player
            || player->state() != DDciIconImagePlayer::NotRunning;
    if (!currentHoverImageIsUsable) {
        if (icon.isNull())
            return false;
        image = icon.image(icon.matchIcon(iconSize, theme, DDciIcon::Hover, DDciIcon::DontFallbackMode),
                           iconSize, devicePixelRatio);
    }
    if (image.isNull())
        return false;

    while (!image.atEnd()) {
        if (!image.jumpToNextImage())
            break;
    }
    if (!image.atEnd())
        return false;
    Q_ASSERT(player);
    hoverModeLastImage = image.toImage(player->palette());
    return !hoverModeLastImage.isNull();
}

void DDciIconPlayerPrivate::play(DDciIcon::Mode mode, DDciIconImagePlayer::Flags flags)
{
    if (diPlayer().isDebugEnabled()) {
        qCDebug(diPlayer) << "Immediate play animation for" << iconModeToString(mode);
    }

    ensureInit();
    if (normal.isNull())
        return;

    // Ensure the old animations clean
    animationJobs.clear();
    player->stop();
    finishedImage = QImage();

    const DDciIconImage &image = getImage(mode);
    if (!image.supportsAnimation())
        return;

    player->setImages({image});
    start(mode, 1.0, flags);
}

bool DDciIconPlayerPrivate::play(DDciIcon::Mode fromMode, DDciIcon::Mode toMode,
                                 DDciIconImagePlayer::Flags extraflags)
{
    ensureInit();
    finishedImage = QImage();

    if (normal.isNull()) {
        setImage(QImage());
        return false;
    }

    if (fromMode == DDciIcon::Normal) {
        if (toMode == DDciIcon::Normal) {
            setImage(DDciIcon::Normal);
        } else if (toMode == DDciIcon::Hover) {
            if (hover.isNull()) {
                return false;
            }

            if (hover.supportsAnimation()) {
                player->setImages({hover});
                return start(toMode, 1.0, DDciIconImagePlayer::CacheAll | extraflags);
            } else {
                setImage(DDciIcon::Hover);
            }
        } else if (toMode == DDciIcon::Pressed) {
            if (pressed.isNull()) {
                return false;
            }

            if (pressed.supportsAnimation()) {
                if (hover.supportsAnimation()) {
                    player->setImages({hover, pressed});
                    return start(toMode, 2.0, DDciIconImagePlayer::CacheAll | extraflags);
                } else {
                    player->setImages({pressed});
                    return start(toMode, 1.0, DDciIconImagePlayer::CacheAll | extraflags);
                }
            } else {
                setImage(DDciIcon::Pressed);
            }
        } else if (toMode == DDciIcon::Disabled) {
            if (disabled.isNull())
                return false;

            if (disabled.supportsAnimation()) {
                player->setImages({disabled});
                return start(toMode, 1.0, DDciIconImagePlayer::IgnoreLastImageLoop | extraflags);
            } else {
                setImage(DDciIcon::Disabled);
            }
        }
    } else if (fromMode == DDciIcon::Hover) {
        Q_ASSERT(toMode != DDciIcon::Hover);

        if (toMode == DDciIcon::Normal) {
            if (hover.supportsAnimation()) {
                setFinishedImage(DDciIcon::Normal);
                player->setImages({hover});
                return start(toMode, 1.0, DDciIconImagePlayer::InvertedOrder
                             | DDciIconImagePlayer::IgnoreLastImageLoop
                             | DDciIconImagePlayer::ClearCacheOnStop
                             | extraflags);
            } else {
                setImage(DDciIcon::Normal);
            }
        } else if (toMode == DDciIcon::Hover) {
        } else if (toMode == DDciIcon::Pressed) {
            if (pressed.isNull()) {
                if (hover.supportsAnimation()) {
                    player->setImages({hover});
                    return start(toMode, 1.0, DDciIconImagePlayer::InvertedOrder
                                 | DDciIconImagePlayer::IgnoreLastImageLoop
                                 | extraflags);
                } else {
                    setImage(DDciIcon::Normal);
                }
                return false;
            }

            if (pressed.supportsAnimation()) {
                player->setImages({pressed});
                return start(toMode, 1.0, DDciIconImagePlayer::CacheAll | extraflags);
            } else {
                setImage(DDciIcon::Pressed);
            }
        } else if (toMode == DDciIcon::Disabled) {
            if (disabled.isNull()) {
                setImage(DDciIcon::Normal);
                return false;
            }

            if (disabled.supportsAnimation()) {
                player->setImages({disabled});
                return start(toMode, 1.0, DDciIconImagePlayer::IgnoreLastImageLoop | extraflags);
            } else {
                setImage(DDciIcon::Disabled);
            }
        }
    } else if (fromMode == DDciIcon::Pressed) {
        Q_ASSERT(toMode != DDciIcon::Pressed);

        if (toMode == DDciIcon::Normal) {
            if (!pressed.supportsAnimation()) {
                setImage(DDciIcon::Normal);
                return false;
            }

            setFinishedImage(DDciIcon::Normal);
            if (hover.supportsAnimation()) {
                player->setImages({hover, pressed});
                return start(toMode, 2.0, DDciIconImagePlayer::InvertedOrder
                             | DDciIconImagePlayer::IgnoreLastImageLoop
                             | extraflags);
            } else {
                player->setImages({pressed});
                return start(toMode, 1.0, DDciIconImagePlayer::InvertedOrder
                             | DDciIconImagePlayer::IgnoreLastImageLoop
                             | extraflags);
            }
        } else if (toMode == DDciIcon::Hover) {
            if (pressed.isNull()) {
                if (hover.supportsAnimation()) {
                    player->setImages({hover});
                    return start(toMode, 1.0, DDciIconImagePlayer::CacheAll | extraflags);
                } else {
                    setImage(DDciIcon::Hover);
                }
                return false;
            }

            if (pressed.supportsAnimation()) {
                ensureHoverModeLastImage();
                finishedImage = hoverModeLastImage;
                player->setImages({pressed});
                return start(toMode, 1.0, DDciIconImagePlayer::InvertedOrder
                             | DDciIconImagePlayer::IgnoreLastImageLoop
                             | extraflags);
            } else {
                setImage(DDciIcon::Hover);
            }
        } else if (toMode == DDciIcon::Pressed) {
        } else if (toMode == DDciIcon::Disabled) {
            if (disabled.isNull()) {
                setImage(DDciIcon::Normal);
                return false;
            }

            if (disabled.supportsAnimation()) {
                player->setImages({disabled});
                return start(toMode, 1.0, DDciIconImagePlayer::IgnoreLastImageLoop | extraflags);
            } else {
                setImage(DDciIcon::Disabled);
            }
        }
    } else if (fromMode == DDciIcon::Disabled) {
        Q_ASSERT(toMode != DDciIcon::Disabled);

        if (disabled.supportsAnimation()) {
            setFinishedImage(DDciIcon::Normal);
            player->setImages({disabled});
            return start(toMode, 1.0, DDciIconImagePlayer::InvertedOrder
                         | DDciIconImagePlayer::IgnoreLastImageLoop
                         | extraflags);
        } else {
            setImage(DDciIcon::Normal);
        }
    }

    return false;
}

void DDciIconPlayerPrivate::playToQueue()
{
    DDciIconImagePlayer::Flags extraFlags = {};

    if (diPlayer().isDebugEnabled()) {
        qCDebug(diPlayer, "Request play animation in queue, from mode is \"%s\", to mode is \"%s\"",
                qPrintable(iconModeToString(lastMode)), qPrintable(iconModeToString(mode)));
    }

    if (!animationJobs.isEmpty()) {
        if (diPlayer().isDebugEnabled()) {
            qCDebug(diPlayer, "Old Animation Queue:");
            for (int i = 0; i < animationJobs.count(); ++i) {
                const Animation &job = animationJobs.at(i);
                qCDebug(diPlayer, "    %d. from mode is \"%s\", to mode is \"%s\"", i + 1,
                        qPrintable(iconModeToString(job.from)), qPrintable(iconModeToString(job.to)));
            }
        }

        // Ignore the same request
        if (animationJobs.last().from == lastMode && animationJobs.last().to == mode) {
            qCDebug(diPlayer, "Same as the last animation, ignores this request");
            return;
        }

        if (animationJobs.last().from == mode && animationJobs.last().to == lastMode) {
            if (animationJobs.size() > 1) {
                animationJobs.removeLast();
                qCDebug(diPlayer, "Offsets the last unplayed animation, discards the animation, and ignores this request");
                return;
            } else {
                if (player && player->state() != DDciIconImagePlayer::NotRunning) {
                    extraFlags |= DDciIconImagePlayer::Continue;
                    player->stop();
                    animationJobs.removeFirst();
                }
            }
        }
    } else {
        if (player)
            player->stop();
        qCDebug(diPlayer, "Old Animation queue is empty");
    }

    animationJobs.append({lastMode, mode});

    if (player && player->state() != DDciIconImagePlayer::NotRunning) {
        player->abortLoop();
        Q_ASSERT(!player->currentLoopForever());

        // Don't stop current animation, wating play to end.
        // Ensure the continuity of the animation when multiple states change continuously.
        // An example is changing from the Normal state to the Hover state and then to the
        // Pressed state in a short period of time. At this time, the animation in the
        // hover state may not have finished playing.
        qCDebug(diPlayer, "Wait the current animation finished to continue play the new animation.");
        return;
    }

    QMetaObject::invokeMethod(q_func(), "_q_playFromQueue", Qt::QueuedConnection, Q_ARG(int, extraFlags));
}

DDciIconPlayer::DDciIconPlayer(QObject *parent)
    : QObject(parent)
    , DObject(*new DDciIconPlayerPrivate(this))
{

}

DDciIconPlayer::State DDciIconPlayer::state() const
{
    D_DC(DDciIconPlayer);
    return d->state;
}

void DDciIconPlayer::setIcon(const DDciIcon &icon)
{
    D_D(DDciIconPlayer);
    d->icon = icon;
    d->reset();
    d->playToQueue();
}

DDciIcon DDciIconPlayer::icon() const
{
    D_DC(DDciIconPlayer);
    return d->icon;
}

void DDciIconPlayer::setTheme(DDciIcon::Theme theme)
{
    D_D(DDciIconPlayer);
    if (d->theme == theme)
        return;
    d->theme = theme;
    d->reset();
    d->playToQueue();
}

DDciIcon::Theme DDciIconPlayer::theme() const
{
    D_DC(DDciIconPlayer);
    return d->theme;
}

void DDciIconPlayer::setMode(DDciIcon::Mode mode)
{
    D_D(DDciIconPlayer);
    if (d->mode == mode)
        return;
    d->lastMode = d->mode;
    d->mode = mode;
    Q_EMIT modeChanged(d->lastMode, mode);

    if (diPlayer().isDebugEnabled()) {
        qCDebug(diPlayer) << this
                          << "Old Mode:" << iconModeToString(d->lastMode)
                          << "New Mode" << iconModeToString(d->mode);
    }

    if (mode == DDciIcon::Disabled)
        abort();
    d->playToQueue();
}

DDciIcon::Mode DDciIconPlayer::mode() const
{
    D_DC(DDciIconPlayer);
    return d->mode;
}

void DDciIconPlayer::setIconSize(int size)
{
    D_D(DDciIconPlayer);
    if (d->iconSize == size)
        return;
    d->iconSize = size;
    d->reset();
    d->playToQueue();
}

int DDciIconPlayer::iconSize() const
{
    D_DC(DDciIconPlayer);
    return d->iconSize;
}

void DDciIconPlayer::setDevicePixelRatio(qreal devicePixelRatio)
{
    D_D(DDciIconPlayer);
    if (qFuzzyCompare(d->devicePixelRatio, devicePixelRatio))
        return;
    d->devicePixelRatio = devicePixelRatio;
    d->reset();
    d->playToQueue();
}

qreal DDciIconPlayer::devicePixelRatio() const
{
    D_DC(DDciIconPlayer);
    return d->devicePixelRatio;
}

void DDciIconPlayer::setPalette(const DDciIconPalette &palette)
{
    D_D(DDciIconPlayer);
    d->initPlayer();
    if (d->player->setPalette(palette)) {
        if (d->hover.hasPalette())
            d->hoverModeLastImage = QImage();

        const auto &image = d->getImage(d->mode);
        if (image.hasPalette())
            d->playToQueue();
    }
}

QImage DDciIconPlayer::currentImage() const
{
    D_DC(DDciIconPlayer);
    return d->image;
}

void DDciIconPlayer::play(DDciIcon::Mode mode)
{
    D_D(DDciIconPlayer);
    d->play(mode, DDciIconImagePlayer::IgnoreLastImageLoop);
}

void DDciIconPlayer::stop()
{
    D_D(DDciIconPlayer);
    if (d->player)
        d->player->stop();
    d->setState(Idle);
}

void DDciIconPlayer::abort()
{
    D_D(DDciIconPlayer);
    d->animationJobs.clear();
    if (d->player)
        d->player->stop();
    d->setState(Idle);
}

DGUI_END_NAMESPACE

#include "moc_ddciiconplayer.cpp"
