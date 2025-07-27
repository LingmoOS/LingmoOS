/*
    SPDX-FileCopyrightText: 2010, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDECRYPT_H
#define KGPGDECRYPT_H

#include <QObject>

#include <QUrl>
#include <QStringList>
#include "kgpgtextorfiletransaction.h"

/**
 * @brief decrypt the given text or files
 */
class KGpgDecrypt: public KGpgTextOrFileTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgDecrypt)
	KGpgDecrypt() = delete;
public:
	/**
	 * @brief decrypt given text
	 * @param parent parent object
	 * @param text text to decrypt
	 */
	explicit KGpgDecrypt(QObject *parent, const QString &text = QString());

	/**
	 * @brief decrypt file(s)
	 * @param parent parent object
	 * @param files list of file locations to decrypt
	 */
	KGpgDecrypt(QObject *parent, const QList<QUrl> &files);

	/**
	 * @brief decrypt file to given output filename
	 * @param parent parent object
	 * @param infile name of file to decrypt
	 * @param outfile name of file to write output to (will be overwritten)
	 */
	KGpgDecrypt(QObject *parent, const QUrl &infile, const QUrl &outfile);

	/**
	 * @brief destructor
	 */
    ~KGpgDecrypt() override;

	/**
	 * @brief get decryption result
	 * @return decrypted text
	 */
	QStringList decryptedText() const;

	/**
	 * @brief check if the given text contains an encoded message
	 * @param text text to check
	 * @param startPos if not nullptr start offset of encoded text will be returned here
	 * @param endPos if not nullptr end offset of encoded text will be returned here
	 */
	static bool isEncryptedText(const QString &text, int *startPos = nullptr, int *endPos = nullptr);

protected:
	QStringList command() const override;
	bool nextLine(const QString &line) override;
	void finish() override;
	bool closeInputAfterText() const override;

private:
	int m_fileIndex;
	int m_plainLength;	///< length of decrypted plain text if given by GnuPG
	const QString m_outFilename;	///< name of file to write output to
	bool decryptSuccess = false; //< flag to determine if decryption succeeded
};

#endif // KGPGDECRYPT_H
