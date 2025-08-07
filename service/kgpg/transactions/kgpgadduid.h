/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGADDUID_H
#define KGPGADDUID_H

#include <QObject>

#include "kgpgeditkeytransaction.h"

class QString;

/**
 * @brief add a new user id to a key pair
 */
class KGpgAddUid: public KGpgEditKeyTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgAddUid)
public:
	KGpgAddUid(QObject *parent, const QString &keyid, const QString &name, const QString &email = QString(), const QString &comment = QString());
    ~KGpgAddUid() override;

	void setName(const QString &name);
	void setEmail(const QString &email);
	void setComment(const QString &comment);

protected:
	bool preStart() override;
	bool nextLine(const QString &line) override;

private:
	QString m_name;
	QString m_email;
	QString m_comment;
};

#endif // KGPGADDUID_H
