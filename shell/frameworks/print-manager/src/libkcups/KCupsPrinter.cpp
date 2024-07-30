/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsPrinter.h"

KCupsPrinter::KCupsPrinter()
    : m_isClass(false)
{
}

KCupsPrinter::KCupsPrinter(const QString &printer, bool isClass)
    : m_printer(printer)
    , m_isClass(isClass)
{
}

KCupsPrinter::KCupsPrinter(const QVariantMap &arguments)
    : m_printer(arguments[QLatin1String(KCUPS_PRINTER_NAME)].toString())
    , m_isClass(arguments[QLatin1String(KCUPS_PRINTER_TYPE)].toInt() & CUPS_PRINTER_CLASS)
    , m_arguments(arguments)
{
}

QString KCupsPrinter::name() const
{
    return m_printer;
}

bool KCupsPrinter::isClass() const
{
    return m_isClass;
}

bool KCupsPrinter::isDefault() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_TYPE)].toUInt() & CUPS_PRINTER_DEFAULT;
}

bool KCupsPrinter::isShared() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_IS_SHARED)].toBool();
}

bool KCupsPrinter::isAcceptingJobs() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_IS_ACCEPTING_JOBS)].toBool();
}

cups_ptype_e KCupsPrinter::type() const
{
    return static_cast<cups_ptype_e>(m_arguments[QLatin1String(KCUPS_PRINTER_TYPE)].toUInt());
}

QString KCupsPrinter::location() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_LOCATION)].toString();
}

QString KCupsPrinter::info() const
{
    const QString printerInfo = m_arguments[QLatin1String(KCUPS_PRINTER_INFO)].toString();
    if (printerInfo.isEmpty()) {
        return name();
    }
    return printerInfo;
}

QString KCupsPrinter::makeAndModel() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_MAKE_AND_MODEL)].toString();
}

QStringList KCupsPrinter::commands() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_COMMANDS)].toStringList();
}

QStringList KCupsPrinter::memberNames() const
{
    return m_arguments[QLatin1String(KCUPS_MEMBER_NAMES)].toStringList();
}

QString KCupsPrinter::deviceUri() const
{
    return m_arguments[QLatin1String(KCUPS_DEVICE_URI)].toString();
}

QStringList KCupsPrinter::errorPolicy() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_ERROR_POLICY)].toStringList();
}

QStringList KCupsPrinter::errorPolicySupported() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_ERROR_POLICY_SUPPORTED)].toStringList();
}

QStringList KCupsPrinter::opPolicy() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_OP_POLICY)].toStringList();
}

QStringList KCupsPrinter::opPolicySupported() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_OP_POLICY_SUPPORTED)].toStringList();
}

QStringList KCupsPrinter::jobSheetsDefault() const
{
    return m_arguments[QLatin1String(KCUPS_JOB_SHEETS_DEFAULT)].toStringList();
}

QStringList KCupsPrinter::jobSheetsSupported() const
{
    return m_arguments[QLatin1String(KCUPS_JOB_SHEETS_SUPPORTED)].toStringList();
}

QStringList KCupsPrinter::requestingUserNameAllowed() const
{
    return m_arguments[QLatin1String(KCUPS_REQUESTING_USER_NAME_ALLOWED)].toStringList();
}

QStringList KCupsPrinter::requestingUserNameDenied() const
{
    return m_arguments[QLatin1String(KCUPS_REQUESTING_USER_NAME_DENIED)].toStringList();
}

QStringList KCupsPrinter::authInfoRequired() const
{
    return m_arguments[QLatin1String(KCUPS_AUTH_INFO_REQUIRED)].toStringList();
}

QString KCupsPrinter::uriSupported() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_URI_SUPPORTED)].toString();
}

KCupsPrinter::Status KCupsPrinter::state() const
{
    return static_cast<Status>(m_arguments[QLatin1String(KCUPS_PRINTER_STATE)].toUInt());
}

QString KCupsPrinter::stateMsg() const
{
    return m_arguments[QLatin1String(KCUPS_PRINTER_STATE_MESSAGE)].toString();
}

int KCupsPrinter::markerChangeTime() const
{
    return m_arguments[QLatin1String(KCUPS_MARKER_CHANGE_TIME)].toInt();
}

QVariant KCupsPrinter::argument(const QString &name) const
{
    return m_arguments.value(name);
}

QIcon KCupsPrinter::icon() const
{
    return icon(type());
}

QIcon KCupsPrinter::icon(cups_ptype_e type)
{
    return QIcon::fromTheme(iconName(type));
}

QString KCupsPrinter::iconName() const
{
    return iconName(type());
}

QString KCupsPrinter::iconName(cups_ptype_e type)
{
    // TODO get the ppd or something to get the real printer icon
    if (!(type & CUPS_PRINTER_COLOR)) {
        // If the printer is not color it is probably a laser one
        return QStringLiteral("printer-laser");
    } else if (type & CUPS_PRINTER_SCANNER) {
        return QStringLiteral("scanner");
    } else {
        return QStringLiteral("printer");
    }
}
