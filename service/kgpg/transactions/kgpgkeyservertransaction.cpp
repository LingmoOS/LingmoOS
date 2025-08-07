/*
    SPDX-FileCopyrightText: 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgkeyservertransaction.h"

#include "gpgproc.h"

#include <KLocalizedString>

#include <QLabel>
#include <QProgressDialog>

KGpgKeyserverTransaction::KGpgKeyserverTransaction(QObject *parent, const QString &keyserver, const bool withProgress, const QString &proxy)
	: KGpgTransaction(parent),
	m_progress(nullptr),
	m_showprogress(false)
{
	addArgument(QLatin1String( "--status-fd=1" ));
	addArgument(QLatin1String( "--command-fd=0" ));
	addArgument(QLatin1String( "--keyserver-options" ));
	m_proxypos = addArgument(QString());
	addArgument(QLatin1String( "--keyserver" ));
	m_keyserverpos = addArgument(QString());

	setKeyserver(keyserver);
	setProxy(proxy);

	setProgressEnable(withProgress);
}

KGpgKeyserverTransaction::~KGpgKeyserverTransaction()
{
	delete m_progress;
}

void
KGpgKeyserverTransaction::setKeyserver(const QString &server)
{
	m_keyserver = server;

	replaceArgument(m_keyserverpos, server);
}

void
KGpgKeyserverTransaction::setProxy(const QString &proxy)
{
	m_proxy = proxy;

	replaceArgument(m_proxypos, proxy);
}

void
KGpgKeyserverTransaction::finish()
{
	if (m_progress != nullptr)
		m_progress->hide();
}

bool
KGpgKeyserverTransaction::preStart()
{
	if (m_showprogress) {
		Q_ASSERT(m_progress != nullptr);
		m_progress->show();
	}

	return true;
}

void
KGpgKeyserverTransaction::slotAbort()
{
	// no idea if this works on Windows, maybe we need ->kill() there
	getProcess()->terminate();
	setSuccess(TS_USER_ABORTED);
}

void
KGpgKeyserverTransaction::setProgressEnable(const bool b)
{
	m_showprogress = b;

	if (b && (m_progress == nullptr)) {
		m_progress = new QProgressDialog(qobject_cast<QWidget *>(parent()));
		QLabel *label = new QLabel(i18n("<b>Connecting to the server...</b>"));
		m_progress->setWindowTitle(i18n("Keyserver"));
		m_progress->setLabel(label);

		m_progress->hide();
		m_progress->setModal(true);
		m_progress->setRange(0, 0);

		connect(m_progress, &QProgressDialog::canceled, this, &KGpgKeyserverTransaction::slotAbort);
	}
}
