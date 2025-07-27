/*
    SPDX-FileCopyrightText: 2008, 2009, 2010 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGSIGNNODE_H
#define KGPGSIGNNODE_H

#include "KGpgRefNode.h"

class KGpgSignableNode;

class KGpgSignNodePrivate;

/**
 * @brief A signature to another key object
 */
class KGpgSignNode : public KGpgRefNode
{
private:
	KGpgSignNodePrivate * const d_ptr;
	Q_DECLARE_PRIVATE(KGpgSignNode)

public:
	typedef QList<KGpgSignNode *> List;

	/**
	 * @brief constructor for KGpgSignNode
	 * @param parent the signed node
	 * @param s GnuPG line describing this signature
	 */
	explicit KGpgSignNode(KGpgSignableNode *parent, const QStringList &s);
    ~KGpgSignNode() override;

	KgpgCore::KgpgItemType getType() const override;
	QDateTime getExpiration() const override;
	QString getName() const override;
	QDateTime getCreation() const override;
};

#endif /* KGPGSIGNNODE_H */
