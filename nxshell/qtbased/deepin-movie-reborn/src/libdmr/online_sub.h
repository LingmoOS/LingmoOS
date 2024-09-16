// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DMR_ONLINE_SUB_H
#define _DMR_ONLINE_SUB_H 

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


namespace dmr {
struct ShooterSubtitleMeta {
    int id;     //error warning(cppcheck)
    int delay;      //error warning(cppcheck)
    QString desc;
    QString ext;
    QString link; // url to download
    QString local; // saved position when downloaded
};

class OnlineSubtitle: public QObject {
    Q_OBJECT
public:
    enum FailReason {
        NoError,
        NetworkError,
        NoSubFound,
        Duplicated,  // the same hash with local cache
    };

    static OnlineSubtitle& get();
    QString storeLocation();

public slots:
    void requestSubtitle(const QUrl& url);

private slots:
    void replyReceived(QNetworkReply*);  
    void downloadSubtitles();

signals:
    void subtitlesDownloadedFor(const QUrl& url, const QList<QString>& filenames, FailReason r);
    void onlineSubtitleStateChanged(const FailReason reason);

private:
    QString _defaultLocation;
    QNetworkAccessManager *_nam {nullptr};

    int _pendingDownloads {0}; // this should equal to _subs.size() basically
    QList<ShooterSubtitleMeta> _subs;
    QFileInfo _lastReqVideo;
    FailReason _lastReason {NoError};

    OnlineSubtitle();
    void subtitlesDownloadComplete();
    QString findAvailableName(const QString& tmpl, int id);
    bool hasHashConflict(const QString& path, const QString& tmpl, QString& conflictPath);
};
}

#endif /* ifndef _DMR_ONLINE_SUB_H */
