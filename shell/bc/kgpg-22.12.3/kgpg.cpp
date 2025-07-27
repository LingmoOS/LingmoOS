/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-FileCopyrightText: 2016 Andrius Štikonas <andrius@stikonas.eu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpg.h"

#include "gpgproc.h"
#include "kgpgsettings.h"
#include "keysmanager.h"
#include "kgpg_interface.h"
#include "kgpgexternalactions.h"
#include "kgpginterface.h"
#include "core/images.h"
#include "editor/kgpgeditor.h"
#include "transactions/kgpgimport.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTextStream>

#include <KMessageBox>
#include <KWindowSystem>

using namespace KgpgCore;

KGpgApp::KGpgApp(int &argc, char **argv)
             : QApplication(argc, argv),
	     w(nullptr),
         s_keyManager(nullptr)
{
}

KGpgApp::~KGpgApp()
{
	delete s_keyManager;
}

void KGpgApp::slotHandleQuit()
{
	s_keyManager->saveToggleOpts();
	quit();
}

void KGpgApp::assistantOver(const QString &defaultKeyId)
{
	if (!defaultKeyId.isEmpty())
		s_keyManager->slotSetDefaultKey(defaultKeyId);

	s_keyManager->show();
	s_keyManager->raise();
}

bool KGpgApp::newInstance()
{
	const QString gpgError = GPGProc::getGpgStartupError(KGpgSettings::gpgBinaryPath());
	if (!gpgError.isEmpty()) {
                KMessageBox::detailedError(nullptr, i18n("GnuPG failed to start.<br />You must fix the GnuPG error first before running KGpg."), gpgError, i18n("GnuPG error"));
		QApplication::quit();
		return false;
	}

	s_keyManager = new KeysManager();

	w = new KGpgExternalActions(s_keyManager, s_keyManager->getModel());

	connect(s_keyManager, &KeysManager::readAgainOptions, w, &KGpgExternalActions::readOptions);
	connect(w, &KGpgExternalActions::updateDefault, this, &KGpgApp::assistantOver);
	connect(w, &KGpgExternalActions::createNewKey, s_keyManager, &KeysManager::slotGenerateKey);

	const QString gpgPath = KGpgSettings::gpgConfigPath();

	if (!gpgPath.isEmpty()) {
		const int gpgver = GPGProc::gpgVersion(GPGProc::gpgVersionString(KGpgSettings::gpgBinaryPath()));

		// Warn if sign of a properly running gpg-agent cannot be determined
		// The environment variable has been removed in GnuPG 2.1, the agent is started internally by
		// any program part of GnuPG that needs it, so simply assume everything is fine.
		if ((gpgver < 0x20100) && KgpgInterface::getGpgBoolSetting(QLatin1String("use-agent"), gpgPath) &&
				qEnvironmentVariableIsEmpty("GPG_AGENT_INFO"))
                        KMessageBox::error(nullptr, i18n("<qt>The use of <b>GnuPG Agent</b> is enabled in GnuPG's configuration file (%1).<br />"
				"However, the agent does not seem to be running. This could result in problems with signing/decryption.<br />"
				"Please disable GnuPG Agent from KGpg settings, or fix the agent.</qt>", gpgPath));
	}

	return true;
}

void KGpgApp::handleArguments(const QCommandLineParser &parser, const QDir &workingDirectory)
{
	// parsing of command line args
    if (parser.isSet(QStringLiteral("k")) || (!KGpgSettings::showSystray() && parser.positionalArguments().isEmpty() && !parser.isSet(QStringLiteral("d")))) {
		s_keyManager->show();
		KWindowSystem::setOnDesktop(s_keyManager->winId(), KWindowSystem::currentDesktop());  //set on the current desktop
		KWindowSystem::unminimizeWindow(s_keyManager->winId());  //de-iconify window
		s_keyManager->raise();  // set on top
    } else if (parser.isSet(QStringLiteral("d"))) {
		s_keyManager->slotOpenEditor();
		s_keyManager->hide();
	} else {
		QList<QUrl> urlList;

		const QStringList positionalArguments = parser.positionalArguments();
		for (const QString &arg : positionalArguments)
			urlList.append(QUrl::fromLocalFile(workingDirectory.absoluteFilePath(arg)));

		bool directoryInside = false;
		for (const QUrl &url : qAsConst(urlList)) {
			QMimeDatabase db;
			if (db.mimeTypeForUrl(url).name() == QLatin1String( "inode/directory" )) {
				directoryInside = true;
				break;
			}
		}

        if (parser.isSet(QStringLiteral("e"))) {
			if (urlList.isEmpty())
                                KMessageBox::error(nullptr, i18n("No files given."));
			else if (!directoryInside)
				KGpgExternalActions::encryptFiles(s_keyManager, urlList);
			else
				KGpgExternalActions::encryptFolders(s_keyManager, urlList);
        } else if (parser.isSet(QStringLiteral("s"))) {
			if (urlList.isEmpty())
                                KMessageBox::error(nullptr, i18n("No files given."));
			else if (!directoryInside)
				w->showDroppedFile(urlList.first());
			else
                                KMessageBox::error(nullptr, i18n("Cannot decrypt and show folder."));
        } else if (parser.isSet(QStringLiteral("S"))) {
			if (urlList.isEmpty())
                                KMessageBox::error(nullptr, i18n("No files given."));
			else if (!directoryInside)
				KGpgExternalActions::signFiles(s_keyManager, urlList);
			else
                                KMessageBox::error(nullptr, i18n("Cannot sign folder."));
        } else if (parser.isSet(QStringLiteral("V")) != 0) {
			if (urlList.isEmpty())
                                KMessageBox::error(nullptr, i18n("No files given."));
			else if (!directoryInside)
				w->verifyFile(urlList.first());
			else
                                KMessageBox::error(nullptr, i18n("Cannot verify folder."));
		} else {
			if (directoryInside && (urlList.count() > 1)) {
                                KMessageBox::error(nullptr, i18n("Unable to perform requested operation.\nPlease select only one folder, or several files, but do not mix files and folders."));
				return;
			}

			if (urlList.isEmpty()) {
				/* do nothing */
			} else if (urlList.first().fileName().endsWith(QLatin1String(".sig"))) {
				w->verifyFile(urlList.first());
			} else {
				bool haskeys = false;
				bool hastext = false;
				for (const QUrl &url : qAsConst(urlList)) {
					QFile qfile(url.path());
					if (qfile.open(QIODevice::ReadOnly)) {
						const int probelen = 4096;
						QTextStream t(&qfile);
						QString probetext(t.read(probelen));
						qfile.close();

						if (KGpgImport::isKey(probetext, probetext.length() == probelen))
							haskeys = true;
						else
							hastext = true;
					}
				}

				if (hastext) {
					KGpgExternalActions::decryptFiles(s_keyManager, urlList);
				} else if (haskeys) {
					s_keyManager->slotImport(urlList);
				}
			}
		}
	}
}

void KGpgApp::setupCmdlineParser(QCommandLineParser& parser)
{
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("e"), i18n("Encrypt file")));
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("k"), i18n("Open key manager")));
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("d"), i18n("Open editor")));
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("s"), i18n("Show encrypted file")));
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("S"), i18n("Sign File")));
	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("V"), i18n("Verify signature")));
	parser.addPositionalArgument(QLatin1String("[File]"), i18n("File to open"));
}

void KGpgApp::slotDBusActivation(const QStringList &arguments, const QString &workingDirectory)
{
	QCommandLineParser parser;

	setupCmdlineParser(parser);

	parser.parse(arguments);

	handleArguments(parser, QDir(workingDirectory));
}
