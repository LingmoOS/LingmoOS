/*
    SPDX-FileCopyrightText: 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGEDITKEYTRANSACTION_H
#define KGPGEDITKEYTRANSACTION_H

#include <QObject>

#include "kgpgtransaction.h"

/**
 * @brief edit a single property of a key
 */
class KGpgEditKeyTransaction: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgEditKeyTransaction)
	KGpgEditKeyTransaction() = delete;

protected:
	/**
	 * @brief constructor
	 * @param parent parent object
	 * @param keyid key to edit
	 * @param command GnuPG command to use
	 * @param hasValue if the command takes an extra argument
	 * @param autoSave if a "save" command should be sent to GnuPG immediately
	 */
	KGpgEditKeyTransaction(QObject *parent, const QString &keyid, const QString &command, const bool hasValue, const bool autoSave = true);

public:
	/**
	 * @brief destructor
	 */
    ~KGpgEditKeyTransaction() override;

	/**
	 * @brief return the id of the key we are editing
	 */
	QString getKeyid() const;

protected:
	/**
	 * @brief reset class before next operation starts
	 *
	 * If you inherit from this class make sure this method is called
	 * from your inherited method before you do anything else there.
	 */
	bool preStart() override;

	/**
	 * @brief handle standard GnuPG prompts
	 * @param line the line to handle
	 *
	 * By default this handles passphrase questions and quits the
	 * operation when GnuPG returns to it's command prompt. The
	 * "GOOD_PASSPHRASE" line is _not_ handled here. When you inherit
	 * this class and need to handle specific line do them first and
	 * then call this method at the end of your method to handle all
	 * standard things (if you don't want to handle them yourself).
	 * Every line sent here by GnuPG not recognised as command handled
	 * here will set a sequence error so be sure to handle your stuff first!
	 */
	bool nextLine(const QString &line) override;

	ts_boolanswer boolQuestion(const QString &line) override;

	/**
	 * @brief replace the argument of the edit command
	 * @param arg new argument
	 *
	 * Calling this function when the hasValue parameter of the constructor was false is an error.
	 */
	void replaceValue(const QString &arg);

	/**
	 * @brief replace the command
	 * @param cmd new command
	 *
	 * This is seldomly needed, only when a command has different names
	 * for positive or negative action instead of taking that as argument.
	 */
	void replaceCommand(const QString &cmd);

private:
	int m_cmdpos;	///< position of the command thas is passed to the edit command
	int m_argpos;	///< position of the argument that is passed to the edit command
	const bool m_autosave;	///< if autosave was requested in constructor
	const QString m_keyid;	///< id of the key we are editing
};

#endif // KGPGEDITKEYTRANSACTION_H
