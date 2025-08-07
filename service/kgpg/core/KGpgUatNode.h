/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGUATNODE_H
#define KGPGUATNODE_H

#include "KGpgSignableNode.h"

#include <QDateTime>
#include <QPixmap>

class KGpgKeyNode;
class QPixmap;

class KGpgUatNodePrivate;

/**
 * @brief A user attribute (i.e. photo id) of a public key or key pair
 */
class KGpgUatNode : public KGpgSignableNode
{
private:
	KGpgUatNodePrivate * const d_ptr;
	Q_DECLARE_PRIVATE(KGpgUatNode)

protected:
	void readChildren() override;

public:
	explicit KGpgUatNode(KGpgKeyNode *parent, const unsigned int index, const QStringList &sl);
    ~KGpgUatNode() override;

	KgpgCore::KgpgItemType getType() const override;
	KgpgCore::KgpgKeyTrust getTrust() const override;
	const QPixmap &getPixmap() const;
	QString getId() const override;
	QString getSize() const override;
	QString getName() const override;
	QDateTime getCreation() const override;
	virtual KGpgKeyNode *getParentKeyNode() const;
	KGpgKeyNode *getKeyNode(void) override;
	const KGpgKeyNode *getKeyNode(void) const override;
};

#endif /* KGPGUATNODE_H */
