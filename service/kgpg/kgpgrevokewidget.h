/*
    SPDX-FileCopyrightText: 2002, 2003 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2010, 2012, 2014, 2016, 2017 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef KGPGREVOKEWIDGET_H
#define KGPGREVOKEWIDGET_H

#include <QDialog>

#include "ui_kgpgrevokewidget.h"

class KGpgKeyNode;
class QUrl;

class KgpgRevokeWidget : public QWidget, public Ui_KgpgRevokeWidget
{
    Q_OBJECT

public:
    explicit KgpgRevokeWidget(QWidget* parent = nullptr);


public Q_SLOTS:
   virtual void cbSave_toggled(bool isOn);
};

class KGpgRevokeDialog: public QDialog
{
	Q_OBJECT

	Q_DISABLE_COPY(KGpgRevokeDialog)
	KGpgRevokeDialog() = delete;
public:
	KGpgRevokeDialog(QWidget* parent, const KGpgKeyNode* node);

	QString getDescription() const;
	int getReason() const;
	QUrl saveUrl() const;
	QString getId() const;
	bool printChecked();
	bool importChecked();

	static QUrl revokeUrl(const QString &name, const QString &email);

private:
	KgpgRevokeWidget *m_revWidget;
	const QString m_id;
};

#endif
