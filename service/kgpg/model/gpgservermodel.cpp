/*
    SPDX-FileCopyrightText: 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gpgservermodel.h"

#include <KLocalizedString>
#include <QStringList>

GpgServerModel::GpgServerModel(QObject *parent)
	: QStringListModel(parent),
	m_defaultRow(-1)
{
	connect(this, &GpgServerModel::rowsRemoved, this, &GpgServerModel::slotRowsRemoved);
}

GpgServerModel::~GpgServerModel()
{
}

void
GpgServerModel::setDefault(const int row)
{
	Q_ASSERT(row < stringList().count());

	if (m_defaultRow == row)
		return;

	const int oldRow = m_defaultRow;
	m_defaultRow = row;
	if (oldRow >= 0)
		Q_EMIT dataChanged(index(oldRow, 0), index(oldRow, 0));
	if (row >= 0)
		Q_EMIT dataChanged(index(row, 0), index(row, 0));
}

void
GpgServerModel::setDefault(const QString &server)
{
	if (server.isEmpty()) {
		setDefault(-1);
	} else {
		const int row = stringList().indexOf(server);
		Q_ASSERT(row >= 0);
		setDefault(row);
	}
}

int
GpgServerModel::defaultRow() const
{
	// only in case there is not set any default yet promote the first entry of the list
	if ((m_defaultRow >= 0) || stringList().empty())
		return m_defaultRow;
	else
		return 0;
}

QString
GpgServerModel::defaultServer() const
{
	const QStringList &servers = stringList();

	if (servers.isEmpty())
		return QString();
	return servers.at(qMax<int>(0, m_defaultRow));
}

QVariant
GpgServerModel::data(const QModelIndex &index, int role) const
{
	QVariant res = QStringListModel::data(index, role);

	if ((role == Qt::DisplayRole) && (index.row() == m_defaultRow))
		res = i18nc("Mark default keyserver in GUI", "%1 (Default)", res.toString());

	return res;
}

void
GpgServerModel::slotRowsRemoved(const QModelIndex &, int start, int end)
{
	if (end < m_defaultRow) {
		// removed before default, i.e. default is moved up
		setDefault(m_defaultRow - (end - start) - 1);
	} else if ((start <= m_defaultRow) && (end >= m_defaultRow)) {
		// the default was deleted
		if (m_defaultRow >= rowCount())
			m_defaultRow = -1; // avoid sending dataChanged() for the already deleted row
		if (rowCount() > 0)
			setDefault(0);
	}
}
