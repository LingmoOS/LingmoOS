/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGTEXTORFILETRANSACTION_H
#define KGPGTEXTORFILETRANSACTION_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <QUrl>

#include "kgpgtransaction.h"

/**
 * @brief feed a text or file through gpg
 */
class KGpgTextOrFileTransaction: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgTextOrFileTransaction)

public:
	/**
	 * @brief additional status codes for KGpgImport
	 */
	enum ts_import {
		TS_KIO_FAILED = TS_COMMON_END + 1	///< download of remote file failed
	};

protected:
	/**
	 * @brief work with given text
	 * @param parent parent object
	 * @param text text to work with
	 * @param allowChaining whether to allow chaining
	 */
    explicit KGpgTextOrFileTransaction(QObject *parent = nullptr, const QString &text = QString(), const bool allowChaining = false);

	/**
	 * @brief work with given file(s)
	 * @param parent parent object
	 * @param files list of file locations to work with
	 * @param allowChaining whether to allow chaining
	 */
	KGpgTextOrFileTransaction(QObject *parent, const QList<QUrl> &files, const bool allowChaining = false);

public:
	/**
	 * @brief destructor
	 */
    ~KGpgTextOrFileTransaction() override;

	/**
	 * @brief set text to work with
	 * @param text text to work with
	 */
	void setText(const QString &text);
	/**
	 * @brief set file locations to work with
	 * @param files list of file locations to work with
	 */
	void setUrls(const QList<QUrl> &files);

	/**
	 * @brief get gpg info message
	 * @return the raw messages from gpg during the operation
	 */
	const QStringList &getMessages() const;

protected:
	/**
	 * @brief construct the command line of the process
	 */
	bool preStart() override;
	bool nextLine(const QString &line) override;
	/**
	 * @brief implement special handling for GnuPG return codes
	 */
	void finish() override;

	virtual QStringList command() const = 0;

	const QList<QUrl> &getInputFiles() const;

	/**
	 * @brief if the input channel of GnuPG should be closed when text is written
	 *
	 * This only has an effect in text mode (i.e. without urls).
	 *
	 * If not given the input channel is closed when "--command-fd" is not used.
	 */
	virtual bool closeInputAfterText() const
	{
		return false;
	}

private:
	QStringList m_tempfiles;
	QStringList m_locfiles;
	QList<QUrl> m_inpfiles;
	QString m_text;
	QStringList m_messages;
	bool m_closeInput = false;	///< if input channel of GnuPG should be closed after m_text is written

	void cleanUrls();

private Q_SLOTS:
	void postStart() override;
};

#endif // KGPGTEXTORFILETRANSACTION_H
