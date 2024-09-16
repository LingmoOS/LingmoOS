// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef HELPER_H
#define HELPER_H

#include <QByteArray>
#include <QJsonArray>
#include <QObject>
#include <QLoggingCategory>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include <typeinfo>

QT_BEGIN_NAMESPACE
class QProcess;
QT_END_NAMESPACE

class DPartInfo;
class Helper : public QObject
{
    Q_OBJECT

public:
    static Helper *instance();

    static int processExec(QProcess *process, const QString &program, QStringList args,
                           int timeout = -1, QIODevice::OpenMode mode = QIODevice::ReadOnly);
    static int processExec(const QString &command, const QStringList &args, int timeout = -1);
    static QByteArray lastProcessStandardOutput();
    static QByteArray lastProcessStandardError();

    static const QLoggingCategory &loggerCategory();
    static const QLoggingCategory &formatLogger();
    static void formatLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void registerFormatLogHandler(const QString &logFile);

    void warning(const QString &message);
    void error(const QString &message);
    QString lastWarningString();
    QString lastErrorString();

    static QString sizeDisplay(qint64 size);
    static QString secondsToString(qint64 seconds);

    static bool refreshSystemPartList(const QString &device = QString());
    static QString getPartcloneExecuter(const DPartInfo &info, QStringList &args);
    static bool getPartitionSizeInfo(const QString &partDevice, qint64 *used, qint64 *free, int *blockSize);

    static QByteArray callLsblk(const QStringList &extraArg = QStringList());
    static QJsonArray getBlockDevices(const QStringList &commandExtraArg = QStringList());

    static QString mountPoint(const QString &device);
    static bool isMounted(const QString &device);
    static bool umountDevice(const QString &device);
    static bool tryUmountDevice(const QString &device);
    static bool mountDevice(const QString &device, const QString &path, bool readonly = false);
    static QString temporaryMountDevice(const QString &device, const QString &name, bool readonly = false);

    static QString findDiskBySerialIndexNumber(const QString &serialNumber, int partIndexNumber = -1);
    static int partitionIndexNumber(const QString &partDevice);

    static QByteArray getPartitionTable(const QString &devicePath);
    static bool setPartitionTable(const QString &devicePath, const QString &ptFile);
    static bool saveToFile(const QString &fileName, const QByteArray &data, bool override = true);
    static bool isBlockSpecialFile(const QString &fileName);
    static bool isPartcloneFile(const QString &fileName);
    static bool isDiskDevice(const QString &devicePath);
    static bool isPartitionDevice(const QString &devicePath);
    static QString parentDevice(const QString &device);
    static bool deviceHaveKinship(const QString &device1, const QString &device2);

    static int clonePartition(const DPartInfo &part, const QString &to, bool override = true);
    static int restorePartition(const QString &from, const DPartInfo &to);

    static bool existLiveSystem();
    static bool restartToLiveSystem(const QStringList &arguments);

    static bool isDeepinSystem(const DPartInfo &part);
    static bool resetPartUUID(const DPartInfo &part, QByteArray uuid = QByteArray());

    static QString getDeviceForFile(const QString &file, QString *rootPath = 0);
    static QString parseSerialUrl(const QString &urlString, QString *errorString = 0);
    static QString toSerialUrl(const QString &file);

    // for lsblk
    static qint64 getIntValue(const QJsonValue &value);
    static bool getBoolValue(const QJsonValue &value);

    //for custom file
    /**
     * @brief setCustomFile write data into dim file from custom file
     * @param source dim file,format: dim://example.dim/custom
     * @param customFile custom file name
     * @return successful return true, else return false
     */
    static bool writeCustomFile(const QString &source, const QString &customFileName);

    /**
     * @brief getCustomFile get a custom file from the dim
     * @param source dim file,format: dim://example.dim/custom
     * @param customFile custom file name
     * @return successful return true, else return false
     */
    static bool readCustomFile(const QString &source, const QString &customFileName);

signals:
    void newWarning(const QString &message);
    void newError(const QString &message);

private:
    static QByteArray m_processStandardOutput;
    static QByteArray m_processStandardError;

    QString m_warningString;
    QString m_errorString;
};

template<typename... Args>
static
typename QtPrivate::QEnableIf<int(sizeof...(Args)) >= 1, QString>::Type
__d_asprintf__(const char *format, Args&&... args)
{
    return QString::asprintf(format, std::forward<Args>(args)...);
}
template<typename... Args>
static
typename QtPrivate::QEnableIf<int(sizeof...(Args)) == 0, QString>::Type
__d_asprintf__(const char *message, Args&&...)
{
    return QString::asprintf("%s", message);
}
template<typename... Args>
static QString __d_asprintf__(const QString &string, Args&&... args)
{
    return __d_asprintf__(string.toUtf8().constData(), std::forward<Args>(args)...);
}
template<typename... Args>
static QString __d_asprintf__(const QByteArray &array, Args&&... args)
{
    return __d_asprintf__(array.constData(), std::forward<Args>(args)...);
}

#ifdef dCDebug
#undef dCDebug
#endif
#define dCDebug(...) qCDebug(Helper::loggerCategory, __VA_ARGS__)

#ifdef dCInfo
#undef dCInfo
#endif
#define dCInfo(format, ...) { \
    QString __m = __d_asprintf__(format, ##__VA_ARGS__); \
    __m.prepend("\033[33m"); __m.append("\033[0m"); \
    qCInfo(Helper::loggerCategory, qPrintable(__m));}

#ifdef dCWarning
#undef dCWarning
#endif
#define dCWarning(format, ...) { \
    QString __m = __d_asprintf__(format, ##__VA_ARGS__); \
    __m.prepend("\033[30;41m"); __m.append("\033[0m"); \
    Helper::instance()->warning(__m); \
    qCWarning(Helper::loggerCategory, qPrintable(__m));}

#ifdef dCError
#undef dCError
#endif
#define dCError(format, ...) { \
    QString __m = __d_asprintf__(format, ##__VA_ARGS__); \
    __m.prepend("\033[30;45m"); __m.append("\033[0m"); \
    Helper::instance()->warning(__m); \
    qCCritical(Helper::loggerCategory, qPrintable(__m));}

#ifndef dfPrint
#define dfPrint(...) qCInfo(Helper::formatLogger, __VA_ARGS__)
#endif

namespace ThreadUtil {
template <typename ReturnType>
class _TMP
{
public:
    template <typename Fun, typename... Args>
    static ReturnType runInNewThread(Fun fun, Args&&... args)
    {
        QFutureWatcher<ReturnType> watcher;
        QEventLoop loop;

        QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);
        watcher.setFuture(QtConcurrent::run(fun, std::forward<Args>(args)...));

        loop.exec();

        return watcher.result();
    }
};

template <typename Fun, typename... Args>
auto runInNewThread(Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return _TMP<decltype(fun(args...))>::runInNewThread(fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInNewThread(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInNewThread([&] {
        return (obj->*fun)(std::forward<Args>(args)...);
    });
}

}

#endif // HELPER_H
