/*
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2012, 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgchangekey.h"

#include "model/kgpgitemnode.h"
#include "transactions/kgpgchangetrust.h"
#include "transactions/kgpgchangeexpire.h"
#include "transactions/kgpgchangedisable.h"

#include <QWidget>

KGpgChangeKey::KGpgChangeKey(KGpgKeyNode *node, QWidget *widget)
	: QObject(nullptr),
	m_expiration(node->getExpiration()),
	m_key(*node->copyKey()),
	m_node(node),
	m_current(nullptr),
	m_parentWidget(widget),
	m_step(0),
	m_failed(0),
	m_autodestroy(false)
{
	m_disable = !m_key.valid();
	m_owtrust = m_key.ownerTrust();
}

KGpgChangeKey::~KGpgChangeKey()
{
	Q_ASSERT(m_current == nullptr);
}

void KGpgChangeKey::setExpiration(const QDateTime &date)
{
	m_expiration = date;
}

void KGpgChangeKey::setDisable(const bool disable)
{
	m_disable = disable;
}

void KGpgChangeKey::setOwTrust(const gpgme_validity_t trust)
{
	m_owtrust = trust;
}

bool KGpgChangeKey::apply()
{
	if (!wasChanged()) {
		Q_EMIT done(0);
		return true;
	}

	if (m_step != 0)
		return false;

	m_step = 0;
	m_failed = 0;

	nextStep(KGpgTransaction::TS_OK);

	return true;
}

void KGpgChangeKey::nextStep(int result)
{
	if (m_step == 0) {
		Q_ASSERT(sender() == nullptr);
		Q_ASSERT(m_current == nullptr);
	} else {
		Q_ASSERT(sender() != nullptr);
		Q_ASSERT(sender() == m_current);
		sender()->deleteLater();
		m_current = nullptr;
	}

	m_step++;

	switch (m_step) {
	case 1:
		if (m_expiration != m_key.expirationDate()) {
			m_current = new KGpgChangeExpire(m_parentWidget, m_key.fingerprint(), m_expiration);

			connect(m_current, &KGpgTransaction::done, this, &KGpgChangeKey::nextStep);

			m_current->start();
			break;
		} else {
			m_step++;
		}
		// fall through
        Q_FALLTHROUGH();
	case 2:
		if (result == KGpgTransaction::TS_OK) {
			m_key.setExpiration(m_expiration);
		} else {
			m_failed |= 1;
		}
		if (m_owtrust != m_key.ownerTrust()) {
			m_current = new KGpgChangeTrust(m_parentWidget, m_key.fingerprint(), m_owtrust);

			connect(m_current, &KGpgTransaction::done, this, &KGpgChangeKey::nextStep);

			m_current->start();
			break;
		} else {
			m_step++;
		}
		// fall through
        Q_FALLTHROUGH();
	case 3:
		if (result == KGpgTransaction::TS_OK) {
			m_key.setOwnerTrust(m_owtrust);
		} else {
			m_failed |= 2;
		}
		if (m_key.valid() == m_disable) {
			m_current = new KGpgChangeDisable(m_parentWidget, m_key.fingerprint(), m_disable);

			connect(m_current, &KGpgTransaction::done, this, &KGpgChangeKey::nextStep);

			m_current->start();
			break;
		} else {
			m_step++;
		}
		// fall through
        Q_FALLTHROUGH();
	default:
		if (result == KGpgTransaction::TS_OK) {
			m_key.setValid(!m_disable);
		} else {
			m_failed |= 4;
		}
		m_step = 0;
		Q_EMIT done(m_failed);
		if (m_autodestroy) {
			if (m_node)
				Q_EMIT keyNeedsRefresh(m_node);
			deleteLater();
		}
	}
}

bool KGpgChangeKey::wasChanged()
{
	if (m_key.expirationDate() != m_expiration)
		return true;

	if (m_key.ownerTrust() != m_owtrust)
		return true;

	if (m_key.valid() == m_disable)
		return true;

	return false;
}

void KGpgChangeKey::selfdestruct(const bool applyChanges)
{
	m_autodestroy = true;

	// if apply is already running it will take care of everything
	if (m_step != 0)
		return;

	if (applyChanges && wasChanged())
		apply();
	else
		deleteLater();
}

void KGpgChangeKey::setParentWidget(QWidget *widget)
{
	m_parentWidget = widget;
	if (m_current != nullptr)
		m_current->setParent(widget);
}
