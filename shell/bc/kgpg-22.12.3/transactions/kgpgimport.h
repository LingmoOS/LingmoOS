/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGIMPORT_H
#define KGPGIMPORT_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <QUrl>

#include "kgpgtextorfiletransaction.h"

class KGpgItemModel;

/**
 * @brief import one or more keys into the keyring
 */
class KGpgImport: public KGpgTextOrFileTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgImport)
public:
	/**
	 * @brief import given text
	 * @param parent parent object
	 * @param text key text to import
	 */
	explicit KGpgImport(QObject *parent, const QString &text = QString());

	/**
	 * @brief import key(s) from file(s)
	 * @param parent parent object
	 * @param files list of file locations to import from
	 */
	KGpgImport(QObject *parent, const QList<QUrl> &files);

	/**
	 * @brief destructor
	 */
    ~KGpgImport() override;

	/**
	 * @brief get the names and short fingerprints of the imported keys
	 * @return list of keys that were imported
	 */
	QStringList getImportedKeys() const;

	/**
	 * @brief get the full fingerprints of the imported keys
	 * @param log transaction log to scan
	 * @param reason key import reason
	 * @return list of ids that were imported
	 *
	 * You can filter the list of keys returned by the status of that key
	 * as reported by GnuPG. See doc/DETAILS of GnuPG for the meaning of
	 * the different flags.
	 *
	 * If reason is -1 (the default) all processed key ids are returned.
	 * If reason is 0 only keys of status 0 (unchanged) are returned. For
	 * any other value a key is returned if one of his status bits matched
	 * one of the bits in reason (i.e. (reason & status) != 0).
	 */
	static QStringList getImportedIds(const QStringList &log, const int reason = -1);
	/**
	 * @brief get the full fingerprints of the imported keys
	 *
	 * This is an overloaded member. It calls the static function with the
	 * result log from this transaction object.
	 */
	QStringList getImportedIds(const int reason = -1) const;

	/**
	 * @brief get textual summary of the import events
	 * @return messages describing what was imported
	 *
	 * This is an overloaded member. It calls the static function with the
	 * result log from this transaction object.
	 */
	QString getImportMessage() const;

	/**
	 * @brief get textual summary of the import events
	 * @param log import log
	 * @return messages describing what was imported
	 *
	 * The log must contain a "IMPORT_RES" line. If this is not present
	 * the result string will contain an error message.
	 */
	static QString getImportMessage(const QStringList &log);

	/**
	 * @brief get detailed summary of import
	 * @param log import log
	 * @param model item model, @see KGpgItemModel
	 * @return message describing which keys changed and how
	 *
	 * The log must contain a "IMPORT_RES" line. If this is not present
	 * the result string will contain an error message.
	 */
	static QString getDetailedImportMessage(const QStringList &log, const KGpgItemModel *model = nullptr);

	/**
	 * @brief check if the given text contains a private or public key
	 * @param text text to check
	 * @param incomplete assume text is only the beginning of the data
	 * @return if text contains a key or not
	 * @retval 0 no key found
	 * @retval 1 public key found
	 * @retval 2 private key found
	 */
	static int isKey(const QString &text, const bool incomplete = false);

protected:
	QStringList command() const override;
};

#endif // KGPGIMPORT_H
