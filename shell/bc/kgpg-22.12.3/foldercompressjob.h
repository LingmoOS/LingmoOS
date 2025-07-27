/*
    SPDX-FileCopyrightText: 2011, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FOLDERCOMPRESSJOB_H
#define FOLDERCOMPRESSJOB_H

#include <KJob>
#include <QUrl>
#include <QStringList>

class QTemporaryFile;
class QString;

#include "transactions/kgpgencrypt.h"

class FolderCompressJobPrivate;

/**
 * @brief Create an encrypted archive of the given folders
 *
 * @author Rolf Eike Beer
 */
class FolderCompressJob : public KJob {
	Q_OBJECT

	Q_DISABLE_COPY(FolderCompressJob)
	FolderCompressJob() = delete;

	FolderCompressJobPrivate * const d_ptr;
	Q_DECLARE_PRIVATE(FolderCompressJob)

public:
	/**
	 * @brief create a new KJob to compress and encrypt a folder
	 * @param parent object owning this job
	 * @param sources the source directories to include
	 * @param dest the name of the encrypted file
	 * @param tempfile the temporary file that should be used for archiving
	 * @param keys the public key ids to encrypt to
	 * @param options special options to pass to the GnuPG process
	 * @param encOptions special options to pass to the GnuPG process
	 * @param archive the archive type to use
	 */
	FolderCompressJob(QObject *parent, const QList<QUrl> &sources, const QUrl &dest, QTemporaryFile *tempfile, const QStringList &keys, const QStringList &options, const KGpgEncrypt::EncryptOptions encOptions, const int archive);

	/**
	 * @brief FolderCompressJob destructor
	 */
        ~FolderCompressJob() override;

	/**
	 * @brief shows the progress indicator
	 */
        void start() override;

	/**
	 * @brief query extension for archive type
	 * @param archive the archive type
	 * @return the extension including leading dot
	 */
	static QString extensionForArchive(const int archive);

	/**
	 * @brief get list of supported archive names
	 * @return list of archive names
	 */
	static const QStringList &archiveNames();

private Q_SLOTS:
	void doWork();
	void slotEncryptionDone(int result);
};

#endif /* FOLDERCOMPRESSJOB_H */
