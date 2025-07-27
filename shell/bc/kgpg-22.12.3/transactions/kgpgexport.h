/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGEXPORT_H
#define KGPGEXPORT_H

#include <QObject>
#include <QStringList>

#include "kgpgtransaction.h"

class QProcess;

/**
 * @brief export one or more keys from keyring
 *
 * The exported keys can be written to a file or sent to standard input of another
 * QProcess.
 */
class KGpgExport: public KGpgTransaction {
	Q_OBJECT

	KGpgExport();	// = delete C++0x
	Q_DISABLE_COPY(KGpgExport)
public:
	/**
	 * @brief export keys to QProcess
	 * @param parent parent object
	 * @param ids ids to export
	 * @param outp process to write into
	 * @param options additional options to pass to GnuPG (e.g. export ascii armored)
	 * @param secret if secret key exporting is allowed
	 */
	KGpgExport(QObject *parent, const QStringList &ids, QProcess *outp, const QStringList &options = QStringList(), const bool secret = false);

	/**
	 * @brief export keys to KGpgTransaction
	 * @param parent parent object
	 * @param ids ids to export
	 * @param outt transaction to write into
	 * @param options additional options to pass to GnuPG (e.g. export ascii armored)
	 * @param secret if secret key exporting is allowed
	 */
	KGpgExport(QObject *parent, const QStringList &ids, KGpgTransaction *outt, const QStringList &options = QStringList(), const bool secret = false);

	/**
	 * @brief export keys to file
	 * @param parent parent object
	 * @param ids ids to export
	 * @param file filename to write into
	 * @param options additional options to pass to GnuPG (e.g. export ascii armored)
	 * @param secret if secret key exporting is allowed
	 */
	KGpgExport(QObject *parent, const QStringList &ids, const QString &file, const QStringList &options = QStringList(), const bool secret = false);

	/**
	 * @brief export keys to standard output
	 * @param parent parent object
	 * @param ids ids to export
	 * @param options additional options to pass to GnuPG (e.g. export ascii armored)
	 * @param secret if secret key exporting is allowed
	 *
	 * Only ascii-armored export is supported in standard output mode. If it is not
	 * already set in the given option it will be added automatically.
	 */
	KGpgExport(QObject *parent, const QStringList &ids, const QStringList &options = QStringList(), const bool secret = false);

	/**
	 * @brief destructor
	 */
    ~KGpgExport() override;

	/**
	 * @brief set key id to export
	 * @param id key fingerprint
	 */
	void setKeyId(const QString &id);
	/**
	 * @brief set key ids to export
	 * @param ids key fingerprints
	 */
	void setKeyIds(const QStringList &ids);
	/**
	 * @brief return the key ids to export
	 * @return list of key fingerprints
	 */
	const QStringList &getKeyIds() const;
	/**
	 * @brief set the process the output is sent to
	 * @param outp process to send output to
	 */
	void setOutputProcess(QProcess *outp);
	/**
	 * @brief set the transaction the output is sent to
	 * @param outt transaction to send output to
	 */
	void setOutputTransaction(KGpgTransaction *outt);
	/**
	 * @brief set filename to send output to
	 * @param filename file to send output to
	 */
	void setOutputFile(const QString &filename);
	/**
	 * @brief return the output filename currently set
	 * @return filename key will get written to
	 */
	const QString &getOutputFile() const;
	/**
	 * @brief return the data read from standard output
	 * @return standard output data
	 */
	const QByteArray &getOutputData() const;

protected:
	bool preStart() override;
	bool nextLine(const QString &line) override;

private:
	QStringList m_keyids;
	QProcess *m_outp;
	QString m_outf;
	QByteArray m_data;

	enum OutputMode {
		ModeFile = 0,
		ModeProcess = 1,
		ModeStdout = 2,
		ModeTransaction = 3
	};
	enum OutputMode m_outputmode;

	void procSetup(const QStringList &options, const bool secret);
};

#endif // KGPGEXPORT_H
