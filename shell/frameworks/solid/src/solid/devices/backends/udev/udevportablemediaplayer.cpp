/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2011 Matej Laitl <matej@laitl.cz>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevportablemediaplayer.h"

#include <QChar>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <qstandardpaths.h>

using namespace Solid::Backends::UDev;

/**
 * Reads one value from media-player-info ini-like file.
 *
 * @param file file to open. May advance current seek position
 * @param group group name to read from, e.g. "Device" for [Device] group
 * @param key key name, e.g. "AccessProtocol"
 * @return value as a string or an empty string
 */
static QString readMpiValue(QIODevice &file, const QString &group, const QString &key)
{
    QTextStream mpiStream(&file);
    QString line;
    QString currGroup;

    while (!mpiStream.atEnd()) {
        line = mpiStream.readLine().trimmed(); // trimmed is needed for possible indentation
        if (line.isEmpty() || line.startsWith(QChar(';'))) {
            // skip empty and comment lines
        } else if (line.startsWith(QChar('[')) && line.endsWith(QChar(']'))) {
            currGroup = line.mid(1, line.length() - 2); // strip [ and ]
        } else if (line.indexOf(QChar('=')) != -1) {
            int index = line.indexOf(QChar('='));
            if (currGroup == group && line.left(index) == key) {
                line = line.right(line.length() - index - 1);
                if (line.startsWith(QChar('"')) && line.endsWith(QChar('"'))) {
                    line = line.mid(1, line.length() - 2); // strip enclosing double quotes
                }
                return line;
            }
        } else {
            qWarning() << "readMpiValue: cannot parse line:" << line;
        }
    }
    return QString();
}

PortableMediaPlayer::PortableMediaPlayer(UDevDevice *device)
    : DeviceInterface(device)
{
}

PortableMediaPlayer::~PortableMediaPlayer()
{
}

QStringList PortableMediaPlayer::supportedProtocols() const
{
    /* There are multiple packages that set ID_MEDIA_PLAYER:
     *  * gphoto2 sets it to numeric 1 (for _some_ cameras it supports) and it hopefully
     *    means MTP-compatible device.
     *  * libmtp >= 1.0.4 sets it to numeric 1 and this always denotes MTP-compatible player.
     *  * media-player-info sets it to a string that denotes a name of the .mpi file with
     *    additional info.
     */
    if (m_device->property("ID_MEDIA_PLAYER").toInt() == 1) {
        return QStringList() << "mtp";
    }

    QString mpiFileName = mediaPlayerInfoFilePath();
    if (mpiFileName.isEmpty()) {
        return QStringList();
    }
    // we unfornutately cannot use QSettings as it cannot read unquoted valued with semicolons in it
    QFile mpiFile(mpiFileName);
    if (!mpiFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open" << mpiFileName << "for reading."
                   << "Check your media-player-info installation.";
        return QStringList();
    }
    QString value = readMpiValue(mpiFile, QString("Device"), QString("AccessProtocol"));
    return value.split(QChar(';'), Qt::SkipEmptyParts);
}

QStringList PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_UNUSED(protocol)
    QStringList res;

    if (!supportedProtocols().isEmpty()) {
        res << "usb";
    }
    if (m_device->property("USBMUX_SUPPORTED").toBool() == true) {
        res << "usbmux";
    }
    return res;
}

QVariant PortableMediaPlayer::driverHandle(const QString &driver) const
{
    if (driver == "mtp" || driver == "usbmux") {
        return m_device->property("ID_SERIAL_SHORT");
    }

    return QVariant();
}

QString PortableMediaPlayer::mediaPlayerInfoFilePath() const
{
    QString relativeFilename = m_device->property("ID_MEDIA_PLAYER").toString();
    if (relativeFilename.isEmpty()) {
        qWarning() << "We attached PortableMediaPlayer interface to device" << m_device->udi() << "but m_device->property(\"ID_MEDIA_PLAYER\") is empty???";
        return QString();
    }
    relativeFilename.prepend("media-player-info/");
    relativeFilename.append(".mpi");
    QString filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relativeFilename);
    if (filename.isEmpty()) {
        qWarning() << "media player info file" << relativeFilename << "not found under user and"
                   << "system XDG data directories. Do you have media-player-info installed?";
    }
    return filename;
}

#include "moc_udevportablemediaplayer.cpp"
