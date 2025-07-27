/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgtextinterface.h"

#include "gpgproc.h"
#include "kgpgsettings.h"

#include <QString>

class KGpgTextInterfacePrivate
{
	Q_DISABLE_COPY(KGpgTextInterfacePrivate)
public:
	KGpgTextInterfacePrivate(QObject *parent, const QString &keyID, const QStringList &options);

	GPGProc * const m_process;
	int m_step;
	const QStringList m_gpgopts;
	QList<QUrl> m_files;

	void signFile(const QString &fileName);
};

static QStringList
buildCmdLine(const QString &keyID, const QStringList &options)
{
	return QStringList(QLatin1String("-u")) <<
		keyID <<
		options <<
		QLatin1String("--detach-sign") <<
		QLatin1String("--output");
}

KGpgTextInterfacePrivate::KGpgTextInterfacePrivate(QObject *parent, const QString &keyID, const QStringList &options)
	: m_process(new GPGProc(parent)),
	m_step(0),
	m_gpgopts(buildCmdLine(keyID, options))
{
}

void
KGpgTextInterfacePrivate::signFile(const QString &fileName)
{
	m_process->resetProcess();
	*m_process <<
			m_gpgopts <<
			fileName + QLatin1String(".sig") <<
			fileName;

	m_process->start();
}

KGpgTextInterface::KGpgTextInterface(QObject *parent, const QString &keyID, const QStringList &options)
	: QObject(parent),
	d(new KGpgTextInterfacePrivate(parent, keyID, options))
{
	connect(d->m_process, &GPGProc::processExited, this, &KGpgTextInterface::slotSignFile);
}

KGpgTextInterface::~KGpgTextInterface()
{
	delete d->m_process;
	delete d;
}

// signatures
void
KGpgTextInterface::signFiles(const QList<QUrl> &srcUrls)
{
	d->m_files = srcUrls;

	slotSignFile();
}

void
KGpgTextInterface::slotSignFile()
{
	const QString fileName = d->m_files.takeFirst().path();

	if (d->m_files.isEmpty()) {
		disconnect(d->m_process, &GPGProc::processExited, this, &KGpgTextInterface::slotSignFile);
		connect(d->m_process, &GPGProc::processExited, this, &KGpgTextInterface::slotSignFinished);
	}

	d->signFile(fileName);
}

void
KGpgTextInterface::slotSignFinished()
{
	Q_EMIT fileSignFinished();
}
