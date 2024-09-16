// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dblockpartition_p.h"

DMOUNT_BEGIN_NAMESPACE

quint64 DBlockPartition::flags() const
{
    Q_D(const DBlockPartition);

    return d->dbus->flags();
}

bool DBlockPartition::isContained() const
{
    Q_D(const DBlockPartition);

    return d->dbus->isContained();
}

bool DBlockPartition::isContainer() const
{
    Q_D(const DBlockPartition);

    return d->dbus->isContainer();
}

QString DBlockPartition::name() const
{
    Q_D(const DBlockPartition);

    return d->dbus->name();
}

uint DBlockPartition::number() const
{
    Q_D(const DBlockPartition);

    return d->dbus->number();
}

quint64 DBlockPartition::offset() const
{
    Q_D(const DBlockPartition);

    return d->dbus->offset();
}

quint64 DBlockPartition::size() const
{
    Q_D(const DBlockPartition);

    return d->dbus->size();
}

QString DBlockPartition::table() const
{
    Q_D(const DBlockPartition);

    return d->dbus->table().path();
}

QString DBlockPartition::type() const
{
    Q_D(const DBlockPartition);

    return d->dbus->type();
}

DBlockPartition::MbrPartitionType DBlockPartition::mbrType() const
{
    const QString &type = this->type();

    if (type.isEmpty())
        return MbrPartitionType::EmptyMbrType;

    bool ok = false;
    int value = type.toInt(&ok, 16);

    if (!ok)
        return MbrPartitionType::UnknownMbrType;

    return static_cast<MbrPartitionType>(value);
}

DBlockPartition::GptPartitionType DBlockPartition::gptType() const
{
    static const QMap<QString, GptPartitionType> kGuidMap = {
        { "00000000-0000-0000-0000-000000000000", GptPartitionType::UnusedEntryNA },
        { "024DEE41-33E7-11D3-9D69-0008C781F39F", GptPartitionType::MBRPartitionSchemeNA },
        { "C12A7328-F81F-11D2-BA4B-00A0C93EC93B", GptPartitionType::EFISystemPartitionNA },
        { "21686148-6449-6E6F-744E-656564454649", GptPartitionType::BIOSBootPartitionNA },
        { "D3BFE2DE-3DAF-11DF-BA40-E3A556D89593", GptPartitionType::IntelFastFlashPartitionNA },
        { "F4019732-066E-4E12-8273-346C5641494F", GptPartitionType::SonyBootPartitionNA },
        { "BFBFAFE7-A34F-448A-9A5B-6213EB736C22", GptPartitionType::LenovoBootPartitionNA },
        { "E3C9E316-0B5C-4DB8-817D-F92DF00215AE", GptPartitionType::MicrosoftReservedPartitionWin },
        { "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7", GptPartitionType::BasicDataPartitionWin },
        { "5808C8AA-7E8F-42E0-85D2-E1E90434CFB3", GptPartitionType::LogicalDiskManagerMetaDataPartitionWin },
        { "AF9B60A0-1431-4F62-BC68-3311714A69AD", GptPartitionType::LogicalDiskManagerDataPartitionWin },
        { "DE94BBA4-06D1-4D40-A16A-BFD50179D6AC", GptPartitionType::WindowsRecoveryEnvironmentWin },
        { "37AFFC90-EF7D-4E96-91C3-2D7AE055B174", GptPartitionType::IBMGeneralParallelFileSystemPartitionWin },
        { "E75CAF8F-F680-4CEE-AFA3-B001E56EFC2D", GptPartitionType::StorageSpacesPartitionWin },
        { "558D43C5-A1AC-43C0-AAC8-D1472B2923D1", GptPartitionType::StorageReplicaPartitionWin },
        { "75894C1E-3AEB-11D3-B7C1-7B03A0000000", GptPartitionType::DataPartitionHPUX },
        { "E2A1E728-32E3-11D6-A682-7B03A0000000", GptPartitionType::ServicePartitionHPUX },
        { "0FC63DAF-8483-4772-8E79-3D69D8477DE4", GptPartitionType::LinuxFilesystemDataLinux },
        { "A19D880F-05FC-4D3B-A006-743F0F84911E", GptPartitionType::RAIDPartitionLinux },
        { "44479540-F297-41B2-9AF7-D131D5F0458A", GptPartitionType::RootPartitionX86Linux },
        { "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709", GptPartitionType::RootPartitionX8664Linux },
        { "69DAD710-2CE4-4E3C-B16C-21A1D49ABED3", GptPartitionType::RootPartitionArm32Linux },
        { "B921B045-1DF0-41C3-AF44-4C6F280D3FAE", GptPartitionType::RootPartitionArm64Linux },
        { "BC13C2FF-59E6-4262-A352-B275FD6F7172", GptPartitionType::BootPartitionLinux },
        { "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F", GptPartitionType::SwapPartitionLinux },
        { "E6D6D379-F507-44C2-A23C-238F2A3DF928", GptPartitionType::LogicalVolumeManagerPartitionLinux },
        { "933AC7E1-2EB4-4F13-B844-0E14E2AEF915", GptPartitionType::HomePartitionLinux },
        { "3B8F8425-20E0-4F3B-907F-1A25A76F98E8", GptPartitionType::ServerDataPartitionLinux },
        { "7FFEC5C9-2D00-49B7-8941-3EA10A5586B7", GptPartitionType::PlainDMCryptPartitionLinux },
        { "CA7D7CCB-63ED-4C53-861C-1742536059CC", GptPartitionType::LUKSPartitionLinux },
        { "8DA63339-0007-60C0-C436-083AC8230908", GptPartitionType::ReservedLinux },
        { "83BD6B9D-7F41-11DC-BE0B-001560B84F0F", GptPartitionType::BootPartitionFreeBSD },
        { "516E7CB4-6ECF-11D6-8FF8-00022D09712B", GptPartitionType::BSDDisklabelPartitionFreeBSD },
        { "516E7CB5-6ECF-11D6-8FF8-00022D09712B", GptPartitionType::SwapPartitionFreeBSD },
        { "516E7CB6-6ECF-11D6-8FF8-00022D09712B", GptPartitionType::UnixFileSystemPartitionFreeBSD },
        { "516E7CB8-6ECF-11D6-8FF8-00022D09712B", GptPartitionType::VinumVolumeManagerPartitionFreeBSD },
        { "516E7CBA-6ECF-11D6-8FF8-00022D09712B", GptPartitionType::ZFSPartitionFreeBSD },
        { "74BA7DD9-A689-11E1-BD04-00E081286ACF", GptPartitionType::NandfsPartitionFreeBSD },
        { "48465300-0000-11AA-AA11-00306543ECAC", GptPartitionType::HierarchialFileSystemPlusPartitionMacOS },
        { "7C3457EF-0000-11AA-AA11-00306543ECAC", GptPartitionType::AppleAPFSContainerMacOS },
        { "55465300-0000-11AA-AA11-00306543ECAC", GptPartitionType::AppleUFSContainerMacOS },
        { "6A898CC3-1DD2-11B2-99A6-080020736631", GptPartitionType::ZFSMacOS },
        { "52414944-0000-11AA-AA11-00306543ECAC", GptPartitionType::AppleRAIDPartitionMacOS },
        { "52414944-5F4F-11AA-AA11-00306543ECAC", GptPartitionType::AppleRAIDPartitionOfflineMacOS },
        { "426F6F74-0000-11AA-AA11-00306543ECAC", GptPartitionType::AppleBootPartitionMacOS },
        { "4C616265-6C00-11AA-AA11-00306543ECAC", GptPartitionType::AppleLabelMacOS },
        { "5265636F-7665-11AA-AA11-00306543ECAC", GptPartitionType::AppleTVRecoveryPartitionMacOS },
        { "53746F72-6167-11AA-AA11-00306543ECAC", GptPartitionType::AppleCoreStorageContainerMacOS },
        { "69646961-6700-11AA-AA11-00306543ECAC", GptPartitionType::AppleAPFSPrebootPartitionMacOS },
        { "52637672-7900-11AA-AA11-00306543ECAC", GptPartitionType::AppleAPFSRecoveryPartitionMacOS },
        { "6A82CB45-1DD2-11B2-99A6-080020736631", GptPartitionType::BootPartitionSolaris },
        { "6A85CF4D-1DD2-11B2-99A6-080020736631", GptPartitionType::RootPartitionSolaris },
        { "6A87C46F-1DD2-11B2-99A6-080020736631", GptPartitionType::SwapPartitionSolaris },
        { "6A8B642B-1DD2-11B2-99A6-080020736631", GptPartitionType::BackupPartitionSolaris },
        { "6A898CC3-1DD2-11B2-99A6-080020736631", GptPartitionType::UsrPartitionSolaris },
        { "6A8EF2E9-1DD2-11B2-99A6-080020736631", GptPartitionType::VarPartitionSolaris },
        { "6A90BA39-1DD2-11B2-99A6-080020736631", GptPartitionType::HomePartitionSolaris },
        { "6A9283A5-1DD2-11B2-99A6-080020736631", GptPartitionType::AlternateSectorSolaris },
        { "6A945A3B-1DD2-11B2-99A6-080020736631", GptPartitionType::ReservedPartitionSolaris },
        { "6A9630D1-1DD2-11B2-99A6-080020736631", GptPartitionType::ReservedPartitionSolaris },
        { "6A980767-1DD2-11B2-99A6-080020736631", GptPartitionType::ReservedPartitionSolaris },
        { "6A96237F-1DD2-11B2-99A6-080020736631", GptPartitionType::ReservedPartitionSolaris },
        { "6A8D2AC7-1DD2-11B2-99A6-080020736631", GptPartitionType::ReservedPartitionSolaris },
        { "49F48D32-B10E-11DC-B99B-0019D1879648", GptPartitionType::SwapPartitionNetBSD },
        { "49F48D5A-B10E-11DC-B99B-0019D1879648", GptPartitionType::FFSPartitionNetBSD },
        { "49F48D82-B10E-11DC-B99B-0019D1879648", GptPartitionType::LFSPartitionNetBSD },
        { "49F48DAA-B10E-11DC-B99B-0019D1879648", GptPartitionType::RAIDPartitionNetBSD },
        { "2DB519C4-B10F-11DC-B99B-0019D1879648", GptPartitionType::ConcatenatedPartitionNetBSD },
        { "2DB519EC-B10F-11DC-B99B-0019D1879648", GptPartitionType::EncryptedPartitionNetBSD },
        { "FE3A2A5D-4F32-41A7-B725-ACCC3285A309", GptPartitionType::KernelChromeOS },
        { "3CB8E202-3B7E-47DD-8A3C-7FF2A13CFCEC", GptPartitionType::RootfsChromeOS },
        { "CAB6E88E-ABF3-4102-A07A-D4BB9BE3C1D3", GptPartitionType::FirmwareChromeOS },
        { "2E0A753D-9E48-43B0-8337-B15192CB1B5E", GptPartitionType::FutureUseChromeOS },
        { "09845860-705F-4BB5-B16C-8A8A099CAF52", GptPartitionType::MiniOSChromeOS },
        { "3F0F8318-F146-4E6B-8222-C28C8F02E0D5", GptPartitionType::HibernateChromeOS },
        { "5DFBF5F4-2848-4BAC-AA5E-0D9A20B745A6", GptPartitionType::UsrPartitionCoreOS },
        { "3884DD41-8582-4404-B9A8-E9B84F2DF50E", GptPartitionType::ResizableRootfsCoreOS },
        { "C95DC21A-DF0E-4340-8D7B-26CBFA9A03E0", GptPartitionType::OEMCustomizationsCoreOS },
        { "BE9067B9-EA49-4F15-B4F6-F36F8C9E1818", GptPartitionType::RootFilesystemOnRAIDCoreOS },
        { "42465331-3BA3-10F1-802A-4861696B7521", GptPartitionType::HaikuBFSHaiku },
        { "85D5E45E-237C-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::BootPartitionMidnightBSD },
        { "85D5E45A-237C-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::DataPartitionMidnightBSD },
        { "85D5E45B-237C-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::SwapPartitionMidnightBSD },
        { "0394EF8B-237E-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::UnixFileSystemPartitionMidnightBSD },
        { "85D5E45C-237C-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::VinumVolumemanagerPartitionMidnightBSD },
        { "85D5E45D-237C-11E1-B4B3-E89A8F7FC3A7", GptPartitionType::ZFSPartitionMidnightBSD },
        { "45B0969E-9B03-4F30-B4C6-B4B80CEFF106", GptPartitionType::JournalCeph },
        { "45B0969E-9B03-4F30-B4C6-5EC00CEFF106", GptPartitionType::DmCryptJournalCeph },
        { "4FBD7E29-9D25-41B8-AFD0-062C0CEFF05D", GptPartitionType::OSDCeph },
        { "4FBD7E29-9D25-41B8-AFD0-5EC00CEFF05D", GptPartitionType::DmCryptOSDCeph },
        { "89C57F98-2FE5-4DC0-89C1-F3AD0CEFF2BE", GptPartitionType::DiskinCreationCeph },
        { "89C57F98-2FE5-4DC0-89C1-5EC00CEFF2BE", GptPartitionType::DmCryptDiskinCreationCeph },
        { "CAFECAFE-9B03-4F30-B4C6-B4B80CEFF106", GptPartitionType::BlockCeph },
        { "30CD0809-C2B2-499C-8879-2D6B78529876", GptPartitionType::BlockDBCeph },
        { "5CE17FCE-4087-4169-B7FF-056CC58473F9", GptPartitionType::BlockWriteAheadlogCeph },
        { "FB3AABF9-D25F-47CC-BF5E-721D1816496B", GptPartitionType::LockboxForDmCryptKeysCeph },
        { "4FBD7E29-8AE0-4982-BF9D-5A8D867AF560", GptPartitionType::MultipathOSDCeph },
        { "45B0969E-8AE0-4982-BF9D-5A8D867AF560", GptPartitionType::MultipathJournalCeph },
        { "CAFECAFE-8AE0-4982-BF9D-5A8D867AF560", GptPartitionType::MultipathBlockCeph },
        { "7F4A666A-16F3-47A2-8445-152EF4D03F6C", GptPartitionType::MultipathBlockCeph },
        { "EC6D6385-E346-45DC-BE91-DA2A7C8B3261", GptPartitionType::MultipathBlockDBCeph },
        { "01B41E1B-002A-453C-9F17-88793989FF8F", GptPartitionType::MultipathblockwriteAheadogCeph },
        { "CAFECAFE-9B03-4F30-B4C6-5EC00CEFF106", GptPartitionType::DmCryptBlockCeph },
        { "93B0052D-02D9-4D8A-A43B-33A3EE4DFBC3", GptPartitionType::DmCryptBlockDBCeph },
        { "306E8683-4FE2-4330-B7C0-00A917C16966", GptPartitionType::DmCryptBlockWriteAheadlogCeph },
        { "45B0969E-9B03-4F30-B4C6-35865CEFF106", GptPartitionType::DmCryptLUKSjournalCeph },
        { "CAFECAFE-9B03-4F30-B4C6-35865CEFF106", GptPartitionType::DmCryptLUKSBlockCeph },
        { "166418DA-C469-4022-ADF4-B30AFD37F176", GptPartitionType::DmCryptLUKSBlockDBCeph },
        { "86A32090-3647-40B9-BBBD-38D8C573AA86", GptPartitionType::DmCryptLUKSBlockwriteAheadlogCeph },
        { "4FBD7E29-9D25-41B8-AFD0-35865CEFF05D", GptPartitionType::DmCryptLUKSOSDCeph },
        { "824CC7A0-36A8-11E3-890A-952519AD3F61", GptPartitionType::DataPartitionOpenBSD },
        { "CEF5A9AD-73BC-4601-89F3-CDEEEEE321A1", GptPartitionType::PowerSafeFilesystemQNX },
        { "C91818F9-8025-47AF-89D2-F030D7000C2C", GptPartitionType::Plan9PartitionPlan9 },
        { "9D275380-40AD-11DB-BF97-000C2911D1B8", GptPartitionType::VmkCoreVMwareESX },
        { "AA31E02A-400F-11DB-9590-000C2911D1B8", GptPartitionType::VMFSFilesystemPartitionVMwareESX },
        { "9198EFFC-31C0-11DB-8F78-000C2911D1B8", GptPartitionType::VMWareReservedVMwareESX },
        { "2568845D-2332-4675-BC39-8FA5A4748D15", GptPartitionType::BootloaderAndroidIA },
        { "114EAFFE-1552-4022-B26E-9B053604CF84", GptPartitionType::Bootloader2AndroidIA },
        { "49A4D17F-93A3-45C1-A0DE-F50B2EBE2599", GptPartitionType::BootAndroidIA },
        { "4177C722-9E92-4AAB-8644-43502BFD5506", GptPartitionType::RecoveryAndroidIA },
        { "EF32A33B-A409-486C-9141-9FFB711F6266", GptPartitionType::MiscAndroidIA },
        { "20AC26BE-20B7-11E3-84C5-6CFDB94711E9", GptPartitionType::MetadataAndroidIA },
        { "38F428E6-D326-425D-9140-6E0EA133647C", GptPartitionType::SystemAndroidIA },
        { "A893EF21-E428-470A-9E55-0668FD91A2D9", GptPartitionType::CacheAndroidIA },
        { "DC76DDA9-5AC1-491C-AF42-A82591580C0D", GptPartitionType::DataAndroidIA },
        { "EBC597D0-2053-4B15-8B64-E0AAC75F4DB1", GptPartitionType::PersistentAndroidIA },
        { "C5A0AEEC-13EA-11E5-A1B1-001E67CA0C3C", GptPartitionType::VendorAndroidIA },
        { "BD59408B-4514-490D-BF12-9878D963F378", GptPartitionType::ConfigAndroidIA },
        { "8F68CC74-C5E5-48DA-BE91-A0C8C15E9C80", GptPartitionType::FactoryAndroidIA },
        { "9FDAA6EF-4B3F-40D2-BA8D-BFF16BFB887B", GptPartitionType::FactoryAltAndroidIA },
        { "767941D0-2085-11E3-AD3B-6CFDB94711E9", GptPartitionType::FastbootOrTertiaryAndroidIA },
        { "AC6D7924-EB71-4DF8-B48D-E267B27148FF", GptPartitionType::OEMAndroidIA },
        { "19A710A2-B3CA-11E4-B026-10604B889DCF", GptPartitionType::AndroidMetaAndroid6Arm },
        { "193D1EA4-B3CA-11E4-B075-10604B889DCF", GptPartitionType::AndroidEXTAndroid6Arm },
        { "7412F7D5-A156-4B13-81DC-867174929325", GptPartitionType::BootONIE },
        { "D4E6E2CD-4469-46F3-B5CB-1BFF57AFC149", GptPartitionType::ConfigONIE },
        { "9E1A2D38-C612-4316-AA26-8B49521E5A8B", GptPartitionType::PRePBootPowerPC },
        { "BC13C2FF-59E6-4262-A352-B275FD6F7172", GptPartitionType::SharedBootloaderConfigurationFreedesktop },
        { "734E5AFE-F61A-11E6-BC64-92361F002671", GptPartitionType::BasicDataPartitionAtariTOS },
        { "8C8F8EFF-AC95-4770-814A-21994F2DBC8F", GptPartitionType::EncryptedDataPartitionVeraCrypt },
        { "90B6FF38-B98F-4358-A21F-48F35B4A8AD3", GptPartitionType::ArcaOSType1OS2 },
        { "7C5222BD-8F5D-4087-9C00-BF9843C7B58C", GptPartitionType::SPDKBlockDeviceSPDK },
        { "4778ED65-BF42-45FA-9C5B-287A1DC4AAB1", GptPartitionType::BareBoxStateBareboxBootloader },
        { "3DE21764-95BD-54BD-A5C3-4ABE786F38A8", GptPartitionType::UBootEnvironmentUBootBootloader },
        { "B6FA30DA-92D2-4A9A-96F1-871EC6486200", GptPartitionType::StatusSoftRAID },
        { "2E313465-19B9-463F-8126-8A7993773801", GptPartitionType::ScratchSoftRAID },
        { "FA709C7E-65B1-4593-BFD5-E71D61DE9B02", GptPartitionType::VolumeSoftRAID },
        { "BBBA6DF5-F46F-4A89-8F59-8765B2727503", GptPartitionType::CacheSoftRAID },
        { "FE8A2634-5E2E-46BA-99E3-3A192091A350", GptPartitionType::BootloaderFuchsiaStandard },
        { "D9FD4535-106C-4CEC-8D37-DFC020CA87CB", GptPartitionType::DurablemutableencryptedsystemdataFuchsiaStandard },
        { "A409E16B-78AA-4ACC-995C-302352621A41", GptPartitionType::DurablemutablebootloaderdataFuchsiaStandard },
        { "F95D940E-CABA-4578-9B93-BB6C90F29D3E", GptPartitionType::FactoryProvisionedreadOnlysystemdataFuchsiaStandard },
        { "10B8DBAA-D2BF-42A9-98C6-A7C5DB3701E7", GptPartitionType::FactoryProvisionedreadOnlybootloaderdataFuchsiaStandard },
        { "49FD7CB8-DF15-4E73-B9D9-992070127F0F", GptPartitionType::FuchsiaVolumeManagerFuchsiaStandard },
        { "421A8BFC-85D9-4D85-ACDA-B64EEC0133E9", GptPartitionType::VerifiedbootmetadataFuchsiaStandard },
        { "9B37FFF6-2E58-466A-983A-F7926D0B04E0", GptPartitionType::ZirconbootimageFuchsiaStandard },
        { "C12A7328-F81F-11D2-BA4B-00A0C93EC93B", GptPartitionType::FuchsiaEspFuchsiaLegacy },
        { "606B000B-B7C7-4653-A7D5-B737332C899D", GptPartitionType::FuchsiaSystemFuchsiaLegacy },
        { "08185F0C-892D-428A-A789-DBEEC8F55E6A", GptPartitionType::FuchsiaDataFuchsiaLegacy },
        { "48435546-4953-2041-494E-5354414C4C52", GptPartitionType::FuchsiaInstallFuchsiaLegacy },
        { "2967380E-134C-4CBB-B6DA-17E7CE1CA45D", GptPartitionType::FuchsiaBlobFuchsiaLegacy },
        { "41D0E340-57E3-954E-8C1E-17ECAC44CFF5", GptPartitionType::FuchsiaFvmFuchsiaLegacy },
        { "DE30CC86-1F4A-4A31-93C4-66F147D33E05", GptPartitionType::ZirconbootimageSlotAFuchsiaLegacy },
        { "23CC04DF-C278-4CE7-8471-897D1A4BCDF7", GptPartitionType::ZirconbootimageSlotBFuchsiaLegacy },
        { "A0E5CF57-2DEF-46BE-A80C-A2067C37CD49", GptPartitionType::ZirconbootimageSlotRFuchsiaLegacy },
        { "4E5E989E-4C86-11E8-A15B-480FCF35F8E6", GptPartitionType::SysConfigFuchsiaLegacy },
        { "5A3A90BE-4C86-11E8-A15B-480FCF35F8E6", GptPartitionType::FactoryConfigFuchsiaLegacy },
        { "5ECE94FE-4C86-11E8-A15B-480FCF35F8E6", GptPartitionType::BootloaderFuchsiaLegacy },
        { "8B94D043-30BE-4871-9DFA-D69556E8C1F3", GptPartitionType::GuidTestFuchsiaLegacy },
        { "A13B4D9A-EC5F-11E8-97D8-6C3BE52705BF", GptPartitionType::VerifiedbootmetadataSlotAFuchsiaLegacy },
        { "A288ABF2-EC5F-11E8-97D8-6C3BE52705BF", GptPartitionType::VerifiedbootmetadataSlotBFuchsiaLegacy },
        { "6A2460C3-CD11-4E8B-80A8-12CCE268ED0A", GptPartitionType::VerifiedbootmetadataSlotRFuchsiaLegacy },
        { "1D75395D-F2C6-476B-A8B7-45CC1C97B476", GptPartitionType::MiscFuchsiaLegacy },
        { "900B0FC5-90CD-4D4F-84F9-9F8ED579DB88", GptPartitionType::EmmcBoot1FuchsiaLegacy },
        { "B2B2E8D1-7C10-4EBC-A2D0-4614568260AD", GptPartitionType::EmmcBoot2FuchsiaLegacy },
    };

    const QString &guid = this->type();

    if (guid.isEmpty())
        return GptPartitionType::EmptyGptType;

    if (!kGuidMap.contains(guid))
        return GptPartitionType::UnknownGptType;

    return kGuidMap.value(guid);
}

QString DBlockPartition::UUID() const
{
    Q_D(const DBlockPartition);

    return d->dbus->uUID();
}

QString DBlockPartition::mbrTypeDescription(DBlockPartition::MbrPartitionType type)
{
    switch (type) {
    case EmptyMbrType:
        return "Empty";
    case FAT12Type:
        return "FAT12";
    case XENIX_root:
        return "XENIX root";
    case XENIX_usr:
        return "XENIX usr";
    case FAT16_Less_32M:
        return "FAT16 <32M";
    case Extended:
        return "Extended";
    case FAT16Type:
        return "FAT16";
    case HPFS_NTFS:
        return "HPFS/NTFS";
    case AIX:
        return "AIX";
    case AIX_bootable:
        return "AIX bootable";
    case OS2_Boot_Manager:
        return "OS2 Boot Manager";
    case Win95_FAT32:
        return "Win95 FAT32";
    case Win95_FAT32_LBA:
        return "Win95 FAT32 (LBA)";
    case Win95_FAT16_LBA:
        return "Win95 FAT16 (LBA)";
    case Win95_Extended_LBA:
        return "Win95 Ext'd (LBA)";
    case OPUS:
        return "OPUS";
    case Hidden_FAT12:
        return "Hidden FAT12";
    case Compaq_diagnostics:
        return "Compaq diagnostics";
    case Hidden_FAT16_Less_32M:
        return "Hidden FAT16 <32M";
    case Hidden_FAT16:
        return "Hidden FAT16";
    case Hidden_HPFS_or_NTFS:
        return "Hidden HPFS/NTFS";
    case AST_SmartSleep:
        return "AST SmartSleep";
    case Hidden_Win95_FAT32:
        return "Hidden Win95 FAT32";
    case Hidden_Win95_FAT32_LBA:
        return "Hidden Win95 FAT32 (LBA)";
    case Hidden_Win95_FAT16_LBA:
        return "Hidden Win95 FAT16";
    case NEC_DOS:
        return "NEC DOS";
    case Plan9:
        return "Plan 9";
    case PartitionMagic_recovery:
        return "PartitionMagic recovery";
    case Venix_80286:
        return "Venix 80286";
    case PPC_PReP_Boot:
        return "PPC PReP Boot";
    case SFS:
        return "SFS";
    case QNX4_dot_x:
        return "QNX4.x";
    case QNX4_dot_x_2nd_part:
        return "QNX4.x.2nd part";
    case QNX4_dot_x_3rd_part:
        return "QNX4.x 3rd part";
    case OnTrack_DM:
        return "OnTrack DM";
    case OnTrack_DM6_Aux1:
        return "OnTrack DM6 Aux1";
    case CP_M:
        return "CP/M";
    case OnTrack_DM6_Aux3:
        return "OnTrack DM6 Aux3";
    case OnTrackDM6:
        return "OnTrackDM6";
    case EZ_Drive:
        return "EZ-Drive";
    case Golden_Bow:
        return "Golden Bow";
    case Priam_Edisk:
        return "Priam Edisk";
    case SpeedStor:
        return "SpeedStor";
    case GNU_HURD_or_SysV:
        return "GNU HURD or SysV";
    case Novell_Netware_286:
        return "Novell Netware 286";
    case Novell_Netware_386:
        return "Novell Netware 386";
    case DiskSecure_Multi_Boot:
        return "DiskSecure Multi-Boot";
    case PC_IX:
        return "PC/IX";
    case Old_Minix:
        return "Old Minix";
    case Minix_old_Linux:
        return "Minix / old Linux";
    case Linux_swap:
        return "Linux swap";
    case Linux:
        return "Linux";
    case OS2_hidden_C_drive:
        return "OS/2 hidden C: drive";
    case Linux_extended:
        return "Linux extended";
    case NTFS_volume_set_1:
        return "NTFS volume set";
    case NTFS_volume_set_2:
        return "NTFS volume set";
    case Linux_LVM:
        return "Linux LVM";
    case Amoeba:
        return "Amoeba";
    case Amoeba_BBT:
        return "Amoeba BBT";
    case BSD_OS:
        return "BSD/OS";
    case IBM_Thinkpad_hibernation:
        return "IBM Thinkpad hibernation";
    case FreeBSD:
        return "FreeBSD";
    case OpenBSD:
        return "OpenBSD";
    case NeXTSTEP:
        return "NeXTSTEP";
    case NetBSD:
        return "NetBSD";
    case BSDI_fs:
        return "BSDI fs";
    case BSDI_swap:
        return "BSDI swap";
    case Boot_Wizard_hidden:
        return "Boot Wizard hidden";
    case DRDOS_sec_FAT12:
        return "DRDOS/sec (FAT-12)";
    case DRDOS_sec_FAT16_Less_32M:
        return "DRDOS/sec (FAT-16 < 32M)";
    case DRDOS_sec_FAT16:
        return "DRDOS/sec (FAT-16)";
    case Syrinx:
        return "Syrinx";
    case Non_FS_data:
        return "Non-FS data";
    case CP_M_CTOS_dot_dot_dot:
        return "CP/M / CTOS / ...";
    case Dell_Utility:
        return "Dell Utility";
    case BootIt:
        return "BootIt";
    case DOS_access:
        return "DOS access";
    case DOS_R_O:
        return "DOS R/O";
    case SpeedStor_1:
        return "SpeedStor";
    case BeOS_fs:
        return "BeOS fs";
    case EFI_GPT:
        return "EFI GPT";
    case EFI_FAT12_16_32:
        return "EFI (FAT-12/16/32)";
    case Linux_PA_RISC_boot:
        return "Linux/PA-RISC boot";
    case SpeedStor_2:
        return "SpeedStor";
    case SeppdStor_3:
        return "SpeedStor";
    case DOS_secondary:
        return "DOS secondary";
    case Linux_raid_autodetect:
        return "Linux raid autodetect";
    case LANstep:
        return "LANstep";
    case BBT:
        return "BBT";
    default:
        break;
    }

    return QString();
}

QString DBlockPartition::gptTypeDescription(DBlockPartition::GptPartitionType type)
{
    switch (type) {
    case UnusedEntryNA:
        return "Unused entry";
    case MBRPartitionSchemeNA:
        return "MBR partition scheme";
    case EFISystemPartitionNA:
        return "EFI System partition";
    case BIOSBootPartitionNA:
        return "BIOS boot partition";
    case IntelFastFlashPartitionNA:
        return "Intel Fast Flash (iFFS) partition (for Intel Rapid Start technology)";
    case SonyBootPartitionNA:
        return "Sony boot partition";
    case LenovoBootPartitionNA:
        return "Lenovo boot partition";
    // Windows
    case MicrosoftReservedPartitionWin:
        return "Microsoft Reserved Partition (MSR)";
    case BasicDataPartitionWin:
        return "Basic data partition of Windows";
    case LogicalDiskManagerMetaDataPartitionWin:
        return "Logical Disk Manager (LDM) metadata partition of Windows";
    case LogicalDiskManagerDataPartitionWin:
        return "Logical Disk Manager data partition of Windows";
    case WindowsRecoveryEnvironmentWin:
        return "Windows Recovery Environment";
    case IBMGeneralParallelFileSystemPartitionWin:
        return "IBM General Parallel File System (GPFS) partition of Windows";
    case StorageSpacesPartitionWin:
        return "Storage Spaces partition of Windows";
    case StorageReplicaPartitionWin:
        return "Storage Replica partition of Windows";
    // HP-UX
    case DataPartitionHPUX:
        return "Data partition of HP-UX";
    case ServicePartitionHPUX:
        return "Service Partition of HP-UX";
    // Linux
    case LinuxFilesystemDataLinux:
        return "Linux filesystem data";
    case RAIDPartitionLinux:
        return "RAID partition of Linux";
    case RootPartitionX86Linux:
        return "Root partition (x86) of Linux";
    case RootPartitionX8664Linux:
        return "Root partition (x86-64) of Linux";
    case RootPartitionArm32Linux:
        return "Root partition (32-bit ARM) of Linux";
    case RootPartitionArm64Linux:
        return "Root partition (64-bit ARM/AArch64) of Linux";
    case BootPartitionLinux:
        return "Boot partition of Linux";
    case SwapPartitionLinux:
        return "Swap partition of Linux";
    case LogicalVolumeManagerPartitionLinux:
        return "Logical Volume Manager (LVM) partition of Linux";
    case HomePartitionLinux:
        return "/home partition of Linux";
    case ServerDataPartitionLinux:
        return "/srv (server data) partition of Linux";
    case PlainDMCryptPartitionLinux:
        return "Plain dm-crypt partition of Linux";
    case LUKSPartitionLinux:
        return "LUKS partition of Linux";
    case ReservedLinux:
        return "Reserved of Linux";
    // FreeBSD
    case BootPartitionFreeBSD:
        return "Boot partition of FreeBSD";
    case BSDDisklabelPartitionFreeBSD:
        return "Data partition of FreeBSD";
    case SwapPartitionFreeBSD:
        return "Swap partition of FreeBSD";
    case UnixFileSystemPartitionFreeBSD:
        return "Unix File System (UFS) partition of FreeBSD";
    case VinumVolumeManagerPartitionFreeBSD:
        return "Vinum volume manager partition of FreeBSD";
    case ZFSPartitionFreeBSD:
        return "ZFS partition of FreeBSD";
    case NandfsPartitionFreeBSD:
        return "nandfs partition of FreeBSD";
    // macOS Darwin
    case HierarchialFileSystemPlusPartitionMacOS:
        return "Hierarchical File System Plus (HFS+) partition of macOS";
    case AppleAPFSContainerMacOS:
        return "Apple UFS";
    case AppleUFSContainerMacOS:
        return "Apple UFS";
    case ZFSMacOS:
        return "ZFS of macOS(Or /usr partition of Solaris illumos)";
    case AppleRAIDPartitionMacOS:
        return "Apple RAID partition";
    case AppleRAIDPartitionOfflineMacOS:
        return "Apple RAID partition, offline";
    case AppleBootPartitionMacOS:
        return "Apple Boot partition (Recovery HD)";
    case AppleLabelMacOS:
        return "Apple Label";
    case AppleTVRecoveryPartitionMacOS:
        return "Apple TV Recovery partition";
    case AppleCoreStorageContainerMacOS:
        return "Apple Core Storage (i.e. Lion FileVault) partition";
    case AppleAPFSPrebootPartitionMacOS:
        return "Apple APFS Preboot partition";
    case AppleAPFSRecoveryPartitionMacOS:
        return "Apple APFS Recovery partition";
    // Solaris illumos
    case BootPartitionSolaris:
        return "Boot partition of Solaris illumos";
    case RootPartitionSolaris:
        return "Root partition of Solaris illumos";
    case SwapPartitionSolaris:
        return "Swap partition of Solaris illumos";
    case BackupPartitionSolaris:
        return "Backup partition of Solaris illumos";
    case UsrPartitionSolaris:
        return "/usr partition of Solaris illumos";
    case VarPartitionSolaris:
        return "/var partition of Solaris illumos";
    case HomePartitionSolaris:
        return "/home partition of Solaris illumos";
    case AlternateSectorSolaris:
        return "Alternate sector os Solaris illumos";
    case ReservedPartitionSolaris:
        return "Reserved partition os Solaris illumos";
    // NetBSD
    case SwapPartitionNetBSD:
        return "Swap partition of NetBSD";
    case FFSPartitionNetBSD:
        return "FFS partition of NetBSD";
    case LFSPartitionNetBSD:
        return "LFS partition of NetBSD";
    case RAIDPartitionNetBSD:
        return "RAID partition of NetBSD";
    case ConcatenatedPartitionNetBSD:
        return "Concatenated partition of NetBSD";
    case EncryptedPartitionNetBSD:
        return "Encrypted partition of NetBSD";
    // ChromeOS
    case KernelChromeOS:
        return "ChromeOS kernel";
    case RootfsChromeOS:
        return "ChromeOS rootfs";
    case FirmwareChromeOS:
        return "Chrome OS firmware";
    case FutureUseChromeOS:
        return "ChromeOS future use";
    case MiniOSChromeOS:
        return "Chrome OS miniOS";
    case HibernateChromeOS:
        return "Chrome OS hibernate";
    // Container Linux by CoreOS
    case UsrPartitionCoreOS:
        return "/usr partition (coreos-usr)";
    case ResizableRootfsCoreOS:
        return "Resizable rootfs (coreos-resize)";
    case OEMCustomizationsCoreOS:
        return "OEM customizations (coreos-reserved)";
    case RootFilesystemOnRAIDCoreOS:
        return "Root filesystem on RAID (coreos-root-raid)";
    // Haiku
    case HaikuBFSHaiku:
        return "Haiku BFS";
    // MidnightBSD
    case BootPartitionMidnightBSD:
        return "Boot partition";
    case DataPartitionMidnightBSD:
        return "Data partition";
    case SwapPartitionMidnightBSD:
        return "Swap partition";
    case UnixFileSystemPartitionMidnightBSD:
        return "Unix File System (UFS) partition";
    case VinumVolumemanagerPartitionMidnightBSD:
        return "Vinum volume manager partition";
    case ZFSPartitionMidnightBSD:
        return "ZFS partition";
    // Ceph
    case JournalCeph:
        return "Ceph Journal";
    case DmCryptJournalCeph:
        return "dm-crypt journal";
    case OSDCeph:
        return "Ceph OSD";
    case DmCryptOSDCeph:
        return "Ceph dm-crypt OSD";
    case DiskinCreationCeph:
        return "Ceph Disk in creation";
    case DmCryptDiskinCreationCeph:
        return "Ceph dm-crypt disk in creation";
    case BlockCeph:
        return "Ceph Block";
    case BlockDBCeph:
        return "Ceph Block DB";
    case BlockWriteAheadlogCeph:
        return "Ceph Block write-ahead log";
    case LockboxForDmCryptKeysCeph:
        return "Ceph Lockbox for dm-crypt keys";
    case MultipathOSDCeph:
        return "Ceph Multipath OSD";
    case MultipathJournalCeph:
        return "Ceph Multipath journal";
    case MultipathBlockCeph:
        return "Ceph Multipath block";
    case MultipathBlockDBCeph:
        return "Ceph Multipath block";
    case MultipathblockwriteAheadogCeph:
        return "Ceph Multipath block DB";
    case DmCryptBlockCeph:
        return "Ceph Multipath block write-ahead log";
    case DmCryptBlockDBCeph:
        return "Ceph dm-crypt block";
    case DmCryptBlockWriteAheadlogCeph:
        return "Ceph dm-crypt block DB";
    case DmCryptLUKSjournalCeph:
        return "Ceph dm-crypt block write-ahead log";
    case DmCryptLUKSBlockCeph:
        return "Ceph dm-crypt LUKS journal";
    case DmCryptLUKSBlockDBCeph:
        return "Ceph dm-crypt LUKS block";
    case DmCryptLUKSBlockwriteAheadlogCeph:
        return "Ceph dm-crypt LUKS block DB";
    case DmCryptLUKSOSDCeph:
        return "Ceph dm-crypt LUKS OSD";
    // OpenBSD
    case DataPartitionOpenBSD:
        return "Data partition of OpenBSD";
    // QNX
    case PowerSafeFilesystemQNX:
        return "Power-safe (QNX6) file system of QNX";
    // Plan9
    case Plan9PartitionPlan9:
        return "Plan 9 partition of Plan9";
    // VMware ESX
    case VmkCoreVMwareESX:
        return "vmkcore (coredump partition)";
    case VMFSFilesystemPartitionVMwareESX:
        return "VMFS filesystem partition";
    case VMWareReservedVMwareESX:
        return "VMware Reserved";
    // Android-IA
    case BootloaderAndroidIA:
        return "Bootloader of Android-IA";
    case Bootloader2AndroidIA:
        return "Bootloader2 of Android-IA";
    case BootAndroidIA:
        return "Boot of Android-IA";
    case RecoveryAndroidIA:
        return "Recovery of Android-IA";
    case MiscAndroidIA:
        return "Misc of Android-IA";
    case MetadataAndroidIA:
        return "Metadata of Android-IA";
    case SystemAndroidIA:
        return "System of Android-IA";
    case CacheAndroidIA:
        return "Cache of Android-IA";
    case DataAndroidIA:
        return "Data of Android-IA";
    case PersistentAndroidIA:
        return "Persistent of Android-IA";
    case VendorAndroidIA:
        return "Vendor of Android-IA";
    case ConfigAndroidIA:
        return "Config of Android-IA";
    case FactoryAndroidIA:
        return "Factory of Android-IA";
    case FactoryAltAndroidIA:
        return "Factory (alt) of Android-IA";
    case FastbootOrTertiaryAndroidIA:
        return "Fastboot / Tertiary of Android-IA";
    case OEMAndroidIA:
        return "OEM of Android-IA";
    // Android 6.0+ ARM
    case AndroidMetaAndroid6Arm:
        return "Android Meta of Android 6.0+ ARM";
    case AndroidEXTAndroid6Arm:
        return "Android EXT of Android 6.0+ ARM";
    // Open Network Install Environment (ONIE)
    case BootONIE:
        return "Open Network Install Environment Boot";
    case ConfigONIE:
        return "Open Network Install Environment Config";
    // PowerPC
    case PRePBootPowerPC:
        return "PowerPC PReP boot";
    // freedesktop.org OSes (Linux, etc.)
    case SharedBootloaderConfigurationFreedesktop:
        return "Shared boot loader configuration of freedesktop.org OSes (Linux, etc.)";
    // Atari TOS
    case BasicDataPartitionAtariTOS:
        return "Basic data partition (GEM, BGM, F32)";
    // VeraCrypt
    case EncryptedDataPartitionVeraCrypt:
        return "Encrypted data partition";
    // OS/2
    case ArcaOSType1OS2:
        return "ArcaOS Type 1";
    // Storage Performance Development Kit (SPDK)
    case SPDKBlockDeviceSPDK:
        return "SPDK block device";
    // barebox bootloader
    case BareBoxStateBareboxBootloader:
        return "barebox-state";
    // U-Boot bootloader
    case UBootEnvironmentUBootBootloader:
        return "U-Boot environment";
    // SoftRAID
    case StatusSoftRAID:
        return "SoftRAID_Status";
    case ScratchSoftRAID:
        return "SoftRAID_Scratch";
    case VolumeSoftRAID:
        return "SoftRAID_Volume";
    case CacheSoftRAID:
        return "SoftRAID_Cache";
    // Fuchsia standard partitions
    case BootloaderFuchsiaStandard:
        return "Bootloader (slot A/B/R)";
    case DurablemutableencryptedsystemdataFuchsiaStandard:
        return "Durable mutable encrypted system data";
    case DurablemutablebootloaderdataFuchsiaStandard:
        return "Durable mutable bootloader data (including A/B/R metadata)";
    case FactoryProvisionedreadOnlysystemdataFuchsiaStandard:
        return "Factory-provisioned read-only system data";
    case FactoryProvisionedreadOnlybootloaderdataFuchsiaStandard:
        return "Factory-provisioned read-only bootloader data";
    case FuchsiaVolumeManagerFuchsiaStandard:
        return "Fuchsia Volume Manager";
    case VerifiedbootmetadataFuchsiaStandard:
        return "Verified boot metadata (slot A/B/R)";
    case ZirconbootimageFuchsiaStandard:
        return "Zircon boot image (slot A/B/R)";
    // Fuchsia legacy partition
    case FuchsiaEspFuchsiaLegacy:
        return "fuchsia-esp";
    case FuchsiaSystemFuchsiaLegacy:
        return "fuchsia-system";
    case FuchsiaDataFuchsiaLegacy:
        return "fuchsia-data";
    case FuchsiaInstallFuchsiaLegacy:
        return "fuchsia-install";
    case FuchsiaBlobFuchsiaLegacy:
        return "fuchsia-blob";
    case FuchsiaFvmFuchsiaLegacy:
        return "fuchsia-fvm";
    case ZirconbootimageSlotAFuchsiaLegacy:
        return "Zircon boot image (slot A)";
    case ZirconbootimageSlotBFuchsiaLegacy:
        return "Zircon boot image (slot B)";
    case ZirconbootimageSlotRFuchsiaLegacy:
        return "Zircon boot image (slot R)";
    case SysConfigFuchsiaLegacy:
        return "sys-config";
    case FactoryConfigFuchsiaLegacy:
        return "factory-config";
    case BootloaderFuchsiaLegacy:
        return "bootloader";
    case GuidTestFuchsiaLegacy:
        return "guid-test";
    case VerifiedbootmetadataSlotAFuchsiaLegacy:
        return "Verified boot metadata (slot A)";
    case VerifiedbootmetadataSlotBFuchsiaLegacy:
        return "Verified boot metadata (slot B)";
    case VerifiedbootmetadataSlotRFuchsiaLegacy:
        return "Verified boot metadata (slot R)";
    case MiscFuchsiaLegacy:
        return "misc";
    case EmmcBoot1FuchsiaLegacy:
        return "emmc-boot1";
    case EmmcBoot2FuchsiaLegacy:
        return "emmc-boot2";

    case EmptyGptType:
        return "Empty";
    case UnknownGptType:
        return "Unknow GUID";
    }

    return "Invalid GUID type";
}

void DBlockPartition::deletePartition(const QVariantMap &options)
{
    Q_D(DBlockPartition);

    d->dbus->Delete(options);
}

void DBlockPartition::resize(quint64 size, const QVariantMap &options)
{
    Q_D(DBlockPartition);

    d->dbus->Resize(size, options);
}

void DBlockPartition::setFlags(quint64 flags, const QVariantMap &options)
{
    Q_D(DBlockPartition);

    d->dbus->SetFlags(flags, options);
}

void DBlockPartition::setName(const QString &name, const QVariantMap &options)
{
    Q_D(DBlockPartition);

    d->dbus->SetName(name, options);
}

void DBlockPartition::setType(const QString &type, const QVariantMap &options)
{
    Q_D(DBlockPartition);

    d->dbus->SetType(type, options);
}

void DBlockPartition::setType(DBlockPartition::MbrPartitionType type, const QVariantMap &options)
{
    if (type == UnknownMbrType)
        return;

    QString typeString = QString::asprintf("0x%.2s", QByteArray::number(type, 16).constData());

    typeString.replace(" ", "0");
    setType(typeString, options);
}

DBlockPartition::DBlockPartition(const QString &path, QObject *parent)
    : DBlockDevice(*new DBlockPartitionPrivate(this), path, parent)
{
    d_func()->dbus = new OrgFreedesktopUDisks2PartitionInterface(kUDisks2Service, path, QDBusConnection::systemBus(), this);

    // BUG(xust): don't report mbr/gpt typechanged sametime
    //    connect(this, &DBlockPartition::typeChanged, this, &DBlockPartition::mbrPartitionTypeChanged);
    //    connect(this, &DBlockPartition::UUIDChanged, this, &DBlockPartition::gptPartitionTypeChanged);
}

DBlockPartitionPrivate::DBlockPartitionPrivate(DBlockPartition *qq)
    : DBlockDevicePrivate(qq)
{
}

DMOUNT_END_NAMESPACE
