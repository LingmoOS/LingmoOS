#ifndef ZIPWORKER_H
#define ZIPWORKER_H

#include <QThread>

class QElapsedTimer;
class QuaZip;
class ZipWork : public QThread
{
    Q_OBJECT
public:
    ZipWork(QObject *parent = nullptr);
    ~ZipWork() override;

    void run() override;

private:
    void getUserDataPackagingFile();

    int getPathFileNum(const QString &filePath);
    int getAllFileNum(const QStringList &fileList);

    bool addFileToZip(const QString &filePath, const QString &relativeTo, QuaZip &zip,
                      QElapsedTimer &timer);
    bool addFolderToZip(const QString &sourceFolder, const QString &relativeTo, QuaZip &zip,
                        QElapsedTimer &timer);
    bool backupFile(const QStringList &sourceFilePath, const QString &zipFileSave);

    void sendBackupFileProcess(const QString &filePath, QElapsedTimer &timer,int size);

    QString getBackupFilName();

    void sendBackupFileFailMessage(const QString &path);
signals:
    void backupFileProcessSingal(const QString &content, int progressbar, int estimatedtime);
public slots:
    void abortingBackupFileProcess();

private:
    quint64 allFileSize{ 0 };
    quint64 zipFileSize{ 0 };
    int lastZipFileNum{ 0 };
    int needTime{ 3600 };
    bool abort{ false };
    quint64 num{ 0 };
    quint64 maxNum{ 0 };
    QString zipFile;
    bool firstFlag {true};
};
#endif
