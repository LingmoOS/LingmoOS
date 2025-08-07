/*
    SPDX-FileCopyrightText: 2009, 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KGPGCAFF_H
#define _KGPGCAFF_H

#include "core/KGpgSignableNode.h"

#include <QObject>
#include <QStringList>


class KGpgCaffPrivate;

class KGpgCaff : public QObject {
	Q_OBJECT

	KGpgCaffPrivate * const d_ptr;
	Q_DECLARE_PRIVATE(KGpgCaff)
	Q_DISABLE_COPY(KGpgCaff)

public:
	enum OperationFlags {
		DefaultMode = 0,		///< use none of the other flags
		IgnoreAlreadySigned = 1		///< uids that are already signed will not be mailed again
	};

	/**
	 * @brief create a new object to sign and mail key ids
	 * @param parent parent object
	 * @param ids list of keys to sign
	 * @param signids secret key ids to sign @p ids with
	 * @param checklevel check level, @see carefulCheck
	 * @param flags control flags
	 */
	KGpgCaff(QObject *parent, const KGpgSignableNode::List &ids, const QStringList &signids,
			const int checklevel = 0, const OperationFlags flags = DefaultMode);

public Q_SLOTS:
	void run();

Q_SIGNALS:
	void done();
	void aborted();
};

#endif /* _KGPGCAFF_H */
