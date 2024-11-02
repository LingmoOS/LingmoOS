#ifndef LOADMOVIE_H
#define LOADMOVIE_H

#include <QThread>
#include <QMovie>
#include "lingmoimagecodec_global.h"

using namespace cv;

class LoadMovie : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void loadMovieFinish(const QString &path);

public:
    LoadMovie(QList<Mat> *list, QMovie *movie);

protected:
    void run();

private:
    QList<Mat> *m_list = nullptr;
    QMovie *m_movie = nullptr;
};

#endif // LOADMOVIE_H
