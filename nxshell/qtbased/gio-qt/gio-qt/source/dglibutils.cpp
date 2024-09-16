// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dglibutils.h"

#include <QStringList>

#include <glibmm/miscutils.h>

/*!
 * \brief Get the full path by directory type.
 *
 * Wrapper of Glib::get_user_data_dir(), behavior similar to QStandardPaths::writableLocation(),
 * but at least it has USER_DIRECTORY_TEMPLATES .
 *
 * On UNIX platforms this is determined using the mechanisms described in the
 * [XDG Base Directory Specification](http://www.freedesktop.org/Standards/basedir-spec).
 *
 * \param userDirectory The logical id of special directory.
 * \return The path to the specified special directory, or an empty string if the logical id was not found.
 */
QString DGlibUtils::userSpecialDir(DGlibUserDirectory userDirectory)
{
    return QString::fromStdString(Glib::get_user_special_dir(static_cast<Glib::UserDirectory>(userDirectory)));
}

/*!
 * \brief Get an ordered list of base directories in which to access system-wide application data.
 *
 * Wrapper of Glib::get_system_data_dirs(), behavior similar (should be the same under UNIX) to
 * QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).
 *
 * On UNIX platforms this is determined using the mechanisms described in the
 * [XDG Base Directory Specification](http://www.freedesktop.org/Standards/basedir-spec)
 * In this case the list of directories retrieved will be XDG_DATA_DIRS.
 */
QStringList DGlibUtils::systemDataDirs()
{
    std::vector<std::string> dirs = Glib::get_system_data_dirs();
    QStringList lst;

    for (auto dir : dirs) {
        lst.append(QString::fromStdString(dir));
    }

    return lst;
}

/*!
 * \brief Get a base directory in which to access application data such as icons that is customized for a particular user.
 *
 * Wrapper of Glib::get_user_data_dir(), behavior similar (should be the same under UNIX) to
 * QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).
 *
 * On UNIX platforms this is determined using the mechanisms described in the
 * [XDG Base Directory Specification](http://www.freedesktop.org/Standards/basedir-spec).
 * In this case the directory retrieved will be `XDG_DATA_HOME`.
 */
QString DGlibUtils::userDataDir()
{
    return QString::fromStdString(Glib::get_user_data_dir());
}

/*!
 * \brief Gets the directory to use for temporary files.
 *
 * Wrapper of Glib::get_user_data_dir(), behavior similar (but not the same) to QDir::temp() or
 * QStandardPaths::writableLocation(QStandardPaths::TempLocation).
 *
 * This is found from inspecting the environment variables TMPDIR, TMP, and TEMP in that order. If none of those are defined
 * "/tmp" is returned on UNIX and "C:\\" on Windows.
 *
 * \return The directory to use for temporary files.
 */
QString DGlibUtils::tmpDir()
{
    return QString::fromStdString(Glib::get_tmp_dir());
}

/*!
 * \brief Formats a size (for example the size of a file) into a human readable string.
 *
 * Sizes are rounded to the nearest size prefix (kB, MB, GB) and are displayed rounded to the nearest tenth. E.g. the file
 * size 3292528 bytes will be converted into the string "3.2 MB".
 *
 * The prefix units base is 1000 (i.e. 1 kB is 1000 bytes), unless the DGlibFormatSizeFlags::FORMAT_SIZE_IEC_UNITS flag is set.
 *
 * \param size A size in bytes.
 * \param flags Flags to modify the output.
 * \return A formatted string containing a human readable file size.
 */
QString DGlibUtils::formatSize(quint64 size, DGlibFormatSizeFlags flags)
{
    unsigned int flagValue = flags;
    return QString::fromStdString(Glib::format_size(size, static_cast<Glib::FormatSizeFlags>(flagValue)));
}
