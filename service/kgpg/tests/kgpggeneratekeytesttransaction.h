/*
    SPDX-FileCopyrightText: 2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDELKEY_TEST_TRANSACTION_H
#define KGPGDELKEY_TEST_TRANSACTION_H

#include "../transactions/kgpggeneratekey.h"

/**
 * Special version of the transaction that does not show a window for the
 * passphrase but just uses the one set in the constructor
 */
class KGpgGenerateKeyTestTransaction : public KGpgGenerateKey {
	Q_OBJECT
public:
	KGpgGenerateKeyTestTransaction(QObject *parent, const QString &name, const QString &email, const QString &comment,
					const QByteArray &passphrase, const KgpgCore::KgpgKeyAlgo algorithm, const uint size);
	~KGpgGenerateKeyTestTransaction() override = default;

	void askNewPassphrase(const QString &) override;

private:
	const QByteArray m_passphrase;
};

#endif
