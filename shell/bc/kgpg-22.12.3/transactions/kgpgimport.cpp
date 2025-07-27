/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgimport.h"
#include "kgpg_general_debug.h"

#include "model/kgpgitemmodel.h"
#include "core/KGpgKeyNode.h"



#include <KLocalizedString>

KGpgImport::KGpgImport(QObject *parent, const QString &text)
	: KGpgTextOrFileTransaction(parent, text, true)
{
}

KGpgImport::KGpgImport(QObject *parent, const QList<QUrl> &files)
	: KGpgTextOrFileTransaction(parent, files, true)
{
}

KGpgImport::~KGpgImport()
{
}

QStringList
KGpgImport::command() const
{
	QStringList ret;

	ret << QLatin1String( "--import" ) << QLatin1String( "--allow-secret-key-import" );

	return ret;
}

QStringList
KGpgImport::getImportedKeys() const
{
	QStringList res;

	for (const QString &str : getMessages())
		if (str.startsWith(QLatin1String("[GNUPG:] IMPORTED ")))
			res << str.mid(18);

	return res;
}

QStringList
KGpgImport::getImportedIds(const QStringList &log, const int reason)
{
	QStringList res;

	for (const QString &str : log) {
		if (!str.startsWith(QLatin1String("[GNUPG:] IMPORT_OK ")))
			continue;

		QString tmpstr(str.mid(19).simplified());

		int space = tmpstr.indexOf(QLatin1Char( ' ' ));
		if (space <= 0) {
			qCDebug(KGPG_LOG_GENERAL) << __LINE__ << "invalid format:" << str;
			continue;
		}

		bool ok;
		unsigned char code = QStringView(tmpstr).left(space).toUInt(&ok);
		if (!ok) {
			qCDebug(KGPG_LOG_GENERAL) << __LINE__ << "invalid format:" << str << space << QStringView(tmpstr).left(space - 1);
			continue;
		}

		if ((reason == -1) || ((reason == 0) && (code == 0)) || ((reason & code) != 0))
			res << tmpstr.mid(space + 1);
	}

	return res;
}

QStringList
KGpgImport::getImportedIds(const int reason) const
{
	return getImportedIds(getMessages(), reason);
}

QString
KGpgImport::getImportMessage() const
{
	return getImportMessage(getMessages());
}

QString
KGpgImport::getImportMessage(const QStringList &log)
{
#define RESULT_PARTS_MIN 14
#define RESULT_PARTS_MAX 15
	unsigned long rcode[RESULT_PARTS_MAX];
	int line = 0;
	bool fine = false;

	memset(rcode, 0, sizeof(rcode));

	for (const QString &str : log) {
		line++;
		if (!str.startsWith(QLatin1String("[GNUPG:] IMPORT_RES ")))
			continue;

		const QStringList rstr = str.mid(20).simplified().split(QLatin1Char(' '));

		bool syn = (rstr.count() >= RESULT_PARTS_MIN);

		for (int i = std::min<int>(rstr.count(), RESULT_PARTS_MAX) - 1; (i >= 0) && syn; i--) {
			rcode[i] += rstr.at(i).toULong(&syn);
			fine |= (rcode[i] != 0);
		}

		if (!syn)
			return xi18nc("@info", "The import result string has an unsupported format in line %1.<nl/>Please see the detailed log for more information.", line);
	}

	if (!fine)
		return i18n("No key imported.<br />Please see the detailed log for more information.");

	QString resultMessage(xi18ncp("@info", "<para>%1 key processed.</para>", "<para>%1 keys processed.</para>", rcode[0]));

	if (rcode[1])
		resultMessage += xi18ncp("@info", "<para>One key without ID.</para>", "<para>%1 keys without ID.</para>", rcode[1]);
	if (rcode[2])
		resultMessage += xi18ncp("@info", "<para><emphasis strong='true'>One key imported:</emphasis></para>", "<para><emphasis strong='true'>%1 keys imported:</emphasis></para>", rcode[2]);
	if (rcode[3])
		resultMessage += xi18ncp("@info", "<para>One RSA key imported.</para>", "<para>%1 RSA keys imported.</para>", rcode[3]);
	if (rcode[4])
		resultMessage += xi18ncp("@info", "<para>One key unchanged.</para>", "<para>%1 keys unchanged.</para>", rcode[4]);
	if (rcode[5])
		resultMessage += xi18ncp("@info", "<para>One user ID imported.</para>", "<para>%1 user IDs imported.</para>", rcode[5]);
	if (rcode[6])
		resultMessage += xi18ncp("@info", "<para>One subkey imported.</para>", "<para>%1 subkeys imported.</para>", rcode[6]);
	if (rcode[7])
		resultMessage += xi18ncp("@info", "<para>One signature imported.</para>", "<para>%1 signatures imported.</para>", rcode[7]);
	if (rcode[8])
		resultMessage += xi18ncp("@info", "<para>One revocation certificate imported.</para>", "<para>%1 revocation certificates imported.</para>", rcode[8]);
	if (rcode[9])
		resultMessage += xi18ncp("@info", "<para>One secret key processed.</para>", "<para>%1 secret keys processed.</para>", rcode[9]);
	if (rcode[10])
		resultMessage +=  xi18ncp("@info", "<para><emphasis strong='true'>One secret key imported.</emphasis></para>", "<para><emphasis strong='true'>%1 secret keys imported.</emphasis></para>", rcode[10]);
	if (rcode[11])
		resultMessage += xi18ncp("@info", "<para>One secret key unchanged.</para>", "<para>%1 secret keys unchanged.</para>", rcode[11]);
	if (rcode[12])
		resultMessage += xi18ncp("@info", "<para>One secret key not imported.</para>", "<para>%1 secret keys not imported.</para>", rcode[12]);

	if (rcode[9])
		resultMessage += xi18nc("@info", "<para><emphasis strong='true'>You have imported a secret key.</emphasis><nl/>"
		"Please note that imported secret keys are not trusted by default.<nl/>"
		"To fully use this secret key for signing and encryption, you must edit the key (double click on it) and set its trust to Full or Ultimate.</para>");

	return resultMessage;
}

static QString
beautifyKeyList(const QStringList &keyIds, const KGpgItemModel *model)
{
	QString result;

	result.append(QLatin1String("\n"));
	if (model == nullptr) {
		result.append(QLatin1Char(' ') + keyIds.join(QLatin1String("\n ")));
	} else {
		for (const QString &changed : keyIds) {
			const KGpgKeyNode *node = model->findKeyNode(changed);
			QString line;

			if (node == nullptr) {
				line = changed;
			} else {
				if (node->getEmail().isEmpty())
					line = xi18nc("@item ID: Name", "%1: %2", node->getFingerprint(), node->getName());
				else
					line = xi18nc("@item ID: Name <Email>", "%1: %2 <email>%3</email>", node->getFingerprint(), node->getName(), node->getEmail());
			}

			result.append(QLatin1Char(' ') + line + QLatin1String("\n"));
		}
	}

	return result;
}

QString
KGpgImport::getDetailedImportMessage(const QStringList &log, const KGpgItemModel *model)
{
	QString result;
	QMap<QString, unsigned int> resultcodes;

	for (const QString &keyresult : log) {
		if (!keyresult.startsWith(QLatin1String("[GNUPG:] IMPORT_OK ")))
			continue;

		QStringList rc(keyresult.mid(19).split(QLatin1Char( ' ' )));
		if (rc.count() < 2) {
			qCDebug(KGPG_LOG_GENERAL) << "unexpected syntax:" << keyresult;
			continue;
		}

		resultcodes[rc.at(1)] = rc.at(0).toUInt();
	}

	const QMap<QString, unsigned int>::const_iterator iterend = resultcodes.constEnd();

	for (unsigned int flag = 1; flag <= 16; flag <<= 1) {
		QStringList thischanged;

		for (QMap<QString, unsigned int>::const_iterator iter = resultcodes.constBegin(); iter != iterend; ++iter) {
			if (iter.value() & flag)
				thischanged << iter.key();
		}

		if (thischanged.isEmpty())
			continue;

		switch (flag) {
		case 1:
			result.append(i18np("New Key", "New Keys", thischanged.count()));
			break;
		case 2:
			result.append(i18np("Key with new User Id", "Keys with new User Ids", thischanged.count()));
			break;
		case 4:
			result.append(i18np("Key with new Signatures", "Keys with new Signatures", thischanged.count()));
			break;
		case 8:
			result.append(i18np("Key with new Subkeys", "Keys with new Subkeys", thischanged.count()));
			break;
		case 16:
			result.append(i18np("New Private Key", "New Private Keys", thischanged.count()));
			break;
		default:
			Q_ASSERT(flag == 1);
		}

		result.append(beautifyKeyList(thischanged, model));
		result.append(QLatin1String("\n\n"));
	}

	QStringList unchanged(resultcodes.keys(0));

	if (unchanged.isEmpty()) {
		// remove empty line at end
		result.chop(1);
	} else {
		result.append(i18np("Unchanged Key", "Unchanged Keys", unchanged.count()));
		result.append(beautifyKeyList(unchanged, model));
		result.append(QLatin1String("\n"));
	}

	return result;
}

int
KGpgImport::isKey(const QString &text, const bool incomplete)
{
	int markpos = text.indexOf(QLatin1String("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
	if (markpos >= 0) {
		markpos = text.indexOf(QLatin1String("-----END PGP PUBLIC KEY BLOCK-----"), markpos);
		return ((markpos > 0) || incomplete) ? 1 : 0;
	}

	markpos = text.indexOf(QLatin1String("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
	if (markpos < 0)
		return 0;

	markpos = text.indexOf(QLatin1String("-----END PGP PRIVATE KEY BLOCK-----"), markpos);
	if ((markpos < 0) && !incomplete)
		return 0;

	return 2;
}
