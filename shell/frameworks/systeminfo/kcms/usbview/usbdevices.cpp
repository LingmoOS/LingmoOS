/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "usbdevices.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QDebug>
#include <QFile>
#include <QHash>

#include <KLocalizedString>

#include "usbdb.h"

// FreeBSD has its own libusb implementation that does not provide
// libusb_get_parent(), even if the advertised LIBUSB_API_VERSION
// is the same as when libusb upstream introduced libusb_get_parent().
#ifdef Q_OS_FREEBSD

#define libusb_get_parent(device) nullptr

#endif

QList<USBDevice *> USBDevice::_devices;
libusb_context *USBDevice::_context;
USBDB *USBDevice::_db;

static double convertLibusbSpeed(int speed)
{
    switch (speed) {
    case LIBUSB_SPEED_UNKNOWN:
        // not using default here to catch future enum values
        return 0;
    case LIBUSB_SPEED_LOW:
        return 1.5;
    case LIBUSB_SPEED_FULL:
        return 12;
    case LIBUSB_SPEED_HIGH:
        return 480;
    case LIBUSB_SPEED_SUPER:
        return 5000;
#if LIBUSB_API_VERSION >= 0x01000106
    case LIBUSB_SPEED_SUPER_PLUS:
        return 10000;
#endif
    }
    return 0;
};

static void convertLibusbUsbVersion(uint16_t bcdUSB, unsigned int *verMajor, unsigned int *verMinor)
{
    *verMajor = bcdUSB >> 8;
    *verMinor = ((bcdUSB & 0xf0) >> 4) * 10 + (bcdUSB & 0xf);
}

static QString prettyLibusbClassName(int class_code)
{
    switch (class_code) {
    case LIBUSB_CLASS_PER_INTERFACE:
        return i18nc("USB device class", "(Defined at Interface level)");
    case LIBUSB_CLASS_AUDIO:
        return i18nc("USB device class", "Audio");
    case LIBUSB_CLASS_COMM:
        return i18nc("USB device class", "Communications");
    case LIBUSB_CLASS_HID:
        return i18nc("USB device class", "Human Interface Device");
    case LIBUSB_CLASS_PHYSICAL:
        return i18nc("USB device class", "Physical Interface Device");
    case LIBUSB_CLASS_PRINTER:
        return i18nc("USB device class", "Printer");
    case LIBUSB_CLASS_IMAGE:
        return i18nc("USB device class", "Imaging");
    case LIBUSB_CLASS_MASS_STORAGE:
        return i18nc("USB device class", "Mass Storage");
    case LIBUSB_CLASS_HUB:
        return i18nc("USB device class", "Hub");
    case LIBUSB_CLASS_DATA:
        return i18nc("USB device class", "CDC Data");
    case LIBUSB_CLASS_SMART_CARD:
        return i18nc("USB device class", "Chip/SmartCard");
    case LIBUSB_CLASS_CONTENT_SECURITY:
        return i18nc("USB device class", "Content Security");
    case LIBUSB_CLASS_VIDEO:
        return i18nc("USB device class", "Video");
    case LIBUSB_CLASS_PERSONAL_HEALTHCARE:
        return i18nc("USB device class", "Personal Healthcare");
    case LIBUSB_CLASS_DIAGNOSTIC_DEVICE:
        return i18nc("USB device class", "Diagnostic");
    case LIBUSB_CLASS_WIRELESS:
        return i18nc("USB device class", "Wireless");
#if LIBUSB_API_VERSION >= 0x01000108
    case LIBUSB_CLASS_MISCELLANEOUS:
        return i18nc("USB device class", "Miscellaneous Device");
#endif
    case LIBUSB_CLASS_APPLICATION:
        return i18nc("USB device class", "Application Specific Interface");
    case LIBUSB_CLASS_VENDOR_SPEC:
        return i18nc("USB device class", "Vendor Specific Class");
    }
    return QString();
}

USBDevice::USBDevice(libusb_device *dev, struct libusb_device_descriptor &dev_desc)
    : _bus(libusb_get_bus_number(dev))
    , _level(0)
    , _parent(0)
    , _port(libusb_get_port_number(dev))
    , _device(libusb_get_device_address(dev))
    , _channels(0)
    , _speed(convertLibusbSpeed(libusb_get_device_speed(dev)))
    , _verMajor(0)
    , _verMinor(0)
    , _class(dev_desc.bDeviceClass)
    , _sub(dev_desc.bDeviceSubClass)
    , _prot(dev_desc.bDeviceProtocol)
    , _maxPacketSize(dev_desc.bMaxPacketSize0)
    , _vendorID(dev_desc.idVendor)
    , _prodID(dev_desc.idProduct)
{
    _devices.append(this);

    if (!_db)
        _db = new USBDB;

    convertLibusbUsbVersion(dev_desc.bcdUSB, &_verMajor, &_verMinor);

    collectDataSys(dev);
}

USBDevice::~USBDevice()
{
}

#if defined(Q_OS_LINUX)

static QString catFile(const QString &fname)
{
    char buffer[256];
    QString result;
    int fd = ::open(QFile::encodeName(fname).constData(), O_RDONLY);
    if (fd < 0)
        return QString();

    if (fd >= 0) {
        ssize_t count;
        while ((count = ::read(fd, buffer, 256)) > 0)
            result.append(QString::fromLatin1(buffer, count));

        ::close(fd);
    }
    return result.trimmed();
}

static QString devpath(libusb_device *dev)
{
    // hardcoded to 7 as the libusb apidocs says:
    // "As per the USB 3.0 specs, the current maximum limit for the depth is 7."
    static const int ports = 7;
    uint8_t port_numbers[ports];
    const int num = libusb_get_port_numbers(dev, port_numbers, ports);
    QString ret;
    for (uint8_t i = 0; i < num; ++i) {
        if (i > 0)
            ret += QLatin1Char('.');
        ret += QString::number(port_numbers[i]);
    }
    return ret;
}

void USBDevice::collectDataSys(libusb_device *dev)
{
    const QString dname =
        _port == 0 ? QStringLiteral("/sys/bus/usb/devices/usb%1").arg(_bus) : QStringLiteral("/sys/bus/usb/devices/%1-%2").arg(_bus).arg(devpath(dev));

    _manufacturer = catFile(dname + QStringLiteral("/manufacturer"));
    _product = catFile(dname + QStringLiteral("/product"));
    if (_device == 1)
        _product += QStringLiteral(" (%1)").arg(_bus);
    _serial = catFile(dname + QStringLiteral("/serial"));
    _channels = catFile(dname + QStringLiteral("/maxchild")).toUInt();
}

#else

void USBDevice::collectDataSys(libusb_device *dev)
{
    Q_UNUSED(dev);
}

#endif

USBDevice *USBDevice::find(int bus, int device)
{
    for (USBDevice *usbDevice : std::as_const(_devices)) {
        if (usbDevice->bus() == bus && usbDevice->device() == device)
            return usbDevice;
    }

    return nullptr;
}

QString USBDevice::product()
{
    if (!_product.isEmpty())
        return _product;
    QString pname = _db->device(_vendorID, _prodID);
    if (!pname.isEmpty())
        return pname;
    return i18n("Unknown");
}

QString USBDevice::dump()
{
    QString r;

    r = QStringLiteral("<qml><h2><center>") + product() + QStringLiteral("</center></h2><br/><hl/>");

    if (!_manufacturer.isEmpty())
        r += i18n("<b>Manufacturer:</b> ") + _manufacturer + QStringLiteral("<br/>");
    if (!_serial.isEmpty())
        r += i18n("<b>Serial #:</b> ") + _serial + QStringLiteral("<br/>");

    r += QLatin1String("<br/><table>");

    QString c = QStringLiteral("<td>%1</td>").arg(_class);
    QString cname = prettyLibusbClassName(_class);
    if (cname.isEmpty()) {
        cname = _db->cls(_class);
        if (!cname.isEmpty())
            cname = i18nc("USB device class", cname.toUtf8().constData());
    }
    if (!cname.isEmpty())
        c += QStringLiteral("<td>(") + cname + QStringLiteral(")</td>");
    r += i18n("<tr><td><i>Class</i></td>%1</tr>", c);
    QString sc = QStringLiteral("<td>%1</td>").arg(_sub);
    QString scname = _db->subclass(_class, _sub);
    if (!scname.isEmpty())
        sc += QStringLiteral("<td>(") + i18nc("USB device subclass", scname.toLatin1().constData()) + QStringLiteral(")</td>");
    r += i18n("<tr><td><i>Subclass</i></td>%1</tr>", sc);
    QString pr = QStringLiteral("<td>%1</td>").arg(_prot);
    QString prname = _db->protocol(_class, _sub, _prot);
    if (!prname.isEmpty())
        pr += QStringLiteral("<td>(") + prname + QStringLiteral(")</td>");
    r += i18n("<tr><td><i>Protocol</i></td>%1</tr>", pr);
    r += ki18n("<tr><td><i>USB Version</i></td><td>%1.%2</td></tr>").subs(_verMajor).subs(_verMinor, 2, 10, QChar::fromLatin1('0')).toString();
    r += QLatin1String("<tr><td></td></tr>");

    QString v = QStringLiteral("%1").arg(_vendorID, 4, 16, QLatin1Char('0'));
    QString name = _db->vendor(_vendorID);
    if (!name.isEmpty())
        v += QStringLiteral("<td>(") + name + QStringLiteral(")</td>");
    r += i18n("<tr><td><i>Vendor ID</i></td><td>0x%1</td></tr>", v);
    QString p = QStringLiteral("%1").arg(_prodID, 4, 16, QLatin1Char('0'));
    QString pname = _db->device(_vendorID, _prodID);
    if (!pname.isEmpty())
        p += QStringLiteral("<td>(") + pname + QStringLiteral(")</td>");
    r += i18n("<tr><td><i>Product ID</i></td><td>0x%1</td></tr>", p);
    r += QLatin1String("<tr><td></td></tr>");

    r += i18n("<tr><td><i>Speed</i></td><td>%1 Mbit/s</td></tr>", _speed);
    r += i18n("<tr><td><i>Channels</i></td><td>%1</td></tr>", _channels);
    r += i18n("<tr><td><i>Max. Packet Size</i></td><td>%1</td></tr>", _maxPacketSize);
    r += QLatin1String("<tr><td></td></tr>");

    r += QLatin1String("</table>");

    return r;
}

bool USBDevice::load()
{
    if (!_context) {
        const int r = libusb_init(&_context);
        if (r < 0) {
            qWarning() << "Failed to initialize libusb:" << r << libusb_error_name(r);
            return false;
        }
    }

    qDeleteAll(_devices);
    _devices.clear();

    libusb_device **devs;
    const ssize_t count = libusb_get_device_list(_context, &devs);
    if (count < 0) {
        qWarning() << "Cannot get the list of USB devices";
        return false;
    }

    QHash<libusb_device *, USBDevice *> devBylibusbMap;
    QHash<USBDevice *, libusb_device *> libusbByDevMap;

    for (ssize_t i = 0; i < count; ++i) {
        libusb_device *dev = devs[i];
        struct libusb_device_descriptor dev_desc;
        int r = libusb_get_device_descriptor(dev, &dev_desc);
        if (r < 0) {
            qWarning() << "libusb_get_device_descriptor failed:" << r << libusb_error_name(r);
            continue;
        }
        USBDevice *device = new USBDevice(dev, dev_desc);
        devBylibusbMap.insert(dev, device);
        libusbByDevMap.insert(device, dev);
    }

    auto levels = [](libusb_device *dev) {
        int level = 0;
        for (libusb_device *p = libusb_get_parent(dev); p; p = libusb_get_parent(p)) {
            ++level;
        }
        return level;
    };
    for (int i = 0; i < _devices.count(); ++i) {
        USBDevice *device = _devices[i];
        libusb_device *dev = libusbByDevMap.value(device);
        device->_level = levels(dev);
        libusb_device *parentDev = libusb_get_parent(dev);
        if (parentDev)
            device->_parent = devBylibusbMap.value(parentDev)->_device;
    }

    libusb_free_device_list(devs, 1);

    return true;
}

void USBDevice::clear()
{
    qDeleteAll(_devices);
    _devices.clear();

    if (_context) {
        libusb_exit(_context);
        _context = nullptr;
    }
}
