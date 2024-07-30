/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_MODEMFIRMWARE_H
#define MODEMMANAGERQT_MODEMFIRMWARE_H

#include "interface.h"
#include <modemmanagerqt_export.h>

#include <QDBusPendingReply>
#include <QObject>
#include <QSharedPointer>

namespace ModemManager
{
class ModemFirmwarePrivate;

/**
 * @brief The ModemFirmware class
 *
 * This class allows clients to select or install firmware images on modems.
 *
 * Firmware slots and firmware images are identified by arbitrary opaque strings.
 *
 * @since 1.1.94
 */
class MODEMMANAGERQT_EXPORT ModemFirmware : public Interface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ModemFirmware)

public:
    typedef QSharedPointer<ModemFirmware> Ptr;
    typedef QList<Ptr> List;

    explicit ModemFirmware(const QString &path, QObject *parent = nullptr);
    ~ModemFirmware() override;

    /**
     * List installed firmware images.
     *
     * Depending on the type of modem, installed images may be stored on the host or the modem.
     * Installed images can be selected non-destructively.
     *
     * @return @param selected (QString)
     *         The unique name of the selected firmware image, or the empty string if no image is selected.
     *         @param installed (QVariantMapList)
     *         A map of dictionaries containing the properties of the installed firmware images.
     */
    QDBusPendingReply<QString, QVariantMapList> listImages();

    /**
     * Selects a different firmware image to use, and immediately resets the modem so that it
     * begins using the new firmware image.
     *
     * The method will fail if the identifier does not match any of the names returned by listImages(),
     * or if the image could not be selected for some reason.
     *
     * @param uniqueid The unique ID of the firmware image to select.
     */
    void selectImage(const QString &uniqueid);
};

} // namespace ModemManager

#endif
