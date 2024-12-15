// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videoproxy.h"

#include "dfm-base/dfm_desktop_defines.h"

#include <QPaintEvent>
#include <QPainter>

using namespace ddplugin_videowallpaper;
DFMBASE_USE_NAMESPACE

#ifdef USE_LIBDMR

VideoProxy::VideoProxy(QWidget *parent) : dmr::PlayerWidget(parent)
{
    dmr::PlayerEngine &eng = engine();
    eng.setMute(true);

    auto pal = palette();
    pal.setBrush(backgroundRole(), Qt::black);
    setPalette(pal);

    // do not decode audio
    _engine->setBackendProperty("ao", "no");
    _engine->setBackendProperty("color", QVariant::fromValue(QColor(Qt::black)));
    _engine->setBackendProperty("keep-open", "yes");
    _engine->setBackendProperty("dmrhwdec-switch", true);

    connect(_engine, &dmr::PlayerEngine::stateChanged, this, &VideoProxy::playNext);
}

VideoProxy::~VideoProxy()
{
    stop();
}

void VideoProxy::setPlayList(const QList<QUrl> &list)
{
    playList = list;
    if (list.contains(current))
        return;

    _engine->stop();
    _engine->getplaylist()->clear();

    play();
}

void VideoProxy::play()
{
    if (playList.isEmpty())
        return;

    QUrl next;
    if (playList.contains(current))
        next = current;
    else
        next = playList.first();

    run = true;
    current = next;
    PlayerWidget::play(next);

    QString hd =_engine->getBackendProperty("hwdec").toString();
    fmDebug() << "play" << next << "hardward decode" << hd;
}

void VideoProxy::stop()
{
    run = false;
    _engine->stop();
}

void VideoProxy::playNext()
{
    auto stat = _engine->state();
    if (run && stat != dmr::PlayerEngine::Playing) {
        if (playList.isEmpty()) {
            _engine->getplaylist()->clear();
            current.clear();
            return;
        }

        // 循环播放
        if (playList.size() == 1 && stat == dmr::PlayerEngine::Paused) {
            engine().seekAbsolute(0);
            engine().pauseResume();
            return;
        }

        // 播放下一个
        int idx = playList.indexOf(current);
        if (idx < 0 || idx >= playList.size() - 1)
            idx = 0;
        else
            idx++;

        _engine->getplaylist()->clear();
        current = playList.at(idx);
        PlayerWidget::play(current);

        QString hd =_engine->getBackendProperty("hwdec").toString();
        fmDebug() << "play" << current << "hardward decode" << hd;
    }
}

#else

VideoProxy::VideoProxy(QWidget *parent) : QWidget(parent)
{
    auto pal = palette();
    pal.setColor(backgroundRole(), Qt::black);
    setPalette(pal);
    setAutoFillBackground(false);
}

VideoProxy::~VideoProxy()
{

}

void VideoProxy::updateImage(const QImage &img)
{
    image = img.scaled(size() * devicePixelRatioF(), Qt::KeepAspectRatio);
    image.setDevicePixelRatio(devicePixelRatioF());
    update();
}

void VideoProxy::paintEvent(QPaintEvent *e)
{
    QPainter pa(this);
    auto fill = QRect(QPoint(0,0), size());
    pa.fillRect(fill, palette().background());

    if (image.isNull())
        return;
    QSize tar = image.size() / devicePixelRatioF();

    int x = (fill.width() - tar.width()) / 2.0;
    int y = (fill.height() - tar.height()) / 2.0;
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;

    pa.drawImage(x, y, image);
}

#endif
