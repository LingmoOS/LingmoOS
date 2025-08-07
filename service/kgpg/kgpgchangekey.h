/*
    SPDX-FileCopyrightText: 2008, 2012, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGCHANGEKEY_H
#define KGPGCHANGEKEY_H

#include "core/kgpgkey.h"

#include <gpgme.h>
#include <QObject>
#include <QDateTime>

class KGpgKeyNode;
class KGpgTransaction;
class QWidget;

/**
 * @short A class for changing several properties of a key at once
 *
 * This class can enable or disable a key and at the same time change owner
 * trust and expiration. It may also run in "detached" mode, i.e. the creator
 * may be destroyed and the class will finish it's operation and delete itself
 * when done.
 *
 * The class may be reused, i.e. if one change operation finished another one
 * can be started (for the same key). It does not take care of any locking,
 * the caller must prevent calls to this object while it works.
 *
 * None of the functions in this object block so it's safe to be called from
 * the main application thread (e.g. it will not freeze the GUI).
 *
 * @author Rolf Eike Beer <kde@opensource.sf-tec.de>
 */
class KGpgChangeKey : public QObject
{
	Q_OBJECT

public:
	/**
	 * Creates a change object for a given key node
	 *
	 * @param node pointer to key node to take care of
	 * @param widget parent widget for password dialogs
	 *
	 * KGpgChangeKey stores a copy of the key object of the node
	 * internally to track changes it made. Once everything is
	 * finished the caller get notified that this node needs refresh.
	 *
	 * The widget parameter will not be used to parent this object as
	 * it may need to run even after the parent window was closed.
	 * This widget will be used as parent for the transactions so that
	 * they can show a passphrase prompt with correct widget inheritance,
	 * otherwise the modal passphrase dialog may be blocked by the modal
	 * key info dialog. Do not forget to call setParentWidget() if you
	 * destroy the parent widget while this still needs to run.
	 */
	KGpgChangeKey(KGpgKeyNode *node, QWidget *widget);
	/**
	 * Destroys the object
	 */
	~KGpgChangeKey() override;

	/**
	 * Cache new expiration date
	 *
	 * @param date new expiration date or QDateTime() if key should get
	 * unlimited lifetime
	 */
	void setExpiration(const QDateTime &date);

	/**
	 * Cache new disable flag
	 *
	 * @param disable if the key should become disabled or not
	 */
	void setDisable(const bool disable);

	/**
	 * Cache new owner trust
	 *
	 * @param trust new owner trust level
	 */
	void setOwTrust(const gpgme_validity_t trust);

	/**
	 * Apply all cached changes to the key
	 *
	 * @return true if started successfully or false if already running
	 * (this should never happen).
	 *
	 * It is save to call this function if there were no changes to the
	 * key. It will detect this case and will exit immediately.
	 *
	 * The done() signal is emitted when this function has done all the
	 * work. It is also emitted if the function was called without work to
	 * do. This case is not considered an error.
	 */
	bool apply();

	/**
	 * Checks if the cached values differ from those of the key
	 *
	 * @return true if the cached values differ from the stored key
	 *
	 * This compares the cached values to those of the stored key. If you
	 * change one value forth and back this function would return false at
	 * the end.
	 */
	bool wasChanged();

	/**
	 * Tell the object to remove itself once all work is done
	 *
	 * @param applyChanges if pending changes should be applied or dropped
	 *
	 * This function may safely be called whether apply() is running or not.
	 * If applyChanges is set to yes apply() is called and the object
	 * deletes itself once all changes are done. If apply() already runs
	 * it is noticed to destruct afterwards. If applyChanges is set to
	 * false and apply() is not running or if no work has to be done the
	 * object is destroyed the next time the event loop runs.
	 */
	void selfdestruct(const bool applyChanges);

	/**
	 * @brief set a new parent widget for the transactions
	 */
	void setParentWidget(QWidget *widget);
Q_SIGNALS:
	/**
	 * This signal gets emitted every time apply() has done all of it's work.
	 *
	 * The result argument will be 0 if everything went fine. If no work
	 * had to be done that was fine, too. If anything goes wrong it will be
	 * a logic or of every change that caused trouble which can be
	 * expiration date (1), owner trust (2), or disable (4).
	 */
	void done(int result);

	/**
	 * This signal get's emitted if apply finishes in detached mode
	 *
	 * When the class is in detached mode (i.e. selfdestruct() was called)
	 * and the key operation finishes this signal is emitted. If apply()
	 * finishes and the object is not in detached mode only done() is
	 * emitted.
	 *
	 * The reason for this behaviour is that if the owner is around he can
	 * take care of noticing it's parent when it's the best time (e.g. on
	 * dialog close). If the owner is gone we need to inform his parent
	 * about the change ourself. When the owner is around we usually don't
	 * want to refresh the key immediately as the user might do another
	 * change and we want to avoid useless refreshes as they are rather
	 * expensive.
	 */
	void keyNeedsRefresh(KGpgKeyNode *node);

private Q_SLOTS:
	/**
	 * @internal
	 */
	void nextStep(int result);

private:
	QDateTime m_expiration;
	bool m_disable;
	gpgme_validity_t m_owtrust;
	KgpgCore::KgpgKey m_key;
	KGpgKeyNode *m_node;
	KGpgTransaction *m_current;	///< the currently active transaction object
	QWidget *m_parentWidget;
	int m_step;
	int m_failed;
	bool m_autodestroy;
};

#endif // KGPGCHANGEKEY_H
