/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGSUBKEYNODE_H
#define KGPGSUBKEYNODE_H

#include "KGpgSignableNode.h"

#include "kgpgkey.h"

using namespace KgpgCore;

/**
 * @brief a subkey of a public key or key pair
 */
class KGpgSubkeyNode : public KGpgSignableNode
{
private:
	KgpgCore::KgpgKeySub m_skey;
	QString m_fingerprint;

protected:
	void readChildren() override;

public:
	explicit KGpgSubkeyNode(KGpgKeyNode *parent, const KgpgCore::KgpgKeySub &k);
    ~KGpgSubkeyNode() override;

	KgpgCore::KgpgItemType getType() const override;
	KgpgCore::KgpgKeyTrust getTrust() const override;
	QString getSize() const override;
	QString getName() const override;
	QDateTime getExpiration() const override;
	QDateTime getCreation() const override;
	QString getId() const override;
	KGpgKeyNode *getKeyNode(void) override;
	const KGpgKeyNode *getKeyNode(void) const override;
	virtual KGpgKeyNode *getParentKeyNode() const;

	void setFingerprint(const QString &fpr);
	const QString &getFingerprint() const;
};

#endif /* KGPGSUBKEYNODE_H */
