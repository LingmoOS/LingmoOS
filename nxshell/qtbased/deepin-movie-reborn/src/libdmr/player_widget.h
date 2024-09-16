// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QtWidgets>

namespace dmr {
class PlayerEngine;

class PlayerWidget: public QWidget {
    Q_OBJECT
public:
    explicit PlayerWidget(QWidget *parent = nullptr);
    virtual ~PlayerWidget();

    /**
     * engine is instantiated in constructor, and all interaction comes from
     * engine
     */
    PlayerEngine& engine();
    void play(const QUrl& url);

protected:
    PlayerEngine *_engine {nullptr};
};
}

