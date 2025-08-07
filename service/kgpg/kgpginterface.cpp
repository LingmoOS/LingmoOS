/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpginterface.h"

#include "gpgproc.h"
#include "core/convert.h"
#include "core/KGpgKeyNode.h"
#include "core/KGpgSignNode.h"
#include "core/KGpgSubkeyNode.h"
#include "core/KGpgUatNode.h"
#include "core/KGpgUidNode.h"
#include "kgpgsettings.h"

#include <gpgme.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <QFile>
#include <QString>
#include <QTextStream>

using namespace KgpgCore;

QString KgpgInterface::getGpgSetting(const QString &name, const QString &configfile)
{
	const QString tmp = name.simplified() + QLatin1Char(' ');
	QFile qfile(configfile);

	if (qfile.exists() && qfile.open(QIODevice::ReadOnly)) {
		QTextStream t(&qfile);
		while (!t.atEnd()) {
			QString result(t.readLine().simplified());
			if (result.startsWith(tmp)) {
				result = result.mid(tmp.length()).simplified();
				return result.section(QLatin1Char( ' ' ), 0, 0);
			}
		}
		qfile.close();
	}

	return QString();
}

void KgpgInterface::setGpgSetting(const QString &name, const QString &value, const QString &url)
{
	QFile qfile(url);

	if (qfile.exists() && qfile.open(QIODevice::ReadOnly)) {
		const QString temp = name + QLatin1Char(' ');
		QString texttowrite;
		bool found = false;
		QTextStream tin(&qfile);

		while (!tin.atEnd()) {
			QString result = tin.readLine();
			if (result.simplified().startsWith(temp)) {
				if (!value.isEmpty())
					result = temp + QLatin1Char( ' ' ) + value;
				else
					result.clear();
				found = true;
			}

			texttowrite += result + QLatin1Char( '\n' );
		}

		qfile.close();
		if ((!found) && (!value.isEmpty()))
			texttowrite += QLatin1Char( '\n' ) + temp + QLatin1Char( ' ' ) + value;

		if (qfile.open(QIODevice::WriteOnly)) {
			QTextStream tout(&qfile);
			tout << texttowrite;
			qfile.close();
		}
	}
}

bool KgpgInterface::getGpgBoolSetting(const QString &name, const QString &configfile)
{
	QFile qfile(configfile);
	if (qfile.exists() && qfile.open(QIODevice::ReadOnly)) {
		QTextStream t(&qfile);
		while (!t.atEnd()) {
			if (t.readLine().simplified().startsWith(name))
				return true;
		}
		qfile.close();
	}
	return false;
}

void KgpgInterface::setGpgBoolSetting(const QString &name, const bool enable, const QString &url)
{
	QFile qfile(url);

	if (qfile.exists() && qfile.open(QIODevice::ReadOnly)) {
		QString texttowrite;
		bool found = false;
		QTextStream tin(&qfile);

		while (!tin.atEnd()) {
			QString result(tin.readLine());

			if (result.simplified().startsWith(name)) {
				if (enable)
					result = name;
				else
					result.clear();

				found = true;
			}

			texttowrite += result + QLatin1Char( '\n' );
		}
		qfile.close();

		if ((!found) && (enable))
			texttowrite += name;

		if (qfile.open(QIODevice::WriteOnly)) {
			QTextStream tout(&qfile);
			tout << texttowrite;
			qfile.close();
		}
	}
}

/**
 * @param p the process that reads the GnuPG data
 * @param readNode the node where the signatures are read for
 */
static KgpgCore::KgpgKeyList
readPublicKeysProcess(GPGProc &p, KGpgKeyNode *readNode)
{
	QStringList lsp;
	int items;
	KgpgCore::KgpgKeyList publiclistkeys;
	KgpgCore::KgpgKey *publickey = nullptr;
	unsigned int idIndex = 0;
	QString log;
	KGpgSignableNode *currentSNode = nullptr;	///< the current (sub)node signatures are read for

	while ((items = p.readln(lsp)) >= 0) {
		if ((lsp.at(0) == QLatin1String( "pub" )) && (items >= 10)) {
			KgpgSubKeyType subtype;
			KgpgSubKeyType keytype;
			bool enabled = true;
			if (items > 11) {
				const QString &caps = lsp.at(11);

				enabled = !caps.contains(QLatin1Char('D'), Qt::CaseSensitive);

				subtype = Convert::toSubType(caps, false);
				keytype = Convert::toSubType(caps, true);
			}

			const QString curve = (items > 16) ? lsp.at(16) : QString();
			publiclistkeys << KgpgKey(lsp.at(4), lsp.at(2).toUInt(), Convert::toTrust(lsp.at(1)),
					Convert::toAlgo(lsp.at(3)), subtype, keytype,
					Convert::toDateTime(lsp.at(5)), curve);

			publickey = &publiclistkeys.last();

			const QString &owTrust = lsp.at(8);
			if (owTrust.isEmpty())
				publickey->setOwnerTrust(GPGME_VALIDITY_UNDEFINED);
			else
				publickey->setOwnerTrust(Convert::toOwnerTrust(owTrust[0]));

			publickey->setExpiration(Convert::toDateTime(lsp.at(6)));

			publickey->setValid(enabled);  // disabled key

			// GnuPG since 2.1 can flag immediately if a key has a secret key
			if (items > 14 && lsp.at(14).contains(QLatin1Char('+')))
				publickey->setSecret(true);

			idIndex = 0;
		} else if (publickey && (lsp.at(0) == QLatin1String("fpr")) && (items >= 10)) {
			const QString &fingervalue = lsp.at(9);

			if ((currentSNode != nullptr) && (currentSNode->getType() == ITYPE_SUB))
				static_cast<KGpgSubkeyNode *>(currentSNode)->setFingerprint(fingervalue);
			else if (publickey->fingerprint().isEmpty())
				publickey->setFingerprint(fingervalue);
		} else if (publickey && (lsp.at(0) == QLatin1String( "sub" )) && (items >= 7)) {
			KgpgSubKeyType subtype;

			if (items > 11)
				subtype = Convert::toSubType(lsp.at(11), false);

			const QString curve = (items > 16) ? lsp.at(16) : QString();
			KgpgKeySub sub(lsp.at(4), lsp.at(2).toUInt(), Convert::toTrust(lsp.at(1)),
					Convert::toAlgo(lsp.at(3)), subtype, Convert::toDateTime(lsp.at(5)),
					curve);

			// FIXME: Please see kgpgkey.h, KgpgSubKey class
			if (items <= 11)
				sub.setValid(true);
			else
				sub.setValid(!lsp.at(11).contains(QLatin1Char( 'D' )));

			sub.setExpiration(Convert::toDateTime(lsp.at(6)));

			publickey->subList()->append(sub);
			if (readNode == nullptr)
				currentSNode = nullptr;
			else
				currentSNode = new KGpgSubkeyNode(readNode, sub);
		} else if (publickey && (lsp.at(0) == QLatin1String( "uat" ))) {
			idIndex++;
			if (readNode != nullptr) {
				currentSNode = new KGpgUatNode(readNode, idIndex, lsp);
			}
		} else if (publickey && (lsp.at(0) == QLatin1String( "uid" )) && (items >= 10)) {
			if (idIndex == 0) {
				QString fullname(lsp.at(9));
				QString kmail;
				if (fullname.contains(QLatin1Char( '<' )) ) {
					kmail = fullname;

					if (fullname.contains(QLatin1Char( ')' )) )
						kmail = kmail.section(QLatin1Char( ')' ), 1);

					kmail = kmail.section(QLatin1Char( '<' ), 1);
					kmail.chop(1);

					if (kmail.contains(QLatin1Char( '<' ))) {
						// several email addresses in the same key
						kmail.replace(QLatin1Char( '>' ), QLatin1Char( ';' ));
						kmail.remove(QLatin1Char( '<' ));
					}
				}

				QString kname(fullname.section( QLatin1String( " <" ), 0, 0));
				QString comment;
				if (fullname.contains(QLatin1Char( '(' )) ) {
					kname = kname.section( QLatin1String( " (" ), 0, 0);
					comment = fullname.section(QLatin1Char( '(' ), 1, 1);
					comment = comment.section(QLatin1Char( ')' ), 0, 0);
				}

				idIndex++;
				publickey->setEmail(kmail);
				publickey->setComment(comment);
				publickey->setName(kname);

				currentSNode = readNode;
			} else {
				idIndex++;
				if (readNode != nullptr) {
					currentSNode = new KGpgUidNode(readNode, idIndex, lsp);
				}
			}
		} else if (publickey && ((lsp.at(0) == QLatin1String( "sig" )) || (lsp.at(0) == QLatin1String( "rev" ))) && (items >= 11)) {
			if (currentSNode != nullptr)
				(void) new KGpgSignNode(currentSNode, lsp);
		} else {
			log += lsp.join(QString(QLatin1Char( ':' ))) + QLatin1Char( '\n' );
		}
	}

	if (p.exitCode() != 0) {
		KMessageBox::detailedError(nullptr, i18n("An error occurred while scanning your keyring"), log);
		log.clear();
	}

	return publiclistkeys;
}

KgpgKeyList KgpgInterface::readPublicKeys(const QStringList &ids)
{
	GPGProc process;
	process <<
			QLatin1String("--with-colons") <<
			QLatin1String("--with-fingerprint") <<
			QLatin1String("--fixed-list-mode") <<
			QLatin1String("--list-keys");

	if (GPGProc::gpgVersion(GPGProc::gpgVersionString(KGpgSettings::gpgBinaryPath())) > 0x20100)
		process << QLatin1String("--with-secret");

	process << ids;

	process.setOutputChannelMode(KProcess::MergedChannels);

	process.start();
	process.waitForFinished(-1);
	return readPublicKeysProcess(process, nullptr);
}

void KgpgInterface::readSignatures(KGpgKeyNode *node)
{
	GPGProc process;
	process <<
			QLatin1String("--with-colons") <<
			QLatin1String("--with-fingerprint") <<
			QLatin1String("--fixed-list-mode") <<
			QLatin1String("--list-sigs") <<
			node->getId();

	process.setOutputChannelMode(KProcess::MergedChannels);

	process.start();
	process.waitForFinished(-1);

	readPublicKeysProcess(process, node);
}

static KgpgCore::KgpgKeyList
readSecretKeysProcess(GPGProc &p)
{
	QStringList lsp;
	int items;
	bool hasuid = false;
	KgpgCore::KgpgKeyList result;
	KgpgCore::KgpgKey *secretkey = nullptr;

	while ( (items = p.readln(lsp)) >= 0 ) {
		if ((lsp.at(0) == QLatin1String( "sec" )) && (items >= 10)) {
			KgpgSubKeyType subtype;
			KgpgSubKeyType keytype;

			if (items >= 11) {
				const QString &caps = lsp.at(11);

				subtype = Convert::toSubType(caps, false);
				keytype = Convert::toSubType(caps, true);
			}

			const QString curve = (items > 16) ? lsp.at(16) : QString();
			result << KgpgKey(lsp.at(4), lsp.at(2).toUInt(), Convert::toTrust(lsp.at(1)),
				Convert::toAlgo(lsp.at(3)), subtype, keytype,
				Convert::toDateTime(lsp.at(5)), curve);

			secretkey = &result.last();

			secretkey->setSecret(true);

			secretkey->setExpiration(Convert::toDateTime(lsp.at(6)));

			hasuid = true;
		} else if ((lsp.at(0) == QLatin1String( "uid" )) && (items >= 10)) {
			if (hasuid)
				continue;

			hasuid = true;

			const QString &fullname = lsp.at(9);
			if (fullname.contains(QLatin1Char( '<' ) )) {
				QString kmail(fullname);

				if (fullname.contains(QLatin1Char( ')' ) ))
					kmail = kmail.section(QLatin1Char( ')' ), 1);

				kmail = kmail.section(QLatin1Char( '<' ), 1);
				kmail.chop(1);

				if (kmail.contains(QLatin1Char( '<' ) )) { // several email addresses in the same key
					kmail.replace(QLatin1Char( '>' ), QLatin1Char( ';' ));
					kmail.remove(QLatin1Char( '<' ));
				}

				secretkey->setEmail(kmail);
			} else {
				secretkey->setEmail(QString());
			}

			QString kname(fullname.section( QLatin1String( " <" ), 0, 0));
			if (fullname.contains(QLatin1Char( '(' ) )) {
				kname = kname.section( QLatin1String( " (" ), 0, 0);
				QString comment = fullname.section(QLatin1Char( '(' ), 1, 1);
				comment = comment.section(QLatin1Char( ')' ), 0, 0);

				secretkey->setComment(comment);
			} else {
				secretkey->setComment(QString());
			}
			secretkey->setName(kname);
		} else if ((lsp.at(0) == QLatin1String( "fpr" )) && (items >= 10)) {
			secretkey->setFingerprint(lsp.at(9));
		}
	}

	return result;
}

KgpgKeyList KgpgInterface::readSecretKeys(const QStringList &ids)
{
	GPGProc process;
	process <<
			QLatin1String("--with-colons") <<
			QLatin1String("--list-secret-keys") <<
			QLatin1String("--with-fingerprint") <<
			QLatin1String("--fixed-list-mode") <<
			ids;

	process.start();
	process.waitForFinished(-1);
	return readSecretKeysProcess(process);
}
