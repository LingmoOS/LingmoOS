/*
    SPDX-FileCopyrightText: 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGSIGNTEXT_H
#define KGPGSIGNTEXT_H

#include "kgpgtextorfiletransaction.h"

#include <QUrl>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief sign the given text or files
 */
class KGpgSignText: public KGpgTextOrFileTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgSignText)
	KGpgSignText() = delete;
public:
	enum SignOption {
		DefaultSignature = 0,		///< use whatever GnuPGs defaults are
		AsciiArmored = 0x1,		///< output the data as printable ASCII as opposed to binary data
		DetachedSignature = 0x2,	///< save the signature in a separate file
	};
        Q_DECLARE_FLAGS(SignOptions, SignOption)

	/**
	 * @brief sign given text
	 * @param parent parent object
	 * @param signId the key to use for signing
	 * @param text text to sign
	 * @param options signing options
	 * @param extraOptions extra signing options
	 */
	KGpgSignText(QObject *parent, const QString &signId, const QString &text = QString(), const SignOptions &options = AsciiArmored, const QStringList &extraOptions = QStringList());

	/**
	 * @brief sign file
	 * @param parent parent object
	 * @param signId the key to use for signing
	 * @param files list of file locations to sign (must only be 1 file)
	 * @param options signing options
	 * @param extraOptions extra signing options
	 *
	 * @warning GnuPG can currently handle only one file per invocation for
	 * signing, so files may only contain one single file.
	 */
	KGpgSignText(QObject *parent, const QString &signId, const QList<QUrl> &files, const SignOptions &options = DefaultSignature, const QStringList &extraOptions = QStringList());

	/**
	 * @brief destructor
	 */
    ~KGpgSignText() override;

	/**
	 * @brief get signing result
	 * @return signed text
	 */
	QStringList signedText() const;

protected:
	QStringList command() const override;

private:
	int m_fileIndex;
	const SignOptions m_options;
	const QString m_signId;
	QStringList m_extraOptions;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KGpgSignText::SignOptions)

#endif // KGPGSIGNTEXT_H
