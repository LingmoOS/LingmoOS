/*
    SPDX-FileCopyrightText: 2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpggeneratekeytesttransaction.h"

KGpgGenerateKeyTestTransaction::KGpgGenerateKeyTestTransaction(QObject *parent, const QString &name, const QString &email,
							       const QString &comment, const QByteArray &passphrase,
							       const KgpgCore::KgpgKeyAlgo algorithm, const uint size)
	: KGpgGenerateKey(parent, name, email, comment, algorithm, size)
	, m_passphrase(passphrase)
{
}

void KGpgGenerateKeyTestTransaction::askNewPassphrase(const QString &)
{
	write(m_passphrase);
	newPassphraseEntered();
}
