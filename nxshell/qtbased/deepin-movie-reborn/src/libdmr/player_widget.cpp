// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "player_widget.h"
#include "filefilter.h"
#include <player_engine.h>

namespace dmr {

PlayerWidget::PlayerWidget(QWidget *parent)
    : QWidget (parent)
{
    utils::first_check_wayland_env();
    _engine = new PlayerEngine(this);
    auto *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(_engine);
    setLayout(l);
}

PlayerWidget::~PlayerWidget()
{
}

PlayerEngine &PlayerWidget::engine()
{
    return *_engine;
}

void PlayerWidget::play(const QUrl &url)
{
    QUrl realUrl;
    realUrl = FileFilter::instance()->fileTransfer(url.toString());

    if (!realUrl.isValid())
        return;

    if (!_engine->addPlayFile(realUrl)) {
        return;
    }
    _engine->playByName(realUrl);
}

}
