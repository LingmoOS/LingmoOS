/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitopticaldrive.h"

#include <QDebug>
#include <QProcess>

#ifdef EJECT_USING_DISKARBITRATION
// for QCFType:
#include <private/qcore_mac_p.h>
#else
#include <QStandardPaths>
#endif

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/scsi/IOSCSIMultimediaCommandsDevice.h>

using namespace Solid::Backends::IOKit;

class IOKitOpticalDrive::Private
{
public:
    Private(const IOKitDevice *device, const QVariantMap &devCharMap)
        : m_device(device)
        , m_deviceCharacteristics(devCharMap)
    {
    }
    virtual ~Private()
    {
    }

    QVariant property(const QString &key) const
    {
        return m_deviceCharacteristics.value(key);
    }

    const IOKitDevice *m_device;
    const QVariantMap m_deviceCharacteristics;

    static const QMap<Solid::OpticalDrive::MediumType, uint32_t> cdTypeMap;
    static const QMap<Solid::OpticalDrive::MediumType, uint32_t> dvdTypeMap;
    static const QMap<Solid::OpticalDrive::MediumType, uint32_t> bdTypeMap;

#ifdef EJECT_USING_DISKARBITRATION
    // DiskArbitration-based ejection based on the implementation in libcdio's osx.c
    // in turn based on VideoLAN (VLC) code.
    // Not activated by default ATM because I have only been able to test it with the
    // solid-hardware6 utility and that one remains stuck after a successful return
    // from IOKitOpticalDrive::eject(). It does so too when using the hdiutil external
    // utility which cannot be due to using QProcess (to the best of my knowledge).
    // NB: the full-fledged approach using a cancel sourc ref (cancel_signal) etc. may
    // well be too complicated.

    typedef struct DAContext {
        const IOKitDevice *device;
        int success;
        bool completed;
        DASessionRef session;
        CFRunLoopRef runloop;
        CFRunLoopSourceRef cancel_signal;
    } DAContext;

    static void cancelEjectRunloop(void *){};

    static void daEjectCallback(DADiskRef disk, DADissenterRef dissenter, void *context)
    {
        Q_UNUSED(disk);
        DAContext *daContext = static_cast<DAContext *>(context);

        if (dissenter) {
            CFStringRef status = DADissenterGetStatusString(dissenter);
            if (status) {
                qWarning() << "Warning while ejecting" << daContext->device->property("BSD Name").toString() << ":" << QString::fromCFString(status);
                CFRelease(status);
            }
        }

        daContext->success = dissenter ? false : true;
        daContext->completed = TRUE;
        CFRunLoopSourceSignal(daContext->cancel_signal);
        CFRunLoopWakeUp(daContext->runloop);
    }

    static void daUnmountCallback(DADiskRef disk, DADissenterRef dissenter, void *context)
    {
        DAContext *daContext = (DAContext *)context;

        if (!dissenter) {
            DADiskEject(disk, kDADiskEjectOptionDefault, daEjectCallback, context);
            daContext->success = (daContext->success == -1 ? true : daContext->success);
        } else {
            daContext->success = false;
            daContext->completed = true;
            CFRunLoopSourceSignal(daContext->cancel_signal);
            CFRunLoopWakeUp(daContext->runloop);
        }
    }

    bool eject(double timeoutSeconds)
    {
        CFDictionaryRef description = nullptr;
        CFRunLoopSourceContext cancelRunLoopSourceContext = {.perform = cancelEjectRunloop};
        DAContext daContext = {m_device, -1, false, 0, CFRunLoopGetCurrent(), 0};
        QCFType<CFRunLoopSourceRef> cancel = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &cancelRunLoopSourceContext);
        if (!(daContext.cancel_signal = cancel)) {
            qWarning() << Q_FUNC_INFO << "failed to create cancel runloop source";
            return false;
        }
        QCFType<DASessionRef> session = DASessionCreate(kCFAllocatorDefault);
        if (!(daContext.session = session)) {
            qWarning() << Q_FUNC_INFO << "failed to create DiskArbitration session";
            return false;
        }
        const QString devName = m_device->property(QStringLiteral("BSD Name")).toString();
        QCFType<DADiskRef> daRef = DADiskCreateFromBSDName(kCFAllocatorDefault, daContext.session, devName.toStdString().c_str());
        if (!daRef) {
            qWarning() << Q_FUNC_INFO << "failed to create DiskArbitration reference for" << devName;
            return false;
        }
        description = DADiskCopyDescription(daRef);
        if (description) {
            DASessionScheduleWithRunLoop(daContext.session, daContext.runloop, kCFRunLoopDefaultMode);
            CFRunLoopAddSource(daContext.runloop, daContext.cancel_signal, kCFRunLoopDefaultMode);
            if (CFDictionaryGetValueIfPresent(description, kDADiskDescriptionVolumePathKey, nullptr)) {
                DADiskUnmount(daRef, kDADiskUnmountOptionWhole, daUnmountCallback, &daContext);
            }
            DADiskEject(daRef, kDADiskEjectOptionDefault, daEjectCallback, &daContext);
            daContext.success = (daContext.success == -1 ? true : daContext.success);
            while (!daContext.completed) {
                if (CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeoutSeconds, true) == kCFRunLoopRunTimedOut) {
                    break;
                }
            }
            if (daContext.completed) {
                qWarning() << Q_FUNC_INFO << "ejected" << devName;
            } else {
                qWarning() << Q_FUNC_INFO << "timeout ejecting" << devName;
            }
            CFRunLoopRemoveSource(daContext.runloop, daContext.cancel_signal, kCFRunLoopDefaultMode);
            DASessionSetDispatchQueue(daContext.session, 0);
            DASessionUnscheduleFromRunLoop(daContext.session, daContext.runloop, kCFRunLoopDefaultMode);
            CFRelease(description);
        } else {
            qWarning() << Q_FUNC_INFO << "failed to fetch DiskArbitration description for" << devName;
        }
        return daContext.success == -1 ? false : daContext.success;
    }
#endif // EJECT_USING_DISKARBITRATION
};

const QMap<Solid::OpticalDrive::MediumType, uint32_t> IOKitOpticalDrive::Private::cdTypeMap = {
    {Solid::OpticalDrive::Cdr, kCDFeaturesWriteOnceMask},
    {Solid::OpticalDrive::Cdrw, kCDFeaturesReWriteableMask},
};
const QMap<Solid::OpticalDrive::MediumType, uint32_t> IOKitOpticalDrive::Private::dvdTypeMap = {
    {Solid::OpticalDrive::Dvd, kDVDFeaturesReadStructuresMask},
    {Solid::OpticalDrive::Dvdr, kDVDFeaturesWriteOnceMask},
    {Solid::OpticalDrive::Dvdrw, kDVDFeaturesReWriteableMask},
    {Solid::OpticalDrive::Dvdram, kDVDFeaturesRandomWriteableMask},
    {Solid::OpticalDrive::Dvdplusr, kDVDFeaturesPlusRMask},
    {Solid::OpticalDrive::Dvdplusrw, kDVDFeaturesPlusRWMask},
    // not supported:
    //         {Solid::OpticalDrive::Dvdplusdl, "dvdplusrdl"}
    //         {Solid::OpticalDrive::Dvdplusdlrw, "dvdplusrwdl"}
    {Solid::OpticalDrive::HdDvd, kDVDFeaturesHDReadMask},
    {Solid::OpticalDrive::HdDvdr, kDVDFeaturesHDRMask},
    {Solid::OpticalDrive::HdDvdrw, kDVDFeaturesHDRWMask},
};
const QMap<Solid::OpticalDrive::MediumType, uint32_t> IOKitOpticalDrive::Private::bdTypeMap = {
    {Solid::OpticalDrive::Bd, kBDFeaturesReadMask},
    {Solid::OpticalDrive::Bdr, kBDFeaturesWriteMask},
}; // also Solid::OpticalDrive::Bdre

IOKitOpticalDrive::IOKitOpticalDrive(IOKitDevice *device)
    : IOKitStorage(device)
{
    // walk up the IOKit chain to find the parent that has the "Device Characteristics" property
    // In the examples I've seen this is always the 2nd parent but if ever that turns out
    // to be non-guaranteed we'll need to do a true walk.
    IOKitDevice ioDVDServices(IOKitDevice(device->parentUdi()).parentUdi());
    QVariantMap devCharMap;
    if (!ioDVDServices.iOKitPropertyExists(QStringLiteral("Device Characteristics"))) {
        qWarning() << Q_FUNC_INFO << "Grandparent of" << m_device->udi() << "doesn't have the \"Device Characteristics\" but is" << ioDVDServices.udi();
    } else {
        const QVariant devCharVar = ioDVDServices.property(QStringLiteral("Device Characteristics"));
        devCharMap = devCharVar.toMap();
    }
    d = new Private(device, devCharMap);
}

IOKitOpticalDrive::~IOKitOpticalDrive()
{
}

/* clang-format off */
// // Example properties: QMap(("BSD Major", QVariant(int, 1))
//     ("BSD Minor", QVariant(int, 12))
//     ("BSD Name", QVariant(QString, "disk3"))
//     ("BSD Unit", QVariant(int, 3))
//     ("Content", QVariant(QString, "CD_partition_scheme"))
//     ("Content Hint", QVariant(QString, ""))
//     ("Ejectable", QVariant(bool, true))
//     ("IOBusyInterest", QVariant(QString, "IOCommand is not serializable"))
//     ("IOGeneralInterest", QVariant(QString, "IOCommand is not serializable"))
//     ("IOMediaIcon", QVariant(QVariantMap, QMap(("CFBundleIdentifier", QVariant(QString, "com.apple.iokit.IOCDStorageFamily"))
//         ("IOBundleResourceFile", QVariant(QString, "CD.icns")))))
//     ("Leaf", QVariant(bool, false))
//     ("Open", QVariant(bool, true))
//     ("Preferred Block Size", QVariant(qlonglong, 2352))
//     ("Removable", QVariant(bool, true))
//     ("Size", QVariant(qlonglong, 750932448))
//     ("TOC", QVariant(QByteArray, "\x00\xA7\x01\x01\x01\x10\x00\xA0\x00\x00\x00\x00\x01\x00\x00\x01\x12\x00\xA1\x00\x00\x00\x00\f\x00\x00\x01\x12\x00\xA2\x00\x00\x00\x00""F:J\x01\x12\x00\x01\x00\x00\x00\x00\x00\x02\x00\x01\x12\x00\x02\x00\x00\x00\x00\x07/\b\x01\x12\x00\x03\x00\x00\x00\x00\x12\b\x0E\x01\x12\x00\x04\x00\x00\x00\x00\x17\x12""0\x01\x12\x00\x05\x00\x00\x00\x00\x1B+ \x01\x12\x00\x06\x00\x00\x00\x00 \x11\n\x01\x12\x00\x07\x00\x00\x00\x00!-\n\x01\x12\x00\b\x00\x00\x00\x00'\f\x1F\x01\x12\x00\t\x00\x00\x00\x00-\x13;\x01\x12\x00\n\x00\x00\x00\x00""4%\x1E\x01\x12\x00\x0B\x00\x00\x00\x00""62 \x01\x12\x00\f\x00\x00\x00\x00""C\x06""E"))
//     ("Type", QVariant(QString, "CD-ROM"))
//     ("Whole", QVariant(bool, true))
//     ("Writable", QVariant(bool, false))
//     ("className", QVariant(QString, "IOCDMedia")))
// // related useful entry: QMap(("Device Characteristics", QVariant(QVariantMap, QMap(("Async Notification", QVariant(bool, false))
//         ("BD Features", QVariant(int, 0))
//         ("CD Features", QVariant(int, 2047))
//         ("DVD Features", QVariant(int, 503))
//         ("Fast Spindown", QVariant(bool, true))
//         ("Loading Mechanism", QVariant(QString, "Slot"))
//         ("Low Power Polling", QVariant(bool, false))
//         ("Power Off", QVariant(bool, true))
//         ("Product Name", QVariant(QString, "DVD-R   UJ-8A8"))
//         ("Product Revision Level", QVariant(QString, "HA13"))
//         ("Vendor Name", QVariant(QString, "MATSHITA")))))
//     ("IOCFPlugInTypes", QVariant(QVariantMap, QMap(("97ABCF2C-23CC-11D5-A0E8-003065704866", QVariant(QString,     "IOSCSIArchitectureModelFamily.kext/Contents/PlugIns/SCSITaskUserClient.kext/Contents/PlugIns/SCSITaskLib.plugin")))))
//     ("IOGeneralInterest", QVariant(QString, "IOCommand is not serializable"))
//     ("IOMatchCategory", QVariant(QString, "SCSITaskUserClientIniter"))
//     ("IOMinimumSegmentAlignmentByteCount", QVariant(qlonglong, 4))
//     ("IOUserClientClass", QVariant(QString, "SCSITaskUserClient"))
//     ("Protocol Characteristics", QVariant(QVariantMap, QMap(("AHCI Port Number", QVariant(qlonglong, 0))
//         ("ATAPI", QVariant(bool, true))
//         ("Physical Interconnect", QVariant(QString, "SATA"))
//         ("Physical Interconnect Location", QVariant(QString, "Internal"))
//         ("Port Speed", QVariant(QString, "1.5 Gigabit"))
//         ("Read Time Out Duration", QVariant(qlonglong, 15000))
//         ("Retry Count", QVariant(qlonglong, 1))
//         ("Write Time Out Duration", QVariant(qlonglong, 15000)))))
//     ("SCSITaskDeviceCategory", QVariant(QString, "SCSITaskAuthoringDevice"))
//     ("SCSITaskUserClient GUID", QVariant(QByteArray, "\x00]\x0F""F\x80\xFF\xFF\xFFg\xB6\xAB\x1B\x00\x00\x00\x00"))
//     ("className", QVariant(QString, "IODVDServices"))
//     ("device-type", QVariant(QString, "DVD")))
// //                       QMap(("CFBundleIdentifier", QVariant(QString, "com.apple.iokit.IODVDStorageFamily"))
//     ("IOClass", QVariant(QString, "IODVDBlockStorageDriver"))
//     ("IOGeneralInterest", QVariant(QString, "IOCommand is not serializable"))
//     ("IOMatchCategory", QVariant(QString, "IODefaultMatchCategory"))
//     ("IOProbeScore", QVariant(int, 0))
//     ("IOPropertyMatch", QVariant(QVariantMap, QMap(("device-type", QVariant(QString, "DVD")))))
//     ("IOProviderClass", QVariant(QString, "IODVDBlockStorageDevice"))
//     ("Statistics", QVariant(QVariantMap, QMap(("Bytes (Read)", QVariant(qlonglong, 578020608))
//         ("Bytes (Write)", QVariant(qlonglong, 0))
//         ("Errors (Read)", QVariant(qlonglong, 0))
//         ("Errors (Write)", QVariant(qlonglong, 0))
//         ("Latency Time (Read)", QVariant(qlonglong, 0))
//         ("Latency Time (Write)", QVariant(qlonglong, 0))
//         ("Operations (Read)", QVariant(qlonglong, 18475))
//         ("Operations (Write)", QVariant(qlonglong, 0))
//         ("Retries (Read)", QVariant(qlonglong, 0))
//         ("Retries (Write)", QVariant(qlonglong, 0))
//         ("Total Time (Read)", QVariant(qlonglong, 219944025102))
//         ("Total Time (Write)", QVariant(qlonglong, 0)))))
//     ("className", QVariant(QString, "IODVDBlockStorageDriver")))
/* clang-format on */

Solid::OpticalDrive::MediumTypes IOKitOpticalDrive::supportedMedia() const
{
    Solid::OpticalDrive::MediumTypes supported;

    uint32_t cdFeatures = d->property(QStringLiteral("CD Features")).toInt();
    uint32_t dvdFeatures = d->property(QStringLiteral("DVD Features")).toInt();
    uint32_t bdFeatures = d->property(QStringLiteral("BD Features")).toInt();

    qDebug() << Q_FUNC_INFO << "cdFeatures" << cdFeatures << "dvdFeatures" << dvdFeatures << "bdFeatures" << bdFeatures;

    for (auto it = d->cdTypeMap.cbegin(); it != d->cdTypeMap.cend(); ++it) {
        if (cdFeatures & it.value()) {
            supported |= it.key();
        }
    }
    for (auto it = d->dvdTypeMap.cbegin(); it != d->dvdTypeMap.cend(); ++it) {
        if (dvdFeatures & it.value()) {
            supported |= it.key();
        }
    }
    for (auto it = d->bdTypeMap.cbegin(); it != d->bdTypeMap.cend(); ++it) {
        const uint32_t value = it.value();
        if (bdFeatures & value) {
            supported |= it.key();
            if (value == kBDFeaturesWriteMask) {
                supported |= Solid::OpticalDrive::Bdre;
            }
        }
    }

    return supported;
}

int IOKitOpticalDrive::readSpeed() const
{
    return 0;
}

int IOKitOpticalDrive::writeSpeed() const
{
    return 0;
}

QList<int> IOKitOpticalDrive::writeSpeeds() const
{
    return {};
}

bool IOKitOpticalDrive::eject()
{
#ifdef EJECT_USING_DISKARBITRATION
    // give the devices 30 seconds to eject
    int error = !d->eject(30.0);
#else
    QProcess ejectJob;
    int error = ejectJob.execute(
        QStandardPaths::findExecutable(QStringLiteral("hdiutil")), //
        {QStringLiteral("detach"), QStringLiteral("-verbose"), QStringLiteral("/dev/") + m_device->property(QStringLiteral("BSD Name")).toString()});
    if (error) {
        qWarning() << "hdiutil returned" << error << "trying to eject" << m_device->product();
    }
#endif // EJECT_USING_DISKARBITRATION
    if (error) {
        Q_EMIT ejectDone(Solid::ErrorType::OperationFailed, QVariant(), m_device->udi());
        return false;
    } else {
        Q_EMIT ejectDone(Solid::ErrorType::NoError, QVariant(), m_device->udi());
        return true;
    }
}

#include "moc_iokitopticaldrive.cpp"
