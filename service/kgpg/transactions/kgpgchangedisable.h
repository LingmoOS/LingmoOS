/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGCHANGEDISABLE_H
#define KGPGCHANGEDISABLE_H

#include <QObject>

#include "kgpgeditkeytransaction.h"

/**
 * @brief enable of disable a key
 */
class KGpgChangeDisable: public KGpgEditKeyTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgChangeDisable)
	KGpgChangeDisable() = delete;

public:
    explicit KGpgChangeDisable(QObject *parent, const QString &keyid, const bool disable);
    ~KGpgChangeDisable() override;

	void setDisable(bool disable);

protected:
	bool preStart() override;
};

#endif // KGPGCHANGEDISABLE_H
