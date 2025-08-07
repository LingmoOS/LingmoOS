/*
    SPDX-FileCopyrightText: 2009, 2010, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KGPGCAFF_P_H
#define _KGPGCAFF_P_H

#include "caff.h"
#include "core/KGpgSignableNode.h"
#include "transactions/kgpgsigntransactionhelper.h"

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QStringList>

class QTemporaryDir;

class KGpgCaffPrivate : public QObject {

	KGpgCaff * const q_ptr;
	Q_DECLARE_PUBLIC(KGpgCaff)
	Q_DISABLE_COPY(KGpgCaffPrivate)

	QScopedPointer<QTemporaryDir> m_tempdir;
	QStringList m_signers;
	QString m_secringfile;
	QString m_secringdir; ///< where GnuPG >=2.1 store their secret keyring information
	const KGpgCaff::OperationFlags m_flags;
	const KGpgSignTransactionHelper::carefulCheck m_checklevel;
	const int m_gpgVersion;

	void reexportKey(const KGpgSignableNode *node);
	void abortOperation(int result);
	void checkNextLoop();
public:
	KGpgCaffPrivate(KGpgCaff *parent, const KGpgSignableNode::List &ids, const QStringList &signers,
			const KGpgCaff::OperationFlags flags, const KGpgSignTransactionHelper::carefulCheck checklevel);
	~KGpgCaffPrivate() override;

	KGpgSignableNode::List m_allids;
	KGpgSignableNode::const_List m_noEncIds;	///< keys without encryption capability that were skipped
	KGpgSignableNode::const_List m_alreadyIds;	///< ids already signed

private:
	void slotSigningFinished(int result);
	void slotDelUidFinished(int result);
	void slotExportFinished(int result);
	void slotTextEncrypted(int result);
	void slotReimportDone(int result);
};

#endif /* _KGPGCAFF_P_H */
