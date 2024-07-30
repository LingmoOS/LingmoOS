/*
    SPDX-FileCopyrightText: 2014-2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KOSRELEASE_H
#define KOSRELEASE_H

#include <kcoreaddons_export.h>

#include <QString>
#include <QStringList>

#include <memory>

/**
 * @brief The OSRelease class parses /etc/os-release files
 *
 * https://www.freedesktop.org/software/systemd/man/os-release.html
 *
 * os-release is a free desktop standard for describing an operating system.
 * This class parses and models os-release files.
 *
 * @since 5.58.0
 */
class KCOREADDONS_EXPORT KOSRelease final
{
public:
    /**
     * Constructs a new OSRelease instance. Parsing happens in the constructor
     * and the data is not cached across instances.
     *
     * @note The format specification makes no assertions about trailing #
     *   comments being supported. They result in undefined behavior.
     *
     * @param filePath The path to the os-release file. By default the first
     *   available file of the paths specified in the os-release manpage is
     *   parsed.
     */
    explicit KOSRelease(const QString &filePath = QString());
    ~KOSRelease();

    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#NAME= */
    QString name() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#VERSION= */
    QString version() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#ID= */
    QString id() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#ID_LIKE= */
    QStringList idLike() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#VERSION_CODENAME= */
    QString versionCodename() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#VERSION_ID= */
    QString versionId() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#PRETTY_NAME= */
    QString prettyName() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#ANSI_COLOR= */
    QString ansiColor() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#CPE_NAME= */
    QString cpeName() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#HOME_URL= */
    QString homeUrl() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#HOME_URL= */
    QString documentationUrl() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#HOME_URL= */
    QString supportUrl() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#HOME_URL= */
    QString bugReportUrl() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#HOME_URL= */
    QString privacyPolicyUrl() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#BUILD_ID= */
    QString buildId() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#VARIANT= */
    QString variant() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#VARIANT_ID= */
    QString variantId() const;
    /** @see https://www.freedesktop.org/software/systemd/man/os-release.html#LOGO= */
    QString logo() const;

    /**
     * Extra keys are keys that are unknown or specified by a vendor.
     */
    QStringList extraKeys() const;

    /** Extra values are values assoicated with keys that are unknown. */
    QString extraValue(const QString &key) const;

private:
    Q_DISABLE_COPY(KOSRelease)

    std::unique_ptr<class KOSReleasePrivate> const d;
};

#endif // KOSRELEASE_H
