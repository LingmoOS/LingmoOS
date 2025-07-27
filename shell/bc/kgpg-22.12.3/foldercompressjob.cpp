/*
    SPDX-FileCopyrightText: 2011, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foldercompressjob.h"

#include <KArchive>
#include <KLocalizedString>
#include <KTar>
#include <KZip>

#include <QDir>
#include <QMetaObject>
#include <QTemporaryFile>

class FolderCompressJobPrivate {
	FolderCompressJob * const q_ptr;
	Q_DECLARE_PUBLIC(FolderCompressJob)

public:
	FolderCompressJobPrivate(FolderCompressJob *parent, const QList<QUrl> &sources, const QUrl &dest, QTemporaryFile *tempfile, const QStringList &keys, const QStringList &options, const KGpgEncrypt::EncryptOptions encOptions, const int archive);

	const QString m_description;
	const QList<QUrl> m_sources;
	const QUrl m_dest;
	QTemporaryFile * const m_tempfile;
	const QStringList m_keys;
	QStringList m_options;
	const KGpgEncrypt::EncryptOptions m_encOptions;
	const int m_archiveType;
};

FolderCompressJobPrivate::FolderCompressJobPrivate(FolderCompressJob *parent, const QList<QUrl> &sources, const QUrl &dest, QTemporaryFile *tempfile, const QStringList &keys, const QStringList &options, const KGpgEncrypt::EncryptOptions encOptions, const int archive)
	: q_ptr(parent),
	m_description(i18n("Processing folder compression and encryption")),
	m_sources(sources),
	m_dest(dest),
	m_tempfile(tempfile),
	m_keys(keys),
	m_options(options),
	m_encOptions(encOptions),
	m_archiveType(archive)
{
}

FolderCompressJob::FolderCompressJob(QObject *parent, const QList<QUrl> &sources, const QUrl &dest, QTemporaryFile *tempfile, const QStringList &keys, const QStringList &options,  const KGpgEncrypt::EncryptOptions encOptions, const int archive)
	: KJob(parent),
	d_ptr(new FolderCompressJobPrivate(this, sources, dest, tempfile, keys, options, encOptions, archive))
{
}

FolderCompressJob::~FolderCompressJob()
{
	delete d_ptr;
}

void
FolderCompressJob::start()
{
	Q_D(FolderCompressJob);

	Q_EMIT description(this, d->m_description, qMakePair(i18nc("State of operation as in status", "State"), i18nc("Job is started up", "Startup")));
	QMetaObject::invokeMethod(this, "doWork", Qt::QueuedConnection);
}

void
FolderCompressJob::doWork()
{
	Q_D(FolderCompressJob);
	KArchive *arch = nullptr;

	switch (d->m_archiveType) {
	case 0:
		arch = new KZip(d->m_tempfile->fileName());
		break;
	case 1:
		arch = new KTar(d->m_tempfile->fileName(), QLatin1String( "application/x-gzip" ));
		break;
	case 2:
		arch = new KTar(d->m_tempfile->fileName(), QLatin1String( "application/x-bzip" ));
		break;
	case 3:
		arch = new KTar(d->m_tempfile->fileName(), QLatin1String( "application/x-tar" ));
		break;
	case 4:
		arch = new KTar(d->m_tempfile->fileName(), QLatin1String( "application/x-xz" ));
		break;
	default:
		Q_ASSERT(0);
		return;
	}

	if (!arch->open(QIODevice::WriteOnly)) {
		setError(UserDefinedError);
		setErrorText(i18n("Unable to create temporary file"));
		delete arch;
		emitResult();
		return;
	}

	for (const QUrl &url : d->m_sources)
		arch->addLocalDirectory(url.path(), url.fileName());
	arch->close();
	delete arch;

	setPercent(50);

	QDir outPath = d->m_sources.first().path();
	outPath.cdUp();

	d->m_options << QLatin1String("--output") << QDir::toNativeSeparators(outPath.path() + QDir::separator()) + d->m_dest.fileName();

	Q_EMIT description(this, d->m_description, qMakePair(i18nc("State of operation as in status", "State"),
			i18nc("Status message 'Encrypting <filename>' (operation starts)", "Encrypting %1", d->m_dest.path())));
		

	KGpgEncrypt *enc = new KGpgEncrypt(this, d->m_keys, QList<QUrl>({QUrl::fromLocalFile(d->m_tempfile->fileName())}), d->m_encOptions, d->m_options);
	connect(enc, &KGpgEncrypt::done, this, &FolderCompressJob::slotEncryptionDone);
	enc->start();
}

void
FolderCompressJob::slotEncryptionDone(int result)
{
	Q_D(FolderCompressJob);

	sender()->deleteLater();

	if ((result != KGpgTransaction::TS_OK) && (result != KGpgTransaction::TS_USER_ABORTED)) {
		setError(KJob::UserDefinedError + 1);
		setErrorText(i18n("The encryption failed with error code %1", result));
		Q_EMIT description(this, d->m_description, qMakePair(i18nc("State of operation as in status", "State"), i18n("Encryption failed.")));
	} else {
		Q_EMIT description(this, d->m_description, qMakePair(i18nc("State of operation as in status", "State"),
				i18nc("Status message 'Encrypted <filename>' (operation was completed)", "Encrypted %1", d->m_dest.path())));
	}

	emitResult();
}

QString
FolderCompressJob::extensionForArchive(const int archive)
{
	switch (archive) {
	case 0:
		return QLatin1String(".zip");
	case 1:
		return QLatin1String(".tar.gz");
	case 2:
		return QLatin1String(".tar.bz2");
	case 3:
		return QLatin1String(".tar");
	case 4:
		return QLatin1String(".tar.xz");
	default:
		Q_ASSERT(archive <= archiveNames().count());
		Q_ASSERT(archive >= 0);
		return QString();
	}
}

const QStringList &
FolderCompressJob::archiveNames()
{
	static const QStringList archives =
			QStringList(i18n("Zip")) <<
					i18n("Tar/Gzip") <<
					i18n("Tar/Bzip2") <<
					i18n("Tar") <<
					i18n("Tar/XZ");

	return archives;
}
