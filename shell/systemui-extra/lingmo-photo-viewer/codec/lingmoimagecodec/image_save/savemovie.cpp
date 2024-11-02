#include "../savemovie.h"

//临时文件路径
const QString SaveMovie::TEMP_PATH = SaveMovie::creatTempPath();

SaveMovie::SaveMovie(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat,
                     QStringList *savelist)
{
    //结束回收资源
    connect(this, &SaveMovie::finished, this, &SaveMovie::deleteLater);
    m_fps = fps;
    m_savepath = savepath;
    m_savelist = savelist;
    m_realFormat = realFormat;
    m_list = new QList<Mat>;
    for (Mat &mat : *list) {
        m_list->append(mat.clone());
    }
}
const QString SaveMovie::creatTempPath()
{
    const QString filePath = "/tmp/.lingmo-image-codec/";
    QDir dir;
    if (!dir.exists(filePath)) {
        dir.mkdir(filePath);
    }
    return filePath;
}
void SaveMovie::run()
{
    if (m_savelist != nullptr) {
        m_savelist->append(m_savepath);
    }

    m_process = new QProcess;
    connect(m_process, &QProcess::readyReadStandardError, this, &SaveMovie::processLog);

    QFileInfo info(m_savepath);
    QString name = info.completeBaseName();
    QString tmpName = info.completeBaseName() + "_tmp";

    QString suffix;
    if ("" == m_realFormat) {
        suffix = info.suffix().toLower();
    } else {
        suffix = m_realFormat;
    }
    QString tmpDir = TEMP_PATH + name + "/";
    QString ster = "\"";
    //新建目录
    QDir dir;
    dir.mkdir(tmpDir);
    int num = 0;
    //存储所有帧
    for (Mat &mat : *m_list) {
        num++;
        QString str = tmpDir + QString::number(num) + ".png";
        imwrite(str.toStdString(), mat);
    }
    //析构临时队列
    m_list->clear();
    //构造命令
    QString tmpFilePath = tmpDir + tmpName + "." + "apng";
    QString cmd = "apngasm ";
    cmd += ster + tmpFilePath + ster;
    cmd += " " + ster + tmpDir + "*.png\" ";
    cmd += QString::number(m_fps);
    cmd += " 1000 -z0";
    //执行命令
    m_process->start(cmd);
    m_process->waitForStarted();
    m_process->waitForFinished();
    if (!QFileInfo::exists(tmpFilePath)) {
        qDebug() << "动图保存失败";
        return saveFinish();
    }
    //转码
    if (suffix == "gif") {
        QString cmd2 = "apng2gif ";
        cmd2 += " " + ster + tmpFilePath + ster;
        m_process->start(cmd2);
        m_process->waitForStarted();
        m_process->waitForFinished();
        tmpFilePath.chop(4);
        tmpFilePath += suffix;
        if (!QFileInfo::exists(tmpFilePath)) {
            qDebug() << "动图保存失败";
            return saveFinish();
        }
    }
    //移动回原目录
    QString cmd3 = "mv ";
    cmd3 += ster + tmpFilePath + ster;
    cmd3 += " " + ster + m_savepath + ster;
    m_process->start(cmd3);
    m_process->waitForStarted();
    m_process->waitForFinished();
    //删除临时文件
    QString deleteTmpImages = "rm -rf " + ster + tmpDir + ster;
    m_process->start(deleteTmpImages);
    m_process->waitForStarted();
    m_process->waitForFinished();
    m_process->deleteLater();
    return saveFinish();
}

void SaveMovie::saveFinish()
{
    if (m_savelist != nullptr) {
        m_savelist->removeOne(m_savepath);
    }
    //    emit saveMovieFinish(m_savepath);
    Q_EMIT saveMovieFinish(m_savepath);
}

void SaveMovie::processLog()
{
    QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (error == "") {
        return;
    }
    qDebug() << "--------process error--------\n" << error << "\n-----------------------------";
}
