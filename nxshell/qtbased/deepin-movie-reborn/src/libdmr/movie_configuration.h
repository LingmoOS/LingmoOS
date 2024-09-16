// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_MOVIE_CONFIGURATION_H
#define _DMR_MOVIE_CONFIGURATION_H 

#include <QtCore>

namespace dmr 
{
class MovieConfigurationBackend;


class MovieConfiguration: public QObject 
{
    Q_OBJECT
public:
    enum KnownKey {
        SubDelay,
        SubCodepage,
        SubId,
        StartPos,
        ExternalSubs
    };

    static MovieConfiguration& get();
    void init(); // call once

    void removeUrl(const QUrl& url);
    void clear();
    bool urlExists(const QUrl& url);
    void updateUrl(const QUrl& url, const QString& key, const QVariant& val);
    void updateUrl(const QUrl& url, KnownKey key, const QVariant& val);
    //helper for update list type entries
    void append2ListUrl(const QUrl& url, KnownKey key, const QString& val);
    void removeFromListUrl(const QUrl& url, KnownKey key, const QString& val);

    //list all settings for url
    QMap<QString, QVariant> queryByUrl(const QUrl& url);

    QVariant getByUrl(const QUrl& url, const QString& key);
    QVariant getByUrl(const QUrl& url, KnownKey key);
    //helper for get list type entries
    QStringList getListByUrl(const QUrl& url, KnownKey key);

    //helper
    QStringList decodeList(const QVariant& val);

    ~MovieConfiguration();
    static QString knownKey2String(KnownKey kk);

private:
    MovieConfiguration();

    MovieConfigurationBackend* _backend {nullptr};
};

using ConfigKnownKey = MovieConfiguration::KnownKey;
}

#endif /* ifndef _DMR_MOVIE_CONFIGURATION_H */

