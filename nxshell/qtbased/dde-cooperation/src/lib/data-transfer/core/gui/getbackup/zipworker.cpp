#include <utils/optionsmanager.h>
#include <net/helper/transferhepler.h>

#include "zipworker.h"
#include "../win/drapwindowsdata.h"
#include <common/commonutils.h>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QDir>
#include <QElapsedTimer>
#include <QDirIterator>
#include <QCoreApplication>
#include <JlCompress.h>
#include <QDataStream>

#define BUFFER_SIZE 8 * 1024
ZipWork::ZipWork(QObject *parent) : QThread(parent)
{
    LOG << "zipwork start.";

    maxNum = BUFFER_SIZE * 10240;
    // connect backup file process
    QObject::connect(this, &ZipWork::backupFileProcessSingal, TransferHelper::instance(),
                     &TransferHelper::zipTransferContent);
    // connect the main thread exit signal
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                     &ZipWork::abortingBackupFileProcess);
}

ZipWork::~ZipWork() { }

void ZipWork::run()
{
    getUserDataPackagingFile();
}

void ZipWork::getUserDataPackagingFile()
{
    QStringList zipFilePathList =
            OptionsManager::instance()->getUserOption(Options::kTransferFileList);

    // Get the number of files to zip

    QString size = OptionsManager::instance()->getUserOption(Options::KBackupFileSize)[0];
    allFileSize = size.toULongLong();
    LOG << "bakc up file size:" << allFileSize;
    backupFile(zipFilePathList, getBackupFilName());
}

int ZipWork::getPathFileNum(const QString &filePath)
{
    if (QFileInfo(filePath).isFile())
        return 1;
    int fileCount = 0;
    QDir dir(filePath);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            fileCount += getPathFileNum(entry.absoluteFilePath());
        } else {
            fileCount++;
        }
    }

    return fileCount;
}

int ZipWork::getAllFileNum(const QStringList &fileList)
{
    int fileNum = 0;

    for (const QString &filePath : fileList) {
        int curPathFileNum = getPathFileNum(filePath);
        fileNum += curPathFileNum;
    }
    return fileNum;
}

bool ZipWork::addFileToZip(const QString &filePath, const QString &relativeTo, QuaZip &zip,
                           QElapsedTimer &timer)
{
    if (abort) {
        zip.close();
        QFile::remove(zipFile);
        return false;
    }

    QFile sourceFile(filePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Error reading source file:" << filePath;
        // backup file false
        sendBackupFileFailMessage(filePath);
        return false;
    }

    QuaZipFile destinationFile(&zip);
    QString destinationFileName = QDir(relativeTo).relativeFilePath(filePath);
    QuaZipNewInfo newInfo(destinationFileName, sourceFile.fileName());
    if (!destinationFile.open(QIODevice::WriteOnly, newInfo)) {
        qCritical() << "Error writing to ZIP file for:" << filePath;
        // backup file false

        sendBackupFileFailMessage(filePath);
        return false;
    }

    QDataStream in(&sourceFile);
    char *buffer = reinterpret_cast<char *>(malloc(BUFFER_SIZE));
    while (!in.atEnd()) {
        memset(buffer, 0, BUFFER_SIZE);
        qint64 bytesRead = in.readRawData(buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            free(buffer);
            qCritical() << "Error reading from source file:" << filePath;
            destinationFile.close();
            sourceFile.close();
            sendBackupFileFailMessage(filePath);
            return false;
        }

        qint64 bytesWritten = destinationFile.write(buffer, bytesRead);
        if (bytesWritten == -1) {
            free(buffer);
            qCritical() << "Error writing to ZIP file for:" << filePath;
            destinationFile.close();
            sourceFile.close();
            sendBackupFileFailMessage(filePath);
            return false;
        }

        sendBackupFileProcess(filePath, timer, bytesRead);
    }
    free(buffer);
    destinationFile.close();
    sourceFile.close();

    return true;
}

bool ZipWork::addFolderToZip(const QString &sourceFolder, const QString &relativeTo, QuaZip &zip,
                             QElapsedTimer &timer)
{
    QDir directory(sourceFolder);
    QFileInfoList entries = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

    for (QFileInfo entry : entries) {
        if (entry.isDir()) {
            if (!addFolderToZip(entry.absoluteFilePath(), relativeTo, zip, timer)) {
                return false;
            }
        } else {
            if (!addFileToZip(entry.absoluteFilePath(), relativeTo, zip, timer)) {
                return false;
            }
        }
    }

    // If the current folder is empty, then create an empty directory
    if (entries.isEmpty()) {
        QuaZipFile dirZipFile(&zip);
        QString dirFileName = QDir(relativeTo).relativeFilePath(sourceFolder) + "/";
        QuaZipNewInfo newInfo(dirFileName);
        dirZipFile.open(QIODevice::WriteOnly, newInfo);
        dirZipFile.close();
    }

    return true;
}

bool ZipWork::backupFile(const QStringList &entries, const QString &destinationZipFile)
{
    zipFile = destinationZipFile;
    QuaZip zip(destinationZipFile);
    QElapsedTimer timer;
    zip.setFileNameCodec("UTF-8");
    if (!zip.open(QuaZip::mdCreate)) {
        qCritical("Error creating the ZIP file.");
        // backup file false
        sendBackupFileFailMessage(destinationZipFile);
        return false;
    }

    for (QString entry : entries) {

        QFileInfo fileInfo(entry);
        if (fileInfo.isDir()) {
            QDir parent = QDir(entry);
            parent.cdUp();
            if (!addFolderToZip(entry, QDir(parent).absolutePath(), zip, timer)) {
                return false;
            }
        } else if (fileInfo.isFile()) {
            if (!addFileToZip(entry, fileInfo.absolutePath(), zip, timer)) {
                return false;
            }
        }
    }

    zip.close();

    if (zip.getZipError() != UNZ_OK) {
        qCritical() << "Error while compressing. Error code:" << zip.getZipError();
        // backup file false

        sendBackupFileFailMessage(destinationZipFile);
        return false;
    }

    // backup file true
    emit backupFileProcessSingal(QString(tr("Back up file done")), 100, 0);
    return true;
}

void ZipWork::sendBackupFileProcess(const QString &filePath, QElapsedTimer &timer, int size)
{
    zipFileSize += size;
    num += size;
    double progress = (static_cast<double>(zipFileSize) / static_cast<double>(allFileSize)) * 100;
    if (!timer.isValid()) {
        timer.start();
    } else {
        quint64 tempTime = 0;
        if (firstFlag) {
            tempTime = BUFFER_SIZE * 5;
        } else {
            tempTime = maxNum;
        }
        LOG << "num:" << num << " tempTime:" << tempTime;
        // If the timer is started, the elapsed time is calculated and the timer is restarted
        if (num >= tempTime) {
            firstFlag = false;
            qint64 elapsed = timer.restart();
            if (allFileSize <= zipFileSize) {
                needTime = static_cast<int>(elapsed * (0 / num)) / 1000;
                LOG << "needtime:" << needTime << "(allFileSize - zipFileSize):" << 0
                    << "zipFileSize:" << zipFileSize << " num:" << num << " elapsed:" << elapsed;
            } else {
                needTime = static_cast<int>(elapsed * ((allFileSize - zipFileSize) / num)) / 1000;
                LOG << "needtime:" << needTime
                    << "(allFileSize - zipFileSize)/num:" << (allFileSize - zipFileSize) / num
                    << " zipFileSize:" << zipFileSize << " num:" << num << "elapsed:" << elapsed;
            }
            num = 0;
        }
    }

    needTime = std::max(std::min(needTime, 3600), 1);
    int iprogress = std::max(std::min((int)progress, 100), 0);

    emit backupFileProcessSingal(filePath, iprogress, needTime);
}

QString ZipWork::getBackupFilName()
{
    QStringList zipFileSavePath =
            OptionsManager::instance()->getUserOption(Options::kBackupFileSavePath);
    QStringList zipFileNameList =
            OptionsManager::instance()->getUserOption(Options::kBackupFileName);

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyyMMddhhmm");

    QString zipFileName;

    if (zipFileNameList[0] == "") {
        zipFileName = TransferHelper::instance()->defaultBackupFileName() + ".zip";
    } else {
        QString path = zipFileSavePath.isEmpty() ? "" : zipFileSavePath[0];
        zipFileName = path + "/" + zipFileNameList[0] + ".zip";
    }
    LOG << "backup file save path:" << zipFileName.toStdString();

    return zipFileName;
}

void ZipWork::sendBackupFileFailMessage(const QString &path)
{
    QString text = deepin_cross::CommonUitls::elidedText(path, Qt::ElideRight,50);
    emit backupFileProcessSingal(
            QString(tr("%1 File compression failed")).arg(text), -1, -1);
}

void ZipWork::abortingBackupFileProcess()
{
    abort = true;
    //  quit();
    // wait();
    LOG << "backup file exit.";
}
