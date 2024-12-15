// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPAPPSEARCHER_H
#define DESKTOPAPPSEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class DesktopAppSearcherPrivate;
class DesktopAppSearcher : public Searcher
{
    Q_OBJECT
    friend class DesktopAppSearcherPrivate;
public:
    explicit DesktopAppSearcher(QObject *parent = nullptr);
    ~DesktopAppSearcher();
    void asyncInit();
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
private slots:
    void onDirectoryChanged(const QString &path);
private:
    DesktopAppSearcherPrivate *d;
};

}

#endif // DESKTOPAPPSEARCHER_H
