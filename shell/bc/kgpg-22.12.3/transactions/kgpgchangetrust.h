/*
    SPDX-FileCopyrightText: 2008, 2009, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGCHANGETRUST_H
#define KGPGCHANGETRUST_H

#include "kgpgeditkeytransaction.h"

#include "core/kgpgkey.h"

#include <gpgme.h>
#include <QObject>

/**
 * @brief change the owner trust level of a public key
 */
class KGpgChangeTrust: public KGpgEditKeyTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgChangeTrust)
public:
    explicit KGpgChangeTrust(QObject *parent, const QString &keyid, const gpgme_validity_t trust);
    ~KGpgChangeTrust() override;

	void setTrust(const gpgme_validity_t trust);

protected:
	bool nextLine(const QString &line) override;
	ts_boolanswer boolQuestion(const QString &line) override;
	bool preStart() override;

private:
	gpgme_validity_t m_trust;
};

#endif // KGPGCHANGETRUST_H
