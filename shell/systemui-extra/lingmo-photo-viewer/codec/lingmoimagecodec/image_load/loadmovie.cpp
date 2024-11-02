#include "../loadmovie.h"

LoadMovie::LoadMovie(QList<Mat> *list, QMovie *movie)
{
    m_list = list;
    m_movie = movie;
    //结束回收资源
    connect(this, &LoadMovie::finished, m_movie, &QMovie::deleteLater);
    connect(this, &LoadMovie::finished, this, &LoadMovie::deleteLater);
}

void LoadMovie::run()
{
    for (int i = 2; i < m_movie->frameCount(); ++i) {
        m_movie->jumpToFrame(i);
        QImage image = m_movie->currentImage();
        Mat mat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                      static_cast<size_t>(image.bytesPerLine()))
                      .clone();
        //如果切换了图片，结束加载
        if (m_list->isEmpty()) {
            return;
        }
        m_list->append(mat);
    }
    //    emit loadMovieFinish(m_movie->fileName());
    Q_EMIT loadMovieFinish(m_movie->fileName());
}
