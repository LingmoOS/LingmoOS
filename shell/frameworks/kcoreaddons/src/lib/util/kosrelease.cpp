/*
    SPDX-FileCopyrightText: 2014-2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kosrelease.h"

#include <QFile>

#include "kcoreaddons_debug.h"
#include "kshell.h"

// Sets a QString var
static void setVar(QString *var, const QString &value)
{
    // Values may contain quotation marks, strip them as we have no use for them.
    KShell::Errors error;
    QStringList args = KShell::splitArgs(value, KShell::NoOptions, &error);
    if (error != KShell::NoError) { // Failed to parse.
        return;
    }
    *var = args.join(QLatin1Char(' '));
}

// Sets a QStringList var (i.e. splits a string value)
static void setVar(QStringList *var, const QString &value)
{
    // Instead of passing the verbatim value we manually strip any initial quotes
    // and then run it through KShell. At this point KShell will actually split
    // by spaces giving us the final QStringList.
    // NOTE: Splitting like this does not actually allow escaped substrings to
    //       be handled correctly, so "kitteh \"french fries\"" would result in
    //       three list entries. I'd argue that if someone makes an id like that
    //       they are at fault for the bogus parsing here though as id explicitly
    //       is required to not contain spaces even if more advanced shell escaping
    //       is also allowed...
    QString value_ = value;
    if (value_.at(0) == QLatin1Char('"') && value_.at(value_.size() - 1) == QLatin1Char('"')) {
        value_.remove(0, 1);
        value_.remove(-1, 1);
    }
    KShell::Errors error;
    QStringList args = KShell::splitArgs(value_, KShell::NoOptions, &error);
    if (error != KShell::NoError) { // Failed to parse.
        return;
    }
    *var = args;
}

static QStringList splitEntry(const QString &line)
{
    QStringList list;
    const int separatorIndex = line.indexOf(QLatin1Char('='));
    list << line.mid(0, separatorIndex);
    if (separatorIndex != -1) {
        list << line.mid(separatorIndex + 1, -1);
    }
    return list;
}

static QString defaultFilePath()
{
    if (QFile::exists(QStringLiteral("/etc/os-release"))) {
        return QStringLiteral("/etc/os-release");
    } else if (QFile::exists(QStringLiteral("/usr/lib/os-release"))) {
        return QStringLiteral("/usr/lib/os-release");
    } else {
        return QString();
    }
}

class KOSReleasePrivate
{
public:
    explicit KOSReleasePrivate(QString filePath)
        : name(QStringLiteral("Linux"))
        , id(QStringLiteral("linux"))
        , prettyName(QStringLiteral("Linux"))
    {
        // Default values for non-optional fields set above ^.

        QHash<QString, QString *> stringHash = {{QStringLiteral("NAME"), &name},
                                                {QStringLiteral("VERSION"), &version},
                                                {QStringLiteral("ID"), &id},
                                                // idLike is not a QString, special handling below!
                                                {QStringLiteral("VERSION_CODENAME"), &versionCodename},
                                                {QStringLiteral("VERSION_ID"), &versionId},
                                                {QStringLiteral("PRETTY_NAME"), &prettyName},
                                                {QStringLiteral("ANSI_COLOR"), &ansiColor},
                                                {QStringLiteral("CPE_NAME"), &cpeName},
                                                {QStringLiteral("HOME_URL"), &homeUrl},
                                                {QStringLiteral("DOCUMENTATION_URL"), &documentationUrl},
                                                {QStringLiteral("SUPPORT_URL"), &supportUrl},
                                                {QStringLiteral("BUG_REPORT_URL"), &bugReportUrl},
                                                {QStringLiteral("PRIVACY_POLICY_URL"), &privacyPolicyUrl},
                                                {QStringLiteral("BUILD_ID"), &buildId},
                                                {QStringLiteral("VARIANT"), &variant},
                                                {QStringLiteral("VARIANT_ID"), &variantId},
                                                {QStringLiteral("LOGO"), &logo}};

        if (filePath.isEmpty()) {
            filePath = defaultFilePath();
        }
        if (filePath.isEmpty()) {
            qCWarning(KCOREADDONS_DEBUG) << "Failed to find os-release file!";
            return;
        }

        QFile file(filePath);
        // NOTE: The os-release specification defines default values for specific
        //       fields which means that even if we can not read the os-release file
        //       we have sort of expected default values to use.
        // TODO: it might still be handy to indicate to the outside whether
        //       fallback values are being used or not.
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString line;
        QStringList parts;
        while (!file.atEnd()) {
            // Trimmed to handle indented comment lines properly
            line = QString::fromUtf8(file.readLine()).trimmed();

            if (line.startsWith(QLatin1Char('#'))) {
                // Comment line
                // Lines beginning with "#" shall be ignored as comments.
                continue;
            }

            parts = splitEntry(line);

            if (parts.size() != 2) {
                // Line has no =, must be invalid.
                qCDebug(KCOREADDONS_DEBUG) << "Unexpected/invalid os-release line:" << line;
                continue;
            }

            QString key = parts.at(0);
            QString value = parts.at(1).trimmed();

            if (QString *var = stringHash.value(key, nullptr)) {
                setVar(var, value);
                continue;
            }

            // ID_LIKE is a list and parsed as such (rather than a QString).
            if (key == QLatin1String("ID_LIKE")) {
                setVar(&idLike, value);
                continue;
            }

            // os-release explicitly allows for vendor specific additions, we'll
            // collect them as strings and exposes them as "extras".
            QString parsedValue;
            setVar(&parsedValue, value);
            extras.insert(key, parsedValue);
        }
    }

    QString name;
    QString version;
    QString id;
    QStringList idLike;
    QString versionCodename;
    QString versionId;
    QString prettyName;
    QString ansiColor;
    QString cpeName;
    QString homeUrl;
    QString documentationUrl;
    QString supportUrl;
    QString bugReportUrl;
    QString privacyPolicyUrl;
    QString buildId;
    QString variant;
    QString variantId;
    QString logo;

    QHash<QString, QString> extras;
};

KOSRelease::KOSRelease(const QString &filePath)
    : d(new KOSReleasePrivate(filePath))
{
}

KOSRelease::~KOSRelease() = default;

QString KOSRelease::name() const
{
    return d->name;
}

QString KOSRelease::version() const
{
    return d->version;
}

QString KOSRelease::id() const
{
    return d->id;
}

QStringList KOSRelease::idLike() const
{
    return d->idLike;
}

QString KOSRelease::versionCodename() const
{
    return d->versionCodename;
}

QString KOSRelease::versionId() const
{
    return d->versionId;
}

QString KOSRelease::prettyName() const
{
    return d->prettyName;
}

QString KOSRelease::ansiColor() const
{
    return d->ansiColor;
}

QString KOSRelease::cpeName() const
{
    return d->cpeName;
}

QString KOSRelease::homeUrl() const
{
    return d->homeUrl;
}

QString KOSRelease::documentationUrl() const
{
    return d->documentationUrl;
}

QString KOSRelease::supportUrl() const
{
    return d->supportUrl;
}

QString KOSRelease::bugReportUrl() const
{
    return d->bugReportUrl;
}

QString KOSRelease::privacyPolicyUrl() const
{
    return d->privacyPolicyUrl;
}

QString KOSRelease::buildId() const
{
    return d->buildId;
}

QString KOSRelease::variant() const
{
    return d->variant;
}

QString KOSRelease::variantId() const
{
    return d->variantId;
}

QString KOSRelease::logo() const
{
    return d->logo;
}

QStringList KOSRelease::extraKeys() const
{
    return d->extras.keys();
}

QString KOSRelease::extraValue(const QString &key) const
{
    return d->extras.value(key);
}
