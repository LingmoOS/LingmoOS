// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDESHOWCONFIG_H
#define SLIDESHOWCONFIG_H

#include <QObject>
#include <QSettings>
#include <QScopedPointer>
#include <DConfig>

DCORE_USE_NAMESPACE

static const char *const kGroupSlideshowPath = "custom_screensaver.path.select_path";
static const char *const kGroupIntervalTime = "custom_screensaver.slideshow.time_interval";
static const char *const kGroupShuffle = "custom_screensaver.slideshow.shuffle";
static const char *const kKeyValue = "value";

static const char *const kDeepinScreenSaver = "deepin-screensaver";

static const int kMaxIntervalTime = 3600;
static const int kMinIntervalTime = 3;
static const int kDefaultTime = 10;

static constexpr char kCfgAppId[] { "org.deepin.screensaver" };
static constexpr char kCfgName[] { "org.deepin.customscreensaver" };

static constexpr char kSlideshowPath[] { "slideshowPath" };
static constexpr char kIntervalTime[] { "intervalTime" };
static constexpr char kPlayMode[] { "playMode" };

class SlideshowConfig : public QObject
{
    Q_OBJECT

public:
    enum PlayMode {
        Order = 0,
        Shuffle = 1
    };

    static SlideshowConfig *instance();

    ~SlideshowConfig();

    QString slideshowPath() const;
    void setSlideShowPath(QString path);
    void setIntervalTime(int tempTime);
    int intervalTime() const;

    bool isShuffle() const;
    void setShuffle(const bool shuffle);

    bool startCustomConfig();
    static QString defaultPath();
private:
    QString confPath();

protected:
    explicit SlideshowConfig(QObject *parent = nullptr);

private:
    SlideshowConfig(SlideshowConfig &) = delete;
    SlideshowConfig &operator=(SlideshowConfig &) = delete;

private:
    QScopedPointer<QSettings> m_settings;
    DConfig *m_dCfg;
};

#endif   // SLIDESHOWCONFIG_H
