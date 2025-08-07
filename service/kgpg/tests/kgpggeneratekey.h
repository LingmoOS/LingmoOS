/*
    SPDX-FileCopyrightText: 2022 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDELKEY_TEST_H
#define KGPGDELKEY_TEST_H

#include <QObject>
#include <QTemporaryDir>

class KGpgGenerateKeyTest : public QObject {
	Q_OBJECT
private Q_SLOTS:
	void init();
	void testDeleteKey();
	void testDeleteKey_data();

private:
	QTemporaryDir m_tempdir;
};

#endif
