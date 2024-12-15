#ifndef CALCULATEFILESIZE_H
#define CALCULATEFILESIZE_H

#include <QModelIndex>
#include <QObject>
#include <QRunnable>
#include <QThread>

class QListView;
class QThreadPool;
class QTimer;
class QStandardItem;
class QMutex;
struct FileInfo
{
    quint64 size;
    bool isSelect;
    bool isCalculate;
    QStandardItem *fileItem;
    QStandardItem *siderbarItem;
};

QString fromByteToQstring(quint64 bytes);
quint64 fromQstringToByte(QString sizeString);

class CalculateFileSizeTask : public QRunnable
{
public:
    CalculateFileSizeTask(QObject *pool, const QString &path);
    ~CalculateFileSizeTask() override;
    void run() override;
    void abortTask();

private:
    qlonglong calculate(const QString &path);
    QString filePath;
    qlonglong fileSize{ 0 };
    QObject *calculatePool{ nullptr };
    bool abort{ false };
};

class CalculateFileSizeThreadPool : public QObject
{
    Q_OBJECT
public:
    static CalculateFileSizeThreadPool *instance();
    ~CalculateFileSizeThreadPool();
    void work(const QList<QString> &list);

    void addFileMap(const QString &path, const FileInfo &fileinfo);
    void delFileMap(const QString &path);
    void delDevice(const QStandardItem *siderbarItem);

    QMap<QString, FileInfo> *getFileMap();

public slots:
    void sendFileSizeSlots(quint64 fileSize, const QString &path);
    void addFileSlots(const QList<QString> &list);
    void exitPool();

signals:
    void sendFileSizeSignal(quint64 fileSize, const QString &path);

private:
    CalculateFileSizeThreadPool();
    QThreadPool *threadPool;
    QList<CalculateFileSizeTask *> workList;

public:
    QMap<QString, FileInfo> *fileMap;
};

#endif // CALCULATEFILESIZE_H
