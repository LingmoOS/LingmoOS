#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QVariantList>
#include <QIcon>
#include <QFileInfo>
#include <QDir>

class SystemInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cpuModel READ cpuModel NOTIFY cpuModelChanged)
    Q_PROPERTY(QString osInfo READ osInfo NOTIFY osInfoChanged)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
    Q_PROPERTY(double memoryUsage READ memoryUsage NOTIFY memoryUsageChanged)
    Q_PROPERTY(QString totalMemory READ totalMemory NOTIFY totalMemoryChanged)
    Q_PROPERTY(QString usedMemory READ usedMemory NOTIFY usedMemoryChanged)
    Q_PROPERTY(QVariantList processList READ processList NOTIFY processListChanged)
    Q_PROPERTY(QVariantList diskList READ diskList NOTIFY diskListChanged)
    Q_PROPERTY(QVariantList networkList READ networkList NOTIFY networkListChanged)
    Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY kernelVersionChanged)
    Q_PROPERTY(QString hostname READ hostname NOTIFY hostnameChanged)
    Q_PROPERTY(QString desktopEnvironment READ desktopEnvironment NOTIFY desktopEnvironmentChanged)
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(int cpuCoreCount READ cpuCoreCount CONSTANT)
    Q_PROPERTY(QVariantList cpuCoreUsage READ cpuCoreUsage NOTIFY cpuCoreUsageChanged)

public:
    explicit SystemInfo(QObject *parent = nullptr);

    QString cpuModel() const;
    QString osInfo() const;
    double cpuUsage() const;
    double memoryUsage() const;
    QString totalMemory() const;
    QString usedMemory() const;
    QVariantList processList() const;
    QVariantList diskList() const;
    QVariantList networkList() const;
    QString kernelVersion() const;
    QString hostname() const;
    QString desktopEnvironment() const;
    int updateInterval() const { return m_updateTimer->interval(); }
    int cpuCoreCount() const { return m_cpuCoreCount; }
    QVariantList cpuCoreUsage() const {
        QVariantList list;
        for (double value : m_cpuCoreUsage) {
            list.append(value);
        }
        return list;
    }

    Q_INVOKABLE void killProcess(qint64 pid);
    Q_INVOKABLE void updateProcessPriority(qint64 pid, int priority);
    Q_INVOKABLE QVariantList filteredProcessList(const QString &filter, const QString &sortBy, bool ascending);
    Q_INVOKABLE void refreshProcesses();
    void setUpdateInterval(int interval) {
        if (m_updateTimer->interval() != interval) {
            m_updateTimer->setInterval(interval);
            emit updateIntervalChanged();
        }
    }

signals:
    void cpuModelChanged();
    void osInfoChanged();
    void cpuUsageChanged();
    void memoryUsageChanged();
    void totalMemoryChanged();
    void usedMemoryChanged();
    void processListChanged();
    void diskListChanged();
    void networkListChanged();
    void kernelVersionChanged();
    void hostnameChanged();
    void desktopEnvironmentChanged();
    void updateIntervalChanged();
    void cpuCoreUsageChanged();

private slots:
    void updateCpuUsage();
    void updateMemoryInfo();
    void updateProcessList();
    void updateDiskInfo();
    void updateNetworkInfo();

private:
    QString m_cpuModel;
    QString m_osInfo;
    double m_cpuUsage;
    double m_memoryUsage;
    QString m_totalMemory;
    QString m_usedMemory;
    QVariantList m_processList;
    QVariantList m_diskList;
    QVariantList m_networkList;
    QString m_kernelVersion;
    QString m_hostname;
    QString m_desktopEnvironment;

    QTimer *m_updateTimer;
    unsigned long long m_prevIdleTime;
    unsigned long long m_prevTotalTime;
    QMap<QString, QPair<quint64, quint64>> m_prevNetworkBytes;
    QMap<QString, unsigned long long> m_prevProcessTimes;  // 存储每个进程的上一次 CPU 时间

    int m_cpuCoreCount;
    QVector<double> m_cpuCoreUsage;
    QVector<unsigned long long> m_prevCoreIdleTime;
    QVector<unsigned long long> m_prevCoreTotalTime;

    void initSystemInfo();
    QString formatSize(unsigned long long size);
    QString formatSpeed(double bytesPerSecond);
    QString findProcessIcon(const QString &name, qint64 pid);
};

#endif // SYSTEMINFO_H 