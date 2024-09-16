// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

int main(int argc, char *argv[])
{
    //dmr::CompositingManager::detectOpenGLEarly();
    QApplication app(argc, argv);

    // required by mpv
    setlocale(LC_NUMERIC, "C");
    dmr::Backend::setDebugLevel(dmr::Backend::DebugLevel::Debug);

    auto mw = new Window;
    mw->resize(400, 300);
    mw->show();

    if (argc == 2)
        mw->play(QString::fromUtf8(argv[1]));

    //api支持多线程demo
    QThread *thread = new QThread();
    dmr::PlaylistModel *playModel = new dmr::PlaylistModel(nullptr);
    playModel->moveToThread(thread);
    thread->start();
    bool is = false;
    //获取视频信息 demo
    dmr::MovieInfo mi = playModel->getMovieInfo(QUrl("file:///usr/share/dde-introduction/demo.mp4"),&is);
    if (is) {
        qInfo() << "# 文件类型:"<< mi.fileType
                << "# 文件大小:"<< mi.sizeStr()
                << "# 时长:" << mi.durationStr()
                << "# 文件路径:"<< mi.filePath
                << "# 视频流信息:" << mi.videoCodec()
                << "# 视频码率:"<< mi.vCodeRate << "bps"
                << "# 视频帧率:"<< mi.fps << "fps"
                << "# 视频显示比例:" << QString("%1").arg(static_cast<double>(mi.proportion))
                << "# 视频分辨率:"<< mi.resolution
                << "# 音频编码样式:"<< mi.audioCodec()
                << "# 音频编码码率:"<< mi.aCodeRate << "bps"
                << "# 音频位数:"<< mi.aDigit << "bits"
                << "# 声道数:"<< mi.channels << "声道"
                << "# 采样数:"<< mi.sampling << "hz";
        //获取预览图 demo
        QImage img = playModel->getMovieCover(QUrl("file:///usr/share/dde-introduction/demo.mp4"));
        img.save(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Desktop/test.png");

        QImage img1 = playModel->getMovieCover(QUrl("file:///usr/share/dde-introduction/demo.mp4"));
        img1.save(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Desktop/test1.png");
    }
    playModel->deleteLater();
    app.exec();

    delete mw;
    return 0;
}
