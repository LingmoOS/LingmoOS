#ifndef VALIDATESTATE_H
#define VALIDATESTATE_H

#include <QObject>
#include "../partman/partition.h"
namespace KInstaller {
using namespace Partman;
typedef long ValidateStateId;

class Validator {
public:
    typedef  QSharedPointer<Validator>Ptr;
    explicit Validator(ValidateStateId state);
    explicit Validator(ValidateStateId state, const Partition::Ptr& partition);

    ValidateStateId state() const;
    const Partition::Ptr partition() const;
    bool equals(const Validator& validator) const;

private:
    Partition::Ptr m_partition;
    ValidateStateId m_state;
};

class ValidateState : public Validator::Ptr
{
public:
    ValidateState();
    ValidateState(Validator* p);
    ValidateState(ValidateStateId state);
    ValidateState(ValidateStateId state, const Partition::Ptr& partition);

    bool eqauls(const ValidateState& state) const;
    operator ValidateStateId() const;

    enum{
        InvalidId = 0,//wuxiao
        BootFsInvalid ,//vfat
        BootPartNumberInvalid ,//first partition
        BootTooSmall ,//500M
        BootMissing,//not found boot partition
        EfiMissing ,
        EfiTooMany,//
        EfiSizeError ,//256M vfat
        EfiPartNumberInvalid ,
        RootMissing ,
        RootTooSmall ,
        PartitionTooSmall ,
        BackUpPartitionTooSmall ,
        RepeatedMountPoint,
        DataORBackupPartition,
        MipsBootExt3,//适配龙芯的boot分区必须ext3才能启动的问题
        HUAWEIKirin990,//华为HUAWEI_Kirin990的EFI分区必须为fat16
        MipsNotSupportEFI,//龙芯部分机型不支持efi分区
        InvalidFS,//除了EFI分区使用fat16或fat32外，其他分区不建议使用fat文件系统
        OK

    } ;

};
typedef QList<ValidateState> ValidateStates;
}
#endif // VALIDATESTATE_H
