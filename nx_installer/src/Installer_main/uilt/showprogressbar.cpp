#include "showprogressbar.h"

#include <QDir>
#include <QThread>
#include <QDebug>
#include <sys/statvfs.h>

namespace KInstaller {
ShowProgressBar::ShowProgressBar(QObject *parent) : QObject(parent),
    m_value(0)
{
    timer = new QTimer(this);
    timer->setInterval(500);

    connect(timer, &QTimer::timeout, this, &ShowProgressBar::flushProgressBar);
    start();
}

quint64 ShowProgressBar::getDirSize(const QString filepath)
{
    QDir tmpDir(filepath);
    quint64 size = 0;
    /*获取文件列表  统计文件大小*/
    foreach (QFileInfo fileinfo, tmpDir.entryInfoList(QDir::Files)) {
        size += fileinfo.size();
    }
    /*获取文件夹  并且过滤掉.和..文件夹 统计各个文件夹的文件大小 */
    foreach (QString subDir, tmpDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        size += getDirSize(filepath + QDir::separator() + subDir);  //递归进行  统计所有子目录
    }
    return size;
}

bool ShowProgressBar::copyDir(const QString &source, const QString &destination, bool override)
{
    QDir directory(source);
    if (!directory.exists()) {
        return false;
    }

    QString srcPath = QDir::toNativeSeparators(source);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();
    QString dstPath = QDir::toNativeSeparators(destination);
    if (!dstPath.endsWith(QDir::separator()))
        dstPath += QDir::separator();

    bool error = false;
    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    //for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    {
        QString fileName = fileNames.at(i);
        QString srcFilePath = srcPath + fileName;
        QString dstFilePath = dstPath + fileName;
        QFileInfo fileInfo(srcFilePath);
        if (fileInfo.isFile() || fileInfo.isSymLink())
        {
            if (override)
            {
                QFile::setPermissions(dstFilePath, QFile::WriteOwner);
            }
            QFile::copy(srcFilePath, dstFilePath);
        }
        else if (fileInfo.isDir())
        {
            QDir dstDir(dstFilePath);
            dstDir.mkpath(dstFilePath);
            if (!copyDir(srcFilePath, dstFilePath, override))
            {
                error = true;
            }
        }
    }

    return !error;
}

void ShowProgressBar::flushProgressBar()
{
    struct statvfs buf;
    struct statvfs targetbuf;

    fsfilcnt_t sourceInodes = 1;
    fsfilcnt_t targetInodes = 1;

    if (statvfs("/rofs", &buf) == 0) {
         sourceInodes = buf.f_files - buf.f_ffree;
         qDebug() << sourceInodes;
         if (statvfs("/target", &targetbuf) == 0) {
            targetInodes = targetbuf.f_files - targetbuf.f_ffree;
            qDebug() << targetInodes;
            if (sourceInodes > 0) {
                if (sourceInodes >= targetInodes) {
                    m_value = 90 * targetInodes / sourceInodes;
                    if(m_value == 90) {
                        timer->stop();
                    }
                } else {
                    m_value = 90;
                    timer->stop();
                }
                emit signelProgressValue(m_value);
            }
        }
    }
}


void ShowProgressBar::start()
{
    timer->start();
}

}
