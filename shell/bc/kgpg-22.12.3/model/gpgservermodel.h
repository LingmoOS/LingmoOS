/*
    SPDX-FileCopyrightText: 2014 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef GPGSERVERMODEL_H
#define GPGSERVERMODEL_H

#include <QStringListModel>

/**
 * @brief model holding the configured GnuPG key servers
 */
class GpgServerModel: public QStringListModel {
	Q_OBJECT
public:
	explicit GpgServerModel(QObject *parent = nullptr);
    ~GpgServerModel() override;

	void setDefault(const QString &server);
	void setDefault(const int index);

	/**
	 * @brief returns the row of the current default keyserver
	 */
	int defaultRow() const;

	/**
	 * @brief returns the URL of the default server
	 * @retval QString() if no default server is selected
	 */
	QString defaultServer() const;

    QVariant data(const QModelIndex &index, int role) const override;

private Q_SLOTS:
	void slotRowsRemoved(const QModelIndex &, int start, int end);

private:
	int m_defaultRow;
};

#endif
