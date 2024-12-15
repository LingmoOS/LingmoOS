// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTENGINE_H
#define FULLTEXTENGINE_H

#include <QObject>

namespace GrandSearch {
class FullTextEnginePrivate;
class FullTextEngine : public QObject
{
    Q_OBJECT
public:
    typedef bool (*CheckAndPushItem)(const QString &file, void *pdata, void *ctx);
public:
    explicit FullTextEngine(QObject *parent = nullptr);
    ~FullTextEngine();
    bool init(const QString &cache);
    void query(const QString &searchPath, const QStringList &keys, CheckAndPushItem func, void *pdata);
    QSet<QString> matchedKeys(void *ctx) const;
private:
    FullTextEnginePrivate *d;
};
}
#endif // FULLTEXTENGINE_H
