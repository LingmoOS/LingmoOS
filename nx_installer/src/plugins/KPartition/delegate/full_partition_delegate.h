#ifndef QUICK_PARTITION_DELEGATE_H
#define QUICK_PARTITION_DELEGATE_H

#include <QObject>
#include "partition_delegate.h"
#include "partman/device.h"
#include "partman/filesystem.h"

namespace KInstaller {
using namespace Partman;
struct FinalFullDiskPolicy
{
    QString filesystem;
    QString mountPoint;
    QString label;
    QString device;
    qint64 offset;
    qint64 size;
};

typedef QList<FinalFullDiskPolicy> FinalQuickDiskPolicyList;
struct FinalFullDiskOption
{
    QString device;
    QString passwd;
    FinalQuickDiskPolicyList policyList;
    bool encrypt;
};

typedef QList<FinalFullDiskOption>FinalFullDiskOptionList;
struct FinalQuickDiskResolution
{
    FinalFullDiskOptionList optionList;
};

void writeQuickDiskResolution(const FinalQuickDiskResolution& resolution);
void writeQuickDiskMode(bool value);

QByteArray getQuickDiskInstallPolicy();
struct FullPolicy
{
    FSType filesystem;
    QString mountPoint;
    QString label;
    QString usage;
    QString device;
    bool alignStart;
    qint64 startSector;
    qint64 endSector;
    qint64 sectors;
    PartitionType partitionType;
};

typedef QList<FullPolicy> FullDiskPolicyList;
struct QuickDiskOption
{
    FullDiskPolicyList policyList;
    bool isSystemDisk;
};
struct SizeRange
{
    qint64 minSizeBytes;
    qint64 maxSizeBytes;
};

struct PartitionAdjustOption
{
    int rootSizeCount;
    int percent100Count;
    qint64 startOffsetSector;
    qint64 freeBytes;
    SizeRange rootSizeRange;
};


class PartitionDelegate;
class FullPartitionDelegate : public PartitionDelegate
{
    Q_OBJECT
public:
    explicit FullPartitionDelegate(QObject* parent = nullptr);

    Device::Ptr fullDiskScheme(Device::Ptr device) ;

    void addSystmDisk(const QString& devPath);
    void addDataDisk(const QString& devPath);
    const QStringList& selectedDisks();
    void removeAllSelectedDisk();
    bool DiskSizeValidate(QString devpath);
    bool DiskSizeLimited(QString devpath);

    bool formatWholeDeviceMultipleDisk();
    void getFinalDiskResolution(FinalQuickDiskResolution& resolution);
    void setAutoInstall(bool autoinstall);
signals:
    void requestAutoInstallFinished(bool finished) ;

private:
    const SizeRange getRootPartitionSizeRange();
    void adjustFullDiskPolicy(const Device::Ptr& dev, QuickDiskOption& option, const PartitionAdjustOption& adjust);

    bool formatWholeDeviceV2(const Device::Ptr& device, QuickDiskOption& option);

    uint getSwapSize();
public slots:
    bool formatWholeDevcie(const QString& devPath, PartTableType type);
    void onCustomPartDone(const DeviceList& devices) override;

    void saveSwapSize();

private:
    bool m_quickInstall;

};
}
#endif // QUICKPARTITIONDELEGATE_H
