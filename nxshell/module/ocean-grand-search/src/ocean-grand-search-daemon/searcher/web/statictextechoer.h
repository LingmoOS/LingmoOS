// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATICTEXTECHOER_H
#define STATICTEXTECHOER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class StaticTextEchoer : public Searcher
{
public:
    explicit StaticTextEchoer(QObject *parent = nullptr);
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
};

}

#endif // STATICTEXTECHOER_H
