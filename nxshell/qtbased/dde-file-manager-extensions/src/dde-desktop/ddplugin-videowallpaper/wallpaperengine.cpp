// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperengine_p.h"
#include "util/ddpugin_eventinterface_helper.h"
#include "wallpaperconfig.h"
#include "videowallpapermenuscene.h"

#include "util/menu_eventinterface_helper.h"

#include "dfm-base/dfm_desktop_defines.h"

#ifndef USE_LIBDMR
#include <QtMultimedia/QMediaContent>
#endif

#include <QDir>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDebug>

using namespace ddplugin_videowallpaper;
DFMBASE_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);


static QString getScreenName(QWidget *win)
{
    return win->property(DesktopFrameProperty::kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> ret;
    for (QWidget *win : root) {
        QString name = getScreenName(win);
        if (name.isEmpty())
            continue;
        ret.insert(name, win);
    }

    return ret;
}


WallpaperEnginePrivate::WallpaperEnginePrivate(WallpaperEngine *qq)
    : q(qq)
{

}

#ifdef USE_LIBDMR
QList<QUrl> WallpaperEnginePrivate::getVideos(const QString &path)
{
    QList<QUrl> ret;
    QDir dir(path);
    for (const QFileInfo &file : dir.entryInfoList(QDir::Files))
        ret << QUrl::fromLocalFile(file.absoluteFilePath());
    return ret;
}
#else
QList<QMediaContent> WallpaperEnginePrivate::getVideos(const QString &path)
{
    QList<QMediaContent> ret;
    QDir dir(path);
    for (const QFileInfo &file : dir.entryInfoList(QDir::Files))
        ret << QMediaContent(QUrl::fromLocalFile(file.absoluteFilePath()));

    return ret;
}
#endif

VideoProxyPointer WallpaperEnginePrivate::createWidget(QWidget *root)
{
    const QString screenName = getScreenName(root);
    VideoProxyPointer bwp(new VideoProxy());

    bwp->setParent(root);
    QRect geometry = relativeGeometry(root->geometry());   // scaled area
    bwp->setGeometry(geometry);

    bwp->setProperty(DesktopFrameProperty::kPropScreenName, getScreenName(root));
    bwp->setProperty(DesktopFrameProperty::kPropWidgetName, "videowallpaper");
    bwp->setProperty(DesktopFrameProperty::kPropWidgetLevel, 5.1);

    fmDebug() << "screen name" << screenName << "geometry" << root->geometry() << bwp.get();
    return bwp;
}

void WallpaperEnginePrivate::setBackgroundVisible(bool v)
{
    QList<QWidget *> roots = ddplugin_desktop_util::desktopFrameRootWindows();
    for (QWidget *root  : roots) {
        for (QObject *obj : root->children()) {
            if (QWidget *wid = dynamic_cast<QWidget *>(obj)) {
                QString type = wid->property(DesktopFrameProperty::kPropWidgetName).toString();
                if (type == "background") {
                    wid->setVisible(v);
                }
            }
        }
    }
}

QString WallpaperEnginePrivate::sourcePath() const
{
    QString path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first() + "/video-wallpaper";
    return path;
}

WallpaperEngine::WallpaperEngine(QObject *parent)
    : QObject(parent)
    , d(new WallpaperEnginePrivate(this))
{

}

WallpaperEngine::~WallpaperEngine()
{
    turnOff();
}

bool WallpaperEngine::init()
{
    WpCfg->initialize();

    QFileInfo source(d->sourcePath());
    if (!source.exists()) {
        source.absoluteDir().mkpath(source.fileName());
    }
    fmInfo() << "the wallpaper resource is in" << source.absoluteFilePath();

    if (!registerMenu()) {
        // waiting canvas menu
        dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &WallpaperEngine::registerMenu);
    }
    connect(WpCfg, &WallpaperConfig::checkResource, this, &WallpaperEngine::checkResouce);
    connect(WpCfg, &WallpaperConfig::changeEnableState, this, [this](bool e){
        if (WpCfg->enable() == e)
            return;
        WpCfg->setEnable(e);
        if (e) {
            turnOn();
            play();
        } else
            turnOff();
    });

    if (WpCfg->enable())
        turnOn(false);

    return true;
}

void WallpaperEngine::turnOn(bool b)
{
    Q_ASSERT(d->watcher == nullptr);

    CanvasCoreSubscribe(signal_DesktopFrame_WindowShowed, &WallpaperEngine::play);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &WallpaperEngine::build);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &WallpaperEngine::geometryChanged);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &WallpaperEngine::onDetachWindows);

    d->watcher = new QFileSystemWatcher(this);
    {
        d->watcher->addPath(d->sourcePath());
        connect(d->watcher, &QFileSystemWatcher::directoryChanged, this, &WallpaperEngine::refreshSource);
    }
#ifndef USE_LIBDMR
    d->surface = new VideoSurface;

    d->player = new QMediaPlayer(nullptr, QMediaPlayer::LowLatency);

    connect(d->surface, &VideoSurface::pushImage, this, &WallpaperEngine::catchImage);

    d->player->setVideoOutput(d->surface);
    d->player->setMuted(true);
    d->playlist = new QMediaPlaylist(d->player);
#endif
    refreshSource();
    if (b) {
        build();
        show();
    }
}

void WallpaperEngine::turnOff()
{
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowShowed, &WallpaperEngine::play);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &WallpaperEngine::build);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &WallpaperEngine::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &WallpaperEngine::geometryChanged);

    delete d->watcher;
    d->watcher = nullptr;
#ifndef USE_LIBDMR
    d->player->pause();

    delete d->playlist;
    d->playlist = nullptr;

    delete d->player;
    d->player = nullptr;

    delete d->surface;
    d->surface = nullptr;
#endif
    d->widgets.clear();
    d->videos.clear();

    // show background.
    d->setBackgroundVisible(true);
}

void WallpaperEngine::refreshSource()
{
    d->videos = d->getVideos(d->sourcePath());

#ifndef USE_LIBDMR
    bool run = d->player->state() == QMediaPlayer::PlayingState;

    d->playlist->clear();
    d->playlist->addMedia(d->videos);
    if (!d->videos.isEmpty()) {
        d->playlist->setCurrentIndex(0);
        d->playlist->setPlaybackMode(QMediaPlaylist::Loop);
    }

    d->player->setPlaylist(d->playlist);

    if (run)
        d->player->play();
#else
    for (const VideoProxyPointer &bwp : d->widgets.values())
        bwp->setPlayList(d->videos);
#endif
}

void WallpaperEngine::build()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    if (root.size() == 1) {
        QWidget *primary = root.first();
        if (primary == nullptr) {
            // get screen failed,clear all widget
            d->widgets.clear();
            fmCritical() << "get primary screen failed return.";
            return;
        }

        const QString screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            fmWarning() << "can not get screen name from root window";
            return;
        }

        VideoProxyPointer bwp = d->widgets.value(screeName);
        d->widgets.clear();
        if (!bwp.isNull()) {
            bwp->setParent(primary);
            QRect geometry = d->relativeGeometry(primary->geometry());   // scaled area
            bwp->setGeometry(geometry);
        } else {
            bwp = d->createWidget(primary);
        }

        d->widgets.insert(screeName, bwp);
    } else {
        // check whether to add
        for (QWidget *win : root) {

            const QString screenName = getScreenName(win);
            if (screenName.isEmpty()) {
                fmWarning() << "can not get screen name from root window";
                continue;
            }

            VideoProxyPointer bwp = d->widgets.value(screenName);
            if (!bwp.isNull()) {
                // update widget
                bwp->setParent(win);
                QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
                bwp->setGeometry(geometry);
            } else {
                // add new widget
                fmInfo() << "screen:" << screenName << "  added, create it.";
                bwp = d->createWidget(win);
                d->widgets.insert(screenName, bwp);
            }
        }

        // clean up invalid widget
        {
            auto winMap = rootMap();
            for (const QString &sp : d->widgets.keys()) {
                if (!winMap.contains(sp)) {
                    d->widgets.take(sp);
                    fmInfo() << "remove screen:" << sp;
                }
            }
        }
    }
}

void WallpaperEngine::onDetachWindows()
{
    for (const VideoProxyPointer &bwp : d->widgets.values())
        bwp->setParent(nullptr);
}

void WallpaperEngine::geometryChanged()
{
    auto winMap = rootMap();
    for (auto itor = d->widgets.begin(); itor != d->widgets.end(); ++itor) {
        VideoProxyPointer bw = itor.value();
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            fmCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        if (bw.get() != nullptr) {
            QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
            bw->setGeometry(geometry);
        }
    }
}

void WallpaperEngine::play()
{
    if (WpCfg->enable()) {
#ifndef USE_LIBDMR
        d->player->play();
#else
        for (const VideoProxyPointer &bwp : d->widgets.values())
            bwp->setPlayList(d->videos);
#endif
        d->setBackgroundVisible(false);
        show();
    }
}

void WallpaperEngine::show()
{
    // relayout
    dpfSlotChannel->push("ddplugin_core", "slot_DesktopFrame_LayoutWidget");
    for (const VideoProxyPointer &bwp : d->widgets.values())
        bwp->show();
}

bool WallpaperEngine::registerMenu()
{
    if (dfmplugin_menu_util::menuSceneContains("CanvasMenu")) {
        // register menu for canvas
        dfmplugin_menu_util::menuSceneRegisterScene(VideoWallpaerMenuCreator::name(), new VideoWallpaerMenuCreator());
        dfmplugin_menu_util::menuSceneBind(VideoWallpaerMenuCreator::name(), "CanvasMenu");

        dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &WallpaperEngine::registerMenu);
        return true;
    } else
        return false;
}

void WallpaperEngine::checkResouce()
{
    if (d->videos.isEmpty()) {
       QString text = tr("Please add the video file to %0").arg(d->sourcePath());
       QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
       notify.setTimeout(1000);
       QDBusPendingReply<uint> p = notify.asyncCall(QString("Notify"),
                        QString("Video Wallpaper"),   // title
                        static_cast<uint>(0),
                        QString("deepin-toggle-desktop"),   // icon
                        text,
                        QString(), QStringList(), QVariantMap(), 5000);
    }
}

#ifndef USE_LIBDMR
void WallpaperEngine::catchImage(const QImage &img)
{
    for (const VideoProxyPointer &bwp : d->widgets.values())
        bwp->updateImage(img);
}
#endif
