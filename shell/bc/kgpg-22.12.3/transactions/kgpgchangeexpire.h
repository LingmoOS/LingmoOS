/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGCHANGEEXPIRE_H
#define KGPGCHANGEEXPIRE_H

#include <QObject>
#include <QDateTime>

#include "kgpgeditkeytransaction.h"

/**
 * @brief change the key lifetime
 */
class KGpgChangeExpire: public KGpgEditKeyTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgChangeExpire)
public:
    explicit KGpgChangeExpire(QObject *parent, const QString &keyid, const QDateTime &date);
    ~KGpgChangeExpire() override;

	void setDate(const QDateTime &date);

protected:
	bool nextLine(const QString &line) override;

private:
	QDateTime m_date;
};

#endif // KGPGCHANGEEXPIRE_H
