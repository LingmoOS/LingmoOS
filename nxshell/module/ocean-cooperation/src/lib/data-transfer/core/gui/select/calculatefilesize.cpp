#include "calculatefilesize.h"

#include "common/log.h"

#include <QThreadPool>
#include <QListView>
#include <QStandardItemModel>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <math.h>
#include <QMutex>

QString fromByteToQstring(quint64 bytes)
{
    float tempresult = static_cast<float>(bytes);
    float result = tempresult;
    if (tempresult < 100.0) {
        return QString("%1B").arg(QString::number(result));
    }
    tempresult = tempresult / 1024;
    result = roundf(tempresult * 10) / 10;
    if (result < 100.0) {
        return QString("%1KB").arg(QString::number(result));
    }
    tempresult = tempresult / 1024;
    result = roundf(tempresult * 10) / 10;
    if (result < 100.0) {
        return QString("%1MB").arg(QString::number(result));
    }
    tempresult = tempresult / 1024;
    result = roundf(tempresult * 10) / 10;
    if (result < 100.0) {
        return QString("%1GB").arg(QString::number(result));
    }
    tempresult = tempresult / 1024;
    result = roundf(tempresult * 10) / 10;
    return QString("%1TB").arg(QString::number(result));
}
quint64 fromQstringToByte(QString sizeString)
{
    quint64 bytes = 0;
    if (sizeString.endsWith("KB")) {
        sizeString.chop(2);
        bytes = sizeString.toDouble() * 1024;
    } else if (sizeString.endsWith("MB")) {
        sizeString.chop(2);
        bytes = sizeString.toDouble() * 1024 * 1024;
    } else if (sizeString.endsWith("GB")) {
        sizeString.chop(2);
        bytes = sizeString.toDouble() * 1024 * 1024 * 1024;
    } else if (sizeString.endsWith("TB")) {
        sizeString.chop(2);
        bytes = sizeString.toDouble() * 1024 * 1024 * 1024 * 1024;
    } else if (sizeString.endsWith("B")) {
        sizeString.chop(1);
        bytes = sizeString.toDouble();
    }
    return bytes;
}

CalculateFileSizeTask::CalculateFileSizeTask(QObject *pool, const QString &path)
    : filePath(path), calculatePool(pool)
{
}

CalculateFileSizeTask::~CalculateFileSizeTask() { }

void CalculateFileSizeTask::run()
{
    fileSize = calculate(filePath);
    QMetaObject::invokeMethod(calculatePool, "sendFileSizeSlots", Qt::QueuedConnection,
                              Q_ARG(quint64, fileSize), Q_ARG(QString, filePath));
}

void CalculateFileSizeTask::abortTask()
{
    abort = true;
}

qlonglong CalculateFileSizeTask::calculate(const QString &path)
{
    if (abort) {
        return 0;
    }

    QDir directory(path);
    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList fileList = directory.entryInfoList();
    qlonglong tempSize = 0;
    for (const QFileInfo &fileInfo : fileList) {
        if (fileInfo.isDir()) {
            tempSize += calculate(fileInfo.absoluteFilePath());
        } else {
            tempSize += fileInfo.size();
        }
    }

    return tempSize;
}

CalculateFileSizeThreadPool *CalculateFileSizeThreadPool::instance()
{
    static CalculateFileSizeThreadPool ins;
    return &ins;
}

CalculateFileSizeThreadPool::~CalculateFileSizeThreadPool() { }

CalculateFileSizeThreadPool::CalculateFileSizeThreadPool()
{
    threadPool = new QThreadPool();
    fileMap = new QMap<QString, FileInfo>();
    threadPool->setMaxThreadCount(4);
    // connect main thread exit signal
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                     &CalculateFileSizeThreadPool::exitPool, Qt::DirectConnection);
}

void CalculateFileSizeThreadPool::work(const QList<QString> &list)
{
    for (const QString &path : list) {
        QFileInfo fileInfo(path);
        if (fileInfo.isFile()) {
            continue;
        } else if (fileInfo.isDir()) {
            CalculateFileSizeTask *task = new CalculateFileSizeTask(this, path);
            workList.push_back(task);
            threadPool->start(task);
        } else {
            WLOG << "Path is neither a file nor a directory:" << path.toStdString();
        }
    }
}

void CalculateFileSizeThreadPool::addFileMap(const QString &path, const FileInfo &fileinfo)
{
    (*fileMap)[path] = fileinfo;
}

void CalculateFileSizeThreadPool::delFileMap(const QString &path)
{
    if (fileMap->contains(path))
        fileMap->remove(path);
}

QMap<QString, FileInfo> *CalculateFileSizeThreadPool::getFileMap()
{
    return fileMap;
}

void CalculateFileSizeThreadPool::sendFileSizeSlots(quint64 fileSize, const QString &path)
{
    if (!fileMap->contains(path))
        return;

    (*fileMap)[path].size = fileSize;
    (*fileMap)[path].isCalculate = true;
    emit sendFileSizeSignal(fileSize, path);
}

void CalculateFileSizeThreadPool::addFileSlots(const QList<QString> &list)
{
    work(list);
}

void CalculateFileSizeThreadPool::exitPool()
{
    threadPool->clear();
    for (CalculateFileSizeTask *task : workList) {
        task->abortTask();
    }
    threadPool->waitForDone();
    LOG << "calculate file size exit.";
    delete threadPool;
    delete fileMap;
}

void CalculateFileSizeThreadPool::delDevice(const QStandardItem *siderbarItem)
{
    QMap<QString, FileInfo>::iterator it = fileMap->begin();
    while (it != fileMap->end()) {
        if (it.value().siderbarItem == siderbarItem) {
            it = fileMap->erase(it);
        } else {
            ++it;
        }
    }
}
