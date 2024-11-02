#ifndef SAVEMOVIE_H
#define SAVEMOVIE_H

#include <QThread>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QSemaphore>
#include "lingmoimagecodec_global.h"

using namespace cv;

class SaveMovie : public QThread
{
    Q_OBJECT


Q_SIGNALS:
    void saveMovieFinish(const QString &path);

public:
    SaveMovie(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat,
              QStringList *savelist = nullptr);

protected:
    void run();

private:
    QList<Mat> *m_list;
    QStringList *m_savelist;
    int m_fps;
    QString m_savepath;
    bool m_special;
    QProcess *m_process = nullptr;
    void saveFinish();
    void processLog();
    static const QString TEMP_PATH;       //临时文件路径
    static const QString creatTempPath(); //创建临时目录
    QString m_realFormat = "";
};

#endif // SAVEMOVIE_H
