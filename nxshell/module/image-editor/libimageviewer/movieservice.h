// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVIESERVICE_H
#define MOVIESERVICE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QFileInfo>
#include <QMutex>
#include <QDateTime>
#include <deque>
#include <QStandardPaths>
#include <mutex>
#include <QDir>
#include <QJsonObject>

#include "image-viewer_global.h"

struct MovieInfo {
    bool valid = false;
    QString filePath = "-";  //文件路径
    QString fileType = "-";  //文件类型
    QString resolution = "-";//分辨率
    QDateTime creation;      //创建时间
    qint64 fileSize = 0;     //文件大小
    QString duration = "-";  //视频长度

    //视频流信息
    QString vCodecID = "-";  //编码格式
    qint64 vCodeRate = 0;    //码率
    int fps = 0;             //帧率
    double proportion = -1;  //长宽比

    //音频流信息
    QString aCodeID = "-"; //编码格式
    qint64 aCodeRate = 0;  //码率
    QString aDigit = "-";  //数据格式
    int channels = 0;      //通道数
    int sampling = 0;      //采样率

    QString sizeStr() const
    {
        auto K = 1024;
        auto M = 1024 * K;
        auto G = 1024 * M;
        if (fileSize > G) {
            return QString("%1G").arg((double)fileSize / G, 0, 'f', 1);
        } else if (fileSize > M) {
            return QString("%1M").arg((double)fileSize / M, 0, 'f', 1);
        } else if (fileSize > K) {
            return QString("%1K").arg((double)fileSize / K, 0, 'f', 1);
        }
        return QString("%1").arg(fileSize);
    }
};

class IMAGEVIEWERSHARED_EXPORT MovieService: public QObject
{
    Q_OBJECT
public:
    static MovieService *instance(QObject *parent = nullptr);

    //获取视频信息
    MovieInfo getMovieInfo(const QUrl &url);

    //获取视频信息，基于JSON的可扩展版本
    QJsonObject getMovieInfoByJson(const QUrl &url);

    //获取视频首帧图片
    QImage getMovieCover(const QUrl &url, const QString &savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator());

private:
    explicit MovieService(QObject *parent = nullptr);
    struct MovieInfo parseFromFile(const QFileInfo &fi);

private:
    QImage getMovieCover_ffmpegthumbnailer(const QUrl &url, const QString &bufferPath);
    MovieInfo getMovieInfo_ffmpeg(const QFileInfo &fi);
    MovieInfo getMovieInfo_mediainfo(const QFileInfo &fi);
    QImage getMovieCover_ffmpegthumbnailerlib(const QUrl &url);

    QMutex m_queuqMutex;
    static MovieService *m_movieService;
    static std::once_flag instanceFlag;
    bool m_ffmpegExist = false;
    bool m_ffmpegthumbnailerExist = false;
    QMutex m_bufferMutex;
    std::deque<std::pair<QUrl, MovieInfo>> m_movieInfoBuffer;
    bool m_ffmpegThumLibExist = false;

    //ffmpeg用
    QString resolutionPattern;
    QString codeRatePattern;
    QString fpsPattern;
};

//C-Style API
extern "C" {
    void getMovieCover(const QUrl &url, const QString &savePath, QImage *imageRet);
    void getMovieInfoByJson(const QUrl &url, QJsonObject *jsonRet);
}

#endif // MOVIESERVICE_H
