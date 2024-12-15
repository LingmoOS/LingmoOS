// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"
using namespace dmr;

Window::Window(QWidget *parent) : QWidget (parent)
{

    auto l = new QVBoxLayout(this);

    //if (dmr::CompositingManager::get().composited()) {
    //dmr::CompositingManager::get().overrideCompositeMode(false);
    //}

    player = new dmr::PlayerWidget;
    l->addWidget(player);

    QObject::connect(&player->engine(), &dmr::PlayerEngine::stateChanged, [=]() {
        qInfo() << "----------------new state: " << player->engine().state();
    });
    player->engine().changeVolume(120);

    auto h = new QHBoxLayout(this);

    auto playBtn = new QPushButton("Play");
    connect(playBtn, &QPushButton::clicked, &player->engine(), &dmr::PlayerEngine::play);
    h->addWidget(playBtn);

    auto pauseBtn = new QPushButton("Pause");
    connect(pauseBtn, &QPushButton::clicked, &player->engine(), &dmr::PlayerEngine::pauseResume);
    h->addWidget(pauseBtn);

    auto stopBtn = new QPushButton("stop");
    connect(stopBtn, &QPushButton::clicked, &player->engine(), &dmr::PlayerEngine::stop);
    h->addWidget(stopBtn);

    auto forward = new QPushButton("forward");
    connect(forward, &QPushButton::clicked, [=]() {
        player->engine().seekForward(60);
    });
    h->addWidget(forward);

    auto volumeUp = new QPushButton("volUp");
    connect(volumeUp, &QPushButton::clicked, &player->engine(), &dmr::PlayerEngine::volumeUp);
    h->addWidget(volumeUp);

    auto volumeDown = new QPushButton("volDown");
    connect(volumeDown, &QPushButton::clicked, &player->engine(), &dmr::PlayerEngine::volumeDown);
    h->addWidget(volumeDown);

    auto keep = new QPushButton("KeepOpen");
    connect(keep, &QPushButton::clicked, &player->engine(), [this]() {
        this->player->engine().setBackendProperty("keep-open", "yes");
    });
    this->player->engine().setBackendProperty("pause-on-start", "true");
    h->addWidget(keep);

    l->addLayout(h);
    setLayout(l);

}

void Window::play(const QUrl& url) {
    if (player->engine().isPlayableFile(url))
        player->play(url);
}
