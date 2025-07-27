/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGCHANGEPASS_H
#define KGPGCHANGEPASS_H

#include <QObject>

#include "kgpgtransaction.h"

/**
 * @brief set a new passphrase for a key pair
 */
class KGpgChangePass: public KGpgTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgChangePass)
public:
    explicit KGpgChangePass(QObject *parent, const QString &keyid);
    ~KGpgChangePass() override;

protected:
	bool nextLine(const QString &line) override;
	bool preStart() override;
	bool passphraseRequested() override;
	bool passphraseReceived() override;
	bool hintLine(const KGpgTransaction::ts_hintType hint, const QString & args) override;

private:
	bool m_seenold;		///< old password correctly entered
};

#endif // KGPGCHANGEPASS_H
