// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKPARTITION_H
#define DBLOCKPARTITION_H

#include <DBlockDevice>

DMOUNT_BEGIN_NAMESPACE

class DBlockPartition;
namespace DDeviceManager {
DExpected<DBlockPartition *> createBlockPartition(const QString &path, QObject *parent);
DExpected<DBlockPartition *> createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent);
}   // namespace DDeviceManager

class DBlockPartitionPrivate;
class DBlockPartition : public DBlockDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DBlockPartition)

    Q_PROPERTY(quint64 flags READ flags NOTIFY flagsChanged)
    Q_PROPERTY(bool isContained READ isContained NOTIFY isContainedChanged)
    Q_PROPERTY(bool isContainer READ isContainer NOTIFY isContainerChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(uint number READ number NOTIFY numberChanged)
    Q_PROPERTY(quint64 offset READ offset NOTIFY offsetChanged)
    Q_PROPERTY(quint64 size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QString table READ table CONSTANT)
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)
    Q_PROPERTY(MbrPartitionType mbrType READ mbrType NOTIFY mbrPartitionTypeChanged)
    Q_PROPERTY(GptPartitionType gptType READ gptType NOTIFY gptPartitionTypeChanged)
    Q_PROPERTY(QString UUID READ UUID NOTIFY UUIDChanged)

public:
    //! @~english @see https://en.wikipedia.org/wiki/Partition_type
    enum MbrPartitionType {
        UnknownMbrType = -1,

        EmptyMbrType = 0x00,
        FAT12Type = 0x01,
        XENIX_root = 0x02,
        XENIX_usr = 0x03,
        FAT16_Less_32M = 0x04,
        Extended = 0x05,
        FAT16Type = 0x06,
        HPFS_NTFS = 0x07,
        AIX = 0x08,
        AIX_bootable = 0x09,
        OS2_Boot_Manager = 0x0a,
        Win95_FAT32 = 0x0b,
        Win95_FAT32_LBA = 0x0c,
        Win95_FAT16_LBA = 0x0e,
        Win95_Extended_LBA = 0x0f,
        OPUS = 0x10,
        Hidden_FAT12 = 0x11,
        Compaq_diagnostics = 0x12,
        Hidden_FAT16_Less_32M = 0x14,
        Hidden_FAT16 = 0x16,
        Hidden_HPFS_or_NTFS = 0x17,
        AST_SmartSleep = 0x18,
        Hidden_Win95_FAT32 = 0x1b,
        Hidden_Win95_FAT32_LBA = 0x1c,
        Hidden_Win95_FAT16_LBA = 0x1e,
        NEC_DOS = 0x24,
        Plan9 = 0x39,
        PartitionMagic_recovery = 0x3c,
        Venix_80286 = 0x40,
        PPC_PReP_Boot = 0x41,
        SFS = 0x42,
        QNX4_dot_x = 0x4d,
        QNX4_dot_x_2nd_part = 0x4e,
        QNX4_dot_x_3rd_part = 0x4f,
        OnTrack_DM = 0x50,
        OnTrack_DM6_Aux1 = 0x51,
        CP_M = 0x52,
        OnTrack_DM6_Aux3 = 0x53,
        OnTrackDM6 = 0x54,
        EZ_Drive = 0x55,
        Golden_Bow = 0x56,
        Priam_Edisk = 0x5c,
        SpeedStor = 0x61,
        GNU_HURD_or_SysV = 0x63,
        Novell_Netware_286 = 0x64,
        Novell_Netware_386 = 0x65,
        DiskSecure_Multi_Boot = 0x70,
        PC_IX = 0x75,
        Old_Minix = 0x80,
        Minix_old_Linux = 0x81,
        Linux_swap = 0x82,
        Linux = 0x83,
        OS2_hidden_C_drive = 0x84,
        Linux_extended = 0x85,
        NTFS_volume_set_1 = 0x86,
        NTFS_volume_set_2 = 0x87,
        Linux_LVM = 0x8e,
        Amoeba = 0x93,
        Amoeba_BBT = 0x94,
        BSD_OS = 0x9f,
        IBM_Thinkpad_hibernation = 0xa0,
        FreeBSD = 0xa5,
        OpenBSD = 0xa6,
        NeXTSTEP = 0xa7,
        NetBSD = 0xa8,
        BSDI_fs = 0xa9,
        BSDI_swap = 0xb7,
        Boot_Wizard_hidden = 0xb8,
        DRDOS_sec_FAT12 = 0xbb,
        DRDOS_sec_FAT16_Less_32M = 0xc1,
        DRDOS_sec_FAT16 = 0xc4,
        DRDOS_sec_extend = 0xc5,
        Syrinx = 0xc6,
        Non_FS_data = 0xc7,
        Multiuser_DOS_extend = 0xd5,
        CP_M_CTOS_dot_dot_dot = 0xda,
        Dell_Utility = 0xdb,
        BootIt = 0xde,
        DOS_access = 0xdf,
        DOS_R_O = 0xe1,
        SpeedStor_1 = 0xe3,
        BeOS_fs = 0xe4,
        EFI_GPT = 0xeb,
        EFI_FAT12_16_32 = 0xee,
        Linux_PA_RISC_boot = 0xef,
        SpeedStor_2 = 0xf0,
        SeppdStor_3 = 0xf4,
        DOS_secondary = 0xf2,
        Linux_raid_autodetect = 0xfd,
        LANstep = 0xfe,
        BBT = 0xff,

    };
    Q_ENUM(MbrPartitionType)

    //! @~english @see https://en.wikipedia.org/wiki/GUID_Partition_Table
    enum GptPartitionType {
        UnknownGptType = -1,
        EmptyGptType = 0,

        UnusedEntryNA = 1,
        MBRPartitionSchemeNA,
        EFISystemPartitionNA,
        BIOSBootPartitionNA,
        IntelFastFlashPartitionNA,
        SonyBootPartitionNA,
        LenovoBootPartitionNA,

        MicrosoftReservedPartitionWin = 20,
        BasicDataPartitionWin,
        LogicalDiskManagerMetaDataPartitionWin,
        LogicalDiskManagerDataPartitionWin,
        WindowsRecoveryEnvironmentWin,
        IBMGeneralParallelFileSystemPartitionWin,
        StorageSpacesPartitionWin,
        StorageReplicaPartitionWin,

        DataPartitionHPUX = 40,
        ServicePartitionHPUX,

        LinuxFilesystemDataLinux = 50,
        RAIDPartitionLinux,
        RootPartitionX86Linux,
        RootPartitionX8664Linux,
        RootPartitionArm32Linux,
        RootPartitionArm64Linux,
        BootPartitionLinux,
        SwapPartitionLinux,
        LogicalVolumeManagerPartitionLinux,
        HomePartitionLinux,
        ServerDataPartitionLinux,
        PlainDMCryptPartitionLinux,
        LUKSPartitionLinux,
        ReservedLinux,

        BootPartitionFreeBSD = 100,
        BSDDisklabelPartitionFreeBSD,
        SwapPartitionFreeBSD,
        UnixFileSystemPartitionFreeBSD,
        VinumVolumeManagerPartitionFreeBSD,
        ZFSPartitionFreeBSD,
        NandfsPartitionFreeBSD,

        HierarchialFileSystemPlusPartitionMacOS = 120,
        AppleAPFSContainerMacOS,
        AppleUFSContainerMacOS,
        ZFSMacOS,
        AppleRAIDPartitionMacOS,
        AppleRAIDPartitionOfflineMacOS,
        AppleBootPartitionMacOS,
        AppleLabelMacOS,
        AppleTVRecoveryPartitionMacOS,
        AppleCoreStorageContainerMacOS,
        AppleAPFSPrebootPartitionMacOS,
        AppleAPFSRecoveryPartitionMacOS,

        BootPartitionSolaris = 150,
        RootPartitionSolaris,
        SwapPartitionSolaris,
        BackupPartitionSolaris,
        UsrPartitionSolaris,
        VarPartitionSolaris,
        HomePartitionSolaris,
        AlternateSectorSolaris,
        ReservedPartitionSolaris,

        SwapPartitionNetBSD = 170,
        FFSPartitionNetBSD,
        LFSPartitionNetBSD,
        RAIDPartitionNetBSD,
        ConcatenatedPartitionNetBSD,
        EncryptedPartitionNetBSD,

        KernelChromeOS = 190,
        RootfsChromeOS,
        FirmwareChromeOS,
        FutureUseChromeOS,
        MiniOSChromeOS,
        HibernateChromeOS,

        UsrPartitionCoreOS = 220,
        ResizableRootfsCoreOS,
        OEMCustomizationsCoreOS,
        RootFilesystemOnRAIDCoreOS,

        HaikuBFSHaiku = 230,

        BootPartitionMidnightBSD = 240,
        DataPartitionMidnightBSD,
        SwapPartitionMidnightBSD,
        UnixFileSystemPartitionMidnightBSD,
        VinumVolumemanagerPartitionMidnightBSD,
        ZFSPartitionMidnightBSD,

        JournalCeph = 260,
        DmCryptJournalCeph,
        OSDCeph,
        DmCryptOSDCeph,
        DiskinCreationCeph,
        DmCryptDiskinCreationCeph,
        BlockCeph,
        BlockDBCeph,
        BlockWriteAheadlogCeph,
        LockboxForDmCryptKeysCeph,
        MultipathOSDCeph,
        MultipathJournalCeph,
        MultipathBlockCeph,
        MultipathBlockDBCeph,
        MultipathblockwriteAheadogCeph,
        DmCryptBlockCeph,
        DmCryptBlockDBCeph,
        DmCryptBlockWriteAheadlogCeph,
        DmCryptLUKSjournalCeph,
        DmCryptLUKSBlockCeph,
        DmCryptLUKSBlockDBCeph,
        DmCryptLUKSBlockwriteAheadlogCeph,
        DmCryptLUKSOSDCeph,

        DataPartitionOpenBSD = 300,

        PowerSafeFilesystemQNX = 360,

        Plan9PartitionPlan9 = 370,

        VmkCoreVMwareESX = 380,
        VMFSFilesystemPartitionVMwareESX,
        VMWareReservedVMwareESX,

        BootloaderAndroidIA = 400,
        Bootloader2AndroidIA,
        BootAndroidIA,
        RecoveryAndroidIA,
        MiscAndroidIA,
        MetadataAndroidIA,
        SystemAndroidIA,
        CacheAndroidIA,
        DataAndroidIA,
        PersistentAndroidIA,
        VendorAndroidIA,
        ConfigAndroidIA,
        FactoryAndroidIA,
        FactoryAltAndroidIA,
        FastbootOrTertiaryAndroidIA,
        OEMAndroidIA,

        AndroidMetaAndroid6Arm = 450,
        AndroidEXTAndroid6Arm,

        BootONIE = 460,
        ConfigONIE,

        PRePBootPowerPC = 470,

        SharedBootloaderConfigurationFreedesktop = 480,

        BasicDataPartitionAtariTOS = 490,

        EncryptedDataPartitionVeraCrypt = 500,

        ArcaOSType1OS2 = 510,

        SPDKBlockDeviceSPDK = 520,

        BareBoxStateBareboxBootloader = 530,

        UBootEnvironmentUBootBootloader = 540,

        StatusSoftRAID = 550,
        ScratchSoftRAID,
        VolumeSoftRAID,
        CacheSoftRAID,

        BootloaderFuchsiaStandard = 570,
        DurablemutableencryptedsystemdataFuchsiaStandard,
        DurablemutablebootloaderdataFuchsiaStandard,
        FactoryProvisionedreadOnlysystemdataFuchsiaStandard,
        FactoryProvisionedreadOnlybootloaderdataFuchsiaStandard,
        FuchsiaVolumeManagerFuchsiaStandard,
        VerifiedbootmetadataFuchsiaStandard,
        ZirconbootimageFuchsiaStandard,

        FuchsiaEspFuchsiaLegacy = 590,
        FuchsiaSystemFuchsiaLegacy,
        FuchsiaDataFuchsiaLegacy,
        FuchsiaInstallFuchsiaLegacy,
        FuchsiaBlobFuchsiaLegacy,
        FuchsiaFvmFuchsiaLegacy,
        ZirconbootimageSlotAFuchsiaLegacy,
        ZirconbootimageSlotBFuchsiaLegacy,
        ZirconbootimageSlotRFuchsiaLegacy,
        SysConfigFuchsiaLegacy,
        FactoryConfigFuchsiaLegacy,
        BootloaderFuchsiaLegacy,
        GuidTestFuchsiaLegacy,
        VerifiedbootmetadataSlotAFuchsiaLegacy,
        VerifiedbootmetadataSlotBFuchsiaLegacy,
        VerifiedbootmetadataSlotRFuchsiaLegacy,
        MiscFuchsiaLegacy,
        EmmcBoot1FuchsiaLegacy,
        EmmcBoot2FuchsiaLegacy,

    };
    Q_ENUM(GptPartitionType)

    quint64 flags() const;
    bool isContained() const;
    bool isContainer() const;
    QString name() const;
    uint number() const;
    quint64 offset() const;
    quint64 size() const;
    QString table() const;
    QString type() const;
    MbrPartitionType mbrType() const;
    GptPartitionType gptType() const;
    QString UUID() const;

    static QString mbrTypeDescription(MbrPartitionType type);
    static QString gptTypeDescription(GptPartitionType type);

public Q_SLOTS:
    void deletePartition(const QVariantMap &options);
    void resize(quint64 size, const QVariantMap &options);
    void setFlags(quint64 flags, const QVariantMap &options);
    void setName(const QString &name, const QVariantMap &options);
    void setType(const QString &type, const QVariantMap &options);
    void setType(Dtk::Mount::DBlockPartition::MbrPartitionType type, const QVariantMap &options);

Q_SIGNALS:
    void flagsChanged(quint64 flags);
    void isContainedChanged(bool isContained);
    void isContainerChanged(bool isContainer);
    void nameChanged(const QString &name);
    void numberChanged(uint number);
    void offsetChanged(quint64 offset);
    void sizeChanged(quint64 size);
    void typeChanged(const QString &type);
    void mbrPartitionTypeChanged(Dtk::Mount::DBlockPartition::MbrPartitionType type);
    void gptPartitionTypeChanged(Dtk::Mount::DBlockPartition::GptPartitionType type);
    void UUIDChanged(const QString &UUID);

private:
    explicit DBlockPartition(const QString &path, QObject *parent = nullptr);

    friend DExpected<DBlockPartition *> DDeviceManager::createBlockPartition(const QString &path, QObject *parent);
    friend DExpected<DBlockPartition *> DDeviceManager::createBlockPartitionByMountPoint(const QByteArray &path, QObject *parent);
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKPARTITION_H
