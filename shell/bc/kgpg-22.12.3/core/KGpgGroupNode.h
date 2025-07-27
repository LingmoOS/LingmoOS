/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KGPGGROUPNODE_H
#define KGPGGROUPNODE_H

#include "KGpgExpandableNode.h"
#include "KGpgKeyNode.h"
#include <QStringList>
class QString;

class KGpgGroupNodePrivate;

/**
 * @brief A GnuPG group of public keys
 */
class KGpgGroupNode : public KGpgExpandableNode
{
private:
	KGpgGroupNodePrivate * const d_ptr;
	Q_DECLARE_PRIVATE(KGpgGroupNode)
	Q_DISABLE_COPY(KGpgGroupNode)

protected:
        void readChildren() override;

public:
	KGpgGroupNode(KGpgRootNode *parent, const QString &name, const QStringList &members);
	KGpgGroupNode(KGpgRootNode *parent, const QString &name, const KGpgKeyNode::List &members);
    ~KGpgGroupNode() override;

        KgpgCore::KgpgItemType getType() const override;
	/**
	 * Return size of group
	 *
	 * @return the number of keys in this group
	 */
        QString getSize() const override;
        QString getName() const override;

	/**
	 * Rename this group node
	 *
	 * @param newName new name of the group
	 */
	void rename(const QString &newName);

	/**
	 * Write the current members to GnuPG config file
	 */
	void saveMembers();

	/**
	 * Remove this group from the GnuPG config file
	 */
	void remove();
};

#endif /* KGPGGROUPNODE_H */
