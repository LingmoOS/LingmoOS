/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_INSTALLATION_P_H
#define KNEWSTUFF3_INSTALLATION_P_H

#include <QObject>
#include <QString>

#include <KConfigGroup>

#include "entry.h"

class QProcess;
class KArchiveDirectory;
class KJob;

namespace KNSCore
{
/**
 * @short KNewStuff entry installation.
 *
 * The installation class stores all information related to an entry's
 * installation.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFFCORE_EXPORT Installation : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit Installation(QObject *parent = nullptr);
    enum UncompressionOptions {
        NeverUncompress, ///@< Never attempt to decompress a file, whatever format it is. Matches "never" knsrc setting
        AlwaysUncompress, ///@< Assume all downloaded files are archives, and attempt to decompress them. Will cause failure if decompression fails. Matches
                          ///"always" knsrc setting
        UncompressIfArchive, ///@< If the file is an archive, decompress it, otherwise just pass it on. Matches "archive" knsrc setting
        UncompressIntoSubdirIfArchive, ///@< If the file is an archive, decompress it in a subdirectory if it contains multiple files, otherwise just pass it
                                       /// on. Matches "subdir-archive" knsrc setting
        UncompressIntoSubdir, ///@< As Archive, except that if there is more than an item in the file, put contents in a subdirectory with the same name as the
                              /// file. Matches "subdir" knsrc setting
        UseKPackageUncompression, ///@< Use the internal KPackage support for installing and uninstalling the package. Matches "kpackage" knsrc setting
    };
    Q_ENUM(UncompressionOptions)

    bool readConfig(const KConfigGroup &group, QString &errorMessage);

    QString targetInstallationPath() const;

    /**
     * Returns the uncompression setting, in a computer-readable format
     *
     * @return The value of this setting
     */
    UncompressionOptions uncompressionSetting() const;

public Q_SLOTS:
    /**
     * Downloads a payload file. The payload file matching most closely
     * the current user language preferences will be downloaded.
     * The file will not be installed set, for this \ref install must
     * be called.
     *
     * @param entry Entry to download payload file for
     *
     * @see signalPayloadLoaded
     * @see signalPayloadFailed
     */
    void downloadPayload(const KNSCore::Entry &entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     * Note that this method is asynchronous and thus the return value will
     * only report the successful start of the installation.
     * Note also that while entry is const at this point, it will change later
     * during the actual installation (the installedFiles list will change, as
     * will its status)
     *
     * @param entry Entry to be installed
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(const KNSCore::Entry &entry);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * The entry emitted by signalEntryChanged will be updated with any new information, in particular the following:
     * <ul>
     * <li>Status will be set to Deleted, unless the uninstall
     * script exists with an error and the user chooses to cancel the uninstallation
     * <li>uninstalledFiles will list files which were removed during uninstallation
     * <li>installedFiles will become empty
     * </ul>
     *
     * @param entry The entry to deinstall
     *
     */
    void uninstall(KNSCore::Entry entry);

    void slotPayloadResult(KJob *job);

Q_SIGNALS:
    void signalEntryChanged(const KNSCore::Entry &entry);
    void signalInstallationFinished(const KNSCore::Entry &entry);
    void signalInstallationFailed(const QString &message, const KNSCore::Entry &entry);
    /**
     * An informational signal fired when a serious error occurs during the installation.
     * @param message The description of the error (a message intended to be human readable)
     * @since 5.69
     */
    void signalInstallationError(const QString &message, const KNSCore::Entry &entry);

    void signalPayloadLoaded(QUrl payload); // FIXME: return Entry

private:
    void install(KNSCore::Entry entry, const QString &downloadedFile);

    QStringList installDownloadedFileAndUncompress(const KNSCore::Entry &entry, const QString &payloadfile, const QString installdir);
    QProcess *runPostInstallationCommand(const QString &installPath, const KNSCore::Entry &entry);

    static QStringList archiveEntries(const QString &path, const KArchiveDirectory *dir);

    // applications can set this if they want the installed files/directories to be piped into a shell command
    QString postInstallationCommand;
    // a custom command to run for the uninstall
    QString uninstallCommand;
    // compression policy

    // only one of the five below can be set, that will be the target install path/file name
    // FIXME: check this when reading the config and make one path out of it if possible?
    QString standardResourceDirectory;
    QString targetDirectory;
    QString xdgTargetDirectory;
    QString installPath;
    QString absoluteInstallPath;

    QMap<KJob *, Entry> entry_jobs;

    QString kpackageStructure;
    UncompressionOptions uncompressSetting = UncompressionOptions::NeverUncompress;

    Q_DISABLE_COPY(Installation)
};

}

#endif
